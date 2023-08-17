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
