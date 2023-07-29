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

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_TEST_HELPER_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/msl/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::msl::writer {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
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

    /// Builds and returns a ASTPrinter from the program.
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same ASTPrinter without rebuilding.
    /// @return the built generator
    ASTPrinter& Build() {
        if (gen_) {
            return *gen_;
        }
        [&] {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n" << Diagnostics().str();
        }();
        program = std::make_unique<Program>(resolver::Resolve(*this));
        [&] { ASSERT_TRUE(program->IsValid()) << program->Diagnostics().str(); }();
        gen_ = std::make_unique<ASTPrinter>(program.get());
        return *gen_;
    }

    /// Builds the program, runs the program through the transform::Msl sanitizer
    /// and returns a ASTPrinter from the sanitized program.
    /// @param options The MSL generator options.
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same ASTPrinter without rebuilding.
    /// @return the built generator
    ASTPrinter& SanitizeAndBuild(const Options& options = DefaultOptions()) {
        if (gen_) {
            return *gen_;
        }
        [&] {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n" << Diagnostics().str();
        }();
        program = std::make_unique<Program>(resolver::Resolve(*this));
        [&] { ASSERT_TRUE(program->IsValid()) << program->Diagnostics().str(); }();

        auto result = Sanitize(program.get(), options);
        [&] { ASSERT_TRUE(result.program.IsValid()) << result.program.Diagnostics().str(); }();
        *program = std::move(result.program);
        gen_ = std::make_unique<ASTPrinter>(program.get());
        return *gen_;
    }

    /// The program built with a call to Build()
    std::unique_ptr<Program> program;

  private:
    std::unique_ptr<ASTPrinter> gen_;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_PRINTER_TEST_HELPER_H_
