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

#ifndef SRC_WRITER_MSL_TEST_HELPER_H_
#define SRC_WRITER_MSL_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/program_builder.h"
#include "src/transform/msl.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {

/// Enables verification of MSL shaders by running the Metal compiler and
/// checking no errors are reported.
/// @param xcrun_path the path to the `xcrun` executable
void EnableMSLValidation(const char* xcrun_path);

/// The return structure of Compile()
struct CompileResult {
  /// Status is an enumerator of status codes from Compile()
  enum class Status { kSuccess, kFailed, kVerificationNotEnabled };
  /// The resulting status of the compilation
  Status status;
  /// Output of the Metal compiler
  std::string output;
  /// The MSL source that was compiled
  std::string msl;
};

/// Compile attempts to compile the shader with xcrun if found on PATH.
/// @param program the MSL program
/// @return the result of the compile
CompileResult Compile(Program* program);

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
 public:
  TestHelperBase() = default;
  ~TestHelperBase() override = default;

  /// Builds and returns a GeneratorImpl from the program.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& Build() {
    if (gen_) {
      return *gen_;
    }
    [&]() {
      ASSERT_TRUE(IsValid()) << "Builder program is not valid\n"
                             << diag::Formatter().format(Diagnostics());
    }();
    program = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program->IsValid())
          << diag::Formatter().format(program->Diagnostics());
    }();
    gen_ = std::make_unique<GeneratorImpl>(program.get());
    return *gen_;
  }

  /// Builds the program, runs the program through the transform::Msl sanitizer
  /// and returns a GeneratorImpl from the sanitized program.
  /// @note The generator is only built once. Multiple calls to Build() will
  /// return the same GeneratorImpl without rebuilding.
  /// @return the built generator
  GeneratorImpl& SanitizeAndBuild() {
    if (gen_) {
      return *gen_;
    }
    [&]() {
      ASSERT_TRUE(IsValid()) << "Builder program is not valid\n"
                             << diag::Formatter().format(Diagnostics());
    }();
    program = std::make_unique<Program>(std::move(*this));
    [&]() {
      ASSERT_TRUE(program->IsValid())
          << diag::Formatter().format(program->Diagnostics());
    }();

    auto result = transform::Msl().Run(program.get());
    [&]() {
      ASSERT_TRUE(result.program.IsValid())
          << diag::Formatter().format(result.program.Diagnostics());
    }();
    *program = std::move(result.program);
    gen_ = std::make_unique<GeneratorImpl>(program.get());
    return *gen_;
  }

  /// Validate generates MSL code for the current contents of `program` and
  /// passes the output of the generator to the XCode SDK Metal compiler.
  ///
  /// If the Metal compiler finds problems, then any GTest test case that
  /// invokes this function test will fail.
  /// This function does nothing, if the Metal compiler path has not been
  /// configured by calling `EnableMSLValidation()`.
  void Validate() {
    auto res = Compile(program.get());
    if (res.status == CompileResult::Status::kFailed) {
      FAIL() << "MSL Validation failed.\n\n" << res.msl << "\n\n" << res.output;
    }
  }

  /// The program built with a call to Build()
  std::unique_ptr<Program> program;

 private:
  std::unique_ptr<GeneratorImpl> gen_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_TEST_HELPER_H_
