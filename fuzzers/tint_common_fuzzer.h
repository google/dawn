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

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "include/tint/tint.h"

#include "fuzzers/data_builder.h"

namespace tint {
namespace fuzzers {

// TODO(crbug.com/tint/1356): Add using shader reflection to generate options
//                            that are potentially valid for Generate*Options
//                            functions.
/// Generates random set of options for SPIRV generation
void GenerateSpirvOptions(DataBuilder* b, writer::spirv::Options* options);

/// Generates random set of options for WGSL generation
void GenerateWgslOptions(DataBuilder* b, writer::wgsl::Options* options);

/// Generates random set of options for HLSL generation
void GenerateHlslOptions(DataBuilder* b, writer::hlsl::Options* options);

/// Generates random set of options for MSL generation
void GenerateMslOptions(DataBuilder* b, writer::msl::Options* options);

/// Shader language the fuzzer is reading
enum class InputFormat { kWGSL, kSpv };

/// Shader language the fuzzer is emitting
enum class OutputFormat { kWGSL, kSpv, kHLSL, kMSL };

/// Generic runner for reading and emitting shaders using Tint, used by most
/// fuzzers to share common code.
class CommonFuzzer {
 public:
  /// Constructor
  /// @param input shader language being read
  /// @param output shader language being emitted
  CommonFuzzer(InputFormat input, OutputFormat output);

  /// Destructor
  ~CommonFuzzer();

  /// Setter for the transform manager and the data map to be used
  void SetTransformManager(transform::Manager* tm, transform::DataMap* inputs) {
    assert((!tm || inputs) && "DataMap must be !nullptr if Manager !nullptr");
    transform_manager_ = tm;
    transform_inputs_ = inputs;
  }

  /// Controls if the input shader for run should be outputted to the log
  void SetDumpInput(bool enabled) { dump_input_ = enabled; }

  /// Controls if the shader being valid after parsing is being enforced.
  /// If false, invalidation of the shader will cause an early exit, but not
  /// throw an error.
  /// If true invalidation will throw an error that is caught by libFuzzer and
  /// will generate a crash report.
  void SetEnforceValidity(bool enabled) { enforce_validity = enabled; }

  /// Convert given shader from input to output format.
  /// Will also apply provided transforms and run the inspector over the result.
  /// @param data buffer of data that will interpreted as a byte array or string
  ///             depending on the shader input format.
  /// @param size number of elements in buffer
  /// @returns 0, this is what libFuzzer expects
  int Run(const uint8_t* data, size_t size);

  /// Diagnostic messages generated while Run() is executed.
  const tint::diag::List& Diagnostics() const { return diagnostics_; }

  /// Are there any errors in the diagnostic messages?
  bool HasErrors() const { return diagnostics_.contains_errors(); }

  /// Generated SPIR-V binary, if SPIR-V was emitted.
  const std::vector<uint32_t>& GetGeneratedSpirv() const {
    return generated_spirv_;
  }

  /// Generated WGSL string, if WGSL was emitted.
  const std::string& GetGeneratedWgsl() const { return generated_wgsl_; }

  /// Generated HLSL string, if HLSL was emitted.
  const std::string& GetGeneratedHlsl() const { return generated_hlsl_; }

  /// Generated MSL string, if HLSL was emitted.
  const std::string& GetGeneratedMsl() const { return generated_msl_; }

  /// Setter for SPIR-V emission options
  void SetOptionsSpirv(const writer::spirv::Options& options) {
    options_spirv_ = options;
  }

  /// Setter for WGSL emission options
  void SetOptionsWgsl(const writer::wgsl::Options& options) {
    options_wgsl_ = options;
  }

  /// Setter for HLSL emission options
  void SetOptionsHlsl(const writer::hlsl::Options& options) {
    options_hlsl_ = options;
  }

  /// Setter for MSL emission options
  void SetOptionsMsl(const writer::msl::Options& options) {
    options_msl_ = options;
  }

 private:
  InputFormat input_;
  OutputFormat output_;
  transform::Manager* transform_manager_ = nullptr;
  transform::DataMap* transform_inputs_ = nullptr;
  bool dump_input_ = false;
  tint::diag::List diagnostics_;
  bool enforce_validity = false;

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

  /// Runs a series of reflection operations to exercise the Inspector API.
  void RunInspector(Program* program);
};

}  // namespace fuzzers
}  // namespace tint

#endif  // FUZZERS_TINT_COMMON_FUZZER_H_
