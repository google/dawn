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

#include "src/tint/lang/core/ir/transform/value_to_let.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// Access is an enumerator of memory access operations
enum class Access : uint8_t { kLoad, kStore };
/// Accesses is a set of of Access
using Accesses = EnumSet<Access>;

/// @returns the accesses that may be performed by the instruction @p inst
Accesses AccessesFor(ir::Instruction* inst) {
    return tint::Switch<Accesses>(
        inst,                                                           //
        [&](const ir::Load*) { return Access::kLoad; },                 //
        [&](const ir::LoadVectorElement*) { return Access::kLoad; },    //
        [&](const ir::Store*) { return Access::kStore; },               //
        [&](const ir::StoreVectorElement*) { return Access::kStore; },  //
        [&](const ir::Call*) {
            return Accesses{Access::kLoad, Access::kStore};
        },
        [&](Default) { return Accesses{}; });
}

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        // Process each block.
        for (auto* block : ir.blocks.Objects()) {
            Process(block);
        }
    }

  private:
    void Process(ir::Block* block) {
        // A set of possibly-inlinable values returned by a instructions that has not yet been
        // marked-for or ruled-out-for inlining.
        Hashset<ir::InstructionResult*, 32> pending_resolution;
        // The accesses of the values in pending_resolution.
        Access pending_access = Access::kLoad;

        auto put_pending_in_lets = [&] {
            for (auto* pending : pending_resolution) {
                PutInLet(pending);
            }
            pending_resolution.Clear();
        };

        auto maybe_put_in_let = [&](auto* inst) {
            if (auto* result = inst->Result(0)) {
                auto& usages = result->Usages();
                switch (usages.Count()) {
                    case 0:  // No usage
                        break;
                    case 1: {  // Single usage
                        auto* usage = (*usages.begin()).instruction;
                        if (usage->Block() == inst->Block()) {
                            // Usage in same block. Assign to pending_resolution, as we don't
                            // know whether its safe to inline yet.
                            pending_resolution.Add(result);
                        } else {
                            // Usage from another block. Cannot inline.
                            inst = PutInLet(result);
                        }
                        break;
                    }
                    default:  // Value has multiple usages. Cannot inline.
                        inst = PutInLet(result);
                        break;
                }
            }
        };

        for (ir::Instruction* inst = block->Front(); inst; inst = inst->next) {
            // This transform assumes that all multi-result instructions have been replaced
            TINT_ASSERT(inst->Results().Length() < 2);

            // The memory accesses of this instruction
            auto accesses = AccessesFor(inst);

            for (auto* operand : inst->Operands()) {
                // If the operand is in pending_resolution, then we know it has a single use and
                // because it hasn't been removed with put_pending_in_lets(), we know its safe to
                // inline without breaking access ordering. By inlining the operand, we are pulling
                // the operand's instruction into the same statement as this instruction, so this
                // instruction adopts the access of the operand.
                if (auto* result = As<InstructionResult>(operand)) {
                    if (pending_resolution.Remove(result)) {
                        // Var and Let are always statements, and so can never be inlined. As such,
                        // they do not need to propagate the pending resolution through them.
                        if (!inst->IsAnyOf<Var, Let>()) {
                            accesses.Add(pending_access);
                        }
                    }
                }
            }

            if (accesses.Contains(Access::kStore)) {  // Note: Also handles load + store
                put_pending_in_lets();
                maybe_put_in_let(inst);
            } else if (accesses.Contains(Access::kLoad)) {
                if (pending_access != Access::kLoad) {
                    put_pending_in_lets();
                    pending_access = Access::kLoad;
                }
                maybe_put_in_let(inst);
            }
        }
    }

    /// PutInLet places the value into a new 'let' instruction, immediately after the value's
    /// instruction
    /// @param value the value to place into the 'let'
    /// @return the created 'let' instruction.
    ir::Let* PutInLet(ir::InstructionResult* value) {
        auto* inst = value->Instruction();
        auto* let = b.Let(value->Type());
        value->ReplaceAllUsesWith(let->Result(0));
        let->SetValue(value);
        let->InsertAfter(inst);
        if (auto name = b.ir.NameOf(value); name.IsValid()) {
            b.ir.SetName(let->Result(0), name);
            b.ir.ClearName(value);
        }
        return let;
    }
};

}  // namespace

Result<SuccessType> ValueToLet(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "ValueToLet transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
