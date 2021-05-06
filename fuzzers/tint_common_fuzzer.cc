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

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "src/ast/module.h"
#include "src/program.h"
#include "src/program_builder.h"

namespace tint {
namespace fuzzers {

namespace {

[[noreturn]] void TintInternalCompilerErrorReporter(
    const tint::diag::List& diagnostics) {
  auto printer = tint::diag::Printer::create(stderr, true);
  tint::diag::Formatter{}.format(diagnostics, printer.get());
  __builtin_trap();
}

[[noreturn]] void ValidityErrorReporter() {
  auto printer = tint::diag::Printer::create(stderr, true);
  printer->write(
      "Fuzzing detected valid input program being transformed into an invalid "
      "output progam\n",
      {diag::Color::kRed, true});
  __builtin_trap();
}

transform::VertexAttributeDescriptor ExtractVertexAttributeDescriptor(
    Reader* r) {
  transform::VertexAttributeDescriptor desc;
  desc.format = r->enum_class<transform::VertexFormat>(
      static_cast<uint8_t>(transform::VertexFormat::kLastEntry) + 1);
  desc.offset = r->read<uint64_t>();
  desc.shader_location = r->read<uint32_t>();
  return desc;
}

transform::VertexBufferLayoutDescriptor ExtractVertexBufferLayoutDescriptor(
    Reader* r) {
  transform::VertexBufferLayoutDescriptor desc;
  desc.array_stride = r->read<uint64_t>();
  desc.step_mode = r->enum_class<transform::InputStepMode>(
      static_cast<uint8_t>(transform::InputStepMode::kLastEntry) + 1);
  desc.attributes = r->vector(ExtractVertexAttributeDescriptor);
  return desc;
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
  memcpy(&out, data_, n);
  data_ += n;
  size_ -= n;
}

void ExtractBindingRemapperInputs(Reader* r, tint::transform::DataMap* inputs) {
  struct Config {
    uint8_t old_group;
    uint8_t old_binding;
    uint8_t new_group;
    uint8_t new_binding;
    ast::AccessControl::Access new_ac;
  };

  std::vector<Config> configs = r->vector<Config>();
  transform::BindingRemapper::BindingPoints binding_points;
  transform::BindingRemapper::AccessControls access_controls;
  for (const auto& config : configs) {
    binding_points[{config.old_binding, config.old_group}] = {
        config.new_binding, config.new_group};
    access_controls[{config.old_binding, config.old_group}] = config.new_ac;
  }

  inputs->Add<transform::BindingRemapper::Remappings>(binding_points,
                                                      access_controls);
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

CommonFuzzer::CommonFuzzer(InputFormat input, OutputFormat output)
    : input_(input),
      output_(output),
      transform_manager_(nullptr),
      inspector_enabled_(false) {}

CommonFuzzer::~CommonFuzzer() = default;

int CommonFuzzer::Run(const uint8_t* data, size_t size) {
  tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

  Program program;

#if TINT_BUILD_WGSL_READER
  std::unique_ptr<Source::File> file;
#endif  // TINT_BUILD_WGSL_READER

  switch (input_) {
#if TINT_BUILD_WGSL_READER
    case InputFormat::kWGSL: {
      std::string str(reinterpret_cast<const char*>(data), size);
      file = std::make_unique<Source::File>("test.wgsl", str);
      program = reader::wgsl::Parse(file.get());
      break;
    }
#endif  // TINT_BUILD_WGSL_READER
#if TINT_BUILD_SPV_READER
    case InputFormat::kSpv: {
      size_t sizeInU32 = size / sizeof(uint32_t);
      const uint32_t* u32Data = reinterpret_cast<const uint32_t*>(data);
      std::vector<uint32_t> input(u32Data, u32Data + sizeInU32);

      if (input.size() != 0) {
        program = reader::spirv::Parse(input);
      }
      break;
    }
#endif  // TINT_BUILD_WGSL_READER
    default:
      return 0;
  }

  if (output_ == OutputFormat::kNone) {
    return 0;
  }

  if (!program.IsValid()) {
    return 0;
  }

  if (inspector_enabled_) {
    inspector::Inspector inspector(&program);

    auto entry_points = inspector.GetEntryPoints();
    if (inspector.has_error()) {
      return 0;
    }

    for (auto& ep : entry_points) {
      auto remapped_name = inspector.GetRemappedNameForEntryPoint(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto constant_ids = inspector.GetConstantIDs();
      if (inspector.has_error()) {
        return 0;
      }

      auto uniform_bindings =
          inspector.GetUniformBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto storage_bindings =
          inspector.GetStorageBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto readonly_bindings =
          inspector.GetReadOnlyStorageBufferResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto sampler_bindings = inspector.GetSamplerResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto comparison_sampler_bindings =
          inspector.GetComparisonSamplerResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto sampled_texture_bindings =
          inspector.GetSampledTextureResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }

      auto multisampled_texture_bindings =
          inspector.GetMultisampledTextureResourceBindings(ep.name);
      if (inspector.has_error()) {
        return 0;
      }
    }
  }

  if (transform_manager_) {
    auto out = transform_manager_->Run(&program, transform_inputs_);
    if (!out.program.IsValid()) {
      ValidityErrorReporter();
    }

    program = std::move(out.program);
  }

  std::unique_ptr<writer::Writer> writer;

  switch (output_) {
    case OutputFormat::kWGSL:
#if TINT_BUILD_WGSL_WRITER
      writer = std::make_unique<writer::wgsl::Generator>(&program);
#endif  // TINT_BUILD_WGSL_WRITER
      break;
    case OutputFormat::kSpv:
#if TINT_BUILD_SPV_WRITER
      writer = std::make_unique<writer::spirv::Generator>(&program);
#endif  // TINT_BUILD_SPV_WRITER
      break;
    case OutputFormat::kHLSL:
#if TINT_BUILD_HLSL_WRITER
      writer = std::make_unique<writer::hlsl::Generator>(&program);
#endif  // TINT_BUILD_HLSL_WRITER
      break;
    case OutputFormat::kMSL:
#if TINT_BUILD_MSL_WRITER
      writer = std::make_unique<writer::msl::Generator>(&program);
#endif  // TINT_BUILD_MSL_WRITER
      break;
    case OutputFormat::kNone:
      break;
  }

  if (writer) {
    writer->Generate();
  }

  return 0;
}

}  // namespace fuzzers
}  // namespace tint
