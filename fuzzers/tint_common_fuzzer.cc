// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "fuzzers/tint_common_fuzzer.h"

#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "src/ast/module.h"
#include "src/diagnostic/formatter.h"
#include "src/program.h"
#include "src/utils/hash.h"

namespace tint {
namespace fuzzers {

namespace {

[[noreturn]] void FatalError(const tint::diag::List& diags,
                             const std::string& msg = "") {
  auto printer = tint::diag::Printer::create(stderr, true);
  if (!msg.empty()) {
    printer->write(msg + "\n", {diag::Color::kRed, true});
  }
  tint::diag::Formatter().format(diags, printer.get());
  __builtin_trap();
}

[[noreturn]] void TintInternalCompilerErrorReporter(
    const tint::diag::List& diagnostics) {
  FatalError(diagnostics);
}

transform::VertexAttributeDescriptor ExtractVertexAttributeDescriptor(
    Reader* r) {
  transform::VertexAttributeDescriptor desc{};
  desc.format = r->enum_class<transform::VertexFormat>(
      static_cast<uint8_t>(transform::VertexFormat::kLastEntry) + 1);
  desc.offset = r->read<uint32_t>();
  desc.shader_location = r->read<uint32_t>();
  return desc;
}

transform::VertexBufferLayoutDescriptor ExtractVertexBufferLayoutDescriptor(
    Reader* r) {
  transform::VertexBufferLayoutDescriptor desc;
  desc.array_stride = r->read<uint32_t>();
  desc.step_mode = r->enum_class<transform::VertexStepMode>(
      static_cast<uint8_t>(transform::VertexStepMode::kLastEntry) + 1);
  desc.attributes = r->vector(ExtractVertexAttributeDescriptor);
  return desc;
}

bool SPIRVToolsValidationCheck(const tint::Program& program,
                               const std::vector<uint32_t>& spirv) {
  spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
  const tint::diag::List& diags = program.Diagnostics();
  tools.SetMessageConsumer([diags](spv_message_level_t, const char*,
                                   const spv_position_t& pos, const char* msg) {
    std::stringstream out;
    out << "Unexpected spirv-val error:\n"
        << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg
        << std::endl;

    auto printer = tint::diag::Printer::create(stderr, true);
    printer->write(out.str(), {diag::Color::kYellow, false});
    tint::diag::Formatter().format(diags, printer.get());
  });

  return tools.Validate(spirv.data(), spirv.size(),
                        spvtools::ValidatorOptions());
}

}  // namespace

Reader::Reader(const uint8_t* data, size_t size) : data_(data), size_(size) {}

std::string Reader::string() {
  auto count = read<uint8_t>();
  if (failed_ || size_ < count) {
    mark_failed();
    return "";
  }
  std::string out(data_, data_ + count);
  data_ += count;
  size_ -= count;
  return out;
}

void Reader::mark_failed() {
  size_ = 0;
  failed_ = true;
}

void Reader::read(void* out, size_t n) {
  if (n > size_) {
    mark_failed();
    return;
  }
  memcpy(out, data_, n);
  data_ += n;
  size_ -= n;
}

void ExtractBindingRemapperInputs(Reader* r, tint::transform::DataMap* inputs) {
  struct Config {
    uint8_t old_group;
    uint8_t old_binding;
    uint8_t new_group;
    uint8_t new_binding;
    ast::Access new_access;
  };

  std::vector<Config> configs = r->vector<Config>();
  transform::BindingRemapper::BindingPoints binding_points;
  transform::BindingRemapper::AccessControls accesses;
  for (const auto& config : configs) {
    binding_points[{config.old_binding, config.old_group}] = {
        config.new_binding, config.new_group};
    accesses[{config.old_binding, config.old_group}] = config.new_access;
  }

  inputs->Add<transform::BindingRemapper::Remappings>(binding_points, accesses);
}

void ExtractFirstIndexOffsetInputs(Reader* r,
                                   tint::transform::DataMap* inputs) {
  struct Config {
    uint32_t group;
    uint32_t binding;
  };

  Config config = r->read<Config>();
  inputs->Add<tint::transform::FirstIndexOffset::BindingPoint>(config.binding,
                                                               config.group);
}

void ExtractSingleEntryPointInputs(Reader* r,
                                   tint::transform::DataMap* inputs) {
  std::string input = r->string();
  transform::SingleEntryPoint::Config cfg(input);
  inputs->Add<transform::SingleEntryPoint::Config>(cfg);
}

void ExtractVertexPullingInputs(Reader* r, tint::transform::DataMap* inputs) {
  transform::VertexPulling::Config cfg;
  cfg.entry_point_name = r->string();
  cfg.vertex_state = r->vector(ExtractVertexBufferLayoutDescriptor);
  cfg.pulling_group = r->read<uint32_t>();
  inputs->Add<transform::VertexPulling::Config>(cfg);
}

void ExtractSpirvOptions(Reader* r, writer::spirv::Options* options) {
  *options = r->read<writer::spirv::Options>();
}

void ExtractWgslOptions(Reader* r, writer::wgsl::Options* options) {
  *options = r->read<writer::wgsl::Options>();
}

void ExtractHlslOptions(Reader* r, writer::hlsl::Options* options) {
  *options = r->read<writer::hlsl::Options>();
}

void ExtractMslOptions(Reader* r, writer::msl::Options* options) {
  *options = r->read<writer::msl::Options>();
}

CommonFuzzer::CommonFuzzer(InputFormat input, OutputFormat output)
    : input_(input),
      output_(output),
      transform_manager_(nullptr),
      inspector_enabled_(false) {}

CommonFuzzer::~CommonFuzzer() = default;

int CommonFuzzer::Run(const uint8_t* data, size_t size) {
  tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

  Program program;

#if TINT_BUILD_SPV_READER
  std::vector<uint32_t> spirv_input(size / sizeof(uint32_t));

#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER || TINT_BUILD_SPV_READER
  auto dump_input_data = [&](auto& content, const char* extension) {
    size_t hash = utils::Hash(content);
    auto filename = "fuzzer_input_" + std::to_string(hash) + extension;  //
    std::ofstream fout(filename, std::ios::binary);
    fout.write(reinterpret_cast<const char*>(data),
               static_cast<std::streamsize>(size));
    std::cout << "Dumped input data to " << filename << std::endl;
  };
#endif

  switch (input_) {
#if TINT_BUILD_WGSL_READER
    case InputFormat::kWGSL: {
      // Clear any existing diagnostics, as these will hold pointers to file_,
      // which we are about to release.
      diagnostics_ = {};
      std::string str(reinterpret_cast<const char*>(data), size);
      file_ = std::make_unique<Source::File>("test.wgsl", str);
      if (dump_input_) {
        dump_input_data(str, ".wgsl");
      }
      program = reader::wgsl::Parse(file_.get());
      break;
    }
#endif  // TINT_BUILD_WGSL_READER
#if TINT_BUILD_SPV_READER
    case InputFormat::kSpv: {
      // `spirv_input` has been initialized with the capacity to store `size /
      // sizeof(uint32_t)` uint32_t values. If `size` is not a multiple of
      // sizeof(uint32_t) then not all of `data` can be copied into
      // `spirv_input`, and any trailing bytes are discarded.
      std::memcpy(spirv_input.data(), data,
                  spirv_input.size() * sizeof(uint32_t));
      if (spirv_input.empty()) {
        return 0;
      }
      if (dump_input_) {
        dump_input_data(spirv_input, ".spv");
      }
      program = reader::spirv::Parse(spirv_input);
      break;
    }
#endif  // TINT_BUILD_SPV_READER
    default:
      return 0;
  }

  if (output_ == OutputFormat::kNone) {
    return 0;
  }

  if (!program.IsValid()) {
    diagnostics_ = program.Diagnostics();
    return 0;
  }

#if TINT_BUILD_SPV_READER
  if (input_ == InputFormat::kSpv &&
      !SPIRVToolsValidationCheck(program, spirv_input)) {
    FatalError(program.Diagnostics(),
               "Fuzzing detected invalid input spirv not being caught by Tint");
  }
#endif  // TINT_BUILD_SPV_READER

  if (inspector_enabled_) {
    inspector::Inspector inspector(&program);

    auto entry_points = inspector.GetEntryPoints();
    if (inspector.has_error()) {
      diagnostics_.add_error(tint::diag::System::Inspector, inspector.error());
      return 0;
    }

    for (auto& ep : entry_points) {
      auto remapped_name = inspector.GetRemappedNameForEntryPoint(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto constant_ids = inspector.GetConstantIDs();
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto uniform_bindings =
          inspector.GetUniformBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto storage_bindings =
          inspector.GetStorageBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto readonly_bindings =
          inspector.GetReadOnlyStorageBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto sampler_bindings = inspector.GetSamplerResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto comparison_sampler_bindings =
          inspector.GetComparisonSamplerResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto sampled_texture_bindings =
          inspector.GetSampledTextureResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }

      auto multisampled_texture_bindings =
          inspector.GetMultisampledTextureResourceBindings(ep.name);
      if (inspector.has_error()) {
        diagnostics_.add_error(tint::diag::System::Inspector,
                               inspector.error());
        return 0;
      }
    }
  }

  if (transform_manager_) {
    auto out = transform_manager_->Run(&program, transform_inputs_);
    if (!out.program.IsValid()) {
      // Transforms can produce error messages for bad input.
      // Catch ICEs and errors from non transform systems.
      for (const auto& diag : out.program.Diagnostics()) {
        if (diag.severity > diag::Severity::Error ||
            diag.system != diag::System::Transform) {
          FatalError(out.program.Diagnostics(),
                     "Fuzzing detected valid input program being transformed "
                     "into an invalid output program");
        }
      }
    }

    program = std::move(out.program);
  }

  switch (output_) {
    case OutputFormat::kWGSL: {
#if TINT_BUILD_WGSL_WRITER
      auto result = writer::wgsl::Generate(&program, options_wgsl_);
      generated_wgsl_ = std::move(result.wgsl);
      if (!result.success) {
        FatalError(program.Diagnostics(),
                   "WGSL writer failed: " + result.error);
      }
#endif  // TINT_BUILD_WGSL_WRITER
      break;
    }
    case OutputFormat::kSpv: {
#if TINT_BUILD_SPV_WRITER
      auto result = writer::spirv::Generate(&program, options_spirv_);
      generated_spirv_ = std::move(result.spirv);
      if (!result.success) {
        FatalError(program.Diagnostics(),
                   "SPIR-V writer failed: " + result.error);
      }
      if (!SPIRVToolsValidationCheck(program, generated_spirv_)) {
        FatalError(program.Diagnostics(),
                   "Fuzzing detected invalid spirv being emitted by Tint");
      }

#endif  // TINT_BUILD_SPV_WRITER
      break;
    }
    case OutputFormat::kHLSL: {
#if TINT_BUILD_HLSL_WRITER
      auto result = writer::hlsl::Generate(&program, options_hlsl_);
      generated_hlsl_ = std::move(result.hlsl);
      if (!result.success) {
        FatalError(program.Diagnostics(),
                   "HLSL writer failed: " + result.error);
      }
#endif  // TINT_BUILD_HLSL_WRITER
      break;
    }
    case OutputFormat::kMSL: {
#if TINT_BUILD_MSL_WRITER
      auto result = writer::msl::Generate(&program, options_msl_);
      generated_msl_ = std::move(result.msl);
      if (!result.success) {
        FatalError(program.Diagnostics(), "MSL writer failed: " + result.error);
      }
#endif  // TINT_BUILD_MSL_WRITER
      break;
    }
    case OutputFormat::kNone:
      break;
  }

  return 0;
}

}  // namespace fuzzers
}  // namespace tint
