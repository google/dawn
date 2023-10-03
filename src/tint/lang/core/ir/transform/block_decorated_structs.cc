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

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

void Run(Module& ir) {
    Builder builder{ir};
    type::Manager& ty{ir.Types()};

    if (ir.root_block->IsEmpty()) {
        return;
    }

    // Loop over module-scope declarations, looking for storage or uniform buffers.
    Vector<Var*, 8> buffer_variables;
    for (auto inst : *ir.root_block) {
        auto* var = inst->As<Var>();
        if (!var) {
            continue;
        }
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        if (!ptr || !core::IsHostShareable(ptr->AddressSpace())) {
            continue;
        }
        buffer_variables.Push(var);
    }

    // Now process the buffer variables.
    for (auto* var : buffer_variables) {
        auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
        auto* store_ty = ptr->StoreType();

        if (auto* str = store_ty->As<core::type::Struct>(); str && !str->HasFixedFootprint()) {
            // We know the original struct will only ever be used as the store type of a buffer, so
            // just mark it as a block-decorated struct.
            // TODO(crbug.com/tint/745): Remove the const_cast.
            const_cast<type::Struct*>(str)->SetStructFlag(type::kBlock);
            continue;
        }

        // The original struct might be used in other places, so create a new block-decorated
        // struct that wraps the original struct.
        auto inner_name = ir.symbols.New();
        auto wrapper_name = ir.symbols.New();
        auto* block_struct = ty.Struct(wrapper_name, {{inner_name, store_ty}});
        block_struct->SetStructFlag(core::type::StructFlag::kBlock);

        // Replace the old variable declaration with one that uses the block-decorated struct type.
        auto* new_var = builder.Var(ty.ptr(ptr->AddressSpace(), block_struct, ptr->Access()));
        if (var->BindingPoint()) {
            new_var->SetBindingPoint(var->BindingPoint()->group, var->BindingPoint()->binding);
        }
        var->ReplaceWith(new_var);

        // Replace uses of the old variable.
        // The structure has been wrapped, so replace all uses of the old variable with a member
        // accessor on the new variable.
        var->Result()->ReplaceAllUsesWith([&](Usage use) -> Value* {
            auto* access = builder.Access(var->Result()->Type(), new_var, 0_u);
            access->InsertBefore(use.instruction);
            return access->Result();
        });

        var->Destroy();
    }
}

}  // namespace

Result<SuccessType> BlockDecoratedStructs(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "BlockDecoratedStructs transform");
    if (!result) {
        return result;
    }

    Run(ir);

    return Success;
}

}  // namespace tint::core::ir::transform
