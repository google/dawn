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

#include "src/tint/lang/core/ir/transform/block_decorated_structs.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/struct.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ir::transform {

namespace {

void Run(Module* ir) {
    Builder builder(*ir);

    if (!ir->root_block) {
        return;
    }

    // Loop over module-scope declarations, looking for storage or uniform buffers.
    Vector<Var*, 8> buffer_variables;
    for (auto inst : *ir->root_block) {
        auto* var = inst->As<Var>();
        if (!var) {
            continue;
        }
        auto* ptr = var->Result()->Type()->As<type::Pointer>();
        if (!ptr || !core::IsHostShareable(ptr->AddressSpace())) {
            continue;
        }
        buffer_variables.Push(var);
    }

    // Now process the buffer variables.
    for (auto* var : buffer_variables) {
        auto* ptr = var->Result()->Type()->As<type::Pointer>();
        auto* store_ty = ptr->StoreType();

        bool wrapped = false;
        Vector<const type::StructMember*, 4> members;

        // Build the member list for the block-decorated structure.
        if (auto* str = store_ty->As<type::Struct>(); str && !str->HasFixedFootprint()) {
            // We know the original struct will only ever be used as the store type of a buffer, so
            // just redeclare it as a block-decorated struct.
            for (auto* member : str->Members()) {
                members.Push(member);
            }
        } else {
            // The original struct might be used in other places, so create a new block-decorated
            // struct that wraps the original struct.
            members.Push(ir->Types().Get<type::StructMember>(
                /* name */ ir->symbols.New(),
                /* type */ store_ty,
                /* index */ 0u,
                /* offset */ 0u,
                /* align */ store_ty->Align(),
                /* size */ store_ty->Size(),
                /* attributes */ type::StructMemberAttributes{}));
            wrapped = true;
        }

        // Create the block-decorated struct.
        auto* block_struct = ir->Types().Get<type::Struct>(
            /* name */ ir->symbols.New(),
            /* members */ members,
            /* align */ store_ty->Align(),
            /* size */ tint::RoundUp(store_ty->Align(), store_ty->Size()),
            /* size_no_padding */ store_ty->Size());
        block_struct->SetStructFlag(type::StructFlag::kBlock);

        // Replace the old variable declaration with one that uses the block-decorated struct type.
        auto* new_var =
            builder.Var(ir->Types().ptr(ptr->AddressSpace(), block_struct, ptr->Access()));
        if (var->BindingPoint()) {
            new_var->SetBindingPoint(var->BindingPoint()->group, var->BindingPoint()->binding);
        }
        var->ReplaceWith(new_var);

        // Replace uses of the old variable.
        var->Result()->ReplaceAllUsesWith([&](Usage use) -> Value* {
            if (wrapped) {
                // The structure has been wrapped, so replace all uses of the old variable with a
                // member accessor on the new variable.
                auto* access = builder.Access(var->Result()->Type(), new_var, 0_u);
                access->InsertBefore(use.instruction);
                return access->Result();
            }
            return new_var->Result();
        });
    }
}

}  // namespace

Result<SuccessType, std::string> BlockDecoratedStructs(Module* ir) {
    auto result = ValidateAndDumpIfNeeded(*ir, "BlockDecoratedStructs transform");
    if (!result) {
        return result;
    }

    Run(ir);

    return Success;
}

}  // namespace tint::ir::transform
