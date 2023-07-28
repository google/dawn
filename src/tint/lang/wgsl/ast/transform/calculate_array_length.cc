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

#include "src/tint/lang/wgsl/ast/transform/calculate_array_length.h"

#include <unordered_map>
#include <utility>

#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/simplify_pointers.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::CalculateArrayLength);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::CalculateArrayLength::BufferSizeIntrinsic);

namespace tint::ast::transform {
namespace {

using namespace tint::builtin::fluent_types;  // NOLINT
using namespace tint::number_suffixes;        // NOLINT

bool ShouldRun(const Program* program) {
    for (auto* fn : program->AST().Functions()) {
        if (auto* sem_fn = program->Sem().Get(fn)) {
            for (auto* builtin : sem_fn->DirectlyCalledBuiltins()) {
                if (builtin->Type() == builtin::Function::kArrayLength) {
                    return true;
                }
            }
        }
    }
    return false;
}

/// ArrayUsage describes a runtime array usage.
/// It is used as a key by the array_length_by_usage map.
struct ArrayUsage {
    BlockStatement const* const block;
    sem::Variable const* const buffer;
    bool operator==(const ArrayUsage& rhs) const {
        return block == rhs.block && buffer == rhs.buffer;
    }
    struct Hasher {
        inline std::size_t operator()(const ArrayUsage& u) const { return Hash(u.block, u.buffer); }
    };
};

}  // namespace

CalculateArrayLength::BufferSizeIntrinsic::BufferSizeIntrinsic(GenerationID pid, NodeID nid)
    : Base(pid, nid, tint::Empty) {}
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

Transform::ApplyResult CalculateArrayLength::Apply(const Program* src,
                                                   const DataMap&,
                                                   DataMap&) const {
    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};
    auto& sem = src->Sem();

    // get_buffer_size_intrinsic() emits the function decorated with
    // BufferSizeIntrinsic that is transformed by the HLSL writer into a call to
    // [RW]ByteAddressBuffer.GetDimensions().
    std::unordered_map<const type::Reference*, Symbol> buffer_size_intrinsics;
    auto get_buffer_size_intrinsic = [&](const type::Reference* buffer_type) {
        return tint::GetOrCreate(buffer_size_intrinsics, buffer_type, [&] {
            auto name = b.Sym();
            auto type = CreateASTTypeFor(ctx, buffer_type);
            auto* disable_validation = b.Disable(DisabledValidation::kFunctionParameter);
            b.Func(name,
                   tint::Vector{
                       b.Param("buffer",
                               b.ty.ptr(buffer_type->AddressSpace(), type, buffer_type->Access()),
                               tint::Vector{disable_validation}),
                       b.Param("result", b.ty.ptr<function, u32>()),
                   },
                   b.ty.void_(), nullptr,
                   tint::Vector{
                       b.ASTNodes().Create<BufferSizeIntrinsic>(b.ID(), b.AllocateNodeID()),
                   });

            return name;
        });
    };

    std::unordered_map<ArrayUsage, Symbol, ArrayUsage::Hasher> array_length_by_usage;

    // Find all the arrayLength() calls...
    for (auto* node : src->ASTNodes().Objects()) {
        if (auto* call_expr = node->As<CallExpression>()) {
            auto* call = sem.Get(call_expr)->UnwrapMaterialize()->As<sem::Call>();
            if (auto* builtin = call->Target()->As<sem::Builtin>()) {
                if (builtin->Type() == builtin::Function::kArrayLength) {
                    // We're dealing with an arrayLength() call

                    if (auto* call_stmt = call->Stmt()->Declaration()->As<CallStatement>()) {
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
                    auto* address_of = arg->As<UnaryOpExpression>();
                    if (TINT_UNLIKELY(!address_of || address_of->op != UnaryOp::kAddressOf)) {
                        TINT_ICE()
                            << "arrayLength() expected address-of, got " << arg->TypeInfo().name;
                    }
                    auto* storage_buffer_expr = address_of->expr;
                    if (auto* accessor = storage_buffer_expr->As<MemberAccessorExpression>()) {
                        storage_buffer_expr = accessor->object;
                    }
                    auto* storage_buffer_sem = sem.Get<sem::VariableUser>(storage_buffer_expr);
                    if (TINT_UNLIKELY(!storage_buffer_sem)) {
                        TINT_ICE() << "expected form of arrayLength argument to be &array_var or "
                                      "&struct_var.array_member";
                        break;
                    }
                    auto* storage_buffer_var = storage_buffer_sem->Variable();
                    auto* storage_buffer_type = storage_buffer_sem->Type()->As<type::Reference>();

                    // Generate BufferSizeIntrinsic for this storage type if we haven't already
                    auto buffer_size = get_buffer_size_intrinsic(storage_buffer_type);

                    // Find the current statement block
                    auto* block = call->Stmt()->Block()->Declaration();

                    auto array_length =
                        tint::GetOrCreate(array_length_by_usage, {block, storage_buffer_var}, [&] {
                            // First time this array length is used for this block.
                            // Let's calculate it.

                            // Construct the variable that'll hold the result of
                            // RWByteAddressBuffer.GetDimensions()
                            auto* buffer_size_result =
                                b.Decl(b.Var(b.Sym(), b.ty.u32(), b.Expr(0_u)));

                            // Call storage_buffer.GetDimensions(&buffer_size_result)
                            auto* call_get_dims = b.CallStmt(b.Call(
                                // BufferSizeIntrinsic(X, ARGS...) is
                                // translated to:
                                //  X.GetDimensions(ARGS..) by the writer
                                buffer_size, b.AddressOf(ctx.Clone(storage_buffer_expr)),
                                b.AddressOf(b.Expr(buffer_size_result->variable->name->symbol))));

                            // Calculate actual array length
                            //                total_storage_buffer_size - array_offset
                            // array_length = ----------------------------------------
                            //                             array_stride
                            auto name = b.Sym();
                            const Expression* total_size = b.Expr(buffer_size_result->variable);

                            const type::Array* array_type = Switch(
                                storage_buffer_type->StoreType(),
                                [&](const type::Struct* str) {
                                    // The variable is a struct, so subtract the byte offset of
                                    // the array member.
                                    auto* array_member_sem = str->Members().Back();
                                    total_size = b.Sub(total_size, u32(array_member_sem->Offset()));
                                    return array_member_sem->Type()->As<type::Array>();
                                },
                                [&](const type::Array* arr) { return arr; });

                            if (TINT_UNLIKELY(!array_type)) {
                                TINT_ICE() << "expected form of arrayLength argument to be "
                                              "&array_var or &struct_var.array_member";
                                return name;
                            }

                            uint32_t array_stride = array_type->Size();
                            auto* array_length_var = b.Decl(
                                b.Let(name, b.ty.u32(), b.Div(total_size, u32(array_stride))));

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
                    ctx.Replace(call_expr, b.Expr(array_length));
                }
            }
        }
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::ast::transform
