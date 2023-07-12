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
#include "src/tint/ir/intrinsic_call.h"
#include "src/tint/ir/let.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/load_vector_element.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/multi_in_block.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/store_vector_element.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/swizzle.h"
#include "src/tint/ir/terminate_invocation.h"
#include "src/tint/ir/unary.h"
#include "src/tint/ir/unreachable.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/reverse.h"
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
            DisassembleIfNeeded();
            diagnostics_.add_note(tint::diag::System::IR,
                                  "# Disassembly\n" + mod_.disassembly_file->content.data, {});
            return std::move(diagnostics_);
        }
        return Success{};
    }

  private:
    Module& mod_;
    diag::List diagnostics_;
    Disassembler dis_{mod_};

    Block* current_block_ = nullptr;
    utils::Hashset<Function*, 4> seen_functions_;
    utils::Vector<ControlInstruction*, 8> control_stack_;

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

    void AddResultError(Instruction* inst, size_t idx, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.ResultSource(Usage{inst, static_cast<uint32_t>(idx)});
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

    void AddNote(Instruction* inst, std::string err) {
        DisassembleIfNeeded();
        auto src = dis_.InstructionSource(inst);
        src.file = mod_.disassembly_file.get();
        AddNote(std::move(err), src);
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

    std::string Name(Value* v) { return mod_.NameOf(v).Name(); }

    void CheckOperandNotNull(ir::Instruction* inst,
                             ir::Value* operand,
                             size_t idx,
                             std::string_view name) {
        if (operand == nullptr) {
            AddError(inst, idx, std::string(name) + ": operand is undefined");
        }
    }

    void CheckOperandsNotNull(ir::Instruction* inst,
                              size_t start_operand,
                              size_t end_operand,
                              std::string_view name) {
        auto operands = inst->Operands();
        for (size_t i = start_operand; i <= end_operand; i++) {
            CheckOperandNotNull(inst, operands[i], i, name);
        }
    }

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
            CheckInstruction(var);
        }
    }

    void CheckFunction(Function* func) {
        if (!seen_functions_.Add(func)) {
            AddError("function '" + Name(func) + "' added to module multiple times");
        }

        CheckBlock(func->Block());
    }

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
            return;
        }
        if (inst->HasResults()) {
            auto results = inst->Results();
            for (size_t i = 0; i < results.Length(); ++i) {
                auto* res = results[i];
                if (!res) {
                    AddResultError(inst, i, "instruction result is undefined");
                    continue;
                }

                if (res->Source() == nullptr) {
                    AddResultError(inst, i, "instruction result source is undefined");
                } else if (res->Source() != inst) {
                    AddResultError(inst, i, "instruction result source has wrong instruction");
                }
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
                AddError(inst, i, "instruction has operand which is not alive");
            }

            if (!op->Usages().Contains({inst, i})) {
                AddError(inst, i, "instruction operand missing usage");
            }
        }

        tint::Switch(
            inst,                                                        //
            [&](Access* a) { CheckAccess(a); },                          //
            [&](Binary* b) { CheckBinary(b); },                          //
            [&](Call* c) { CheckCall(c); },                              //
            [&](If* if_) { CheckIf(if_); },                              //
            [&](Let* let) { CheckLet(let); },                            //
            [&](Load*) {},                                               //
            [&](LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
            [&](Loop* l) { CheckLoop(l); },                              //
            [&](Store*) {},                                              //
            [&](StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
            [&](Switch* s) { CheckSwitch(s); },                          //
            [&](Swizzle*) {},                                            //
            [&](Terminator* b) { CheckTerminator(b); },                  //
            [&](Unary* u) { CheckUnary(u); },                            //
            [&](Var* var) { CheckVar(var); },                            //
            [&](Default) {
                AddError(std::string("missing validation of: ") + inst->TypeInfo().name);
            });
    }

    void CheckVar(Var* var) {
        if (var->Result() && var->Initializer()) {
            if (var->Initializer()->Type() != var->Result()->Type()->UnwrapPtr()) {
                AddError(var, "var initializer has incorrect type");
            }
        }
    }

    void CheckLet(Let* let) {
        CheckOperandNotNull(let, let->Value(), Let::kValueOperandOffset, "let");

        if (let->Result() && let->Value()) {
            if (let->Result()->Type() != let->Value()->Type()) {
                AddError(let, "let result type does not match value type");
            }
        }
    }

    void CheckCall(Call* call) {
        tint::Switch(
            call,                    //
            [&](Bitcast*) {},        //
            [&](BuiltinCall*) {},    //
            [&](IntrinsicCall*) {},  //
            [&](Construct*) {},      //
            [&](Convert*) {},        //
            [&](Discard*) {},        //
            [&](UserCall*) {},       //
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
                AddError(a, i + Access::kIndicesOperandOffset, "access: " + msg);
            };
            auto note = [&](std::string msg) {
                AddNote(a, i + Access::kIndicesOperandOffset, msg);
            };

            auto* index = a->Indices()[i];
            if (TINT_UNLIKELY(!index->Type()->is_integer_scalar())) {
                err("index must be integer, got " + index->Type()->FriendlyName());
                return;
            }

            if (is_ptr && ty->Is<type::Vector>()) {
                err("cannot obtain address of vector element");
                return;
            }

            if (auto* const_index = index->As<ir::Constant>()) {
                auto* value = const_index->Value();
                if (value->Type()->is_signed_integer_scalar()) {
                    // index is a signed integer scalar. Check that the index isn't negative.
                    // If the index is unsigned, we can skip this.
                    auto idx = value->ValueAs<AInt>();
                    if (TINT_UNLIKELY(idx < 0)) {
                        err("constant index must be positive, got " + std::to_string(idx));
                        return;
                    }
                }

                auto idx = value->ValueAs<uint32_t>();
                auto* el = ty->Element(idx);
                if (TINT_UNLIKELY(!el)) {
                    // Is index in bounds?
                    if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                        err("index out of bounds for type " + current());
                        note("acceptable range: [0.." + std::to_string(el_count - 1) + "]");
                        return;
                    }
                    err("type " + current() + " cannot be indexed");
                    return;
                }
                ty = el;
            } else {
                auto* el = ty->Elements().type;
                if (TINT_UNLIKELY(!el)) {
                    err("type " + current() + " cannot be dynamically indexed");
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
        CheckOperandsNotNull(b, Binary::kLhsOperandOffset, Binary::kRhsOperandOffset, "binary");
    }

    void CheckUnary(ir::Unary* u) {
        CheckOperandNotNull(u, u->Val(), Unary::kValueOperandOffset, "unary");

        if (u->Result() && u->Val()) {
            if (u->Result()->Type() != u->Val()->Type()) {
                AddError(u, "unary: result type must match value type");
            }
        }
    }

    void CheckIf(If* if_) {
        CheckOperandNotNull(if_, if_->Condition(), If::kConditionOperandOffset, "if");

        if (if_->Condition() && !if_->Condition()->Type()->Is<type::Bool>()) {
            AddError(if_, If::kConditionOperandOffset, "if: condition must be a `bool` type");
        }

        control_stack_.Push(if_);
        TINT_DEFER(control_stack_.Pop());

        CheckBlock(if_->True());
        if (!if_->False()->IsEmpty()) {
            CheckBlock(if_->False());
        }
    }

    void CheckLoop(Loop* l) {
        control_stack_.Push(l);
        TINT_DEFER(control_stack_.Pop());

        if (!l->Initializer()->IsEmpty()) {
            CheckBlock(l->Initializer());
        }
        CheckBlock(l->Body());

        if (!l->Continuing()->IsEmpty()) {
            CheckBlock(l->Continuing());
        }
    }

    void CheckSwitch(Switch* s) {
        control_stack_.Push(s);
        TINT_DEFER(control_stack_.Pop());

        for (auto& cse : s->Cases()) {
            CheckBlock(cse.block);
        }
    }

    void CheckTerminator(ir::Terminator* b) {
        // Note, transforms create `undef` terminator arguments (this is done in MergeReturn and
        // DemoteToHelper) so we can't add validation.

        tint::Switch(
            b,                                   //
            [&](ir::BreakIf*) {},                //
            [&](ir::Continue*) {},               //
            [&](ir::Exit* e) { CheckExit(e); },  //
            [&](ir::NextIteration*) {},          //
            [&](ir::Return* ret) {
                if (ret->Func() == nullptr) {
                    AddError("return: undefined function");
                }
            },
            [&](ir::TerminateInvocation*) {},  //
            [&](ir::Unreachable*) {},          //
            [&](Default) {
                AddError(std::string("missing validation of terminator: ") + b->TypeInfo().name);
            });
    }

    void CheckExit(ir::Exit* e) {
        if (e->ControlInstruction() == nullptr) {
            AddError(e, "exit: has no parent control instruction");
            return;
        }

        auto results = e->ControlInstruction()->Results();
        auto args = e->Args();
        if (results.Length() != args.Length()) {
            AddError(e, std::string("exit: args count (") + std::to_string(args.Length()) +
                            ") does not match control instruction result count (" +
                            std::to_string(results.Length()) + ")");
            return;
        }

        if (control_stack_.IsEmpty()) {
            AddError(e, "exit: found outside all control instructions");
            return;
        }

        for (size_t i = 0; i < results.Length(); ++i) {
            if (results[i] && args[i] && results[i]->Type() != args[i]->Type()) {
                AddError(e, i,
                         std::string("exit: argument type (") + results[i]->Type()->FriendlyName() +
                             ") does not match control instruction type (" +
                             args[i]->Type()->FriendlyName() + ")");
            }
        }

        tint::Switch(
            e,                                               //
            [&](ir::ExitIf* i) { CheckExitIf(i); },          //
            [&](ir::ExitLoop* l) { CheckExitLoop(l); },      //
            [&](ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
            [&](Default) {
                AddError(std::string("missing validation of exit: ") + e->TypeInfo().name);
            });
    }

    void CheckExitIf(ExitIf* e) {
        if (control_stack_.Back() != e->If()) {
            AddError(e, "exit_if: if target jumps over other control instructions");
            AddNote(control_stack_.Back(), "first control instruction jumped");
        }
    }

    void CheckControlsAllowingIf(Exit* exit, Instruction* control, std::string_view name) {
        bool found = false;
        for (auto ctrl : utils::Reverse(control_stack_)) {
            if (ctrl == control) {
                found = true;
                break;
            }
            // A exit switch can step over if instructions, but no others.
            if (!ctrl->Is<ir::If>()) {
                AddError(exit, "exit: " + std::string(name) +
                                   " target jumps over other control instructions");
                AddNote(ctrl, "first control instruction jumped");
                return;
            }
        }
        if (!found) {
            AddError(exit,
                     "exit: " + std::string(name) + " not found in parent control instructions");
        }
    }

    void CheckExitSwitch(ExitSwitch* s) {
        CheckControlsAllowingIf(s, s->ControlInstruction(), "switch");
    }

    void CheckExitLoop(ExitLoop* l) {
        CheckControlsAllowingIf(l, l->ControlInstruction(), "loop");

        Instruction* inst = l;
        Loop* control = l->Loop();
        while (inst) {
            // Found parent loop
            if (inst->Block()->Parent() == control) {
                if (inst->Block() == control->Continuing()) {
                    AddError(l, "exit_loop: loop exit jumps out of continuing block");
                    if (control->Continuing() != l->Block()) {
                        AddNote(control->Continuing(), "in continuing block");
                    }
                } else if (inst->Block() == control->Initializer()) {
                    AddError(l, "exit_loop: loop exit not permitted in loop initializer");
                    if (control->Initializer() != l->Block()) {
                        AddNote(control->Initializer(), "in initializer block");
                    }
                }
                break;
            }
            inst = inst->Block()->Parent();
        }
    }

    void CheckLoadVectorElement(LoadVectorElement* l) {
        CheckOperandsNotNull(l,  //
                             LoadVectorElement::kFromOperandOffset,
                             LoadVectorElement::kIndexOperandOffset, "load_vector_element");

        if (auto* res = l->Result()) {
            if (auto* el_ty = GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset)) {
                if (res->Type() != el_ty) {
                    AddResultError(l, 0, "result type does not match vector pointer element type");
                }
            }
        }
    }

    void CheckStoreVectorElement(StoreVectorElement* s) {
        CheckOperandsNotNull(s,  //
                             StoreVectorElement::kToOperandOffset,
                             StoreVectorElement::kValueOperandOffset, "store_vector_element");

        if (auto* value = s->Value()) {
            if (auto* el_ty = GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset)) {
                if (value->Type() != el_ty) {
                    AddError(s, StoreVectorElement::kValueOperandOffset,
                             "value type does not match vector pointer element type");
                }
            }
        }
    }

    const type::Type* GetVectorPtrElementType(Instruction* inst, size_t idx) {
        auto* operand = inst->Operands()[idx];
        if (TINT_UNLIKELY(!operand)) {
            return nullptr;
        }

        auto* type = operand->Type();
        if (TINT_UNLIKELY(!type)) {
            return nullptr;
        }

        auto* vec_ptr_ty = type->As<type::Pointer>();
        if (TINT_LIKELY(vec_ptr_ty)) {
            auto* vec_ty = vec_ptr_ty->StoreType()->As<type::Vector>();
            if (TINT_LIKELY(vec_ty)) {
                return vec_ty->type();
            }
        }

        AddError(inst, idx, "operand must be a pointer to vector, got " + type->FriendlyName());
        return nullptr;
    }
};

}  // namespace

utils::Result<Success, diag::List> Validate(Module& mod) {
    Validator v(mod);
    return v.IsValid();
}

}  // namespace tint::ir
