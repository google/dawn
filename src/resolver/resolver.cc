// Copyright 2020 The Tint Authors.
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

#include "src/resolver/resolver.h"

#include <algorithm>
#include <utility>

#include "src/ast/access_decoration.h"
#include "src/ast/alias.h"
#include "src/ast/array.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/depth_texture.h"
#include "src/ast/disable_validation_decoration.h"
#include "src/ast/discard_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/matrix.h"
#include "src/ast/override_decoration.h"
#include "src/ast/pointer.h"
#include "src/ast/return_statement.h"
#include "src/ast/sampled_texture.h"
#include "src/ast/sampler.h"
#include "src/ast/storage_texture.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_name.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/workgroup_decoration.h"
#include "src/sem/array.h"
#include "src/sem/call.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/sampler_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/utils/math.h"
#include "src/utils/scoped_assignment.h"

namespace tint {
namespace resolver {
namespace {

using IntrinsicType = tint::sem::IntrinsicType;

bool IsValidStorageTextureDimension(ast::TextureDimension dim) {
  switch (dim) {
    case ast::TextureDimension::k1d:
    case ast::TextureDimension::k2d:
    case ast::TextureDimension::k2dArray:
    case ast::TextureDimension::k3d:
      return true;
    default:
      return false;
  }
}

bool IsValidStorageTextureImageFormat(ast::ImageFormat format) {
  switch (format) {
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kRgba32Uint:
    case ast::ImageFormat::kRgba32Sint:
    case ast::ImageFormat::kRgba32Float:
      return true;
    default:
      return false;
  }
}

/// @returns true if the decoration list contains a
/// ast::DisableValidationDecoration with the validation mode equal to
/// `validation`
bool IsValidationDisabled(const ast::DecorationList& decorations,
                          ast::DisabledValidation validation) {
  for (auto* decoration : decorations) {
    if (auto* dv = decoration->As<ast::DisableValidationDecoration>()) {
      if (dv->Validation() == validation) {
        return true;
      }
    }
  }
  return false;
}

}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder),
      diagnostics_(builder->Diagnostics()),
      intrinsic_table_(IntrinsicTable::Create()) {}

Resolver::~Resolver() = default;

void Resolver::set_referenced_from_function_if_needed(VariableInfo* var,
                                                      bool local) {
  if (current_function_ == nullptr) {
    return;
  }
  if (var->storage_class == ast::StorageClass::kNone ||
      var->storage_class == ast::StorageClass::kFunction) {
    return;
  }

  current_function_->referenced_module_vars.add(var);
  if (local) {
    current_function_->local_referenced_module_vars.add(var);
  }
}

bool Resolver::Resolve() {
  if (builder_->Diagnostics().contains_errors()) {
    return false;
  }

  bool result = ResolveInternal();

  if (!result && !diagnostics_.contains_errors()) {
    TINT_ICE(diagnostics_) << "resolving failed, but no error was raised";
    return false;
  }

  // Even if resolving failed, create all the semantic nodes for information we
  // did generate.
  CreateSemanticNodes();

  return result;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Resolver::IsStorable(const sem::Type* type) {
  if (type->is_scalar() || type->Is<sem::Vector>() || type->Is<sem::Matrix>()) {
    return true;
  }
  if (auto* arr = type->As<sem::Array>()) {
    return IsStorable(arr->ElemType());
  }
  if (auto* str = type->As<sem::Struct>()) {
    for (const auto* member : str->Members()) {
      if (!IsStorable(member->Type())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable-types
bool Resolver::IsHostShareable(const sem::Type* type) {
  if (type->IsAnyOf<sem::I32, sem::U32, sem::F32>()) {
    return true;
  }
  if (auto* vec = type->As<sem::Vector>()) {
    return IsHostShareable(vec->type());
  }
  if (auto* mat = type->As<sem::Matrix>()) {
    return IsHostShareable(mat->type());
  }
  if (auto* arr = type->As<sem::Array>()) {
    return IsHostShareable(arr->ElemType());
  }
  if (auto* str = type->As<sem::Struct>()) {
    for (auto* member : str->Members()) {
      if (!IsHostShareable(member->Type())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Resolver::ResolveInternal() {
  Mark(&builder_->AST());

  // Process everything else in the order they appear in the module. This is
  // necessary for validation of use-before-declaration.
  for (auto* decl : builder_->AST().GlobalDeclarations()) {
    if (auto* ty = decl->As<ast::NamedType>()) {
      Mark(ty);
      if (!NamedType(ty)) {
        return false;
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      Mark(func);
      if (!Function(func)) {
        return false;
      }
    } else if (auto* var = decl->As<ast::Variable>()) {
      Mark(var);
      if (!GlobalVariable(var)) {
        return false;
      }
    } else {
      TINT_UNREACHABLE(diagnostics_)
          << "unhandled global declaration: " << decl->TypeInfo().name;
      return false;
    }
  }

  bool result = true;

  for (auto* node : builder_->ASTNodes().Objects()) {
    if (marked_.count(node) == 0) {
      if (node->IsAnyOf<ast::AccessDecoration, ast::StrideDecoration,
                        ast::Type>()) {
        // TODO(crbug.com/tint/724) - Remove once tint:724 is complete.
        // ast::AccessDecorations are generated by the WGSL parser, used to
        // build sem::AccessControls and then leaked.
        // ast::StrideDecoration are used to build a sem::Arrays, but
        // multiple arrays of the same stride, size and element type are
        // currently de-duplicated by the type manager, and we leak these
        // decorations.
        // ast::Types are being built, but not yet being handled. This is WIP.
        continue;
      }
      TINT_ICE(diagnostics_) << "AST node '" << node->TypeInfo().name
                             << "' was not reached by the resolver\n"
                             << "At: " << node->source() << "\n"
                             << "Content: " << builder_->str(node) << "\n"
                             << "Pointer: " << node;
      result = false;
    }
  }

  return result;
}

sem::Type* Resolver::Type(const ast::Type* ty) {
  Mark(ty);
  auto* s = [&]() -> sem::Type* {
    if (ty->Is<ast::Void>()) {
      return builder_->create<sem::Void>();
    }
    if (ty->Is<ast::Bool>()) {
      return builder_->create<sem::Bool>();
    }
    if (ty->Is<ast::I32>()) {
      return builder_->create<sem::I32>();
    }
    if (ty->Is<ast::U32>()) {
      return builder_->create<sem::U32>();
    }
    if (ty->Is<ast::F32>()) {
      return builder_->create<sem::F32>();
    }
    if (auto* t = ty->As<ast::AccessControl>()) {
      TINT_SCOPED_ASSIGNMENT(current_access_control_, t);
      if (auto* el = Type(t->type())) {
        return el;
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Vector>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::Vector>(const_cast<sem::Type*>(el),
                                             t->size());
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Matrix>()) {
      if (auto* el = Type(t->type())) {
        auto* column_type = builder_->create<sem::Vector>(
            const_cast<sem::Type*>(el), t->rows());
        return builder_->create<sem::Matrix>(column_type, t->columns());
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Array>()) {
      return Array(t);
    }
    if (auto* t = ty->As<ast::Pointer>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::Pointer>(const_cast<sem::Type*>(el),
                                              t->storage_class());
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Sampler>()) {
      return builder_->create<sem::Sampler>(t->kind());
    }
    if (auto* t = ty->As<ast::SampledTexture>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::SampledTexture>(
            t->dim(), const_cast<sem::Type*>(el));
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::MultisampledTexture>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::MultisampledTexture>(
            t->dim(), const_cast<sem::Type*>(el));
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::DepthTexture>()) {
      return builder_->create<sem::DepthTexture>(t->dim());
    }
    if (auto* t = ty->As<ast::StorageTexture>()) {
      if (auto* el = Type(t->type())) {
        if (!current_access_control_) {
          diagnostics_.add_error("storage textures must have access control",
                                 t->source());
          return nullptr;
        }
        return builder_->create<sem::StorageTexture>(
            t->dim(), t->image_format(),
            current_access_control_->access_control(),
            const_cast<sem::Type*>(el));
      }
      return nullptr;
    }
    if (ty->As<ast::ExternalTexture>()) {
      return builder_->create<sem::ExternalTexture>();
    }
    if (auto* t = ty->As<ast::TypeName>()) {
      auto it = named_type_info_.find(t->name());
      if (it == named_type_info_.end()) {
        diagnostics_.add_error(
            "unknown type '" + builder_->Symbols().NameFor(t->name()) + "'",
            t->source());
        return nullptr;
      }
      return it->second.sem;
    }
    TINT_UNREACHABLE(diagnostics_)
        << "Unhandled ast::Type: " << ty->TypeInfo().name;
    return nullptr;
  }();

  if (s) {
    builder_->Sem().Add(ty, s);
  }
  return s;
}

Resolver::VariableInfo* Resolver::Variable(ast::Variable* var,
                                           VariableKind kind) {
  if (variable_to_info_.count(var)) {
    TINT_ICE(diagnostics_) << "Variable "
                           << builder_->Symbols().NameFor(var->symbol())
                           << " already resolved";
    return nullptr;
  }

  std::string type_name;
  const sem::Type* storage_type = nullptr;

  // If the variable has a declared type, resolve it.
  if (auto* ty = var->type()) {
    type_name = ty->FriendlyName(builder_->Symbols());
    storage_type = Type(ty);
    if (!storage_type) {
      return nullptr;
    }
  }

  std::string rhs_type_name;
  const sem::Type* rhs_type = nullptr;

  // Does the variable have a constructor?
  if (auto* ctor = var->constructor()) {
    Mark(var->constructor());
    if (!Expression(var->constructor())) {
      return nullptr;
    }

    // Fetch the constructor's type
    rhs_type_name = TypeNameOf(ctor);
    rhs_type = TypeOf(ctor);
    if (!rhs_type) {
      return nullptr;
    }

    // If the variable has no declared type, infer it from the RHS
    if (!storage_type) {
      type_name = rhs_type_name;
      storage_type = rhs_type->UnwrapRef();  // Implicit load of RHS
    }
  } else if (var->is_const() && kind != VariableKind::kParameter &&
             !ast::HasDecoration<ast::OverrideDecoration>(var->decorations())) {
    diagnostics_.add_error("let declarations must have initializers",
                           var->source());
    return nullptr;
  }

  if (!storage_type) {
    TINT_ICE(diagnostics_)
        << "failed to determine storage type for variable '" +
               builder_->Symbols().NameFor(var->symbol()) + "'\n"
        << "Source: " << var->source();
    return nullptr;
  }

  auto storage_class = var->declared_storage_class();
  if (storage_class == ast::StorageClass::kNone) {
    if (storage_type->UnwrapRef()->is_handle()) {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // If the store type is a texture type or a sampler type, then the
      // variable declaration must not have a storage class decoration. The
      // storage class will always be handle.
      storage_class = ast::StorageClass::kUniformConstant;
    } else if (kind == VariableKind::kLocal && !var->is_const()) {
      storage_class = ast::StorageClass::kFunction;
    }
  }

  auto* type = storage_type;
  if (!var->is_const()) {
    // Variable declaration. Unlike `let`, `var` has storage.
    // Variables are always of a reference type to the declared storage type.
    type = builder_->create<sem::Reference>(storage_type, storage_class);
  }

  if (rhs_type && !ValidateVariableConstructor(var, storage_type, type_name,
                                               rhs_type, rhs_type_name)) {
    return nullptr;
  }

  // TODO(crbug.com/tint/802): Temporary while ast::AccessControl exits.
  auto find_first_access_control =
      [this](const ast::Type* ty) -> const ast::AccessControl* {
    if (ty == nullptr) {
      return nullptr;
    }
    if (const ast::AccessControl* ac = ty->As<ast::AccessControl>()) {
      return ac;
    }
    while (auto* tn = ty->As<ast::TypeName>()) {
      auto it = named_type_info_.find(tn->name());
      if (it == named_type_info_.end()) {
        break;
      }
      auto* alias = it->second.ast->As<ast::Alias>();
      if (!alias) {
        break;
      }
      ty = alias->type();
      if (auto* ac = ty->As<ast::AccessControl>()) {
        return ac;
      }
    }
    return nullptr;
  };

  auto* access_control = find_first_access_control(var->type());
  auto* info = variable_infos_.Create(var, const_cast<sem::Type*>(type),
                                      type_name, storage_class, access_control);
  variable_to_info_.emplace(var, info);

  return info;
}

bool Resolver::ValidateVariableConstructor(const ast::Variable* var,
                                           const sem::Type* storage_type,
                                           const std::string& type_name,
                                           const sem::Type* rhs_type,
                                           const std::string& rhs_type_name) {
  auto* value_type = rhs_type->UnwrapRef();  // Implicit load of RHS

  // RHS needs to be of a storable type
  if (!var->is_const() && !IsStorable(value_type)) {
    diagnostics_.add_error(
        "'" + rhs_type_name + "' is not storable for assignment",
        var->constructor()->source());
    return false;
  }

  // Value type has to match storage type
  if (storage_type != value_type) {
    std::string decl = var->is_const() ? "let" : "var";
    diagnostics_.add_error("cannot initialize " + decl + " of type '" +
                               type_name + "' with value of type '" +
                               rhs_type_name + "'",
                           var->source());
    return false;
  }
  return true;
}

bool Resolver::GlobalVariable(ast::Variable* var) {
  if (variable_stack_.has(var->symbol())) {
    diagnostics_.add_error("v-0011",
                           "redeclared global identifier '" +
                               builder_->Symbols().NameFor(var->symbol()) + "'",
                           var->source());
    return false;
  }

  auto* info = Variable(var, VariableKind::kGlobal);
  if (!info) {
    return false;
  }
  variable_stack_.set_global(var->symbol(), info);

  if (!var->is_const() && info->storage_class == ast::StorageClass::kNone) {
    diagnostics_.add_error(
        "v-0022", "global variables must have a storage class", var->source());
    return false;
  }
  if (var->is_const() && !(info->storage_class == ast::StorageClass::kNone)) {
    diagnostics_.add_error("v-global01",
                           "global constants shouldn't have a storage class",
                           var->source());
    return false;
  }

  for (auto* deco : var->decorations()) {
    Mark(deco);

    if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
      // Track the constant IDs that are specified in the shader.
      if (override_deco->HasValue()) {
        constant_ids_.emplace(override_deco->value(), info);
      }
    }
  }

  if (auto bp = var->binding_point()) {
    info->binding_point = {bp.group->value(), bp.binding->value()};
  }

  if (!ValidateGlobalVariable(info)) {
    return false;
  }

  if (!ApplyStorageClassUsageToType(
          info->storage_class, const_cast<sem::Type*>(info->type->UnwrapRef()),
          var->source())) {
    diagnostics_.add_note("while instantiating variable " +
                              builder_->Symbols().NameFor(var->symbol()),
                          var->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateGlobalVariable(const VariableInfo* info) {
  for (auto* deco : info->declaration->decorations()) {
    if (info->declaration->is_const()) {
      if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
        if (override_deco->HasValue()) {
          uint32_t id = override_deco->value();
          auto itr = constant_ids_.find(id);
          if (itr != constant_ids_.end() && itr->second != info) {
            diagnostics_.add_error("pipeline constant IDs must be unique",
                                   deco->source());
            diagnostics_.add_note("a pipeline constant with an ID of " +
                                      std::to_string(id) +
                                      " was previously declared "
                                      "here:",
                                  ast::GetDecoration<ast::OverrideDecoration>(
                                      itr->second->declaration->decorations())
                                      ->source());
            return false;
          }
          if (id > 65535) {
            diagnostics_.add_error(
                "pipeline constant IDs must be between 0 and 65535",
                deco->source());
            return false;
          }
        }
      } else {
        diagnostics_.add_error("decoration is not valid for constants",
                               deco->source());
        return false;
      }
    } else {
      if (!(deco->Is<ast::BindingDecoration>() ||
            deco->Is<ast::BuiltinDecoration>() ||
            deco->Is<ast::GroupDecoration>() ||
            deco->Is<ast::LocationDecoration>() ||
            deco->Is<ast::InternalDecoration>())) {
        diagnostics_.add_error("decoration is not valid for variables",
                               deco->source());
        return false;
      }
    }
  }

  auto binding_point = info->declaration->binding_point();
  switch (info->storage_class) {
    case ast::StorageClass::kUniform:
    case ast::StorageClass::kStorage:
    case ast::StorageClass::kUniformConstant: {
      // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
      // Each resource variable must be declared with both group and binding
      // attributes.
      if (!binding_point) {
        diagnostics_.add_error(
            "resource variables require [[group]] and [[binding]] "
            "decorations",
            info->declaration->source());
        return false;
      }
      break;
    }
    default:
      if (binding_point.binding || binding_point.group) {
        // https://gpuweb.github.io/gpuweb/wgsl/#attribute-binding
        // Must only be applied to a resource variable
        diagnostics_.add_error(
            "non-resource variables must not have [[group]] or [[binding]] "
            "decorations",
            info->declaration->source());
        return false;
      }
  }

  switch (info->storage_class) {
    case ast::StorageClass::kStorage: {
      // https://gpuweb.github.io/gpuweb/wgsl/#variable-declaration
      // Variables in the storage storage class and variables with a storage
      // texture type must have an access attribute applied to the store type.

      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // A variable in the storage storage class is a storage buffer variable.
      // Its store type must be a host-shareable structure type with block
      // attribute, satisfying the storage class constraints.

      auto* str = info->access_control
                      ? info->type->UnwrapRef()->As<sem::Struct>()
                      : nullptr;

      if (!str) {
        diagnostics_.add_error(
            "variables declared in the <storage> storage class must be of an "
            "[[access]] qualified structure type",
            info->declaration->source());
        return false;
      }

      if (!str->IsBlockDecorated()) {
        diagnostics_.add_error(
            "structure used as a storage buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source());
        if (info->declaration->source().range.begin.line) {
          diagnostics_.add_note("structure used as storage buffer here",
                                info->declaration->source());
        }
        return false;
      }
      break;
    }
    case ast::StorageClass::kUniform: {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // A variable in the uniform storage class is a uniform buffer variable.
      // Its store type must be a host-shareable structure type with block
      // attribute, satisfying the storage class constraints.
      auto* str = info->type->UnwrapRef()->As<sem::Struct>();
      if (!str) {
        diagnostics_.add_error(
            "variables declared in the <uniform> storage class must be of a "
            "structure type",
            info->declaration->source());
        return false;
      }

      if (!str->IsBlockDecorated()) {
        diagnostics_.add_error(
            "structure used as a uniform buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source());
        if (info->declaration->source().range.begin.line) {
          diagnostics_.add_note("structure used as uniform buffer here",
                                info->declaration->source());
        }
        return false;
      }
      break;
    }
    default:
      break;
  }

  return ValidateVariable(info);
}

bool Resolver::ValidateVariable(const VariableInfo* info) {
  auto* var = info->declaration;
  auto* storage_type = info->type->UnwrapRef();
  if (auto* r = storage_type->As<sem::Array>()) {
    if (r->IsRuntimeSized()) {
      diagnostics_.add_error(
          "v-0015",
          "runtime arrays may only appear as the last member of a struct",
          var->source());
      return false;
    }
  }

  if (auto* r = storage_type->As<sem::MultisampledTexture>()) {
    if (r->dim() != ast::TextureDimension::k2d) {
      diagnostics_.add_error("only 2d multisampled textures are supported",
                             var->source());
      return false;
    }

    if (!r->type()->UnwrapRef()->is_numeric_scalar()) {
      diagnostics_.add_error(
          "texture_multisampled_2d<type>: type must be f32, i32 or u32",
          var->source());
      return false;
    }
  }

  if (auto* storage_tex = info->type->UnwrapRef()->As<sem::StorageTexture>()) {
    if (info->access_control->access_control() ==
        ast::AccessControl::kReadWrite) {
      diagnostics_.add_error(
          "storage textures only support read-only and write-only access",
          var->source());
      return false;
    }

    if (!IsValidStorageTextureDimension(storage_tex->dim())) {
      diagnostics_.add_error(
          "cube dimensions for storage textures are not "
          "supported",
          var->source());
      return false;
    }

    if (!IsValidStorageTextureImageFormat(storage_tex->image_format())) {
      diagnostics_.add_error(
          "image format must be one of the texel formats specified for "
          "storage textues in "
          "https://gpuweb.github.io/gpuweb/wgsl/#texel-formats",
          var->source());
      return false;
    }
  }

  if (storage_type->is_handle() &&
      var->declared_storage_class() != ast::StorageClass::kNone) {
    // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
    // If the store type is a texture type or a sampler type, then the
    // variable declaration must not have a storage class decoration. The
    // storage class will always be handle.
    diagnostics_.add_error("variables of type '" + info->type_name +
                               "' must not have a storage class",
                           var->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateParameter(const VariableInfo* info) {
  return ValidateVariable(info);
}

bool Resolver::ValidateFunction(const ast::Function* func,
                                const FunctionInfo* info) {
  if (symbol_to_function_.find(func->symbol()) != symbol_to_function_.end()) {
    diagnostics_.add_error("v-0016",
                           "function names must be unique '" +
                               builder_->Symbols().NameFor(func->symbol()) +
                               "'",
                           func->source());
    return false;
  }

  auto stage_deco_count = 0;
  auto workgroup_deco_count = 0;
  for (auto* deco : func->decorations()) {
    if (deco->Is<ast::StageDecoration>()) {
      stage_deco_count++;
    } else if (deco->Is<ast::WorkgroupDecoration>()) {
      workgroup_deco_count++;
      if (func->pipeline_stage() != ast::PipelineStage::kCompute) {
        diagnostics_.add_error(
            "the workgroup_size attribute is only valid for compute stages",
            deco->source());
        return false;
      }
    } else if (!deco->Is<ast::InternalDecoration>()) {
      diagnostics_.add_error("decoration is not valid for functions",
                             deco->source());
      return false;
    }
  }
  if (stage_deco_count > 1) {
    diagnostics_.add_error(
        "v-0020", "only one stage decoration permitted per entry point",
        func->source());
    return false;
  }
  if (workgroup_deco_count > 1) {
    diagnostics_.add_error(
        "only one workgroup_size attribute permitted per entry point",
        func->source());
    return false;
  }

  for (auto* param : func->params()) {
    if (!ValidateParameter(variable_to_info_.at(param))) {
      return false;
    }
  }

  if (!info->return_type->Is<sem::Void>()) {
    if (func->body()) {
      if (!func->get_last_statement() ||
          !func->get_last_statement()->Is<ast::ReturnStatement>()) {
        diagnostics_.add_error(
            "v-0002", "non-void function must end with a return statement",
            func->source());
        return false;
      }
    } else if (!IsValidationDisabled(
                   func->decorations(),
                   ast::DisabledValidation::kFunctionHasNoBody)) {
      TINT_ICE(diagnostics_)
          << "Function " << builder_->Symbols().NameFor(func->symbol())
          << " has no body";
    }

    for (auto* deco : func->return_type_decorations()) {
      if (!deco->IsAnyOf<ast::BuiltinDecoration, ast::LocationDecoration>()) {
        diagnostics_.add_error(
            "decoration is not valid for function return types",
            deco->source());
        return false;
      }
    }
  }

  if (func->IsEntryPoint()) {
    if (!ValidateEntryPoint(func, info)) {
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateEntryPoint(const ast::Function* func,
                                  const FunctionInfo* info) {
  // Use a lambda to validate the entry point decorations for a type.
  // Persistent state is used to track which builtins and locations have
  // already been seen, in order to catch conflicts.
  // TODO(jrprice): This state could be stored in FunctionInfo instead, and
  // then passed to sem::Function since it would be useful there too.
  std::unordered_set<ast::Builtin> builtins;
  std::unordered_set<uint32_t> locations;
  enum class ParamOrRetType {
    kParameter,
    kReturnType,
  };
  // Helper to stringify a pipeline IO decoration.
  auto deco_to_str = [](const ast::Decoration* deco) {
    std::stringstream str;
    if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
      str << "builtin(" << builtin->value() << ")";
    } else if (auto* location = deco->As<ast::LocationDecoration>()) {
      str << "location(" << location->value() << ")";
    }
    return str.str();
  };
  // Inner lambda that is applied to a type and all of its members.
  auto validate_entry_point_decorations_inner =
      [&](const ast::DecorationList& decos, sem::Type* ty, Source source,
          ParamOrRetType param_or_ret, bool is_struct_member) {
        // Scan decorations for pipeline IO attributes.
        // Check for overlap with attributes that have been seen previously.
        ast::Decoration* pipeline_io_attribute = nullptr;
        for (auto* deco : decos) {
          if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
            if (pipeline_io_attribute) {
              diagnostics_.add_error("multiple entry point IO attributes",
                                     deco->source());
              diagnostics_.add_note(
                  "previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
              return false;
            }
            pipeline_io_attribute = deco;

            if (builtins.count(builtin->value())) {
              diagnostics_.add_error(
                  deco_to_str(builtin) +
                      " attribute appears multiple times as pipeline " +
                      (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                  : "output"),
                  func->source());
              return false;
            }
            builtins.emplace(builtin->value());

          } else if (auto* location = deco->As<ast::LocationDecoration>()) {
            if (pipeline_io_attribute) {
              diagnostics_.add_error("multiple entry point IO attributes",
                                     deco->source());
              diagnostics_.add_note(
                  "previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
              return false;
            }
            pipeline_io_attribute = deco;

            if (locations.count(location->value())) {
              diagnostics_.add_error(
                  deco_to_str(location) +
                      " attribute appears multiple times as pipeline " +
                      (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                  : "output"),
                  func->source());
              return false;
            }
            locations.emplace(location->value());
          }
        }

        // Check that we saw a pipeline IO attribute iff we need one.
        if (ty->Is<sem::Struct>()) {
          if (pipeline_io_attribute) {
            diagnostics_.add_error(
                "entry point IO attributes must not be used on structure " +
                    std::string(param_or_ret == ParamOrRetType::kParameter
                                    ? "parameters"
                                    : "return types"),
                pipeline_io_attribute->source());
            return false;
          }
        } else {
          if (!pipeline_io_attribute) {
            std::string err = "missing entry point IO attribute";
            if (!is_struct_member) {
              err += (param_or_ret == ParamOrRetType::kParameter
                          ? " on parameter"
                          : " on return type");
            }
            diagnostics_.add_error(err, source);
            return false;
          }

          // Check that all user defined attributes are numeric scalars, vectors
          // of numeric scalars.
          // Testing for being a struct is handled by the if portion above.
          if (!pipeline_io_attribute->Is<ast::BuiltinDecoration>()) {
            if (!ty->is_numeric_scalar_or_vector()) {
              diagnostics_.add_error(
                  "User defined entry point IO types must be a numeric scalar, "
                  "a numeric vector, or a structure",
                  source);
              return false;
            }
          }
        }

        return true;
      };

  // Outer lambda for validating the entry point decorations for a type.
  auto validate_entry_point_decorations = [&](const ast::DecorationList& decos,
                                              sem::Type* ty, Source source,
                                              ParamOrRetType param_or_ret) {
    // Validate the decorations for the type.
    if (!validate_entry_point_decorations_inner(decos, ty, source, param_or_ret,
                                                false)) {
      return false;
    }

    if (auto* str = ty->As<sem::Struct>()) {
      // Validate the decorations for each struct members, and also check for
      // invalid member types.
      for (auto* member : str->Members()) {
        if (member->Type()->Is<sem::Struct>()) {
          diagnostics_.add_error(
              "entry point IO types cannot contain nested structures",
              member->Declaration()->source());
          diagnostics_.add_note("while analysing entry point " +
                                    builder_->Symbols().NameFor(func->symbol()),
                                func->source());
          return false;
        }

        if (auto* arr = member->Type()->As<sem::Array>()) {
          if (arr->IsRuntimeSized()) {
            diagnostics_.add_error(
                "entry point IO types cannot contain runtime sized arrays",
                member->Declaration()->source());
            diagnostics_.add_note(
                "while analysing entry point " +
                    builder_->Symbols().NameFor(func->symbol()),
                func->source());
            return false;
          }
        }

        if (!validate_entry_point_decorations_inner(
                member->Declaration()->decorations(), member->Type(),
                member->Declaration()->source(), param_or_ret, true)) {
          diagnostics_.add_note("while analysing entry point " +
                                    builder_->Symbols().NameFor(func->symbol()),
                                func->source());
          return false;
        }
      }
    }

    return true;
  };

  for (auto* param : info->parameters) {
    if (!validate_entry_point_decorations(
            param->declaration->decorations(), param->type,
            param->declaration->source(), ParamOrRetType::kParameter)) {
      return false;
    }
  }

  // Clear IO sets after parameter validation. Builtin and location attributes
  // in return types should be validated independently from those used in
  // parameters.
  builtins.clear();
  locations.clear();

  if (!info->return_type->Is<sem::Void>()) {
    if (!validate_entry_point_decorations(func->return_type_decorations(),
                                          info->return_type, func->source(),
                                          ParamOrRetType::kReturnType)) {
      return false;
    }
  }

  if (func->pipeline_stage() == ast::PipelineStage::kVertex &&
      builtins.count(ast::Builtin::kPosition) == 0) {
    // Check module-scope variables, as the SPIR-V sanitizer generates these.
    bool found = false;
    for (auto* var : info->referenced_module_vars) {
      if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
              var->declaration->decorations())) {
        if (builtin->value() == ast::Builtin::kPosition) {
          found = true;
          break;
        }
      }
    }
    // TODO(bclayton): Reenable after CTS is updated
    if (((false)) && !found) {
      diagnostics_.add_error(
          "a vertex shader must include the 'position' builtin in its return "
          "type",
          func->source());
      return false;
    }
  }

  // Validate there are no resource variable binding collisions
  std::unordered_map<sem::BindingPoint, const ast::Variable*> binding_points;
  for (auto* var_info : info->referenced_module_vars) {
    if (!var_info->declaration->binding_point()) {
      continue;
    }
    auto bp = var_info->binding_point;
    auto res = binding_points.emplace(bp, var_info->declaration);
    if (!res.second &&
        !IsValidationDisabled(
            var_info->declaration->decorations(),
            ast::DisabledValidation::kBindingPointCollision) &&
        !IsValidationDisabled(
            res.first->second->decorations(),
            ast::DisabledValidation::kBindingPointCollision)) {
      // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
      // Bindings must not alias within a shader stage: two different
      // variables in the resource interface of a given shader must not have
      // the same group and binding values, when considered as a pair of
      // values.
      auto func_name = builder_->Symbols().NameFor(info->declaration->symbol());
      diagnostics_.add_error("entry point '" + func_name +
                                 "' references multiple variables that use the "
                                 "same resource binding [[group(" +
                                 std::to_string(bp.group) + "), binding(" +
                                 std::to_string(bp.binding) + ")]]",
                             var_info->declaration->source());
      diagnostics_.add_note("first resource binding usage declared here",
                            res.first->second->source());
      return false;
    }
  }

  return true;
}

bool Resolver::Function(ast::Function* func) {
  auto* info = function_infos_.Create<FunctionInfo>(func);

  TINT_SCOPED_ASSIGNMENT(current_function_, info);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    Mark(param);
    auto* param_info = Variable(param, VariableKind::kParameter);
    if (!param_info) {
      return false;
    }

    // TODO(amaiorano): Validate parameter decorations
    for (auto* deco : param->decorations()) {
      Mark(deco);
    }

    variable_stack_.set(param->symbol(), param_info);
    info->parameters.emplace_back(param_info);

    if (!ApplyStorageClassUsageToType(param->declared_storage_class(),
                                      param_info->type, param->source())) {
      diagnostics_.add_note("while instantiating parameter " +
                                builder_->Symbols().NameFor(param->symbol()),
                            param->source());
      return false;
    }

    if (auto* str = param_info->type->As<sem::Struct>()) {
      switch (func->pipeline_stage()) {
        case ast::PipelineStage::kVertex:
          str->AddUsage(sem::PipelineStageUsage::kVertexInput);
          break;
        case ast::PipelineStage::kFragment:
          str->AddUsage(sem::PipelineStageUsage::kFragmentInput);
          break;
        case ast::PipelineStage::kCompute:
          str->AddUsage(sem::PipelineStageUsage::kComputeInput);
          break;
        case ast::PipelineStage::kNone:
          break;
      }
    }
  }

  if (auto* ty = func->return_type()) {
    info->return_type = Type(ty);
    info->return_type_name = ty->FriendlyName(builder_->Symbols());
    if (!info->return_type) {
      return false;
    }
  } else {
    info->return_type = builder_->create<sem::Void>();
    info->return_type_name =
        info->return_type->FriendlyName(builder_->Symbols());
  }

  if (auto* str = info->return_type->As<sem::Struct>()) {
    if (!ApplyStorageClassUsageToType(ast::StorageClass::kNone, str,
                                      func->source())) {
      diagnostics_.add_note("while instantiating return type for " +
                                builder_->Symbols().NameFor(func->symbol()),
                            func->source());
      return false;
    }

    switch (func->pipeline_stage()) {
      case ast::PipelineStage::kVertex:
        str->AddUsage(sem::PipelineStageUsage::kVertexOutput);
        break;
      case ast::PipelineStage::kFragment:
        str->AddUsage(sem::PipelineStageUsage::kFragmentOutput);
        break;
      case ast::PipelineStage::kCompute:
        str->AddUsage(sem::PipelineStageUsage::kComputeOutput);
        break;
      case ast::PipelineStage::kNone:
        break;
    }
  }

  if (func->body()) {
    Mark(func->body());
    if (current_statement_) {
      TINT_ICE(diagnostics_)
          << "Resolver::Function() called with a current statement";
      return false;
    }
    auto* sem_block = builder_->create<sem::FunctionBlockStatement>(func);
    builder_->Sem().Add(func->body(), sem_block);
    TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block);
    if (!BlockScope(func->body(),
                    [&] { return Statements(func->body()->list()); })) {
      return false;
    }
  }
  variable_stack_.pop_scope();

  for (auto* deco : func->decorations()) {
    Mark(deco);
  }
  for (auto* deco : func->return_type_decorations()) {
    Mark(deco);
  }

  // Set work-group size defaults.
  for (int i = 0; i < 3; i++) {
    info->workgroup_size[i].value = 1;
    info->workgroup_size[i].overridable_const = nullptr;
  }

  if (auto* workgroup =
          ast::GetDecoration<ast::WorkgroupDecoration>(func->decorations())) {
    auto values = workgroup->values();
    for (int i = 0; i < 3; i++) {
      // Each argument to this decoration can either be a literal, an
      // identifier for a module-scope constants, or nullptr if not specified.

      if (!values[i]) {
        // Not specified, just use the default.
        continue;
      }

      Mark(values[i]);

      int32_t value = 0;
      if (auto* ident = values[i]->As<ast::IdentifierExpression>()) {
        // We have an identifier of a module-scope constant.
        if (!Identifier(ident)) {
          return false;
        }

        VariableInfo* var;
        if (!variable_stack_.get(ident->symbol(), &var) ||
            !(var->declaration->is_const() && var->type->Is<sem::I32>())) {
          diagnostics_.add_error(
              "workgroup_size parameter must be a literal i32 or an i32 "
              "module-scope constant",
              values[i]->source());
          return false;
        }

        // Capture the constant if an [[override]] attribute is present.
        if (ast::HasDecoration<ast::OverrideDecoration>(
                var->declaration->decorations())) {
          info->workgroup_size[i].overridable_const = var->declaration;
        }

        auto* constructor = var->declaration->constructor();
        if (constructor) {
          // Resolve the constructor expression to use as the default value.
          if (!GetScalarConstExprValue(constructor, &value)) {
            return false;
          }
        } else {
          // No constructor means this value must be overriden by the user.
          info->workgroup_size[i].value = 0;
          continue;
        }
      } else if (auto* scalar =
                     values[i]->As<ast::ScalarConstructorExpression>()) {
        // We have a literal.
        Mark(scalar->literal());

        if (!scalar->literal()->Is<ast::IntLiteral>()) {
          diagnostics_.add_error(
              "workgroup_size parameter must be a literal i32 or an i32 "
              "module-scope constant",
              values[i]->source());
          return false;
        }

        if (!GetScalarConstExprValue(scalar, &value)) {
          return false;
        }
      }

      // Validate and set the default value for this dimension.
      if (value < 1) {
        diagnostics_.add_error(
            "workgroup_size parameter must be a positive i32 value",
            values[i]->source());
        return false;
      }
      info->workgroup_size[i].value = value;
    }
  }

  if (!ValidateFunction(func, info)) {
    return false;
  }

  // Register the function information _after_ processing the statements. This
  // allows us to catch a function calling itself when determining the call
  // information as this function doesn't exist until it's finished.
  symbol_to_function_[func->symbol()] = info;
  function_to_info_.emplace(func, info);

  return true;
}

bool Resolver::Statements(const ast::StatementList& stmts) {
  for (auto* stmt : stmts) {
    Mark(stmt);
    if (!Statement(stmt)) {
      return false;
    }
  }
  return true;
}

bool Resolver::Statement(ast::Statement* stmt) {
  sem::Statement* sem_statement;
  if (stmt->As<ast::BlockStatement>()) {
    sem_statement = builder_->create<sem::BlockStatement>(
        stmt->As<ast::BlockStatement>(), current_statement_);
  } else {
    sem_statement = builder_->create<sem::Statement>(stmt, current_statement_);
  }
  builder_->Sem().Add(stmt, sem_statement);

  TINT_SCOPED_ASSIGNMENT(current_statement_, sem_statement);

  if (stmt->Is<ast::ElseStatement>()) {
    TINT_ICE(diagnostics_)
        << "Resolver::Statement() encountered an Else statement. Else "
           "statements are embedded in If statements, so should never be "
           "encountered as top-level statements";
    return false;
  }

  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return Assignment(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return BlockScope(b, [&] { return Statements(b->list()); });
  }
  if (stmt->Is<ast::BreakStatement>()) {
    if (!current_block_->FindFirstParent<sem::LoopBlockStatement>() &&
        !current_block_->FindFirstParent<sem::SwitchCaseBlockStatement>()) {
      diagnostics_.add_error("break statement must be in a loop or switch case",
                             stmt->source());
      return false;
    }
    return true;
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    Mark(c->expr());
    return Expression(c->expr());
  }
  if (auto* c = stmt->As<ast::CaseStatement>()) {
    return CaseStatement(c);
  }
  if (stmt->Is<ast::ContinueStatement>()) {
    // Set if we've hit the first continue statement in our parent loop
    if (auto* loop_block =
            current_block_->FindFirstParent<sem::LoopBlockStatement>()) {
      if (loop_block->FirstContinue() == size_t(~0)) {
        const_cast<sem::LoopBlockStatement*>(loop_block)
            ->SetFirstContinue(loop_block->Decls().size());
      }
    } else {
      diagnostics_.add_error("continue statement must be in a loop",
                             stmt->source());
      return false;
    }

    return true;
  }
  if (stmt->Is<ast::DiscardStatement>()) {
    return true;
  }
  if (stmt->Is<ast::FallthroughStatement>()) {
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return IfStatement(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return LoopStatement(l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return Return(r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return Switch(s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return VariableDeclStatement(v);
  }

  diagnostics_.add_error(
      "unknown statement type for type determination: " + builder_->str(stmt),
      stmt->source());
  return false;
}

bool Resolver::CaseStatement(ast::CaseStatement* stmt) {
  Mark(stmt->body());
  for (auto* sel : stmt->selectors()) {
    Mark(sel);
  }
  auto* sem_block = builder_->create<sem::SwitchCaseBlockStatement>(
      stmt->body(), current_statement_);
  builder_->Sem().Add(stmt->body(), sem_block);
  TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block);
  return BlockScope(stmt->body(),
                    [&] { return Statements(stmt->body()->list()); });
}

bool Resolver::IfStatement(ast::IfStatement* stmt) {
  Mark(stmt->condition());
  if (!Expression(stmt->condition())) {
    return false;
  }

  auto* cond_type = TypeOf(stmt->condition())->UnwrapRef();
  if (!cond_type->Is<sem::Bool>()) {
    diagnostics_.add_error("if statement condition must be bool, got " +
                               cond_type->FriendlyName(builder_->Symbols()),
                           stmt->condition()->source());
    return false;
  }

  Mark(stmt->body());
  {
    auto* sem_block =
        builder_->create<sem::BlockStatement>(stmt->body(), current_statement_);
    builder_->Sem().Add(stmt->body(), sem_block);
    TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block);
    if (!BlockScope(stmt->body(),
                    [&] { return Statements(stmt->body()->list()); })) {
      return false;
    }
  }

  for (auto* else_stmt : stmt->else_statements()) {
    Mark(else_stmt);
    auto* sem_else_stmt =
        builder_->create<sem::Statement>(else_stmt, current_statement_);
    builder_->Sem().Add(else_stmt, sem_else_stmt);
    TINT_SCOPED_ASSIGNMENT(current_statement_, sem_else_stmt);
    if (auto* cond = else_stmt->condition()) {
      Mark(cond);
      if (!Expression(cond)) {
        return false;
      }

      auto* else_cond_type = TypeOf(cond)->UnwrapRef();
      if (!else_cond_type->Is<sem::Bool>()) {
        diagnostics_.add_error(
            "else statement condition must be bool, got " +
                else_cond_type->FriendlyName(builder_->Symbols()),
            cond->source());
        return false;
      }
    }
    Mark(else_stmt->body());
    {
      auto* sem_block = builder_->create<sem::BlockStatement>(
          else_stmt->body(), current_statement_);
      builder_->Sem().Add(else_stmt->body(), sem_block);
      TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block);
      if (!BlockScope(else_stmt->body(),
                      [&] { return Statements(else_stmt->body()->list()); })) {
        return false;
      }
    }
  }
  return true;
}

bool Resolver::LoopStatement(ast::LoopStatement* stmt) {
  // We don't call DetermineBlockStatement on the body and continuing block as
  // these would make their BlockInfo siblings as in the AST, but we want the
  // body BlockInfo to parent the continuing BlockInfo for semantics and
  // validation. Also, we need to set their types differently.
  Mark(stmt->body());

  auto* sem_block_body = builder_->create<sem::LoopBlockStatement>(
      stmt->body(), current_statement_);
  builder_->Sem().Add(stmt->body(), sem_block_body);
  TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block_body);
  return BlockScope(stmt->body(), [&] {
    if (!Statements(stmt->body()->list())) {
      return false;
    }
    if (stmt->continuing()) {  // has_continuing() also checks for empty()
      Mark(stmt->continuing());
    }
    if (stmt->has_continuing()) {
      auto* sem_block_continuing =
          builder_->create<sem::LoopContinuingBlockStatement>(
              stmt->continuing(), current_statement_);
      builder_->Sem().Add(stmt->continuing(), sem_block_continuing);
      TINT_SCOPED_ASSIGNMENT(current_statement_, sem_block_continuing);
      if (!BlockScope(stmt->continuing(),
                      [&] { return Statements(stmt->continuing()->list()); })) {
        return false;
      }
    }
    return true;
  });
}

bool Resolver::Expressions(const ast::ExpressionList& list) {
  for (auto* expr : list) {
    Mark(expr);
    if (!Expression(expr)) {
      return false;
    }
  }
  return true;
}

bool Resolver::Expression(ast::Expression* expr) {
  if (TypeOf(expr)) {
    return true;  // Already resolved
  }

  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return ArrayAccessor(a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return Binary(b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return Bitcast(b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return Call(c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return Constructor(c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return Identifier(i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return MemberAccessor(m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return UnaryOp(u);
  }

  diagnostics_.add_error("unknown expression for type determination",
                         expr->source());
  return false;
}

bool Resolver::ArrayAccessor(ast::ArrayAccessorExpression* expr) {
  Mark(expr->array());
  if (!Expression(expr->array())) {
    return false;
  }
  Mark(expr->idx_expr());
  if (!Expression(expr->idx_expr())) {
    return false;
  }

  auto* res = TypeOf(expr->array());
  auto* parent_type = res->UnwrapRef();
  const sem::Type* ret = nullptr;
  if (auto* arr = parent_type->As<sem::Array>()) {
    ret = arr->ElemType();
  } else if (auto* vec = parent_type->As<sem::Vector>()) {
    ret = vec->type();
  } else if (auto* mat = parent_type->As<sem::Matrix>()) {
    ret = builder_->create<sem::Vector>(mat->type(), mat->rows());
  } else {
    diagnostics_.add_error("invalid parent type (" + parent_type->type_name() +
                               ") in array accessor",
                           expr->source());
    return false;
  }

  // If we're extracting from a reference, we return a reference.
  if (auto* ref = res->As<sem::Reference>()) {
    ret = builder_->create<sem::Reference>(ret, ref->StorageClass());
  }
  SetType(expr, ret);

  return true;
}

bool Resolver::Bitcast(ast::BitcastExpression* expr) {
  Mark(expr->expr());
  if (!Expression(expr->expr())) {
    return false;
  }
  SetType(expr, expr->type());
  return true;
}

bool Resolver::Call(ast::CallExpression* call) {
  if (!Expressions(call->params())) {
    return false;
  }

  // The expression has to be an identifier as you can't store function
  // pointers but, if it isn't we'll just use the normal result determination
  // to be on the safe side.
  Mark(call->func());
  auto* ident = call->func()->As<ast::IdentifierExpression>();
  if (!ident) {
    diagnostics_.add_error("call target is not an identifier", call->source());
    return false;
  }

  auto name = builder_->Symbols().NameFor(ident->symbol());

  auto intrinsic_type = sem::ParseIntrinsicType(name);
  if (intrinsic_type != IntrinsicType::kNone) {
    if (!IntrinsicCall(call, intrinsic_type)) {
      return false;
    }
  } else {
    if (current_function_) {
      auto callee_func_it = symbol_to_function_.find(ident->symbol());
      if (callee_func_it == symbol_to_function_.end()) {
        if (current_function_->declaration->symbol() == ident->symbol()) {
          diagnostics_.add_error("v-0004",
                                 "recursion is not permitted. '" + name +
                                     "' attempted to call itself.",
                                 call->source());
        } else {
          diagnostics_.add_error(
              "v-0006: unable to find called function: " + name,
              call->source());
        }
        return false;
      }
      auto* callee_func = callee_func_it->second;

      // Note: Requires called functions to be resolved first.
      // This is currently guaranteed as functions must be declared before
      // use.
      current_function_->transitive_calls.add(callee_func);
      for (auto* transitive_call : callee_func->transitive_calls) {
        current_function_->transitive_calls.add(transitive_call);
      }

      // We inherit any referenced variables from the callee.
      for (auto* var : callee_func->referenced_module_vars) {
        set_referenced_from_function_if_needed(var, false);
      }
    }

    auto iter = symbol_to_function_.find(ident->symbol());
    if (iter == symbol_to_function_.end()) {
      diagnostics_.add_error(
          "v-0005: function must be declared before use: '" + name + "'",
          call->source());
      return false;
    }

    auto* function = iter->second;
    function_calls_.emplace(call,
                            FunctionCallInfo{function, current_statement_});
    SetType(call, function->return_type, function->return_type_name);
  }

  return true;
}

bool Resolver::IntrinsicCall(ast::CallExpression* call,
                             sem::IntrinsicType intrinsic_type) {
  std::vector<const sem::Type*> arg_tys;
  arg_tys.reserve(call->params().size());
  for (auto* expr : call->params()) {
    arg_tys.emplace_back(TypeOf(expr));
  }

  auto result = intrinsic_table_->Lookup(*builder_, intrinsic_type, arg_tys,
                                         call->source());
  if (!result.intrinsic) {
    // Intrinsic lookup failed.
    diagnostics_.add(result.diagnostics);
    return false;
  }

  builder_->Sem().Add(call, builder_->create<sem::Call>(call, result.intrinsic,
                                                        current_statement_));
  SetType(call, result.intrinsic->ReturnType());
  return true;
}

bool Resolver::Constructor(ast::ConstructorExpression* expr) {
  if (auto* type_ctor = expr->As<ast::TypeConstructorExpression>()) {
    for (auto* value : type_ctor->values()) {
      Mark(value);
      if (!Expression(value)) {
        return false;
      }
    }

    SetType(expr, type_ctor->type());

    const sem::Type* type = TypeOf(expr);

    // Now that the argument types have been determined, make sure that they
    // obey the constructor type rules laid out in
    // https://gpuweb.github.io/gpuweb/wgsl.html#type-constructor-expr.
    if (auto* vec_type = type->As<sem::Vector>()) {
      return ValidateVectorConstructor(type_ctor, vec_type);
    }
    if (auto* mat_type = type->As<sem::Matrix>()) {
      return ValidateMatrixConstructor(type_ctor, mat_type);
    }
    // TODO(crbug.com/tint/634): Validate array constructor
  } else if (auto* scalar_ctor = expr->As<ast::ScalarConstructorExpression>()) {
    Mark(scalar_ctor->literal());
    SetType(expr, TypeOf(scalar_ctor->literal()));
  } else {
    TINT_ICE(diagnostics_) << "unexpected constructor expression type";
  }
  return true;
}

bool Resolver::ValidateVectorConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Vector* vec_type) {
  auto& values = ctor->values();
  auto* elem_type = vec_type->type();
  size_t value_cardinality_sum = 0;
  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapRef();
    if (value_type->is_scalar()) {
      if (elem_type != value_type) {
        diagnostics_.add_error(
            "type in vector constructor does not match vector type: "
            "expected '" +
                elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
                TypeNameOf(value) + "'",
            value->source());
        return false;
      }

      value_cardinality_sum++;
    } else if (auto* value_vec = value_type->As<sem::Vector>()) {
      auto* value_elem_type = value_vec->type();
      // A mismatch of vector type parameter T is only an error if multiple
      // arguments are present. A single argument constructor constitutes a
      // type conversion expression.
      // NOTE: A conversion expression from a vec<bool> to any other vecN<T>
      // is disallowed (see
      // https://gpuweb.github.io/gpuweb/wgsl.html#conversion-expr).
      if (elem_type != value_elem_type &&
          (values.size() > 1u || value_vec->is_bool_vector())) {
        diagnostics_.add_error(
            "type in vector constructor does not match vector type: "
            "expected '" +
                elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
                value_elem_type->FriendlyName(builder_->Symbols()) + "'",
            value->source());
        return false;
      }

      value_cardinality_sum += value_vec->size();
    } else {
      // A vector constructor can only accept vectors and scalars.
      diagnostics_.add_error(
          "expected vector or scalar type in vector constructor; found: " +
              value_type->FriendlyName(builder_->Symbols()),
          value->source());
      return false;
    }
  }

  // A correct vector constructor must either be a zero-value expression
  // or the number of components of all constructor arguments must add up
  // to the vector cardinality.
  if (value_cardinality_sum > 0 && value_cardinality_sum != vec_type->size()) {
    if (values.empty()) {
      TINT_ICE(diagnostics_)
          << "constructor arguments expected to be non-empty!";
    }
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    diagnostics_.add_error(
        "attempted to construct '" + TypeNameOf(ctor) + "' with " +
            std::to_string(value_cardinality_sum) + " component(s)",
        Source::Combine(values_start, values_end));
    return false;
  }
  return true;
}

bool Resolver::ValidateMatrixConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Matrix* matrix_type) {
  auto& values = ctor->values();
  // Zero Value expression
  if (values.empty()) {
    return true;
  }

  auto* elem_type = matrix_type->type();
  if (matrix_type->columns() != values.size()) {
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    diagnostics_.add_error(
        "expected " + std::to_string(matrix_type->columns()) + " '" +
            VectorPretty(matrix_type->rows(), elem_type) + "' arguments in '" +
            TypeNameOf(ctor) + "' constructor, found " +
            std::to_string(values.size()),
        Source::Combine(values_start, values_end));
    return false;
  }

  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapRef();
    auto* value_vec = value_type->As<sem::Vector>();

    if (!value_vec || value_vec->size() != matrix_type->rows() ||
        elem_type != value_vec->type()) {
      diagnostics_.add_error("expected argument type '" +
                                 VectorPretty(matrix_type->rows(), elem_type) +
                                 "' in '" + TypeNameOf(ctor) +
                                 "' constructor, found '" + TypeNameOf(value) +
                                 "'",
                             value->source());
      return false;
    }
  }

  return true;
}

bool Resolver::Identifier(ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol();
  VariableInfo* var;
  if (variable_stack_.get(symbol, &var)) {
    SetType(expr, var->type, var->type_name);

    var->users.push_back(expr);
    set_referenced_from_function_if_needed(var, true);

    if (current_block_) {
      // If identifier is part of a loop continuing block, make sure it
      // doesn't refer to a variable that is bypassed by a continue statement
      // in the loop's body block.
      if (auto* continuing_block =
              current_block_
                  ->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
        auto* loop_block =
            continuing_block->FindFirstParent<sem::LoopBlockStatement>();
        if (loop_block->FirstContinue() != size_t(~0)) {
          auto& decls = loop_block->Decls();
          // If our identifier is in loop_block->decls, make sure its index is
          // less than first_continue
          auto iter = std::find_if(
              decls.begin(), decls.end(),
              [&symbol](auto* v) { return v->symbol() == symbol; });
          if (iter != decls.end()) {
            auto var_decl_index =
                static_cast<size_t>(std::distance(decls.begin(), iter));
            if (var_decl_index >= loop_block->FirstContinue()) {
              diagnostics_.add_error(
                  "continue statement bypasses declaration of '" +
                      builder_->Symbols().NameFor(symbol) +
                      "' in continuing block",
                  expr->source());
              return false;
            }
          }
        }
      }
    }

    return true;
  }

  auto iter = symbol_to_function_.find(symbol);
  if (iter != symbol_to_function_.end()) {
    diagnostics_.add_error("missing '(' for function call",
                           expr->source().End());
    return false;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (sem::ParseIntrinsicType(name) != IntrinsicType::kNone) {
    diagnostics_.add_error("missing '(' for intrinsic call",
                           expr->source().End());
    return false;
  }

  diagnostics_.add_error(
      "v-0006: identifier must be declared before use: " + name,
      expr->source());
  return false;
}

bool Resolver::MemberAccessor(ast::MemberAccessorExpression* expr) {
  Mark(expr->structure());
  if (!Expression(expr->structure())) {
    return false;
  }

  auto* structure = TypeOf(expr->structure());
  auto* storage_type = structure->UnwrapRef();

  sem::Type* ret = nullptr;
  std::vector<uint32_t> swizzle;

  if (auto* str = storage_type->As<sem::Struct>()) {
    Mark(expr->member());
    auto symbol = expr->member()->symbol();

    const sem::StructMember* member = nullptr;
    for (auto* m : str->Members()) {
      if (m->Declaration()->symbol() == symbol) {
        ret = m->Type();
        member = m;
        break;
      }
    }

    if (ret == nullptr) {
      diagnostics_.add_error(
          "struct member " + builder_->Symbols().NameFor(symbol) + " not found",
          expr->source());
      return false;
    }

    // If we're extracting from a reference, we return a reference.
    if (auto* ref = structure->As<sem::Reference>()) {
      ret = builder_->create<sem::Reference>(ret, ref->StorageClass());
    }

    builder_->Sem().Add(expr, builder_->create<sem::StructMemberAccess>(
                                  expr, ret, current_statement_, member));
  } else if (auto* vec = storage_type->As<sem::Vector>()) {
    Mark(expr->member());
    std::string s = builder_->Symbols().NameFor(expr->member()->symbol());
    auto size = s.size();
    swizzle.reserve(s.size());

    for (auto c : s) {
      switch (c) {
        case 'x':
        case 'r':
          swizzle.emplace_back(0);
          break;
        case 'y':
        case 'g':
          swizzle.emplace_back(1);
          break;
        case 'z':
        case 'b':
          swizzle.emplace_back(2);
          break;
        case 'w':
        case 'a':
          swizzle.emplace_back(3);
          break;
        default:
          diagnostics_.add_error(
              "invalid vector swizzle character",
              expr->member()->source().Begin() + swizzle.size());
          return false;
      }
    }

    if (size < 1 || size > 4) {
      diagnostics_.add_error("invalid vector swizzle size",
                             expr->member()->source());
      return false;
    }

    // All characters are valid, check if they're being mixed
    auto is_rgba = [](char c) {
      return c == 'r' || c == 'g' || c == 'b' || c == 'a';
    };
    auto is_xyzw = [](char c) {
      return c == 'x' || c == 'y' || c == 'z' || c == 'w';
    };
    if (!std::all_of(s.begin(), s.end(), is_rgba) &&
        !std::all_of(s.begin(), s.end(), is_xyzw)) {
      diagnostics_.add_error(
          "invalid mixing of vector swizzle characters rgba with xyzw",
          expr->member()->source());
      return false;
    }

    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a reference, we return a reference.
      if (auto* ref = structure->As<sem::Reference>()) {
        ret = builder_->create<sem::Reference>(ret, ref->StorageClass());
      }
    } else {
      // The vector will have a number of components equal to the length of
      // the swizzle.
      ret = builder_->create<sem::Vector>(vec->type(),
                                          static_cast<uint32_t>(size));
    }
    builder_->Sem().Add(
        expr, builder_->create<sem::Swizzle>(expr, ret, current_statement_,
                                             std::move(swizzle)));
  } else {
    diagnostics_.add_error(
        "invalid use of member accessor on a non-vector/non-struct " +
            TypeNameOf(expr->structure()),
        expr->source());
    return false;
  }

  SetType(expr, ret);

  return true;
}

bool Resolver::ValidateBinary(ast::BinaryExpression* expr) {
  using Bool = sem::Bool;
  using F32 = sem::F32;
  using I32 = sem::I32;
  using U32 = sem::U32;
  using Matrix = sem::Matrix;
  using Vector = sem::Vector;

  auto* lhs_type = const_cast<sem::Type*>(TypeOf(expr->lhs())->UnwrapRef());
  auto* rhs_type = const_cast<sem::Type*>(TypeOf(expr->rhs())->UnwrapRef());

  auto* lhs_vec = lhs_type->As<Vector>();
  auto* lhs_vec_elem_type = lhs_vec ? lhs_vec->type() : nullptr;
  auto* rhs_vec = rhs_type->As<Vector>();
  auto* rhs_vec_elem_type = rhs_vec ? rhs_vec->type() : nullptr;

  const bool matching_vec_elem_types =
      lhs_vec_elem_type && rhs_vec_elem_type &&
      (lhs_vec_elem_type == rhs_vec_elem_type) &&
      (lhs_vec->size() == rhs_vec->size());

  const bool matching_types = matching_vec_elem_types || (lhs_type == rhs_type);

  // Binary logical expressions
  if (expr->IsLogicalAnd() || expr->IsLogicalOr()) {
    if (matching_types && lhs_type->Is<Bool>()) {
      return true;
    }
  }
  if (expr->IsOr() || expr->IsAnd()) {
    if (matching_types && lhs_type->Is<Bool>()) {
      return true;
    }
    if (matching_types && lhs_vec_elem_type && lhs_vec_elem_type->Is<Bool>()) {
      return true;
    }
  }

  // Arithmetic expressions
  if (expr->IsArithmetic()) {
    // Binary arithmetic expressions over scalars
    if (matching_types && lhs_type->IsAnyOf<I32, F32, U32>()) {
      return true;
    }

    // Binary arithmetic expressions over vectors
    if (matching_types && lhs_vec_elem_type &&
        lhs_vec_elem_type->IsAnyOf<I32, F32, U32>()) {
      return true;
    }
  }

  // Binary arithmetic expressions with mixed scalar, vector, and matrix
  // operands
  if (expr->IsMultiply()) {
    // Multiplication of a vector and a scalar
    if (lhs_type->Is<F32>() && rhs_vec_elem_type &&
        rhs_vec_elem_type->Is<F32>()) {
      return true;
    }
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_type->Is<F32>()) {
      return true;
    }

    auto* lhs_mat = lhs_type->As<Matrix>();
    auto* lhs_mat_elem_type = lhs_mat ? lhs_mat->type() : nullptr;
    auto* rhs_mat = rhs_type->As<Matrix>();
    auto* rhs_mat_elem_type = rhs_mat ? rhs_mat->type() : nullptr;

    // Multiplication of a matrix and a scalar
    if (lhs_type->Is<F32>() && rhs_mat_elem_type &&
        rhs_mat_elem_type->Is<F32>()) {
      return true;
    }
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_type->Is<F32>()) {
      return true;
    }

    // Vector times matrix
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_vec->size() == rhs_mat->rows())) {
      return true;
    }

    // Matrix times vector
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_vec->size())) {
      return true;
    }

    // Matrix times matrix
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_mat->rows())) {
      return true;
    }
  }

  // Comparison expressions
  if (expr->IsComparison()) {
    if (matching_types) {
      // Special case for bools: only == and !=
      if (lhs_type->Is<Bool>() && (expr->IsEqual() || expr->IsNotEqual())) {
        return true;
      }

      // For the rest, we can compare i32, u32, and f32
      if (lhs_type->IsAnyOf<I32, U32, F32>()) {
        return true;
      }
    }

    // Same for vectors
    if (matching_vec_elem_types) {
      if (lhs_vec_elem_type->Is<Bool>() &&
          (expr->IsEqual() || expr->IsNotEqual())) {
        return true;
      }

      if (lhs_vec_elem_type->IsAnyOf<I32, U32, F32>()) {
        return true;
      }
    }
  }

  // Binary bitwise operations
  if (expr->IsBitwise()) {
    if (matching_types && lhs_type->IsAnyOf<I32, U32>()) {
      return true;
    }
  }

  // Bit shift expressions
  if (expr->IsBitshift()) {
    // Type validation rules are the same for left or right shift, despite
    // differences in computation rules (i.e. right shift can be arithmetic or
    // logical depending on lhs type).

    if (lhs_type->IsAnyOf<I32, U32>() && rhs_type->Is<U32>()) {
      return true;
    }

    if (lhs_vec_elem_type && lhs_vec_elem_type->IsAnyOf<I32, U32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<U32>()) {
      return true;
    }
  }

  diagnostics_.add_error(
      "Binary expression operand types are invalid for this operation: " +
          lhs_type->FriendlyName(builder_->Symbols()) + " " +
          FriendlyName(expr->op()) + " " +
          rhs_type->FriendlyName(builder_->Symbols()),
      expr->source());
  return false;
}

bool Resolver::Binary(ast::BinaryExpression* expr) {
  Mark(expr->lhs());
  Mark(expr->rhs());
  if (!Expression(expr->lhs()) || !Expression(expr->rhs())) {
    return false;
  }

  if (!ValidateBinary(expr)) {
    return false;
  }

  // Result type matches first parameter type
  if (expr->IsAnd() || expr->IsOr() || expr->IsXor() || expr->IsShiftLeft() ||
      expr->IsShiftRight() || expr->IsAdd() || expr->IsSubtract() ||
      expr->IsDivide() || expr->IsModulo()) {
    SetType(expr, TypeOf(expr->lhs())->UnwrapRef());
    return true;
  }
  // Result type is a scalar or vector of boolean type
  if (expr->IsLogicalAnd() || expr->IsLogicalOr() || expr->IsEqual() ||
      expr->IsNotEqual() || expr->IsLessThan() || expr->IsGreaterThan() ||
      expr->IsLessThanEqual() || expr->IsGreaterThanEqual()) {
    auto* bool_type = builder_->create<sem::Bool>();
    auto* param_type = TypeOf(expr->lhs())->UnwrapRef();
    sem::Type* result_type = bool_type;
    if (auto* vec = param_type->As<sem::Vector>()) {
      result_type = builder_->create<sem::Vector>(bool_type, vec->size());
    }
    SetType(expr, result_type);
    return true;
  }
  if (expr->IsMultiply()) {
    auto* lhs_type = TypeOf(expr->lhs())->UnwrapRef();
    auto* rhs_type = TypeOf(expr->rhs())->UnwrapRef();

    // Note, the ordering here matters. The later checks depend on the prior
    // checks having been done.
    auto* lhs_mat = lhs_type->As<sem::Matrix>();
    auto* rhs_mat = rhs_type->As<sem::Matrix>();
    auto* lhs_vec = lhs_type->As<sem::Vector>();
    auto* rhs_vec = rhs_type->As<sem::Vector>();
    const sem::Type* result_type = nullptr;
    if (lhs_mat && rhs_mat) {
      auto* column_type =
          builder_->create<sem::Vector>(lhs_mat->type(), lhs_mat->rows());
      result_type =
          builder_->create<sem::Matrix>(column_type, rhs_mat->columns());
    } else if (lhs_mat && rhs_vec) {
      result_type =
          builder_->create<sem::Vector>(lhs_mat->type(), lhs_mat->rows());
    } else if (lhs_vec && rhs_mat) {
      result_type =
          builder_->create<sem::Vector>(rhs_mat->type(), rhs_mat->columns());
    } else if (lhs_mat) {
      // matrix * scalar
      result_type = lhs_type;
    } else if (rhs_mat) {
      // scalar * matrix
      result_type = rhs_type;
    } else if (lhs_vec && rhs_vec) {
      result_type = lhs_type;
    } else if (lhs_vec) {
      // Vector * scalar
      result_type = lhs_type;
    } else if (rhs_vec) {
      // Scalar * vector
      result_type = rhs_type;
    } else {
      // Scalar * Scalar
      result_type = lhs_type;
    }

    SetType(expr, result_type);
    return true;
  }

  diagnostics_.add_error("Unknown binary expression", expr->source());
  return false;
}

bool Resolver::UnaryOp(ast::UnaryOpExpression* unary) {
  Mark(unary->expr());

  // Resolve the inner expression
  if (!Expression(unary->expr())) {
    return false;
  }

  auto* expr_type = TypeOf(unary->expr());
  if (!expr_type) {
    return false;
  }

  std::string type_name;
  const sem::Type* type = nullptr;

  switch (unary->op()) {
    case ast::UnaryOp::kNegation:
    case ast::UnaryOp::kNot:
      // Result type matches the deref'd inner type.
      type_name = TypeNameOf(unary->expr());
      type = expr_type->UnwrapRef();
      break;

    case ast::UnaryOp::kAddressOf:
      if (auto* ref = expr_type->As<sem::Reference>()) {
        type = builder_->create<sem::Pointer>(ref->StoreType(),
                                              ref->StorageClass());
      } else {
        diagnostics_.add_error("cannot take the address of expression",
                               unary->expr()->source());
        return false;
      }
      break;

    case ast::UnaryOp::kIndirection:
      if (auto* ptr = expr_type->As<sem::Pointer>()) {
        type = builder_->create<sem::Reference>(ptr->StoreType(),
                                                ptr->StorageClass());
      } else {
        diagnostics_.add_error("cannot dereference expression of type '" +
                                   TypeNameOf(unary->expr()) + "'",
                               unary->expr()->source());
        return false;
      }
      break;
  }

  SetType(unary, type);
  return true;
}

bool Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
  ast::Variable* var = stmt->variable();
  Mark(var);

  bool is_global = false;
  if (variable_stack_.get(var->symbol(), nullptr, &is_global)) {
    const char* error_code = is_global ? "v-0013" : "v-0014";
    diagnostics_.add_error(error_code,
                           "redeclared identifier '" +
                               builder_->Symbols().NameFor(var->symbol()) + "'",
                           var->source());
    return false;
  }

  auto* info = Variable(var, VariableKind::kLocal);
  if (!info) {
    return false;
  }

  for (auto* deco : var->decorations()) {
    // TODO(bclayton): Validate decorations
    Mark(deco);
  }

  variable_stack_.set(var->symbol(), info);
  current_block_->AddDecl(var);

  if (!ValidateVariable(info)) {
    return false;
  }

  if (!var->is_const()) {
    if (info->storage_class != ast::StorageClass::kFunction) {
      if (info->storage_class != ast::StorageClass::kNone) {
        diagnostics_.add_error(
            "function variable has a non-function storage class",
            stmt->source());
        return false;
      }
      info->storage_class = ast::StorageClass::kFunction;
    }
  }

  if (!ApplyStorageClassUsageToType(info->storage_class, info->type,
                                    var->source())) {
    diagnostics_.add_note("while instantiating variable " +
                              builder_->Symbols().NameFor(var->symbol()),
                          var->source());
    return false;
  }

  return true;
}

sem::Type* Resolver::NamedType(const ast::NamedType* named_type) {
  sem::Type* result = nullptr;
  if (auto* alias = named_type->As<ast::Alias>()) {
    result = Type(alias->type());
  } else if (auto* str = named_type->As<ast::Struct>()) {
    result = Structure(str);
  } else {
    TINT_UNREACHABLE(diagnostics_) << "Unhandled NamedType";
  }

  if (!result) {
    return nullptr;
  }

  named_type_info_.emplace(named_type->name(),
                           NamedTypeInfo{named_type, result});

  if (!ValidateNamedType(named_type)) {
    return nullptr;
  }

  builder_->Sem().Add(named_type, result);
  return result;
}

bool Resolver::ValidateNamedType(const ast::NamedType* named_type) const {
  auto iter = named_type_info_.find(named_type->name());
  if (iter == named_type_info_.end()) {
    TINT_ICE(diagnostics_) << "ValidateNamedType called() before NamedType()";
  }
  if (iter->second.ast != named_type) {
    diagnostics_.add_error("type with the name '" +
                               builder_->Symbols().NameFor(named_type->name()) +
                               "' was already declared",
                           named_type->source());
    diagnostics_.add_note("first declared here", iter->second.ast->source());
    return false;
  }
  return true;
}

sem::Type* Resolver::TypeOf(const ast::Expression* expr) {
  auto it = expr_info_.find(expr);
  if (it != expr_info_.end()) {
    return const_cast<sem::Type*>(it->second.type);
  }
  return nullptr;
}

std::string Resolver::TypeNameOf(const ast::Expression* expr) {
  auto it = expr_info_.find(expr);
  if (it != expr_info_.end()) {
    return it->second.type_name;
  }
  return "";
}

sem::Type* Resolver::TypeOf(const ast::Literal* lit) {
  if (lit->Is<ast::SintLiteral>()) {
    return builder_->create<sem::I32>();
  }
  if (lit->Is<ast::UintLiteral>()) {
    return builder_->create<sem::U32>();
  }
  if (lit->Is<ast::FloatLiteral>()) {
    return builder_->create<sem::F32>();
  }
  if (lit->Is<ast::BoolLiteral>()) {
    return builder_->create<sem::Bool>();
  }
  TINT_UNREACHABLE(diagnostics_)
      << "Unhandled literal type: " << lit->TypeInfo().name;
  return nullptr;
}

void Resolver::SetType(ast::Expression* expr, const ast::Type* type) {
  SetType(expr, Type(type), type->FriendlyName(builder_->Symbols()));
}

void Resolver::SetType(ast::Expression* expr, const sem::Type* type) {
  SetType(expr, type, type->FriendlyName(builder_->Symbols()));
}

void Resolver::SetType(ast::Expression* expr,
                       const sem::Type* type,
                       const std::string& type_name) {
  if (expr_info_.count(expr)) {
    TINT_ICE(diagnostics_) << "SetType() called twice for the same expression";
  }
  expr_info_.emplace(expr, ExpressionInfo{type, type_name, current_statement_});
}

void Resolver::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  // Collate all the 'ancestor_entry_points' - this is a map of function
  // symbol to all the entry points that transitively call the function.
  std::unordered_map<Symbol, std::vector<Symbol>> ancestor_entry_points;
  for (auto* func : builder_->AST().Functions()) {
    auto it = function_to_info_.find(func);
    if (it == function_to_info_.end()) {
      continue;  // Resolver has likely errored. Process what we can.
    }

    auto* info = it->second;
    if (!func->IsEntryPoint()) {
      continue;
    }
    for (auto* call : info->transitive_calls) {
      auto& vec = ancestor_entry_points[call->declaration->symbol()];
      vec.emplace_back(func->symbol());
    }
  }

  // The next pipeline constant ID to try to allocate.
  uint16_t next_constant_id = 0;

  // Create semantic nodes for all ast::Variables
  for (auto it : variable_to_info_) {
    auto* var = it.first;
    auto* info = it.second;

    sem::Variable* sem_var = nullptr;

    if (auto* override_deco =
            ast::GetDecoration<ast::OverrideDecoration>(var->decorations())) {
      // Create a pipeline overridable constant.
      uint16_t constant_id;
      if (override_deco->HasValue()) {
        constant_id = static_cast<uint16_t>(override_deco->value());
      } else {
        // No ID was specified, so allocate the next available ID.
        constant_id = next_constant_id;
        while (constant_ids_.count(constant_id)) {
          if (constant_id == UINT16_MAX) {
            TINT_ICE(builder_->Diagnostics())
                << "no more pipeline constant IDs available";
            return;
          }
          constant_id++;
        }
        next_constant_id = constant_id + 1;
      }

      sem_var = builder_->create<sem::Variable>(var, info->type, constant_id);
    } else {
      sem_var = builder_->create<sem::Variable>(
          var, info->type, info->storage_class,
          info->access_control ? info->access_control->access_control()
                               : ast::AccessControl::kReadWrite);
    }

    std::vector<const sem::VariableUser*> users;
    for (auto* user : info->users) {
      // Create semantic node for the identifier expression if necessary
      auto* sem_expr = sem.Get(user);
      if (sem_expr == nullptr) {
        auto* type = expr_info_.at(user).type;
        auto* stmt = expr_info_.at(user).statement;
        auto* sem_user =
            builder_->create<sem::VariableUser>(user, type, stmt, sem_var);
        sem_var->AddUser(sem_user);
        sem.Add(user, sem_user);
      } else {
        auto* sem_user = sem_expr->As<sem::VariableUser>();
        if (!sem_user) {
          TINT_ICE(diagnostics_) << "expected sem::VariableUser, got "
                                 << sem_expr->TypeInfo().name;
        }
        sem_var->AddUser(sem_user);
      }
    }
    sem.Add(var, sem_var);
  }

  auto remap_vars = [&sem](const std::vector<VariableInfo*>& in) {
    std::vector<const sem::Variable*> out;
    out.reserve(in.size());
    for (auto* info : in) {
      out.emplace_back(sem.Get(info->declaration));
    }
    return out;
  };

  // Create semantic nodes for all ast::Functions
  std::unordered_map<FunctionInfo*, sem::Function*> func_info_to_sem_func;
  for (auto it : function_to_info_) {
    auto* func = it.first;
    auto* info = it.second;

    auto* sem_func = builder_->create<sem::Function>(
        info->declaration, const_cast<sem::Type*>(info->return_type),
        remap_vars(info->parameters), remap_vars(info->referenced_module_vars),
        remap_vars(info->local_referenced_module_vars), info->return_statements,
        ancestor_entry_points[func->symbol()], info->workgroup_size);
    func_info_to_sem_func.emplace(info, sem_func);
    sem.Add(func, sem_func);
  }

  // Create semantic nodes for all ast::CallExpressions
  for (auto it : function_calls_) {
    auto* call = it.first;
    auto info = it.second;
    auto* sem_func = func_info_to_sem_func.at(info.function);
    sem.Add(call, builder_->create<sem::Call>(call, sem_func, info.statement));
  }

  // Create semantic nodes for all remaining expression types
  for (auto it : expr_info_) {
    auto* expr = it.first;
    auto& info = it.second;
    if (sem.Get(expr)) {
      // Expression has already been assigned a semantic node
      continue;
    }
    sem.Add(expr,
            builder_->create<sem::Expression>(
                const_cast<ast::Expression*>(expr), info.type, info.statement));
  }
}

bool Resolver::DefaultAlignAndSize(const sem::Type* ty,
                                   uint32_t& align,
                                   uint32_t& size) {
  static constexpr uint32_t vector_size[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 12,
      /*vec4*/ 16,
  };
  static constexpr uint32_t vector_align[] = {
      /* padding */ 0,
      /* padding */ 0,
      /*vec2*/ 8,
      /*vec3*/ 16,
      /*vec4*/ 16,
  };

  if (ty->is_scalar()) {
    // Note: Also captures booleans, but these are not host-shareable.
    align = 4;
    size = 4;
    return true;
  } else if (auto* vec = ty->As<sem::Vector>()) {
    if (vec->size() < 2 || vec->size() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid vector size: vec" << vec->size();
      return false;
    }
    align = vector_align[vec->size()];
    size = vector_size[vec->size()];
    return true;
  } else if (auto* mat = ty->As<sem::Matrix>()) {
    if (mat->columns() < 2 || mat->columns() > 4 || mat->rows() < 2 ||
        mat->rows() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid matrix size: mat" << mat->columns() << "x" << mat->rows();
      return false;
    }
    align = vector_align[mat->rows()];
    size = vector_align[mat->rows()] * mat->columns();
    return true;
  } else if (auto* s = ty->As<sem::Struct>()) {
    align = s->Align();
    size = s->Size();
    return true;
  } else if (auto* a = ty->As<sem::Array>()) {
    align = a->Align();
    size = a->SizeInBytes();
    return true;
  }
  TINT_UNREACHABLE(diagnostics_) << "Invalid type " << ty->TypeInfo().name;
  return false;
}

sem::Array* Resolver::Array(const ast::Array* arr) {
  auto source = arr->source();

  auto* el_ty = Type(arr->type());
  if (!el_ty) {
    return nullptr;
  }

  uint32_t el_align = 0;
  uint32_t el_size = 0;
  if (!DefaultAlignAndSize(el_ty, el_align, el_size)) {
    return nullptr;
  }

  // Look for explicit stride via [[stride(n)]] decoration
  uint32_t explicit_stride = 0;
  for (auto* deco : arr->decorations()) {
    Mark(deco);
    if (auto* sd = deco->As<ast::StrideDecoration>()) {
      if (explicit_stride) {
        diagnostics_.add_error(
            "array must have at most one [[stride]] decoration", source);
        return nullptr;
      }
      explicit_stride = sd->stride();
      if (!ValidateArrayStrideDecoration(sd, el_size, el_align, source)) {
        return nullptr;
      }
      continue;
    }

    diagnostics_.add_error("decoration is not valid for array types",
                           deco->source());
    return nullptr;
  }

  // Calculate implicit stride
  auto implicit_stride = utils::RoundUp(el_align, el_size);

  auto stride = explicit_stride ? explicit_stride : implicit_stride;

  // WebGPU requires runtime arrays have at least one element, but the AST
  // records an element count of 0 for it.
  auto size = std::max<uint32_t>(arr->size(), 1) * stride;
  auto* sem = builder_->create<sem::Array>(el_ty, arr->size(), el_align, size,
                                           stride, stride == implicit_stride);

  if (!ValidateArray(sem, source)) {
    return nullptr;
  }

  return sem;
}

bool Resolver::ValidateArray(const sem::Array* arr, const Source& source) {
  auto* el_ty = arr->ElemType();

  if (!IsStorable(el_ty)) {
    builder_->Diagnostics().add_error(
        el_ty->FriendlyName(builder_->Symbols()) +
            " cannot be used as an element type of an array",
        source);
    return false;
  }

  if (auto* el_str = el_ty->As<sem::Struct>()) {
    if (el_str->IsBlockDecorated()) {
      // https://gpuweb.github.io/gpuweb/wgsl/#attributes
      // A structure type with the block attribute must not be:
      // * the element type of an array type
      // * the member type in another structure
      diagnostics_.add_error(
          "A structure type with a [[block]] decoration cannot be used as an "
          "element of an array",
          source);
      return false;
    }
  }
  return true;
}

bool Resolver::ValidateArrayStrideDecoration(const ast::StrideDecoration* deco,
                                             uint32_t el_size,
                                             uint32_t el_align,
                                             const Source& source) {
  auto stride = deco->stride();
  bool is_valid_stride =
      (stride >= el_size) && (stride >= el_align) && (stride % el_align == 0);
  if (!is_valid_stride) {
    // https://gpuweb.github.io/gpuweb/wgsl/#array-layout-rules
    // Arrays decorated with the stride attribute must have a stride that is
    // at least the size of the element type, and be a multiple of the
    // element type's alignment value.
    diagnostics_.add_error(
        "arrays decorated with the stride attribute must have a stride "
        "that is at least the size of the element type, and be a multiple "
        "of the element type's alignment value.",
        source);
    return false;
  }
  return true;
}

bool Resolver::ValidateStructure(const sem::Struct* str) {
  for (auto* member : str->Members()) {
    if (auto* r = member->Type()->As<sem::Array>()) {
      if (r->IsRuntimeSized()) {
        if (member != str->Members().back()) {
          diagnostics_.add_error(
              "v-0015",
              "runtime arrays may only appear as the last member of a struct",
              member->Declaration()->source());
          return false;
        }
        if (!str->IsBlockDecorated()) {
          diagnostics_.add_error(
              "v-0015",
              "a struct containing a runtime-sized array "
              "requires the [[block]] attribute: '" +
                  builder_->Symbols().NameFor(str->Declaration()->name()) + "'",
              member->Declaration()->source());
          return false;
        }
      }
    }

    for (auto* deco : member->Declaration()->decorations()) {
      if (!(deco->Is<ast::BuiltinDecoration>() ||
            deco->Is<ast::LocationDecoration>() ||
            deco->Is<ast::StructMemberOffsetDecoration>() ||
            deco->Is<ast::StructMemberSizeDecoration>() ||
            deco->Is<ast::StructMemberAlignDecoration>())) {
        diagnostics_.add_error("decoration is not valid for structure members",
                               deco->source());
        return false;
      }
    }
  }

  for (auto* deco : str->Declaration()->decorations()) {
    if (!(deco->Is<ast::StructBlockDecoration>())) {
      diagnostics_.add_error("decoration is not valid for struct declarations",
                             deco->source());
      return false;
    }
  }

  return true;
}

sem::Struct* Resolver::Structure(const ast::Struct* str) {
  for (auto* deco : str->decorations()) {
    Mark(deco);
  }

  sem::StructMemberList sem_members;
  sem_members.reserve(str->members().size());

  // Calculate the effective size and alignment of each field, and the overall
  // size of the structure.
  // For size, use the size attribute if provided, otherwise use the default
  // size for the type.
  // For alignment, use the alignment attribute if provided, otherwise use the
  // default alignment for the member type.
  // Diagnostic errors are raised if a basic rule is violated.
  // Validation of storage-class rules requires analysing the actual variable
  // usage of the structure, and so is performed as part of the variable
  // validation.
  // TODO(crbug.com/tint/628): Actually implement storage-class validation.
  uint32_t struct_size = 0;
  uint32_t struct_align = 1;

  for (auto* member : str->members()) {
    Mark(member);

    // Resolve member type
    auto* type = Type(member->type());
    if (!type) {
      return nullptr;
    }

    // Validate member type
    if (!IsStorable(type)) {
      diagnostics_.add_error(
          type->FriendlyName(builder_->Symbols()) +
          " cannot be used as the type of a structure member");
      return nullptr;
    }

    uint32_t offset = struct_size;
    uint32_t align = 0;
    uint32_t size = 0;
    if (!DefaultAlignAndSize(type, align, size)) {
      return nullptr;
    }

    bool has_offset_deco = false;
    bool has_align_deco = false;
    bool has_size_deco = false;
    for (auto* deco : member->decorations()) {
      Mark(deco);
      if (auto* o = deco->As<ast::StructMemberOffsetDecoration>()) {
        // Offset decorations are not part of the WGSL spec, but are emitted
        // by the SPIR-V reader.
        if (o->offset() < struct_size) {
          diagnostics_.add_error("offsets must be in ascending order",
                                 o->source());
          return nullptr;
        }
        offset = o->offset();
        align = 1;
        has_offset_deco = true;
      } else if (auto* a = deco->As<ast::StructMemberAlignDecoration>()) {
        if (a->align() <= 0 || !utils::IsPowerOfTwo(a->align())) {
          diagnostics_.add_error(
              "align value must be a positive, power-of-two integer",
              a->source());
          return nullptr;
        }
        align = a->align();
        has_align_deco = true;
      } else if (auto* s = deco->As<ast::StructMemberSizeDecoration>()) {
        if (s->size() < size) {
          diagnostics_.add_error(
              "size must be at least as big as the type's size (" +
                  std::to_string(size) + ")",
              s->source());
          return nullptr;
        }
        size = s->size();
        has_size_deco = true;
      }
    }

    if (has_offset_deco && (has_align_deco || has_size_deco)) {
      diagnostics_.add_error(
          "offset decorations cannot be used with align or size decorations",
          member->source());
      return nullptr;
    }

    offset = utils::RoundUp(align, offset);

    auto* sem_member = builder_->create<sem::StructMember>(
        member, const_cast<sem::Type*>(type),
        static_cast<uint32_t>(sem_members.size()), offset, align, size);
    builder_->Sem().Add(member, sem_member);
    sem_members.emplace_back(sem_member);

    struct_size = offset + size;
    struct_align = std::max(struct_align, align);
  }

  auto size_no_padding = struct_size;
  struct_size = utils::RoundUp(struct_align, struct_size);

  auto* out = builder_->create<sem::Struct>(
      str, std::move(sem_members), struct_align, struct_size, size_no_padding);

  if (!ValidateStructure(out)) {
    return nullptr;
  }

  return out;
}

bool Resolver::ValidateReturn(const ast::ReturnStatement* ret) {
  auto* func_type = current_function_->return_type;

  auto* ret_type = ret->has_value() ? TypeOf(ret->value())->UnwrapRef()
                                    : builder_->create<sem::Void>();

  if (func_type->UnwrapRef() != ret_type) {
    diagnostics_.add_error("v-000y",
                           "return statement type must match its function "
                           "return type, returned '" +
                               ret_type->FriendlyName(builder_->Symbols()) +
                               "', expected '" +
                               current_function_->return_type_name + "'",
                           ret->source());
    return false;
  }

  return true;
}

bool Resolver::Return(ast::ReturnStatement* ret) {
  current_function_->return_statements.push_back(ret);

  if (auto* value = ret->value()) {
    Mark(value);

    // Validate after processing the return value expression so that its type
    // is available for validation
    return Expression(value) && ValidateReturn(ret);
  }

  return true;
}

bool Resolver::ValidateSwitch(const ast::SwitchStatement* s) {
  auto* cond_type = TypeOf(s->condition())->UnwrapRef();
  if (!cond_type->is_integer_scalar()) {
    diagnostics_.add_error("v-0025",
                           "switch statement selector expression must be of a "
                           "scalar integer type",
                           s->condition()->source());
    return false;
  }

  bool has_default = false;
  std::unordered_set<uint32_t> selector_set;

  for (auto* case_stmt : s->body()) {
    if (case_stmt->IsDefault()) {
      if (has_default) {
        // More than one default clause
        diagnostics_.add_error(
            "v-0008", "switch statement must have exactly one default clause",
            case_stmt->source());
        return false;
      }
      has_default = true;
    }

    for (auto* selector : case_stmt->selectors()) {
      if (cond_type != TypeOf(selector)) {
        diagnostics_.add_error("v-0026",
                               "the case selector values must have the same "
                               "type as the selector expression.",
                               case_stmt->source());
        return false;
      }

      auto v = selector->value_as_u32();
      if (selector_set.find(v) != selector_set.end()) {
        diagnostics_.add_error(
            "v-0027",
            "a literal value must not appear more than once in "
            "the case selectors for a switch statement: '" +
                builder_->str(selector) + "'",
            case_stmt->source());
        return false;
      }
      selector_set.emplace(v);
    }
  }

  if (!has_default) {
    // No default clause
    diagnostics_.add_error("switch statement must have a default clause",
                           s->source());
    return false;
  }

  if (!s->body().empty()) {
    auto* last_clause = s->body().back()->As<ast::CaseStatement>();
    auto* last_stmt = last_clause->body()->last();
    if (last_stmt && last_stmt->Is<ast::FallthroughStatement>()) {
      diagnostics_.add_error("v-0028",
                             "a fallthrough statement must not appear as "
                             "the last statement in last clause of a switch",
                             last_stmt->source());
      return false;
    }
  }

  return true;
}

bool Resolver::Switch(ast::SwitchStatement* s) {
  Mark(s->condition());
  if (!Expression(s->condition())) {
    return false;
  }
  for (auto* case_stmt : s->body()) {
    Mark(case_stmt);

    sem::Statement* sem_statement =
        builder_->create<sem::Statement>(case_stmt, current_statement_);
    builder_->Sem().Add(case_stmt, sem_statement);
    TINT_SCOPED_ASSIGNMENT(current_statement_, sem_statement);
    if (!CaseStatement(case_stmt)) {
      return false;
    }
  }
  if (!ValidateSwitch(s)) {
    return false;
  }
  return true;
}

bool Resolver::Assignment(ast::AssignmentStatement* a) {
  Mark(a->lhs());
  Mark(a->rhs());

  if (!Expression(a->lhs()) || !Expression(a->rhs())) {
    return false;
  }
  return ValidateAssignment(a);
}

bool Resolver::ValidateAssignment(const ast::AssignmentStatement* a) {
  // https://gpuweb.github.io/gpuweb/wgsl/#assignment-statement
  auto const* lhs_type = TypeOf(a->lhs());
  auto const* rhs_type = TypeOf(a->rhs());

  auto* lhs_ref = lhs_type->As<sem::Reference>();
  if (!lhs_ref) {
    // LHS is not a reference, so it has no storage.
    diagnostics_.add_error(
        "cannot assign to value of type '" + TypeNameOf(a->lhs()) + "'",
        a->lhs()->source());

    return false;
  }

  auto* storage_type = lhs_ref->StoreType();

  // TODO(crbug.com/tint/809): The originating variable of the left-hand side
  // must not have an access(read) access attribute.
  // https://gpuweb.github.io/gpuweb/wgsl/#assignment

  auto* value_type = rhs_type->UnwrapRef();  // Implicit load of RHS

  // RHS needs to be of a storable type
  if (!IsStorable(value_type)) {
    diagnostics_.add_error("'" + TypeNameOf(a->rhs()) + "' is not storable",
                           a->rhs()->source());
    return false;
  }

  // Value type has to match storage type
  if (storage_type != value_type) {
    diagnostics_.add_error("cannot assign '" + TypeNameOf(a->rhs()) + "' to '" +
                               TypeNameOf(a->lhs()) + "'",
                           a->source());
    return false;
  }

  return true;
}

bool Resolver::ApplyStorageClassUsageToType(ast::StorageClass sc,
                                            sem::Type* ty,
                                            const Source& usage) {
  ty = const_cast<sem::Type*>(ty->UnwrapRef());

  if (auto* str = ty->As<sem::Struct>()) {
    if (str->StorageClassUsage().count(sc)) {
      return true;  // Already applied
    }

    str->AddUsage(sc);

    for (auto* member : str->Members()) {
      if (!ApplyStorageClassUsageToType(sc, member->Type(), usage)) {
        std::stringstream err;
        err << "while analysing structure member "
            << str->FriendlyName(builder_->Symbols()) << "."
            << builder_->Symbols().NameFor(member->Declaration()->symbol());
        diagnostics_.add_note(err.str(), member->Declaration()->source());
        return false;
      }
    }
    return true;
  }

  if (auto* arr = ty->As<sem::Array>()) {
    return ApplyStorageClassUsageToType(
        sc, const_cast<sem::Type*>(arr->ElemType()), usage);
  }

  if (ast::IsHostShareable(sc) && !IsHostShareable(ty)) {
    std::stringstream err;
    err << "Type '" << ty->FriendlyName(builder_->Symbols())
        << "' cannot be used in storage class '" << sc
        << "' as it is non-host-shareable";
    diagnostics_.add_error(err.str(), usage);
    return false;
  }

  return true;
}

template <typename T>
bool Resolver::GetScalarConstExprValue(ast::Expression* expr, T* result) {
  if (auto* type_constructor = expr->As<ast::TypeConstructorExpression>()) {
    if (type_constructor->values().size() == 0) {
      // Zero-valued constructor.
      *result = static_cast<T>(0);
      return true;
    } else if (type_constructor->values().size() == 1) {
      // Recurse into the constructor argument expression.
      return GetScalarConstExprValue(type_constructor->values()[0], result);
    } else {
      TINT_ICE(diagnostics_) << "malformed scalar type constructor";
    }
  } else if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    // Cast literal to result type.
    if (auto* int_lit = scalar->literal()->As<ast::IntLiteral>()) {
      *result = static_cast<T>(int_lit->value_as_u32());
      return true;
    } else if (auto* float_lit = scalar->literal()->As<ast::FloatLiteral>()) {
      *result = static_cast<T>(float_lit->value());
      return true;
    } else if (auto* bool_lit = scalar->literal()->As<ast::BoolLiteral>()) {
      *result = static_cast<T>(bool_lit->IsTrue());
      return true;
    } else {
      TINT_ICE(diagnostics_) << "unhandled scalar constructor";
    }
  } else {
    TINT_ICE(diagnostics_) << "unhandled constant expression";
  }

  return false;
}

template <typename F>
bool Resolver::BlockScope(const ast::BlockStatement* block, F&& callback) {
  auto* sem_block = builder_->Sem().Get<sem::BlockStatement>(block);
  if (!sem_block) {
    TINT_ICE(diagnostics_) << "Resolver::BlockScope() called on a block for "
                              "which semantic information is not available";
    return false;
  }
  TINT_SCOPED_ASSIGNMENT(current_block_,
                         const_cast<sem::BlockStatement*>(sem_block));
  variable_stack_.push_scope();
  bool result = callback();
  variable_stack_.pop_scope();
  return result;
}

std::string Resolver::VectorPretty(uint32_t size,
                                   const sem::Type* element_type) {
  sem::Vector vec_type(element_type, size);
  return vec_type.FriendlyName(builder_->Symbols());
}

void Resolver::Mark(const ast::Node* node) {
  if (node == nullptr) {
    TINT_ICE(diagnostics_) << "Resolver::Mark() called with nullptr";
  }
  if (marked_.emplace(node).second) {
    return;
  }
  TINT_ICE(diagnostics_)
      << "AST node '" << node->TypeInfo().name
      << "' was encountered twice in the same AST of a Program\n"
      << "At: " << node->source() << "\n"
      << "Content: " << builder_->str(node) << "\n"
      << "Pointer: " << node;
}

Resolver::VariableInfo::VariableInfo(const ast::Variable* decl,
                                     sem::Type* ty,
                                     const std::string& tn,
                                     ast::StorageClass sc,
                                     const ast::AccessControl* ac)
    : declaration(decl),
      type(ty),
      type_name(tn),
      storage_class(sc),
      access_control(ac) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}
Resolver::FunctionInfo::~FunctionInfo() = default;

}  // namespace resolver
}  // namespace tint
