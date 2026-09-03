// Copyright 2026 The Dawn & Tint Authors
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

#include <string_view>

#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/intrinsic/table.h"
#include "src/tint/lang/core/ir/constexpr_if.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unused.h"
#include "src/tint/lang/core/ir/validator/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/i8.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/u8.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/internal_limits.h"
#include "src/tint/utils/result.h"

namespace tint::core::ir::validator {
namespace {

template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl);

/// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param str the struct to walk the members of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
    for (auto* member : str->Members()) {
        WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
    }
}

/// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
/// @param ctx a context object to pass to the impl function
/// @param arr the array to walk the element type of
/// @param impl an impl function to be run, see WalkTypeAndMembers for details
template <typename CTX, typename IMPL>
void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
    WalkTypeAndMembers(ctx, arr->ElemType(), IOAttributes{}, impl);
}

/// Helper for walking a type that maybe a struct, calling an impl function for the type and each of
/// its members.
/// @param ctx a context object to pass to the implementation function
/// @param type the type to walk
/// @param attr the attributes for @p type
/// @param impl a function that is called for each type with the signature
///             `void(const core::type::Type*, const IOAttributes&, CTX&)`
template <typename CTX, typename IMPL>
void WalkTypeAndMembers(CTX& ctx,
                        const core::type::Type* type,
                        const IOAttributes& attr,
                        IMPL&& impl) {
    impl(ctx, type, attr);
    tint::Switch(
        type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
        [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
}

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

template <typename T>
bool ContainsType(const core::type::Type* ty) {
    bool found = false;
    WalkTypeAndMembers(found, ty, IOAttributes{},
                       [&](bool& ctx, const core::type::Type* t, const IOAttributes&) {
                           if (t != nullptr && t->DeepestElement()->Is<T>()) {
                               ctx = true;
                           }
                       });
    return found;
}

const constant::Value* GetConstArg(const CoreBuiltinCall* call, uint32_t param_index) {
    if ((call->Args().size() <= param_index) || (call->Args()[param_index] == nullptr) ||
        (!call->Args()[param_index]->Is<ir::Constant>())) {
        return nullptr;
    }
    return call->Args()[param_index]->As<ir::Constant>()->Value();
}

}  // namespace

uint64_t Validator::ElementsCount(const core::type::Type* root_ty) {
    TINT_ASSERT(root_ty);

    Vector<const core::type::Type*, 16> stack;
    stack.Push(root_ty);

    while (!stack.IsEmpty()) {
        const core::type::Type* ty = stack.Back();

        if (elements_counts_.Contains(ty)) {
            stack.Pop();
            continue;
        }

        bool children_ready = true;
        uint64_t count = 0;

        tint::Switch(
            ty,
            [&](const core::type::Struct* s) {
                for (auto* member : s->Members()) {
                    if (auto res = elements_counts_.Get(member->Type())) {
                        count += *res;
                    } else {
                        stack.Push(member->Type());
                        children_ready = false;
                    }
                }
            },
            [&](const core::type::Array* a) {
                if (auto res = elements_counts_.Get(a->ElemType())) {
                    uint64_t array_count = 0;
                    if (auto* const_count = a->Count()->As<core::type::ConstantArrayCount>()) {
                        array_count = const_count->value;
                    }
                    count = array_count * (*res);
                } else {
                    stack.Push(a->ElemType());
                    children_ready = false;
                }
            },
            [&](const core::type::Matrix* m) {
                if (auto res = elements_counts_.Get(m->ColumnType())) {
                    count = static_cast<uint64_t>(m->Columns()) * (*res);
                } else {
                    stack.Push(m->ColumnType());
                    children_ready = false;
                }
            },
            [&](const core::type::Vector* v) { count = static_cast<uint64_t>(v->Width()); },
            [&](Default) { count = 1; });

        if (children_ready) {
            elements_counts_.Add(ty, count);
            stack.Pop();
        }
    }

    return *elements_counts_.Get(root_ty);
}

void Validator::CheckInstruction(const Instruction* inst) {
    visited_instructions_.Add(inst);
    if (!inst->Alive()) {
        AddError(inst) << "destroyed instruction found in instruction list";
        return;
    }

    auto results = inst->Results();
    for (size_t i = 0; i < results.Length(); ++i) {
        auto* res = results[i];
        if (!res) {
            continue;
        }

        CheckType(res->Type(), [&]() -> diag::Diagnostic& { return AddResultError(inst, i); });
    }

    auto ops = inst->Operands();
    for (size_t i = 0; i < ops.Length(); ++i) {
        auto* op = ops[i];
        if (!op) {
            continue;
        }

        CheckType(op->Type(), [&]() -> diag::Diagnostic& { return AddError(inst, i); });
    }

    // Push a task to add the results to the scope.
    // This ensures that for control instructions, the results are only added to the scope
    // after their nested blocks have been evaluated (since tasks are processed LIFO).
    tasks_.Push([this, inst] {
        for (auto* result : inst->Results()) {
            if (result) {
                scope_stack_.Add(result);
            }
        }
    });

    tint::Switch(
        inst,                                                              //
        [&](const Access* a) { CheckAccess(a); },                          //
        [&](const Binary* b) { CheckBinary(b); },                          //
        [&](const Call* c) { CheckCall(c); },                              //
        [&](const If* if_) { CheckIf(if_); },                              //
        [&](const Let* let) { CheckLet(let); },                            //
        [&](const Load* load) { CheckLoad(load); },                        //
        [&](const LoadVectorElement* l) { CheckLoadVectorElement(l); },    //
        [&](const Loop* l) { CheckLoop(l); },                              //
        [&](const Phony* p) { CheckPhony(p); },                            //
        [&](const Store* s) { CheckStore(s); },                            //
        [&](const StoreVectorElement* s) { CheckStoreVectorElement(s); },  //
        [&](const Switch* s) { CheckSwitch(s); },                          //
        [&](const Swizzle* s) { CheckSwizzle(s); },                        //
        [&](const Terminator* b) { CheckTerminator(b); },                  //
        [&](const Unary* u) { CheckUnary(u); },                            //
        [&](const Override* o) { CheckOverride(o); },                      //
        [&](const Var* var) { CheckVar(var); },                            //
        TINT_ICE_ON_NO_MATCH);

    // Only check alignment if the instruction passed previous checks.
    if (!diag_.ContainsErrors()) {
        CheckAlignment(inst);
    }
}

void Validator::CheckOverride(const Override* o) {
    if (!CheckResultsAndOperands(o, Override::kNumResults, Override::kNumOperands)) {
        return;
    }

    if (o->Block() != ir_.root_block) {
        AddError(o) << "override must be declared at module scope";
    }

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

void Validator::CheckVar(const Var* var) {
    if (!CheckResultsAndOperands(var, Var::kNumResults, Var::kNumOperands)) {
        return;
    }

    auto* result_type = var->Result()->Type();
    auto* mv = result_type->As<core::type::MemoryView>();
    if (!mv) {
        AddError(var) << "result type " << NameOf(result_type)
                      << " must be a pointer or a reference";
        return;
    }
    const type::ValueArrayCount* count = nullptr;
    if (auto* ary = result_type->UnwrapPtr()->As<core::type::Array>()) {
        count = ary->Count()->As<type::ValueArrayCount>();
    } else if (auto* buf = result_type->UnwrapPtr()->As<core::type::Buffer>()) {
        count = buf->Count()->As<type::ValueArrayCount>();
    }

    if (count) {
        if (!scope_stack_.Contains(count->value)) {
            AddError(var) << NameOf(count->value) << " is not in scope";
        }
    }

    if (var->Initializer()) {
        if (!CheckOperand(var, ir::Var::kInitializerOperandOffset)) {
            return;
        }
    }

    CheckBindingPoint(var->Result(), var->Result(0)->Type(), var->Attributes(),
                      ShaderIOKind::kModuleScopeVar);

    auto address_space = mv->AddressSpace();
    if (address_space != AddressSpace::kIn && address_space != AddressSpace::kOut) {
        CheckInterpolation(var->Result(), mv->StoreType(), var->Attributes(),
                           Function::PipelineStage::kUndefined, IODirection::kResource);
    }

    if (var->Block() == ir_.root_block) {
        if (mv->AddressSpace() == AddressSpace::kIn || mv->AddressSpace() == AddressSpace::kOut) {
            ValidateShaderIOAnnotations(var->Result(), var->Result()->Type(), var->BindingPoint(),
                                        var->Attributes(), ShaderIOKind::kModuleScopeVar);
        }
    }

    bool generates_initializer = var->Initializer() != nullptr ||
                                 mv->AddressSpace() == core::AddressSpace::kPrivate ||
                                 mv->AddressSpace() == core::AddressSpace::kFunction;
    if (generates_initializer) {
        if (ElementsCount(result_type->UnwrapPtrOrRef()) >
            internal_limits::kMaxArrayConstructorElements) {
            AddError(var) << "type has excessive number of elements (>"
                          << internal_limits::kMaxArrayConstructorElements
                          << ") for an initializer";
            return;
        }
    }

    // Check that initializer and result type match
    if (var->Initializer()) {
        if (mv->AddressSpace() != AddressSpace::kFunction &&
            mv->AddressSpace() != AddressSpace::kPrivate &&
            mv->AddressSpace() != AddressSpace::kOut) {
            AddError(var) << "only variables in the function, private, or __out address space may "
                             "be initialized";
            return;
        }

        if (var->Initializer()->Type() != result_type->UnwrapPtrOrRef()) {
            AddError(var) << "initializer type " << NameOf(var->Initializer()->Type())
                          << " does not match store type " << NameOf(result_type->UnwrapPtrOrRef());
            return;
        }
    }

    if (var->Block() == ir_.root_block && mv->AddressSpace() == AddressSpace::kFunction) {
        AddError(var) << "vars in the 'function' address space must be in a function scope";
        return;
    }
    if (var->Block() != ir_.root_block && mv->AddressSpace() != AddressSpace::kFunction) {
        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface) ||
            mv->AddressSpace() != AddressSpace::kPrivate) {
            AddError(var) << "vars in a function scope must be in the 'function' address space";
            return;
        }
    }

    if (mv->AddressSpace() != AddressSpace::kStorage &&
        mv->AddressSpace() != AddressSpace::kHandle) {
        if (mv->AddressSpace() == AddressSpace::kWorkgroup ||
            !ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (!mv->StoreType()->HasFixedFootprint()) {
                AddError(var) << "vars not in the 'storage' or 'handle' address spaces "
                                 "must have a fixed footprint";
                return;
            }
        }
    }

    if (ContainsType<core::type::Atomic>(mv->StoreType())) {
        bool is_workgroup = mv->AddressSpace() == AddressSpace::kWorkgroup;
        bool is_read_write_storage = mv->AddressSpace() == AddressSpace::kStorage &&
                                     mv->Access() == core::Access::kReadWrite;
        if (!is_workgroup && !is_read_write_storage) {
            AddError(var)
                << "atomic types may only be used by 'workspace' or read write 'storage' variables";
            return;
        }
    }

    if (var->InputAttachmentIndex().has_value()) {
        if (mv->AddressSpace() != AddressSpace::kHandle) {
            AddError(var) << "'@input_attachment_index' is not valid for non-handle var";
            return;
        }
        if (!ir_.properties.Contains(Property::kAllowAnyInputAttachmentIndexType) &&
            !mv->UnwrapPtrOrRef()->Is<core::type::InputAttachment>()) {
            AddError(var)
                << "'@input_attachment_index' is only valid for 'input_attachment' type var";
            return;
        }
    }

    if (mv->AddressSpace() == AddressSpace::kStorage) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'storage' address space must be host-shareable";
            return;
        }
        if (mv->Access() != core::Access::kReadWrite && mv->Access() != core::Access::kRead) {
            AddError(var)
                << "vars in the 'storage' address space must have access 'read' or 'read-write'";
            return;
        }
    } else if (mv->AddressSpace() == AddressSpace::kUniform) {
        if (!ir_.properties.Contains(Property::kAllowMslEntryPointInterface)) {
            if (!(mv->StoreType()->IsConstructible() ||
                  mv->StoreType()->Is<core::type::Buffer>()) ||
                !mv->StoreType()->IsHostShareable()) {
                AddError(var) << "vars in the 'uniform' address space must be host-shareable and "
                                 "constructible or a buffer";
                return;
            }
        }
    } else if (mv->AddressSpace() == AddressSpace::kImmediate) {
        if (mv->StoreType() && !mv->StoreType()->IsHostShareable()) {
            AddError(var) << "vars in the 'immediate' address space must be host-shareable";
            return;
        }
    } else if (mv->AddressSpace() == core::AddressSpace::kPixelLocal) {
        if (var->Block() == ir_.root_block) {
            if (!mv->StoreType()->Is<core::type::Struct>()) {
                AddError(var) << "pixel_local var must be of type struct";
                return;
            }
        }
    }

    if (mv->AddressSpace() == AddressSpace::kPrivate) {
        total_private_bytes_ += mv->StoreType()->Size();
        if (total_private_bytes_ > internal_limits::kMaxCombinedPrivateVariableSize) {
            AddError(var) << "total size of private address-space variables exceeds "
                          << internal_limits::kMaxCombinedPrivateVariableSize << " bytes";
            return;
        }
    }
}

void Validator::CheckLet(const Let* l) {
    if (!CheckResultsAndOperands(l, Let::kNumResults, Let::kNumOperands)) {
        return;
    }

    auto* result_ty = l->Result()->Type();
    if (ElementsCount(result_ty) > internal_limits::kMaxArrayConstructorElements) {
        AddError(l) << "type has excessive number of elements (>"
                    << internal_limits::kMaxArrayConstructorElements << ") for an initializer";
        return;
    }
    auto* value_ty = l->Value()->Type();
    if (value_ty != result_ty) {
        AddError(l) << "result type " << NameOf(l->Result()->Type())
                    << " does not match value type " << NameOf(l->Value()->Type());
    }

    if (ir_.properties.Contains(Property::kAllowAnyLetType)) {
        if (value_ty->Is<core::type::Void>()) {
            AddError(l) << "value type cannot be void";
        }
        return;
    }

    if (!value_ty->IsConstructible() && !value_ty->Is<core::type::Pointer>()) {
        AddError(l) << "value type, " << NameOf(value_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }

    if (auto* ptr = result_ty->As<core::type::Pointer>()) {
        if (ptr->AddressSpace() == AddressSpace::kHandle &&
            !ir_.properties.Contains(Property::kAllowPointerToHandle)) {
            AddError(l) << "handle pointer cannot be captured in a let";
        }
    } else if (!result_ty->IsConstructible()) {
        AddError(l) << "result type, " << NameOf(result_ty)
                    << ", must be a concrete constructible type or a pointer type";
    }
}

void Validator::CheckCall(const Call* call) {
    tint::Switch(
        call,                                                            //
        [&](const BuiltinCall* c) { CheckBuiltinCall(c); },              //
        [&](const MemberBuiltinCall* c) { CheckMemberBuiltinCall(c); },  //
        [&](const Construct* c) { CheckConstruct(c); },                  //
        [&](const Convert* c) { CheckConvert(c); },                      //
        [&](const Discard* d) {                                          //
            stage_restricted_instructions_.Add(
                d, SupportedStages{Function::PipelineStage::kFragment});        //
            CheckDiscard(d);                                                    //
        },                                                                      //
        [&](const UserCall* c) {                                                //
            if (c->Target()) {                                                  //
                auto calls =                                                    //
                    user_func_calls_.GetOr(c->Target(),                         //
                                           Hashset<const ir::UserCall*, 4>{});  //
                calls.Add(c);                                                   //
                user_func_calls_.Replace(c->Target(), calls);                   //
            }
            CheckUserCall(c);
        },
        [&](Default) {
            // Validation of custom IR instructions
        });
}

void Validator::CheckBuiltinCall(const BuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, BuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

    auto args = Transform<8>(call->Args(), [&](const ir::Value* v) { return v->Type(); });

    intrinsic::Context context{call->TableData(), type_mgr_, symbols_};
    auto builtin = core::intrinsic::LookupFn(context, call->FriendlyName().c_str(), call->FuncId(),
                                             call->ExplicitTemplateParams(), args,
                                             core::EvaluationStage::kRuntime);
    if (builtin != Success) {
        AddError(call) << builtin.Failure();
        return;
    }

    // Track the stages that this builtin call is limited to, so that we can check them against the
    // entry points that they are used from.
    SupportedStages stages;
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsComputePipeline)) {
        stages.Add(Function::PipelineStage::kCompute);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsFragmentPipeline)) {
        stages.Add(Function::PipelineStage::kFragment);
    }
    if (builtin->info->flags.Contains(intrinsic::OverloadFlag::kSupportsVertexPipeline)) {
        stages.Add(Function::PipelineStage::kVertex);
    }
    stage_restricted_instructions_.Add(call, stages);

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

void Validator::CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                                     const core::intrinsic::Overload& overload) {
    if (ir_.properties.Contains(Property::kDisallowVectorMinMaxClamp)) {
        switch (call->Func()) {
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
                if (call->Result()->Type()->Is<core::type::Vector>()) {
                    AddError(call) << "vector " << call->FriendlyName()
                                   << " disallowed by the DisallowVectorMinMaxClamp property";
                }
                break;
            default:
                break;
        }
    }
    if (ir_.properties.Contains(Property::kAllowBufferTypes)) {
        switch (call->Func()) {
            case core::BuiltinFn::kBufferArrayView:
                if (call->Result()->Type()->UnwrapPtr()->HasFixedFootprint()) {
                    AddError(call)
                        << call->FriendlyName() << " result type must not have a fixed footprint";
                }
                break;
            default:
                break;
        }
    }

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

    if (!IsWGSLValidation()) {
        return;
    }

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

void Validator::CheckSubgroupCall(const CoreBuiltinCall* call) {
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

void Validator::CheckExtractBitsCall(const CoreBuiltinCall* call) {
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

void Validator::CheckInsertBitsCall(const CoreBuiltinCall* call) {
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

void Validator::CheckLdexpCall(const CoreBuiltinCall* call) {
    auto* param0 = call->Args()[0];
    if (auto const_val = GetConstArg(call, 1)) {
        auto* zero = const_eval_.Zero(param0->Type(), {}, Source{}).Get();
        auto fakeArgs = Vector{zero, const_val};
        [[maybe_unused]] auto result =
            const_eval_.ldexp(param0->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Validator::CheckQuantizeToF16(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.quantizeToF16(
            call->Result()->Type(), Vector{const_val}, ir_.SourceOf(call));
    }
}

void Validator::CheckPack2x16float(const CoreBuiltinCall* call) {
    if (auto const_val = GetConstArg(call, 0)) {
        [[maybe_unused]] auto result = const_eval_.pack2x16float(
            call->Result()->Type(), Vector{const_val}, ir_.SourceOf(call));
    }
}

void Validator::CheckClampCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 1);
    auto* const_val_high = GetConstArg(call, 2);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_low, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.clamp(call->Result()->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Validator::CheckSmoothstepCall(const CoreBuiltinCall* call) {
    auto* const_val_low = GetConstArg(call, 0);
    auto* const_val_high = GetConstArg(call, 1);
    if (const_val_low && const_val_high) {
        auto fakeArgs = Vector{const_val_low, const_val_high, const_val_high};
        [[maybe_unused]] auto result =
            const_eval_.smoothstep(call->Result()->Type(), fakeArgs, ir_.SourceOf(call));
    }
}

void Validator::CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call) {
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

void Validator::CheckMemberBuiltinCall(const MemberBuiltinCall* call) {
    // This check cannot be more precise, since until intrinsic lookup below, it is unknown what
    // number of operands are expected, but still need to enforce things are in scope,
    // have types, etc.
    if (!CheckResults(call, MemberBuiltinCall::kNumResults) || !CheckOperands(call)) {
        return;
    }

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

void Validator::CheckConstruct(const Construct* construct) {
    if (!CheckResultsAndOperandRange(construct, Construct::kNumResults, Construct::kMinOperands)) {
        return;
    }

    auto* result_type = construct->Result()->Type();
    if (ElementsCount(result_type) > internal_limits::kMaxArrayConstructorElements) {
        AddError(construct) << "type has excessive number of elements (>"
                            << internal_limits::kMaxArrayConstructorElements
                            << ") for an initializer";
        return;
    }
    if (!result_type->IsConstructible()) {
        // We only allow `construct` to create non-constructible types when they are structures that
        // contain pointers and handle types, with the corresponding property enabled.
        if (!(result_type->Is<core::type::Struct>() &&
              ir_.properties.Contains(Property::kAllowMslEntryPointInterface))) {
            AddError(construct) << "type is not constructible";
            return;
        }
    }

    auto args = construct->Args();

    // Zero-value constructors are valid for all constructible types.
    if (args.empty()) {
        return;
    }

    // Check that type type of each argument matches the expected element type of the composite.
    auto check_args_match_elements = [&] {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->Is<ir::Unused>()) {
                continue;
            }
            auto* expected_type = result_type->Element(static_cast<uint32_t>(i));
            if (args[i]->Type() != expected_type) {
                AddError(construct, Construct::kArgsOperandOffset + i)
                    << "type " << NameOf(args[i]->Type()) << " of argument " << i
                    << " does not match expected type " << NameOf(expected_type);
            }
        }
    };

    if (result_type->Is<core::type::Scalar>()) {
        // The only valid non-zero scalar constructor is the identity operation.
        if (args.size() > 1) {
            AddError(construct) << "scalar construct must not have more than one argument";
        }
        if (args[0]->Type() != result_type) {
            AddError(construct, 0u) << "scalar construct argument type " << NameOf(args[0]->Type())
                                    << " does not match result type " << NameOf(result_type);
        }
        return;
    }

    if (auto* sg_mat = result_type->As<core::type::SubgroupMatrix>()) {
        if (args.size() > 1) {
            AddError(construct) << "subgroup matrix construct must not have more than 1 argument";
            return;
        }

        // 8-bit integer matrices use 32-bit shader scalar types in WGSL.
        // Some backends may support 8-bit integers, in which case they would pass an 8-bit
        // type for the constructor value instead.
        const core::type::Type* scalar_ty = sg_mat->Type();
        if (scalar_ty->Is<core::type::I8>()) {
            scalar_ty = type_mgr_.i32();
        } else if (scalar_ty->Is<core::type::U8>()) {
            scalar_ty = type_mgr_.u32();
        }
        if (args[0]->Type() != scalar_ty && args[0]->Type() != sg_mat->Type()) {
            AddError(construct) << "subgroup matrix construct argument type "
                                << NameOf(args[0]->Type())
                                << " does not match matrix shader scalar type "
                                << NameOf(scalar_ty);
        }
        return;
    }

    if (auto* arr = result_type->As<core::type::Array>()) {
        if (args.size() != arr->ConstantCount()) {
            AddError(construct) << "array has " << arr->ConstantCount().value()
                                << " elements, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
        return;
    }

    if (auto* str = As<core::type::Struct>(result_type)) {
        auto members = str->Members();
        if (args.size() != str->Members().Length()) {
            AddError(construct) << "structure has " << members.Length()
                                << " members, but construct provides " << args.size()
                                << " arguments";
            return;
        }
        check_args_match_elements();
    }

    auto table = intrinsic::Table<intrinsic::Dialect>(type_mgr_, symbols_);
    auto arg_types = Transform<4>(args, [&](auto* v) { return v->Type(); });
    if (auto* vec = result_type->As<core::type::Vector>()) {
        auto ctor_conv = intrinsic::VectorCtorConv(vec->Width());
        auto match = table.Lookup(ctor_conv, Vector<TemplateParameter, 1>{vec->Type()},
                                  std::move(arg_types), core::EvaluationStage::kConstant);
        if (match != Success ||
            !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConstructor) ||
            vec->Type() != arg_types[0]->DeepestElement()) {
            AddError(construct) << "no matching overload for " << vec->FriendlyName()
                                << " constructor";
        }
        return;
    }

    if (auto* mat = result_type->As<core::type::Matrix>()) {
        auto ctor_conv = intrinsic::MatrixCtorConv(mat->Columns(), mat->Rows());
        auto match = table.Lookup(ctor_conv, Vector<TemplateParameter, 1>{mat->Type()},
                                  std::move(arg_types), core::EvaluationStage::kConstant);
        if (match != Success ||
            !match->info->flags.Contains(intrinsic::OverloadFlag::kIsConstructor)) {
            AddError(construct) << "no matching overload for " << mat->FriendlyName()
                                << " constructor";
        }
        return;
    }
}

void Validator::CheckConvert(const Convert* convert) {
    if (!CheckResultsAndOperands(convert, Convert::kNumResults, Convert::kNumOperands)) {
        return;
    }

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

void Validator::CheckDiscard(const Discard* discard) {
    if (!CheckResultsAndOperands(discard, Discard::kNumResults, Discard::kNumOperands)) {
        return;
    }
}

void Validator::CheckUserCall(const UserCall* call) {
    if (!CheckResultsAndOperandRange(call, UserCall::kNumResults, UserCall::kMinOperands)) {
        return;
    }

    if (!call->Target()) {
        AddError(call, UserCall::kFunctionOperandOffset) << "target not defined or not a function";
        return;
    }
    if (call->Target()->IsEntryPoint()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "call target must not have a pipeline stage";
    }

    if (call->Target()->ReturnType() != call->Result()->Type()) {
        AddError(call) << "result type does not match function return type";
        return;
    }

    auto args = call->Args();
    auto params = call->Target()->Params();
    if (args.size() != params.Length()) {
        AddError(call, UserCall::kFunctionOperandOffset)
            << "function has " << params.Length() << " parameters, but call provides "
            << args.size() << " arguments";
        return;
    }

    for (size_t i = 0; i < args.size(); i++) {
        bool allow_mismatch = false;
        if (auto* arg_buffer_ty = args[i]->Type()->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
            auto* arg_ptr_ty = args[i]->Type()->As<core::type::Pointer>();
            if (auto* param_ptr_ty = params[i]->Type()->As<core::type::Pointer>()) {
                if (auto* param_buffer_ty =
                        param_ptr_ty->UnwrapPtrOrRef()->As<core::type::Buffer>()) {
                    allow_mismatch = arg_ptr_ty->AddressSpace() == param_ptr_ty->AddressSpace() &&
                                     arg_ptr_ty->Access() == param_ptr_ty->Access();
                    const bool both_constant =
                        arg_buffer_ty->Count()->Is<core::type::ConstantArrayCount>() &&
                        param_buffer_ty->Count()->Is<core::type::ConstantArrayCount>();
                    uint32_t arg_size = arg_buffer_ty->ConstantCount().value_or(0);
                    uint32_t param_size = param_buffer_ty->ConstantCount().value_or(0);
                    allow_mismatch &=
                        param_buffer_ty->Count()->Is<core::type::RuntimeArrayCount>() ||
                        (both_constant && param_size < arg_size);
                }
            }
        }
        if (!allow_mismatch && args[i]->Type() != params[i]->Type()) {
            AddError(call, UserCall::kArgsOperandOffset + i)
                << "type " << NameOf(params[i]->Type()) << " of function parameter " << i
                << " does not match argument type " << NameOf(args[i]->Type());
        }
    }
}

void Validator::CheckAccess(const Access* a) {
    if (!CheckResultsAndOperandRange(a, Access::kNumResults, Access::kMinNumOperands)) {
        return;
    }
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

void Validator::CheckBinary(const Binary* b) {
    if (!CheckResultsAndOperands(b, Binary::kNumResults, Binary::kNumOperands)) {
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalAnd) {
        AddError(b) << "logical-and is not valid in the IR";
        return;
    }
    if (b->Op() == core::BinaryOp::kLogicalOr) {
        AddError(b) << "logical-or is not valid in the IR";
        return;
    }

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

void Validator::CheckCoreBinaryCall(const CoreBinary* call) {
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

void Validator::CheckBinaryDivModCall(const CoreBinary* call) {
    if (!IsWGSLValidation()) {
        return;
    }
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

void Validator::CheckBinaryShiftCall(const CoreBinary* call) {
    if (!IsWGSLValidation()) {
        return;
    }
    // If lhs value is a concrete type, and rhs is a const-expression greater than or equal
    // to the bit width of lhs, then it is a shader-creation error.
    const auto* elem_type = call->LHS()->Type()->DeepestElement();
    const uint32_t bit_width = elem_type->Size() * 8;
    if (auto* rhs_val_as_const = call->RHS()->As<ir::Constant>()) {
        auto* rhs_as_value = rhs_val_as_const->Value();
        for (size_t i = 0, n = rhs_as_value->NumElements(); i < n; i++) {
            auto* shift_val = n == 1 ? rhs_as_value : rhs_as_value->Index(i);
            if (shift_val->ValueAs<u32>() >= bit_width) {
                AddError(call) << "shift "
                               << (call->Op() == core::BinaryOp::kShiftLeft ? "left" : "right")
                               << " value must be less than the bit width of the lhs, which is "
                               << bit_width;
                break;
            }
        }
    }
}

void Validator::CheckUnary(const Unary* u) {
    if (!CheckResultsAndOperands(u, Unary::kNumResults, Unary::kNumOperands)) {
        return;
    }

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

void Validator::CheckIf(const If* if_) {
    CheckResults(if_);
    CheckOperands(if_, If::kNumOperands);

    if (if_->False() && if_->False()->Is<MultiInBlock>()) {
        AddError(if_) << "if false block must be a block";
    }
    if (if_->True() && if_->True()->Is<MultiInBlock>()) {
        AddError(if_) << "if true block must be a block";
    }

    if (auto* constexpr_if = if_->As<ConstExprIf>()) {
        if (constexpr_if->Results().Length() != 1) {
            AddError(constexpr_if) << "constexpr_if must have exactly one result";
        } else if (!constexpr_if->Result(0)->Type()->Is<core::type::Bool>()) {
            AddError(constexpr_if) << "constexpr_if result type must be 'bool'";
        }
        if (constexpr_if->False()->IsEmpty()) {
            AddError(constexpr_if) << "constexpr_if must have a false block";
        } else if (!constexpr_if->False()->Terminator() ||
                   !constexpr_if->False()->Terminator()->Is<ExitIf>()) {
            AddError(constexpr_if->False())
                << "constexpr_if false block terminator must be an exit_if";
        }
        if (!constexpr_if->True()->Terminator() ||
            !constexpr_if->True()->Terminator()->Is<ExitIf>()) {
            AddError(constexpr_if->True())
                << "constexpr_if true block terminator must be an exit_if";
        }
    }

    QueueTasks(
        PushControlStack(if_),
        [this, if_] {
            if (!if_->False()->IsEmpty()) {
                QueueBlock(if_->False());
            }
            QueueBlock(if_->True());
        },
        PopControlStack());
}

void Validator::CheckLoop(const Loop* l) {
    CheckResults(l);
    CheckOperands(l, 0);

    if (l->Initializer()->Is<MultiInBlock>()) {
        AddError(l->Initializer()) << "loop initializer must be a block";
    }

    if (!l->Initializer()->IsEmpty()) {
        if (!l->Initializer()->Terminator() ||
            !l->Initializer()->Terminator()->Is<NextIteration>()) {
            AddError(l->Initializer()) << "loop initializer must have a NextIteration terminator";
        }
    }

    if (!l->Body()->Params().IsEmpty()) {
        if (!l->HasInitializer()) {
            AddError(l) << "loop with body block parameters must have an initializer";
        }
    }

    if (l->Body()->IsEmpty()) {
        AddError(l->Body()) << "loop body block must not be empty";
    }

    if (l->Continuing()->IsEmpty()) {
        if (!l->Continuing()->Params().IsEmpty()) {
            AddError(l) << "loop continuing block has parameters but is empty";
        }
    } else if (!l->Continuing()->Terminator()->IsAnyOf<NextIteration, BreakIf>()) {
        AddError(l->Continuing())
            << "loop continuing terminator can only be next_iteration or break_if";
    }

    // ⎡Initializer              ⎤
    // ⎢    ⎡Body               ⎤⎥
    // ⎣    ⎣    [Continuing ]  ⎦⎦
    QueueTasks(PushControlStack(l),
               QueueNestedTasks(  //
                   BeginBlockTask(l->Initializer()),
                   QueueNestedTasks(  //
                       BeginBlockTask(l->Body()),
                       QueueNestedTasks(                     //
                           BeginBlockTask(l->Continuing()),  //
                           [] {},                            //
                           EndBlockTask(l->Continuing())),
                       EndBlockTask(l->Body())),
                   EndBlockTask(l->Initializer())),
               PopControlStack());
}

void Validator::CheckSwitch(const Switch* s) {
    CheckResults(s);
    CheckOperands(s, Switch::kNumOperands);

    QueueTasks(
        PushControlStack(s),
        [this, s] {
            for (auto& cse : s->Cases()) {
                if (cse.selectors.IsEmpty()) {
                    AddError(s) << "case does not have any selectors";
                }
                if (cse.block->Is<MultiInBlock>()) {
                    AddError(s) << "case block must be a block";
                }
                QueueBlock(cse.block);
            }
        },
        PopControlStack());
}

void Validator::CheckSwizzle(const Swizzle* s) {
    if (!CheckResultsAndOperands(s, Swizzle::kNumResults, Swizzle::kNumOperands)) {
        return;
    }

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

void Validator::CheckTerminator(const Terminator* b) {
    // All terminators should have zero results
    if (!CheckResults(b, 0)) {
        return;
    }

    // Operands must be alive and in scope if they are not nullptr.
    if (!CheckOperands(b)) {
        return;
    }

    tint::Switch(
        b,                                                           //
        [&](const ir::BreakIf* i) { CheckBreakIf(i); },              //
        [&](const ir::Continue* c) { CheckContinue(c); },            //
        [&](const ir::Exit* e) { CheckExit(e); },                    //
        [&](const ir::NextIteration* n) { CheckNextIteration(n); },  //
        [&](const ir::Return* ret) { CheckReturn(ret); },            //
        [&](const ir::TerminateInvocation*) {},                      //
        [&](const ir::Unreachable* u) { CheckUnreachable(u); },      //
        TINT_ICE_ON_NO_MATCH);

    if (b->next) {
        AddError(b) << "must be the last instruction in the block";
    }
}

void Validator::CheckBreakIf(const BreakIf* b) {
    auto* loop = b->Loop();
    if (loop == nullptr) {
        AddError(b) << "has no associated loop";
        return;
    }
    if (b->Condition() == nullptr) {
        AddError(b) << "break_if condition cannot be nullptr";
        return;
    }

    auto next_iter_values = b->NextIterValues();
    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(b, b->ArgsOperandOffset(), next_iter_values.size(), body,
                                 body->Params());
    }

    auto exit_values = b->ExitValues();
    CheckOperandsMatchTarget(b, b->ArgsOperandOffset() + next_iter_values.size(),
                             exit_values.size(), loop, loop->Results());

    if (!b->Condition()->Type() || !b->Condition()->Type()->Is<core::type::Bool>()) {
        AddError(b) << "condition must be a 'bool'";
        return;
    }

    if (loop->Continuing() != b->Block()) {
        AddError(b) << "must only be called directly from loop continuing";
    }
}

void Validator::CheckContinue(const Continue* c) {
    auto* loop = c->Loop();
    if (loop == nullptr) {
        AddError(c) << "has no associated loop";
        return;
    }
    if (!TransitivelyHolds(loop->Body(), c)) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(c) << "must only be called from loop body";
        } else {
            AddError(c) << "called outside of associated loop";
        }
    }

    if (auto* cont = loop->Continuing()) {
        CheckOperandsMatchTarget(c, Continue::kArgsOperandOffset, c->Args().size(), cont,
                                 cont->Params());
    }
}

void Validator::CheckExit(const Exit* e) {
    if (control_stack_.IsEmpty()) {
        AddError(e) << "found outside all control instructions";
        return;
    }
    if (e->ControlInstruction() == nullptr) {
        AddError(e) << "has no parent control instruction";
        return;
    }

    auto args = e->Args();
    CheckOperandsMatchTarget(e, e->ArgsOperandOffset(), args.size(), e->ControlInstruction(),
                             e->ControlInstruction()->Results());

    tint::Switch(
        e,                                                     //
        [&](const ir::ExitIf* i) { CheckExitIf(i); },          //
        [&](const ir::ExitLoop* l) { CheckExitLoop(l); },      //
        [&](const ir::ExitSwitch* s) { CheckExitSwitch(s); },  //
        TINT_ICE_ON_NO_MATCH);
}

void Validator::CheckNextIteration(const NextIteration* n) {
    auto* loop = n->Loop();
    if (loop == nullptr) {
        AddError(n) << "has no associated loop";
        return;
    }

    if (loop->Initializer() != n->Block() && loop->Continuing() != n->Block()) {
        if (control_stack_.Any(Eq<const ControlInstruction*>(loop))) {
            AddError(n) << "must only be called directly from loop initializer or continuing";
        } else {
            AddError(n) << "called outside of associated loop";
        }
    }

    if (auto* body = loop->Body()) {
        CheckOperandsMatchTarget(n, NextIteration::kArgsOperandOffset, n->Args().size(), body,
                                 body->Params());
    }
}

void Validator::CheckExitIf(const ExitIf* e) {
    if (control_stack_.Back() != e->If()) {
        AddError(e) << "if target jumps over other control instructions";
        AddNote(control_stack_.Back()) << "first control instruction jumped";
    }
}

void Validator::CheckReturn(const Return* ret) {
    if (!CheckOperands(ret, Return::kMinOperands, Return::kMaxOperands)) {
        return;
    }

    auto* func = ret->Func();
    if (func == nullptr) {
        // Func() returning nullptr after CheckResultsAndOperandRange is due to the first
        // operand being not a function
        AddError(ret) << "expected function for first operand";
        return;
    }

    if (func != ContainingFunction(ret)) {
        AddError(ret) << "function operand does not match containing function";
        return;
    }

    if (func->ReturnType()->Is<core::type::Void>()) {
        if (ret->HasValue()) {
            AddError(ret) << "unexpected return value";
        }
        return;
    }

    if (!ret->Value()) {
        AddError(ret) << "expected return value";
    } else if (ret->Value()->Type() != func->ReturnType()) {
        AddError(ret) << "return value type " << NameOf(ret->Value()->Type())
                      << " does not match function return type " << NameOf(func->ReturnType());
    }
}

void Validator::CheckUnreachable(const Unreachable* u) {
    if (!CheckResultsAndOperands(u, Unreachable::kNumResults, Unreachable::kNumOperands)) {
        return;
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
            AddError(exit) << control->FriendlyName()
                           << " target jumps over other control instructions";
            AddNote(ctrl) << "first control instruction jumped";
            return;
        }
    }
    if (!found) {
        AddError(exit) << control->FriendlyName() << " not found in parent control instructions";
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
                AddError(l) << "loop exit jumps out of continuing block";
                if (control->Continuing() != l->Block()) {
                    AddNote(control->Continuing()) << "in continuing block";
                }
            } else if (inst->Block() == control->Initializer()) {
                AddError(l) << "loop exit not permitted in loop initializer";
                if (control->Initializer() != l->Block()) {
                    AddNote(control->Initializer()) << "in initializer block";
                }
            }
            break;
        }
        inst = inst->Block()->Parent();
    }
}

void Validator::CheckLoad(const Load* l) {
    if (!CheckResultsAndOperands(l, Load::kNumResults, Load::kNumOperands)) {
        return;
    }

    const Value* from = l->From();
    TINT_ASSERT(from);

    auto* mv = from->Type()->As<core::type::MemoryView>();
    if (!mv) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand " << NameOf(from->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kRead && mv->Access() != core::Access::kReadWrite) {
        AddError(l, Load::kFromOperandOffset)
            << "load source operand has a non-readable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (l->Result()->Type() != mv->StoreType()) {
        AddError(l, Load::kFromOperandOffset)
            << "result type " << NameOf(l->Result()->Type()) << " does not match source store type "
            << NameOf(mv->StoreType());
    }

    if (!CanLoad(mv->StoreType())) {
        AddError(l, Load::kFromOperandOffset)
            << "type " << NameOf(mv->StoreType()) << " cannot be loaded";
        return;
    }
}

void Validator::CheckStore(const Store* s) {
    if (!CheckResultsAndOperands(s, Store::kNumResults, Store::kNumOperands)) {
        return;
    }

    const Value* from = s->From();
    const Value* to = s->To();
    TINT_ASSERT(from != nullptr);
    TINT_ASSERT(to != nullptr);

    auto* mv = As<core::type::MemoryView>(to->Type());
    if (!mv) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand " << NameOf(to->Type()) << " is not a memory view";
        return;
    }

    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, Store::kToOperandOffset)
            << "store target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    const core::type::Type* value_type = from->Type();
    const core::type::Type* store_type = mv->StoreType();
    if (value_type != store_type) {
        AddError(s, Store::kFromOperandOffset)
            << "value type " << NameOf(value_type) << " does not match store type "
            << NameOf(store_type);
        return;
    }

    if (!store_type->IsConstructible()) {
        AddError(s) << "store type " << NameOf(store_type) << " is not constructible";
        return;
    }
}

const core::type::Type* Validator::GetVectorPtrElementType(const Instruction* inst, size_t idx) {
    auto* operand = inst->Operands()[idx];
    TINT_ASSERT(operand) << "missing element operand";

    auto* type = operand->Type();
    TINT_ASSERT(type) << "missing operand type";

    auto* memory_view_ty = type->As<core::type::MemoryView>();
    if (DAWN_LIKELY(memory_view_ty)) {
        auto* vec_ty = memory_view_ty->StoreType()->As<core::type::Vector>();
        if (DAWN_LIKELY(vec_ty)) {
            return vec_ty->Type();
        }
    }

    AddError(inst, idx) << "operand " << NameOf(type) << " must be a pointer to a vector";
    return nullptr;
}

void Validator::CheckLoadVectorElement(const LoadVectorElement* l) {
    if (!CheckResultsAndOperands(l, LoadVectorElement::kNumResults,
                                 LoadVectorElement::kNumOperands)) {
        return;
    }

    const core::type::Type* el_ty =
        GetVectorPtrElementType(l, LoadVectorElement::kFromOperandOffset);
    if (!el_ty) {
        return;
    }

    auto* res = l->Result(0);
    if (res->Type() != el_ty) {
        AddError(l) << "result type " << NameOf(res->Type())
                    << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    if (!l->Index()->Type()->IsIntegerScalar()) {
        AddError(l, LoadVectorElement::kIndexOperandOffset)
            << "load vector element index must be an integer scalar";
    }
    if (auto* c = l->Index()->As<Constant>()) {
        uint32_t val = c->Value()->ValueAs<uint32_t>();

        const core::type::Vector* vec_ty =
            l->From()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
        TINT_ASSERT(vec_ty);

        if (val >= vec_ty->Width()) {
            AddError(l, LoadVectorElement::kIndexOperandOffset)
                << "load vector element index must be in range [0, " << (vec_ty->Width() - 1)
                << "]";
        }
    }
}

void Validator::CheckStoreVectorElement(const StoreVectorElement* s) {
    if (!CheckResultsAndOperands(s, StoreVectorElement::kNumResults,
                                 StoreVectorElement::kNumOperands)) {
        return;
    }

    const core::type::Type* el_ty =
        GetVectorPtrElementType(s, StoreVectorElement::kToOperandOffset);
    if (!el_ty) {
        return;
    }
    auto* value = s->Value();
    if (value->Type() != el_ty) {
        AddError(s, StoreVectorElement::kValueOperandOffset)
            << "value type " << NameOf(value->Type())
            << " does not match vector pointer element type " << NameOf(el_ty);
        return;
    }

    // The `GetVectorPtrElementType` has already validated that the pointer exists.
    const core::type::MemoryView* mv = s->To()->Type()->As<core::type::MemoryView>();
    if (mv->Access() != core::Access::kWrite && mv->Access() != core::Access::kReadWrite) {
        AddError(s, StoreVectorElement::kToOperandOffset)
            << "store_vector_element target operand has a non-writeable access type, "
            << style::Literal(ToString(mv->Access()));
        return;
    }

    if (!s->Index()->Type()->IsIntegerScalar()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be an integer scalar";
    }

    const Constant* c = s->Index()->As<Constant>();
    if (c == nullptr) {
        return;
    }

    uint32_t val = c->Value()->ValueAs<uint32_t>();
    const core::type::Vector* vec_ty = s->To()->Type()->UnwrapPtrOrRef()->As<core::type::Vector>();
    TINT_ASSERT(vec_ty);

    if (val >= vec_ty->Width()) {
        AddError(s, StoreVectorElement::kIndexOperandOffset)
            << "store vector element index must be in range [0, " << (vec_ty->Width() - 1) << "]";
    }
}

void Validator::CheckPhony(const Phony* p) {
    if (!ir_.properties.Contains(Property::kAllowPhonyInstructions)) {
        AddError(p) << "missing property 'kAllowPhonyInstructions'";
        return;
    }

    if (!CheckResultsAndOperands(p, Phony::kNumResults, Phony::kNumOperands)) {
        return;
    }
}

}  // namespace tint::core::ir::validator
