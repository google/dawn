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

CommonFuzzer::CommonFuzzer(InputFormat input, OutputFormat output)
    : input_(input),
      output_(output),
      transform_manager_(nullptr),
      inspector_enabled_(false) {}

CommonFuzzer::~CommonFuzzer() = default;

int CommonFuzzer::Run(const uint8_t* data, size_t size) {
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

  Validator v;
  if (!v.Validate(&program)) {
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
    auto out = transform_manager_->Run(&program);
    if (!out.program.IsValid()) {
      return 0;
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
