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
#include <utility>

#include "src/tint/lang/core/intrinsic/dialect.h"
#include "src/tint/lang/core/ir/array_count.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/multi_in_block.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/memory_view.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/swizzle_view.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/internal_limits.h"
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

const constant::Value* GetConstArg(const CoreBuiltinCall* call, uint32_t param_index) {
    if ((call->Args().size() <= param_index) || (call->Args()[param_index] == nullptr) ||
        (!call->Args()[param_index]->Is<ir::Constant>())) {
        return nullptr;
    }
    return call->Args()[param_index]->As<ir::Constant>()->Value();
}

}  // namespace

Functional::Functional(Module& ir, diag::List& diagnostics, ErrorSource error_source)
    : ir_(ir),
      diag_(diagnostics),
      error_source_(error_source),
      const_eval_(ir_.constant_values, diag_),
      referenced_module_vars_(ir) {}

Functional::~Functional() = default;

void Functional::Validate() {
    CheckRootBlock(ir_.root_block);

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

void Functional::CheckRootBlock(const Block* blk) {
    block_stack_.Push(blk);
    TINT_DEFER({
        block_stack_.Pop();
        TINT_ASSERT(block_stack_.IsEmpty());
    });

    for (auto* inst : *blk) {
        if (auto* var = inst->As<Var>()) {
            CheckBuffersAndMatrices(var);
        }
        CheckInstruction(inst);
    }
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
        [&](const Access* a) { CheckAccess(a); },          //
        [&](const Binary* b) { CheckBinary(b); },          //
        [&](const Call* c) { CheckCall(c); },              //
        [&](const If* if_) { CheckIf(if_); },              //
        [&](const Let*) {},                                //
        [&](const Load*) {},                               //
        [&](const LoadVectorElement*) {},                  //
        [&](const Loop* l) { CheckLoop(l); },              //
        [&](const Override* o) { CheckOverride(o); },      //
        [&](const Phony*) {},                              //
        [&](const Store*) {},                              //
        [&](const StoreVectorElement*) {},                 //
        [&](const Switch* s) { CheckSwitch(s); },          //
        [&](const Swizzle* s) { CheckSwizzle(s); },        //
        [&](const Terminator* b) { CheckTerminator(b); },  //
        [&](const Unary* u) { CheckUnary(u); },            //
        [&](const Var*) {},                                //
        TINT_ICE_ON_NO_MATCH);

    // Only check alignment if the instruction passed previous checks.
    if (!diag_.ContainsErrors()) {
        CheckAlignment(inst);
    }
}

void Functional::CheckAlignment(const Instruction* inst) {
    auto align = inst->Alignment();
    if (align.has_value()) {
        if (inst->GetSideEffects().Size() == 0) {
            AddError(inst) << "alignment can only be set on memory instructions";
        }

        auto align_val = align.value();
        if (!tint::IsPowerOfTwo(align_val)) {
            AddError(inst) << "alignment (" << align_val << ") must be a power of 2";
        }

        if (align_val > 256) {
            AddError(inst) << "alignment (" << align_val << ") must be less than or equal to 256";
        }

        // TODO(b/544359162): Which other instructions should be checked?
        uint32_t natural_align = tint::Switch(
            inst,  //
            [&](const Load* ld) { return ld->Result()->Type()->Align(); },
            [&](const Store* st) { return st->From()->Type()->Align(); },
            [&](const LoadVectorElement* lve) { return lve->Result()->Type()->Align(); },
            [&](const StoreVectorElement* sve) { return sve->Value()->Type()->Align(); },
            [&](const CoreBuiltinCall* call) {
                switch (call->Func()) {
                    case BuiltinFn::kSubgroupMatrixLoad:
                    case BuiltinFn::kSubgroupMatrixStore:
                        return call->Args()[0]->Type()->UnwrapPtr()->Align();
                    default:
                        return 0u;
                }
            },
            [&](Default) { return 0u; });

        if (align_val <= natural_align) {
            AddError(inst) << "alignment (" << align_val
                           << ") must be greater than natural alignment (" << natural_align << ")";
        }
    }
}

void Functional::CheckOverride(const Override* o) {
    if (o->OverrideId().has_value()) {
        if (!seen_override_ids_.Add(o->OverrideId().value())) {
            AddError(o) << "duplicate override id encountered: " << o->OverrideId().value().value;
            return;
        }
    }

    if (!o->Result()->Type()->IsScalar()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type()) << " is not a scalar";
        return;
    }

    if (o->Initializer() && o->Initializer()->Type() != o->Result()->Type()) {
        AddError(o) << "override type " << NameOf(o->Result()->Type())
                    << " does not match initializer type " << NameOf(o->Initializer()->Type());
        return;
    }

    if (!o->OverrideId().has_value() && (o->Initializer() == nullptr)) {
        AddError(o) << "must have an id or an initializer";
        return;
    }
}

void Functional::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const Construct*) {},                                        //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard*) {},                                          //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const UserCall*) {},                                         //
        [&](Default) { /* Validation of custom IR instructions */ });
}

void Functional::CheckAccess(const Access* a) {
    auto* obj_view = a->Object()->Type()->As<core::type::MemoryView>();
    auto* ty = obj_view ? obj_view->StoreType() : a->Object()->Type();

    enum Kind : uint8_t {
        kPtr,
        kRef,
        kValue,
    };
    const Kind in_kind = tint::Switch(
        a->Object()->Type(),                                 //
        [&](const core::type::Pointer*) { return kPtr; },    //
        [&](const core::type::Reference*) { return kRef; },  //
        [&](Default) { return kValue; });

    auto desc_of = [&](Kind kind, const core::type::Type* type) {
        switch (kind) {
            case kPtr:
                return StyledText{}
                       << style::Type("ptr<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            case kRef:
                return StyledText{}
                       << style::Type("ref<", obj_view->AddressSpace(), ", ", type->FriendlyName(),
                                      ", ", obj_view->Access(), ">");
            default:
                return NameOf(type);
        }
    };

    for (size_t i = 0; i < a->Indices().size(); i++) {
        auto err = [&]() -> diag::Diagnostic& {
            return AddError(a, i + Access::kIndicesOperandOffset);
        };

        auto* index = a->Indices()[i];
        if (DAWN_UNLIKELY((!index->Type()->IsAnyOf<core::type::I32, core::type::U32>()))) {
            err() << "index type " << NameOf(index->Type()) << " must be i32 or u32";
            return;
        }

        if (!ir_.properties.Contains(Property::kAllowVectorElementPointer)) {
            if (in_kind != kValue && ty->Is<core::type::Vector>()) {
                err() << "cannot obtain address of vector element";
                return;
            }
        }

        if (auto* const_index = index->As<ir::Constant>()) {
            auto* value = const_index->Value();
            if (value->Type()->IsSignedIntegerScalar()) {
                // index is a signed integer scalar. Check that the index isn't negative.
                // If the index is unsigned, we can skip this.
                auto idx = value->ValueAs<AInt>();
                if (DAWN_UNLIKELY(idx < 0)) {
                    err() << "constant index must be positive, got " << idx;
                    return;
                }
            }

            auto idx = value->ValueAs<uint32_t>();
            auto* el = ty->Element(idx);
            if (DAWN_UNLIKELY(!el)) {
                // Is index in bounds?
                if (auto el_count = ty->Elements().count; el_count != 0 && idx >= el_count) {
                    err() << "index out of bounds for type " << desc_of(in_kind, ty);
                    AddNote(a, i + Access::kIndicesOperandOffset)
                        << "acceptable range: [0.." << (el_count - 1) << "]";
                    return;
                }
                err() << "type " << desc_of(in_kind, ty) << " cannot be indexed";
                return;
            }
            ty = el;
        } else {
            auto* el = ty->Elements().type;
            if (DAWN_UNLIKELY(!el)) {
                err() << "type " << desc_of(in_kind, ty) << " cannot be dynamically indexed";
                return;
            }
            ty = el;
        }
    }

    auto* want = a->Result()->Type();
    auto* want_view = want->As<core::type::MemoryView>();
    bool ok = true;
    if (obj_view) {
        // Pointer source always means pointer result.
        ok = (want_view != nullptr) && ty == want_view->StoreType();
        if (ok) {
            // Also check that the address space and access modes match.
            bool base_is_ptr = obj_view->IsAnyOf<core::type::Pointer, core::type::SwizzleView>();
            ok =
                base_is_ptr == want_view->IsAnyOf<core::type::Pointer, core::type::SwizzleView>() &&
                obj_view->AddressSpace() == want_view->AddressSpace() &&
                obj_view->Access() == want_view->Access();
        }
    } else {
        // Otherwise, result types should exactly match.
        ok = ty == want;
    }
    if (DAWN_UNLIKELY(!ok)) {
        AddError(a) << "result of access chain is type " << desc_of(in_kind, ty)
                    << " but instruction type is " << NameOf(want);
    }
}

void Functional::CheckBinary(const Binary* b) {
    intrinsic::Context context{b->TableData(), type_mgr_, symbols_};

    auto overload =
        core::intrinsic::LookupBinary(context, b->Op(), b->LHS()->Type(), b->RHS()->Type(),
                                      core::EvaluationStage::kRuntime, /* is_compound */ false);
    if (overload != Success) {
        AddError(b) << overload.Failure();
        return;
    }

    auto* result = b->Result(0);
    TINT_ASSERT(result);

    if (overload->return_type != result->Type()) {
        AddError(b) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(b->Op()) << " result type "
                    << NameOf(overload->return_type);
    }

    if (auto* c = b->As<CoreBinary>()) {
        CheckCoreBinaryCall(c);
    }
}

void Functional::CheckCoreBinaryCall(const CoreBinary* call) {
    switch (call->Op()) {
        case core::BinaryOp::kDivide:
        case core::BinaryOp::kModulo:
            CheckBinaryDivModCall(call);
            break;
        case core::BinaryOp::kShiftLeft:
        case core::BinaryOp::kShiftRight:
            CheckBinaryShiftCall(call);
            break;
        default:
            break;
    }
}

void Functional::CheckBinaryDivModCall(const CoreBinary* call) {
    if (error_source_ == ErrorSource::kWgsl) {
        // Integer division by zero should be checked for the partial evaluation case (only rhs
        // is const). FP division by zero is only invalid when the whole expression is
        // constant-evaluated.
        if (call->RHS()->Type()->IsIntegerScalarOrVector()) {
            auto rhs_constant = call->RHS()->As<ir::Constant>();
            if (rhs_constant && rhs_constant->Value()->AnyZero()) {
                AddError(call) << "integer division by zero is invalid";
            }
        }
    }
}

void Functional::CheckBinaryShiftCall(const CoreBinary* call) {
    if (error_source_ == ErrorSource::kWgsl) {
        // If lhs value is a concrete type, and rhs is a const-expression greater than or equal
        // to the bit width of lhs, then it is a shader-creation error.
        const auto* elem_type = call->LHS()->Type()->DeepestElement();
        const uint32_t bit_width = elem_type->Size() * 8;
        if (auto* rhs_val_as_const = call->RHS()->As<ir::Constant>()) {
            auto* rhs_as_value = rhs_val_as_const->Value();
            for (size_t i = 0, n = rhs_as_value->NumElements(); i < n; i++) {
                auto* shift_val = n == 1 ? rhs_as_value : rhs_as_value->Index(i);
                if (shift_val->ValueAs<u32>() >= bit_width) {
                    AddError(call)
                        << "shift " << (call->Op() == core::BinaryOp::kShiftLeft ? "left" : "right")
                        << " value must be less than the bit width of the lhs, which is "
                        << bit_width;
                    break;
                }
            }
        }
    }
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

void Functional::CheckSwizzle(const Swizzle* s) {
    auto* result_ty = s->Result()->Type();

    auto* obj_ty = s->Object()->Type();
    auto* src_mv = obj_ty->As<core::type::MemoryView>();
    auto* src_vec =
        src_mv ? src_mv->StoreType()->As<core::type::Vector>() : obj_ty->As<core::type::Vector>();
    if (!src_vec) {
        AddError(s) << "object of swizzle, " << NameOf(s->Object()) << ", is not a vector, "
                    << NameOf(s->Object()->Type());
        return;
    }

    auto indices = s->Indices();
    if (indices.Length() < Swizzle::kMinNumIndices) {
        AddError(s) << "expected at least " << Swizzle::kMinNumIndices << " indices";
        return;
    }

    if (indices.Length() > Swizzle::kMaxNumIndices) {
        AddError(s) << "expected at most " << Swizzle::kMaxNumIndices << " indices";
        return;
    }

    auto elem_count = src_vec->Elements().count;
    for (auto& idx : indices) {
        if (idx > Swizzle::kMaxIndexValue || idx >= elem_count) {
            AddError(s) << "invalid index value";
            return;
        }
    }

    auto* elem_ty = src_vec->Elements().type;
    auto* expected_store_ty = type_mgr_.MatchWidth(elem_ty, indices.Length());
    const core::type::Type* expected_ty = nullptr;
    if (src_mv) {
        expected_ty = type_mgr_.Get<core::type::SwizzleView>(
            src_mv->AddressSpace(), expected_store_ty, src_mv->Access(), src_vec->Width(),
            static_cast<uint32_t>(indices.Length()));
    } else {
        expected_ty = expected_store_ty;
    }

    if (result_ty != expected_ty) {
        AddError(s) << "result type " << NameOf(result_ty) << " does not match expected type, "
                    << NameOf(expected_ty);
        return;
    }
}

void Functional::CheckUnary(const Unary* u) {
    TINT_ASSERT(u->Val());

    intrinsic::Context context{u->TableData(), type_mgr_, symbols_};
    auto overload = core::intrinsic::LookupUnary(context, u->Op(), u->Val()->Type(),
                                                 core::EvaluationStage::kRuntime);
    if (overload != Success) {
        AddError(u) << overload.Failure();
        return;
    }

    const Value* result = u->Result(0);
    if (overload->return_type != result->Type()) {
        AddError(u) << "result value type " << NameOf(result->Type()) << " does not match "
                    << style::Instruction(u->Op()) << " result type "
                    << NameOf(overload->return_type);
    }
}

void Functional::CheckBuiltinCall(const BuiltinCall* call) {
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    TINT_ASSERT(builtin->return_type);

    if (builtin->return_type != call->Result()->Type()) {
        AddError(call) << "call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(builtin->return_type);
        return;
    }

    // Check evaluation stage of parameters that are required to be const-expressions.
    for (uint32_t i = 0; i < builtin->parameters.Length(); i++) {
        const auto& p = builtin->parameters[i];
        const auto* arg = call->Args()[i];
        if (p.is_const && !arg->Is<Constant>()) {
            AddError(call, BuiltinCall::kArgsOperandOffset + i)
                << "the " << style::Variable(p.usage) << " argument must be a constant";
            return;
        }
    }

    const CoreBuiltinCall* bc = call->As<CoreBuiltinCall>();
    if (bc == nullptr) {
        return;
    }

    CheckCoreBuiltinCall(bc, builtin.Get());
}

void Functional::CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                                      const core::intrinsic::Overload& overload) {
    auto idx_for_usage = [&](core::ParameterUsage usage) -> std::optional<uint32_t> {
        for (uint32_t i = 0; i < overload.parameters.Length(); ++i) {
            auto& p = overload.parameters[i];
            if (p.usage == usage) {
                return i;
            }
        }
        return std::nullopt;
    };

    auto check_arg_in_range = [&](core::ParameterUsage usage, int32_t min, int32_t max) {
        auto idx_opt = idx_for_usage(usage);
        if (!idx_opt.has_value()) {
            return;
        }
        uint32_t idx = idx_opt.value();
        TINT_ASSERT(idx < call->Args().size());

        auto* val = call->Args()[idx];
        auto* const_val = val->As<ir::Constant>();
        TINT_ASSERT(const_val);
        auto* cnst = const_val->Value();

        if (val->Type()->Is<core::type::Vector>()) {
            for (size_t i = 0; i < cnst->NumElements(); i++) {
                auto value = cnst->Index(i)->ValueAs<int32_t>();
                if (value < min || value > max) {
                    AddError(call, idx)
                        << value << " outside range of [" << min << ", " << max << "]";
                    return;
                }
            }
        } else {
            auto value = cnst->ValueAs<int32_t>();
            if (value < min || value > max) {
                AddError(call, idx) << value << " outside range of [" << min << ", " << max << "]";
                return;
            }
        }
    };

    if (core::IsTexture(call->Func())) {
        check_arg_in_range(core::ParameterUsage::kComponent, 0, 3);
        check_arg_in_range(core::ParameterUsage::kOffset, -8, 7);
    }

    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad ||
        call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        CheckSubgroupMatrixOpOffset(call);
    }

    if (IsWGSLValidation()) {
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffle:
            case core::BuiltinFn::kSubgroupShuffleXor:
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                CheckSubgroupCall(call);
                break;
            case core::BuiltinFn::kExtractBits:
                CheckExtractBitsCall(call);
                break;
            case core::BuiltinFn::kInsertBits:
                CheckInsertBitsCall(call);
                break;
            case core::BuiltinFn::kLdexp:
                CheckLdexpCall(call);
                break;
            case core::BuiltinFn::kClamp:
                CheckClampCall(call);
                break;
            case core::BuiltinFn::kSmoothstep:
                CheckSmoothstepCall(call);
                break;
            case core::BuiltinFn::kQuantizeToF16:
                CheckQuantizeToF16(call);
                break;
            case core::BuiltinFn::kPack2X16Float:
                CheckPack2x16float(call);
                break;
            default:
                break;
        }
    }
}

void Functional::CheckSubgroupCall(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 1)) {
        auto as_aint = const_val->ValueAs<AInt>();
        // User friendly param name.
        std::string paramName = "sourceLaneIndex";
        switch (call->Func()) {
            case core::BuiltinFn::kSubgroupShuffleXor:
                paramName = "mask";
                break;
            case core::BuiltinFn::kSubgroupShuffleUp:
            case core::BuiltinFn::kSubgroupShuffleDown:
                paramName = "delta";
                break;
            default:
                break;
        }

        if (as_aint >= tint::internal_limits::kMaxSubgroupSize) {
            AddError(call, 1) << "The " << paramName << " argument of " << call->FriendlyName()
                              << " must be less than " << tint::internal_limits::kMaxSubgroupSize;
        } else if (as_aint < 0) {
            AddError(call, 1) << "The " << paramName << " argument of " << call->FriendlyName()
                              << " must be greater than or equal to zero";
        }
    }
}

void Functional::CheckExtractBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 1);
    auto* const_val_count = GetConstArg(call, 2);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.extractBits(param0->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Functional::CheckInsertBitsCall(const CoreBuiltinCall* call) {
    // This can be u32/i32 or vector of those types.
    auto* param0 = call->Args()[0];
    auto* const_val_offset = GetConstArg(call, 2);
    auto* const_val_count = GetConstArg(call, 3);
    if (const_val_count && const_val_offset) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, zero, const_val_offset, const_val_count};
        [[maybe_unused]] auto result =
            const_eval_.insertBits(param0->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Functional::CheckLdexpCall(const CoreBuiltinCall* call) {
    auto* param0 = call->Args()[0];
    if (auto const_val = GetConstArg(call, 1)) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val};
        [[maybe_unused]] auto result =
            const_eval_.ldexp(param0->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Functional::CheckQuantizeToF16(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.quantizeToF16(
            call->Result()->Type(), Vector{const_val}, ir_.SourceOf(call));
    }
}

void Functional::CheckPack2x16float(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.pack2x16float(
            call->Result()->Type(), Vector{const_val}, ir_.SourceOf(call));
    }
}

void Functional::CheckClampCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 1);
    auto* const_val_high = GetConstArg(call, 2);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_low, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.clamp(call->Result()->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Functional::CheckSmoothstepCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 0);
    auto* const_val_high = GetConstArg(call, 1);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_high, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.smoothstep(call->Result()->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Functional::CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call) {
    const Value* p_arg = call->Args()[0];
    const Value* offset_arg = call->Args()[1];

    const core::type::Pointer* ptr_ty = p_arg->Type()->As<core::type::Pointer>();
    TINT_ASSERT(ptr_ty);

    const core::type::Array* arr_ty = ptr_ty->StoreType()->As<core::type::Array>();
    TINT_ASSERT(arr_ty);

    auto const_count = arr_ty->ConstantCount();
    if (!const_count.has_value()) {
        return;
    }

    const core::type::SubgroupMatrix* mat_ty = nullptr;
    if (call->Func() == core::BuiltinFn::kSubgroupMatrixLoad) {
        mat_ty = call->Result()->Type()->As<core::type::SubgroupMatrix>();
    } else if (call->Func() == core::BuiltinFn::kSubgroupMatrixStore) {
        mat_ty = call->Args()[2]->Type()->As<core::type::SubgroupMatrix>();
    }
    TINT_ASSERT(mat_ty);

    auto mat_comp_size = mat_ty->Type()->Size();
    TINT_ASSERT(mat_comp_size > 0);

    auto limit = const_count.value();

    if (auto* offset_const = offset_arg->As<ir::Constant>()) {
        auto* offset_val = offset_const->Value();
        uint32_t offset = 0;
        if (offset_arg->Type()->IsUnsignedIntegerScalar()) {
            offset = offset_val->ValueAs<u32>();
        } else if (offset_arg->Type()->IsSignedIntegerScalar()) {
            auto ival = offset_val->ValueAs<i32>();
            if (ival < 0) {
                AddError(call, 1) << "the offset argument of " << call->Func()
                                  << " must be non-negative";
                return;
            }
            offset = static_cast<uint32_t>(ival);
        }

        if (offset >= limit) {
            AddError(call, 1) << "the offset argument of " << call->Func() << " (" << offset
                              << ") is out of bounds of the array type of size " << limit;
        }
    }
}

void Functional::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });
    args.Insert(0, call->Object()->Type());

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto result = core::intrinsic::LookupMemberFn(context, call->FriendlyName().c_str(),
                                                  call->FuncId(), call->ExplicitTemplateParams(),
                                                  std::move(args), core::EvaluationStage::kRuntime);
    if (result != Success) {
        AddError(call) << result.Failure();
        return;
    }

    if (result->return_type != call->Result()->Type()) {
        // Note: This is not currently tested in core unittests as there are no concrete
        // MemberBuiltinCall implementations in core IR. This is tested by backend-specific
        // (e.g. HLSL) validation tests.
        AddError(call) << "member call result type " << NameOf(call->Result()->Type())
                       << " does not match builtin return type " << NameOf(result->return_type);
    }
}

void Functional::CheckConvert(const Convert* convert) {
    auto* result_type = convert->Result()->Type();
    auto* value_type = convert->Operand(Convert::kValueOperandOffset)->Type();

    intrinsic::CtorConv conv_ty;
    Vector<TemplateParameter, 1> template_type;
    tint::Switch(
        result_type,                                                             //
        [&](const core::type::I32*) { conv_ty = intrinsic::CtorConv::kI32; },    //
        [&](const core::type::U32*) { conv_ty = intrinsic::CtorConv::kU32; },    //
        [&](const core::type::U64*) { conv_ty = intrinsic::CtorConv::kU64; },    //
        [&](const core::type::F32*) { conv_ty = intrinsic::CtorConv::kF32; },    //
        [&](const core::type::F16*) { conv_ty = intrinsic::CtorConv::kF16; },    //
        [&](const core::type::Bool*) { conv_ty = intrinsic::CtorConv::kBool; },  //
        [&](const core::type::Vector* v) {
            conv_ty = intrinsic::VectorCtorConv(v->Width());
            template_type.Push(v->Type());
        },
        [&](const core::type::Matrix* m) {
            conv_ty = intrinsic::MatrixCtorConv(m->Columns(), m->Rows());
            template_type.Push(m->Type());
        },
        [&](Default) { conv_ty = intrinsic::CtorConv::kNone; });

    if (conv_ty == intrinsic::CtorConv::kNone) {
        AddError(convert) << "not defined for result type, " << NameOf(result_type);
        return;
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto match =
        table.Lookup(conv_ty, template_type, Vector{value_type}, core::EvaluationStage::kOverride);
    if (match != Success || !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConverter)) {
        AddError(convert) << "No defined converter for " << NameOf(value_type) << " -> "
                          << NameOf(result_type);
        return;
    }
}

void Functional::CheckBuffersAndMatrices(const Var* var) {
    if (error_source_ != ErrorSource::kWgsl) {
        return;
    }

    uint32_t var_size = 0;
    if (var->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
        var_size = var->Result()->Type()->UnwrapPtr()->Size();
    }

    Vector<UseInfo, 4> uses;
    for (auto& u : var->Result()->UsagesSorted()) {
        uses.Push({u, var_size, 0, 0});
    }
    while (!uses.IsEmpty()) {
        auto info = uses.Pop();
        diag::Diagnostic error;
        bool errored = tint::Switch(
            info.use.instruction,
            [&](const Let* let) {
                for (auto& u : let->Result()->UsagesSorted()) {
                    uses.Push({u, info.storage_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const UserCall* user) {
                // If the buffer size is decreased at a function boundary, use that size
                // instead.
                auto* target = user->Target();
                auto* param = target->Params()[info.use.operand_index - user->ArgsOperandOffset()];
                auto* param_buffer_ty = param->Type()->UnwrapPtr()->As<core::type::Buffer>();
                uint32_t next_size = param_buffer_ty && param_buffer_ty->Size() > 0
                                         ? param_buffer_ty->Size()
                                         : info.storage_size;
                for (auto& u : param->UsagesSorted()) {
                    uses.Push({u, next_size, info.offset, info.pointer_size});
                }
                return false;
            },
            [&](const CoreBuiltinCall* call) {
                if (call->Func() == BuiltinFn::kBufferView ||
                    call->Func() == BuiltinFn::kBufferArrayView) {
                    if (!CheckBufferView(call, var, info.storage_size)) {
                        return true;
                    }

                    uint32_t offset = 0;
                    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
                        offset = const_offset->Value()->ValueAs<uint32_t>();
                    }
                    uint32_t pointer_size = 0;
                    if (call->Func() == BuiltinFn::kBufferArrayView) {
                        if (auto* const_size = call->Args()[2]->As<Constant>()) {
                            pointer_size = const_size->Value()->ValueAs<uint32_t>();
                        }
                    } else if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                        // Use the bufferView result size if it has a fixed size.
                        pointer_size = call->Result()->Type()->UnwrapPtr()->Size();
                    }

                    // Keep tracing to catch subgroupMatrixLoad/Store transitive uses.
                    for (auto& u : call->Result()->UsagesSorted()) {
                        uses.Push({u, info.storage_size, offset, pointer_size});
                    }
                } else if (call->Func() == BuiltinFn::kSubgroupMatrixLoad ||
                           call->Func() == BuiltinFn::kSubgroupMatrixStore) {
                    if (!CheckSubgroupMatrixMemory(call, var, info)) {
                        return true;
                    }
                }

                return false;
            },
            [&](const Access* access) {
                auto* obj_ty = access->Object()->Type()->UnwrapPtr();

                uint32_t offset = 0;
                for (auto* idx : access->Indices()) {
                    uint32_t idx_value = 0;
                    if (auto* const_idx = idx->As<Constant>()) {
                        idx_value = const_idx->Value()->ValueAs<uint32_t>();
                    }
                    // Matrix and vector can't be hit on the way to a subgroupMatrix and access
                    // won't be hit at all on the way to buffer[Array]View so we only handle
                    // structure and array here.
                    tint::Switch(
                        obj_ty,  //
                        [&](const core::type::Array* ary) {
                            obj_ty = ary->ElemType();
                            offset += idx_value * ary->ImplicitStride();
                        },
                        [&](const core::type::Struct* s) {
                            auto* mem = s->Members()[idx_value];
                            obj_ty = mem->Type();
                            offset += mem->Offset();
                        },
                        [&](Default) {});
                }

                uint32_t pointer_size = info.pointer_size;
                if (access->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    // If the result has a fixed size, update pointer size.
                    pointer_size = access->Result()->Type()->UnwrapPtr()->Size();
                }

                for (auto& u : access->Result()->UsagesSorted()) {
                    // Accumulate the offset.
                    uses.Push({u, info.storage_size, info.offset + offset, pointer_size});
                }

                return false;
            },
            [&](Default) { return false; });
        if (errored) {
            return;
        }
    }
}

bool Functional::CheckBufferView(const CoreBuiltinCall* call,
                                 const Var* var,
                                 uint32_t buffer_size) {
    // Calculate the minimum type size.
    auto* store_ty = call->Result()->Type()->UnwrapPtr();
    uint64_t ty_required_size = 0;
    uint64_t ty_offset = 0;
    uint64_t ty_stride = 0;
    if (store_ty->HasFixedFootprint()) {
        ty_required_size = store_ty->Size();
    } else if (auto* str = store_ty->As<core::type::Struct>()) {
        auto* last = str->Members().Back();
        auto* arr_ty = last->Type()->As<core::type::Array>();
        ty_offset = last->Offset();
        ty_stride = arr_ty->ImplicitStride();
        ty_required_size = ty_offset + ty_stride;
    } else {
        ty_stride = store_ty->As<core::type::Array>()->ImplicitStride();
        ty_required_size = ty_stride;
    }

    // Error conditions:
    // For both bufferView and bufferArrayView:
    // * ty_required_size + offset < buffer_size
    // * offset % store_ty->Align() != 0
    // For bufferArrayView
    // * size + offset < buffer_size
    // * size < ty_required_size
    // * (size - offset) % stride != 0
    //
    // Also error if any addition overflows a uint32_t.

    uint64_t offset_val = 0;
    if (auto* const_offset = call->Args()[1]->As<Constant>()) {
        if (const_offset->Type()->IsSignedIntegerScalar()) {
            if (const_offset->Value()->ValueAs<int32_t>() < 0) {
                AddError(call) << call->FriendlyName() << " offset must be greater than 0";
                return false;
            }
        }
        offset_val = const_offset->Value()->ValueAs<uint64_t>();
    }

    if (offset_val + ty_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < offset_val + ty_required_size) {
        AddError(var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << offset_val + ty_required_size
                      << " bytes required)";
        return false;
    }

    if (offset_val % store_ty->Align() != 0) {
        AddError(call) << call->FriendlyName() << " offset (" << offset_val
                       << " bytes) must be a multiple of result alignment (" << store_ty->Align()
                       << " bytes)";
        return false;
    }

    if (call->Func() == BuiltinFn::kBufferView) {
        return true;
    }

    uint64_t size_val = 0;
    if (auto* const_size = call->Args()[2]->As<Constant>()) {
        if (const_size->Type()->IsSignedIntegerScalar()) {
            if (const_size->Value()->ValueAs<int32_t>() < 0) {
                AddError(call) << call->FriendlyName() << " size must be greater than 0";
                return false;
            }
        }
        size_val = const_size->Value()->ValueAs<uint64_t>();
        if (size_val == 0) {
            AddError(call) << call->FriendlyName() << " cannot be 0 sized";
            return false;
        }
    }

    if (offset_val + size_val > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " requires a size beyond 32 bits";
        return false;
    }

    if (buffer_size > 0 && buffer_size < size_val + offset_val) {
        AddError(var) << "invalid buffer size (" << buffer_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << size_val + offset_val
                      << " bytes required)";
        return false;
    }

    if (size_val > 0 && size_val < ty_required_size) {
        AddError(call) << call->FriendlyName() << " has invalid size (" << size_val
                       << " bytes, requires " << ty_required_size << " bytes)";
        return false;
    }

    if (size_val > 0 && ((size_val - ty_offset) % ty_stride != 0)) {
        AddError(call) << call->FriendlyName() << " size (" << size_val
                       << " bytes) minus type offset (" << ty_offset
                       << " bytes) must be a multiple of the type stride (" << ty_stride
                       << " bytes)";
        return false;
    }

    return true;
}

bool Functional::CheckSubgroupMatrixMemory(const CoreBuiltinCall* call,
                                           const Var* var,
                                           const UseInfo& info) {
    const bool is_load = call->Func() == BuiltinFn::kSubgroupMatrixLoad;
    bool col_major = false;
    auto* offset_arg = call->Args()[1];
    const Value* stride_arg = nullptr;
    if (is_load) {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[1]) == Majorness::kColMajor;
        stride_arg = call->Args()[2];
    } else {
        col_major = std::get<Majorness>(call->ExplicitTemplateParams()[0]) == Majorness::kColMajor;
        stride_arg = call->Args()[3];
    }
    auto* ty = is_load ? call->Result()->Type() : call->Args()[2]->Type();
    auto* mat_ty = ty->As<core::type::SubgroupMatrix>();
    auto* ele_ty = mat_ty->Type();

    auto* array_ty = call->Args()[0]->Type()->UnwrapPtr()->As<core::type::Array>();
    const uint32_t array_stride = array_ty->ImplicitStride();

    // Error conditions:
    // * stride is less than minimal required stride
    // * pointed to memory is smaller than matrix requires
    // * variable memory is smaller than total required
    //
    // Also if any calculation overflows 32 bits.

    const uint32_t major_size = col_major ? mat_ty->Columns() : mat_ty->Rows();
    const uint32_t minor_size = col_major ? mat_ty->Rows() : mat_ty->Columns();

    uint64_t offset = 0;
    if (auto* const_offset = offset_arg->As<Constant>()) {
        // Offset is array elements of shader scalar type.
        offset = const_offset->Value()->ValueAs<uint64_t>() * array_stride;

        if (offset > std::numeric_limits<uint32_t>::max()) {
            AddError(call) << call->FriendlyName() << " has an offset exceeding 32 bits";
            return false;
        }
    }
    uint32_t min_stride = minor_size * ele_ty->Size();
    uint64_t stride = 0;
    if (auto* const_stride = stride_arg->As<Constant>()) {
        // Stride is in array elements of shader scalar type.
        stride = const_stride->Value()->ValueAs<uint64_t>() * array_stride;

        if (stride > std::numeric_limits<uint32_t>::max()) {
            AddError(call) << call->FriendlyName() << " has a stride exceeding 32 bits";
            return false;
        }
        if (stride < min_stride) {
            AddError(call) << call->FriendlyName() << " stride (" << stride
                           << " bytes) must be greater or equal to " << min_stride << " bytes";
            return false;
        }
    } else {
        stride = min_stride;
    }

    // Note: Offset and stride are in bytes.
    uint64_t mat_required_size = offset + static_cast<uint64_t>(stride) * (major_size - 1) +
                                 static_cast<uint64_t>(minor_size) * ele_ty->Size();
    // Round up to array element size.
    mat_required_size = RoundUp(static_cast<uint64_t>(array_stride), mat_required_size);
    if (mat_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << call->FriendlyName() << " has a memory requirement exceeding 32 bits";
        return false;
    }

    if (info.pointer_size > 0 && info.pointer_size < mat_required_size) {
        AddError(call) << call->FriendlyName() << " requires more memory (" << mat_required_size
                       << " bytes) than pointed to (" << info.pointer_size << " bytes)";
        return false;
    }

    uint64_t mem_required_size = mat_required_size + info.offset;
    if (mem_required_size > std::numeric_limits<uint32_t>::max()) {
        AddError(call) << " has a total memory requirement exceeding 32 bits";
        return false;
    }

    if (info.storage_size > 0 && info.storage_size < mem_required_size) {
        AddError(var) << "invalid storage size (" << info.storage_size << " bytes) when used with "
                      << call->FriendlyName() << " (" << mem_required_size << " bytes required)";
        AddNote(call) << call->FriendlyName() << " here";
        return false;
    }

    return true;
}

}  // namespace tint::core::ir::validator
