// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/transform/zero_init_workgroup_memory.h"

#include <map>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/utils/containers/reverse.h"

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

    /// VarSet is a hash set of workgroup variables.
    using VarSet = Hashset<Var*, 8>;

    /// A map from variable to an ID used for sorting.
    Hashmap<Var*, uint32_t, 8> var_to_id{};

    /// A map from blocks to their directly referenced workgroup variables.
    Hashmap<Block*, VarSet, 64> block_to_direct_vars{};

    /// A map from functions to their transitively referenced workgroup variables.
    Hashmap<Function*, VarSet, 8> function_to_transitive_vars{};

    /// ArrayIndex represents a required array index for an access instruction.
    struct ArrayIndex {
        /// The size of the array that will be indexed.
        uint32_t count = 0u;
    };

    /// Index represents an index for an access instruction, which is either a constant value or
    /// an array index that will be dynamically calculated from an array size.
    using Index = std::variant<uint32_t, ArrayIndex>;

    /// Store describes a store to a sub-element of a workgroup variable.
    struct Store {
        /// The workgroup variable.
        Var* var = nullptr;
        /// The store type of the element.
        const type::Type* store_type = nullptr;
        /// The list of index operands to get to the element.
        Vector<Index, 4> indices;
    };

    /// StoreList is a list of `Store` descriptors.
    using StoreList = Vector<Store, 8>;

    /// StoreMap is a map from iteration count to a list of `Store` descriptors.
    using StoreMap = Hashmap<uint32_t, StoreList, 8>;

    /// Process the module.
    void Process() {
        if (ir.root_block->IsEmpty()) {
            return;
        }

        // Loop over module-scope variables, looking for workgroup variables.
        uint32_t next_id = 0;
        for (auto inst : *ir.root_block) {
            if (auto* var = inst->As<Var>()) {
                auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
                if (ptr && ptr->AddressSpace() == core::AddressSpace::kWorkgroup) {
                    // Record the usage of the variable for each block that references it.
                    var->Result()->ForEachUse([&](const Usage& use) {
                        block_to_direct_vars.GetOrZero(use.instruction->Block())->Add(var);
                    });
                    var_to_id.Add(var, next_id++);
                }
            }
        }

        // Process each entry point function.
        for (auto* func : ir.functions) {
            if (func->Stage() == Function::PipelineStage::kCompute) {
                ProcessEntryPoint(func);
            }
        }
    }

    /// Process an entry point function to zero-initialize the workgroup variables that it uses.
    /// @param func the entry point function
    void ProcessEntryPoint(Function* func) {
        // Get list of transitively referenced workgroup variables.
        auto vars = GetReferencedVars(func);
        if (vars.IsEmpty()) {
            return;
        }

        // Sort the variables to get deterministic output in tests.
        auto sorted_vars = vars.Vector();
        sorted_vars.Sort([&](Var* first, Var* second) {
            return *var_to_id.Get(first) < *var_to_id.Get(second);
        });

        // Build list of store descriptors for all workgroup variables.
        StoreMap stores;
        for (auto* var : sorted_vars) {
            PrepareStores(var, var->Result()->Type()->UnwrapPtr(), 1, {}, stores);
        }

        // Sort the iteration counts to get deterministic output in tests.
        auto sorted_iteration_counts = stores.Keys();
        sorted_iteration_counts.Sort();

        // Capture the first instruction of the function.
        // All new instructions will be inserted before this.
        auto* function_start = func->Block()->Front();

        // Get the local invocation index and the linearized workgroup size.
        auto* local_index = GetLocalInvocationIndex(func);
        auto wgsizes = func->WorkgroupSize().value();
        auto wgsize = wgsizes[0] * wgsizes[1] * wgsizes[2];

        // Insert instructions to zero-initialize every variable.
        b.InsertBefore(function_start, [&] {
            for (auto count : sorted_iteration_counts) {
                auto element_stores = stores.Get(count);
                if (count == 1u) {
                    // Make the first invocation in the group perform all of the non-arrayed stores.
                    auto* ifelse = b.If(b.Equal(ty.bool_(), local_index, 0_u));
                    b.Append(ifelse->True(), [&] {
                        for (auto& store : *element_stores) {
                            GenerateStore(store, count, b.Constant(0_u));
                        }
                        b.ExitIf(ifelse);
                    });
                } else {
                    // Use a loop for arrayed stores.
                    b.LoopRange(ty, local_index, u32(count), u32(wgsize), [&](Value* index) {
                        for (auto& store : *element_stores) {
                            GenerateStore(store, count, index);
                        }
                    });
                }
            }
            b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        });
    }

    /// Get the set of workgroup variables transitively referenced by @p func.
    /// @param func the function
    /// @returns the set of transitively referenced workgroup variables
    VarSet GetReferencedVars(Function* func) {
        return function_to_transitive_vars.GetOrCreate(func, [&] {
            VarSet vars;
            GetReferencedVars(func->Block(), vars);
            return vars;
        });
    }

    /// Get the set of workgroup variables transitively referenced by @p block.
    /// @param block the block
    /// @param vars the set of transitively referenced workgroup variables to populate
    void GetReferencedVars(Block* block, VarSet& vars) {
        // Add directly referenced vars.
        if (auto itr = block_to_direct_vars.Find(block)) {
            for (auto* var : *itr) {
                vars.Add(var);
            }
        }

        // Loop over instructions in the block.
        for (auto* inst : *block) {
            tint::Switch(
                inst,
                [&](UserCall* call) {
                    // Get variables referenced by a function called from this block.
                    auto callee_vars = GetReferencedVars(call->Target());
                    for (auto* var : callee_vars) {
                        vars.Add(var);
                    }
                },
                [&](ControlInstruction* ctrl) {
                    // Recurse into control instructions and gather their referenced vars.
                    ctrl->ForeachBlock([&](Block* blk) { GetReferencedVars(blk, vars); });
                });
        }
    }

    /// Recursively generate store descriptors for a workgroup variable.
    /// Determines the combined array iteration count of each inner element.
    /// @param var the workgroup variable
    /// @param type the current element type
    /// @param iteration_count the iteration count of this inner element of the variable
    /// @param indices the access indices needed to get to this element
    /// @param stores the map of stores to populate
    void PrepareStores(Var* var,
                       const type::Type* type,
                       uint32_t iteration_count,
                       Vector<Index, 4> indices,
                       StoreMap& stores) {
        // If this type can be trivially zeroed, store to the whole element.
        if (CanTriviallyZero(type)) {
            stores.GetOrZero(iteration_count)->Push(Store{var, type, indices});
            return;
        }

        tint::Switch(
            type,
            [&](const type::Array* arr) {
                // Add an array index to the list and recurse into the element type.
                TINT_ASSERT(arr->ConstantCount());
                auto count = arr->ConstantCount().value();
                auto new_indices = indices;
                if (count > 1) {
                    new_indices.Push(ArrayIndex{count});
                } else {
                    new_indices.Push(0u);
                }
                PrepareStores(var, arr->ElemType(), iteration_count * count, new_indices, stores);
            },
            [&](const type::Atomic*) {
                stores.GetOrZero(iteration_count)->Push(Store{var, type, indices});
            },
            [&](const type::Struct* str) {
                for (auto* member : str->Members()) {
                    // Add the member index to the index list and recurse into its type.
                    auto new_indices = indices;
                    new_indices.Push(member->Index());
                    PrepareStores(var, member->Type(), iteration_count, new_indices, stores);
                }
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Get or inject an entry point builtin for the local invocation index.
    /// @param func the entry point function
    /// @returns the local invocation index builtin
    Value* GetLocalInvocationIndex(Function* func) {
        // Look for an existing local_invocation_index builtin parameter.
        for (auto* param : func->Params()) {
            if (auto* str = param->Type()->As<type::Struct>()) {
                // Check each member for the local invocation index builtin attribute.
                for (auto* member : str->Members()) {
                    if (member->Attributes().builtin && member->Attributes().builtin.value() ==
                                                            BuiltinValue::kLocalInvocationIndex) {
                        auto* access = b.Access(ty.u32(), param, u32(member->Index()));
                        access->InsertBefore(func->Block()->Front());
                        return access->Result();
                    }
                }
            } else {
                // Check if the parameter is the local invocation index.
                if (param->Builtin() &&
                    param->Builtin().value() == FunctionParam::Builtin::kLocalInvocationIndex) {
                    return param;
                }
            }
        }

        // No local invocation index was found, so add one to the parameter list and use that.
        Vector<FunctionParam*, 4> params = func->Params();
        auto* param = b.FunctionParam("tint_local_index", ty.u32());
        param->SetBuiltin(FunctionParam::Builtin::kLocalInvocationIndex);
        params.Push(param);
        func->SetParams(params);
        return param;
    }

    /// Generate the store instruction for a given store descriptor.
    /// @param store the store descriptor
    /// @param total_count the total number of elements that will be zeroed
    /// @param linear_index the linear index of the single element that will be zeroed
    void GenerateStore(const Store& store, uint32_t total_count, Value* linear_index) {
        auto* to = store.var->Result();
        if (!store.indices.IsEmpty()) {
            // Build the access indices to get to the target element.
            // We walk backwards along the index list so that adjacent invocation store to
            // adjacent array elements.
            uint32_t count = 1;
            Vector<Value*, 4> indices;
            for (auto idx : Reverse(store.indices)) {
                if (std::holds_alternative<ArrayIndex>(idx)) {
                    // Array indices are computed from the linear index based on the size of the
                    // array and the size of the sub-arrays that have already been indexed.
                    auto array_index = std::get<ArrayIndex>(idx);
                    Value* index = linear_index;
                    if (count > 1) {
                        index = b.Divide(ty.u32(), index, u32(count))->Result();
                    }
                    if (total_count > count * array_index.count) {
                        index = b.Modulo(ty.u32(), index, u32(array_index.count))->Result();
                    }
                    indices.Push(index);
                    count *= array_index.count;
                } else {
                    // Constant indices are added to the list unmodified.
                    indices.Push(b.Constant(u32(std::get<uint32_t>(idx))));
                }
            }
            indices.Reverse();
            to = b.Access(ty.ptr(workgroup, store.store_type), to, indices)->Result();
        }

        // Generate the store instruction.
        if (auto* atomic = store.store_type->As<type::Atomic>()) {
            auto* zero = b.Constant(ir.constant_values.Zero(atomic->Type()));
            b.Call(ty.void_(), core::BuiltinFn::kAtomicStore, to, zero);
        } else {
            auto* zero = b.Constant(ir.constant_values.Zero(store.store_type));
            b.Store(to, zero);
        }
    }

    /// Check if a type can be efficiently zeroed with a single store. Returns `false` if there are
    /// any nested arrays or atomics.
    /// @param type the type to inspect
    /// @returns true if a variable with store type @p ty can be efficiently zeroed
    bool CanTriviallyZero(const core::type::Type* type) {
        if (type->IsAnyOf<core::type::Atomic, core::type::Array>()) {
            return false;
        }
        if (auto* str = type->As<core::type::Struct>()) {
            for (auto* member : str->Members()) {
                if (!CanTriviallyZero(member->Type())) {
                    return false;
                }
            }
        }
        return true;
    }
};

}  // namespace

Result<SuccessType> ZeroInitWorkgroupMemory(Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "ZeroInitWorkgroupMemory transform");
    if (!result) {
        return result;
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::core::ir::transform
