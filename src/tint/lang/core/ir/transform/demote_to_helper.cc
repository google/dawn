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

#include "src/tint/lang/core/ir/transform/demote_to_helper.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/ice/ice.h"

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// The global "has not discarded" flag.
    Var* continue_execution = nullptr;

    /// Map from function to a flag that indicates whether it (transitively) contains a discard.
    Hashmap<Function*, bool, 4> function_discard_status;

    /// Set of functions that have been processed.
    Hashset<Function*, 4> processed_functions;

    /// Constructor
    /// @param mod the module
    explicit State(Module* mod) : ir(mod) {}

    /// Process the module.
    void Process() {
        // Check each fragment shader entry point for discard instruction, potentially inside
        // functions called (transitively) by the entry point.
        Vector<Function*, 4> to_process;
        for (auto* func : ir->functions) {
            // If the function is a fragment shader that contains a discard, we need to process it.
            if (func->Stage() == Function::PipelineStage::kFragment) {
                if (HasDiscard(func)) {
                    to_process.Push(func);
                }
            }
        }
        if (to_process.IsEmpty()) {
            return;
        }

        // Create a boolean variable that can be used to check whether the shader has discarded.
        continue_execution = b.Var("continue_execution", ty.ptr<private_, bool>());
        continue_execution->SetInitializer(b.Constant(true));
        b.RootBlock()->Append(continue_execution);

        // Process each entry point function that contains a discard.
        for (auto* ep : to_process) {
            ProcessFunction(ep);
        }
    }

    /// Check if a function (transitively) contains a discard instruction.
    /// @param func the function to check
    /// @returns true if @p func contains a discard instruction
    bool HasDiscard(Function* func) {
        return function_discard_status.GetOrCreate(func, [&] { return HasDiscard(func->Block()); });
    }

    /// Check if a block (transitively) contains a discard instruction.
    /// @param block the block to check
    /// @returns true if @p block contains a discard instruction
    bool HasDiscard(Block* block) {
        // Loop over all instructions in the block.
        for (auto* inst : *block) {
            bool discard = false;
            tint::Switch(
                inst,
                [&](Discard*) {
                    // Found a discard.
                    discard = true;
                },
                [&](UserCall* call) {
                    // Check if we are calling a function that contains a discard.
                    discard = HasDiscard(call->Func());
                },
                [&](ControlInstruction* ctrl) {
                    // Recurse into control instructions and check their blocks.
                    ctrl->ForeachBlock([&](Block* blk) { discard = discard || HasDiscard(blk); });
                });
            if (discard) {
                return true;
            }
        }
        return false;
    }

    /// Process a function to replace its discard instruction and conditionalize its stores.
    /// @param func the function to process
    void ProcessFunction(Function* func) {
        if (processed_functions.Add(func)) {
            ProcessBlock(func->Block());
        }
    }

    /// Process a block to replace its discard instruction and conditionalize its stores.
    /// @param block the block to process
    void ProcessBlock(Block* block) {
        // Helper that wraps an instruction in an if statement so that it only executes if the
        // invocation has not discarded.
        auto conditionalize = [&](Instruction* inst) {
            // Create an if instruction in place of the original instruction.
            auto* cond = b.Load(continue_execution);
            auto* ifelse = b.If(cond);
            cond->InsertBefore(inst);
            inst->ReplaceWith(ifelse);

            // Move the original instruction into the if-true block.
            auto* result = ifelse->True()->Append(inst);

            TINT_ASSERT(!inst->HasMultiResults());
            if (inst->HasResults() && !inst->Result()->Type()->Is<type::Void>()) {
                // The original instruction had a result, so return it from the if instruction.
                ifelse->SetResults(Vector{b.InstructionResult(inst->Result()->Type())});
                inst->Result()->ReplaceAllUsesWith(ifelse->Result());
                ifelse->True()->Append(b.ExitIf(ifelse, result));
            } else {
                ifelse->True()->Append(b.ExitIf(ifelse));
            }
        };

        // Loop over all instructions in the block.
        for (auto* inst = *block->begin(); inst;) {
            // As we're (potentially) modifying the block that we're iterating over, grab a pointer
            // to the next instruction before we make any changes.
            auto* next = inst->next;
            TINT_DEFER(inst = next);

            tint::Switch(
                inst,
                [&](Discard* discard) {
                    // Replace every discard instruction with a store to the global flag.
                    discard->ReplaceWith(b.Store(continue_execution, false));
                    discard->Destroy();
                },
                [&](UserCall* call) {
                    // Recurse into user functions.
                    ProcessFunction(call->Func());
                },
                [&](Store* store) {
                    // Conditionalize stores to host-visible address spaces.
                    auto* ptr = store->To()->Type()->As<type::Pointer>();
                    if (ptr && ptr->AddressSpace() == builtin::AddressSpace::kStorage) {
                        conditionalize(store);
                    }
                },
                [&](CoreBuiltinCall* builtin) {
                    // Conditionalize calls to builtins that have side effects.
                    if (builtin::HasSideEffects(builtin->Func())) {
                        conditionalize(builtin);
                    }
                },
                [&](Return* ret) {
                    // Insert a conditional terminate invocation instruction before each return
                    // instruction in the entry point function.
                    if (ret->Func()->Stage() == Function::PipelineStage::kFragment) {
                        auto* cond = b.Load(continue_execution);
                        auto* ifelse = b.If(cond);
                        cond->InsertBefore(ret);
                        ifelse->InsertBefore(ret);
                        ifelse->True()->Append(b.TerminateInvocation());
                    }
                },
                [&](ControlInstruction* ctrl) {
                    // Recurse into control instructions.
                    ctrl->ForeachBlock([&](Block* blk) { ProcessBlock(blk); });
                });
        }
    }
};

}  // namespace

Result<SuccessType, std::string> DemoteToHelper(Module* ir) {
    auto result = ValidateAndDumpIfNeeded(*ir, "DemoteToHelper transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::ir::transform
