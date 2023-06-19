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

#include "src/tint/ir/transform/merge_return.h"

#include <utility>

#include "src/tint/ir/builder.h"
#include "src/tint/ir/module.h"
#include "src/tint/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::MergeReturn);

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::ir::transform {

MergeReturn::MergeReturn() = default;

MergeReturn::~MergeReturn() = default;

/// PIMPL state for the transform, for a single function.
struct MergeReturn::State {
    /// The IR module.
    Module* ir = nullptr;
    /// The IR builder.
    Builder b{*ir};
    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// The "is returning" flag.
    Var* return_flag = nullptr;

    /// The return value.
    Var* return_val = nullptr;

    /// A set of merge blocks that have already been processed.
    utils::Hashset<Block*, 8> processed_merges;

    /// The final merge block that will contain the unique return instruction.
    Block* final_merge = nullptr;

    /// Track whether the return flag was actually used to conditionalize a merge block.
    bool uses_return_flag = false;

    /// Constructor
    /// @param mod the module
    explicit State(Module* mod) : ir(mod) {}

    /// Get the nearest non-merge parent block of `block`.
    /// @param block the block
    /// @returns the enclosing non-merge block
    Block* GetEnclosingNonMergeBlock(Block* block) {
        while (block->Is<MultiInBlock>()) {
            auto* parent = block->Parent();
            if (auto* loop = parent->As<Loop>()) {
                if (block != loop->Merge()) {
                    break;
                }
            }
            block = parent->Block();
        }
        return block;
    }

    /// Process the function.
    /// @param func the function to process
    void Process(Function* func) {
        // Find all of the return instructions in the function.
        utils::Vector<Return*, 4> returns;
        for (const auto& usage : func->Usages()) {
            if (auto* ret = usage.instruction->As<Return>()) {
                returns.Push(ret);
            }
        }

        // If there are no returns, or just a single return at the end of the function (potentially
        // inside a nested merge block), then nothing needs to be done.
        if (returns.Length() == 0 ||
            (returns.Length() == 1 &&
             GetEnclosingNonMergeBlock(returns[0]->Block()) == func->StartTarget())) {
            return;
        }

        // Create a boolean variable that can be used to check whether the function is returning.
        return_flag = b.Var(ty.ptr<function, bool>());
        return_flag->SetInitializer(b.Constant(false));
        func->StartTarget()->Prepend(return_flag);
        ir->SetName(return_flag, "return_flag");

        // Create a variable to hold the return value if needed.
        if (!func->ReturnType()->Is<type::Void>()) {
            return_val = b.Var(ty.ptr(function, func->ReturnType()));
            func->StartTarget()->Prepend(return_val);
            ir->SetName(return_val, "return_value");
        }

        // Process all of the return instructions in the function.
        for (auto* ret : returns) {
            ProcessReturn(ret);
        }

        // Add the unique return instruction to the final merge block if needed.
        if (final_merge) {
            if (return_val) {
                auto* retval = final_merge->Append(b.Load(return_val));
                final_merge->Append(b.Return(func, retval));
            } else {
                final_merge->Append(b.Return(func));
            }
        }

        // If the return flag was never actually read from, remove it and the corresponding stores.
        if (!uses_return_flag) {
            for (const auto& use : return_flag->Result()->Usages()) {
                use.instruction->Remove();
            }
            return_flag->Remove();
        }
    }

    /// Process a return instruction.
    /// @param ret the return instruction
    void ProcessReturn(Return* ret) {
        // If this return is at the end of the function, with no value, then we can leave it alone.
        if (GetEnclosingNonMergeBlock(ret->Block()) == ret->Func()->StartTarget() &&
            ret->Block()->Length() == 1 && !ret->Value()) {
            return;
        }

        // Set the "is returning" flag to `true`, and record the return value if present.
        b.Store(return_flag, b.Constant(true))->InsertBefore(ret);
        if (return_val) {
            b.Store(return_val, ret->Value())->InsertBefore(ret);
        }

        // Exit from the containing block, which will recursively insert conditionals into the
        // containing merge blocks as necessary, eventually inserting a unique return instruction.
        ExitFromBlock(ret->Block());
        ret->Remove();
        if (ret->Value()) {
            ret->Value()->RemoveUsage({ret, 0u});
        }
    }

    /// Process a merge block by wrapping its existing instructions (if any) in a conditional such
    /// that they will only execute if we are not returning.
    /// @param merge the merge block to process
    void ProcessMerge(MultiInBlock* merge) {
        if (processed_merges.Contains(merge)) {
            return;
        }
        processed_merges.Add(merge);

        // If the merge block was empty, we just need to exit from it.
        if (merge->IsEmpty()) {
            ExitFromBlock(merge);
            return;
        }

        if (merge->Length() == 1) {
            // If the block only contains an exit_{if,loop,switch}, we can skip adding a conditional
            // around its contents and just recurse into the parent merge block.
            if (utils::IsAnyOf<ExitIf, ExitLoop, ExitSwitch>(merge->Branch())) {
                tint::Switch(
                    merge->Branch(),  //
                    [&](If* ifelse) { ProcessMerge(ifelse->Merge()); },
                    [&](Loop* loop) { ProcessMerge(loop->Merge()); },
                    [&](Switch* swtch) { ProcessMerge(swtch->Merge()); });
                return;
            }

            // If the block only contains a return (with no value), we don't need to do anything.
            if (auto* ret = utils::As<Return>(merge->Branch())) {
                if (!ret->Value()) {
                    return;
                }
            }
        }

        // Wrap the existing contents of the merge block in a conditional so that it will only
        // execute if the "is returning" flag is `false`.
        uses_return_flag = true;
        auto* condition = b.Load(return_flag);
        auto* ifelse = b.If(condition);

        // Move all pre-existing instructions to the new false block.
        while (!merge->IsEmpty()) {
            auto* inst = merge->Front();
            inst->Remove();
            ifelse->False()->Append(inst);
        }

        // Now the merge block will just contain the new conditional.
        merge->Append(condition);
        merge->Append(ifelse);

        utils::Vector<Value*, 4> block_args_from_true;
        utils::Vector<BlockParam*, 4> merge_block_params;
        if (auto* exitif = ifelse->False()->Back()->As<ExitIf>()) {
            // If the previous terminator was an exit_if, we need replace it with one that exits to
            // the new merge block, and propagate the original basic block arguments if any.
            // The exit_if from the `true` block will just pass undef values to the merge block.
            utils::Vector<Value*, 4> block_args_from_false;
            for (uint32_t i = 0; i < exitif->Args().Length(); i++) {
                block_args_from_true.Push(nullptr);
                block_args_from_false.Push(exitif->Args()[i]);
                merge_block_params.Push(b.BlockParam(exitif->If()->Merge()->Params()[i]->Type()));
            }
            exitif->ReplaceWith(b.ExitIf(ifelse, block_args_from_false));
        } else {
            // If this merge block was the final merge block of the function, it won't have a branch
            // yet. Add an `exit_if` to the new merge block, and record the new merge block as the
            // new final merge block.
            if (merge == final_merge) {
                ifelse->False()->Append(b.ExitIf(ifelse));
                final_merge = ifelse->Merge();
            }
        }

        // Exit from the `true` block to the new merge block.
        ifelse->True()->Append(b.ExitIf(ifelse, block_args_from_true));

        // Exit from the new merge block, which will recursively process the parent merge.
        ifelse->Merge()->SetParams(merge_block_params);
        ExitFromBlock(ifelse->Merge(), merge_block_params);

        // We never need to process the merge that we've just added, as it only exits.
        processed_merges.Add(ifelse->Merge());
    }

    /// Add an exit_{if,loop,switch} instruction to `block`, and process the target merge block.
    /// @param block the block to exit from
    /// @param args the optional basic block arguments
    void ExitFromBlock(Block* block, utils::VectorRef<Value*> args = utils::Empty) {
        // Helper to get the block arguments for an instruction that is exiting to `merge`.
        auto block_args = [&](auto* merge) -> utils::Vector<Value*, 4> {
            // If arguments were explicitly provided, use those.
            if (!args.IsEmpty()) {
                return args;
            }

            // Otherwise, we will pass a list of `undef` values.
            utils::Vector<Value*, 4> undef_args;
            undef_args.Resize(merge->Params().Length(), nullptr);
            return undef_args;
        };

        auto* parent_control_flow = GetEnclosingNonMergeBlock(block)->Parent();
        tint::Switch(
            parent_control_flow,
            [&](If* ifelse) {
                ProcessMerge(ifelse->Merge());
                block->Append(b.ExitIf(ifelse, block_args(ifelse->Merge())));
            },
            [&](Loop* loop) {
                ProcessMerge(loop->Merge());
                block->Append(b.ExitLoop(loop, block_args(loop->Merge())));
            },
            [&](Switch* swtch) {
                ProcessMerge(swtch->Merge());
                block->Append(b.ExitSwitch(swtch, block_args(swtch->Merge())));
            },
            [&](Default) {
                // This is the top-level merge block, so just record it so that we can add the
                // unique return instruction to it later.
                final_merge = block;
            });
    }
};

void MergeReturn::Run(Module* ir, const DataMap&, DataMap&) const {
    // Process each function.
    for (auto* func : ir->functions) {
        State state(ir);
        state.Process(func);
    }
}

}  // namespace tint::ir::transform
