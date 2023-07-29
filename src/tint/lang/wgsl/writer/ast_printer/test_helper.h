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

#ifndef SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_TEST_HELPER_H_
#define SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_TEST_HELPER_H_

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/writer/ast_printer/ast_printer.h"

namespace tint::wgsl::writer {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
  public:
    TestHelperBase() = default;

    ~TestHelperBase() override = default;

    /// Builds and returns a ASTPrinter from the program.
    /// @param resolve if true, the program will be resolved before returning
    /// @note The generator is only built once. Multiple calls to Build() will
    /// return the same ASTPrinter without rebuilding.
    /// @return the built generator
    ASTPrinter& Build(bool resolve = true) {
        if (gen_) {
            return *gen_;
        }
        if (resolve) {
            program = std::make_unique<Program>(resolver::Resolve(*this));
        } else {
            program = std::make_unique<Program>(std::move(*this));
        }
        [&] { ASSERT_TRUE(program->IsValid()) << program->Diagnostics().str(); }();
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

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_TEST_HELPER_H_
