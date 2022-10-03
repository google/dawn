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

#include "src/tint/transform/calculate_array_length.h"

#include <unordered_map>
#include <utility>

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::CalculateArrayLength);
TINT_INSTANTIATE_TYPEINFO(tint::transform::CalculateArrayLength::BufferSizeIntrinsic);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

namespace {

/// ArrayUsage describes a runtime array usage.
/// It is used as a key by the array_length_by_usage map.
struct ArrayUsage {
    ast::BlockStatement const* const block;
    sem::Variable const* const buffer;
    bool operator==(const ArrayUsage& rhs) const {
        return block == rhs.block && buffer == rhs.buffer;
    }
    struct Hasher {
        inline std::size_t operator()(const ArrayUsage& u) const {
            return utils::Hash(u.block, u.buffer);
        }
    };
};

}  // namespace

CalculateArrayLength::BufferSizeIntrinsic::BufferSizeIntrinsic(ProgramID pid, ast::NodeID nid)
    : Base(pid, nid) {}
CalculateArrayLength::BufferSizeIntrinsic::~BufferSizeIntrinsic() = default;
std::string CalculateArrayLength::BufferSizeIntrinsic::InternalName() const {
    return "intrinsic_buffer_size";
}

const CalculateArrayLength::BufferSizeIntrinsic* CalculateArrayLength::BufferSizeIntrinsic::Clone(
    CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<CalculateArrayLength::BufferSizeIntrinsic>(
        ctx->dst->ID(), ctx->dst->AllocateNodeID());
}

CalculateArrayLength::CalculateArrayLength() = default;
CalculateArrayLength::~CalculateArrayLength() = default;

bool CalculateArrayLength::ShouldRun(const Program* program, const DataMap&) const {
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

void CalculateArrayLength::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    auto& sem = ctx.src->Sem();

    // get_buffer_size_intrinsic() emits the function decorated with
    // BufferSizeIntrinsic that is transformed by the HLSL writer into a call to
    // [RW]ByteAddressBuffer.GetDimensions().
    std::unordered_map<const sem::Reference*, Symbol> buffer_size_intrinsics;
    auto get_buffer_size_intrinsic = [&](const sem::Reference* buffer_type) {
        return utils::GetOrCreate(buffer_size_intrinsics, buffer_type, [&] {
            auto name = ctx.dst->Sym();
            auto* type = CreateASTTypeFor(ctx, buffer_type);
            auto* disable_validation =
                ctx.dst->Disable(ast::DisabledValidation::kFunctionParameter);
            ctx.dst->AST().AddFunction(ctx.dst->create<ast::Function>(
                name,
                utils::Vector{
                    ctx.dst->Param("buffer",
                                   ctx.dst->ty.pointer(type, buffer_type->AddressSpace(),
                                                       buffer_type->Access()),
                                   utils::Vector{disable_validation}),
                    ctx.dst->Param("result", ctx.dst->ty.pointer(ctx.dst->ty.u32(),
                                                                 ast::AddressSpace::kFunction)),
                },
                ctx.dst->ty.void_(), nullptr,
                utils::Vector{
                    ctx.dst->ASTNodes().Create<BufferSizeIntrinsic>(ctx.dst->ID(),
                                                                    ctx.dst->AllocateNodeID()),
                },
                utils::Empty));

            return name;
        });
    };

    std::unordered_map<ArrayUsage, Symbol, ArrayUsage::Hasher> array_length_by_usage;

    // Find all the arrayLength() calls...
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        if (auto* call_expr = node->As<ast::CallExpression>()) {
            auto* call = sem.Get(call_expr)->UnwrapMaterialize()->As<sem::Call>();
            if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                if (builtin->Type() == sem::BuiltinType::kArrayLength) {
                    // We're dealing with an arrayLength() call

                    if (auto* call_stmt = call->Stmt()->Declaration()->As<ast::CallStatement>()) {
                        if (call_stmt->expr == call_expr) {
                            // arrayLength() is used as a statement.
                            // The argument expression must be side-effect free, so just drop the
                            // statement.
                            RemoveStatement(ctx, call_stmt);
                            continue;
                        }
                    }

                    // A runtime-sized array can only appear as the store type of a variable, or the
                    // last element of a structure (which cannot itself be nested). Given that we
                    // require SimplifyPointers, we can assume that the arrayLength() call has one
                    // of two forms:
                    //   arrayLength(&struct_var.array_member)
                    //   arrayLength(&array_var)
                    auto* arg = call_expr->args[0];
                    auto* address_of = arg->As<ast::UnaryOpExpression>();
                    if (!address_of || address_of->op != ast::UnaryOp::kAddressOf) {
                        TINT_ICE(Transform, ctx.dst->Diagnostics())
                            << "arrayLength() expected address-of, got " << arg->TypeInfo().name;
                    }
                    auto* storage_buffer_expr = address_of->expr;
                    if (auto* accessor = storage_buffer_expr->As<ast::MemberAccessorExpression>()) {
                        storage_buffer_expr = accessor->structure;
                    }
                    auto* storage_buffer_sem = sem.Get<sem::VariableUser>(storage_buffer_expr);
                    if (!storage_buffer_sem) {
                        TINT_ICE(Transform, ctx.dst->Diagnostics())
                            << "expected form of arrayLength argument to be &array_var or "
                               "&struct_var.array_member";
                        break;
                    }
                    auto* storage_buffer_var = storage_buffer_sem->Variable();
                    auto* storage_buffer_type = storage_buffer_sem->Type()->As<sem::Reference>();

                    // Generate BufferSizeIntrinsic for this storage type if we haven't already
                    auto buffer_size = get_buffer_size_intrinsic(storage_buffer_type);

                    // Find the current statement block
                    auto* block = call->Stmt()->Block()->Declaration();

                    auto array_length =
                        utils::GetOrCreate(array_length_by_usage, {block, storage_buffer_var}, [&] {
                            // First time this array length is used for this block.
                            // Let's calculate it.

                            // Construct the variable that'll hold the result of
                            // RWByteAddressBuffer.GetDimensions()
                            auto* buffer_size_result = ctx.dst->Decl(ctx.dst->Var(
                                ctx.dst->Sym(), ctx.dst->ty.u32(), ctx.dst->Expr(0_u)));

                            // Call storage_buffer.GetDimensions(&buffer_size_result)
                            auto* call_get_dims = ctx.dst->CallStmt(ctx.dst->Call(
                                // BufferSizeIntrinsic(X, ARGS...) is
                                // translated to:
                                //  X.GetDimensions(ARGS..) by the writer
                                buffer_size, ctx.dst->AddressOf(ctx.Clone(storage_buffer_expr)),
                                ctx.dst->AddressOf(
                                    ctx.dst->Expr(buffer_size_result->variable->symbol))));

                            // Calculate actual array length
                            //                total_storage_buffer_size - array_offset
                            // array_length = ----------------------------------------
                            //                             array_stride
                            auto name = ctx.dst->Sym();
                            const ast::Expression* total_size =
                                ctx.dst->Expr(buffer_size_result->variable);

                            const sem::Array* array_type = Switch(
                                storage_buffer_type->StoreType(),
                                [&](const sem::Struct* str) {
                                    // The variable is a struct, so subtract the byte offset of
                                    // the array member.
                                    auto* array_member_sem = str->Members().back();
                                    total_size =
                                        ctx.dst->Sub(total_size, u32(array_member_sem->Offset()));
                                    return array_member_sem->Type()->As<sem::Array>();
                                },
                                [&](const sem::Array* arr) { return arr; });

                            if (!array_type) {
                                TINT_ICE(Transform, ctx.dst->Diagnostics())
                                    << "expected form of arrayLength argument to be "
                                       "&array_var or &struct_var.array_member";
                                return name;
                            }

                            uint32_t array_stride = array_type->Size();
                            auto* array_length_var = ctx.dst->Decl(
                                ctx.dst->Let(name, ctx.dst->ty.u32(),
                                             ctx.dst->Div(total_size, u32(array_stride))));

                            // Insert the array length calculations at the top of the block
                            ctx.InsertBefore(block->statements, block->statements[0],
                                             buffer_size_result);
                            ctx.InsertBefore(block->statements, block->statements[0],
                                             call_get_dims);
                            ctx.InsertBefore(block->statements, block->statements[0],
                                             array_length_var);
                            return name;
                        });

                    // Replace the call to arrayLength() with the array length variable
                    ctx.Replace(call_expr, ctx.dst->Expr(array_length));
                }
            }
        }
    }

    ctx.Clone();
}

}  // namespace tint::transform
