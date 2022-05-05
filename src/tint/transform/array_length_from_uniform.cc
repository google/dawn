// Copyright 2021 The Tint Authors.
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

#include "src/tint/transform/array_length_from_uniform.h"

#include <memory>
#include <string>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/simplify_pointers.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ArrayLengthFromUniform);
TINT_INSTANTIATE_TYPEINFO(tint::transform::ArrayLengthFromUniform::Config);
TINT_INSTANTIATE_TYPEINFO(tint::transform::ArrayLengthFromUniform::Result);

namespace tint::transform {

ArrayLengthFromUniform::ArrayLengthFromUniform() = default;
ArrayLengthFromUniform::~ArrayLengthFromUniform() = default;

/// Iterate over all arrayLength() builtins that operate on
/// storage buffer variables.
/// @param ctx the CloneContext.
/// @param functor of type void(const ast::CallExpression*, const
/// sem::VariableUser, const sem::GlobalVariable*). It takes in an
/// ast::CallExpression of the arrayLength call expression node, a
/// sem::VariableUser of the used storage buffer variable, and the
/// sem::GlobalVariable for the storage buffer.
template <typename F>
static void IterateArrayLengthOnStorageVar(CloneContext& ctx, F&& functor) {
    auto& sem = ctx.src->Sem();

    // Find all calls to the arrayLength() builtin.
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        auto* call_expr = node->As<ast::CallExpression>();
        if (!call_expr) {
            continue;
        }

        auto* call = sem.Get(call_expr);
        auto* builtin = call->Target()->As<sem::Builtin>();
        if (!builtin || builtin->Type() != sem::BuiltinType::kArrayLength) {
            continue;
        }

        // Get the storage buffer that contains the runtime array.
        // Since we require SimplifyPointers, we can assume that the arrayLength()
        // call has one of two forms:
        //   arrayLength(&struct_var.array_member)
        //   arrayLength(&array_var)
        auto* param = call_expr->args[0]->As<ast::UnaryOpExpression>();
        if (!param || param->op != ast::UnaryOp::kAddressOf) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "expected form of arrayLength argument to be &array_var or "
                   "&struct_var.array_member";
            break;
        }
        auto* storage_buffer_expr = param->expr;
        if (auto* accessor = param->expr->As<ast::MemberAccessorExpression>()) {
            storage_buffer_expr = accessor->structure;
        }
        auto* storage_buffer_sem = sem.Get<sem::VariableUser>(storage_buffer_expr);
        if (!storage_buffer_sem) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "expected form of arrayLength argument to be &array_var or "
                   "&struct_var.array_member";
            break;
        }

        // Get the index to use for the buffer size array.
        auto* var = tint::As<sem::GlobalVariable>(storage_buffer_sem->Variable());
        if (!var) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "storage buffer is not a global variable";
            break;
        }
        functor(call_expr, storage_buffer_sem, var);
    }
}

bool ArrayLengthFromUniform::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* fn : program->AST().Functions()) {
        if (auto* sem_fn = program->Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                if (builtin->Type() == sem::BuiltinType::kArrayLength) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ArrayLengthFromUniform::Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) const {
    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        ctx.dst->Diagnostics().add_error(
            diag::System::Transform, "missing transform data for " + std::string(TypeInfo().name));
        return;
    }

    const char* kBufferSizeMemberName = "buffer_size";

    // Determine the size of the buffer size array.
    uint32_t max_buffer_size_index = 0;

    IterateArrayLengthOnStorageVar(ctx, [&](const ast::CallExpression*, const sem::VariableUser*,
                                            const sem::GlobalVariable* var) {
        auto binding = var->BindingPoint();
        auto idx_itr = cfg->bindpoint_to_size_index.find(binding);
        if (idx_itr == cfg->bindpoint_to_size_index.end()) {
            return;
        }
        if (idx_itr->second > max_buffer_size_index) {
            max_buffer_size_index = idx_itr->second;
        }
    });

    // Get (or create, on first call) the uniform buffer that will receive the
    // size of each storage buffer in the module.
    const ast::Variable* buffer_size_ubo = nullptr;
    auto get_ubo = [&]() {
        if (!buffer_size_ubo) {
            // Emit an array<vec4<u32>, N>, where N is 1/4 number of elements.
            // We do this because UBOs require an element stride that is 16-byte
            // aligned.
            auto* buffer_size_struct = ctx.dst->Structure(
                ctx.dst->Sym(),
                {ctx.dst->Member(kBufferSizeMemberName,
                                 ctx.dst->ty.array(ctx.dst->ty.vec4(ctx.dst->ty.u32()),
                                                   u32((max_buffer_size_index / 4) + 1)))});
            buffer_size_ubo = ctx.dst->Global(
                ctx.dst->Sym(), ctx.dst->ty.Of(buffer_size_struct), ast::StorageClass::kUniform,
                ast::AttributeList{
                    ctx.dst->GroupAndBinding(cfg->ubo_binding.group, cfg->ubo_binding.binding)});
        }
        return buffer_size_ubo;
    };

    std::unordered_set<uint32_t> used_size_indices;

    IterateArrayLengthOnStorageVar(ctx, [&](const ast::CallExpression* call_expr,
                                            const sem::VariableUser* storage_buffer_sem,
                                            const sem::GlobalVariable* var) {
        auto binding = var->BindingPoint();
        auto idx_itr = cfg->bindpoint_to_size_index.find(binding);
        if (idx_itr == cfg->bindpoint_to_size_index.end()) {
            return;
        }

        uint32_t size_index = idx_itr->second;
        used_size_indices.insert(size_index);

        // Load the total storage buffer size from the UBO.
        uint32_t array_index = size_index / 4;
        auto* vec_expr = ctx.dst->IndexAccessor(
            ctx.dst->MemberAccessor(get_ubo()->symbol, kBufferSizeMemberName), u32(array_index));
        uint32_t vec_index = size_index % 4;
        auto* total_storage_buffer_size = ctx.dst->IndexAccessor(vec_expr, u32(vec_index));

        // Calculate actual array length
        //                total_storage_buffer_size - array_offset
        // array_length = ----------------------------------------
        //                             array_stride
        const ast::Expression* total_size = total_storage_buffer_size;
        auto* storage_buffer_type = storage_buffer_sem->Type()->UnwrapRef();
        const sem::Array* array_type = nullptr;
        if (auto* str = storage_buffer_type->As<sem::Struct>()) {
            // The variable is a struct, so subtract the byte offset of the array
            // member.
            auto* array_member_sem = str->Members().back();
            array_type = array_member_sem->Type()->As<sem::Array>();
            total_size = ctx.dst->Sub(total_storage_buffer_size, u32(array_member_sem->Offset()));
        } else if (auto* arr = storage_buffer_type->As<sem::Array>()) {
            array_type = arr;
        } else {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "expected form of arrayLength argument to be &array_var or "
                   "&struct_var.array_member";
            return;
        }
        auto* array_length = ctx.dst->Div(total_size, u32(array_type->Stride()));

        ctx.Replace(call_expr, array_length);
    });

    ctx.Clone();

    outputs.Add<Result>(used_size_indices);
}

ArrayLengthFromUniform::Config::Config(sem::BindingPoint ubo_bp) : ubo_binding(ubo_bp) {}
ArrayLengthFromUniform::Config::Config(const Config&) = default;
ArrayLengthFromUniform::Config& ArrayLengthFromUniform::Config::operator=(const Config&) = default;
ArrayLengthFromUniform::Config::~Config() = default;

ArrayLengthFromUniform::Result::Result(std::unordered_set<uint32_t> used_size_indices_in)
    : used_size_indices(std::move(used_size_indices_in)) {}
ArrayLengthFromUniform::Result::Result(const Result&) = default;
ArrayLengthFromUniform::Result::~Result() = default;

}  // namespace tint::transform
