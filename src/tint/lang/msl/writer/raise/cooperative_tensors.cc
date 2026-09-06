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
#include "src/tint/lang/msl/type/cooperative_tensor.h"

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
                }
            });
            for (auto* inst : worklist) {
                tint::Switch(
                    inst,                                                  //
                    [&](core::ir::Construct* c) { ProcessConstruct(c); },  //
                    [&](core::ir::Let* let) { ProcessLet(let); },          //
                    [&](core::ir::Var* var) { ProcessVar(var); },          //
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

            value_to_local_var.Add(c->Result(), var);
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

        if (init->NumUsages() == 1u && local_var->Block() == let->Block()) {
            // If the initializer value is only used here and was declared in the same block,
            // then we can just take the local variable that it allocated and use that directly.
            ir.SetName(local_var, ir.NameOf(let));
            value_to_local_var.Add(let->Result(), local_var);
            let->Destroy();
        } else {
            // TODO(555778427): Copy the contents of the initializer cooperative_tensor into this
            // variable.
            TINT_IR_UNIMPLEMENTED(ir);
        }
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

            if (init->NumUsages() == 1u && local_var->Block() == var->Block()) {
                // If the initializer value is only used here and was declared in the same block,
                // then we can just take the local variable that it allocated and use that directly.
                ir.SetName(local_var, ir.NameOf(var));
                var->Result()->ReplaceAllUsesWith(local_var->Result());
                var->Destroy();
            } else {
                // TODO(555778427): Copy the contents of the initializer cooperative_tensor into
                // this variable.
                TINT_IR_UNIMPLEMENTED(ir);
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
};

}  // namespace

Result<SuccessType> CooperativeTensors(core::ir::Module& ir) {
    AssertValid(ir, "before msl.CooperativeTensors");

    State{ir}.Process();

    ir.properties.Add(core::ir::Property::kAllowNonCoreTypes);

    return Success;
}

}  // namespace tint::msl::writer::raise
