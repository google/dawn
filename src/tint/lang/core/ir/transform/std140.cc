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

#include "src/tint/lang/core/ir/transform/std140.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::transform::Std140);

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

namespace tint::ir::transform {

Std140::Std140() = default;

Std140::~Std140() = default;

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    Module* ir = nullptr;

    /// The IR builder.
    Builder b{*ir};

    /// The type manager.
    type::Manager& ty{ir->Types()};

    /// The symbol table.
    SymbolTable& sym{ir->symbols};

    /// Map from original type to a new type with decomposed matrices.
    Hashmap<const type::Type*, const type::Type*, 4> rewritten_types{};

    /// Map from struct member to its new index.
    Hashmap<const type::StructMember*, uint32_t, 4> member_index_map{};

    /// Map from a type to a helper function that will convert its rewritten form back to it.
    Hashmap<const type::Struct*, Function*, 4> convert_helpers{};

    /// Process the module.
    void Process() {
        if (!ir->root_block) {
            return;
        }

        // Find uniform buffers that contain matrices that need to be decomposed.
        Vector<Var*, 8> buffer_variables;
        for (auto inst : *ir->root_block) {
            auto* var = inst->As<Var>();
            if (!var || !var->Alive()) {
                continue;
            }
            auto* ptr = var->Result()->Type()->As<type::Pointer>();
            if (!ptr || ptr->AddressSpace() != builtin::AddressSpace::kUniform) {
                continue;
            }
            if (RewriteType(ptr->StoreType()) != ptr->StoreType()) {
                buffer_variables.Push(var);
            }
        }

        // Now process the buffer variables, replacing them with new variables that have decomposed
        // matrices and updating all usages of the variables.
        for (auto* var : buffer_variables) {
            // Create a new variable with the modified store type.
            const auto& bp = var->BindingPoint();
            auto* store_type = var->Result()->Type()->As<type::Pointer>()->StoreType();
            auto* new_var = b.Var(ty.ptr(uniform, RewriteType(store_type)));
            new_var->SetBindingPoint(bp->group, bp->binding);
            if (auto name = ir->NameOf(var)) {
                ir->SetName(new_var->Result(), name);
            }

            // Replace every instruction that uses the original variable.
            var->Result()->ForEachUse(
                [&](Usage use) { Replace(use.instruction, new_var->Result()); });

            // Replace the original variable with the new variable.
            var->ReplaceWith(new_var);
            var->Destroy();
        }
    }

    /// @param mat the matrix type to check
    /// @returns true if @p mat needs to be decomposed
    static bool NeedsDecomposing(const type::Matrix* mat) { return mat->ColumnStride() & 15; }

    /// Rewrite a type if necessary, decomposing contained matrices.
    /// @param type the type to rewrite
    /// @returns the new type
    const type::Type* RewriteType(const type::Type* type) {
        return rewritten_types.GetOrCreate(type, [&]() -> const type::Type* {
            return tint::Switch(
                type,
                [&](const type::Array* arr) -> const type::Type* {
                    // Create a new array with element type potentially rewritten.
                    return ty.array(RewriteType(arr->ElemType()), arr->ConstantCount().value());
                },
                [&](const type::Struct* str) -> const type::Type* {
                    bool needs_rewrite = false;
                    uint32_t member_index = 0;
                    Vector<const type::StructMember*, 4> new_members;
                    for (auto* member : str->Members()) {
                        auto* mat = member->Type()->As<type::Matrix>();
                        if (mat && NeedsDecomposing(mat)) {
                            // Decompose these matrices into a separate member for each column.
                            member_index_map.Add(member, member_index);
                            auto* col = mat->ColumnType();
                            uint32_t offset = member->Offset();
                            for (uint32_t i = 0; i < mat->columns(); i++) {
                                StringStream ss;
                                ss << member->Name().Name() << "_col" << std::to_string(i);
                                new_members.Push(ty.Get<type::StructMember>(
                                    sym.New(ss.str()), col, member_index, offset, col->Align(),
                                    col->Size(), type::StructMemberAttributes{}));
                                offset += col->Align();
                                member_index++;
                            }
                            needs_rewrite = true;
                        } else {
                            // For all other types, recursively rewrite them as necessary.
                            auto* new_member_ty = RewriteType(member->Type());
                            new_members.Push(ty.Get<type::StructMember>(
                                member->Name(), new_member_ty, member_index, member->Offset(),
                                member->Align(), member->Size(), type::StructMemberAttributes{}));
                            member_index_map.Add(member, member_index);
                            member_index++;
                            if (new_member_ty != member->Type()) {
                                needs_rewrite = true;
                            }
                        }
                    }

                    // If no members needed to be rewritten, just return the original struct.
                    if (!needs_rewrite) {
                        return str;
                    }

                    // Create a new struct with the rewritten members.
                    auto* new_str = ty.Get<type::Struct>(sym.New(str->Name().Name() + "_std140"),
                                                         std::move(new_members), str->Align(),
                                                         str->Size(), str->SizeNoPadding());
                    for (auto flag : str->StructFlags()) {
                        new_str->SetStructFlag(flag);
                    }
                    return new_str;
                },
                [&](Default) {
                    // This type cannot contain a matrix, so no changes needed.
                    return type;
                });
        });
    }

    /// Load a decomposed matrix from a structure.
    /// @param mat the matrix type
    /// @param root the root value being accessed into
    /// @param indices the access indices that get to the first column of the decomposed matrix
    /// @returns the loaded matrix
    Value* LoadMatrix(const type::Matrix* mat, Value* root, Vector<Value*, 4> indices) {
        // Load each column vector from the struct and reconstruct the original matrix type.
        Vector<Value*, 4> args;
        auto first_column = indices.Back()->As<Constant>()->Value()->ValueAs<uint32_t>();
        for (uint32_t i = 0; i < mat->columns(); i++) {
            indices.Back() = b.Constant(u32(first_column + i));
            auto* access = b.Access(ty.ptr(uniform, mat->ColumnType()), root, indices);
            args.Push(b.Load(access->Result())->Result());
        }
        return b.Construct(mat, std::move(args))->Result();
    }

    /// Convert a value that may contain decomposed matrices to a value with the original type.
    /// @param source the value to convert
    /// @param orig_ty the original type to convert type
    /// @returns the converted value
    Value* Convert(Value* source, const type::Type* orig_ty) {
        if (source->Type() == orig_ty) {
            // The type was not rewritten, so just return the source value.
            return source;
        }
        return tint::Switch(
            orig_ty,  //
            [&](const type::Struct* str) -> Value* {
                // Create a helper function that converts the struct to the original type.
                auto* helper = convert_helpers.GetOrCreate(str, [&] {
                    auto* input_str = source->Type()->As<type::Struct>();
                    auto* func = b.Function("convert_" + str->FriendlyName(), str);
                    auto* input = b.FunctionParam("input", input_str);
                    func->SetParams({input});
                    b.Append(func->Block(), [&] {
                        uint32_t index = 0;
                        Vector<Value*, 4> args;
                        for (auto* member : str->Members()) {
                            if (auto* mat = member->Type()->As<type::Matrix>();
                                mat && NeedsDecomposing(mat)) {
                                // Extract each decomposed column and reconstruct the matrix.
                                Vector<Value*, 4> columns;
                                for (uint32_t i = 0; i < mat->columns(); i++) {
                                    auto* extract = b.Access(mat->ColumnType(), input, u32(index));
                                    columns.Push(extract->Result());
                                    index++;
                                }
                                args.Push(b.Construct(mat, std::move(columns))->Result());
                            } else {
                                // Extract and convert the member.
                                auto* type = input_str->Element(index);
                                auto* extract = b.Access(type, input, u32(index));
                                args.Push(Convert(extract->Result(), member->Type()));
                                index++;
                            }
                        }

                        // Construct and return the original struct.
                        b.Return(func, b.Construct(str, std::move(args)));
                    });
                    return func;
                });

                // Call the helper function to convert the struct.
                return b.Call(str, helper, source)->Result();
            },
            [&](const type::Array* arr) -> Value* {
                // Create a loop that copies and converts each element of the array.
                auto* el_ty = source->Type()->Elements().type;
                auto* new_arr = b.Var(ty.ptr(function, arr));
                auto* loop = b.Loop();
                auto* idx = b.BlockParam("idx", ty.u32());
                auto* count = b.Constant(u32(arr->ConstantCount().value()));
                loop->Body()->SetParams({idx});
                b.Append(loop->Initializer(), [&] {  //
                    b.NextIteration(loop, b.Constant(0_u));
                });
                b.Append(loop->Body(), [&] {
                    // Loop until idx == count.
                    auto* breakif = b.If(b.Equal(ty.bool_(), idx, count));
                    b.Append(breakif->True(), [&] {  //
                        b.ExitLoop(loop);
                    });

                    // Convert arr[idx] and store to new_arr[idx];
                    auto* to = b.Access(ty.ptr(function, arr->ElemType()), new_arr, idx);
                    auto* from = b.Access(el_ty, source, idx)->Result();
                    b.Store(to, Convert(from, arr->ElemType()));

                    b.Continue(loop);
                });
                b.Append(loop->Continuing(), [&] {
                    // Increment idx.
                    auto* inc = b.Add(ty.u32(), idx, b.Constant(1_u));
                    b.NextIteration(loop, inc);
                });
                return b.Load(new_arr)->Result();
            },
            [&](Default) { return source; });
    }

    /// Replace a use of a value that contains or was derived from a decomposed matrix.
    /// @param inst the instruction to replace
    /// @param replacement the replacement value
    void Replace(Instruction* inst, Value* replacement) {
        b.InsertBefore(inst, [&] {
            tint::Switch(
                inst,  //
                [&](Access* access) {
                    // Modify the access indices to take decomposed matrices into account.
                    auto* current_type = access->Object()->Type()->UnwrapPtr();
                    Vector<Value*, 4> indices;
                    for (auto idx : access->Indices()) {
                        if (auto* str = current_type->As<type::Struct>()) {
                            uint32_t old_index = idx->As<Constant>()->Value()->ValueAs<uint32_t>();
                            uint32_t new_index = *member_index_map.Get(str->Members()[old_index]);
                            indices.Push(b.Constant(u32(new_index)));
                            current_type = str->Element(old_index);
                        } else {
                            indices.Push(idx);
                            current_type = current_type->Elements().type;
                        }

                        // If we've hit a matrix that was decomposed, load the whole matrix.
                        // Any additional accesses will extract columns instead of producing
                        // pointers.
                        if (auto* mat = current_type->As<type::Matrix>();
                            mat && NeedsDecomposing(mat)) {
                            replacement = LoadMatrix(mat, replacement, std::move(indices));
                            indices.Clear();
                        }
                    }

                    if (!indices.IsEmpty()) {
                        // Emit the access with the modified indices.
                        if (replacement->Type()->Is<type::Pointer>()) {
                            current_type = ty.ptr(uniform, RewriteType(current_type));
                        }
                        auto* new_access = b.Access(current_type, replacement, std::move(indices));
                        replacement = new_access->Result();
                    }

                    // Replace every instruction that uses the original access instruction.
                    access->Result()->ForEachUse(
                        [&](Usage use) { Replace(use.instruction, replacement); });
                    access->Destroy();
                },
                [&](Load* load) {
                    if (!replacement->Type()->Is<type::Pointer>()) {
                        // We have already loaded to a value type, so this load just folds away.
                        load->Result()->ReplaceAllUsesWith(replacement);
                    } else {
                        // Load the decomposed value and then convert it to the original type.
                        auto* decomposed = b.Load(replacement);
                        auto* converted = Convert(decomposed->Result(), load->Result()->Type());
                        load->Result()->ReplaceAllUsesWith(converted);
                    }
                    load->Destroy();
                },
                [&](LoadVectorElement* load) {
                    // We should have loaded the decomposed matrix, reconstructed it, so this is now
                    // extracting from a value type.
                    TINT_ASSERT(!replacement->Type()->Is<type::Pointer>());
                    auto* access = b.Access(load->Result()->Type(), replacement, load->Index());
                    load->Result()->ReplaceAllUsesWith(access->Result());
                    load->Destroy();
                },
                [&](Let* let) {
                    // Let instructions just fold away.
                    let->Result()->ForEachUse(
                        [&](Usage use) { Replace(use.instruction, replacement); });
                    let->Destroy();
                });
        });
    }
};

void Std140::Run(Module* ir) const {
    State{ir}.Process();
}

}  // namespace tint::ir::transform
