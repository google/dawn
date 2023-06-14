// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_IR_PROGRAM_TEST_HELPER_H_
#define SRC_TINT_IR_PROGRAM_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/builtin/number.h"
#include "src/tint/ir/disassembler.h"
#include "src/tint/ir/from_program.h"
#include "src/tint/program_builder.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// Helper class for testing
template <typename BASE>
class ProgramTestHelperBase : public BASE, public ProgramBuilder {
  public:
    ProgramTestHelperBase() = default;
    ~ProgramTestHelperBase() override = default;

    /// Build the module, cleaning up the program before returning.
    /// @returns the generated module
    utils::Result<Module, std::string> Build() {
        SetResolveOnBuild(true);

        auto program = std::make_unique<Program>(std::move(*this));
        [&]() {
            diag::Formatter formatter;
            ASSERT_TRUE(program->IsValid()) << formatter.format(program->Diagnostics());
        }();

        return FromProgram(program.get());
    }

    /// @param mod the module
    /// @returns the disassembly string of the module
    std::string Disassemble(Module& mod) {
        Disassembler d(mod);
        return d.Disassemble();
    }
};

using ProgramTestHelper = ProgramTestHelperBase<testing::Test>;

template <typename T>
using ProgramTestParamHelper = ProgramTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::ir

#endif  // SRC_TINT_IR_PROGRAM_TEST_HELPER_H_
