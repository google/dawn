// Copyright 2022 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader)

#ifndef SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_
#define SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::wgsl::helpers {

/// Helper class for testing IR with an input WGSL program.
template <typename BASE>
class IRProgramTestBase : public BASE, public ProgramBuilder {
  public:
    IRProgramTestBase() = default;
    ~IRProgramTestBase() override = default;

    /// Builds a core-dialect module from this ProgramBuilder.
    /// @returns the generated core-dialect module
    tint::Result<core::ir::Module> Build() {
        Program program{resolver::Resolve(*this)};
        if (!program.IsValid()) {
            return Failure{program.Diagnostics()};
        }

        auto result = wgsl::reader::ProgramToIR(program);
        if (!result) {
            return result.Failure();
        }

        // WGSL-dialect -> core-dialect
        if (auto lower = wgsl::reader::Lower(result.Get()); !lower) {
            return lower.Failure();
        }

        if (auto validate = core::ir::Validate(result.Get()); !validate) {
            return validate.Failure();
        }
        return result;
    }

    /// Build the module from the given WGSL.
    /// @param wgsl the WGSL to convert to IR
    /// @returns the generated module
    Result<core::ir::Module> Build(std::string wgsl) {
        Source::File file("test.wgsl", std::move(wgsl));
        auto result = wgsl::reader::WgslToIR(&file);
        if (result) {
            auto validated = core::ir::Validate(result.Get());
            if (!validated) {
                return validated.Failure();
            }
        }
        return result;
    }
};

using IRProgramTest = IRProgramTestBase<testing::Test>;

template <typename T>
using IRProgramTestParam = IRProgramTestBase<testing::TestWithParam<T>>;

}  // namespace tint::wgsl::helpers

#endif  // SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_
