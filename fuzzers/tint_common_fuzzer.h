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

#include "include/tint/tint.h"

namespace tint {
namespace fuzzers {

class Reader {
 public:
  Reader(const uint8_t* data, size_t size);

  bool failed() const { return failed_; }
  const uint8_t* data() { return data_; }
  size_t size() const { return size_; }

  template <typename T>
  T read() {
    T out{};
    read(&out, sizeof(T));
    return out;
  }

  std::string string();

  template <typename T>
  std::vector<T> vector() {
    auto count = read<uint8_t>();
    auto size = static_cast<size_t>(count) * sizeof(T);
    if (failed_ || size_ < size) {
      mark_failed();
      return {};
    }
    std::vector<T> out(count);
    if (!out.empty()) {
      memcpy(out.data(), data_, size);
      data_ += size;
      size_ -= size;
    }
    return out;
  }

  template <typename T>
  std::vector<T> vector(T (*extract)(Reader*)) {
    auto count = read<uint8_t>();
    if (failed_) {
      return {};
    }
    std::vector<T> out(count);
    for (uint8_t i = 0; i < count; i++) {
      out[i] = extract(this);
      if (failed_) {
        return {};
      }
    }
    return out;
  }
  template <typename T>
  T enum_class(uint8_t count) {
    auto val = read<uint8_t>();
    return static_cast<T>(val % count);
  }

 private:
  void mark_failed();
  void read(void* out, size_t n);

  const uint8_t* data_;
  size_t size_;
  bool failed_ = false;
};

void ExtractBindingRemapperInputs(Reader* r, tint::transform::DataMap* inputs);

void ExtractFirstIndexOffsetInputs(Reader* r, tint::transform::DataMap* inputs);

void ExtractSingleEntryPointInputs(Reader* r, tint::transform::DataMap* inputs);

void ExtractVertexPullingInputs(Reader* r, tint::transform::DataMap* inputs);

void ExtractSpirvOptions(Reader* r, writer::spirv::Options* options);

void ExtractWgslOptions(Reader* r, writer::wgsl::Options* options);

void ExtractHlslOptions(Reader* r, writer::hlsl::Options* options);

void ExtractMslOptions(Reader* r, writer::msl::Options* options);

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
