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
#include "src/sem/call.h"
#include "src/sem/variable.h"

namespace tint {
namespace transform {

ExternalTextureTransform::ExternalTextureTransform() = default;
ExternalTextureTransform::~ExternalTextureTransform() = default;

Output ExternalTextureTransform::Run(const Program* in, const DataMap&) {
  ProgramBuilder out;
  CloneContext ctx(&out, in);
  auto& sem = ctx.src->Sem();

  // Within this transform, usages of texture_external are replaced with a
  // texture_2d<f32>, which will allow us perform operations on a
  // texture_external without maintaining texture_external-specific code
  // generation paths in the backends.

  // When replacing instances of texture_external with texture_2d<f32> we must
  // also modify calls to the texture_external overloads of textureLoad and
  // textureSampleLevel, which unlike their texture_2d<f32> overloads do not
  // require a level parameter. To do this we identify calls to textureLoad and
  // textureSampleLevel that use texture_external as the first parameter and add
  // a parameter for the level (which is always 0).

  // Scan the AST nodes for calls to textureLoad or textureSampleLevel.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* call_expr = node->As<ast::CallExpression>()) {
      if (auto* intrinsic =
              sem.Get(call_expr)->Target()->As<sem::Intrinsic>()) {
        if (intrinsic->Type() == sem::IntrinsicType::kTextureLoad ||
            intrinsic->Type() == sem::IntrinsicType::kTextureSampleLevel) {
          // When a textureLoad or textureSampleLevel has been identified, check
          // if the first parameter is an external texture.
          if (auto* var =
                  sem.Get(call_expr->params()[0])->As<sem::VariableUser>()) {
            if (var->Variable()
                    ->Type()
                    ->UnwrapRef()
                    ->Is<sem::ExternalTexture>()) {
              if (intrinsic->Type() == sem::IntrinsicType::kTextureLoad &&
                  call_expr->params().size() != 2) {
                TINT_ICE(ctx.dst->Diagnostics())
                    << "expected textureLoad call with a texture_external to "
                       "have 2 parameters, found "
                    << call_expr->params().size() << " parameters";
              }

              if (intrinsic->Type() ==
                      sem::IntrinsicType::kTextureSampleLevel &&
                  call_expr->params().size() != 3) {
                TINT_ICE(ctx.dst->Diagnostics())
                    << "expected textureSampleLevel call with a "
                       "texture_external to have 3 parameters, found "
                    << call_expr->params().size() << " parameters";
              }

              // Replace the call with another that has the same parameters in
              // addition to a level parameter (always zero for external
              // textures).
              auto* exp = ctx.Clone(call_expr->func());
              auto* externalTextureParam = ctx.Clone(call_expr->params()[0]);

              ast::ExpressionList params;
              if (intrinsic->Type() == sem::IntrinsicType::kTextureLoad) {
                auto* coordsParam = ctx.Clone(call_expr->params()[1]);
                auto* levelParam = ctx.dst->Expr(0);
                params = {externalTextureParam, coordsParam, levelParam};
              } else if (intrinsic->Type() ==
                         sem::IntrinsicType::kTextureSampleLevel) {
                auto* samplerParam = ctx.Clone(call_expr->params()[1]);
                auto* coordsParam = ctx.Clone(call_expr->params()[2]);
                auto* levelParam = ctx.dst->Expr(0.0f);
                params = {externalTextureParam, samplerParam, coordsParam,
                          levelParam};
              }

              auto* newCall = ctx.dst->create<ast::CallExpression>(exp, params);
              ctx.Replace(call_expr, newCall);
            }
          }
        }
      }
    }
  }

  // Scan the AST nodes for external texture declarations.
  for (auto* node : ctx.src->ASTNodes().Objects()) {
    if (auto* var = node->As<ast::Variable>()) {
      if (Is<ast::ExternalTexture>(var->type())) {
        // Replace a single-plane external texture with a 2D, f32 sampled
        // texture.
        auto* newType = ctx.dst->ty.sampled_texture(ast::TextureDimension::k2d,
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
