// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_FUZZERS_TINT_COMMON_FUZZER_H_
#define SRC_TINT_FUZZERS_TINT_COMMON_FUZZER_H_

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "include/tint/tint.h"

#include "src/tint/fuzzers/data_builder.h"

namespace tint::fuzzers {

// TODO(crbug.com/tint/1356): Add using shader reflection to generate options
//                            that are potentially valid for Generate*Options
//                            functions.
/// Generates random set of options for SPIRV generation
void GenerateSpirvOptions(DataBuilder* b, spirv::writer::Options* options);

/// Generates random set of options for WGSL generation
void GenerateWgslOptions(DataBuilder* b, wgsl::writer::Options* options);

/// Generates random set of options for HLSL generation
void GenerateHlslOptions(DataBuilder* b, hlsl::writer::Options* options);

/// Generates random set of options for MSL generation
void GenerateMslOptions(DataBuilder* b, msl::writer::Options* options);

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

    /// @param tm manager for transforms to run
    /// @param inputs data for transforms to run
    void SetTransformManager(ast::transform::Manager* tm, ast::transform::DataMap* inputs) {
        assert((!tm || inputs) && "DataMap must be !nullptr if Manager !nullptr");
        transform_manager_ = tm;
        transform_inputs_ = inputs;
    }

    /// @param enabled if the input shader for run should be outputted to the log
    void SetDumpInput(bool enabled) { dump_input_ = enabled; }

    /// @param enabled if the shader being valid after parsing is being enforced.
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

    /// @returns diagnostic messages generated while Run() is executed.
    const tint::diag::List& Diagnostics() const { return diagnostics_; }

    /// @returns if there are any errors in the diagnostic messages
    bool HasErrors() const { return diagnostics_.contains_errors(); }

    /// @returns generated SPIR-V binary, if SPIR-V was emitted.
    const std::vector<uint32_t>& GetGeneratedSpirv() const { return generated_spirv_; }

    /// @returns generated WGSL string, if WGSL was emitted.
    const std::string& GetGeneratedWgsl() const { return generated_wgsl_; }

    /// @returns generated HLSL string, if HLSL was emitted.
    const std::string& GetGeneratedHlsl() const { return generated_hlsl_; }

    /// @returns generated MSL string, if HLSL was emitted.
    const std::string& GetGeneratedMsl() const { return generated_msl_; }

    /// @param options SPIR-V emission options
    void SetOptionsSpirv(const spirv::writer::Options& options) { options_spirv_ = options; }

    /// @param options WGSL emission options
    void SetOptionsWgsl(const wgsl::writer::Options& options) { options_wgsl_ = options; }

    /// @param options HLSL emission options
    void SetOptionsHlsl(const hlsl::writer::Options& options) { options_hlsl_ = options; }

    /// @param options MSL emission options
    void SetOptionsMsl(const msl::writer::Options& options) { options_msl_ = options; }

  private:
    InputFormat input_;
    OutputFormat output_;
    ast::transform::Manager* transform_manager_ = nullptr;
    ast::transform::DataMap* transform_inputs_ = nullptr;
    bool dump_input_ = false;
    tint::diag::List diagnostics_;
    bool enforce_validity = false;

    std::vector<uint32_t> generated_spirv_;
    std::string generated_wgsl_;
    std::string generated_hlsl_;
    std::string generated_msl_;

    spirv::writer::Options options_spirv_;
    wgsl::writer::Options options_wgsl_;
    hlsl::writer::Options options_hlsl_;
    msl::writer::Options options_msl_;

#if TINT_BUILD_WGSL_READER
    /// The source file needs to live at least as long as #diagnostics_
    std::unique_ptr<Source::File> file_;
#endif  // TINT_BUILD_WGSL_READER

    /// Runs a series of reflection operations to exercise the Inspector API.
    void RunInspector(Program& program);
};

}  // namespace tint::fuzzers

#endif  // SRC_TINT_FUZZERS_TINT_COMMON_FUZZER_H_
