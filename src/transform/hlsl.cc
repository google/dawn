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

#include "src/transform/hlsl.h"

#include <utility>

#include "src/program_builder.h"
#include "src/transform/calculate_array_length.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/decompose_storage_access.h"
#include "src/transform/external_texture_transform.h"
#include "src/transform/inline_pointer_lets.h"
#include "src/transform/manager.h"
#include "src/transform/promote_initializers_to_const_var.h"
#include "src/transform/simplify.h"

namespace tint {
namespace transform {

Hlsl::Hlsl() = default;
Hlsl::~Hlsl() = default;

Output Hlsl::Run(const Program* in, const DataMap& data) {
  Manager manager;
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<InlinePointerLets>();
  // Simplify cleans up messy `*(&(expr))` expressions from InlinePointerLets.
  manager.Add<Simplify>();
  // DecomposeStorageAccess must come after InlinePointerLets as we cannot take
  // the address of calls to DecomposeStorageAccess::Intrinsic. Must also come
  // after Simplify, as we need to fold away the address-of and defers of
  // `*(&(intrinsic_load()))` expressions.
  manager.Add<DecomposeStorageAccess>();
  manager.Add<CalculateArrayLength>();
  manager.Add<ExternalTextureTransform>();
  manager.Add<PromoteInitializersToConstVar>();
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  AddEmptyEntryPoint(ctx);
  ctx.Clone();
  return Output{Program(std::move(builder))};
}

void Hlsl::AddEmptyEntryPoint(CloneContext& ctx) const {
  for (auto* func : ctx.src->AST().Functions()) {
    if (func->IsEntryPoint()) {
      return;
    }
  }
  ctx.dst->Func(ctx.dst->Symbols().New("unused_entry_point"), {},
                ctx.dst->ty.void_(), {},
                {ctx.dst->Stage(ast::PipelineStage::kCompute)});
}

}  // namespace transform
}  // namespace tint
