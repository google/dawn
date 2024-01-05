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

#include "src/tint/lang/core/ir/validator.h"

#include <memory>
#include <string>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"

/// If set to 1 then the Tint will dump the IR when validating.
#define TINT_DUMP_IR_WHEN_VALIDATING 0
#if TINT_DUMP_IR_WHEN_VALIDATING
#include <iostream>
#endif

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {

namespace {

/// The core IR validator.
class Validator {
  public:
    /// Create a core validator
    /// @param mod the module to be validated
    explicit Validator(const Module& mod);

    /// Destructor
    ~Validator();

    /// Runs the validator over the module provided during construction
    /// @returns success or failure
    Result<SuccessType> Run();

  protected:
    /// @param inst the instruction
    /// @param err the error message
    /// @returns a string with the instruction name name and error message formatted
    std::string InstError(const Instruction* inst, std::string err);

    /// Adds an error for the @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @param err the error string
    void AddError(const Instruction* inst, std::string err);

    /// Adds an error for the @p inst operand at @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instaruction
    /// @param idx the operand index
    /// @param err the error string
    void AddError(const Instruction* inst, size_t idx, std::string err);

    /// Adds an error for the @p inst result at @p idx and highlgihts the result in the disassembly
    /// @param inst the instruction
    /// @param idx the result index
    /// @param err the error string
    void AddResultError(const Instruction* inst, size_t idx, std::string err);

    /// Adds an error the @p block and highlights the block header in the disassembly
    /// @param blk the block
    /// @param err the error string
    void AddError(const Block* blk, std::string err);

    /// Adds a note to @p inst and highlights the instruction in the disassembly
    /// @param inst the instruction
    /// @param err the message to emit
    void AddNote(const Instruction* inst, std::string err);

    /// Adds a note to @p inst for operand @p idx and highlights the operand in the
    /// disassembly
    /// @param inst the instruction
    /// @param idx the operand index
    /// @param err the message string
    void AddNote(const Instruction* inst, size_t idx, std::string err);

    /// Adds a note to @p blk and highlights the block in the disassembly
    /// @param blk the block
    /// @param err the message to emit
    void AddNote(const Block* blk, std::string err);

    /// Adds an error to the diagnostics
    /// @param err the message to emit
    /// @param src the source lines to highlight
    void AddError(std::string err, Source src = {});

    /// Adds a note to the diagnostics
    /// @param note the note to emit
    /// @param src the source lines to highlight
    void AddNote(std::string note, Source src = {});

    /// @param v the value to get the name for
    /// @returns the name for the given value
    std::string Name(const Value* v);

    /// Checks the given operand is not null
    /// @param inst the instruction
    /// @param operand the operand
    /// @param idx the operand index
    void CheckOperandNotNull(const ir::Instruction* inst, const ir::Value* operand, size_t idx);

    /// Checks all operands in the given range (inclusive) for @p inst are not null
    /// @param inst the instruction
    /// @param start_operand the first operand to check
    /// @param end_operand the last operand to check
    void CheckOperandsNotNull(const ir::Instruction* inst,
                              size_t start_operand,
                              size_t end_operand);

    /// Validates the root block
    /// @param blk the block
    void CheckRootBlock(const Block* blk);

    /// Validates the given function
    /// @param func the function validate
    void CheckFunction(const Function* func);

    /// Validates the given block
    /// @param blk the block to validate
    void CheckBlock(const Block* blk);

    /// Validates the given instruction
    /// @param inst the instruction to validate
    void CheckInstruction(const Instruction* inst);

    /// Validates the given var
    /// @param var the var to validate
    void CheckVar(const Var* var);

    /// Validates the given let
    /// @param let the let to validate
    void CheckLet(const Let* let);

    /// Validates the given call
    /// @param call the call to validate
    void CheckCall(const Call* call);

    /// Validates the given builtin call
    /// @param call the call to validate
    void CheckBuiltinCall(const BuiltinCall* call);

    /// Validates the given user call
    /// @param call the call to validate
    void CheckUserCall(const UserCall* call);

    /// Validates the given access
    /// @param a the access to validate
    void CheckAccess(const Access* a);

    /// Validates the given binary
    /// @param b the binary to validate
    void CheckBinary(const Binary* b);

    /// Validates the given unary
    /// @param u the unary to validate
    void CheckUnary(const Unary* u);

    /// Validates the given if
    /// @param if_ the if to validate
    void CheckIf(const If* if_);

    /// Validates the given loop
    /// @param l the loop to validate
    void CheckLoop(const Loop* l);

    /// Validates the given switch
    /// @param s the switch to validate
    void CheckSwitch(const Switch* s);

    /// Validates the given terminator
    /// @param b the terminator to validate
    void CheckTerminator(const Terminator* b);

    /// Validates the given exit
    /// @param e the exit to validate
    void CheckExit(const Exit* e);

    /// Validates the given exit if
    /// @param e the exit if to validate
    void CheckExitIf(const ExitIf* e);

    /// Validates the given return
    /// @param r the return to validate
    void CheckReturn(const Return* r);

    /// Validates the @p exit targets a valid @p control instruction where the instruction may jump
    /// over if control instructions.
    /// @param exit the exit to validate
    /// @param control the control instruction targeted
    void CheckControlsAllowingIf(const Exit* exit, const Instruction* control);

    /// Validates the given exit switch
    /// @param s the exit switch to validate
    void CheckExitSwitch(const ExitSwitch* s);

    /// Validates the given exit loop
    /// @param l the exit loop to validate
    void CheckExitLoop(const ExitLoop* l);

    /// Validates the given store
    /// @param s the store to validate
    void CheckStore(const Store* s);

    /// Validates the given load vector element
    /// @param l the load vector element to validate
    void CheckLoadVectorElement(const LoadVectorElement* l);

    /// Validates the given store vector element
    /// @param s the store vector element to validate
    void CheckStoreVectorElement(const StoreVectorElement* s);

    /// @param inst the instruction
    /// @param idx the operand index
    /// @returns the vector pointer type for the given instruction operand
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);

  private:
    const Module& mod_;
    std::shared_ptr<Source::File> disassembly_file;
    diag::List diagnostics_;
    Disassembler dis_{mod_};
    const Block* current_block_ = nullptr;
    Hashset<const Function*, 4> all_functions_;
    Hashset<const Instruction*, 4> visited_instructions_;
    Vector<const ControlInstruction*, 8> control_stack_;

    void DisassembleIfNeeded();
};

Validator::Validator(const Module& mod) : mod_(mod) {}

Validator::~Validator() = default;

void Validator::DisassembleIfNeeded() {
    if (disassembly_file) {
        return;
    }
    disassembly_file = std::make_unique<Source::File>("", dis_.Disassemble());
}

Result<SuccessType> Validator::Run() {
    CheckRootBlock(mod_.root_block);

    for (auto& func : mod_.functions) {
        if (!all_functions_.Add(func.Get())) {
            AddError("function '" + Name(func.Get()) + "' added to module multiple times");
        }
    }

    for (auto& func : mod_.functions) {
        CheckFunction(func);
    }

    if (!diagnostics_.contains_errors()) {
        // Check for orphaned instructions.
        for (auto* inst : mod_.instructions.Objects()) {
            if (inst->Alive() && !visited_instructions_.Contains(inst)) {
                AddError("orphaned instruction: " + inst->FriendlyName());
            }
        }
    }

    if (diagnostics_.contains_errors()) {
        DisassembleIfNeeded();
        diagnostics_.add_note(tint::diag::System::IR,
                              "# Disassembly\n" + disassembly_file->content.data, {});
        return Failure{std::move(diagnostics_)};
    }
    return Success;
}

std::string Validator::InstError(const Instruction* inst, std::string err) {
    return std::string(inst->FriendlyName()) + ": " + err;
}

void Validator::AddError(const Instruction* inst, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.InstructionSource(inst);
    AddError(std::move(err), src);

    if (current_block_) {
        AddNote(current_block_, "In block");
    }
}

void Validator::AddError(const Instruction* inst, size_t idx, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    AddError(std::move(err), src);

    if (current_block_) {
        AddNote(current_block_, "In block");
    }
}

void Validator::AddResultError(const Instruction* inst, size_t idx, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.ResultSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    AddError(std::move(err), src);

    if (current_block_) {
        AddNote(current_block_, "In block");
    }
}

void Validator::AddError(const Block* blk, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.BlockSource(blk);
    AddError(std::move(err), src);
}

void Validator::AddNote(const Instruction* inst, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.InstructionSource(inst);
    AddNote(std::move(err), src);
}

void Validator::AddNote(const Instruction* inst, size_t idx, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.OperandSource(Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
    AddNote(std::move(err), src);
}

void Validator::AddNote(const Block* blk, std::string err) {
    DisassembleIfNeeded();
    auto src = dis_.BlockSource(blk);
    AddNote(std::move(err), src);
}

void Validator::AddError(std::string err, Source src) {
    auto& diag = diagnostics_.add_error(tint::diag::System::IR, std::move(err), src);
    if (src.range != Source::Range{{}}) {
        diag.source.file = disassembly_file.get();
        diag.owned_file = disassembly_file;
    }
}

void Validator::AddNote(std::string note, Source src) {
    auto& diag = diagnostics_.add_note(tint::diag::System::IR, std::move(note), src);
    if (src.range != Source::Range{{}}) {
        diag.source.file = disassembly_file.get();
        diag.owned_file = disassembly_file;
    }
}

std::string Validator::Name(const Value* v) {
    return mod_.NameOf(v).Name();
}

void Validator::CheckOperandNotNull(const Instruction* inst, const ir::Value* operand, size_t idx) {
    if (operand == nullptr) {
        AddError(inst, idx, InstError(inst, "operand is undefined"));
    }
}

void Validator::CheckOperandsNotNull(const Instruction* inst,
                                     size_t start_operand,
                                     size_t end_operand) {
    auto operands = inst->Operands();
    for (size_t i = start_operand; i <= end_operand; i++) {
        CheckOperandNotNull(inst, operands[i], i);
    }
}

void Validator::CheckRootBlock(const Block* blk) {
    TINT_SCOPED_ASSIGNMENT(current_block_, blk);

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(
                inst,
                InstError(inst, "instruction in root block does not have root block as parent"));
            continue;
        }
        auto* var = inst->As<ir::Var>();
        if (!var) {
            AddError(inst,
                     std::string("root block: invalid instruction: ") + inst->TypeInfo().name);
            continue;
        }
        CheckInstruction(var);
    }
}

void Validator::CheckFunction(const Function* func) {
    CheckBlock(func->Block());
}

void Validator::CheckBlock(const Block* blk) {
    TINT_SCOPED_ASSIGNMENT(current_block_, blk);

    if (!blk->Terminator()) {
        AddError(blk, "block: does not end in a terminator instruction");
    }

    for (auto* inst : *blk) {
        if (inst->Block() != blk) {
            AddError(inst, InstError(inst, "block instruction does not have same block as parent"));
            AddNote(current_block_, "In block");
            continue;
        }
        if (inst->Is<ir::Terminator>() && inst != blk->Terminator()) {
            AddError(inst, "block: terminator which isn't the final instruction");
            continue;
        }

        CheckInstruction(inst);
    }
}

void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst, InstError(inst, "destroyed instruction found in instruction list"));
        return;
    }
    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            AddResultError(inst, i, InstError(inst, "result is undefined"));
            continue;
        }

        if (res->Instruction() == nullptr) {
            AddResultError(inst, i, InstError(inst, "instruction of result is undefined"));
        } else if (res->Instruction() != inst) {
            AddResultError(inst, i,
                           InstError(inst, "instruction of result is a different instruction"));
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
            AddError(inst, i, InstError(inst, "operand is not alive"));
        }

        if (!op->HasUsage(inst, i)) {
            AddError(inst, i, InstError(inst, "operand missing usage"));
        }
    }

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load*) {},                                               //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle*) {},                                            //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Var* var) { CheckVar(var); },                            //
        [&](const Default) { AddError(inst, InstError(inst, "missing validation")); });
}

void Validator::CheckVar(const Var* var) {
    if (var->Result(0) && var->Initializer()) {
        if (var->Initializer()->Type() != var->Result(0)->Type()->UnwrapPtr()) {
            AddError(var, InstError(var, "initializer has incorrect type"));
        }
    }
}

void Validator::CheckLet(const Let* let) {
    CheckOperandNotNull(let, let->Value(), Let::kValueOperandOffset);

    if (let->Result(0) && let->Value()) {
        if (let->Result(0)->Type() != let->Value()->Type()) {
            AddError(let, InstError(let, "result type does not match value type"));
        }
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                //
        [&](const Bitcast*) {},                              //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },  //
        [&](const Construct*) {},                            //
        [&](const Convert*) {},                              //
        [&](const Discard*) {},                              //
        [&](const UserCall* c) { CheckUserCall(c); },        //
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    auto symbols = SymbolTable::Wrap(mod_.symbols);
    auto type_mgr = type::Manager::Wrap(mod_.Types());

    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });
    intrinsic::Context context{
        call->TableData(),
        type_mgr,
        symbols,
    };

    auto result = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                            args, core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call, InstError(call, result.Failure()));
        return;
    }

    if (result->return_type != call->Result(0)->Type()) {
        AddError(call, InstError(call, "call result type does not match builtin return type"));
    }
}

void Validator::CheckUserCall(const UserCall* call) {
    if (!all_functions_.Contains(call->Target())) {
        AddError(call, UserCall::kFunctionOperandOffset,
                 InstError(call, "call target is not part of the module"));
    }
}

void Validator::CheckAccess(const Access* a) {
    bool is_ptr = a->Object()->Type()->Is<core::type::Pointer>();
    auto* ty = a->Object()->Type()->UnwrapPtr();

    auto current = [&] { return is_ptr ? "ptr<" + ty->FriendlyName() + ">" : ty->FriendlyName(); };

    for (size_t i = 0; i < a->Indices().Length(); i++) {
        auto err = [&](std::string msg) {
            AddError(a, i + Access::kIndicesOperandOffset, InstError(a, msg));
        };
        auto note = [&](std::string msg) { AddNote(a, i + Access::kIndicesOperandOffset, msg); };

        auto* index = a->Indices()[i];
        if (TINT_UNLIKELY(!index->Type()->is_integer_scalar())) {
            err("index must be integer, got " + index->Type()->FriendlyName());
            return;
        }

        if (is_ptr && ty->Is<core::type::Vector>()) {
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

    auto* want_ty = a->Result(0)->Type()->UnwrapPtr();
    bool want_ptr = a->Result(0)->Type()->Is<core::type::Pointer>();
    if (TINT_UNLIKELY(ty != want_ty || is_ptr != want_ptr)) {
        std::string want =
            want_ptr ? "ptr<" + want_ty->FriendlyName() + ">" : want_ty->FriendlyName();
        AddError(a, InstError(a, "result of access chain is type " + current() +
                                     " but instruction type is " + want));
        return;
    }
}

void Validator::CheckBinary(const Binary* b) {
    CheckOperandsNotNull(b, Binary::kLhsOperandOffset, Binary::kRhsOperandOffset);
}

void Validator::CheckUnary(const Unary* u) {
    CheckOperandNotNull(u, u->Val(), Unary::kValueOperandOffset);

    if (u->Result(0) && u->Val()) {
        if (u->Result(0)->Type() != u->Val()->Type()) {
            AddError(u, InstError(u, "result type must match value type"));
        }
    }
}

void Validator::CheckIf(const If* if_) {
    CheckOperandNotNull(if_, if_->Condition(), If::kConditionOperandOffset);

    if (if_->Condition() && !if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset,
                 InstError(if_, "condition must be a `bool` type"));
    }

    control_stack_.Push(if_);
    TINT_DEFER(control_stack_.Pop());

    CheckBlock(if_->True());
    if (!if_->False()->IsEmpty()) {
        CheckBlock(if_->False());
    }
}

void Validator::CheckLoop(const Loop* l) {
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

void Validator::CheckSwitch(const Switch* s) {
    control_stack_.Push(s);
    TINT_DEFER(control_stack_.Pop());

    for (auto& cse : s->Cases()) {
        CheckBlock(cse.block);
    }
}

void Validator::CheckTerminator(const Terminator* b) {
    // Note, transforms create `undef` terminator arguments (this is done in MergeReturn and
    // DemoteToHelper) so we can't add validation.

    tint::Switch(
        b,                                                 //
        [&](const ir::BreakIf*) {},                        //
        [&](const ir::Continue*) {},                       //
        [&](const ir::Exit* e) { CheckExit(e); },          //
        [&](const ir::NextIteration*) {},                  //
        [&](const ir::Return* ret) { CheckReturn(ret); },  //
        [&](const ir::TerminateInvocation*) {},            //
        [&](const ir::Unreachable*) {},                    //
        [&](Default) { AddError(b, InstError(b, "missing validation")); });
}

void Validator::CheckExit(const Exit* e) {
    if (e->ControlInstruction() == nullptr) {
        AddError(e, InstError(e, "has no parent control instruction"));
        return;
    }

    if (control_stack_.IsEmpty()) {
        AddError(e, InstError(e, "found outside all control instructions"));
        return;
    }

    auto results = e->ControlInstruction()->Results();
    auto args = e->Args();
    if (results.Length() != args.Length()) {
        AddError(e, InstError(e, std::string("args count (") + std::to_string(args.Length()) +
                                     ") does not match control instruction result count (" +
                                     std::to_string(results.Length()) + ")"));
        AddNote(e->ControlInstruction(), "control instruction");
        return;
    }

    for (size_t i = 0; i < results.Length(); ++i) {
        if (results[i] && args[i] && results[i]->Type() != args[i]->Type()) {
            AddError(
                e, i,
                InstError(e, std::string("argument type (") + results[i]->Type()->FriendlyName() +
                                 ") does not match control instruction type (" +
                                 args[i]->Type()->FriendlyName() + ")"));
            AddNote(e->ControlInstruction(), "control instruction");
        }
    }

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        [&](Default) { AddError(e, InstError(e, "missing validation")); });
}

void Validator::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e, InstError(e, "if target jumps over other control instructions"));
        AddNote(control_stack_.Back(), "first control instruction jumped");
    }
}

void Validator::CheckReturn(const Return* ret) {
    auto* func = ret->Func();
    if (func == nullptr) {
        AddError(ret, InstError(ret, "undefined function"));
        return;
    }
    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->Value()) {
            AddError(ret, InstError(ret, "unexpected return value"));
        }
    } else {
        if (!ret->Value()) {
            AddError(ret, InstError(ret, "expected return value"));
        } else if (ret->Value()->Type() != func->ReturnType()) {
            AddError(ret, InstError(ret, "return value type does not match function return type"));
        }
    }
}

void Validator::CheckControlsAllowingIf(const Exit* exit, const Instruction* control) {
    bool found = false;
    for (auto ctrl : tint::Reverse(control_stack_)) {
        if (ctrl == control) {
            found = true;
            break;
        }
        // A exit switch can step over if instructions, but no others.
        if (!ctrl->Is<ir::If>()) {
            AddError(exit, InstError(exit, std::string(control->FriendlyName()) +
                                               " target jumps over other control instructions"));
            AddNote(ctrl, "first control instruction jumped");
            return;
        }
    }
    if (!found) {
        AddError(exit, InstError(exit, std::string(control->FriendlyName()) +
                                           " not found in parent control instructions"));
    }
}

void Validator::CheckExitSwitch(const ExitSwitch* s) {
    CheckControlsAllowingIf(s, s->ControlInstruction());
}

void Validator::CheckExitLoop(const ExitLoop* l) {
    CheckControlsAllowingIf(l, l->ControlInstruction());

    const Instruction* inst = l;
    const Loop* control = l->Loop();
    while (inst) {
        // Found parent loop
        if (inst->Block()->Parent() == control) {
            if (inst->Block() == control->Continuing()) {
                AddError(l, InstError(l, "loop exit jumps out of continuing block"));
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing(), "in continuing block");
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l, InstError(l, "loop exit not permitted in loop initializer"));
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer(), "in initializer block");
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Validator::CheckStore(const Store* s) {
    CheckOperandsNotNull(s, Store::kToOperandOffset, Store::kFromOperandOffset);

    if (auto* from = s->From()) {
        if (auto* to = s->To()) {
            if (from->Type() != to->Type()->UnwrapPtr()) {
                AddError(s, Store::kFromOperandOffset,
                         "value type does not match pointer element type");
            }
        }
    }
}

void Validator::CheckLoadVectorElement(const LoadVectorElement* l) {
    CheckOperandsNotNull(l,  //
                         LoadVectorElement::kFromOperandOffset,
                         LoadVectorElement::kIndexOperandOffset);

    if (auto* res = l->Result(0)) {
        if (auto* el_ty = GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset)) {
            if (res->Type() != el_ty) {
                AddResultError(l, 0, "result type does not match vector pointer element type");
            }
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    CheckOperandsNotNull(s,  //
                         StoreVectorElement::kToOperandOffset,
                         StoreVectorElement::kValueOperandOffset);

    if (auto* value = s->Value()) {
        if (auto* el_ty = GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset)) {
            if (value->Type() != el_ty) {
                AddError(s, StoreVectorElement::kValueOperandOffset,
                         "value type does not match vector pointer element type");
            }
        }
    }
}

const core::type::Type* Validator::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    if (TINT_UNLIKELY(!operand)) {
        return nullptr;
    }

    auto* type = operand->Type();
    if (TINT_UNLIKELY(!type)) {
        return nullptr;
    }

    auto* vec_ptr_ty = type->As<core::type::Pointer>();
    if (TINT_LIKELY(vec_ptr_ty)) {
        auto* vec_ty = vec_ptr_ty->StoreType()->As<core::type::Vector>();
        if (TINT_LIKELY(vec_ty)) {
            return vec_ty->type();
        }
    }

    AddError(inst, idx, "operand must be a pointer to vector, got " + type->FriendlyName());
    return nullptr;
}

}  // namespace

Result<SuccessType> Validate(const Module& mod) {
    Validator v(mod);
    return v.Run();
}

Result<SuccessType> ValidateAndDumpIfNeeded([[maybe_unused]] const Module& ir,
                                            [[maybe_unused]] const char* msg) {
#if TINT_DUMP_IR_WHEN_VALIDATING
    std::cout << "=========================================================" << std::endl;
    std::cout << "== IR dump before " << msg << ":" << std::endl;
    std::cout << "=========================================================" << std::endl;
    std::cout << Disassemble(ir);
#endif

#ifndef NDEBUG
    auto result = Validate(ir);
    if (result != Success) {
        return result.Failure();
    }
#endif

    return Success;
}

}  // namespace tint::core::ir
