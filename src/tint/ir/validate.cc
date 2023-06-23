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
#include "src/tint/ir/builtin_call.h"
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
#include "src/tint/ir/unreachable.h"
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

        for (auto* func : mod_.functions) {
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

    Block* current_block_ = nullptr;

    void DisassembleIfNeeded() {
        if (mod_.disassembly_file) {
            return;
        }
        mod_.disassembly_file = std::make_unique<Source::File>("", dis_.Disassemble());
    }

    void AddError(Instruction* inst, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.InstructionSource(inst);
        src.file = mod_.disassembly_file.get();
        AddError(std::move(err), src);

        if (current_block_) {
            AddNote(current_block_, "In block");
        }
    }

    void AddError(Instruction* inst, size_t idx, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.OperandSource(Usage{inst, static_cast<uint32_t>(idx)});
        src.file = mod_.disassembly_file.get();
        AddError(std::move(err), src);

        if (current_block_) {
            AddNote(current_block_, "In block");
        }
    }

    void AddError(Block* blk, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.BlockSource(blk);
        src.file = mod_.disassembly_file.get();
        AddError(std::move(err), src);
    }

    void AddNote(Instruction* inst, size_t idx, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.OperandSource(Usage{inst, static_cast<uint32_t>(idx)});
        src.file = mod_.disassembly_file.get();
        AddNote(std::move(err), src);
    }

    void AddNote(Block* blk, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.BlockSource(blk);
        src.file = mod_.disassembly_file.get();
        AddNote(std::move(err), src);
    }

    void AddError(std::string err, Source src = {}) {
        diagnostics_.add_error(tint::diag::System::IR, std::move(err), src);
    }

    void AddNote(std::string note, Source src = {}) {
        diagnostics_.add_note(tint::diag::System::IR, std::move(note), src);
    }

    // std::string Name(Value* v) { return mod_.NameOf(v).Name(); }

    void CheckRootBlock(Block* blk) {
        if (!blk) {
            return;
        }

        TINT_SCOPED_ASSIGNMENT(current_block_, blk);

        for (auto* inst : *blk) {
            auto* var = inst->As<ir::Var>();
            if (!var) {
                AddError(inst,
                         std::string("root block: invalid instruction: ") + inst->TypeInfo().name);
                continue;
            }
            CheckVar(var);
        }
    }

    void CheckFunction(Function* func) { CheckBlock(func->Block()); }

    void CheckBlock(Block* blk) {
        TINT_SCOPED_ASSIGNMENT(current_block_, blk);

        if (!blk->HasTerminator()) {
            AddError(blk, "block: does not end in a terminator instruction");
        }

        for (auto* inst : *blk) {
            if (inst->Is<ir::Terminator>() && inst != blk->Terminator()) {
                AddError(inst, "block: terminator which isn't the final instruction");
                continue;
            }

            CheckInstruction(inst);
        }
    }

    void CheckInstruction(Instruction* inst) {
        if (!inst->Alive()) {
            AddError(inst, "destroyed instruction found in instruction list");
        }
        if (inst->Result()) {
            if (inst->Result()->Source() == nullptr) {
                AddError(inst, "instruction result source is undefined");
            } else if (inst->Result()->Source() != inst) {
                AddError(inst, "instruction result source has wrong instruction");
            }
        }

        auto ops = inst->Operands();
        for (size_t i = 0; i < ops.Length(); ++i) {
            auto* op = ops[i];
            if (!op) {
                continue;
            }

            // Note, a `nullptr` is a valid operand in some cases, like `var` so we can't just check
            // for `nullptr` here.
            if (!op->Alive()) {
                AddError(inst, "instruction has undefined operand");
            }

            if (!op->Usages().Contains({inst, i})) {
                AddError(inst, i, "instruction operand missing usage");
            }
        }

        tint::Switch(
            inst,                                        //
            [&](Access* a) { CheckAccess(a); },          //
            [&](Binary* b) { CheckBinary(b); },          //
            [&](Call* c) { CheckCall(c); },              //
            [&](If* if_) { CheckIf(if_); },              //
            [&](Load*) {},                               //
            [&](Loop*) {},                               //
            [&](Store*) {},                              //
            [&](Switch*) {},                             //
            [&](Swizzle*) {},                            //
            [&](Terminator* b) { CheckTerminator(b); },  //
            [&](Unary*) {},                              //
            [&](Var* var) { CheckVar(var); },            //
            [&](Default) {
                AddError(std::string("missing validation of: ") + inst->TypeInfo().name);
            });
    }

    void CheckCall(Call* call) {
        tint::Switch(
            call,                  //
            [&](Bitcast*) {},      //
            [&](BuiltinCall*) {},  //
            [&](Construct*) {},    //
            [&](Convert*) {},      //
            [&](Discard*) {},      //
            [&](UserCall*) {},     //
            [&](Default) {
                AddError(std::string("missing validation of call: ") + call->TypeInfo().name);
            });
    }

    void CheckAccess(ir::Access* a) {
        bool is_ptr = a->Object()->Type()->Is<type::Pointer>();
        auto* ty = a->Object()->Type()->UnwrapPtr();

        auto current = [&] {
            return is_ptr ? "ptr<" + ty->FriendlyName() + ">" : ty->FriendlyName();
        };

        for (size_t i = 0; i < a->Indices().Length(); i++) {
            auto err = [&](std::string msg) {
                AddError(a, i + Access::kIndicesOperandOffset, std::move(msg));
            };
            auto note = [&](std::string msg) {
                AddNote(a, i + Access::kIndicesOperandOffset, std::move(msg));
            };

            auto* index = a->Indices()[i];
            if (TINT_UNLIKELY(!index->Type()->is_integer_scalar())) {
                err("access: index must be integer, got " + index->Type()->FriendlyName());
                return;
            }

            if (auto* const_index = index->As<ir::Constant>()) {
                auto* value = const_index->Value();
                if (value->Type()->is_signed_integer_scalar()) {
                    // index is a signed integer scalar. Check that the index isn't negative.
                    // If the index is unsigned, we can skip this.
                    auto idx = value->ValueAs<AInt>();
                    if (TINT_UNLIKELY(idx < 0)) {
                        err("access: constant index must be positive, got " + std::to_string(idx));
                        return;
                    }
                }

                auto idx = value->ValueAs<uint32_t>();
                auto* el = ty->Element(idx);
                if (TINT_UNLIKELY(!el)) {
                    // Is index in bounds?
                    if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                        err("access: index out of bounds for type " + current());
                        note("acceptable range: [0.." + std::to_string(el_count - 1) + "]");
                        return;
                    }
                    err("access: type " + current() + " cannot be indexed");
                    return;
                }
                ty = el;
            } else {
                auto* el = ty->Elements().type;
                if (TINT_UNLIKELY(!el)) {
                    err("access: type " + current() + " cannot be dynamically indexed");
                    return;
                }
                ty = el;
            }
        }

        auto* want_ty = a->Result()->Type()->UnwrapPtr();
        bool want_ptr = a->Result()->Type()->Is<type::Pointer>();
        if (TINT_UNLIKELY(ty != want_ty || is_ptr != want_ptr)) {
            std::string want =
                want_ptr ? "ptr<" + want_ty->FriendlyName() + ">" : want_ty->FriendlyName();
            AddError(a, "access: result of access chain is type " + current() +
                            " but instruction type is " + want);
            return;
        }
    }

    void CheckBinary(ir::Binary* b) {
        if (b->LHS() == nullptr) {
            AddError(b, "binary: left operand is undefined");
        }
        if (b->RHS() == nullptr) {
            AddError(b, "binary: right operand is undefined");
        }
        if (b->Result() == nullptr) {
            AddError(b, "binary: result is undefined");
        }
    }

    void CheckTerminator(ir::Terminator* b) {
        tint::Switch(
            b,                           //
            [&](ir::BreakIf*) {},        //
            [&](ir::Continue*) {},       //
            [&](ir::ExitIf*) {},         //
            [&](ir::ExitLoop*) {},       //
            [&](ir::ExitSwitch*) {},     //
            [&](ir::NextIteration*) {},  //
            [&](ir::Return* ret) {
                if (ret->Func() == nullptr) {
                    AddError("return: null function");
                }
            },
            [&](ir::Unreachable*) {},  //
            [&](Default) {
                AddError(std::string("missing validation of terminator: ") + b->TypeInfo().name);
            });
    }

    void CheckIf(If* if_) {
        if (!if_->Condition()) {
            AddError(if_, "if: condition is undefined");
        }
        if (if_->Condition() && !if_->Condition()->Type()->Is<type::Bool>()) {
            AddError(if_, If::kConditionOperandOffset, "if: condition must be a `bool` type");
        }
    }

    void CheckVar(Var* var) {
        if (var->Result() == nullptr) {
            AddError(var, "var: result is undefined");
        }

        if (var->Result() && var->Initializer()) {
            if (var->Initializer()->Type() != var->Result()->Type()->UnwrapPtr()) {
                AddError(var, "var initializer has incorrect type");
            }
        }
    }
};  // namespace

}  // namespace

utils::Result<Success, diag::List> Validate(Module& mod) {
    Validator v(mod);
    return v.IsValid();
}

}  // namespace tint::ir
