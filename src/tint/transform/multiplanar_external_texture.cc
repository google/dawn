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

#include "src/tint/transform/multiplanar_external_texture.h"

#include <string>
#include <vector>

#include "src/tint/ast/function.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::MultiplanarExternalTexture);
TINT_INSTANTIATE_TYPEINFO(
    tint::transform::MultiplanarExternalTexture::NewBindingPoints);

namespace tint::transform {
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

  /// Destination binding locations for the expanded texture_external provided
  /// as input into the transform.
  const NewBindingPoints* new_binding_points;

  /// Symbol for the ExternalTextureParams struct
  Symbol params_struct_sym;

  /// Symbol for the textureLoadExternal function
  Symbol texture_load_external_sym;

  /// Symbol for the textureSampleExternal function
  Symbol texture_sample_external_sym;

  /// Storage for new bindings that have been created corresponding to an
  /// original texture_external binding.
  std::unordered_map<const sem::Variable*, NewBindingSymbols>
      new_binding_symbols;

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
    for (auto* var : ctx.src->AST().GlobalVariables()) {
      auto* sem_var = sem.Get(var);
      if (!sem_var->Type()->UnwrapRef()->Is<sem::ExternalTexture>()) {
        continue;
      }

      // If the attributes are empty, then this must be a texture_external
      // passed as a function parameter. These variables are transformed
      // elsewhere.
      if (var->attributes.empty()) {
        continue;
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
        continue;
      }

      BindingPoints bps = it->second;

      // Symbols for the newly created bindings must be saved so they can be
      // passed as parameters later. These are placed in a map and keyed by
      // the source symbol associated with the texture_external binding that
      // corresponds with the new destination bindings.
      // NewBindingSymbols new_binding_syms;
      auto& syms = new_binding_symbols[sem_var];
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
      ast::AttributeList cloned_attributes = ctx.Clone(var->attributes);
      const ast::Expression* cloned_constructor = ctx.Clone(var->constructor);

      auto* replacement =
          b.Var(syms.plane_0,
                b.ty.sampled_texture(ast::TextureDimension::k2d, b.ty.f32()),
                cloned_constructor, cloned_attributes);
      ctx.Replace(var, replacement);
    }

    // We must update all the texture_external parameters for user declared
    // functions.
    for (auto* fn : ctx.src->AST().Functions()) {
      for (const ast::Variable* param : fn->params) {
        if (auto* sem_var = sem.Get(param)) {
          if (!sem_var->Type()->UnwrapRef()->Is<sem::ExternalTexture>()) {
            continue;
          }
          // If we find a texture_external, we must ensure the
          // ExternalTextureParams struct exists.
          if (!params_struct_sym.IsValid()) {
            createExtTexParamsStruct();
          }
          // When a texture_external is found, we insert all components
          // the texture_external into the parameter list. We must also place
          // the new symbols into the transform state so they can be used when
          // transforming function calls.
          auto& syms = new_binding_symbols[sem_var];
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
    }

    // Transform the original textureLoad and textureSampleLevel calls into
    // textureLoadExternal and textureSampleExternal calls.
    ctx.ReplaceAll(
        [&](const ast::CallExpression* expr) -> const ast::CallExpression* {
          auto* builtin = sem.Get(expr)->Target()->As<sem::Builtin>();

          if (builtin && !builtin->Parameters().empty() &&
              builtin->Parameters()[0]->Type()->Is<sem::ExternalTexture>() &&
              builtin->Type() != sem::BuiltinType::kTextureDimensions) {
            if (auto* var_user = sem.Get<sem::VariableUser>(expr->args[0])) {
              auto it = new_binding_symbols.find(var_user->Variable());
              if (it == new_binding_symbols.end()) {
                // If valid new binding locations were not provided earlier, we
                // would have been unable to create these symbols. An error
                // message was emitted earlier, so just return early to avoid
                // internal compiler errors and retain a clean error message.
                return nullptr;
              }
              auto& syms = it->second;

              if (builtin->Type() == sem::BuiltinType::kTextureLoad) {
                return createTexLdExt(expr, syms);
              }

              if (builtin->Type() == sem::BuiltinType::kTextureSampleLevel) {
                return createTexSmpExt(expr, syms);
              }
            }

          } else if (sem.Get(expr)->Target()->Is<sem::Function>()) {
            // The call expression may be to a user-defined function that
            // contains a texture_external parameter. These need to be expanded
            // out to multiple plane textures and the texture parameters
            // structure.
            for (auto* arg : expr->args) {
              if (auto* var_user = sem.Get<sem::VariableUser>(arg)) {
                // Check if a parameter is a texture_external by trying to find
                // it in the transform state.
                auto it = new_binding_symbols.find(var_user->Variable());
                if (it != new_binding_symbols.end()) {
                  auto& syms = it->second;
                  // When we find a texture_external, we must unpack it into its
                  // components.
                  ctx.Replace(arg, b.Expr(syms.plane_0));
                  ctx.InsertAfter(expr->args, arg, b.Expr(syms.plane_1));
                  ctx.InsertAfter(expr->args, arg, b.Expr(syms.params));
                }
              }
            }
          }

          return nullptr;
        });
  }

  /// Creates the ExternalTextureParams struct.
  void createExtTexParamsStruct() {
    ast::StructMemberList member_list = {
        b.Member("numPlanes", b.ty.u32()),
        b.Member("yuvToRgbConversionMatrix", b.ty.mat3x4(b.ty.f32()))};

    params_struct_sym = b.Symbols().New("ExternalTextureParams");

    b.Structure(params_struct_sym, member_list);
  }

  /// Constructs a StatementList containing all the statements making up the
  /// bodies of the textureSampleExternal and textureLoadExternal functions.
  /// @param call_type determines which function body to generate
  /// @returns a statement list that makes of the body of the chosen function
  ast::StatementList createTexFnExtStatementList(sem::BuiltinType call_type) {
    using f32 = ProgramBuilder::f32;
    const ast::CallExpression* single_plane_call = nullptr;
    const ast::CallExpression* plane_0_call = nullptr;
    const ast::CallExpression* plane_1_call = nullptr;
    if (call_type == sem::BuiltinType::kTextureSampleLevel) {
      // textureSampleLevel(plane0, smp, coord.xy, 0.0);
      single_plane_call =
          b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
      // textureSampleLevel(plane0, smp, coord.xy, 0.0);
      plane_0_call =
          b.Call("textureSampleLevel", "plane0", "smp", "coord", 0.0f);
      // textureSampleLevel(plane1, smp, coord.xy, 0.0);
      plane_1_call =
          b.Call("textureSampleLevel", "plane1", "smp", "coord", 0.0f);
    } else if (call_type == sem::BuiltinType::kTextureLoad) {
      // textureLoad(plane0, coords.xy, 0);
      single_plane_call = b.Call("textureLoad", "plane0", "coord", 0);
      // textureLoad(plane0, coords.xy, 0);
      plane_0_call = b.Call("textureLoad", "plane0", "coord", 0);
      // textureLoad(plane1, coords.xy, 0);
      plane_1_call = b.Call("textureLoad", "plane1", "coord", 0);
    } else {
      TINT_ICE(Transform, b.Diagnostics())
          << "unhandled builtin: " << call_type;
    }

    return {
        // var color: vec3<f32>;
        b.Decl(b.Var("color", b.ty.vec3(b.ty.f32()))),
        // if ((params.numPlanes == 1u))
        b.If(b.create<ast::BinaryExpression>(
                 ast::BinaryOp::kEqual, b.MemberAccessor("params", "numPlanes"),
                 b.Expr(1u)),
             b.Block(
                 // color = textureLoad(plane0, coord, 0).rgb;
                 b.Assign("color", b.MemberAccessor(single_plane_call, "rgb"))),
             b.Block(
                 // color = vec4<f32>(plane_0_call.r, plane_1_call.rg, 1.0) *
                 //         params.yuvToRgbConversionMatrix;
                 b.Assign("color",
                          b.Mul(b.vec4<f32>(
                                    b.MemberAccessor(plane_0_call, "r"),
                                    b.MemberAccessor(plane_1_call, "rg"), 1.0f),
                                b.MemberAccessor(
                                    "params", "yuvToRgbConversionMatrix"))))),
        // return vec4<f32>(color, 1.0f);
        b.Return(b.vec4<f32>("color", 1.0f))};
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
          createTexFnExtStatementList(sem::BuiltinType::kTextureSampleLevel);

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
          createTexFnExtStatementList(sem::BuiltinType::kTextureLoad);

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

bool MultiplanarExternalTexture::ShouldRun(const Program* program,
                                           const DataMap&) const {
  for (auto* node : program->ASTNodes().Objects()) {
    if (auto* ty = node->As<ast::Type>()) {
      if (program->Sem().Get<sem::ExternalTexture>(ty)) {
        return true;
      }
    }
  }
  return false;
}

// Within this transform, an instance of a texture_external binding is unpacked
// into two texture_2d<f32> bindings representing two possible planes of a
// single texture and a uniform buffer binding representing a struct of
// parameters. Calls to textureLoad or textureSampleLevel that contain a
// texture_external parameter will be transformed into a newly generated version
// of the function, which can perform the desired operation on a single RGBA
// plane or on separate Y and UV planes.
void MultiplanarExternalTexture::Run(CloneContext& ctx,
                                     const DataMap& inputs,
                                     DataMap&) const {
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

}  // namespace tint::transform
