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

#include "src/transform/external_texture_transform.h"
#include "src/program_builder.h"

namespace tint {
namespace transform {

ExternalTextureTransform::ExternalTextureTransform() = default;
ExternalTextureTransform::~ExternalTextureTransform() = default;

Output ExternalTextureTransform::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);

  // Scan the AST nodes for external texture declarations.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* var = node->As<ast::Variable>()) {
      if (var->type()->Is<ast::ExternalTexture>()) {
        // Replace a single-plane external texture with a 2D, f32 sampled
        // texture.
        auto newType = ctx.dst->ty.sampled_texture(ast::TextureDimension::k2d,
                                                   ctx.dst->ty.f32());
        auto clonedSrc = ctx.Clone(var->source());
        auto clonedSym = ctx.Clone(var->symbol());
        auto* clonedConstructor = ctx.Clone(var->constructor());
        auto clonedDecorations = ctx.Clone(var->decorations());
        auto* newVar = ctx.dst->create<ast::Variable>(
            clonedSrc, clonedSym, var->declared_storage_class(), newType,
            var->is_const(), clonedConstructor, clonedDecorations);

        ctx.Replace(var, newVar);
      }
    }
  }

  ctx.Clone();
  return Output{Program(std::move(out))};
}

}  // namespace transform
}  // namespace tint
