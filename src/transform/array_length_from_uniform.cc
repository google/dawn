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

#include "src/transform/array_length_from_uniform.h"

#include <memory>
#include <utility>

#include "src/ast/struct_block_decoration.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ArrayLengthFromUniform::Config);
TINT_INSTANTIATE_TYPEINFO(tint::transform::ArrayLengthFromUniform::Result);

namespace tint {
namespace transform {

ArrayLengthFromUniform::ArrayLengthFromUniform() = default;
ArrayLengthFromUniform::~ArrayLengthFromUniform() = default;

Output ArrayLengthFromUniform::Run(const Program* in, const DataMap& data) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  auto* cfg = data.Get<Config>();
  if (cfg == nullptr) {
    out.Diagnostics().add_error(
        "missing transform data for ArrayLengthFromUniform");
    return Output(Program(std::move(out)));
  }

  auto& sem = ctx.src->Sem();

  const char* kBufferSizeMemberName = "buffer_size";

  // Determine the size of the buffer size array.
  uint32_t max_buffer_size_index = 0;
  for (auto& idx : cfg->bindpoint_to_size_index) {
    if (idx.second > max_buffer_size_index) {
      max_buffer_size_index = idx.second;
    }
  }

  // Get (or create, on first call) the uniform buffer that will receive the
  // size of each storage buffer in the module.
  ast::Variable* buffer_size_ubo = nullptr;
  auto get_ubo = [&]() {
    if (!buffer_size_ubo) {
      auto* buffer_size_struct = ctx.dst->Structure(
          ctx.dst->Sym(),
          {ctx.dst->Member(
              kBufferSizeMemberName,
              ctx.dst->ty.array(ctx.dst->ty.u32(), max_buffer_size_index + 1))},
          ast::DecorationList{ctx.dst->create<ast::StructBlockDecoration>()});
      buffer_size_ubo = ctx.dst->Global(
          ctx.dst->Sym(), ctx.dst->ty.Of(buffer_size_struct),
          ast::StorageClass::kUniform,
          ast::DecorationList{
              ctx.dst->create<ast::GroupDecoration>(cfg->ubo_binding.group),
              ctx.dst->create<ast::BindingDecoration>(
                  cfg->ubo_binding.binding)});
    }
    return buffer_size_ubo;
  };

  // Find all calls to the arrayLength() intrinsic.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    auto* call_expr = node->As<ast::CallExpression>();
    if (!call_expr) {
      continue;
    }

    auto* call = sem.Get(call_expr);
    auto* intrinsic = call->Target()->As<sem::Intrinsic>();
    if (!intrinsic || intrinsic->Type() != sem::IntrinsicType::kArrayLength) {
      continue;
    }

    // Get the storage buffer that contains the runtime array.
    // We assume that the argument to `arrayLength` has the form
    // `&resource.array`, which requires that `InlinePointerLets` and `Simplify`
    // have been run before this transform.
    auto* param = call_expr->params()[0]->As<ast::UnaryOpExpression>();
    if (!param || param->op() != ast::UnaryOp::kAddressOf) {
      TINT_ICE(ctx.dst->Diagnostics())
          << "expected form of arrayLength argument to be &resource.array";
      break;
    }
    auto* accessor = param->expr()->As<ast::MemberAccessorExpression>();
    if (!accessor) {
      TINT_ICE(ctx.dst->Diagnostics())
          << "expected form of arrayLength argument to be &resource.array";
      break;
    }
    auto* storage_buffer_expr = accessor->structure();
    auto* storage_buffer_sem =
        sem.Get(storage_buffer_expr)->As<sem::VariableUser>();
    if (!storage_buffer_sem) {
      TINT_ICE(ctx.dst->Diagnostics())
          << "expected form of arrayLength argument to be &resource.array";
      break;
    }

    // Get the index to use for the buffer size array.
    auto binding = storage_buffer_sem->Variable()->BindingPoint();
    auto idx_itr = cfg->bindpoint_to_size_index.find(binding);
    if (idx_itr == cfg->bindpoint_to_size_index.end()) {
      ctx.dst->Diagnostics().add_error(
          "missing size index mapping for binding point (" +
          std::to_string(binding.group) + "," +
          std::to_string(binding.binding) + ")");
      continue;
    }

    // Load the total storage buffer size from the UBO.
    auto* total_storage_buffer_size = ctx.dst->IndexAccessor(
        ctx.dst->MemberAccessor(get_ubo()->symbol(), kBufferSizeMemberName),
        idx_itr->second);

    // Calculate actual array length
    //                total_storage_buffer_size - array_offset
    // array_length = ----------------------------------------
    //                             array_stride
    auto* storage_buffer_type =
        storage_buffer_sem->Type()->UnwrapRef()->As<sem::Struct>();
    auto* array_member_sem = storage_buffer_type->Members().back();
    uint32_t array_offset = array_member_sem->Offset();
    uint32_t array_stride = array_member_sem->Size();
    auto* array_length = ctx.dst->Div(
        ctx.dst->Sub(total_storage_buffer_size, array_offset), array_stride);

    ctx.Replace(call_expr, array_length);
  }

  ctx.Clone();

  return Output{Program(std::move(out)),
                std::make_unique<Result>(buffer_size_ubo ? true : false)};
}

ArrayLengthFromUniform::Config::Config(sem::BindingPoint ubo_bp)
    : ubo_binding(ubo_bp) {}
ArrayLengthFromUniform::Config::Config(const Config&) = default;
ArrayLengthFromUniform::Config::~Config() = default;

ArrayLengthFromUniform::Result::Result(bool needs_sizes)
    : needs_buffer_sizes(needs_sizes) {}
ArrayLengthFromUniform::Result::Result(const Result&) = default;
ArrayLengthFromUniform::Result::~Result() = default;

}  // namespace transform
}  // namespace tint
