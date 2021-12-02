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
#include "src/sem/function.h"
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
  Symbol params;
  Symbol plane_0;
  Symbol plane_1;
};
}  // namespace

/// State holds the current transform state
struct MultiplanarExternalTexture::State {
  /// The clone context.
  CloneContext& ctx;

  /// ProgramBuilder for the context
  ProgramBuilder& b;

  /// Desination binding locations for the expanded texture_external provided as
  /// input into the transform.
  const NewBindingPoints* new_binding_points;

  /// Symbol for the ExternalTextureParams struct
  Symbol params_struct_sym;

  /// Symbol for the textureLoadExternal function
  Symbol texture_load_external_sym;

  /// Symbol for the textureSampleExternal function
  Symbol texture_sample_external_sym;

  /// Storage for new bindings that have been created corresponding to an
  /// original texture_external binding.
  std::unordered_map<Symbol, NewBindingSymbols> new_binding_symbols;

  /// Constructor
  /// @param context the clone
  /// @param newBindingPoints the input destination binding locations for the
  /// expanded texture_external
  State(CloneContext& context, const NewBindingPoints* newBindingPoints)
      : ctx(context), b(*context.dst), new_binding_points(newBindingPoints) {}

  /// Processes the module
  void Process() {
    auto& sem = ctx.src->Sem();

    // For each texture_external binding, we replace it with a texture_2d<f32>
    // binding and create two additional bindings (one texture_2d<f32> to
    // represent the secondary plane and one uniform buffer for the
    // ExternalTextureParams struct).
    ctx.ReplaceAll([&](const ast::Variable* var) -> const ast::Variable* {
      if (!::tint::Is<ast::ExternalTexture>(var->type)) {
        return nullptr;
      }

      // If the decorations are empty, then this must be a texture_external
      // passed as a function parameter. These variables are transformed
      // elsewhere.
      if (var->decorations.empty()) {
        return nullptr;
      }

      // If we find a texture_external binding, we know we must emit the
      // ExternalTextureParams struct.
      if (!params_struct_sym.IsValid()) {
        createExtTexParamsStruct();
      }

      // The binding points for the newly introduced bindings must have been
      // provided to this transform. We fetch the new binding points by
      // providing the original texture_external binding points into the
      // passed map.
      BindingPoint bp = {var->BindingPoint().group->value,
                         var->BindingPoint().binding->value};

      BindingsMap::const_iterator it =
          new_binding_points->bindings_map.find(bp);
      if (it == new_binding_points->bindings_map.end()) {
        b.Diagnostics().add_error(
            diag::System::Transform,
            "missing new binding points for texture_external at binding {" +
                std::to_string(bp.group) + "," + std::to_string(bp.binding) +
                "}");
        return nullptr;
      }

      BindingPoints bps = it->second;

      // Symbols for the newly created bindings must be saved so they can be
      // passed as parameters later. These are placed in a map and keyed by
      // the source symbol associated with the texture_external binding that
      // corresponds with the new destination bindings.
      // NewBindingSymbols new_binding_syms;
      auto& syms = new_binding_symbols[var->symbol];
      syms.plane_0 = ctx.Clone(var->symbol);
      syms.plane_1 = b.Symbols().New("ext_tex_plane_1");
      b.Global(syms.plane_1,
               b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
               b.GroupAndBinding(bps.plane_1.group, bps.plane_1.binding));
      syms.params = b.Symbols().New("ext_tex_params");
      b.Global(syms.params, b.ty.type_name("ExternalTextureParams"),
               ast::StorageClass::kUniform,
               b.GroupAndBinding(bps.params.group, bps.params.binding));

      // Replace the original texture_external binding with a texture_2d<f32>
      // binding.
      ast::DecorationList cloned_decorations = ctx.Clone(var->decorations);
      const ast::Expression* cloned_constructor = ctx.Clone(var->constructor);

      return b.Var(syms.plane_0,
                   b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
                   cloned_constructor, cloned_decorations);
    });

    // Transform the original textureLoad and textureSampleLevel calls into
    // textureLoadExternal and textureSampleExternal calls.
    ctx.ReplaceAll(
        [&](const ast::CallExpression* expr) -> const ast::CallExpression* {
          auto* intrinsic = sem.Get(expr)->Target()->As<sem::Intrinsic>();

          if (intrinsic && !intrinsic->Parameters().empty() &&
              intrinsic->Parameters()[0]->Type()->Is<sem::ExternalTexture>() &&
              intrinsic->Type() != sem::IntrinsicType::kTextureDimensions) {
            auto it = new_binding_symbols.find(
                expr->args[0]->As<ast::IdentifierExpression>()->symbol);
            if (it == new_binding_symbols.end()) {
              // If valid new binding locations were not provided earlier, we
              // would have been unable to create these symbols. An error
              // message was emitted earlier, so just return early to avoid
              // internal compiler errors and retain a clean error message.
              return nullptr;
            }
            auto& syms = it->second;

            if (intrinsic->Type() == sem::IntrinsicType::kTextureLoad) {
              return createTexLdExt(expr, syms);
            }

            if (intrinsic->Type() == sem::IntrinsicType::kTextureSampleLevel) {
              return createTexSmpExt(expr, syms);
            }

          } else if (sem.Get(expr)->Target()->Is<sem::Function>()) {
            // The call expression may be to a user-defined function that
            // contains a texture_external parameter. These need to be expanded
            // out to multiple plane textures and the texture parameters
            // structure.
            for (const ast::Expression* arg : expr->args) {
              if (auto* id_expr = arg->As<ast::IdentifierExpression>()) {
                // Check if a parameter is a texture_external by trying to find
                // it in the transform state.
                auto it = new_binding_symbols.find(id_expr->symbol);
                if (it != new_binding_symbols.end()) {
                  auto& syms = it->second;
                  // When we find a texture_external, we must unpack it into its
                  // components.
                  ctx.Replace(id_expr, b.Expr(syms.plane_0));
                  ctx.InsertAfter(expr->args, id_expr, b.Expr(syms.plane_1));
                  ctx.InsertAfter(expr->args, id_expr, b.Expr(syms.params));
                }
              }
            }
          }

          return nullptr;
        });

    // We must update all the texture_external parameters for user declared
    // functions.
    ctx.ReplaceAll([&](const ast::Function* fn) -> const ast::Function* {
      for (const ast::Variable* param : fn->params) {
        if (::tint::Is<ast::ExternalTexture>(param->type)) {
          // If we find a texture_external, we must ensure the
          // ExternalTextureParams struct exists.
          if (!params_struct_sym.IsValid()) {
            createExtTexParamsStruct();
          }
          // When a texture_external is found, we insert all components
          // the texture_external into the parameter list. We must also place
          // the new symbols into the transform state so they can be used when
          // transforming function calls.
          auto& syms = new_binding_symbols[param->symbol];
          syms.plane_0 = ctx.Clone(param->symbol);
          syms.plane_1 = b.Symbols().New("ext_tex_plane_1");
          syms.params = b.Symbols().New("ext_tex_params");
          auto tex2d_f32 = [&] {
            return b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32());
          };
          ctx.Replace(param, b.Param(syms.plane_0, tex2d_f32()));
          ctx.InsertAfter(fn->params, param,
                          b.Param(syms.plane_1, tex2d_f32()));
          ctx.InsertAfter(
              fn->params, param,
              b.Param(syms.params, b.ty.type_name(params_struct_sym)));
        }
      }
      // Clone the function. This will use the Replace() and InsertAfter() calls
      // above.
      return nullptr;
    });
  }

  /// Creates the ExternalTextureParams struct.
  void createExtTexParamsStruct() {
    ast::StructMemberList member_list = {
        b.Member("numPlanes", b.ty.u32()), b.Member("vr", b.ty.f32()),
        b.Member("ug", b.ty.f32()), b.Member("vg", b.ty.f32()),
        b.Member("ub", b.ty.f32())};

    params_struct_sym = b.Symbols().New("ExternalTextureParams");

    b.Structure(params_struct_sym, member_list,
                ast::DecorationList{b.StructBlock()});
  }

  /// Constructs a StatementList containing all the statements making up the
  /// bodies of the textureSampleExternal and textureLoadExternal functions.
  /// @param callType determines which function body to generate
  /// @returns a statement list that makes of the body of the chosen function
  ast::StatementList createTexFnExtStatementList(sem::IntrinsicType callType) {
    using f32 = ProgramBuilder::f32;
    const ast::CallExpression* single_plane_call;
    const ast::CallExpression* plane_0_call;
    const ast::CallExpression* plane_1_call;
    if (callType == sem::IntrinsicType::kTextureSampleLevel) {
      // textureSampleLevel(plane0, smp, coord.xy, 0.0);
      single_plane_call =
          b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
      // textureSampleLevel(plane0, smp, coord.xy, 0.0);
      plane_0_call =
          b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
      // textureSampleLevel(plane1, smp, coord.xy, 0.0);
      plane_1_call =
          b.Call("textureSampleLevel", "plane1", "smp", "coord", 0.0f);
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
        b.Decl(
            b.Const("g", nullptr,
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

  /// Creates the textureSampleExternal function if needed and returns a call
  /// expression to it.
  /// @param expr the call expression being transformed
  /// @param syms the expanded symbols to be used in the new call
  /// @returns a call expression to textureSampleExternal
  const ast::CallExpression* createTexSmpExt(const ast::CallExpression* expr,
                                             NewBindingSymbols syms) {
    ast::ExpressionList params;
    const ast::Expression* plane_0_binding_param = ctx.Clone(expr->args[0]);

    if (expr->args.size() != 3) {
      TINT_ICE(Transform, b.Diagnostics())
          << "expected textureSampleLevel call with a "
             "texture_external to have 3 parameters, found "
          << expr->args.size() << " parameters";
    }

    if (!texture_sample_external_sym.IsValid()) {
      texture_sample_external_sym = b.Symbols().New("textureSampleExternal");

      // Emit the textureSampleExternal function.
      ast::VariableList varList = {
          b.Param("plane0",
                  b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32())),
          b.Param("plane1",
                  b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32())),
          b.Param("smp", b.ty.sampler(ast::SamplerKind::kSampler)),
          b.Param("coord", b.ty.vec2(b.ty.f32())),
          b.Param("params", b.ty.type_name(params_struct_sym))};

      ast::StatementList statementList =
          createTexFnExtStatementList(sem::IntrinsicType::kTextureSampleLevel);

      b.Func(texture_sample_external_sym, varList, b.ty.vec4(b.ty.f32()),
             statementList, {});
    }

    const ast::IdentifierExpression* exp = b.Expr(texture_sample_external_sym);
    params = {plane_0_binding_param, b.Expr(syms.plane_1),
              ctx.Clone(expr->args[1]), ctx.Clone(expr->args[2]),
              b.Expr(syms.params)};
    return b.Call(exp, params);
  }

  /// Creates the textureLoadExternal function if needed and returns a call
  /// expression to it.
  /// @param expr the call expression being transformed
  /// @param syms the expanded symbols to be used in the new call
  /// @returns a call expression to textureLoadExternal
  const ast::CallExpression* createTexLdExt(const ast::CallExpression* expr,
                                            NewBindingSymbols syms) {
    ast::ExpressionList params;
    const ast::Expression* plane_0_binding_param = ctx.Clone(expr->args[0]);

    if (expr->args.size() != 2) {
      TINT_ICE(Transform, b.Diagnostics())
          << "expected textureLoad call with a texture_external "
             "to have 2 parameters, found "
          << expr->args.size() << " parameters";
    }

    if (!texture_load_external_sym.IsValid()) {
      texture_load_external_sym = b.Symbols().New("textureLoadExternal");

      // Emit the textureLoadExternal function.
      ast::VariableList var_list = {
          b.Param("plane0",
                  b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32())),
          b.Param("plane1",
                  b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32())),
          b.Param("coord", b.ty.vec2(b.ty.i32())),
          b.Param("params", b.ty.type_name(params_struct_sym))};

      ast::StatementList statement_list =
          createTexFnExtStatementList(sem::IntrinsicType::kTextureLoad);

      b.Func(texture_load_external_sym, var_list, b.ty.vec4(b.ty.f32()),
             statement_list, {});
    }

    const ast::IdentifierExpression* exp = b.Expr(texture_load_external_sym);
    params = {plane_0_binding_param, b.Expr(syms.plane_1),
              ctx.Clone(expr->args[1]), b.Expr(syms.params)};
    return b.Call(exp, params);
  }
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
  auto* new_binding_points = inputs.Get<NewBindingPoints>();

  if (!new_binding_points) {
    ctx.dst->Diagnostics().add_error(
        diag::System::Transform,
        "missing new binding point data for " + std::string(TypeInfo().name));
    return;
  }

  State state(ctx, new_binding_points);

  state.Process();

  ctx.Clone();
}

}  // namespace transform
}  // namespace tint
