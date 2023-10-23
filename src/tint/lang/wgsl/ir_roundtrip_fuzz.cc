// Copyright 2023 The Dawn & Tint Authors
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

// GEN_BUILD:CONDITION(tint_build_wgsl_reader && tint_build_wgsl_writer)

#include <iostream>

#include "src/tint/cmd/fuzz/wgsl/wgsl_fuzz.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/writer/writer.h"

namespace tint::wgsl {
namespace {

bool IsUnsupported(const tint::ast::Enable* enable) {
    for (auto ext : enable->extensions) {
        switch (ext->name) {
            case tint::wgsl::Extension::kChromiumExperimentalDp4A:
            case tint::wgsl::Extension::kChromiumExperimentalFullPtrParameters:
            case tint::wgsl::Extension::kChromiumExperimentalPixelLocal:
            case tint::wgsl::Extension::kChromiumExperimentalPushConstant:
            case tint::wgsl::Extension::kChromiumInternalDualSourceBlending:
            case tint::wgsl::Extension::kChromiumInternalRelaxedUniformLayout:
                return true;
            default:
                break;
        }
    }
    return false;
}

}  // namespace

void IRRoundtripFuzzer(const tint::Program& program) {
    if (program.AST().Enables().Any(IsUnsupported)) {
        return;
    }

    auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
    auto& src = transformed ? transformed.value() : program;
    if (!src.IsValid()) {
        return;
    }

    auto ir = tint::wgsl::reader::ProgramToIR(src);
    if (!ir) {
        TINT_ICE() << ir.Failure();
        return;
    }

    if (auto res = tint::wgsl::reader::Lower(ir.Get()); !res) {
        TINT_ICE() << res.Failure();
        return;
    }

    if (auto res = tint::wgsl::writer::Raise(ir.Get()); !res) {
        TINT_ICE() << res.Failure();
        return;
    }

    auto dst = tint::wgsl::writer::IRToProgram(ir.Get());
    if (!dst.IsValid()) {
        std::cerr << "IR:\n" << core::ir::Disassemble(ir.Get()) << std::endl;
        if (auto result = tint::wgsl::writer::Generate(dst, {}); result) {
            std::cerr << "WGSL:\n" << result->wgsl << std::endl << std::endl;
        }
        TINT_ICE() << dst.Diagnostics();
        return;
    }

    return;
}

}  // namespace tint::wgsl

TINT_WGSL_PROGRAM_FUZZER(tint::wgsl::IRRoundtripFuzzer);
