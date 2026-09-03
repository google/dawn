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

/// @returns true if @p ty meets the basic function parameter rules (i.e. one of constructible,
///          pointer, handle).
///
/// Note: Does not handle corner cases like if certain properties are present.
bool IsValidFunctionParamType(const core::type::Type* ty) {
    if (ty->IsConstructible() || ty->IsHandle()) {
        return true;
    }

    if (auto* ptr = ty->As<core::type::Pointer>()) {
        return ptr->AddressSpace() != core::AddressSpace::kHandle;
    }
    return false;
}

/// @returns true if @p ty is a non-struct and decorated with @builtin(position), or if it is a
/// struct and one of its members is decorated, otherwise false.
/// @param attr attributes attached to data
/// @param ty type of the data being tested
bool IsPositionPresent(const IOAttributes& attr, const core::type::Type* ty) {
    if (auto* ty_struct = ty->As<core::type::Struct>()) {
        for (const auto* mem : ty_struct->Members()) {
            if (mem->Attributes().builtin == BuiltinValue::kPosition) {
                return true;
            }
        }
        return false;
    }

    return attr.builtin == BuiltinValue::kPosition;
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
    CheckEntryPoint(func);

    // void needs to be filtered out, since it isn't constructible, but used in the IR when no
    // return is specified.
    if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>() &&
                      !func->ReturnType()->IsConstructible())) {
        AddError(func) << "function return type must be constructible";
    }

    for (auto* param : func->Params()) {
        CheckFunctionParam(param);
    }

    CheckBlock(func->Block());
}

void Functional::CheckEntryPoint(const Function* func) {
    if (!func->IsEntryPoint()) {
        return;
    }

    // Check that there is at most one entry point unless we allow multiple entry points.
    if (!ir_.properties.Contains(Property::kAllowMultipleEntryPoints) &&
        !entry_point_names_.IsEmpty()) {
        AddError(func) << "a module with multiple entry points requires the "
                          "AllowMultipleEntryPoints property";
        return;
    }

    if (DAWN_UNLIKELY(ir_.NameOf(func).Name().empty())) {
        AddError(func) << "entry points must have names";
    } else {
        // Checking the name early, so its usage can be recorded, even if the function is
        // malformed.
        const auto name = ir_.NameOf(func).Name();
        if (!entry_point_names_.Add(name)) {
            AddError(func) << "entry point name " << style::Function(name) << " is not unique";
        }
    }

    Hashset<BindingPoint, 4> binding_points{};
    bool seen_immediate = false;
    for (auto var : referenced_module_vars_.TransitiveReferences(func)) {
        if (!ir_.properties.Contains(Property::kAllowDuplicateBindings) &&
            var->BindingPoint().has_value()) {
            auto bp = var->BindingPoint().value();
            if (!binding_points.Add(bp)) {
                AddError(var) << "found non-unique binding point, " << bp
                              << ", being referenced in entry point, " << NameOf(func);
            }
        }

        const auto* mv = var->Result()->Type()->As<core::type::MemoryView>();
        if (!mv) {
            continue;
        }

        auto address_space = mv->AddressSpace();
        switch (address_space) {
            case AddressSpace::kImmediate:
                if (seen_immediate) {
                    AddError(var) << "multiple user-declared immediate data variables referenced "
                                     "by entry point "
                                  << NameOf(func);
                }
                seen_immediate = true;
                continue;
            case AddressSpace::kWorkgroup:
                if (!func->IsCompute()) {
                    AddError(var) << "workgroup variable cannot be used in a " << func->Stage()
                                  << " shader";
                }
                continue;
            case AddressSpace::kPixelLocal:
                if (!func->IsFragment()) {
                    AddError(var) << "pixel_local variable cannot be used in a " << func->Stage()
                                  << " shader";
                }
                continue;
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                break;
            default:
                continue;
        }
    }

    if (func->IsCompute()) {
        if (DAWN_UNLIKELY(!func->ReturnType()->Is<core::type::Void>())) {
            AddError(func) << "compute entry point must not have a return type, found "
                           << NameOf(func->ReturnType());
        }
    } else if (func->IsVertex()) {
        CheckPositionPresentForVertexOutput(func);
    }
}

void Functional::CheckPositionPresentForVertexOutput(const Function* ep) {
    if (IsPositionPresent(ep->ReturnAttributes(), ep->ReturnType())) {
        return;
    }

    for (const auto& var : referenced_module_vars_.TransitiveReferences(ep)) {
        const auto* ty = var->Result()->Type()->UnwrapPtrOrRef();
        if (!ty) {
            continue;
        }

        const auto attr = var->Attributes();
        if (IsPositionPresent(attr, ty)) {
            if (!ir_.properties.Contains(Property::kAllowBackendSpecificShaderIO)) {
                AddError(var) << "position as part of a `var`, it must be part of the return";
                AddNote(ep) << "used in entry point here";
                return;
            }
            return;
        }
    }
    AddError(ep) << "position must be declared on the return of a vertex entry point";
}

void Functional::CheckFunctionParam(const FunctionParam* param) {
    TINT_ASSERT(param->Function() != nullptr);

    bool func_is_entry_point = param->Function()->IsEntryPoint();

    if (!IsValidFunctionParamType(param->Type())) {
        auto ptr_ty = param->Type()->As<core::type::Pointer>();
        bool allowed_ptr_to_handle = ir_.properties.Contains(Property::kAllowPointerToHandle) &&
                                     ptr_ty != nullptr && ptr_ty->StoreType()->IsHandle();

        auto struct_ty = param->Type()->As<core::type::Struct>();
        if (!allowed_ptr_to_handle &&
            (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
             (struct_ty == nullptr) ||
             struct_ty->Members().Any([](const core::type::StructMember* m) {
                 return !IsValidFunctionParamType(m->Type());
             }))) {
            AddError(param) << "function parameter type, " << NameOf(param->Type())
                            << ", must be constructible, a pointer, or a handle";
        }
    }

    AddressSpace address_space = AddressSpace::kUndefined;
    auto* mv = param->Type()->As<core::type::MemoryView>();
    if (mv) {
        address_space = mv->AddressSpace();
    } else {
        // ModuleScopeVars transform in MSL backends unwraps pointers to handles
        if (param->Type()->IsHandle()) {
            address_space = AddressSpace::kHandle;
        }
    }

    if (address_space == AddressSpace::kPixelLocal) {
        if (!mv->StoreType()->Is<core::type::Struct>()) {
            AddError(param) << "pixel_local param must be of type struct";
        }
    }

    if (func_is_entry_point && !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
        if (param->Type()->Is<core::type::Pointer>()) {
            AddError(param) << "entry point parameters cannot be pointers";
        }

        if (mv && mv->Is<core::type::Pointer>() && address_space == AddressSpace::kWorkgroup) {
            AddError(param) << "input param to entry point cannot be a ptr in the 'workgroup' "
                               "address space";
        }
    }
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
