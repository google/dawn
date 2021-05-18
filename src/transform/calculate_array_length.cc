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

#include "src/transform/calculate_array_length.h"

#include <unordered_map>
#include <utility>

#include "src/ast/call_statement.h"
#include "src/program_builder.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/statement.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(
    tint::transform::CalculateArrayLength::BufferSizeIntrinsic);

namespace tint {
namespace transform {

namespace {

/// ArrayUsage describes a runtime array usage.
/// It is used as a key by the array_length_by_usage map.
struct ArrayUsage {
  ast::BlockStatement const* const block;
  sem::Node const* const buffer;
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

CalculateArrayLength::BufferSizeIntrinsic::BufferSizeIntrinsic(
    ProgramID program_id)
    : Base(program_id) {}
CalculateArrayLength::BufferSizeIntrinsic::~BufferSizeIntrinsic() = default;
std::string CalculateArrayLength::BufferSizeIntrinsic::Name() const {
  return "intrinsic_buffer_size";
}

CalculateArrayLength::BufferSizeIntrinsic*
CalculateArrayLength::BufferSizeIntrinsic::Clone(CloneContext* ctx) const {
  return ctx->dst->ASTNodes().Create<CalculateArrayLength::BufferSizeIntrinsic>(
      ctx->dst->ID());
}

CalculateArrayLength::CalculateArrayLength() = default;
CalculateArrayLength::~CalculateArrayLength() = default;

Output CalculateArrayLength::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  auto& sem = ctx.src->Sem();

  // get_buffer_size_intrinsic() emits the function decorated with
  // BufferSizeIntrinsic that is transformed by the HLSL writer into a call to
  // [RW]ByteAddressBuffer.GetDimensions().
  std::unordered_map<const sem::Struct*, Symbol> buffer_size_intrinsics;
  auto get_buffer_size_intrinsic = [&](const sem::Struct* buffer_type) {
    return utils::GetOrCreate(buffer_size_intrinsics, buffer_type, [&] {
      auto name = ctx.dst->Sym();
      auto* buffer_typename =
          ctx.dst->ty.type_name(ctx.Clone(buffer_type->Declaration()->name()));
      auto* func = ctx.dst->create<ast::Function>(
          name,
          ast::VariableList{
              // Note: The buffer parameter requires the kStorage StorageClass
              // in order for HLSL to emit this as a ByteAddressBuffer.
              ctx.dst->create<ast::Variable>(
                  ctx.dst->Sym("buffer"), ast::StorageClass::kStorage,
                  buffer_typename, true, nullptr, ast::DecorationList{}),
              ctx.dst->Param("result",
                             ctx.dst->ty.pointer(ctx.dst->ty.u32(),
                                                 ast::StorageClass::kFunction)),
          },
          ctx.dst->ty.void_(), nullptr,
          ast::DecorationList{
              ctx.dst->ASTNodes().Create<BufferSizeIntrinsic>(ctx.dst->ID()),
          },
          ast::DecorationList{});
      ctx.InsertAfter(ctx.src->AST().GlobalDeclarations(),
                      buffer_type->Declaration(), func);
      return name;
    });
  };

  std::unordered_map<ArrayUsage, Symbol, ArrayUsage::Hasher>
      array_length_by_usage;

  // Find all the arrayLength() calls...
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* call_expr = node->As<ast::CallExpression>()) {
      auto* call = sem.Get(call_expr);
      if (auto* intrinsic = call->Target()->As<sem::Intrinsic>()) {
        if (intrinsic->Type() == sem::IntrinsicType::kArrayLength) {
          // We're dealing with an arrayLength() call

          // https://gpuweb.github.io/gpuweb/wgsl.html#array-types states:
          //
          // * The last member of the structure type defining the store type for
          //   a variable in the storage storage class may be a runtime-sized
          //   array.
          // * A runtime-sized array must not be used as the store type or
          //   contained within a store type in any other cases.
          // * The type of an expression must not be a runtime-sized array type.
          //   arrayLength()
          //
          // We can assume that the arrayLength() call has a single argument of
          // the form: arrayLength(X.Y) where X is an expression that resolves
          // to the storage buffer structure, and Y is the runtime sized array.
          auto* array_expr = call_expr->params()[0];
          auto* accessor = array_expr->As<ast::MemberAccessorExpression>();
          if (!accessor) {
            TINT_ICE(ctx.dst->Diagnostics())
                << "arrayLength() expected ast::MemberAccessorExpression, got "
                << array_expr->TypeInfo().name;
            break;
          }
          auto* storage_buffer_expr = accessor->structure();
          auto* storage_buffer_sem = sem.Get(storage_buffer_expr);
          auto* storage_buffer_type =
              storage_buffer_sem->Type()->UnwrapRef()->As<sem::Struct>();

          // Generate BufferSizeIntrinsic for this storage type if we haven't
          // already
          auto buffer_size = get_buffer_size_intrinsic(storage_buffer_type);

          if (!storage_buffer_type) {
            TINT_ICE(ctx.dst->Diagnostics())
                << "arrayLength(X.Y) expected X to be sem::Struct, got "
                << storage_buffer_type->FriendlyName(ctx.src->Symbols());
            break;
          }

          // Find the current statement block
          auto* block = call->Stmt()->Block()->Declaration();

          // If the storage_buffer_expr is resolves to a variable (typically
          // true) then key the array_length from the variable. If not, key off
          // the expression semantic node, which will be unique per call to
          // arrayLength().
          const sem::Node* storage_buffer_usage = storage_buffer_sem;
          if (auto* user = storage_buffer_sem->As<sem::VariableUser>()) {
            storage_buffer_usage = user->Variable();
          }

          auto array_length = utils::GetOrCreate(
              array_length_by_usage, {block, storage_buffer_usage}, [&] {
                // First time this array length is used for this block.
                // Let's calculate it.

                // Semantic info for the runtime array structure member
                auto* array_member_sem = storage_buffer_type->Members().back();

                // Construct the variable that'll hold the result of
                // RWByteAddressBuffer.GetDimensions()
                auto* buffer_size_result = ctx.dst->Decl(
                    ctx.dst->Var(ctx.dst->Sym(), ctx.dst->ty.u32(),
                                 ast::StorageClass::kNone, ctx.dst->Expr(0u)));

                // Call storage_buffer.GetDimensions(buffer_size_result)
                auto* call_get_dims =
                    ctx.dst->create<ast::CallStatement>(ctx.dst->Call(
                        // BufferSizeIntrinsic(X, ARGS...) is
                        // translated to:
                        //  X.GetDimensions(ARGS..) by the writer
                        buffer_size, ctx.Clone(storage_buffer_expr),
                        buffer_size_result->variable()->symbol()));

                // Calculate actual array length
                //                total_storage_buffer_size - array_offset
                // array_length = ----------------------------------------
                //                             array_stride
                auto name = ctx.dst->Sym();
                uint32_t array_offset = array_member_sem->Offset();
                uint32_t array_stride = array_member_sem->Size();
                auto* array_length_var = ctx.dst->Decl(ctx.dst->Const(
                    name, ctx.dst->ty.u32(),
                    ctx.dst->Div(
                        ctx.dst->Sub(buffer_size_result->variable()->symbol(),
                                     array_offset),
                        array_stride)));

                // Insert the array length calculations at the top of the block
                ctx.InsertBefore(block->statements(), *block->begin(),
                                 buffer_size_result);
                ctx.InsertBefore(block->statements(), *block->begin(),
                                 call_get_dims);
                ctx.InsertBefore(block->statements(), *block->begin(),
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

  return Output{Program(std::move(out))};
}

}  // namespace transform
}  // namespace tint
