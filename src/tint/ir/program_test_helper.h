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
#include "src/tint/ir/disassembler.h"
#include "src/tint/ir/from_program.h"
#include "src/tint/ir/validator.h"
#include "src/tint/lang/core/builtin/number.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/parser.h"
#include "src/tint/utils/text/string_stream.h"

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

        Program program{std::move(*this)};
        if (!program.IsValid()) {
            return program.Diagnostics().str();
        }

        auto result = FromProgram(&program);
        if (result) {
            auto validated = ir::Validate(result.Get());
            if (!validated) {
                return validated.Failure().str();
            }
        }
        return result;
    }

    /// Build the module from the given WGSL.
    /// @param wgsl the WGSL to convert to IR
    /// @returns the generated module
    utils::Result<Module, std::string> Build(std::string wgsl) {
#if TINT_BUILD_WGSL_READER
        Source::File file("test.wgsl", std::move(wgsl));
        auto program = reader::wgsl::Parse(&file);
        if (!program.IsValid()) {
            return program.Diagnostics().str();
        }

        auto result = FromProgram(&program);
        if (result) {
            auto validated = ir::Validate(result.Get());
            if (!validated) {
                return validated.Failure().str();
            }
        }
        return result;
#else
        return std::string("error: Tint not built with the WGSL reader");
#endif
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
