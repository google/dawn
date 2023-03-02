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

#ifndef SRC_TINT_WRITER_HLSL_TEST_HELPER_H_
#define SRC_TINT_WRITER_HLSL_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/renamer.h"
#include "src/tint/writer/hlsl/generator.h"
#include "src/tint/writer/hlsl/generator_impl.h"

namespace tint::writer::hlsl {

/// Helper class for testing
template <typename BODY>
class TestHelperBase : public BODY, public ProgramBuilder {
  public:
    TestHelperBase() = default;
    ~TestHelperBase() override = default;

    /// @returns the default generator options for SanitizeAndBuild(), if no explicit options are
    /// provided.
    static Options DefaultOptions() {
        Options opts;
        opts.disable_robustness = true;
        return opts;
    }

    /// Builds the program and returns a GeneratorImpl from the program.
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
            ASSERT_TRUE(program->IsValid()) << diag::Formatter().format(program->Diagnostics());
        }();
        gen_ = std::make_unique<GeneratorImpl>(program.get());
        return *gen_;
    }

    /// Builds the program, runs the program through the HLSL sanitizer
    /// and returns a GeneratorImpl from the sanitized program.
    /// @param options The HLSL generator options.
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same GeneratorImpl without rebuilding.
    /// @return the built generator
    GeneratorImpl& SanitizeAndBuild(const Options& options = DefaultOptions()) {
        if (gen_) {
            return *gen_;
        }
        diag::Formatter formatter;
        [&]() {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n"
                                   << formatter.format(Diagnostics());
        }();
        program = std::make_unique<Program>(std::move(*this));
        [&]() { ASSERT_TRUE(program->IsValid()) << formatter.format(program->Diagnostics()); }();

        auto sanitized_result = Sanitize(program.get(), options);
        [&]() {
            ASSERT_TRUE(sanitized_result.program.IsValid())
                << formatter.format(sanitized_result.program.Diagnostics());
        }();

        transform::Manager transform_manager;
        transform::DataMap transform_data;
        transform_data.Add<transform::Renamer::Config>(transform::Renamer::Target::kHlslKeywords,
                                                       /* preserve_unicode */ true);
        transform_manager.Add<tint::transform::Renamer>();
        auto result = transform_manager.Run(&sanitized_result.program, transform_data);
        [&]() {
            ASSERT_TRUE(result.program.IsValid()) << formatter.format(result.program.Diagnostics());
        }();
        *program = std::move(result.program);
        gen_ = std::make_unique<GeneratorImpl>(program.get());
        return *gen_;
    }

    /// The program built with a call to Build()
    std::unique_ptr<Program> program;

  private:
    std::unique_ptr<GeneratorImpl> gen_;
};

/// TestHelper the the base class for HLSL writer unit tests.
/// Use this form when you don't need to template any further.
using TestHelper = TestHelperBase<testing::Test>;

/// TestParamHelper the the base class for HLSL unit tests that take a templated
/// parameter.
template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::writer::hlsl

#endif  // SRC_TINT_WRITER_HLSL_TEST_HELPER_H_
