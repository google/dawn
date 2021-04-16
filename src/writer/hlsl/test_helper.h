// Copyright 2020 The Tint Authors.
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

#ifndef SRC_WRITER_HLSL_TEST_HELPER_H_
#define SRC_WRITER_HLSL_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/transform/hlsl.h"
#include "src/transform/manager.h"
#include "src/transform/renamer.h"
#include "src/writer/hlsl/generator_impl.h"

namespace tint {
namespace writer {
namespace hlsl {

/// EnableHLSLValidation enables verification of HLSL shaders by running DXC and
/// checking no errors are reported.
/// @param dxc_path the path to the DXC executable
void EnableHLSLValidation(const char* dxc_path);

/// The return structure of Compile()
struct CompileResult {
  /// Status is an enumerator of status codes from Compile()
  enum class Status { kSuccess, kFailed, kVerificationNotEnabled };
  /// The resulting status of the compile
  Status status;
  /// Output of DXC.
  std::string output;
  /// The HLSL source that was compiled
  std::string hlsl;
};

/// Compile attempts to compile the shader with DXC if found on PATH.
/// @param program the HLSL program
/// @param generator the HLSL generator
/// @return the result of the compile
CompileResult Compile(Program* program, GeneratorImpl* generator);

/// Helper class for testing
template <typename BODY>
class TestHelperBase : public BODY, public ProgramBuilder {
 public:
  TestHelperBase() = default;
  ~TestHelperBase() override = default;

  /// Builds the program and returns a GeneratorImpl from the program.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& Build() {
    if (gen_) {
      return *gen_;
    }
    diag::Formatter formatter;
    [&]() {
      ASSERT_TRUE(IsValid()) << "Builder program is not valid\n"
                             << formatter.format(Diagnostics());
    }();
    program = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program->IsValid())
          << formatter.format(program->Diagnostics());
    }();
    gen_ = std::make_unique<GeneratorImpl>(program.get());
    return *gen_;
  }

  /// Builds the program, runs the program through the transform::Hlsl sanitizer
  /// and returns a GeneratorImpl from the sanitized program.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& SanitizeAndBuild() {
    if (gen_) {
      return *gen_;
    }
    diag::Formatter formatter;
    [&]() {
      ASSERT_TRUE(IsValid()) << "Builder program is not valid\n"
                             << formatter.format(Diagnostics());
    }();
    program = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program->IsValid())
          << formatter.format(program->Diagnostics());
    }();

    transform::Manager transform_manager;
    transform::Renamer::Config renamer_config{
        transform::Renamer::Target::kHlslKeywords};
    transform_manager.append(
        std::make_unique<tint::transform::Renamer>(renamer_config));
    transform_manager.append(std::make_unique<tint::transform::Hlsl>());
    auto result = transform_manager.Run(program.get());
    [&]() {
      ASSERT_TRUE(result.program.IsValid())
          << formatter.format(result.program.Diagnostics());
    }();
    *program = std::move(result.program);
    gen_ = std::make_unique<GeneratorImpl>(program.get());
    return *gen_;
  }

  /// Validate passes the generated HLSL from the generator to the DXC compiler
  /// on `PATH` for checking the program can be compiled.
  /// If DXC finds problems the test will fail.
  /// If DXC is not on `PATH` then Validate() does nothing.
  void Validate() const {
    auto res = Compile(program.get(), gen_.get());
    if (res.status == CompileResult::Status::kFailed) {
      FAIL() << "HLSL Validation failed.\n\n"
             << res.hlsl << "\n\n"
             << res.output;
    }
  }

  /// @returns the result string
  std::string result() const { return out.str(); }

  /// @returns the pre result string
  std::string pre_result() const { return pre.str(); }

  /// The output stream
  std::ostringstream out;
  /// The pre-output stream
  std::ostringstream pre;
  /// The program built with a call to Build()
  std::unique_ptr<Program> program;

 private:
  std::unique_ptr<GeneratorImpl> gen_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_TEST_HELPER_H_
