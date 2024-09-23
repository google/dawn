// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/raise/binary_polyfill.h"

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"

namespace tint::glsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Find the binary instructions that need replacing.
        Vector<core::ir::Binary*, 4> binary_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* binary = inst->As<core::ir::Binary>()) {
                switch (binary->Op()) {
                    case core::BinaryOp::kAnd:
                    case core::BinaryOp::kOr: {
                        if (binary->LHS()->Type()->IsBoolScalarOrVector()) {
                            binary_worklist.Push(binary);
                        }
                        break;
                    }
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the binary calls
        for (auto* binary : binary_worklist) {
            switch (binary->Op()) {
                case core::BinaryOp::kAnd:
                case core::BinaryOp::kOr:
                    BitwiseBoolean(binary);
                    break;
                default:
                    TINT_UNIMPLEMENTED();
            }
        }
    }

    void BitwiseBoolean(core::ir::Binary* binary) {
        b.InsertBefore(binary, [&] {
            auto* res_ty = ty.MatchWidth(ty.u32(), binary->Result(0)->Type());
            auto* lhs = b.Convert(res_ty, binary->LHS());
            auto* rhs = b.Convert(res_ty, binary->RHS());

            core::ir::Value* result = nullptr;
            switch (binary->Op()) {
                case core::BinaryOp::kAnd:
                    result = b.And(res_ty, lhs, rhs)->Result(0);
                    break;
                case core::BinaryOp::kOr:
                    result = b.Or(res_ty, lhs, rhs)->Result(0);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
            b.ConvertWithResult(binary->DetachResult(), result);
        });
        binary->Destroy();
    }
};

}  // namespace

Result<SuccessType> BinaryPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "glsl.BinaryPolyfill transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
