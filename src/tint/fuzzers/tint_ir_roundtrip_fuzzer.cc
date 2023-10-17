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

#include <iostream>
#include <string>
#include <unordered_set>

#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/parser/parser.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/writer/ir_to_program/ir_to_program.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"
#include "src/tint/lang/wgsl/writer/writer.h"

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::InternalCompilerError& err) {
    std::cerr << err.Error() << std::endl;
    __builtin_trap();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string str(reinterpret_cast<const char*>(data), size);

    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    tint::Source::File file("test.wgsl", str);

    // Parse the wgsl, create the src program
    tint::wgsl::reader::Parser parser(&file);
    parser.set_max_errors(1);
    if (!parser.Parse()) {
        return 0;
    }
    auto src = parser.program();
    if (!src.IsValid()) {
        return 0;
    }

    auto is_unsupported = [](const tint::ast::Enable* enable) {
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
    };

    if (src.AST().Enables().Any(is_unsupported)) {
        return 0;
    }

    if (auto transformed = tint::wgsl::ApplySubstituteOverrides(src)) {
        src = std::move(*transformed);
        if (!src.IsValid()) {
            return 0;
        }
    }

    auto ir = tint::wgsl::reader::ProgramToIR(src);
    if (!ir) {
        std::cerr << ir.Failure() << std::endl;
        __builtin_trap();
    }

    if (auto res = tint::wgsl::reader::Lower(ir.Get()); !res) {
        std::cerr << res.Failure() << std::endl;
        __builtin_trap();
    }

    if (auto res = tint::wgsl::writer::Raise(ir.Get()); !res) {
        std::cerr << res.Failure() << std::endl;
        __builtin_trap();
    }

    auto dst = tint::wgsl::writer::IRToProgram(ir.Get());
    if (!dst.IsValid()) {
#if TINT_BUILD_WGSL_WRITER
        if (auto result = tint::wgsl::writer::Generate(dst, {}); result) {
            std::cerr << result->wgsl << std::endl << std::endl;
        }
#endif

        std::cerr << dst.Diagnostics() << std::endl;
        __builtin_trap();
    }

    return 0;
}
