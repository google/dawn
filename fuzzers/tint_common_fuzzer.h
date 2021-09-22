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

#ifndef FUZZERS_TINT_COMMON_FUZZER_H_
#define FUZZERS_TINT_COMMON_FUZZER_H_

#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fuzzers/random_generator.h"
#include "include/tint/tint.h"

namespace tint {
namespace fuzzers {

class DataBuilder {
 public:
  DataBuilder(const uint8_t* data, size_t size);

  template <typename T>
  T build() {
    T out{};
    build(&out, sizeof(T));
    return out;
  }

  std::string string();

  template <typename T>
  std::vector<T> vector() {
    auto count = build<uint8_t>();
    std::vector<T> out(count);
    for (uint8_t i = 0; i < count; i++) {
      out[i] = build<T>();
    }
    return out;
  }

  template <typename T>
  std::vector<T> vector(T (*generate)(DataBuilder*)) {
    auto count = build<uint8_t>();
    std::vector<T> out(count);
    for (uint8_t i = 0; i < count; i++) {
      out[i] = generate(this);
    }
    return out;
  }

  template <typename T>
  T enum_class(uint8_t count) {
    auto val = build<uint8_t>();
    return static_cast<T>(val % count);
  }

 private:
  void build(void* out, size_t n);

  RandomGenerator generator_;
};

void GenerateBindingRemapperInputs(DataBuilder* b,
                                   tint::transform::DataMap* inputs);
void GenerateFirstIndexOffsetInputs(DataBuilder* b,
                                    tint::transform::DataMap* inputs);
void GenerateSingleEntryPointInputs(DataBuilder* b,
                                    tint::transform::DataMap* inputs);
void GenerateVertexPullingInputs(DataBuilder* b,
                                 tint::transform::DataMap* inputs);
void GenerateSpirvOptions(DataBuilder* b, writer::spirv::Options* options);
void GenerateWgslOptions(DataBuilder* b, writer::wgsl::Options* options);
void GenerateHlslOptions(DataBuilder* b, writer::hlsl::Options* options);
void GenerateMslOptions(DataBuilder* b, writer::msl::Options* options);

enum class InputFormat { kWGSL, kSpv, kNone };

enum class OutputFormat { kWGSL, kSpv, kHLSL, kMSL, kNone };

class CommonFuzzer {
 public:
  explicit CommonFuzzer(InputFormat input, OutputFormat output);
  ~CommonFuzzer();

  void SetTransformManager(transform::Manager* tm, transform::DataMap inputs) {
    transform_manager_ = tm;
    transform_inputs_ = std::move(inputs);
  }
  void EnableInspector() { inspector_enabled_ = true; }

  void SetDumpInput(bool enabled) { dump_input_ = enabled; }

  int Run(const uint8_t* data, size_t size);

  const tint::diag::List& Diagnostics() const { return diagnostics_; }

  bool HasErrors() const { return diagnostics_.contains_errors(); }

  const std::vector<uint32_t>& GetGeneratedSpirv() const {
    return generated_spirv_;
  }

  const std::string& GetGeneratedWgsl() const { return generated_wgsl_; }

  const std::string& GetGeneratedHlsl() const { return generated_hlsl_; }

  const std::string& GetGeneratedMsl() const { return generated_msl_; }

  void SetOptionsSpirv(const writer::spirv::Options& options) {
    options_spirv_ = options;
  }

  void SetOptionsWgsl(const writer::wgsl::Options& options) {
    options_wgsl_ = options;
  }

  void SetOptionsHlsl(const writer::hlsl::Options& options) {
    options_hlsl_ = options;
  }

  void SetOptionsMsl(const writer::msl::Options& options) {
    options_msl_ = options;
  }

 private:
  InputFormat input_;
  OutputFormat output_;
  transform::Manager* transform_manager_;
  transform::DataMap transform_inputs_;
  bool inspector_enabled_;
  bool dump_input_ = false;
  tint::diag::List diagnostics_;

  std::vector<uint32_t> generated_spirv_;
  std::string generated_wgsl_;
  std::string generated_hlsl_;
  std::string generated_msl_;

  writer::spirv::Options options_spirv_;
  writer::wgsl::Options options_wgsl_;
  writer::hlsl::Options options_hlsl_;
  writer::msl::Options options_msl_;

#if TINT_BUILD_WGSL_READER
  /// The source file needs to live at least as long as #diagnostics_
  std::unique_ptr<Source::File> file_;
#endif  // TINT_BUILD_WGSL_READER
};

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_COMMON_FUZZER_H_
