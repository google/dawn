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

#include "src/transform/multiplanar_external_texture.h"

#include <string>
#include <vector>

#include "src/ast/function.h"
#include "src/program_builder.h"
#include "src/sem/call.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::MultiplanarExternalTexture);
TINT_INSTANTIATE_TYPEINFO(
    tint::transform::MultiplanarExternalTexture::NewBindingPoints);

namespace tint {
namespace transform {
namespace {

/// This struct stores symbols for new bindings created as a result of
/// transforming a texture_external instance.
struct NewBindingSymbols {
  Symbol ext_tex_params_binding_sym;
  Symbol ext_tex_plane_1_binding_sym;
};
}  // namespace

/// State holds the current transform state
struct MultiplanarExternalTexture::State {
  /// Symbol for the ExternalTextureParams struct
  Symbol external_texture_params_struct_sym;

  /// Symbol for the textureLoadExternal function
  Symbol texture_load_external_sym;

  /// Symbol for the textureSampleExternal function
  Symbol texture_sample_external_sym;

  /// Storage for new bindings that have been created corresponding to an
  /// original texture_external binding.
  std::unordered_map<Symbol, NewBindingSymbols> new_binding_symbols;
};

MultiplanarExternalTexture::NewBindingPoints::NewBindingPoints(
    BindingsMap inputBindingsMap)
    : bindings_map(std::move(inputBindingsMap)) {}
MultiplanarExternalTexture::NewBindingPoints::~NewBindingPoints() = default;

MultiplanarExternalTexture::MultiplanarExternalTexture() = default;
MultiplanarExternalTexture::~MultiplanarExternalTexture() = default;

// Within this transform, an instance of a texture_external binding is unpacked
// into two texture_2d<f32> bindings representing two possible planes of a
// single texture and a uniform buffer binding representing a struct of
// parameters. Calls to textureLoad or textureSampleLevel that contain a
// texture_external parameter will be transformed into a newly generated version
// of the function, which can perform the desired operation on a single RGBA
// plane or on seperate Y and UV planes.
void MultiplanarExternalTexture::Run(CloneContext& ctx,
                                     const DataMap& inputs,
                                     DataMap&) {
  State state;
  auto& b = *ctx.dst;

  auto* new_binding_points = inputs.Get<NewBindingPoints>();

  if (!new_binding_points) {
    b.Diagnostics().add_error(
        diag::System::Transform,
        "missing new binding point data for " + std::string(TypeInfo().name));
    return;
  }

  auto& sem = ctx.src->Sem();

  // For each texture_external binding, we replace it with a texture_2d<f32>
  // binding and create two additional bindings (one texture_2d<f32> to
  // represent the secondary plane and one uniform buffer for the
  // ExternalTextureParams struct).
  ctx.ReplaceAll([&](const ast::Variable* var) -> const ast::Variable* {
    if (!::tint::Is<ast::ExternalTexture>(var->type)) {
      return nullptr;
    }

    // If the decorations are empty, then this must be a texture_external being
    // passed as a function parameter. We need to unpack this into multiple
    // parameters - but this hasn't been implemented so produce an error.
    if (var->decorations.empty()) {
      b.Diagnostics().add_error(
          diag::System::Transform,
          "transforming a texture_external passed as a user-defined function "
          "parameter has not been implemented.");
      return nullptr;
    }

    // If we find a texture_external binding, we know we must emit the
    // ExternalTextureParams struct.
    if (!state.external_texture_params_struct_sym.IsValid()) {
      ast::StructMemberList member_list = {
          b.Member("numPlanes", b.ty.u32()), b.Member("vr", b.ty.f32()),
          b.Member("ug", b.ty.f32()), b.Member("vg", b.ty.f32()),
          b.Member("ub", b.ty.f32())};

      state.external_texture_params_struct_sym =
          b.Symbols().New("ExternalTextureParams");

      b.Structure(state.external_texture_params_struct_sym, member_list,
                  ast::DecorationList{b.StructBlock()});
    }

    // The binding points for the newly introduced bindings must have been
    // provided to this transform. We fetch the new binding points by
    // providing the original texture_external binding points into the
    // passed map.
    BindingPoint bp = {var->BindingPoint().group->value,
                       var->BindingPoint().binding->value};
    BindingPoints bps;
    BindingsMap::const_iterator it = new_binding_points->bindings_map.find(bp);
    if (it == new_binding_points->bindings_map.end()) {
      b.Diagnostics().add_error(
          diag::System::Transform,
          "missing new binding points for texture_external at binding {" +
              std::to_string(bp.group) + "," + std::to_string(bp.binding) +
              "}");
      return nullptr;
    } else {
      bps = it->second;
    }

    // Symbols for the newly created bindings must be saved so they can be
    // passed as parameters later. These are placed in a map and keyed by
    // the symbol associated with the texture_external binding that
    // corresponds with the new bindings.
    NewBindingSymbols new_binding_syms;
    new_binding_syms.ext_tex_plane_1_binding_sym =
        b.Symbols().New("ext_tex_plane_1");
    b.Global(new_binding_syms.ext_tex_plane_1_binding_sym,
             b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
             b.GroupAndBinding(bps.plane_1.group, bps.plane_1.binding));

    new_binding_syms.ext_tex_params_binding_sym =
        b.Symbols().New("ext_tex_params");
    b.Global(new_binding_syms.ext_tex_params_binding_sym,
             b.ty.type_name("ExternalTextureParams"),
             ast::StorageClass::kUniform,
             b.GroupAndBinding(bps.params.group, bps.params.binding));

    // Replace the original texture_external binding with a texture_2d<f32>
    // binding.
    auto cloned_sym = ctx.Clone(var->symbol);
    ast::DecorationList cloned_decorations = ctx.Clone(var->decorations);
    const ast::Expression* cloned_constructor = ctx.Clone(var->constructor);
    state.new_binding_symbols[cloned_sym] = new_binding_syms;

    return b.Var(cloned_sym,
                 b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
                 cloned_constructor, cloned_decorations);
  });

  // Transform the original textureLoad and textureSampleLevel calls into
  // textureLoadExternal and textureSampleExternal calls.
  ctx.ReplaceAll([&](const ast::CallExpression* expr)
                     -> const ast::CallExpression* {
    auto* intrinsic = sem.Get(expr)->Target()->As<sem::Intrinsic>();

    if (!intrinsic ||
        !intrinsic->Parameters()[0]->Type()->Is<sem::ExternalTexture>() ||
        intrinsic->Parameters().empty() ||
        intrinsic->Type() == sem::IntrinsicType::kTextureDimensions) {
      return nullptr;
    }

    const ast::Expression* ext_tex_plane_0_binding_param =
        ctx.Clone(expr->args[0]);

    // Lookup the symbols for the new bindings using the symbol from the
    // original texture_external.
    Symbol ext_tex_plane_1_binding_sym =
        state
            .new_binding_symbols[ext_tex_plane_0_binding_param
                                     ->As<ast::IdentifierExpression>()
                                     ->symbol]
            .ext_tex_plane_1_binding_sym;
    Symbol ext_tex_params_binding_sym =
        state
            .new_binding_symbols[ext_tex_plane_0_binding_param
                                     ->As<ast::IdentifierExpression>()
                                     ->symbol]
            .ext_tex_params_binding_sym;

    // If valid new binding locations were not provided earlier, we would
    // have been unable to create these symbols. An error message was
    // emitted earlier, so just return early to avoid internal compiler
    // errors and retain a clean error message.
    if (!ext_tex_plane_1_binding_sym.IsValid() ||
        !ext_tex_params_binding_sym.IsValid()) {
      return nullptr;
    }

    ast::IdentifierExpression* exp;
    ast::ExpressionList params;

    if (intrinsic->Type() == sem::IntrinsicType::kTextureLoad) {
      if (expr->args.size() != 2) {
        TINT_ICE(Transform, b.Diagnostics())
            << "expected textureLoad call with a texture_external "
               "to "
               "have 2 parameters, found "
            << expr->args.size() << " parameters";
      }

      if (!state.texture_load_external_sym.IsValid()) {
        state.texture_load_external_sym =
            b.Symbols().New("textureLoadExternal");

        // Emit the textureLoadExternal function.
        ast::VariableList var_list = {
            b.Param("plane0", b.ty.sampled_texture(ast::TextureDimension::k2d,
                                                   b.ty.f32())),
            b.Param("plane1", b.ty.sampled_texture(ast::TextureDimension::k2d,
                                                   b.ty.f32())),
            b.Param("coord", b.ty.vec2(b.ty.i32())),
            b.Param("params",
                    b.ty.type_name(state.external_texture_params_struct_sym))};

        ast::StatementList statement_list =
            createTexFnExtStatementList(b, sem::IntrinsicType::kTextureLoad);

        b.Func(state.texture_load_external_sym, var_list, b.ty.vec4(b.ty.f32()),
               statement_list, {});
      }

      exp =
          b.create<ast::IdentifierExpression>(state.texture_load_external_sym);
      params = {ext_tex_plane_0_binding_param,
                b.Expr(ext_tex_plane_1_binding_sym), ctx.Clone(expr->args[1]),
                b.Expr(ext_tex_params_binding_sym)};
    } else if (intrinsic->Type() == sem::IntrinsicType::kTextureSampleLevel) {
      if (expr->args.size() != 3) {
        TINT_ICE(Transform, b.Diagnostics())
            << "expected textureSampleLevel call with a "
               "texture_external to have 3 parameters, found "
            << expr->args.size() << " parameters";
      }

      if (!state.texture_sample_external_sym.IsValid()) {
        state.texture_sample_external_sym =
            b.Symbols().New("textureSampleExternal");

        // Emit the textureSampleExternal function.
        ast::VariableList varList = {
            b.Param("plane0", b.ty.sampled_texture(ast::TextureDimension::k2d,
                                                   b.ty.f32())),
            b.Param("plane1", b.ty.sampled_texture(ast::TextureDimension::k2d,
                                                   b.ty.f32())),
            b.Param("smp", b.ty.sampler(ast::SamplerKind::kSampler)),
            b.Param("coord", b.ty.vec2(b.ty.f32())),
            b.Param("params",
                    b.ty.type_name(state.external_texture_params_struct_sym))};

        ast::StatementList statementList = createTexFnExtStatementList(
            b, sem::IntrinsicType::kTextureSampleLevel);

        b.Func(state.texture_sample_external_sym, varList,
               b.ty.vec4(b.ty.f32()), statementList, {});
      }
      exp = b.create<ast::IdentifierExpression>(
          state.texture_sample_external_sym);
      params = {ext_tex_plane_0_binding_param,
                b.Expr(ext_tex_plane_1_binding_sym), ctx.Clone(expr->args[1]),
                ctx.Clone(expr->args[2]), b.Expr(ext_tex_params_binding_sym)};
    }

    return b.Call(exp, params);
  });

  ctx.Clone();
}

// Constructs a StatementList containing all the statements making up the bodies
// of the textureSampleExternal and textureLoadExternal functions.
ast::StatementList MultiplanarExternalTexture::createTexFnExtStatementList(
    ProgramBuilder& b,
    sem::IntrinsicType callType) {
  using f32 = ProgramBuilder::f32;
  const ast::CallExpression* single_plane_call;
  const ast::CallExpression* plane_0_call;
  const ast::CallExpression* plane_1_call;
  if (callType == sem::IntrinsicType::kTextureSampleLevel) {
    // textureSampleLevel(plane0, smp, coord.xy, 0.0);
    single_plane_call =
        b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
    // textureSampleLevel(plane0, smp, coord.xy, 0.0);
    plane_0_call = b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
    // textureSampleLevel(plane1, smp, coord.xy, 0.0);
    plane_1_call = b.Call("textureSampleLevel", "plane1", "smp", "coord", 0.0f);
  } else if (callType == sem::IntrinsicType::kTextureLoad) {
    // textureLoad(plane0, coords.xy, 0);
    single_plane_call = b.Call("textureLoad", "plane0", "coord", 0);
    // textureLoad(plane0, coords.xy, 0);
    plane_0_call = b.Call("textureLoad", "plane0", "coord", 0);
    // textureLoad(plane1, coords.xy, 0);
    plane_1_call = b.Call("textureLoad", "plane1", "coord", 0);
  }

  return {
      // if (params.numPlanes == 1u) {
      //    return singlePlaneCall
      // }
      b.If(b.create<ast::BinaryExpression>(
               ast::BinaryOp::kEqual, b.MemberAccessor("params", "numPlanes"),
               b.Expr(1u)),
           b.Block(b.Return(single_plane_call))),
      // let y = plane0Call.r - 0.0625;
      b.Decl(b.Const("y", nullptr,
                     b.Sub(b.MemberAccessor(plane_0_call, "r"), 0.0625f))),
      // let uv = plane1Call.rg - 0.5;
      b.Decl(b.Const("uv", nullptr,
                     b.Sub(b.MemberAccessor(plane_1_call, "rg"), 0.5f))),
      // let u = uv.x;
      b.Decl(b.Const("u", nullptr, b.MemberAccessor("uv", "x"))),
      // let v = uv.y;
      b.Decl(b.Const("v", nullptr, b.MemberAccessor("uv", "y"))),
      // let r = 1.164 * y + params.vr * v;
      b.Decl(b.Const("r", nullptr,
                     b.Add(b.Mul(1.164f, "y"),
                           b.Mul(b.MemberAccessor("params", "vr"), "v")))),
      // let g = 1.164 * y - params.ug * u - params.vg * v;
      b.Decl(b.Const("g", nullptr,
                     b.Sub(b.Sub(b.Mul(1.164f, "y"),
                                 b.Mul(b.MemberAccessor("params", "ug"), "u")),
                           b.Mul(b.MemberAccessor("params", "vg"), "v")))),
      // let b = 1.164 * y + params.ub * u;
      b.Decl(b.Const("b", nullptr,
                     b.Add(b.Mul(1.164f, "y"),
                           b.Mul(b.MemberAccessor("params", "ub"), "u")))),
      // return vec4<f32>(r, g, b, 1.0);
      b.Return(b.vec4<f32>("r", "g", "b", 1.0f)),
  };
}

}  // namespace transform
}  // namespace tint
