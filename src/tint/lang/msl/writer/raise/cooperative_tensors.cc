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

#include "src/tint/lang/msl/writer/raise/cooperative_tensors.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/traverse.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"
#include "src/tint/lang/msl/ir/member_builtin_call.h"
#include "src/tint/lang/msl/type/cooperative_tensor.h"
#include "src/tint/lang/msl/type/tensor_inline.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// A map from a subgroup matrix value to the local var that contains the cooperative_tensor.
    Hashmap<const core::ir::Value*, core::ir::Var*, 8> value_to_local_var{};

    /// The set of local variables that are shared across multiple uses.
    Hashset<const core::ir::Var*, 8> shared_local_vars{};

    /// Process the module.
    void Process() {
        // Take a copy of the functions since we may introduce helper functions.
        auto functions = ir.functions;
        for (auto* func : functions) {
            // TODO(555763296): Replace function parameters.
            // TODO(555764596): Replace function return type.

            // Process all instructions in the function that produce subgroup matrix result types.
            Vector<core::ir::Instruction*, 32> worklist;
            core::ir::Traverse(func->Block(), [&](core::ir::Instruction* inst) {
                if (inst->Results().Length() != 1u) {
                    return;
                }
                if (ContainsSubgroupMatrix(inst->Result()->Type()->UnwrapPtr())) {
                    worklist.Push(inst);
                } else if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                    switch (call->Func()) {
                        case core::BuiltinFn::kSubgroupMatrixStore: {
                            worklist.Push(inst);
                            break;
                        }
                        default:
                            break;
                    }
                }
            });
            for (auto* inst : worklist) {
                tint::Switch(
                    inst,                                                   //
                    [&](core::ir::CoreBuiltinCall* c) { ProcessCall(c); },  //
                    [&](core::ir::Construct* c) { ProcessConstruct(c); },   //
                    [&](core::ir::Let* let) { ProcessLet(let); },           //
                    [&](core::ir::Var* var) { ProcessVar(var); },           //
                    TINT_ICE_ON_NO_MATCH);
            }
        }
    }

    /// @returns true if @p type contains a subgroup matrix type
    bool ContainsSubgroupMatrix(const core::type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const core::type::SubgroupMatrix*) { return true; },
            [&](const core::type::Array* arr) { return ContainsSubgroupMatrix(arr->ElemType()); },
            [&](const core::type::Struct* s) {
                for (auto* member : s->Members()) {
                    if (ContainsSubgroupMatrix(member->Type())) {
                        return true;
                    }
                }
                return false;
            },
            [](Default) { return false; });
    }

    /// Register @p var as the local variable for the value @p val.
    /// If the value is used multiple times, the local variable is marked as shared.
    void RegisterLocalVar(core::ir::Value* val, core::ir::Var* var) {
        if (val->NumUsages() > 1u) {
            shared_local_vars.Add(var);
        }
        value_to_local_var.Add(val, var);
    }

    /// Process a `call` instruction to replace its type and initializer.
    /// @param c the construct instruction
    void ProcessCall(core::ir::CoreBuiltinCall* c) {
        switch (c->Func()) {
            case core::BuiltinFn::kSubgroupMatrixStore:
                ReplaceSubgroupMatrixStore(c);
                break;
            default:
                TINT_IR_UNREACHABLE(ir);
        }
        c->Destroy();
    }

    /// Process a `construct` instruction.
    /// @param c the construct instruction
    void ProcessConstruct(core::ir::Construct* c) {
        auto* sm_ty = c->Result()->Type()->As<core::type::SubgroupMatrix>();

        // TODO(555437691): Handle aggregates.
        TINT_IR_ASSERT(ir, sm_ty);

        auto args = c->Args();
        core::ir::Value* value = nullptr;
        if (args.size() > 0) {
            value = args[0];
        } else {
            value = b.Zero(sm_ty->Type());
        }

        // Declare a local variable to hold the result of the construct and then use a helper to
        // fill it with the constructor value.
        b.InsertAfter(c, [&] {
            auto* tensor_type = ToCooperativeTensor(sm_ty);
            auto* ptr_type = ty.ptr(function, tensor_type, read_write);
            auto* var = b.Var(ptr_type);

            b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kFillCooperativeTensor, var, value);

            RegisterLocalVar(c->Result(), var);
        });
        c->Destroy();
    }

    /// Process a `let` instruction to replace its value.
    /// @param let the let instruction
    void ProcessLet(core::ir::Let* let) {
        auto* sm_ty = let->Result()->Type()->As<core::type::SubgroupMatrix>();

        // TODO(555437691): Handle aggregates.
        TINT_IR_ASSERT(ir, sm_ty);

        auto* init = let->Value();
        auto* local_var = value_to_local_var.GetOr(init, nullptr);
        TINT_IR_ASSERT(ir, local_var);

        if (!shared_local_vars.Contains(local_var) && local_var->Block() == let->Block()) {
            // If the initializer value is not shared, and was declared in the same block,
            // then we can just take the local variable that it allocated and use that directly.
            ir.SetName(local_var, ir.NameOf(let));
            RegisterLocalVar(let->Result(), local_var);
        } else {
            // Declare a new local variable and copy the contents of the initializer
            // cooperative_tensor into it.
            b.InsertAfter(let, [&] {
                auto* tensor_type = ToCooperativeTensor(sm_ty);
                auto* ptr_type = ty.ptr(function, tensor_type, read_write);
                auto* var = b.Var(ptr_type);
                ir.SetName(var, ir.NameOf(let));

                b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kCopyCooperativeTensor, var,
                                        local_var);

                RegisterLocalVar(let->Result(), var);
            });
        }
        let->Destroy();
    }

    /// Process a `var` instruction to replace its type and initializer.
    /// @param var the var instruction
    void ProcessVar(core::ir::Var* var) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        auto* sm_ty = ptr->StoreType()->As<core::type::SubgroupMatrix>();

        // TODO(555437691): Handle aggregates.
        TINT_IR_ASSERT(ir, sm_ty);

        // Change the type to a cooperative_tensor.
        auto* tensor_type = ToCooperativeTensor(sm_ty);
        var->Result()->SetType(ty.ptr(ptr->AddressSpace(), tensor_type, ptr->Access()));

        auto* init = var->Initializer();
        if (init) {
            auto* local_var = value_to_local_var.GetOr(init, nullptr);
            TINT_IR_ASSERT(ir, local_var);

            if (!shared_local_vars.Contains(local_var) && local_var->Block() == var->Block()) {
                // If the initializer value is not shared, and was declared in the same block, then
                // then we can just take the local variable that it allocated and use that directly.
                ir.SetName(local_var, ir.NameOf(var));
                var->Result()->ReplaceAllUsesWith(local_var->Result());
                var->Destroy();
            } else {
                // Copy the contents of the initializer cooperative_tensor into this variable.
                var->SetInitializer(nullptr);
                auto* copy = b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kCopyCooperativeTensor,
                                                     var, local_var);
                copy->InsertAfter(var);
            }
        } else {
            // Use a helper to fill the cooperative_tensor with zero values.
            auto* fill = b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kFillCooperativeTensor, var,
                                                 b.Zero(sm_ty->Type()));
            fill->InsertAfter(var);
        }
    }

    /// @returns the cooperative tensor equivalent of @p sm
    type::CooperativeTensor* ToCooperativeTensor(const core::type::SubgroupMatrix* sm) {
        // MSL needs the full MNK shape and both the input and output types to declare a
        // cooperative_tensor object. We do not have all of that information in the subgroup matrix
        // type, so fill in the missing dimension with `32` and use the element type for both the
        // input and output type.
        // TODO(555369543): We could walk the uses of this object to try and determine better values
        // for the missing information, to avoid needing to copy the data to a compatible type
        // later.
        switch (sm->Kind()) {
            case core::SubgroupMatrixKind::kLeft:
                return ty.Get<type::CooperativeTensor>(sm->Kind(), sm->Rows(), 32u, sm->Columns(),
                                                       sm->Type(), sm->Type());
            case core::SubgroupMatrixKind::kRight:
                return ty.Get<type::CooperativeTensor>(sm->Kind(), 32u, sm->Columns(), sm->Rows(),
                                                       sm->Type(), sm->Type());
            case core::SubgroupMatrixKind::kResult:
                return ty.Get<type::CooperativeTensor>(sm->Kind(), sm->Rows(), sm->Columns(), 32u,
                                                       sm->Type(), sm->Type());
            case core::SubgroupMatrixKind::kUndefined:
                TINT_IR_UNREACHABLE(ir);
        }
    }

    void ElideRedundantPointerOffset(core::ir::Value*& p) {
        if (auto* pre_cast = p->AsInstruction<msl::ir::BuiltinCall>()) {
            if (pre_cast->Func() == msl::BuiltinFn::kPointerOffset &&
                pre_cast->Args()[1] == b.Constant(u32(0))) {
                p = pre_cast->Args()[0];

                if (pre_cast->Result()->NumUsages() == 1) {
                    pre_cast->Destroy();
                }
            }
        }
    }

    core::ir::Let* MakeTensorInline(std::string_view name,
                                    core::ir::Value* p,
                                    core::ir::Value* offset,
                                    core::ir::Value* stride,
                                    const core::type::SubgroupMatrix* mat) {
        ElideRedundantPointerOffset(p);

        auto* ptr = p->Type()->As<core::type::Pointer>();
        auto* arr = ptr->StoreType()->As<core::type::Array>();
        const uint32_t arr_stride = arr->ImplicitStride();

        auto* mat_ele = mat->Type();

        core::ir::Value* data = nullptr;
        if (arr->ElemType() != mat_ele) {
            // MSL requires that pointee type match matrix element type.
            // Use pointer offset to generate the correct pointer.
            // Note: the offset needs converted to bytes.
            offset = b.InsertBitcastIfNeeded(ty.u32(), offset);
            offset = b.Multiply(offset, u32(arr_stride));
            data = b.CallExplicit<msl::ir::BuiltinCall>(
                        ty.ptr(ptr->AddressSpace(), mat_ele, ptr->Access()),
                        msl::BuiltinFn::kPointerOffset,
                        Vector<core::ir::TemplateParameter, 1>{mat_ele}, p, offset)
                       ->Result();

            // Stride is changed to elements_per_row which is in terms of matrix element type.
            stride = b.Multiply(stride, u32(arr_stride / mat_ele->Size()));
        } else {
            // Make a pointer to the first element of the array that we will access.
            auto* elem_ptr = ty.ptr(ptr->AddressSpace(), arr->ElemType(), ptr->Access());
            data = b.Access(elem_ptr, p, offset)->Result();
        }

        // The tensor extents are the dimensions of the matrix.
        auto* extents = b.Composite(ty.vec2u(), u32(mat->Columns()), u32(mat->Rows()));

        // Create a tensor_inline from the data pointer.
        return b.Let(name, b.Call<msl::ir::BuiltinCall>(ty.Get<msl::type::TensorInline>(),
                                                        msl::BuiltinFn::kMakeTensorInline, data,
                                                        extents, stride));
    }

    void ReplaceSubgroupMatrixStore(core::ir::CoreBuiltinCall* c) {
        auto* p = c->Args()[0];
        auto* offset = c->Args()[1];
        auto* value = c->Args()[2];
        auto* stride = b.InsertBitcastIfNeeded(ty.u32(), c->Args()[3]);

        auto majorness = std::get<core::Majorness>(c->ExplicitTemplateParams()[0]);
        if (majorness == core::Majorness::kColMajor) {
            // TODO(556210460): Add polyfill for column-major layouts.
            TINT_IR_UNIMPLEMENTED(ir) << "column-major layouts not yet supported";
        }

        b.InsertAfter(c, [&] {
            TINT_IR_ASSERT(ir,
                           std::holds_alternative<core::Majorness>(c->ExplicitTemplateParams()[0]));
            auto* mat = value->Type()->As<core::type::SubgroupMatrix>();

            auto* tensor_inline = MakeTensorInline("tint_dst_tensor", p, offset, stride, mat);

            // Store the cooperative_tensor value to the tensor_inline.
            auto* local_var = value_to_local_var.GetOr(value, nullptr);
            b.MemberCall<msl::ir::MemberBuiltinCall>(ty.void_(), msl::BuiltinFn::kStore,
                                                     b.Load(local_var), tensor_inline);
        });
    }
};

}  // namespace

Result<SuccessType> CooperativeTensors(core::ir::Module& ir) {
    AssertValid(ir, "before msl.CooperativeTensors");

    State{ir}.Process();

    ir.properties.Add(core::ir::Property::kAllowNonCoreTypes);

    return Success;
}

}  // namespace tint::msl::writer::raise
