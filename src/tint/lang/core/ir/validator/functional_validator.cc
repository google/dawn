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

#include "src/tint/lang/core/ir/validator/functional_validator.h"

#include <limits>

#include "src/tint/lang/core/ir/array_count.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/multi_in_block.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {
namespace {

/// @returns the parent block of @p block
const Block* ParentBlockOf(const Block* block) {
    if (auto* parent = block->Parent()) {
        return parent->Block();
    }
    return nullptr;
}

/// @returns true if @p block directly or transitively holds the instruction @p inst
bool TransitivelyHolds(const Block* block, const Instruction* inst) {
    for (auto* b = inst->Block(); b; b = ParentBlockOf(b)) {
        if (b == block) {
            return true;
        }
    }
    return false;
}

}  // namespace

Functional::Functional(Module& ir, diag::List& diagnostics, ErrorSource error_source)
    : ir_(ir), diag_(diagnostics), error_source_(error_source), referenced_module_vars_(ir) {}

Functional::~Functional() = default;

void Functional::Validate() {
    for (const Function* func : ir_.functions) {
        CheckFunction(func);
    }
}

Disassembler& Functional::Disassemble() {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);

    if (!disassembler_) {
        disassembler_.emplace(ir::Disassembler(ir_));
    }
    return *disassembler_;
}

bool Functional::IsWGSLValidation() const {
    return error_source_ == ErrorSource::kWgsl;
}

StyledText Functional::NameOf(const core::type::Type* ty) {
    auto name = ty ? ty->FriendlyName() : "undef";
    return StyledText{} << style::Type(name);
}

StyledText Functional::NameOf(const Value* value) {
    if (error_source_ == ErrorSource::kWgsl) {
        return StyledText{} << ir_.NameOf(value).to_str();
    }
    return Disassemble().NameOf(value);
}

Source Functional::SourceOf(const Function* func) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(func);
    }
    return Disassemble().FunctionSource(func);
}

Source Functional::SourceOf(const FunctionParam* param) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(param);
    }
    return Disassemble().FunctionParamSource(param);
}

Source Functional::SourceOf(const Instruction* inst) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst);
    }
    return Disassemble().InstructionSource(inst);
}

Source Functional::SourceOf(const Instruction* inst, size_t idx) {
    if (error_source_ == ErrorSource::kWgsl) {
        return ir_.SourceOf(inst->Operands()[idx]);
    }
    return Disassemble().OperandSource(
        Disassembler::IndexedValue{inst, static_cast<uint32_t>(idx)});
}

diag::Diagnostic& Functional::AddError(Source src) {
    auto& diag = diag_.AddError(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Function* func) {
    return AddError(SourceOf(func));
}

diag::Diagnostic& Functional::AddError(const FunctionParam* param) {
    return AddError(SourceOf(param));
}

diag::Diagnostic& Functional::AddError(const Instruction* inst) {
    auto& diag = AddError(SourceOf(inst));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";
    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference,
        // so we need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddError(const Instruction* inst, size_t idx) {
    auto& diag = AddError(SourceOf(inst, idx));
    if (error_source_ == ErrorSource::kWgsl) {
        return diag;
    }

    diag << inst->FriendlyName() << ": ";

    if (!block_stack_.IsEmpty()) {
        AddNote(block_stack_.Back()) << "in block";

        // Adding the note may trigger a resize and invalidate the error diagnostic reference, so we
        // need to get a new reference to the error diagnostic here.
        return *(diag_.end() - 2);
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(Source src) {
    auto& diag = diag_.AddNote(src);
    if (error_source_ == ErrorSource::kIr) {
        diag.owned_file = Disassemble().File();
    }
    return diag;
}

diag::Diagnostic& Functional::AddNote(const Block* blk) {
    TINT_ASSERT(error_source_ == ErrorSource::kIr);
    auto src = Disassemble().BlockSource(blk);
    return AddNote(src);
}

diag::Diagnostic& Functional::AddNote(const Function* func) {
    return AddNote(SourceOf(func));
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst) {
    return AddNote(SourceOf(inst));
}

diag::Diagnostic& Functional::AddNote(const Instruction* inst, size_t idx) {
    return AddNote(SourceOf(inst, idx));
}

void Functional::CheckFunction(const Function* func) {
    CheckBlock(func->Block());
}

void Functional::CheckBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER({ block_stack_.Pop(); });

    const Instruction* inst = blk->Instructions();
    while (inst != nullptr) {
        CheckInstruction(inst);
        inst = inst->next;
    }
}

void Functional::CheckInstruction(const Instruction* inst) {
    tint::Switch(
        inst,                                              //
        [&](const Access*) {},                             //
        [&](const Binary*) {},                             //
        [&](const Call*) {},                               //
        [&](const If* if_) { CheckIf(if_); },              //
        [&](const Let*) {},                                //
        [&](const Load*) {},                               //
        [&](const LoadVectorElement*) {},                  //
        [&](const Loop* l) { CheckLoop(l); },              //
        [&](const Override*) {},                           //
        [&](const Phony*) {},                              //
        [&](const Store*) {},                              //
        [&](const StoreVectorElement*) {},                 //
        [&](const Switch* s) { CheckSwitch(s); },          //
        [&](const Swizzle*) {},                            //
        [&](const Terminator* b) { CheckTerminator(b); },  //
        [&](const Unary*) {},                              //
        [&](const Var*) {},                                //
        TINT_ICE_ON_NO_MATCH);
}

void Functional::CheckIf(const If* if_) {
    if (!if_->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(if_, If::kConditionOperandOffset) << "condition type must be 'bool'";
    }

    CheckBlock(if_->True());
    CheckBlock(if_->False());
}

void Functional::CheckLoop(const Loop* l) {
    CheckBlock(l->Initializer());
    CheckLoopBody(l);
    CheckLoopContinuing(l);

    first_continues_.Remove(l);
}

void Functional::CheckLoopBody(const Loop* loop) {
    CheckBlock(loop->Body());
}

void Functional::CheckLoopContinuing(const Loop* loop) {
    // Ensure that values used in the loop continuing are not from the loop body, after a continue
    // instruction.
    auto* first_continue = first_continues_.GetOr(loop, nullptr);
    if (first_continue != nullptr) {
        // Find the instruction in the body block that is or holds the first continue instruction.
        const Instruction* holds_continue = first_continue;
        while (holds_continue && holds_continue->Block() &&
               holds_continue->Block() != loop->Body()) {
            holds_continue = holds_continue->Block()->Parent();
        }

        auto check_usage = [&](Usage use) {
            if (TransitivelyHolds(loop->Continuing(), use.instruction)) {
                AddError(use.instruction, use.operand_index)
                    << NameOf(use.instruction->Operands()[use.operand_index])
                    << " cannot be used in continuing block as it is declared after the first "
                    << style::Instruction("continue") << " in the loop's body";
                AddNote(first_continue) << "loop body's first " << style::Instruction("continue");
            }
        };

        // Check that all subsequent instruction values are not used in the continuing block.
        for (auto* inst = holds_continue; inst; inst = inst->next) {
            for (auto* result : inst->Results()) {
                result->ForEachUseUnsorted(check_usage);
            }
        }
    }

    CheckBlock(loop->Continuing());
}

void Functional::CheckTerminator(const Terminator* b) {
    tint::Switch(
        b,                                                 //
        [&](const ir::BreakIf*) {},                        //
        [&](const ir::Continue* c) { CheckContinue(c); },  //
        [&](const ir::Exit*) {},                           //
        [&](const ir::NextIteration*) {},                  //
        [&](const ir::Return*) {},                         //
        [&](const ir::TerminateInvocation*) {},            //
        [&](const ir::Unreachable*) {},                    //
        TINT_ICE_ON_NO_MATCH);
}

void Functional::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    TINT_ASSERT(loop);

    first_continues_.Add(loop, c);
}

void Functional::CheckSwitch(const Switch* s) {
    if (!s->Condition()->Type()->IsIntegerScalar()) {
        auto* cond_ty = s->Condition() ? s->Condition()->Type() : nullptr;
        AddError(s, Switch::kConditionOperandOffset)
            << "condition type " << NameOf(cond_ty) << " must be an integer scalar";
    }

    bool found_default = false;
    for (auto& case_ : s->Cases()) {
        CheckBlock(case_.block);

        for (const auto& sel : case_.selectors) {
            if (sel.IsDefault()) {
                if (found_default) {
                    AddError(s) << "multiple default selectors in switch";
                }
                found_default = true;
            } else if (!sel.val->Type()->IsIntegerScalar()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must be an integer scalar";
            } else if (sel.val->Type() != s->Condition()->Type()) {
                AddError(s) << "case selector type " << NameOf(sel.val->Type())
                            << " must match the switch condition type "
                            << NameOf(s->Condition()->Type());
            }
        }
    }

    if (!found_default) {
        AddError(s) << "missing default case for switch";
    }
}

}  // namespace tint::core::ir::validator
