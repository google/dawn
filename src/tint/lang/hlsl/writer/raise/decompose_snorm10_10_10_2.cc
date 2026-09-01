// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/decompose_snorm10_10_10_2.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::hlsl::writer::raise {
namespace {

struct State {
    core::ir::Module& ir;
    const std::vector<uint32_t>& locations;
    core::ir::Builder b{ir};
    core::type::Manager& ty{ir.Types()};

    struct DecodeResult {
        core::ir::Value* value = nullptr;
        core::ir::Value* multiply = nullptr;
    };

    void Process() {
        std::unordered_set<uint32_t> emulated_locs(locations.begin(), locations.end());

        for (auto* func : ir.functions) {
            if (!func->IsVertex()) {
                continue;
            }

            for (auto* param : func->Params()) {
                auto* struct_ty = param->Type()->As<core::type::Struct>();
                // Vertex shader parameters are always structs after ShaderIO
                TINT_ASSERT(struct_ty);

                for (auto* member : struct_ty->Members()) {
                    auto member_loc = member->Attributes().location;
                    if (!member_loc.has_value() || !emulated_locs.contains(member_loc.value())) {
                        continue;
                    }
                    uint32_t member_index = member->Index();

                    param->ForEachUseUnsorted([&](core::ir::Usage u) {
                        auto* access = u.instruction->As<core::ir::Access>();
                        if (!access || access->Indices().size() != 1) {
                            return;
                        }
                        auto* const_idx = access->Indices()[0]->As<core::ir::Constant>();
                        if (!const_idx || const_idx->Value()->ValueAs<uint32_t>() != member_index) {
                            return;
                        }

                        // Decode the access locally close to its usage to minimize register
                        // liveness/pressure. Duplicate access decodes will be merged later by
                        // Common Subexpression Elimination (CSE) and instruction deduplication
                        // at the downstream native shader compilers
                        auto* access_val = access->Result();
                        b.InsertAfter(access, [&] {
                            auto decoded = Decode(access_val);

                            // Replace all uses of the access result with the decoded result, except
                            // the use in the decoding logic's first instruction. We manually
                            // iterate a copy of the usages rather than using
                            // Value::ReplaceAllUsesWith(replacer) because the latter expects every
                            // usage to be replaced with a different value, and would infinite loop
                            // if we returned the original value for the decoder's first
                            // instruction.
                            access_val->ReplaceAllUsesWith(decoded.value);
                            if (auto* mul_inst =
                                    decoded.multiply->AsInstruction<core::ir::Binary>()) {
                                mul_inst->SetOperand(0, access_val);
                            }
                        });
                    });
                }
            }
        }
    }

    DecodeResult Decode(core::ir::Value* input) {
        auto* float_ty = input->Type();
        auto* int_ty = ty.MatchWidth(ty.i32(), float_ty);
        auto* uint_ty = ty.MatchWidth(ty.u32(), float_ty);

        uint32_t width = 1;
        if (auto* vec = float_ty->As<core::type::Vector>()) {
            width = vec->Width();
        }

        // Sign-extend:
        // The shift left values (22 for 10-bit XYZ, 30 for 2-bit W) differ from vertex pulling's
        // (22, 12, 2, 0) because the input components have already been unpacked into separate
        // vector lanes by the hardware fetcher before reaching the shader. We only need to
        // sign-extend each lane's value (10 bits for XYZ, 2 bits for W) in-place.
        // 10-bit signed normalized format parameters (XYZ components):
        const auto kScale10 = 1023_f;
        const auto kShift10 = 22_u;
        const auto kDiv10 = 511_f;

        // 2-bit signed normalized format parameters (W component):
        const auto kScale2 = 3_f;
        const auto kShift2 = 30_u;
        const auto kDiv2 = 1_f;

        core::ir::Value* scale = nullptr;
        core::ir::Value* shift = nullptr;
        core::ir::Value* div = nullptr;

        if (width == 4) {
            scale = b.Composite<vec4f>(kScale10, kScale10, kScale10, kScale2);
            shift = b.Composite<vec4u>(kShift10, kShift10, kShift10, kShift2);
            div = b.Composite<vec4f>(kDiv10, kDiv10, kDiv10, kDiv2);
        } else {
            scale = b.MatchWidth(kScale10, float_ty);
            shift = b.MatchWidth(kShift10, uint_ty);
            div = b.MatchWidth(kDiv10, float_ty);
        }
        auto* min_val = b.MatchWidth(-1_f, float_ty);

        // Reconstruct integer values:
        auto* scaled = b.Multiply(input, scale);
        auto* rounded = b.Call(float_ty, core::BuiltinFn::kRound, scaled);

        // Sign-extend:
        auto* s32s = b.Convert(int_ty, rounded);
        auto* shl = b.ShiftLeft(s32s, shift);
        auto* shr = b.ShiftRight(shl, shift);

        // Normalize:
        auto* normalized = b.Divide(b.Convert(float_ty, shr), div);
        auto* decoded = b.Call(float_ty, core::BuiltinFn::kMax, normalized, min_val);

        return {decoded, scaled};
    }
};

}  // namespace

Result<SuccessType> DecomposeSnorm10_10_10_2(core::ir::Module& ir,
                                             const std::vector<uint32_t>& locations) {
    AssertValid(ir, "before hlsl.DecomposeSnorm10_10_10_2");

    if (!locations.empty()) {
        State{ir, locations}.Process();
    }

    return Success;
}

}  // namespace tint::hlsl::writer::raise
