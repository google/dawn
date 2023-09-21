// Copyright 2023 The Tint Authors.
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
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    core::type::Manager& ty{ir->Types()};

    /// The symbol table.
    SymbolTable& sym{ir->symbols};

    /// Map from integer type to its divide helper function.
    Hashmap<const type::Type*, Function*, 4> int_div_helpers{};

    /// Map from integer type to its modulo helper function.
    Hashmap<const type::Type*, Function*, 4> int_mod_helpers{};

    /// Process the module.
    void Process() {
        // Find the binary instructions that need to be polyfilled.
        Vector<ir::Binary*, 64> worklist;
        for (auto* inst : ir->instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* binary = inst->As<ir::Binary>()) {
                switch (binary->Kind()) {
                    case ir::Binary::Kind::kDivide:
                    case ir::Binary::Kind::kModulo:
                        if (config.int_div_mod &&
                            binary->Result()->Type()->is_integer_scalar_or_vector()) {
                            worklist.Push(binary);
                        }
                        break;
                    case ir::Binary::Kind::kShiftLeft:
                    case ir::Binary::Kind::kShiftRight:
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
            switch (binary->Kind()) {
                case Binary::Kind::kDivide:
                case Binary::Kind::kModulo:
                    replacement = IntDivMod(binary);
                    break;
                case Binary::Kind::kShiftLeft:
                case Binary::Kind::kShiftRight:
                    replacement = MaskShiftAmount(binary);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(replacement);

            if (replacement != binary->Result()) {
                // Replace the old binary instruction result with the new value.
                if (auto name = ir->NameOf(binary->Result())) {
                    ir->SetName(replacement, name);
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
        bool is_div = binary->Kind() == Binary::Kind::kDivide;
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

                if (binary->Kind() == Binary::Kind::kDivide) {
                    // Perform the divide with the modified RHS.
                    b.Return(func, b.Divide(result_ty, lhs, rhs_or_one)->Result());
                } else if (binary->Kind() == Binary::Kind::kModulo) {
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

Result<SuccessType, std::string> BinaryPolyfill(Module* ir, const BinaryPolyfillConfig& config) {
    auto result = ValidateAndDumpIfNeeded(*ir, "BinaryPolyfill transform");
    if (!result) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
