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

#include "src/tint/ir/validate.h"

#include <memory>
#include <string>
#include <utility>

#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/convert.h"
#include "src/tint/ir/disassembler.h"
#include "src/tint/ir/discard.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/swizzle.h"
#include "src/tint/ir/unary.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/pointer.h"
#include "src/tint/utils/scoped_assignment.h"

namespace tint::ir {
namespace {

class Validator {
  public:
    explicit Validator(Module& mod) : mod_(mod) {}

    ~Validator() {}

    utils::Result<Success, diag::List> IsValid() {
        CheckRootBlock(mod_.root_block);

        for (const auto* func : mod_.functions) {
            CheckFunction(func);
        }

        if (diagnostics_.contains_errors()) {
            // If a diassembly file was generated then one of the diagnostics referenced the
            // disasembly. Emit the entire disassembly file at the end of the messages.
            if (mod_.disassembly_file) {
                diagnostics_.add_note(tint::diag::System::IR,
                                      "# Disassembly\n" + mod_.disassembly_file->content.data, {});
            }
            return std::move(diagnostics_);
        }
        return Success{};
    }

  private:
    Module& mod_;
    diag::List diagnostics_;
    Disassembler dis_{mod_};

    const Block* current_block_ = nullptr;

    void DisassembleIfNeeded() {
        if (mod_.disassembly_file) {
            return;
        }
        mod_.disassembly_file = std::make_unique<Source::File>("", dis_.Disassemble());
    }

    void AddError(const Instruction* inst, const std::string& err) {
        DisassembleIfNeeded();
        auto src = dis_.InstructionSource(inst);
        src.file = mod_.disassembly_file.get();
        AddError(err, src);

        if (current_block_) {
            AddNote(current_block_, "In block");
        }
    }

    void AddError(const Instruction* inst, uint32_t idx, const std::string& err) {
        DisassembleIfNeeded();
        auto src = dis_.OperandSource(Disassembler::Operand{inst, idx});
        src.file = mod_.disassembly_file.get();
        AddError(err, src);

        if (current_block_) {
            AddNote(current_block_, "In block");
        }
    }

    void AddError(const Block* blk, const std::string& err) {
        DisassembleIfNeeded();
        auto src = dis_.BlockSource(blk);
        src.file = mod_.disassembly_file.get();
        AddError(err, src);
    }

    void AddNote(const Block* blk, const std::string& err) {
        DisassembleIfNeeded();
        auto src = dis_.BlockSource(blk);
        src.file = mod_.disassembly_file.get();
        AddNote(err, src);
    }

    void AddError(const std::string& err, Source src = {}) {
        diagnostics_.add_error(tint::diag::System::IR, err, src);
    }

    void AddNote(const std::string& note, Source src = {}) {
        diagnostics_.add_note(tint::diag::System::IR, note, src);
    }

    std::string Name(const Value* v) { return mod_.NameOf(v).Name(); }

    void CheckRootBlock(const Block* blk) {
        if (!blk) {
            return;
        }

        TINT_SCOPED_ASSIGNMENT(current_block_, blk);

        for (const auto* inst : *blk) {
            auto* var = inst->As<ir::Var>();
            if (!var) {
                AddError(inst,
                         std::string("root block: invalid instruction: ") + inst->TypeInfo().name);
                continue;
            }
            if (!var->Type()->Is<type::Pointer>()) {
                AddError(inst, std::string("root block: 'var' ") + Name(var) +
                                   "type is not a pointer: " + var->Type()->TypeInfo().name);
            }
        }
    }

    void CheckFunction(const Function* func) { CheckBlock(func->StartTarget()); }

    void CheckBlock(const Block* blk) {
        TINT_SCOPED_ASSIGNMENT(current_block_, blk);

        if (!blk->HasBranchTarget()) {
            AddError(blk, "block: does not end in a branch");
        }

        for (const auto* inst : *blk) {
            if (inst->Is<ir::Branch>() && inst != blk->Branch()) {
                AddError(inst, "block: branch which isn't the final instruction");
                continue;
            }

            CheckInstruction(inst);
        }
    }

    void CheckInstruction(const Instruction* inst) {
        tint::Switch(
            inst,                                          //
            [&](const ir::Access*) {},                     //
            [&](const ir::Binary*) {},                     //
            [&](const ir::Branch* b) { CheckBranch(b); },  //
            [&](const ir::Call* c) { CheckCall(c); },      //
            [&](const ir::Load*) {},                       //
            [&](const ir::Store*) {},                      //
            [&](const ir::Swizzle*) {},                    //
            [&](const ir::Unary*) {},                      //
            [&](const ir::Var*) {},                        //
            [&](Default) {
                AddError(std::string("missing validation of: ") + inst->TypeInfo().name);
            });
    }

    void CheckCall(const ir::Call* call) {
        tint::Switch(
            call,                          //
            [&](const ir::Bitcast*) {},    //
            [&](const ir::Builtin*) {},    //
            [&](const ir::Construct*) {},  //
            [&](const ir::Convert*) {},    //
            [&](const ir::Discard*) {},    //
            [&](const ir::UserCall*) {},   //
            [&](Default) {
                AddError(std::string("missing validation of call: ") + call->TypeInfo().name);
            });
    }

    void CheckBranch(const ir::Branch* b) {
        tint::Switch(
            b,                                         //
            [&](const ir::BreakIf*) {},                //
            [&](const ir::Continue*) {},               //
            [&](const ir::ExitIf*) {},                 //
            [&](const ir::ExitLoop*) {},               //
            [&](const ir::ExitSwitch*) {},             //
            [&](const ir::If* if_) { CheckIf(if_); },  //
            [&](const ir::Loop*) {},                   //
            [&](const ir::NextIteration*) {},          //
            [&](const ir::Return* ret) {
                if (ret->Func() == nullptr) {
                    AddError("return: null function");
                }
            },                          //
            [&](const ir::Switch*) {},  //
            [&](Default) {
                AddError(std::string("missing validation of branch: ") + b->TypeInfo().name);
            });
    }

    void CheckIf(const ir::If* if_) {
        if (!if_->Condition()) {
            AddError(if_, "if: condition is nullptr");
        }
        if (if_->Condition() && !if_->Condition()->Type()->Is<type::Bool>()) {
            AddError(if_, If::kConditionOperandIndex, "if: condition must be a `bool` type");
        }
    }
};

}  // namespace

utils::Result<Success, diag::List> Validate(Module& mod) {
    Validator v(mod);
    return v.IsValid();
}

}  // namespace tint::ir
