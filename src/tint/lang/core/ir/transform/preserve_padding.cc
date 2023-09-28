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

#include "src/tint/lang/core/ir/transform/preserve_padding.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::ir::transform {

namespace {

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module& ir;

    /// The IR builder.
    Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// The symbol table.
    SymbolTable& sym{ir.symbols};

    /// Map from a type to a helper function that will store a decomposed value.
    Hashmap<const core::type::Type*, Function*, 4> helpers{};

    /// Process the module.
    void Process() {
        // Find host-visible stores of types that contain padding bytes.
        Vector<Store*, 8> worklist;
        for (auto inst : ir.instructions.Objects()) {
            if (auto* store = inst->As<Store>(); store && store->Alive()) {
                auto* ptr = store->To()->Type()->As<core::type::Pointer>();
                if (ptr->AddressSpace() == core::AddressSpace::kStorage &&
                    ContainsPadding(ptr->StoreType())) {
                    worklist.Push(store);
                }
            }
        }

        // Replace the stores we found with calls to helper functions that decompose the accesses.
        for (auto* store : worklist) {
            auto* replacement = MakeStore(store->To(), store->From());
            store->ReplaceWith(replacement);
            store->Destroy();
        }
    }

    /// Check if a type contains padding bytes.
    /// @param type the type to check
    /// @returns true if the type contains padding bytes
    bool ContainsPadding(const type::Type* type) {
        return tint::Switch(
            type,  //
            [&](const type::Array* arr) {
                auto* elem_ty = arr->ElemType();
                if (arr->Stride() > elem_ty->Size()) {
                    return true;
                }
                return ContainsPadding(elem_ty);
            },
            [&](const type::Matrix* mat) {
                return mat->ColumnStride() > mat->ColumnType()->Size();
            },
            [&](const type::Struct* str) {
                uint32_t current_offset = 0;
                for (auto* member : str->Members()) {
                    if (member->Offset() > current_offset) {
                        return true;
                    }
                    if (ContainsPadding(member->Type())) {
                        return true;
                    }
                    current_offset += member->Type()->Size();
                }
                return (current_offset < str->Size());
            });
    }

    /// Create an instruction that stores a (possibly padded) type to memory, decomposing the access
    /// into separate components to preserve padding if necessary.
    /// @param to the pointer to store to
    /// @param value the value to store
    /// @returns the instruction that performs the store
    Instruction* MakeStore(Value* to, Value* value) {
        auto* store_type = value->Type();

        // If there are no padding bytes in this type, just use a regular store instruction.
        if (!ContainsPadding(store_type)) {
            return b.Store(to, value);
        }

        // The type contains padding bytes, so call a helper function that decomposes the accesses.
        auto* helper = helpers.GetOrCreate(store_type, [&] {
            auto* func = b.Function("tint_store_and_preserve_padding", ty.void_());
            auto* target = b.FunctionParam("target", ty.ptr(storage, store_type));
            auto* value_param = b.FunctionParam("value_param", store_type);
            func->SetParams({target, value_param});

            b.Append(func->Block(), [&] {
                tint::Switch(
                    store_type,  //
                    [&](const type::Array* arr) {
                        b.LoopRange(
                            ty, 0_u, u32(arr->ConstantCount().value()), 1_u, [&](Value* idx) {
                                auto* el_ptr =
                                    b.Access(ty.ptr(storage, arr->ElemType()), target, idx);
                                auto* el_value = b.Access(arr->ElemType(), value_param, idx);
                                MakeStore(el_ptr->Result(), el_value->Result());
                            });
                    },
                    [&](const type::Matrix* mat) {
                        for (uint32_t i = 0; i < mat->columns(); i++) {
                            auto* col_ptr =
                                b.Access(ty.ptr(storage, mat->ColumnType()), target, u32(i));
                            auto* col_value = b.Access(mat->ColumnType(), value_param, u32(i));
                            MakeStore(col_ptr->Result(), col_value->Result());
                        }
                    },
                    [&](const type::Struct* str) {
                        for (auto* member : str->Members()) {
                            auto* sub_ptr = b.Access(ty.ptr(storage, member->Type()), target,
                                                     u32(member->Index()));
                            auto* sub_value =
                                b.Access(member->Type(), value_param, u32(member->Index()));
                            MakeStore(sub_ptr->Result(), sub_value->Result());
                        }
                    });

                b.Return(func);
            });

            return func;
        });

        return b.Call(helper, to, value);
    }
};

}  // namespace

Result<SuccessType> PreservePadding(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "PreservePadding transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
