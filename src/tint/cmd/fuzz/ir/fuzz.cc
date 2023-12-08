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

#include "src/tint/cmd/fuzz/ir/fuzz.h"

#include "src/tint/utils/containers/vector.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/cmd/fuzz/wgsl/fuzz.h"
#include "src/tint/lang/wgsl/ast/enable.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#endif

#include "src/tint/lang/core/ir/validator.h"

#if TINT_BUILD_WGSL_READER
namespace tint::fuzz::ir {
namespace {

bool IsUnsupported(const ast::Enable* enable) {
    for (auto ext : enable->extensions) {
        switch (ext->name) {
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

void Register(const IRFuzzer& fuzzer) {
    wgsl::Register({
        fuzzer.name,
        [fn = fuzzer.fn](const Program& program, Slice<const std::byte> data) {
            if (program.AST().Enables().Any(IsUnsupported)) {
                return;
            }

            auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
            auto& src = transformed ? transformed.value() : program;
            if (!src.IsValid()) {
                return;
            }

            auto ir = tint::wgsl::reader::ProgramToLoweredIR(src);
            if (!ir) {
                return;
            }

            if (auto val = core::ir::Validate(ir.Get()); !val) {
                TINT_ICE() << val.Failure();
                return;
            }

            return fn(ir.Get(), data);
        },
    });
}

}  // namespace tint::fuzz::ir

#else

void tint::fuzz::ir::Register([[maybe_unused]] const IRFuzzer&) {}

#endif
