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

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/store.h"
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

    /// The set of local variables that only have a single use.
    Hashset<const core::ir::Value*, 8> single_use_local_vars{};

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
                if (auto* store = inst->As<core::ir::Store>()) {
                    if (ContainsSubgroupMatrix(store->From()->Type())) {
                        worklist.Push(inst);
                    }
                    return;
                }
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
                    [&](core::ir::Access* a) { ProcessAccess(a); },         //
                    [&](core::ir::CoreBuiltinCall* c) { ProcessCall(c); },  //
                    [&](core::ir::Construct* c) { ProcessConstruct(c); },   //
                    [&](core::ir::Let* let) { ProcessLet(let); },           //
                    [&](core::ir::Load* load) { ProcessLoad(load); },       //
                    [&](core::ir::Store* store) { ProcessStore(store); },   //
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

    /// Create a cooperative_tensor equivalent type for @p type.
    /// A subgroup matrix type used as an aggregate element type will be converted into a pointer to
    /// a cooperative_tensor variable.
    const core::type::Type* RewriteType(const core::type::Type* type) {
        return tint::Switch(
            type,                                        //
            [&](const core::type::SubgroupMatrix* sm) {  //
                return ToCooperativeTensor(sm);
            },
            [&](const core::type::Array* arr) {
                // Wrap cooperative_tensor element types in a pointer.
                auto* el_ty = RewriteType(arr->ElemType());
                if (el_ty->Is<type::CooperativeTensor>()) {
                    el_ty = ty.ptr(function, el_ty, read_write);
                }
                return ty.array(el_ty, arr->ConstantCount().value());
            },
            [&](const core::type::Pointer* ptr) {
                return ty.ptr(ptr->AddressSpace(), RewriteType(ptr->StoreType()), ptr->Access());
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// @returns a new local tensor variable that replaces the result of @p inst
    core::ir::Value* MakeLocalTensorVar(const core::type::Type* type, core::ir::Instruction* inst) {
        auto* var = tint::Switch(
            type,  //
            [&](const core::type::SubgroupMatrix* sm) {
                // Declare a new local cooperative_tensor variable.
                auto* tensor_type = ToCooperativeTensor(sm);
                auto* ptr_type = ty.ptr(function, tensor_type, read_write);
                return b.Var(ptr_type)->Result();
            },
            [&](const core::type::Array* arr) {
                // Create a new local tensor variable for each element of the array.
                Vector<core::ir::Value*, 4> args;
                for (uint32_t i = 0; i < arr->ConstantCount().value(); i++) {
                    args.Push(MakeLocalTensorVar(arr->ElemType(), nullptr));
                }
                return b.Construct(RewriteType(arr), std::move(args));
            },
            [&](const core::type::Pointer* ptr) -> core::ir::Value* {  //
                return MakeLocalTensorVar(ptr->StoreType(), nullptr);
            },
            TINT_ICE_ON_NO_MATCH);

        // Do not try and replace the instruction if we are not the outermost level.
        if (inst == nullptr) {
            return var;
        }

        // If we are creating a local variable for a `var` instruction and we do not yet have a
        // pointer type, wrap the value in a new variable.
        if (inst->Is<core::ir::Var>() && !var->Type()->Is<core::type::Pointer>()) {
            var = b.Var<function>(var)->Result();
        }

        if (auto name = ir.NameOf(inst); name.IsValid()) {
            ir.SetName(var, name);
        }

        inst->Result()->ReplaceAllUsesWith(var);

        if (var->NumUsages() == 1u) {
            single_use_local_vars.Add(var);
        }

        return var;
    }

    /// @returns true if the local tensor variable created for @p pointer can be consumed by @p user
    bool CanTakeLocalTensorVar(core::ir::Value* pointer, core::ir::Instruction* user) {
        // If a value is not shared, and was declared in the same block, then we can just take the
        // local variable that it allocated and use that directly.
        auto* result = pointer->As<core::ir::InstructionResult>();
        TINT_IR_ASSERT(ir, result);
        return result->Instruction()->Block() == user->Block() &&
               single_use_local_vars.Contains(result);
    }

    /// Fill @p dest with zeros.
    void FillWithZero(core::ir::Value* dst) {
        tint::Switch(
            dst->Type()->UnwrapPtr(),  //
            [&](const type::CooperativeTensor* tensor) {
                // Use a helper to fill the local tensor variable with zero values.
                auto* el_ty = tensor->Kind() == core::SubgroupMatrixKind::kResult
                                  ? tensor->ResultType()
                                  : tensor->InputType();
                b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kFillCooperativeTensor, dst,
                                        b.Zero(el_ty));
            },
            [&](const core::type::Array* arr) {
                // Fill each array element with zero values.
                auto* el_type = arr->ElemType();
                if (dst->Type()->Is<core::type::Pointer>()) {
                    el_type = ty.ptr(function, el_type, read_write);
                }
                for (uint32_t i = 0; i < arr->ConstantCount().value(); i++) {
                    auto* el = b.Access(el_type, dst, u32(i));
                    FillWithZero(el);
                }
            },
            [&](const core::type::Pointer*) {  //
                // If there's another pointer inside the pointer, load it.
                FillWithZero(b.Load(dst)->Result());
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Copy tensor variable @p src into @p dst.
    void CopyTensorVar(core::ir::Value* dst, core::ir::Value* src) {
        tint::Switch(
            dst->Type()->UnwrapPtr(),  //
            [&](const type::CooperativeTensor*) {
                // The source could be a pointer to a pointer if it was an aggregate containing
                // local tensor variables, in which case we load here to get the actual tensor
                // pointer.
                if (src->Type()->UnwrapPtr()->Is<core::type::Pointer>()) {
                    src = b.Load(src)->Result();
                }

                b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kCopyCooperativeTensor, dst, src);
            },
            [&](const core::type::Array* arr) {
                // Copy each array element.
                auto* src_el_type = arr->ElemType();
                if (src->Type()->Is<core::type::Pointer>()) {
                    src_el_type = ty.ptr<function>(src_el_type);
                }
                auto* dst_el_type = arr->ElemType();
                if (dst->Type()->Is<core::type::Pointer>()) {
                    dst_el_type = ty.ptr<function>(dst_el_type);
                }
                for (uint32_t i = 0; i < arr->ConstantCount().value(); i++) {
                    auto* d = b.Access(dst_el_type, dst, u32(i));
                    auto* s = b.Access(src_el_type, src, u32(i));
                    CopyTensorVar(d, s);
                }
            },
            [&](const core::type::Pointer*) {
                // If there's another pointer inside the pointer, load it.
                CopyTensorVar(b.Load(dst)->Result(), src);
            },
            TINT_ICE_ON_NO_MATCH);
    }

    /// Process an `access` instruction.
    /// @param a the access instruction
    void ProcessAccess(core::ir::Access* a) {
        // Rewrite the result type. If the result is a subgroup matrix then it will be wrapped in a
        // pointer due to the local variable.
        bool is_tensor = a->Result()->Type()->UnwrapPtr()->Is<core::type::SubgroupMatrix>();
        auto* result_type = RewriteType(a->Result()->Type());
        if (is_tensor) {
            result_type = ty.ptr<function>(result_type);
        }
        a->Result()->SetType(result_type);

        // If we are producing a pointer to a subgroup matrix, we need to load the local variable
        // pointer that would have been introduced for the tensor.
        if (is_tensor && a->Object()->Type()->Is<core::type::Pointer>()) {
            auto* load = b.Load(a->Result());
            load->InsertAfter(a);
            a->Result()->ReplaceAllUsesWith(load->Result());
            load->SetOperand(0, a->Result());
        }
    }

    /// Process a `call` instruction to replace its type and initializer.
    /// @param c the construct instruction
    void ProcessCall(core::ir::CoreBuiltinCall* c) {
        switch (c->Func()) {
            case core::BuiltinFn::kSubgroupMatrixLoad:
                ReplaceSubgroupMatrixLoad(c);
                break;
            case core::BuiltinFn::kSubgroupMatrixStore:
                ReplaceSubgroupMatrixStore(c);
                break;
            case core::BuiltinFn::kSubgroupMatrixMultiply:
                ReplaceSubgroupMatrixMultiply(c);
                break;
            case core::BuiltinFn::kSubgroupMatrixMultiplyAccumulate:
                ReplaceSubgroupMatrixMultiplyAccumulate(c);
                break;
            default:
                TINT_IR_UNREACHABLE(ir);
        }
        c->Destroy();
    }

    /// Process a `construct` instruction.
    /// @param c the construct instruction
    void ProcessConstruct(core::ir::Construct* c) {
        b.InsertBefore(c, [&] {
            auto args = c->Args();
            core::ir::Value* value = nullptr;
            if (args.size() > 0) {
                value = args[0];

                auto* type = c->Result()->Type();
                auto* sm_ty = type->As<core::type::SubgroupMatrix>();
                if (sm_ty) {
                    // If we are constructing a subgroup matrix type, the argument will be used to
                    // fill every element of the matrix.
                    // Declare a local variable to hold the result of the construct and then use a
                    // helper to fill it with the constructor value.
                    auto* var = MakeLocalTensorVar(c->Result()->Type(), c);
                    b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kFillCooperativeTensor, var,
                                            value);
                    c->Destroy();
                } else {
                    // If we are constructing an aggregate, the arguments will already have local
                    // tensor variables, but we may need to introduce copies of them.
                    for (uint32_t i = 0; i < args.size(); i++) {
                        if (!CanTakeLocalTensorVar(args[i], c)) {
                            // Copy the contents of the argument into a new variable.
                            auto* var = MakeLocalTensorVar(type->Element(i), nullptr);
                            CopyTensorVar(var, args[i]);
                            c->SetOperand(core::ir::Construct::kArgsOperandOffset + i, var);
                        }
                    }

                    // Update the result type and check if its user can take ownership of the var.
                    c->Result()->SetType(RewriteType(type));
                    if (c->Result()->NumUsages() == 1u) {
                        single_use_local_vars.Add(c->Result());
                    }
                }
            } else {
                // Generate a new zero-initialized cooperative tensor variable.
                auto* var = MakeLocalTensorVar(c->Result()->Type(), c);
                FillWithZero(var);
                c->Destroy();
            }
        });
    }

    /// Process a `let` instruction to replace its value.
    /// @param let the let instruction
    void ProcessLet(core::ir::Let* let) {
        // If the let is capturing a pointer then just update the result type.
        if (let->Result()->Type()->Is<core::type::Pointer>()) {
            let->Result()->SetType(RewriteType(let->Result()->Type()));
            return;
        }

        auto* init = let->Value();

        if (CanTakeLocalTensorVar(init, let)) {
            ir.SetName(init, ir.NameOf(let));
            let->Result()->ReplaceAllUsesWith(init);
            single_use_local_vars.Remove(init);
        } else {
            // Declare a new local variable and copy the contents of the initializer
            // cooperative_tensor into it.
            b.InsertAfter(let, [&] {
                auto* var = MakeLocalTensorVar(let->Result()->Type(), let);
                CopyTensorVar(var, init);
            });
        }
        let->Destroy();
    }

    /// Process a `load` instruction to copy to a new local variable.
    /// @param load the load instruction
    void ProcessLoad(core::ir::Load* load) {
        b.InsertAfter(load, [&] {
            // TODO(557925365): Elide the copy when possible.
            auto* var = MakeLocalTensorVar(load->Result()->Type(), load);
            CopyTensorVar(var, load->From());
        });
        load->Destroy();
    }

    /// Process a `store` instruction to copy to the destination variable.
    /// @param store the store instruction
    void ProcessStore(core::ir::Store* store) {
        b.InsertBefore(store, [&] {  //
            CopyTensorVar(store->To(), store->From());
        });
        store->Destroy();
    }

    /// Process a `var` instruction to replace its type and initializer.
    /// @param var the var instruction
    void ProcessVar(core::ir::Var* var) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();

        auto* init = var->Initializer();
        if (init) {
            if (CanTakeLocalTensorVar(init, var)) {
                auto* new_var = init;
                if (!new_var->Type()->Is<core::type::Pointer>()) {
                    b.InsertAfter(var, [&] {  //
                        new_var = b.Var<function>(new_var)->Result();
                    });
                }
                ir.SetName(new_var, ir.NameOf(var));
                var->Result()->ReplaceAllUsesWith(new_var);
                var->Destroy();
                single_use_local_vars.Remove(new_var);
            } else {
                // Change the type to use cooperative_tensor and copy the initializer into it.
                var->Result()->SetType(RewriteType(ptr));
                var->SetInitializer(nullptr);
                b.InsertAfter(var, [&] {  //
                    CopyTensorVar(var->Result(), init);
                });
            }
        } else {
            // Generate a new zero-initialized cooperative tensor variable.
            b.InsertAfter(var, [&] {
                auto* new_var = MakeLocalTensorVar(ptr, var);
                FillWithZero(new_var);
            });
            var->Destroy();
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
                                    const type::CooperativeTensor* tensor) {
        ElideRedundantPointerOffset(p);

        auto* ptr = p->Type()->As<core::type::Pointer>();
        auto* arr = ptr->StoreType()->As<core::type::Array>();
        const uint32_t arr_stride = arr->ImplicitStride();

        auto* mat_ele = tensor->Kind() == core::SubgroupMatrixKind::kResult ? tensor->ResultType()
                                                                            : tensor->InputType();

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
            data = b.Access(elem_ptr, p, offset);
        }

        // The tensor extents are the dimensions of the matrix.
        uint32_t rows = 0;
        uint32_t cols = 0;
        switch (tensor->Kind()) {
            case core::SubgroupMatrixKind::kLeft:
                rows = tensor->M();
                cols = tensor->K();
                break;
            case core::SubgroupMatrixKind::kRight:
                rows = tensor->K();
                cols = tensor->N();
                break;
            case core::SubgroupMatrixKind::kResult:
                rows = tensor->M();
                cols = tensor->N();
                break;
            case core::SubgroupMatrixKind::kUndefined:
                TINT_IR_UNREACHABLE(ir);
        }
        auto* extents = b.Composite(ty.vec2u(), u32(rows), u32(cols));

        // Create a tensor_inline from the data pointer.
        return b.Let(name, b.Call<msl::ir::BuiltinCall>(ty.Get<msl::type::TensorInline>(),
                                                        msl::BuiltinFn::kMakeTensorInline, data,
                                                        extents, stride));
    }

    void ReplaceSubgroupMatrixLoad(core::ir::CoreBuiltinCall* c) {
        auto* p = c->Args()[0];
        auto* offset = c->Args()[1];
        auto* stride = b.InsertBitcastIfNeeded(ty.u32(), c->Args()[2]);

        auto majorness = std::get<core::Majorness>(c->ExplicitTemplateParams()[1]);
        if (majorness == core::Majorness::kColMajor) {
            // TODO(556210460): Add polyfill for column-major layouts.
            TINT_IR_UNIMPLEMENTED(ir) << "column-major layouts not yet supported";
        }

        b.InsertAfter(c, [&] {
            TINT_IR_ASSERT(ir,
                           std::holds_alternative<core::Majorness>(c->ExplicitTemplateParams()[1]));
            auto* sm_ty = c->Result()->Type()->As<core::type::SubgroupMatrix>();

            auto* tensor = ToCooperativeTensor(sm_ty);
            auto* tensor_inline = MakeTensorInline("tint_src_tensor", p, offset, stride, tensor);

            // Declare a local variable to hold the loaded cooperative_tensor.
            auto* var = MakeLocalTensorVar(sm_ty, c);

            // Load the cooperative_tensor value from the tensor_inline.
            b.MemberCall<msl::ir::MemberBuiltinCall>(ty.void_(), msl::BuiltinFn::kLoad, b.Load(var),
                                                     tensor_inline);
        });
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

            auto* tensor = value->Type()->UnwrapPtr()->As<type::CooperativeTensor>();
            auto* tensor_inline = MakeTensorInline("tint_dst_tensor", p, offset, stride, tensor);

            // Store the cooperative_tensor value to the tensor_inline.
            b.MemberCall<msl::ir::MemberBuiltinCall>(ty.void_(), msl::BuiltinFn::kStore,
                                                     b.Load(value), tensor_inline);
        });
    }

    void ReplaceSubgroupMatrixMultiply(core::ir::CoreBuiltinCall* c) {
        auto* lhs = c->Args()[0];
        auto* rhs = c->Args()[1];

        b.InsertAfter(c, [&] {
            // Declare a local variable for the result cooperative_tensor.
            auto* sm_ty = c->Result()->Type()->As<core::type::SubgroupMatrix>();
            auto* result_var = MakeLocalTensorVar(sm_ty, c);

            // Load the cooperative tensor values from their local variables.
            auto* load_lhs = b.Load(lhs);
            auto* load_rhs = b.Load(rhs);
            auto* load_result = b.Load(result_var);

            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kRunTensorMultiply, load_lhs,
                                         load_rhs, load_result);
        });
    }

    void ReplaceSubgroupMatrixMultiplyAccumulate(core::ir::CoreBuiltinCall* c) {
        auto* lhs = c->Args()[0];
        auto* rhs = c->Args()[1];
        auto* acc = c->Args()[2];

        b.InsertAfter(c, [&] {
            // Declare a local variable for the result cooperative_tensor.
            auto* sm_ty = c->Result()->Type()->As<core::type::SubgroupMatrix>();
            auto* result_var = MakeLocalTensorVar(sm_ty, c);

            // Copy the contents of the accumulator into the result variable.
            b.Call<ir::BuiltinCall>(ty.void_(), BuiltinFn::kCopyCooperativeTensor, result_var, acc);

            // Load the cooperative tensor values from their local variables.
            auto* load_lhs = b.Load(lhs);
            auto* load_rhs = b.Load(rhs);
            auto* load_result = b.Load(result_var);

            b.Call<msl::ir::BuiltinCall>(ty.void_(), msl::BuiltinFn::kRunTensorMultiplyAccumulate,
                                         load_lhs, load_rhs, load_result);
        });
    }
};

}  // namespace

Result<SuccessType> CooperativeTensors(core::ir::Module& ir) {
    AssertValid(ir, "before msl.CooperativeTensors");

    State{ir}.Process();

    ir.properties.Add(core::ir::Property::kAllowNonCoreTypes);
    ir.properties.Add(core::ir::Property::kAllowPointerAndHandleInAggregates);

    return Success;
}

}  // namespace tint::msl::writer::raise
