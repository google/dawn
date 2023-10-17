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

#include "src/tint/lang/core/ir/transform/binary_polyfill.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The polyfill config.
    const BinaryPolyfillConfig& config;

    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from integer type to its divide helper function.
    Hashmap<const type::Type*, Function*, 4> int_div_helpers{};

    /// Map from integer type to its modulo helper function.
    Hashmap<const type::Type*, Function*, 4> int_mod_helpers{};

    /// Process the module.
    void Process() {
        // Find the binary instructions that need to be polyfilled.
        Vector<ir::Binary*, 64> worklist;
        for (auto* inst : ir.instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* binary = inst->As<ir::Binary>()) {
                switch (binary->Op()) {
                    case BinaryOp::kDivide:
                    case BinaryOp::kModulo:
                        if (config.int_div_mod &&
                            binary->Result()->Type()->is_integer_scalar_or_vector()) {
                            worklist.Push(binary);
                        }
                        break;
                    case BinaryOp::kShiftLeft:
                    case BinaryOp::kShiftRight:
                        if (config.bitshift_modulo) {
                            worklist.Push(binary);
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        // Polyfill the binary instructions that we found.
        for (auto* binary : worklist) {
            ir::Value* replacement = nullptr;
            switch (binary->Op()) {
                case BinaryOp::kDivide:
                case BinaryOp::kModulo:
                    replacement = IntDivMod(binary);
                    break;
                case BinaryOp::kShiftLeft:
                case BinaryOp::kShiftRight:
                    replacement = MaskShiftAmount(binary);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(replacement);

            if (replacement != binary->Result()) {
                // Replace the old binary instruction result with the new value.
                if (auto name = ir.NameOf(binary->Result())) {
                    ir.SetName(replacement, name);
                }
                binary->Result()->ReplaceAllUsesWith(replacement);
                binary->Destroy();
            }
        }
    }

    /// Return a type with element type @p type that has the same number of vector components as
    /// @p match. If @p match is scalar just return @p type.
    /// @param el_ty the type to extend
    /// @param match the type to match the component count of
    /// @returns a type with the same number of vector components as @p match
    const core::type::Type* MatchWidth(const core::type::Type* el_ty,
                                       const core::type::Type* match) {
        if (auto* vec = match->As<core::type::Vector>()) {
            return ty.vec(el_ty, vec->Width());
        }
        return el_ty;
    }

    /// Return a constant that has the same number of vector components as @p match, each with the
    /// value @p element. If @p match is scalar just return @p element.
    /// @param element the value to extend
    /// @param match the type to match the component count of
    /// @returns a value with the same number of vector components as @p match
    ir::Constant* MatchWidth(ir::Constant* element, const core::type::Type* match) {
        if (auto* vec = match->As<core::type::Vector>()) {
            return b.Splat(MatchWidth(element->Type(), match), element, vec->Width());
        }
        return element;
    }

    /// Replace an integer divide or modulo with a call to helper function that prevents
    /// divide-by-zero and signed integer overflow.
    /// @param binary the binary instruction
    /// @returns the replacement value
    ir::Value* IntDivMod(ir::Binary* binary) {
        auto* result_ty = binary->Result()->Type();
        bool is_div = binary->Op() == BinaryOp::kDivide;
        bool is_signed = result_ty->is_signed_integer_scalar_or_vector();

        auto& helpers = is_div ? int_div_helpers : int_mod_helpers;
        auto* helper = helpers.GetOrCreate(result_ty, [&] {
            // Generate a name for the helper function.
            StringStream name;
            name << "tint_" << (is_div ? "div_" : "mod_");
            if (auto* vec = result_ty->As<type::Vector>()) {
                name << "v" << vec->Width() << vec->type()->FriendlyName();
            } else {
                name << result_ty->FriendlyName();
            }

            // Create the helper function.
            auto* func = b.Function(name.str(), result_ty);
            auto* lhs = b.FunctionParam("lhs", result_ty);
            auto* rhs = b.FunctionParam("rhs", result_ty);
            func->SetParams({lhs, rhs});
            b.Append(func->Block(), [&] {
                // Generate constants for zero and one with types that match the result type.
                ir::Constant* one = nullptr;
                ir::Constant* zero = nullptr;
                if (is_signed) {
                    one = MatchWidth(b.Constant(1_i), result_ty);
                    zero = MatchWidth(b.Constant(0_i), result_ty);
                } else {
                    one = MatchWidth(b.Constant(1_u), result_ty);
                    zero = MatchWidth(b.Constant(0_u), result_ty);
                }

                // Select either the RHS or a constant one value if the RHS is zero.
                // If this is a signed operation, we also check for `INT_MIN / -1`.
                auto* bool_ty = MatchWidth(ty.bool_(), result_ty);
                auto* cond = b.Equal(bool_ty, rhs, zero);
                if (is_signed) {
                    auto* lowest = MatchWidth(b.Constant(i32::Lowest()), result_ty);
                    auto* minus_one = MatchWidth(b.Constant(-1_i), result_ty);
                    auto* lhs_is_lowest = b.Equal(bool_ty, lhs, lowest);
                    auto* rhs_is_minus_one = b.Equal(bool_ty, rhs, minus_one);
                    cond = b.Or(bool_ty, cond, b.And(bool_ty, lhs_is_lowest, rhs_is_minus_one));
                }
                auto* rhs_or_one = b.Call(result_ty, core::BuiltinFn::kSelect, rhs, one, cond);

                if (binary->Op() == BinaryOp::kDivide) {
                    // Perform the divide with the modified RHS.
                    b.Return(func, b.Divide(result_ty, lhs, rhs_or_one)->Result());
                } else if (binary->Op() == BinaryOp::kModulo) {
                    // Calculate the modulo manually, as modulo with negative operands is undefined
                    // behavior for many backends:
                    //   result = lhs - ((lhs / rhs_or_one) * rhs_or_one)
                    auto* whole = b.Divide(result_ty, lhs, rhs_or_one);
                    auto* remainder =
                        b.Subtract(result_ty, lhs, b.Multiply(result_ty, whole, rhs_or_one));
                    b.Return(func, remainder->Result());
                }
            });
            return func;
        });

        /// Helper to splat a value to match the vector width of the result type if necessary.
        auto maybe_splat = [&](ir::Value* value) -> ir::Value* {
            if (value->Type()->Is<type::Scalar>() && result_ty->Is<core::type::Vector>()) {
                return b.Construct(result_ty, value)->Result();
            }
            return value;
        };

        // Call the helper function, splatting the arguments to match the target vector width.
        Value* result = nullptr;
        b.InsertBefore(binary, [&] {
            auto* lhs = maybe_splat(binary->LHS());
            auto* rhs = maybe_splat(binary->RHS());
            result = b.Call(result_ty, helper, lhs, rhs)->Result();
        });
        return result;
    }

    /// Mask the RHS of a shift instruction to ensure it is modulo the bitwidth of the LHS.
    /// @param binary the binary instruction
    /// @returns the replacement value
    ir::Value* MaskShiftAmount(ir::Binary* binary) {
        auto* lhs = binary->LHS();
        auto* rhs = binary->RHS();
        auto* mask = b.Constant(u32(lhs->Type()->DeepestElement()->Size() * 8 - 1));
        auto* masked = b.And(rhs->Type(), rhs, MatchWidth(mask, rhs->Type()));
        masked->InsertBefore(binary);
        binary->SetOperand(ir::Binary::kRhsOperandOffset, masked->Result());
        return binary->Result();
    }
};

}  // namespace

Result<SuccessType> BinaryPolyfill(Module& ir, const BinaryPolyfillConfig& config) {
    auto result = ValidateAndDumpIfNeeded(ir, "BinaryPolyfill transform");
    if (!result) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
