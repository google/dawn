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

namespace tint {
namespace fuzzers {

CommonFuzzer::CommonFuzzer(InputFormat input, OutputFormat output)
    : input_(input), output_(output) {}
CommonFuzzer::~CommonFuzzer() = default;

int CommonFuzzer::Run(const uint8_t* data, size_t size) {
  std::unique_ptr<tint::reader::Reader> parser;
#if TINT_BUILD_WGSL_READER
  std::unique_ptr<tint::Source::File> file;
#endif  // TINT_BUILD_WGSL_READER

  switch (input_) {
    case InputFormat::kWGSL:
#if TINT_BUILD_WGSL_READER
    {
      std::string str(reinterpret_cast<const char*>(data), size);

      file = std::make_unique<tint::Source::File>("test.wgsl", str);
      parser = std::make_unique<tint::reader::wgsl::Parser>(file.get());
    }
#endif  // TINT_BUILD_WGSL_READER
    break;
    case InputFormat::kSpv:
#if TINT_BUILD_SPV_READER
    {
      size_t sizeInU32 = size / sizeof(uint32_t);
      const uint32_t* u32Data = reinterpret_cast<const uint32_t*>(data);
      std::vector<uint32_t> input(u32Data, u32Data + sizeInU32);

      if (input.size() != 0) {
        parser = std::make_unique<tint::reader::spirv::Parser>(input);
      }
    }
#endif  // TINT_BUILD_WGSL_READER
    break;
    default:
      break;
  }

  if (!parser) {
    return 0;
  }

  if (!parser->Parse()) {
    return 0;
  }

  if (output_ == OutputFormat::kNone) {
    return 0;
  }

  auto mod = parser->module();
  if (!mod.IsValid()) {
    return 0;
  }

  tint::TypeDeterminer td(&mod);
  if (!td.Determine()) {
    return 0;
  }

  tint::Validator v;
  if (!v.Validate(&mod)) {
    return 0;
  }

  if (transform_manager_) {
    auto out = transform_manager_->Run(&mod);
    if (out.diagnostics.contains_errors()) {
      return 0;
    }

    mod = std::move(out.module);
  }

  std::unique_ptr<tint::writer::Writer> writer;

  switch (output_) {
    case OutputFormat::kWGSL:
#if TINT_BUILD_WGSL_WRITER
      writer = std::make_unique<tint::writer::wgsl::Generator>(std::move(mod));
#endif  // TINT_BUILD_WGSL_WRITER
      break;
    case OutputFormat::kSpv:
#if TINT_BUILD_SPV_WRITER
      writer = std::make_unique<tint::writer::spirv::Generator>(std::move(mod));
#endif  // TINT_BUILD_SPV_WRITER
      break;
    case OutputFormat::kHLSL:
#if TINT_BUILD_HLSL_WRITER
      writer = std::make_unique<tint::writer::hlsl::Generator>(std::move(mod));
#endif  // TINT_BUILD_HLSL_WRITER
      break;
    case OutputFormat::kMSL:
#if TINT_BUILD_MSL_WRITER
      writer = std::make_unique<tint::writer::msl::Generator>(std::move(mod));
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
