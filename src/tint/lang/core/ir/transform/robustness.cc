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

#include "src/tint/lang/core/ir/transform/robustness.h"

#include <algorithm>
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
    /// The robustness config.
    const RobustnessConfig& config;

    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    core::type::Manager& ty{ir->Types()};

    /// Process the module.
    void Process() {
        // Find the access instructions that may need to be clamped.
        Vector<ir::Access*, 64> accesses;
        Vector<ir::LoadVectorElement*, 64> vector_loads;
        Vector<ir::StoreVectorElement*, 64> vector_stores;
        for (auto* inst : ir->instructions.Objects()) {
            if (inst->Alive()) {
                tint::Switch(
                    inst,  //
                    [&](ir::Access* access) {
                        // Check if accesses into this object should be clamped.
                        auto* ptr = access->Object()->Type()->As<type::Pointer>();
                        if (ptr) {
                            if (ShouldClamp(ptr->AddressSpace())) {
                                accesses.Push(access);
                            }
                        } else {
                            if (config.clamp_value) {
                                accesses.Push(access);
                            }
                        }
                    },
                    [&](ir::LoadVectorElement* lve) {
                        // Check if loads from this address space should be clamped.
                        auto* ptr = lve->From()->Type()->As<type::Pointer>();
                        if (ShouldClamp(ptr->AddressSpace())) {
                            vector_loads.Push(lve);
                        }
                    },
                    [&](ir::StoreVectorElement* sve) {
                        // Check if stores to this address space should be clamped.
                        auto* ptr = sve->To()->Type()->As<type::Pointer>();
                        if (ShouldClamp(ptr->AddressSpace())) {
                            vector_stores.Push(sve);
                        }
                    });
            }
        }

        // Clamp access indices.
        for (auto* access : accesses) {
            b.InsertBefore(access, [&] {  //
                ClampAccessIndices(access);
            });
        }

        // Clamp load-vector-element instructions.
        for (auto* lve : vector_loads) {
            auto* vec = lve->From()->Type()->UnwrapPtr()->As<type::Vector>();
            b.InsertBefore(lve, [&] {  //
                ClampOperand(lve, LoadVectorElement::kIndexOperandOffset,
                             b.Constant(u32(vec->Width() - 1u)));
            });
        }

        // Clamp store-vector-element instructions.
        for (auto* sve : vector_stores) {
            auto* vec = sve->To()->Type()->UnwrapPtr()->As<type::Vector>();
            b.InsertBefore(sve, [&] {  //
                ClampOperand(sve, StoreVectorElement::kIndexOperandOffset,
                             b.Constant(u32(vec->Width() - 1u)));
            });
        }

        // TODO(jrprice): Handle texture builtins.
        // TODO(jrprice): Handle config.bindings_ignored.
        // TODO(jrprice): Handle config.disable_runtime_sized_array_index_clamping.
    }

    /// Check if clamping should be applied to a particular address space.
    /// @param addrspace the address space to check
    /// @returns true if pointer accesses in @p param addrspace should be clamped
    bool ShouldClamp(AddressSpace addrspace) {
        switch (addrspace) {
            case AddressSpace::kFunction:
                return config.clamp_function;
            case AddressSpace::kPrivate:
                return config.clamp_private;
            case AddressSpace::kPushConstant:
                return config.clamp_push_constant;
            case AddressSpace::kStorage:
                return config.clamp_storage;
            case AddressSpace::kUniform:
                return config.clamp_uniform;
            case AddressSpace::kWorkgroup:
                return config.clamp_workgroup;
            case AddressSpace::kUndefined:
            case AddressSpace::kPixelLocal:
            case AddressSpace::kHandle:
            case AddressSpace::kIn:
            case AddressSpace::kOut:
                return false;
        }
        return false;
    }

    /// Clamp operand @p op_idx of @p inst to ensure it is within @p limit.
    /// @param inst the instruction
    /// @param op_idx the index of the operand that should be clamped
    /// @param limit the limit to clamp to
    void ClampOperand(ir::Instruction* inst, size_t op_idx, ir::Value* limit) {
        auto* idx = inst->Operands()[op_idx];
        auto* const_idx = idx->As<ir::Constant>();
        auto* const_limit = limit->As<ir::Constant>();

        ir::Value* clamped_idx = nullptr;
        if (const_idx && const_limit) {
            // Generate a new constant index that is clamped to the limit.
            clamped_idx = b.Constant(u32(std::min(const_idx->Value()->ValueAs<uint32_t>(),
                                                  const_limit->Value()->ValueAs<uint32_t>())));
        } else {
            // Convert the index to u32 if needed.
            if (idx->Type()->is_signed_integer_scalar()) {
                idx = b.Convert(ty.u32(), idx)->Result();
            }

            // Clamp it to the dynamic limit.
            clamped_idx = b.Call(ty.u32(), core::Function::kMin, idx, limit)->Result();
        }

        // Replace the index operand with the clamped version.
        inst->SetOperand(op_idx, clamped_idx);
    }

    /// Clamp the indices of an access instruction to ensure they are within the limits of the types
    /// that they are indexing into.
    /// @param access the access instruction
    void ClampAccessIndices(ir::Access* access) {
        auto* type = access->Object()->Type()->UnwrapPtr();
        auto indices = access->Indices();
        for (size_t i = 0; i < indices.Length(); i++) {
            auto* idx = indices[i];
            auto* const_idx = idx->As<ir::Constant>();

            // Determine the limit of the type being indexed into.
            auto limit = tint::Switch(
                type,  //
                [&](const type::Vector* vec) -> ir::Value* {
                    return b.Constant(u32(vec->Width() - 1u));
                },
                [&](const type::Matrix* mat) -> ir::Value* {
                    return b.Constant(u32(mat->columns() - 1u));
                },
                [&](const type::Array* arr) -> ir::Value* {
                    if (arr->ConstantCount()) {
                        return b.Constant(u32(arr->ConstantCount().value() - 1u));
                    }
                    TINT_ASSERT_OR_RETURN_VALUE(arr->Count()->Is<type::RuntimeArrayCount>(),
                                                nullptr);

                    auto* object = access->Object();
                    if (i > 0) {
                        // Generate a pointer to the runtime-sized array if it isn't the base of
                        // this access instruction.
                        auto* base_ptr = object->Type()->As<type::Pointer>();
                        TINT_ASSERT_OR_RETURN_VALUE(base_ptr != nullptr, nullptr);
                        TINT_ASSERT_OR_RETURN_VALUE(i == 1, nullptr);
                        auto* arr_ptr = ty.ptr(base_ptr->AddressSpace(), arr, base_ptr->Access());
                        object = b.Access(arr_ptr, object, indices[0])->Result();
                    }

                    // Use the `arrayLength` builtin to get the limit of a runtime-sized array.
                    auto* length = b.Call(ty.u32(), core::Function::kArrayLength, object);
                    return b.Subtract(ty.u32(), length, b.Constant(1_u))->Result();
                });

            // If there's a dynamic limit that needs enforced, clamp the index operand.
            if (limit) {
                ClampOperand(access, ir::Access::kIndicesOperandOffset + i, limit);
            }

            // Get the type that this index produces.
            type = const_idx ? type->Element(const_idx->Value()->ValueAs<u32>())
                             : type->Elements().type;
        }
    }
};

}  // namespace

Result<SuccessType, std::string> Robustness(Module* ir, const RobustnessConfig& config) {
    auto result = ValidateAndDumpIfNeeded(*ir, "Robustness transform");
    if (!result) {
        return result;
    }

    State{config, ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
