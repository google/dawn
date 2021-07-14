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
#include "src/ast/for_loop_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/interpolate_decoration.h"
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
#include "src/sem/atomic_type.h"
#include "src/sem/call.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/for_loop_statement.h"
#include "src/sem/function.h"
#include "src/sem/if_statement.h"
#include "src/sem/loop_statement.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/sampler_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/switch_statement.h"
#include "src/sem/variable.h"
#include "src/utils/defer.h"
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

/// @returns true if the decoration list does not contains a
/// ast::DisableValidationDecoration with the validation mode equal to
/// `validation`
bool IsValidationEnabled(const ast::DecorationList& decorations,
                         ast::DisabledValidation validation) {
  return !IsValidationDisabled(decorations, validation);
}

// Helper to stringify a pipeline IO decoration.
std::string deco_to_str(const ast::Decoration* deco) {
  std::stringstream str;
  if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
    str << "builtin(" << builtin->value() << ")";
  } else if (auto* location = deco->As<ast::LocationDecoration>()) {
    str << "location(" << location->value() << ")";
  }
  return str.str();
}
}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder),
      diagnostics_(builder->Diagnostics()),
      intrinsic_table_(IntrinsicTable::Create(*builder)) {}

Resolver::~Resolver() = default;

void Resolver::set_referenced_from_function_if_needed(VariableInfo* var,
                                                      bool local) {
  if (current_function_ == nullptr) {
    return;
  }

  if (var->kind != VariableKind::kGlobal) {
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
    TINT_ICE(Resolver, diagnostics_)
        << "resolving failed, but no error was raised";
    return false;
  }

  // Even if resolving failed, create all the semantic nodes for information we
  // did generate.
  CreateSemanticNodes();

  return result;
}

// https://gpuweb.github.io/gpuweb/wgsl/#plain-types-section
bool Resolver::IsPlain(const sem::Type* type) const {
  return type->is_scalar() || type->Is<sem::Atomic>() ||
         type->Is<sem::Vector>() || type->Is<sem::Matrix>() ||
         type->Is<sem::Array>() || type->Is<sem::Struct>();
}

// https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
bool Resolver::IsConstructible(const sem::Type* type) const {
  if (type->Is<sem::Atomic>()) {
    return false;
  }

  if (type->is_scalar() || type->Is<sem::Vector>() || type->Is<sem::Matrix>()) {
    return true;
  }

  if (auto* arr = type->As<sem::Array>()) {
    if (arr->IsRuntimeSized()) {
      return false;
    }

    return IsConstructible(arr->ElemType());
  }

  if (auto* str = type->As<sem::Struct>()) {
    for (auto* m : str->Members()) {
      if (!IsConstructible(m->Type())) {
        return false;
      }
    }
    return true;
  }

  return false;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Resolver::IsStorable(const sem::Type* type) const {
  return IsPlain(type) || type->Is<sem::Texture>() || type->Is<sem::Sampler>();
}

// https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable-types
bool Resolver::IsHostShareable(const sem::Type* type) const {
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
  if (auto* atomic = type->As<sem::Atomic>()) {
    return IsHostShareable(atomic->Type());
  }
  return false;
}

bool Resolver::ResolveInternal() {
  Mark(&builder_->AST());

  // Process everything else in the order they appear in the module. This is
  // necessary for validation of use-before-declaration.
  for (auto* decl : builder_->AST().GlobalDeclarations()) {
    if (auto* td = decl->As<ast::TypeDecl>()) {
      Mark(td);
      if (!TypeDecl(td)) {
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
      TINT_UNREACHABLE(Resolver, diagnostics_)
          << "unhandled global declaration: " << decl->TypeInfo().name;
      return false;
    }
  }

  if (!ValidatePipelineStages()) {
    return false;
  }
  if (!ValidateAtomicUses()) {
    return false;
  }

  bool result = true;

  for (auto* node : builder_->ASTNodes().Objects()) {
    if (marked_.count(node) == 0) {
      TINT_ICE(Resolver, diagnostics_)
          << "AST node '" << node->TypeInfo().name
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
    if (auto* t = ty->As<ast::Vector>()) {
      if (auto* el = Type(t->type())) {
        if (auto* vector = builder_->create<sem::Vector>(
                const_cast<sem::Type*>(el), t->size())) {
          if (ValidateVector(vector, t->source())) {
            return vector;
          }
        }
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Matrix>()) {
      if (auto* el = Type(t->type())) {
        if (auto* column_type = builder_->create<sem::Vector>(
                const_cast<sem::Type*>(el), t->rows())) {
          if (auto* matrix =
                  builder_->create<sem::Matrix>(column_type, t->columns())) {
            if (ValidateMatrix(matrix, t->source())) {
              return matrix;
            }
          }
        }
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Array>()) {
      return Array(t);
    }
    if (auto* t = ty->As<ast::Atomic>()) {
      if (auto* el = Type(t->type())) {
        auto* a = builder_->create<sem::Atomic>(const_cast<sem::Type*>(el));
        if (!ValidateAtomic(t, a)) {
          return nullptr;
        }
        return a;
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Pointer>()) {
      if (auto* el = Type(t->type())) {
        auto access = t->access();
        if (access == ast::kUndefined) {
          access = DefaultAccessForStorageClass(t->storage_class());
        }
        return builder_->create<sem::Pointer>(const_cast<sem::Type*>(el),
                                              t->storage_class(), access);
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
        if (!ValidateStorageTexture(t)) {
          return nullptr;
        }
        return builder_->create<sem::StorageTexture>(
            t->dim(), t->image_format(), t->access(),
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
        AddError(
            "unknown type '" + builder_->Symbols().NameFor(t->name()) + "'",
            t->source());
        return nullptr;
      }
      return it->second.sem;
    }
    TINT_UNREACHABLE(Resolver, diagnostics_)
        << "Unhandled ast::Type: " << ty->TypeInfo().name;
    return nullptr;
  }();

  if (s) {
    builder_->Sem().Add(ty, s);
  }
  return s;
}

bool Resolver::ValidateAtomic(const ast::Atomic* a, const sem::Atomic* s) {
  // https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
  // T must be either u32 or i32.
  if (!s->Type()->IsAnyOf<sem::U32, sem::I32>()) {
    AddError("atomic only supports i32 or u32 types", a->type()->source());
    return false;
  }
  return true;
}

bool Resolver::ValidateAtomicUses() {
  // https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
  // Atomic types may only be instantiated by variables in the workgroup storage
  // class or by storage buffer variables with a read_write access mode.
  for (auto sm : atomic_members_) {
    auto* structure = sm.structure;
    for (auto usage : structure->StorageClassUsage()) {
      if (usage == ast::StorageClass::kWorkgroup) {
        continue;
      }
      if (usage != ast::StorageClass::kStorage) {
        // TODO(crbug.com/tint/901): Validate that the access mode is
        // read_write.
        auto* member = structure->Members()[sm.index];
        AddError(
            "atomic types can only be used in storage classes workgroup or "
            "storage, but was used by storage class " +
                std::string(ast::str(usage)),
            member->Declaration()->type()->source());
        // TODO(crbug.com/tint/901): Add note pointing at where the usage came
        // from.
        return false;
      }
    }
  }
  return true;
}

bool Resolver::ValidateStorageTexture(const ast::StorageTexture* t) {
  switch (t->access()) {
    case ast::Access::kUndefined:
      AddError("storage textures must have access control", t->source());
      return false;
    case ast::Access::kReadWrite:
      AddError("storage textures only support read-only and write-only access",
               t->source());
      return false;

    case ast::Access::kRead:
    case ast::Access::kWrite:
      break;
  }

  if (!IsValidStorageTextureDimension(t->dim())) {
    AddError("cube dimensions for storage textures are not supported",
             t->source());
    return false;
  }

  if (!IsValidStorageTextureImageFormat(t->image_format())) {
    AddError(
        "image format must be one of the texel formats specified for storage "
        "textues in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats",
        t->source());
    return false;
  }
  return true;
}

Resolver::VariableInfo* Resolver::Variable(ast::Variable* var,
                                           VariableKind kind) {
  if (variable_to_info_.count(var)) {
    TINT_ICE(Resolver, diagnostics_)
        << "Variable " << builder_->Symbols().NameFor(var->symbol())
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
      if (!var->is_const() && kind == VariableKind::kGlobal) {
        AddError("global var declaration must specify a type", var->source());
        return nullptr;
      }

      type_name = rhs_type_name;
      storage_type = rhs_type->UnwrapRef();  // Implicit load of RHS
    }
  } else if (var->is_const() && kind != VariableKind::kParameter &&
             !ast::HasDecoration<ast::OverrideDecoration>(var->decorations())) {
    AddError("let declarations must have initializers", var->source());
    return nullptr;
  }

  if (!storage_type) {
    TINT_ICE(Resolver, diagnostics_)
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

  auto access = var->declared_access();
  if (access == ast::Access::kUndefined) {
    access = DefaultAccessForStorageClass(storage_class);
  }

  auto* type = storage_type;
  if (!var->is_const()) {
    // Variable declaration. Unlike `let`, `var` has storage.
    // Variables are always of a reference type to the declared storage type.
    type =
        builder_->create<sem::Reference>(storage_type, storage_class, access);
  }

  if (rhs_type && !ValidateVariableConstructor(var, storage_type, type_name,
                                               rhs_type, rhs_type_name)) {
    return nullptr;
  }

  auto* info = variable_infos_.Create(var, const_cast<sem::Type*>(type),
                                      type_name, storage_class, access, kind);
  variable_to_info_.emplace(var, info);

  return info;
}

ast::Access Resolver::DefaultAccessForStorageClass(
    ast::StorageClass storage_class) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class
  switch (storage_class) {
    case ast::StorageClass::kStorage:
    case ast::StorageClass::kUniform:
    case ast::StorageClass::kUniformConstant:
      return ast::Access::kRead;
    default:
      break;
  }
  return ast::Access::kReadWrite;
}

bool Resolver::ValidateVariableConstructor(const ast::Variable* var,
                                           const sem::Type* storage_type,
                                           const std::string& type_name,
                                           const sem::Type* rhs_type,
                                           const std::string& rhs_type_name) {
  auto* value_type = rhs_type->UnwrapRef();  // Implicit load of RHS

  // Value type has to match storage type
  if (storage_type != value_type) {
    std::string decl = var->is_const() ? "let" : "var";
    AddError("cannot initialize " + decl + " of type '" + type_name +
                 "' with value of type '" + rhs_type_name + "'",
             var->source());
    return false;
  }
  return true;
}

bool Resolver::GlobalVariable(ast::Variable* var) {
  if (!ValidateNoDuplicateDefinition(var->symbol(), var->source(),
                                     /* check_global_scope_only */ true)) {
    return false;
  }

  auto* info = Variable(var, VariableKind::kGlobal);
  if (!info) {
    return false;
  }
  variable_stack_.set_global(var->symbol(), info);

  if (!var->is_const() && info->storage_class == ast::StorageClass::kNone) {
    AddError("global variables must have a storage class", var->source());
    return false;
  }
  if (var->is_const() && !(info->storage_class == ast::StorageClass::kNone)) {
    AddError("global constants shouldn't have a storage class", var->source());
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

  if (!ValidateNoDuplicateDecorations(var->decorations())) {
    return false;
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
    AddNote("while instantiating variable " +
                builder_->Symbols().NameFor(var->symbol()),
            var->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateGlobalVariable(const VariableInfo* info) {
  if (!ValidateNoDuplicateDecorations(info->declaration->decorations())) {
    return false;
  }

  for (auto* deco : info->declaration->decorations()) {
    if (info->declaration->is_const()) {
      if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
        if (override_deco->HasValue()) {
          uint32_t id = override_deco->value();
          auto itr = constant_ids_.find(id);
          if (itr != constant_ids_.end() && itr->second != info) {
            AddError("pipeline constant IDs must be unique", deco->source());
            AddNote("a pipeline constant with an ID of " + std::to_string(id) +
                        " was previously declared "
                        "here:",
                    ast::GetDecoration<ast::OverrideDecoration>(
                        itr->second->declaration->decorations())
                        ->source());
            return false;
          }
          if (id > 65535) {
            AddError("pipeline constant IDs must be between 0 and 65535",
                     deco->source());
            return false;
          }
        }
      } else {
        AddError("decoration is not valid for constants", deco->source());
        return false;
      }
    } else {
      bool is_shader_io_decoration =
          deco->IsAnyOf<ast::BuiltinDecoration, ast::InterpolateDecoration,
                        ast::InvariantDecoration, ast::LocationDecoration>();
      bool has_io_storage_class =
          info->storage_class == ast::StorageClass::kInput ||
          info->storage_class == ast::StorageClass::kOutput;
      if (!(deco->IsAnyOf<ast::BindingDecoration, ast::GroupDecoration,
                          ast::InternalDecoration>()) &&
          (!is_shader_io_decoration || !has_io_storage_class)) {
        AddError("decoration is not valid for variables", deco->source());
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
        AddError(
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
        AddError(
            "non-resource variables must not have [[group]] or [[binding]] "
            "decorations",
            info->declaration->source());
        return false;
      }
  }

  // https://gpuweb.github.io/gpuweb/wgsl/#variable-declaration
  // The access mode always has a default, and except for variables in the
  // storage storage class, must not be written.
  if (info->storage_class != ast::StorageClass::kStorage &&
      info->declaration->declared_access() != ast::Access::kUndefined) {
    AddError(
        "variables declared not declared in the <storage> storage class must "
        "not declare an access control",
        info->declaration->source());
    return false;
  }

  switch (info->storage_class) {
    case ast::StorageClass::kStorage: {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // A variable in the storage storage class is a storage buffer variable.
      // Its store type must be a host-shareable structure type with block
      // attribute, satisfying the storage class constraints.

      auto* str = info->type->UnwrapRef()->As<sem::Struct>();

      if (!str) {
        AddError(
            "variables declared in the <storage> storage class must be of a "
            "structure type",
            info->declaration->source());
        return false;
      }

      if (!str->IsBlockDecorated()) {
        AddError(
            "structure used as a storage buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source());
        if (info->declaration->source().range.begin.line) {
          AddNote("structure used as storage buffer here",
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
        AddError(
            "variables declared in the <uniform> storage class must be of a "
            "structure type",
            info->declaration->source());
        return false;
      }

      if (!str->IsBlockDecorated()) {
        AddError(
            "structure used as a uniform buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source());
        if (info->declaration->source().range.begin.line) {
          AddNote("structure used as uniform buffer here",
                  info->declaration->source());
        }
        return false;
      }

      for (auto* member : str->Members()) {
        if (auto* arr = member->Type()->As<sem::Array>()) {
          if (arr->IsRuntimeSized()) {
            AddError(
                "structure containing a runtime sized array "
                "cannot be used as a uniform buffer",
                info->declaration->source());
            AddNote("structure is declared here", str->Declaration()->source());
            return false;
          }
        }
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

  if (!var->is_const() && !IsStorable(storage_type)) {
    AddError(storage_type->FriendlyName(builder_->Symbols()) +
                 " cannot be used as the type of a var",
             var->source());
    return false;
  }

  if (auto* r = storage_type->As<sem::Array>()) {
    if (r->IsRuntimeSized()) {
      AddError("runtime arrays may only appear as the last member of a struct",
               var->source());
      return false;
    }
  }

  if (auto* r = storage_type->As<sem::MultisampledTexture>()) {
    if (r->dim() != ast::TextureDimension::k2d) {
      AddError("only 2d multisampled textures are supported", var->source());
      return false;
    }

    if (!r->type()->UnwrapRef()->is_numeric_scalar()) {
      AddError("texture_multisampled_2d<type>: type must be f32, i32 or u32",
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
    AddError("variables of type '" + info->type_name +
                 "' must not have a storage class",
             var->source());
    return false;
  }

  if (IsValidationEnabled(var->decorations(),
                          ast::DisabledValidation::kIgnoreStorageClass) &&
      (var->declared_storage_class() == ast::StorageClass::kInput ||
       var->declared_storage_class() == ast::StorageClass::kOutput)) {
    AddError("invalid use of input/output storage class", var->source());
    return false;
  }

  // https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
  // Atomic types may only be instantiated by variables in the workgroup storage
  // class or by storage buffer variables with a read_write access mode.
  if (info->type->UnwrapRef()->Is<sem::Atomic>() &&
      info->storage_class != ast::StorageClass::kWorkgroup) {
    // Storage buffers require a structure, so just check for workgroup
    // storage here.
    AddError("atomic var requires workgroup storage",
             info->declaration->type()->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateFunctionParameter(const ast::Function* func,
                                         const VariableInfo* info) {
  if (!ValidateVariable(info)) {
    return false;
  }

  for (auto* deco : info->declaration->decorations()) {
    if (!func->IsEntryPoint() && !deco->Is<ast::InternalDecoration>()) {
      AddError(
          "decoration is not valid for non-entry point function parameters",
          deco->source());
      return false;
    } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
      if (!ValidateInterpolateDecoration(interpolate, info->type)) {
        return false;
      }
    } else if (!deco->IsAnyOf<ast::LocationDecoration, ast::BuiltinDecoration,
                              ast::InternalDecoration>() &&
               (IsValidationEnabled(
                    info->declaration->decorations(),
                    ast::DisabledValidation::kEntryPointParameter) &&
                IsValidationEnabled(
                    info->declaration->decorations(),
                    ast::DisabledValidation::
                        kIgnoreConstructibleFunctionParameter))) {
      AddError("decoration is not valid for function parameters",
               deco->source());
      return false;
    }
  }

  if (auto* ref = info->type->As<sem::Pointer>()) {
    auto sc = ref->StorageClass();
    if (!(sc == ast::StorageClass::kFunction ||
          sc == ast::StorageClass::kPrivate ||
          sc == ast::StorageClass::kWorkgroup)) {
      std::stringstream ss;
      ss << "function parameter of pointer type cannot be in '" << sc
         << "' storage class";
      AddError(ss.str(), info->declaration->source());
      return false;
    }
  }

  if (IsPlain(info->type)) {
    if (!IsConstructible(info->type) &&
        IsValidationEnabled(
            info->declaration->decorations(),
            ast::DisabledValidation::kIgnoreConstructibleFunctionParameter)) {
      AddError("store type of function parameter must be a constructible type",
               info->declaration->source());
      return false;
    }
  } else if (!info->type->IsAnyOf<sem::Texture, sem::Sampler, sem::Pointer>()) {
    AddError("store type of function parameter cannot be " +
                 info->type->FriendlyName(builder_->Symbols()),
             info->declaration->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateBuiltinDecoration(const ast::BuiltinDecoration* deco,
                                         const sem::Type* storage_type,
                                         const bool is_input) {
  auto* type = storage_type->UnwrapRef();
  const auto stage = current_function_
                         ? current_function_->declaration->pipeline_stage()
                         : ast::PipelineStage::kNone;
  std::stringstream stage_name;
  stage_name << stage;
  bool is_stage_mismatch = false;
  switch (deco->value()) {
    case ast::Builtin::kPosition:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kFragment && is_input) &&
          !(stage == ast::PipelineStage::kVertex && !is_input)) {
        AddError(deco_to_str(deco) + " cannot be used in " +
                     (is_input ? "input of " : "output of ") +
                     stage_name.str() + " pipeline stage",
                 deco->source());
      }
      if (!(type->is_float_vector() && type->As<sem::Vector>()->size() == 4)) {
        AddError("store type of " + deco_to_str(deco) + " must be 'vec4<f32>'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kGlobalInvocationId:
    case ast::Builtin::kLocalInvocationId:
    case ast::Builtin::kWorkgroupId:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kCompute && is_input)) {
        is_stage_mismatch = true;
      }
      if (!(type->is_unsigned_integer_vector() &&
            type->As<sem::Vector>()->size() == 3)) {
        AddError("store type of " + deco_to_str(deco) + " must be 'vec3<u32>'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kFragDepth:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kFragment && !is_input)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::F32>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'f32'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kFrontFacing:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kFragment && is_input)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::Bool>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'bool'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kLocalInvocationIndex:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kCompute && is_input)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::U32>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'u32'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kVertexIndex:
    case ast::Builtin::kInstanceIndex:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kVertex && is_input)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::U32>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'u32'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kSampleMask:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kFragment)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::U32>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'u32'",
                 deco->source());
        return false;
      }
      break;
    case ast::Builtin::kSampleIndex:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kFragment && is_input)) {
        is_stage_mismatch = true;
      }
      if (!type->Is<sem::U32>()) {
        AddError("store type of " + deco_to_str(deco) + " must be 'u32'",
                 deco->source());
        return false;
      }
      break;
    default:
      break;
  }

  if (is_stage_mismatch) {
    AddError(deco_to_str(deco) + " cannot be used in " +
                 (is_input ? "input of " : "output of ") + stage_name.str() +
                 " pipeline stage",
             deco->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateInterpolateDecoration(
    const ast::InterpolateDecoration* deco,
    const sem::Type* storage_type) {
  auto* type = storage_type->UnwrapRef();

  if (!type->is_float_scalar_or_vector()) {
    AddError(
        "store type of interpolate attribute must be floating point scalar or "
        "vector",
        deco->source());
    return false;
  }

  if (deco->type() == ast::InterpolationType::kFlat &&
      deco->sampling() != ast::InterpolationSampling::kNone) {
    AddError("flat interpolation attribute must not have a sampling parameter",
             deco->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateFunction(const ast::Function* func,
                                const FunctionInfo* info) {
  if (!ValidateNoDuplicateDefinition(func->symbol(), func->source(),
                                     /* check_global_scope_only */ true)) {
    return false;
  }

  auto workgroup_deco_count = 0;
  for (auto* deco : func->decorations()) {
    if (deco->Is<ast::WorkgroupDecoration>()) {
      workgroup_deco_count++;
      if (func->pipeline_stage() != ast::PipelineStage::kCompute) {
        AddError(
            "the workgroup_size attribute is only valid for compute stages",
            deco->source());
        return false;
      }
    } else if (!deco->IsAnyOf<ast::StageDecoration,
                              ast::InternalDecoration>()) {
      AddError("decoration is not valid for functions", deco->source());
      return false;
    }
  }

  for (auto* param : func->params()) {
    if (!ValidateFunctionParameter(func, variable_to_info_.at(param))) {
      return false;
    }
  }

  if (!info->return_type->Is<sem::Void>()) {
    if (!IsConstructible(info->return_type)) {
      AddError("function return type must be a constructible type",
               func->return_type()->source());
      return false;
    }

    if (func->body()) {
      if (!func->get_last_statement() ||
          !func->get_last_statement()->Is<ast::ReturnStatement>()) {
        AddError("non-void function must end with a return statement",
                 func->source());
        return false;
      }
    } else if (IsValidationEnabled(
                   func->decorations(),
                   ast::DisabledValidation::kFunctionHasNoBody)) {
      TINT_ICE(Resolver, diagnostics_)
          << "Function " << builder_->Symbols().NameFor(func->symbol())
          << " has no body";
    }

    for (auto* deco : func->return_type_decorations()) {
      if (!func->IsEntryPoint()) {
        AddError(
            "decoration is not valid for non-entry point function return types",
            deco->source());
        return false;
      }

      if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
        if (!ValidateInterpolateDecoration(interpolate, info->return_type)) {
          return false;
        }
      } else if (!deco->IsAnyOf<ast::LocationDecoration, ast::BuiltinDecoration,
                                ast::InvariantDecoration,
                                ast::InternalDecoration>() &&
                 (IsValidationEnabled(
                      info->declaration->decorations(),
                      ast::DisabledValidation::kEntryPointParameter) &&
                  IsValidationEnabled(
                      info->declaration->decorations(),
                      ast::DisabledValidation::
                          kIgnoreConstructibleFunctionParameter))) {
        AddError("decoration is not valid for entry point return types",
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

  // Inner lambda that is applied to a type and all of its members.
  auto validate_entry_point_decorations_inner = [&](const ast::DecorationList&
                                                        decos,
                                                    sem::Type* ty,
                                                    Source source,
                                                    ParamOrRetType param_or_ret,
                                                    bool is_struct_member) {
    // Scan decorations for pipeline IO attributes.
    // Check for overlap with attributes that have been seen previously.
    ast::Decoration* pipeline_io_attribute = nullptr;
    ast::InvariantDecoration* invariant_attribute = nullptr;
    for (auto* deco : decos) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (pipeline_io_attribute) {
          AddError("multiple entry point IO attributes", deco->source());
          AddNote("previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
          return false;
        }
        pipeline_io_attribute = deco;

        if (builtins.count(builtin->value())) {
          AddError(deco_to_str(builtin) +
                       " attribute appears multiple times as pipeline " +
                       (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                   : "output"),
                   func->source());
          return false;
        }
        builtins.emplace(builtin->value());

        if (!ValidateBuiltinDecoration(
                builtin, ty,
                /* is_input */ param_or_ret == ParamOrRetType::kParameter)) {
          return false;
        }

      } else if (auto* location = deco->As<ast::LocationDecoration>()) {
        if (pipeline_io_attribute) {
          AddError("multiple entry point IO attributes", deco->source());
          AddNote("previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
          return false;
        }
        pipeline_io_attribute = deco;

        if (locations.count(location->value())) {
          AddError(deco_to_str(location) +
                       " attribute appears multiple times as pipeline " +
                       (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                   : "output"),
                   func->source());
          return false;
        }
        locations.emplace(location->value());
      } else if (auto* invariant = deco->As<ast::InvariantDecoration>()) {
        invariant_attribute = invariant;
      }
    }

    // Check that we saw a pipeline IO attribute iff we need one.
    if (ty->Is<sem::Struct>()) {
      if (pipeline_io_attribute) {
        AddError("entry point IO attributes must not be used on structure " +
                     std::string(param_or_ret == ParamOrRetType::kParameter
                                     ? "parameters"
                                     : "return types"),
                 pipeline_io_attribute->source());
        return false;
      }
    } else if (IsValidationEnabled(
                   decos, ast::DisabledValidation::kEntryPointParameter)) {
      if (!pipeline_io_attribute) {
        std::string err = "missing entry point IO attribute";
        if (!is_struct_member) {
          err +=
              (param_or_ret == ParamOrRetType::kParameter ? " on parameter"
                                                          : " on return type");
        }
        AddError(err, source);
        return false;
      }

      auto* builtin = pipeline_io_attribute->As<ast::BuiltinDecoration>();
      if (invariant_attribute &&
          !(builtin && builtin->value() == ast::Builtin::kPosition)) {
        AddError(
            "invariant attribute must only be applied to a position builtin",
            invariant_attribute->source());
        return false;
      }

      // Check that all user defined attributes are numeric scalars, vectors
      // of numeric scalars.
      // Testing for being a struct is handled by the if portion above.
      if (!builtin) {
        if (!ty->is_numeric_scalar_or_vector()) {
          AddError(
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
          AddError("entry point IO types cannot contain nested structures",
                   member->Declaration()->source());
          AddNote("while analysing entry point " +
                      builder_->Symbols().NameFor(func->symbol()),
                  func->source());
          return false;
        }

        if (auto* arr = member->Type()->As<sem::Array>()) {
          if (arr->IsRuntimeSized()) {
            AddError("entry point IO types cannot contain runtime sized arrays",
                     member->Declaration()->source());
            AddNote("while analysing entry point " +
                        builder_->Symbols().NameFor(func->symbol()),
                    func->source());
            return false;
          }
        }

        if (!validate_entry_point_decorations_inner(
                member->Declaration()->decorations(), member->Type(),
                member->Declaration()->source(), param_or_ret, true)) {
          AddNote("while analysing entry point " +
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
    if (!found) {
      AddError(
          "a vertex shader must include the 'position' builtin in its return "
          "type",
          func->source());
      return false;
    }
  }

  if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
    if (!ast::HasDecoration<ast::WorkgroupDecoration>(func->decorations())) {
      AddError(
          "a compute shader must include 'workgroup_size' in its "
          "attributes",
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
        IsValidationEnabled(var_info->declaration->decorations(),
                            ast::DisabledValidation::kBindingPointCollision) &&
        IsValidationEnabled(res.first->second->decorations(),
                            ast::DisabledValidation::kBindingPointCollision)) {
      // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
      // Bindings must not alias within a shader stage: two different
      // variables in the resource interface of a given shader must not have
      // the same group and binding values, when considered as a pair of
      // values.
      auto func_name = builder_->Symbols().NameFor(info->declaration->symbol());
      AddError("entry point '" + func_name +
                   "' references multiple variables that use the "
                   "same resource binding [[group(" +
                   std::to_string(bp.group) + "), binding(" +
                   std::to_string(bp.binding) + ")]]",
               var_info->declaration->source());
      AddNote("first resource binding usage declared here",
              res.first->second->source());
      return false;
    }
  }

  return true;
}

bool Resolver::Function(ast::Function* func) {
  auto* info = function_infos_.Create<FunctionInfo>(func);

  if (func->IsEntryPoint()) {
    entry_points_.emplace_back(info);
  }

  TINT_SCOPED_ASSIGNMENT(current_function_, info);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    Mark(param);
    auto* param_info = Variable(param, VariableKind::kParameter);
    if (!param_info) {
      return false;
    }

    for (auto* deco : param->decorations()) {
      Mark(deco);
    }
    if (!ValidateNoDuplicateDecorations(param->decorations())) {
      return false;
    }

    variable_stack_.set(param->symbol(), param_info);
    info->parameters.emplace_back(param_info);

    if (!ApplyStorageClassUsageToType(param->declared_storage_class(),
                                      param_info->type, param->source())) {
      AddNote("while instantiating parameter " +
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
      AddNote("while instantiating return type for " +
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
    if (current_compound_statement_) {
      TINT_ICE(Resolver, diagnostics_)
          << "Resolver::Function() called with a current compound statement";
      return false;
    }
    auto* sem_block = builder_->create<sem::FunctionBlockStatement>(func);
    builder_->Sem().Add(func->body(), sem_block);
    if (!Scope(sem_block, [&] { return Statements(func->body()->list()); })) {
      return false;
    }
  }
  variable_stack_.pop_scope();

  for (auto* deco : func->decorations()) {
    Mark(deco);
  }
  if (!ValidateNoDuplicateDecorations(func->decorations())) {
    return false;
  }

  for (auto* deco : func->return_type_decorations()) {
    Mark(deco);
  }
  if (!ValidateNoDuplicateDecorations(func->return_type_decorations())) {
    return false;
  }

  // Set work-group size defaults.
  for (int i = 0; i < 3; i++) {
    info->workgroup_size[i].value = 1;
    info->workgroup_size[i].overridable_const = nullptr;
  }

  if (auto* workgroup =
          ast::GetDecoration<ast::WorkgroupDecoration>(func->decorations())) {
    auto values = workgroup->values();
    auto is_i32 = false;
    auto is_less_than_one = true;
    for (int i = 0; i < 3; i++) {
      // Each argument to this decoration can either be a literal, an
      // identifier for a module-scope constants, or nullptr if not specified.

      if (!values[i]) {
        // Not specified, just use the default.
        continue;
      }

      Mark(values[i]);

      uint32_t value = 0;
      if (auto* ident = values[i]->As<ast::IdentifierExpression>()) {
        // We have an identifier of a module-scope constant.
        if (!Identifier(ident)) {
          return false;
        }

        VariableInfo* var;
        if (!variable_stack_.get(ident->symbol(), &var) ||
            !(var->declaration->is_const() && var->type->is_integer_scalar())) {
          AddError(
              "workgroup_size parameter must be either literal or module-scope "
              "constant of type i32 or u32",
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
          auto val = ConstantValueOf(constructor);
          if (!val.IsValid() || !val.Type()->is_integer_scalar()) {
            TINT_ICE(Resolver, diagnostics_)
                << "failed to resolve workgroup_size constant value";
            return false;
          }

          if (i == 0) {
            is_i32 = val.Type()->Is<sem::I32>();
          } else {
            if (is_i32 != val.Type()->Is<sem::I32>()) {
              AddError(
                  "workgroup_size parameters must be of the same type, "
                  "either i32 or u32",
                  values[i]->source());
              return false;
            }
          }
          is_less_than_one =
              is_i32 ? val.Elements()[0].i32 < 1 : val.Elements()[0].u32 < 1;

          value = is_i32 ? static_cast<uint32_t>(val.Elements()[0].i32)
                         : val.Elements()[0].u32;
        } else {
          // No constructor means this value must be overriden by the user.
          info->workgroup_size[i].value = 0;
          continue;
        }
      } else if (auto* scalar =
                     values[i]->As<ast::ScalarConstructorExpression>()) {
        // We have a literal.
        Mark(scalar->literal());
        auto* literal = scalar->literal()->As<ast::IntLiteral>();
        if (!literal) {
          AddError(
              "workgroup_size parameter must be either literal or module-scope "
              "constant of type i32 or u32",
              values[i]->source());
          return false;
        }

        if (i == 0) {
          is_i32 = literal->Is<ast::SintLiteral>();
        } else {
          if (literal->Is<ast::SintLiteral>() != is_i32) {
            AddError(
                "workgroup_size parameters must be of the same type, "
                "either i32 or u32",
                values[i]->source());
            return false;
          }
        }

        is_less_than_one =
            is_i32 ? literal->value_as_i32() < 1 : literal->value_as_u32() < 1;
        value = is_i32 ? static_cast<uint32_t>(literal->value_as_i32())
                       : literal->value_as_u32();
      }

      // Validate and set the default value for this dimension.
      if (is_less_than_one) {
        AddError("workgroup_size parameter must be at least 1",
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
  if (!ValidateStatements(stmts)) {
    return false;
  }

  return true;
}

bool Resolver::ValidateStatements(const ast::StatementList& stmts) {
  auto next_stmt = stmts.begin();
  for (auto* stmt : stmts) {
    next_stmt++;
    if (stmt->IsAnyOf<ast::ReturnStatement, ast::BreakStatement,
                      ast::ContinueStatement>()) {
      if (stmt != stmts.back()) {
        AddError("code is unreachable", (*next_stmt)->source());
        return false;
      }
    }
  }
  return true;
}

bool Resolver::Statement(ast::Statement* stmt) {
  if (stmt->Is<ast::CaseStatement>()) {
    AddError("case statement can only be used inside a switch statement",
             stmt->source());
    return false;
  }
  if (stmt->Is<ast::ElseStatement>()) {
    TINT_ICE(Resolver, diagnostics_)
        << "Resolver::Statement() encountered an Else statement. Else "
           "statements are embedded in If statements, so should never be "
           "encountered as top-level statements";
    return false;
  }

  // Compound statements. These create their own sem::CompoundStatement
  // bindings.
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return BlockStatement(b);
  }
  if (auto* l = stmt->As<ast::ForLoopStatement>()) {
    return ForLoopStatement(l);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return LoopStatement(l);
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return IfStatement(i);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return SwitchStatement(s);
  }

  // Non-Compound statements
  sem::Statement* sem_statement =
      builder_->create<sem::Statement>(stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem_statement);
  TINT_SCOPED_ASSIGNMENT(current_statement_, sem_statement);
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return Assignment(a);
  }
  if (stmt->Is<ast::BreakStatement>()) {
    if (!sem_statement->FindFirstParent<sem::LoopBlockStatement>() &&
        !sem_statement->FindFirstParent<sem::SwitchCaseBlockStatement>()) {
      AddError("break statement must be in a loop or switch case",
               stmt->source());
      return false;
    }
    return true;
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    Mark(c->expr());
    if (!Expression(c->expr())) {
      return false;
    }
    if (!ValidateCallStatement(c)) {
      return false;
    }
    return true;
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
      AddError("continue statement must be in a loop", stmt->source());
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
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return Return(r);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return VariableDeclStatement(v);
  }

  AddError(
      "unknown statement type for type determination: " + builder_->str(stmt),
      stmt->source());
  return false;
}

bool Resolver::CaseStatement(ast::CaseStatement* stmt) {
  auto* sem = builder_->create<sem::SwitchCaseBlockStatement>(
      stmt->body(), current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  builder_->Sem().Add(stmt->body(), sem);
  Mark(stmt->body());
  for (auto* sel : stmt->selectors()) {
    Mark(sel);
  }
  return Scope(sem, [&] { return Statements(stmt->body()->list()); });
}

bool Resolver::IfStatement(ast::IfStatement* stmt) {
  auto* sem =
      builder_->create<sem::IfStatement>(stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    Mark(stmt->condition());
    if (!Expression(stmt->condition())) {
      return false;
    }

    auto* cond_type = TypeOf(stmt->condition())->UnwrapRef();
    if (!cond_type->Is<sem::Bool>()) {
      AddError("if statement condition must be bool, got " +
                   cond_type->FriendlyName(builder_->Symbols()),
               stmt->condition()->source());
      return false;
    }

    Mark(stmt->body());
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body(), current_compound_statement_);
    builder_->Sem().Add(stmt->body(), body);
    if (!Scope(body, [&] { return Statements(stmt->body()->list()); })) {
      return false;
    }

    for (auto* else_stmt : stmt->else_statements()) {
      Mark(else_stmt);
      if (!ElseStatement(else_stmt)) {
        return false;
      }
    }
    return true;
  });
}

bool Resolver::ElseStatement(ast::ElseStatement* stmt) {
  auto* sem =
      builder_->create<sem::ElseStatement>(stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (auto* cond = stmt->condition()) {
      Mark(cond);
      if (!Expression(cond)) {
        return false;
      }

      auto* else_cond_type = TypeOf(cond)->UnwrapRef();
      if (!else_cond_type->Is<sem::Bool>()) {
        AddError("else statement condition must be bool, got " +
                     else_cond_type->FriendlyName(builder_->Symbols()),
                 cond->source());
        return false;
      }
    }

    Mark(stmt->body());
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body(), current_compound_statement_);
    builder_->Sem().Add(stmt->body(), body);
    return Scope(body, [&] { return Statements(stmt->body()->list()); });
  });
}

bool Resolver::BlockStatement(ast::BlockStatement* stmt) {
  auto* sem = builder_->create<sem::BlockStatement>(
      stmt->As<ast::BlockStatement>(), current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] { return Statements(stmt->list()); });
}

bool Resolver::LoopStatement(ast::LoopStatement* stmt) {
  auto* sem =
      builder_->create<sem::LoopStatement>(stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    Mark(stmt->body());

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body(), current_compound_statement_);
    builder_->Sem().Add(stmt->body(), body);
    return Scope(body, [&] {
      if (!Statements(stmt->body()->list())) {
        return false;
      }
      if (stmt->continuing()) {  // has_continuing() also checks for empty()
        Mark(stmt->continuing());
      }
      if (stmt->has_continuing()) {
        auto* continuing = builder_->create<sem::LoopContinuingBlockStatement>(
            stmt->continuing(), current_compound_statement_);
        builder_->Sem().Add(stmt->continuing(), continuing);
        if (!Scope(continuing,
                   [&] { return Statements(stmt->continuing()->list()); })) {
          return false;
        }
      }
      return true;
    });
  });
}

bool Resolver::ForLoopStatement(ast::ForLoopStatement* stmt) {
  auto* sem = builder_->create<sem::ForLoopStatement>(
      stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (auto* initializer = stmt->initializer()) {
      Mark(initializer);
      if (!Statement(initializer)) {
        return false;
      }
    }

    if (auto* condition = stmt->condition()) {
      Mark(condition);
      if (!Expression(condition)) {
        return false;
      }

      if (!TypeOf(condition)->Is<sem::Bool>()) {
        AddError(
            "for-loop condition must be bool, got " + TypeNameOf(condition),
            condition->source());
        return false;
      }
    }

    if (auto* continuing = stmt->continuing()) {
      Mark(continuing);
      if (!Statement(continuing)) {
        return false;
      }
    }

    Mark(stmt->body());

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body(), current_compound_statement_);
    builder_->Sem().Add(stmt->body(), body);
    return Scope(body, [&] { return Statements(stmt->body()->statements()); });
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

  bool ok = false;
  if (auto* array = expr->As<ast::ArrayAccessorExpression>()) {
    ok = ArrayAccessor(array);
  } else if (auto* bin_op = expr->As<ast::BinaryExpression>()) {
    ok = Binary(bin_op);
  } else if (auto* bitcast = expr->As<ast::BitcastExpression>()) {
    ok = Bitcast(bitcast);
  } else if (auto* call = expr->As<ast::CallExpression>()) {
    ok = Call(call);
  } else if (auto* ctor = expr->As<ast::ConstructorExpression>()) {
    ok = Constructor(ctor);
  } else if (auto* ident = expr->As<ast::IdentifierExpression>()) {
    ok = Identifier(ident);
  } else if (auto* member = expr->As<ast::MemberAccessorExpression>()) {
    ok = MemberAccessor(member);
  } else if (auto* unary = expr->As<ast::UnaryOpExpression>()) {
    ok = UnaryOp(unary);
  } else {
    AddError("unknown expression for type determination", expr->source());
  }

  if (!ok) {
    return false;
  }

  auto* ty = TypeOf(expr);
  if (ty->Is<sem::Atomic>()) {
    AddError("an expression must not evaluate to an atomic type",
             expr->source());
    return false;
  }

  return true;
}

bool Resolver::ArrayAccessor(ast::ArrayAccessorExpression* expr) {
  Mark(expr->array());
  if (!Expression(expr->array())) {
    return false;
  }
  auto* idx = expr->idx_expr();
  Mark(idx);
  if (!Expression(idx)) {
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
    AddError("invalid parent type (" + parent_type->type_name() +
                 ") in array accessor",
             expr->source());
    return false;
  }

  if (!TypeOf(idx)->UnwrapRef()->IsAnyOf<sem::I32, sem::U32>()) {
    AddError("index must be of type 'i32' or 'u32', found: '" +
                 TypeNameOf(idx) + "'",
             idx->source());
    return false;
  }

  if (parent_type->Is<sem::Array>() || parent_type->Is<sem::Matrix>()) {
    if (!res->Is<sem::Reference>()) {
      // TODO(bclayton): expand this to allow any const_expr expression
      // https://github.com/gpuweb/gpuweb/issues/1272
      auto* scalar = idx->As<ast::ScalarConstructorExpression>();
      if (!scalar || !scalar->literal()->As<ast::IntLiteral>()) {
        AddError("index must be signed or unsigned integer literal",
                 idx->source());
        return false;
      }
    }
  }

  // If we're extracting from a reference, we return a reference.
  if (auto* ref = res->As<sem::Reference>()) {
    ret = builder_->create<sem::Reference>(ret, ref->StorageClass(),
                                           ref->Access());
  }
  SetExprInfo(expr, ret);

  return true;
}

bool Resolver::Bitcast(ast::BitcastExpression* expr) {
  Mark(expr->expr());
  if (!Expression(expr->expr())) {
    return false;
  }
  auto* ty = Type(expr->type());
  if (!ty) {
    return false;
  }
  if (ty->Is<sem::Pointer>()) {
    AddError("cannot cast to a pointer", expr->source());
    return false;
  }
  SetExprInfo(expr, ty, expr->type()->FriendlyName(builder_->Symbols()));
  return true;
}

bool Resolver::Call(ast::CallExpression* call) {
  if (!Expressions(call->params())) {
    return false;
  }

  Mark(call->func());
  auto* ident = call->func();
  auto name = builder_->Symbols().NameFor(ident->symbol());

  auto intrinsic_type = sem::ParseIntrinsicType(name);
  if (intrinsic_type != IntrinsicType::kNone) {
    if (!IntrinsicCall(call, intrinsic_type)) {
      return false;
    }
  } else {
    if (!FunctionCall(call)) {
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateCallStatement(ast::CallStatement* stmt) {
  const sem::Type* return_type = nullptr;
  // A function call is made to either a user declared function or an intrinsic.
  // function_calls_ only maps CallExpression to user declared functions
  auto it = function_calls_.find(stmt->expr());
  if (it != function_calls_.end()) {
    return_type = it->second.function->return_type;
  } else {
    // Must be an intrinsic call
    auto* target = builder_->Sem().Get(stmt->expr())->Target();
    if (auto* intrinsic = target->As<sem::Intrinsic>()) {
      return_type = intrinsic->ReturnType();
    } else {
      TINT_ICE(Resolver, diagnostics_)
          << "call target was not an intrinsic, but a "
          << intrinsic->TypeInfo().name;
    }
  }

  if (!return_type->Is<sem::Void>()) {
    // https://gpuweb.github.io/gpuweb/wgsl/#function-call-statement
    // A function call statement executes a function call where the called
    // function does not return a value. If the called function returns a value,
    // that value must be consumed either through assignment, evaluation in
    // another expression or through use of the ignore built-in function (see
    // § 16.13 Value-steering functions).
    AddError(
        "result of called function was not used. If this was intentional wrap "
        "the function call in ignore()",
        stmt->source());
    return false;
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

  auto* result =
      intrinsic_table_->Lookup(intrinsic_type, arg_tys, call->source());
  if (!result) {
    return false;
  }

  if (result->IsDeprecated()) {
    AddWarning("use of deprecated intrinsic", call->source());
  }

  builder_->Sem().Add(
      call, builder_->create<sem::Call>(call, result, current_statement_));
  SetExprInfo(call, result->ReturnType());

  current_function_->intrinsic_calls.emplace_back(
      IntrinsicCallInfo{call, result});

  return true;
}

bool Resolver::FunctionCall(const ast::CallExpression* call) {
  auto* ident = call->func();
  auto name = builder_->Symbols().NameFor(ident->symbol());

  auto callee_func_it = symbol_to_function_.find(ident->symbol());
  if (callee_func_it == symbol_to_function_.end()) {
    if (current_function_ &&
        current_function_->declaration->symbol() == ident->symbol()) {
      AddError("recursion is not permitted. '" + name +
                   "' attempted to call itself.",
               call->source());
    } else {
      AddError("unable to find called function: " + name, call->source());
    }
    return false;
  }
  auto* callee_func = callee_func_it->second;

  if (current_function_) {
    callee_func->callsites.push_back(call);

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

  // Validate number of arguments match number of parameters
  if (call->params().size() != callee_func->parameters.size()) {
    bool more = call->params().size() > callee_func->parameters.size();
    AddError("too " + (more ? std::string("many") : std::string("few")) +
                 " arguments in call to '" + name + "', expected " +
                 std::to_string(callee_func->parameters.size()) + ", got " +
                 std::to_string(call->params().size()),
             call->source());
    return false;
  }

  // Validate arguments match parameter types
  for (size_t i = 0; i < call->params().size(); ++i) {
    const VariableInfo* param = callee_func->parameters[i];
    const ast::Expression* arg_expr = call->params()[i];
    auto* arg_type = TypeOf(arg_expr)->UnwrapRef();

    if (param->type != arg_type) {
      AddError("type mismatch for argument " + std::to_string(i + 1) +
                   " in call to '" + name + "', expected '" +
                   param->type->FriendlyName(builder_->Symbols()) + "', got '" +
                   arg_type->FriendlyName(builder_->Symbols()) + "'",
               arg_expr->source());
      return false;
    }
  }

  function_calls_.emplace(call,
                          FunctionCallInfo{callee_func, current_statement_});
  SetExprInfo(call, callee_func->return_type, callee_func->return_type_name);

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

    auto* type = Type(type_ctor->type());
    if (!type) {
      return false;
    }

    auto type_name = type_ctor->type()->FriendlyName(builder_->Symbols());

    // Now that the argument types have been determined, make sure that they
    // obey the constructor type rules laid out in
    // https://gpuweb.github.io/gpuweb/wgsl.html#type-constructor-expr.
    if (type->Is<sem::Pointer>()) {
      AddError("cannot cast to a pointer", expr->source());
      return false;
    }

    bool ok = true;
    if (auto* vec_type = type->As<sem::Vector>()) {
      ok = ValidateVectorConstructor(type_ctor, vec_type, type_name);
    } else if (auto* mat_type = type->As<sem::Matrix>()) {
      ok = ValidateMatrixConstructor(type_ctor, mat_type, type_name);
    } else if (type->is_scalar()) {
      ok = ValidateScalarConstructor(type_ctor, type, type_name);
    } else if (auto* arr_type = type->As<sem::Array>()) {
      ok = ValidateArrayConstructor(type_ctor, arr_type);
    } else if (auto* struct_type = type->As<sem::Struct>()) {
      ok = ValidateStructureConstructor(type_ctor, struct_type);
    }
    if (!ok) {
      return false;
    }
    SetExprInfo(expr, type, type_name);
    return true;
  }

  if (auto* scalar_ctor = expr->As<ast::ScalarConstructorExpression>()) {
    Mark(scalar_ctor->literal());
    auto* type = TypeOf(scalar_ctor->literal());
    if (!type) {
      return false;
    }
    SetExprInfo(expr, type);
    return true;
  }

  TINT_ICE(Resolver, diagnostics_) << "unexpected constructor expression type";
  return false;
}

bool Resolver::ValidateStructureConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Struct* struct_type) {
  if (ctor->values().size() > 0) {
    if (ctor->values().size() != struct_type->Members().size()) {
      std::string fm = ctor->values().size() < struct_type->Members().size()
                           ? "few"
                           : "many";
      AddError("struct constructor has too " + fm + " inputs: expected " +
                   std::to_string(struct_type->Members().size()) + ", found " +
                   std::to_string(ctor->values().size()),
               ctor->source());
      return false;
    }
    for (auto* member : struct_type->Members()) {
      auto* value = ctor->values()[member->Index()];
      if (member->Type() != TypeOf(value)->UnwrapRef()) {
        AddError(
            "type in struct constructor does not match struct member type: "
            "expected '" +
                member->Type()->FriendlyName(builder_->Symbols()) +
                "', found '" + TypeNameOf(value) + "'",
            value->source());
        return false;
      }
    }
  }
  return true;
}

bool Resolver::ValidateArrayConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Array* array_type) {
  auto& values = ctor->values();
  auto* elem_type = array_type->ElemType();
  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapRef();
    if (value_type != elem_type) {
      AddError(
          "type in array constructor does not match array type: "
          "expected '" +
              elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
              TypeNameOf(value) + "'",
          value->source());
      return false;
    }
  }

  if (array_type->IsRuntimeSized()) {
    AddError("cannot init a runtime-sized array", ctor->source());
    return false;
  } else if (!values.empty() && (values.size() != array_type->Count())) {
    std::string fm = values.size() < array_type->Count() ? "few" : "many";
    AddError("array constructor has too " + fm + " elements: expected " +
                 std::to_string(array_type->Count()) + ", found " +
                 std::to_string(values.size()),
             ctor->source());
    return false;
  } else if (values.size() > array_type->Count()) {
    AddError("array constructor has too many elements: expected " +
                 std::to_string(array_type->Count()) + ", found " +
                 std::to_string(values.size()),
             ctor->source());
    return false;
  }
  return true;
}

bool Resolver::ValidateVectorConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Vector* vec_type,
    const std::string& type_name) {
  auto& values = ctor->values();
  auto* elem_type = vec_type->type();
  size_t value_cardinality_sum = 0;
  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapRef();
    if (value_type->is_scalar()) {
      if (elem_type != value_type) {
        AddError(
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
        AddError(
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
      AddError("expected vector or scalar type in vector constructor; found: " +
                   value_type->FriendlyName(builder_->Symbols()),
               value->source());
      return false;
    }
  }

  // A correct vector constructor must either be a zero-value expression,
  // a single-value initializer (splat) expression, or the number of components
  // of all constructor arguments must add up to the vector cardinality.
  if (value_cardinality_sum > 1 && value_cardinality_sum != vec_type->size()) {
    if (values.empty()) {
      TINT_ICE(Resolver, diagnostics_)
          << "constructor arguments expected to be non-empty!";
    }
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    AddError("attempted to construct '" + type_name + "' with " +
                 std::to_string(value_cardinality_sum) + " component(s)",
             Source::Combine(values_start, values_end));
    return false;
  }
  return true;
}

bool Resolver::ValidateVector(const sem::Vector* ty, const Source& source) {
  if (!ty->type()->is_scalar()) {
    AddError("vector element type must be 'bool', 'f32', 'i32' or 'u32'",
             source);
    return false;
  }
  return true;
}

bool Resolver::ValidateMatrix(const sem::Matrix* ty, const Source& source) {
  if (!ty->is_float_matrix()) {
    AddError("matrix element type must be 'f32'", source);
    return false;
  }
  return true;
}

bool Resolver::ValidateMatrixConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Matrix* matrix_type,
    const std::string& type_name) {
  auto& values = ctor->values();
  // Zero Value expression
  if (values.empty()) {
    return true;
  }

  if (!ValidateMatrix(matrix_type, ctor->source())) {
    return false;
  }

  auto* elem_type = matrix_type->type();
  if (matrix_type->columns() != values.size()) {
    const Source& values_start = values[0]->source();
    const Source& values_end = values[values.size() - 1]->source();
    AddError("expected " + std::to_string(matrix_type->columns()) + " '" +
                 VectorPretty(matrix_type->rows(), elem_type) +
                 "' arguments in '" + type_name + "' constructor, found " +
                 std::to_string(values.size()),
             Source::Combine(values_start, values_end));
    return false;
  }

  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapRef();
    auto* value_vec = value_type->As<sem::Vector>();

    if (!value_vec || value_vec->size() != matrix_type->rows() ||
        elem_type != value_vec->type()) {
      AddError("expected argument type '" +
                   VectorPretty(matrix_type->rows(), elem_type) + "' in '" +
                   type_name + "' constructor, found '" + TypeNameOf(value) +
                   "'",
               value->source());
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateScalarConstructor(
    const ast::TypeConstructorExpression* ctor,
    const sem::Type* type,
    const std::string& type_name) {
  if (ctor->values().size() == 0) {
    return true;
  }
  if (ctor->values().size() > 1) {
    AddError("expected zero or one value in constructor, got " +
                 std::to_string(ctor->values().size()),
             ctor->source());
    return false;
  }

  // Validate constructor
  auto* value = ctor->values()[0];
  auto* value_type = TypeOf(value)->UnwrapRef();

  using Bool = sem::Bool;
  using I32 = sem::I32;
  using U32 = sem::U32;
  using F32 = sem::F32;

  const bool is_valid =
      (type->Is<Bool>() && value_type->IsAnyOf<Bool, I32, U32, F32>()) ||
      (type->Is<I32>() && value_type->IsAnyOf<I32, U32, F32>()) ||
      (type->Is<U32>() && value_type->IsAnyOf<I32, U32, F32>()) ||
      (type->Is<F32>() && value_type->IsAnyOf<I32, U32, F32>());
  if (!is_valid) {
    AddError("cannot construct '" + type_name + "' with a value of type '" +
                 TypeNameOf(value) + "'",
             ctor->source());

    return false;
  }

  return true;
}

bool Resolver::Identifier(ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol();
  VariableInfo* var;
  if (variable_stack_.get(symbol, &var)) {
    SetExprInfo(expr, var->type, var->type_name);

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
              AddError("continue statement bypasses declaration of '" +
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
    AddError("missing '(' for function call", expr->source().End());
    return false;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (sem::ParseIntrinsicType(name) != IntrinsicType::kNone) {
    AddError("missing '(' for intrinsic call", expr->source().End());
    return false;
  }

  AddError("identifier must be declared before use: " + name, expr->source());
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
      AddError(
          "struct member " + builder_->Symbols().NameFor(symbol) + " not found",
          expr->source());
      return false;
    }

    // If we're extracting from a reference, we return a reference.
    if (auto* ref = structure->As<sem::Reference>()) {
      ret = builder_->create<sem::Reference>(ret, ref->StorageClass(),
                                             ref->Access());
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
          AddError("invalid vector swizzle character",
                   expr->member()->source().Begin() + swizzle.size());
          return false;
      }

      if (swizzle.back() >= vec->size()) {
        AddError("invalid vector swizzle member", expr->member()->source());
        return false;
      }
    }

    if (size < 1 || size > 4) {
      AddError("invalid vector swizzle size", expr->member()->source());
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
      AddError("invalid mixing of vector swizzle characters rgba with xyzw",
               expr->member()->source());
      return false;
    }

    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a reference, we return a reference.
      if (auto* ref = structure->As<sem::Reference>()) {
        ret = builder_->create<sem::Reference>(ret, ref->StorageClass(),
                                               ref->Access());
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
    AddError("invalid use of member accessor on a non-vector/non-struct " +
                 TypeNameOf(expr->structure()),
             expr->source());
    return false;
  }

  SetExprInfo(expr, ret);

  return true;
}

bool Resolver::Binary(ast::BinaryExpression* expr) {
  Mark(expr->lhs());
  Mark(expr->rhs());

  if (!Expression(expr->lhs()) || !Expression(expr->rhs())) {
    return false;
  }

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
      SetExprInfo(expr, lhs_type);
      return true;
    }
  }
  if (expr->IsOr() || expr->IsAnd()) {
    if (matching_types && lhs_type->Is<Bool>()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }
    if (matching_types && lhs_vec_elem_type && lhs_vec_elem_type->Is<Bool>()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }
  }

  // Arithmetic expressions
  if (expr->IsArithmetic()) {
    // Binary arithmetic expressions over scalars
    if (matching_types && lhs_type->is_numeric_scalar()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }

    // Binary arithmetic expressions over vectors
    if (matching_types && lhs_vec_elem_type &&
        lhs_vec_elem_type->is_numeric_scalar()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }

    // Binary arithmetic expressions with mixed scalar and vector operands
    if (lhs_vec_elem_type && (lhs_vec_elem_type == rhs_type)) {
      if (expr->IsModulo()) {
        if (rhs_type->is_integer_scalar()) {
          SetExprInfo(expr, lhs_type);
          return true;
        }
      } else if (rhs_type->is_numeric_scalar()) {
        SetExprInfo(expr, lhs_type);
        return true;
      }
    }
    if (rhs_vec_elem_type && (rhs_vec_elem_type == lhs_type)) {
      if (expr->IsModulo()) {
        if (lhs_type->is_integer_scalar()) {
          SetExprInfo(expr, rhs_type);
          return true;
        }
      } else if (lhs_type->is_numeric_scalar()) {
        SetExprInfo(expr, rhs_type);
        return true;
      }
    }
  }

  // Matrix arithmetic
  auto* lhs_mat = lhs_type->As<Matrix>();
  auto* lhs_mat_elem_type = lhs_mat ? lhs_mat->type() : nullptr;
  auto* rhs_mat = rhs_type->As<Matrix>();
  auto* rhs_mat_elem_type = rhs_mat ? rhs_mat->type() : nullptr;
  // Addition and subtraction of float matrices
  if ((expr->IsAdd() || expr->IsSubtract()) && lhs_mat_elem_type &&
      lhs_mat_elem_type->Is<F32>() && rhs_mat_elem_type &&
      rhs_mat_elem_type->Is<F32>() &&
      (lhs_mat->columns() == rhs_mat->columns()) &&
      (lhs_mat->rows() == rhs_mat->rows())) {
    SetExprInfo(expr, rhs_type);
    return true;
  }
  if (expr->IsMultiply()) {
    // Multiplication of a matrix and a scalar
    if (lhs_type->Is<F32>() && rhs_mat_elem_type &&
        rhs_mat_elem_type->Is<F32>()) {
      SetExprInfo(expr, rhs_type);
      return true;
    }
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_type->Is<F32>()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }

    // Vector times matrix
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_vec->size() == rhs_mat->rows())) {
      SetExprInfo(expr, builder_->create<sem::Vector>(lhs_vec->type(),
                                                      rhs_mat->columns()));
      return true;
    }

    // Matrix times vector
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_vec->size())) {
      SetExprInfo(expr, builder_->create<sem::Vector>(rhs_vec->type(),
                                                      lhs_mat->rows()));
      return true;
    }

    // Matrix times matrix
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_mat->rows())) {
      SetExprInfo(expr, builder_->create<sem::Matrix>(
                            builder_->create<sem::Vector>(lhs_mat_elem_type,
                                                          lhs_mat->rows()),
                            rhs_mat->columns()));
      return true;
    }
  }

  // Comparison expressions
  if (expr->IsComparison()) {
    if (matching_types) {
      // Special case for bools: only == and !=
      if (lhs_type->Is<Bool>() && (expr->IsEqual() || expr->IsNotEqual())) {
        SetExprInfo(expr, builder_->create<sem::Bool>());
        return true;
      }

      // For the rest, we can compare i32, u32, and f32
      if (lhs_type->IsAnyOf<I32, U32, F32>()) {
        SetExprInfo(expr, builder_->create<sem::Bool>());
        return true;
      }
    }

    // Same for vectors
    if (matching_vec_elem_types) {
      if (lhs_vec_elem_type->Is<Bool>() &&
          (expr->IsEqual() || expr->IsNotEqual())) {
        SetExprInfo(expr, builder_->create<sem::Vector>(
                              builder_->create<sem::Bool>(), lhs_vec->size()));
        return true;
      }

      if (lhs_vec_elem_type->is_numeric_scalar()) {
        SetExprInfo(expr, builder_->create<sem::Vector>(
                              builder_->create<sem::Bool>(), lhs_vec->size()));
        return true;
      }
    }
  }

  // Binary bitwise operations
  if (expr->IsBitwise()) {
    if (matching_types && lhs_type->is_integer_scalar_or_vector()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }
  }

  // Bit shift expressions
  if (expr->IsBitshift()) {
    // Type validation rules are the same for left or right shift, despite
    // differences in computation rules (i.e. right shift can be arithmetic or
    // logical depending on lhs type).

    if (lhs_type->IsAnyOf<I32, U32>() && rhs_type->Is<U32>()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }

    if (lhs_vec_elem_type && lhs_vec_elem_type->IsAnyOf<I32, U32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<U32>()) {
      SetExprInfo(expr, lhs_type);
      return true;
    }
  }

  AddError("Binary expression operand types are invalid for this operation: " +
               lhs_type->FriendlyName(builder_->Symbols()) + " " +
               FriendlyName(expr->op()) + " " +
               rhs_type->FriendlyName(builder_->Symbols()),
           expr->source());
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
    case ast::UnaryOp::kNot:
      // Result type matches the deref'd inner type.
      type_name = TypeNameOf(unary->expr());
      type = expr_type->UnwrapRef();
      if (!type->Is<sem::Bool>() && !type->is_bool_vector()) {
        AddError("cannot logical negate expression of type '" +
                     TypeNameOf(unary->expr()),
                 unary->expr()->source());
        return false;
      }
      break;

    case ast::UnaryOp::kComplement:
      // Result type matches the deref'd inner type.
      type_name = TypeNameOf(unary->expr());
      type = expr_type->UnwrapRef();
      if (!type->is_integer_scalar_or_vector()) {
        AddError("cannot bitwise complement expression of type '" +
                     TypeNameOf(unary->expr()),
                 unary->expr()->source());
        return false;
      }
      break;

    case ast::UnaryOp::kNegation:
      // Result type matches the deref'd inner type.
      type_name = TypeNameOf(unary->expr());
      type = expr_type->UnwrapRef();
      if (!(type->IsAnyOf<sem::F32, sem::I32>() ||
            type->is_signed_integer_vector() || type->is_float_vector())) {
        AddError(
            "cannot negate expression of type '" + TypeNameOf(unary->expr()),
            unary->expr()->source());
        return false;
      }
      break;

    case ast::UnaryOp::kAddressOf:
      if (auto* ref = expr_type->As<sem::Reference>()) {
        type = builder_->create<sem::Pointer>(
            ref->StoreType(), ref->StorageClass(), ref->Access());
      } else {
        AddError("cannot take the address of expression",
                 unary->expr()->source());
        return false;
      }
      break;

    case ast::UnaryOp::kIndirection:
      if (auto* ptr = expr_type->As<sem::Pointer>()) {
        type = builder_->create<sem::Reference>(
            ptr->StoreType(), ptr->StorageClass(), ptr->Access());
      } else {
        AddError("cannot dereference expression of type '" +
                     TypeNameOf(unary->expr()) + "'",
                 unary->expr()->source());
        return false;
      }
      break;
  }

  SetExprInfo(unary, type);
  return true;
}

bool Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
  ast::Variable* var = stmt->variable();
  Mark(var);

  if (!ValidateNoDuplicateDefinition(var->symbol(), var->source())) {
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
  if (current_block_) {  // Not all statements are inside a block
    current_block_->AddDecl(var);
  }

  if (!ValidateVariable(info)) {
    return false;
  }

  if (!var->is_const()) {
    if (info->storage_class != ast::StorageClass::kFunction &&
        IsValidationEnabled(var->decorations(),
                            ast::DisabledValidation::kIgnoreStorageClass)) {
      if (info->storage_class != ast::StorageClass::kNone) {
        AddError("function variable has a non-function storage class",
                 stmt->source());
        return false;
      }
      info->storage_class = ast::StorageClass::kFunction;
    }
  }

  if (!ApplyStorageClassUsageToType(info->storage_class, info->type,
                                    var->source())) {
    AddNote("while instantiating variable " +
                builder_->Symbols().NameFor(var->symbol()),
            var->source());
    return false;
  }

  return true;
}

sem::Type* Resolver::TypeDecl(const ast::TypeDecl* named_type) {
  sem::Type* result = nullptr;
  if (auto* alias = named_type->As<ast::Alias>()) {
    result = Type(alias->type());
  } else if (auto* str = named_type->As<ast::Struct>()) {
    result = Structure(str);
  } else {
    TINT_UNREACHABLE(Resolver, diagnostics_) << "Unhandled TypeDecl";
  }

  if (!result) {
    return nullptr;
  }

  named_type_info_.emplace(named_type->name(),
                           TypeDeclInfo{named_type, result});

  if (!ValidateTypeDecl(named_type)) {
    return nullptr;
  }

  builder_->Sem().Add(named_type, result);
  return result;
}

bool Resolver::ValidateTypeDecl(const ast::TypeDecl* named_type) const {
  auto iter = named_type_info_.find(named_type->name());
  if (iter == named_type_info_.end()) {
    TINT_ICE(Resolver, diagnostics_)
        << "ValidateTypeDecl called() before TypeDecl()";
  }
  if (iter->second.ast != named_type) {
    AddError("type with the name '" +
                 builder_->Symbols().NameFor(named_type->name()) +
                 "' was already declared",
             named_type->source());
    AddNote("first declared here", iter->second.ast->source());
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
  TINT_UNREACHABLE(Resolver, diagnostics_)
      << "Unhandled literal type: " << lit->TypeInfo().name;
  return nullptr;
}

void Resolver::SetExprInfo(const ast::Expression* expr,
                           const sem::Type* type,
                           std::string type_name) {
  if (expr_info_.count(expr)) {
    TINT_ICE(Resolver, diagnostics_)
        << "SetExprInfo() called twice for the same expression";
  }
  if (type_name.empty()) {
    type_name = type->FriendlyName(builder_->Symbols());
  }
  auto constant_value = EvaluateConstantValue(expr, type);
  expr_info_.emplace(
      expr, ExpressionInfo{type, std::move(type_name), current_statement_,
                           std::move(constant_value)});
}

bool Resolver::ValidatePipelineStages() {
  auto check_workgroup_storage = [&](FunctionInfo* func,
                                     FunctionInfo* entry_point) {
    auto stage = entry_point->declaration->pipeline_stage();
    if (stage != ast::PipelineStage::kCompute) {
      for (auto* var : func->local_referenced_module_vars) {
        if (var->storage_class == ast::StorageClass::kWorkgroup) {
          std::stringstream stage_name;
          stage_name << stage;
          for (auto* user : var->users) {
            auto it = expr_info_.find(user->As<ast::Expression>());
            if (it != expr_info_.end()) {
              if (func->declaration->symbol() ==
                  it->second.statement->Function()->symbol()) {
                AddError("workgroup memory cannot be used by " +
                             stage_name.str() + " pipeline stage",
                         user->source());
                break;
              }
            }
          }
          AddNote("variable is declared here", var->declaration->source());
          if (func != entry_point) {
            TraverseCallChain(entry_point, func, [&](FunctionInfo* f) {
              AddNote(
                  "called by function '" +
                      builder_->Symbols().NameFor(f->declaration->symbol()) +
                      "'",
                  f->declaration->source());
            });
            AddNote("called by entry point '" +
                        builder_->Symbols().NameFor(
                            entry_point->declaration->symbol()) +
                        "'",
                    entry_point->declaration->source());
          }
          return false;
        }
      }
    }
    return true;
  };

  for (auto* entry_point : entry_points_) {
    if (!check_workgroup_storage(entry_point, entry_point)) {
      return false;
    }
    for (auto* func : entry_point->transitive_calls) {
      if (!check_workgroup_storage(func, entry_point)) {
        return false;
      }
    }
  }

  auto check_intrinsic_calls = [&](FunctionInfo* func,
                                   FunctionInfo* entry_point) {
    auto stage = entry_point->declaration->pipeline_stage();
    for (auto& call : func->intrinsic_calls) {
      if (!call.intrinsic->SupportedStages().Contains(stage)) {
        std::stringstream err;
        err << "built-in cannot be used by " << stage << " pipeline stage";
        AddError(err.str(), call.call->source());
        if (func != entry_point) {
          TraverseCallChain(entry_point, func, [&](FunctionInfo* f) {
            AddNote("called by function '" +
                        builder_->Symbols().NameFor(f->declaration->symbol()) +
                        "'",
                    f->declaration->source());
          });
          AddNote("called by entry point '" +
                      builder_->Symbols().NameFor(
                          entry_point->declaration->symbol()) +
                      "'",
                  entry_point->declaration->source());
        }
        return false;
      }
    }
    return true;
  };

  for (auto* entry_point : entry_points_) {
    if (!check_intrinsic_calls(entry_point, entry_point)) {
      return false;
    }
    for (auto* func : entry_point->transitive_calls) {
      if (!check_intrinsic_calls(func, entry_point)) {
        return false;
      }
    }
  }
  return true;
}

template <typename CALLBACK>
void Resolver::TraverseCallChain(FunctionInfo* from,
                                 FunctionInfo* to,
                                 CALLBACK&& callback) const {
  for (auto* f : from->transitive_calls) {
    if (f == to) {
      callback(f);
      return;
    }
    if (f->transitive_calls.contains(to)) {
      TraverseCallChain(f, to, callback);
      callback(f);
      return;
    }
  }
  TINT_ICE(Resolver, diagnostics_)
      << "TraverseCallChain() 'from' does not transitively call 'to'";
}

void Resolver::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  // Collate all the 'ancestor_entry_points' - this is a map of function
  // symbol to all the entry points that transitively call the function.
  std::unordered_map<Symbol, std::vector<Symbol>> ancestor_entry_points;
  for (auto* entry_point : entry_points_) {
    for (auto* call : entry_point->transitive_calls) {
      auto& vec = ancestor_entry_points[call->declaration->symbol()];
      vec.emplace_back(entry_point->declaration->symbol());
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
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "no more pipeline constant IDs available";
            return;
          }
          constant_id++;
        }
        next_constant_id = constant_id + 1;
      }

      sem_var = builder_->create<sem::Variable>(var, info->type, constant_id);
    } else {
      sem_var =
          builder_->create<sem::Variable>(var, info->type, info->storage_class,
                                          info->access, info->binding_point);
    }

    std::vector<const sem::VariableUser*> users;
    for (auto* user : info->users) {
      // Create semantic node for the identifier expression if necessary
      auto* sem_expr = sem.Get(user);
      if (sem_expr == nullptr) {
        auto& expr_info = expr_info_.at(user);
        auto* type = expr_info.type;
        auto* stmt = expr_info.statement;
        auto* sem_user = builder_->create<sem::VariableUser>(
            user, type, stmt, sem_var, expr_info.constant_value);
        sem_var->AddUser(sem_user);
        sem.Add(user, sem_user);
      } else {
        auto* sem_user = sem_expr->As<sem::VariableUser>();
        if (!sem_user) {
          TINT_ICE(Resolver, diagnostics_) << "expected sem::VariableUser, got "
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
        info->callsites, ancestor_entry_points[func->symbol()],
        info->workgroup_size);
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
    sem.Add(expr, builder_->create<sem::Expression>(
                      const_cast<ast::Expression*>(expr), info.type,
                      info.statement, info.constant_value));
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
  }
  if (auto* vec = ty->As<sem::Vector>()) {
    if (vec->size() < 2 || vec->size() > 4) {
      TINT_UNREACHABLE(Resolver, diagnostics_)
          << "Invalid vector size: vec" << vec->size();
      return false;
    }
    align = vector_align[vec->size()];
    size = vector_size[vec->size()];
    return true;
  }
  if (auto* mat = ty->As<sem::Matrix>()) {
    if (mat->columns() < 2 || mat->columns() > 4 || mat->rows() < 2 ||
        mat->rows() > 4) {
      TINT_UNREACHABLE(Resolver, diagnostics_)
          << "Invalid matrix size: mat" << mat->columns() << "x" << mat->rows();
      return false;
    }
    align = vector_align[mat->rows()];
    size = vector_align[mat->rows()] * mat->columns();
    return true;
  }
  if (auto* s = ty->As<sem::Struct>()) {
    align = s->Align();
    size = s->Size();
    return true;
  }
  if (auto* a = ty->As<sem::Array>()) {
    align = a->Align();
    size = a->SizeInBytes();
    return true;
  }
  if (auto* a = ty->As<sem::Atomic>()) {
    return DefaultAlignAndSize(a->Type(), align, size);
  }
  TINT_UNREACHABLE(Resolver, diagnostics_)
      << "invalid type " << ty->TypeInfo().name;
  return false;
}

sem::Array* Resolver::Array(const ast::Array* arr) {
  auto source = arr->source();

  auto* el_ty = Type(arr->type());
  if (!el_ty) {
    return nullptr;
  }

  if (!IsPlain(el_ty)) {  // Check must come before DefaultAlignAndSize()
    AddError(el_ty->FriendlyName(builder_->Symbols()) +
                 " cannot be used as an element type of an array",
             source);
    return nullptr;
  }

  uint32_t el_align = 0;
  uint32_t el_size = 0;
  if (!DefaultAlignAndSize(el_ty, el_align, el_size)) {
    return nullptr;
  }

  if (!ValidateNoDuplicateDecorations(arr->decorations())) {
    return nullptr;
  }

  // Look for explicit stride via [[stride(n)]] decoration
  uint32_t explicit_stride = 0;
  for (auto* deco : arr->decorations()) {
    Mark(deco);
    if (auto* sd = deco->As<ast::StrideDecoration>()) {
      explicit_stride = sd->stride();
      if (!ValidateArrayStrideDecoration(sd, el_size, el_align, source)) {
        return nullptr;
      }
      continue;
    }

    AddError("decoration is not valid for array types", deco->source());
    return nullptr;
  }

  // Calculate implicit stride
  auto implicit_stride = utils::RoundUp(el_align, el_size);

  auto stride = explicit_stride ? explicit_stride : implicit_stride;

  // WebGPU requires runtime arrays have at least one element, but the AST
  // records an element count of 0 for it.
  auto size = std::max<uint32_t>(arr->size(), 1) * stride;
  auto* sem = builder_->create<sem::Array>(el_ty, arr->size(), el_align, size,
                                           stride, implicit_stride);

  if (!ValidateArray(sem, source)) {
    return nullptr;
  }

  return sem;
}

bool Resolver::ValidateArray(const sem::Array* arr, const Source& source) {
  auto* el_ty = arr->ElemType();

  if (auto* el_str = el_ty->As<sem::Struct>()) {
    if (el_str->IsBlockDecorated()) {
      // https://gpuweb.github.io/gpuweb/wgsl/#attributes
      // A structure type with the block attribute must not be:
      // * the element type of an array type
      // * the member type in another structure
      AddError(
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
    AddError(
        "arrays decorated with the stride attribute must have a stride "
        "that is at least the size of the element type, and be a multiple "
        "of the element type's alignment value.",
        source);
    return false;
  }
  return true;
}

bool Resolver::ValidateStructure(const sem::Struct* str) {
  if (str->Members().empty()) {
    AddError("structures must have at least one member",
             str->Declaration()->source());
    return false;
  }

  for (auto* member : str->Members()) {
    if (auto* r = member->Type()->As<sem::Array>()) {
      if (r->IsRuntimeSized()) {
        if (member != str->Members().back()) {
          AddError(
              "runtime arrays may only appear as the last member of a struct",
              member->Declaration()->source());
          return false;
        }
        if (!str->IsBlockDecorated()) {
          AddError(
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
            deco->Is<ast::InterpolateDecoration>() ||
            deco->Is<ast::InvariantDecoration>() ||
            deco->Is<ast::LocationDecoration>() ||
            deco->Is<ast::StructMemberOffsetDecoration>() ||
            deco->Is<ast::StructMemberSizeDecoration>() ||
            deco->Is<ast::StructMemberAlignDecoration>())) {
        AddError("decoration is not valid for structure members",
                 deco->source());
        return false;
      }
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (!ValidateBuiltinDecoration(builtin, member->Type())) {
          return false;
        }
      } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
        if (!ValidateInterpolateDecoration(interpolate, member->Type())) {
          return false;
        }
      }
    }

    if (auto* member_struct_type = member->Type()->As<sem::Struct>()) {
      if (auto* member_struct_type_block_decoration =
              ast::GetDecoration<ast::StructBlockDecoration>(
                  member_struct_type->Declaration()->decorations())) {
        AddError("structs must not contain [[block]] decorated struct members",
                 member->Declaration()->source());
        AddNote("see member's struct decoration here",
                member_struct_type_block_decoration->source());
        return false;
      }
    }
  }

  for (auto* deco : str->Declaration()->decorations()) {
    if (!(deco->Is<ast::StructBlockDecoration>())) {
      AddError("decoration is not valid for struct declarations",
               deco->source());
      return false;
    }
  }

  return true;
}

sem::Struct* Resolver::Structure(const ast::Struct* str) {
  if (!ValidateNoDuplicateDecorations(str->decorations())) {
    return nullptr;
  }
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
    if (!IsPlain(type)) {
      AddError(type->FriendlyName(builder_->Symbols()) +
                   " cannot be used as the type of a structure member",
               member->source());
      return nullptr;
    }

    uint32_t offset = struct_size;
    uint32_t align = 0;
    uint32_t size = 0;
    if (!DefaultAlignAndSize(type, align, size)) {
      return nullptr;
    }

    if (!ValidateNoDuplicateDecorations(member->decorations())) {
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
          AddError("offsets must be in ascending order", o->source());
          return nullptr;
        }
        offset = o->offset();
        align = 1;
        has_offset_deco = true;
      } else if (auto* a = deco->As<ast::StructMemberAlignDecoration>()) {
        if (a->align() <= 0 || !utils::IsPowerOfTwo(a->align())) {
          AddError("align value must be a positive, power-of-two integer",
                   a->source());
          return nullptr;
        }
        align = a->align();
        has_align_deco = true;
      } else if (auto* s = deco->As<ast::StructMemberSizeDecoration>()) {
        if (s->size() < size) {
          AddError("size must be at least as big as the type's size (" +
                       std::to_string(size) + ")",
                   s->source());
          return nullptr;
        }
        size = s->size();
        has_size_deco = true;
      }
    }

    if (has_offset_deco && (has_align_deco || has_size_deco)) {
      AddError(
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

  auto* out = builder_->create<sem::Struct>(str, sem_members, struct_align,
                                            struct_size, size_no_padding);

  // Keep track of atomic members for validation after all usages have been
  // determined.
  for (size_t i = 0; i < sem_members.size(); i++) {
    if (sem_members[i]->Type()->Is<sem::Atomic>()) {
      atomic_members_.emplace_back(StructMember{out, i});
    }
  }

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
    AddError(
        "return statement type must match its function "
        "return type, returned '" +
            ret_type->FriendlyName(builder_->Symbols()) + "', expected '" +
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
    if (!Expression(value)) {
      return false;
    }
  }

  // Validate after processing the return value expression so that its type is
  // available for validation.
  return ValidateReturn(ret);
}

bool Resolver::ValidateSwitch(const ast::SwitchStatement* s) {
  auto* cond_type = TypeOf(s->condition())->UnwrapRef();
  if (!cond_type->is_integer_scalar()) {
    AddError(
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
        AddError("switch statement must have exactly one default clause",
                 case_stmt->source());
        return false;
      }
      has_default = true;
    }

    for (auto* selector : case_stmt->selectors()) {
      if (cond_type != TypeOf(selector)) {
        AddError(
            "the case selector values must have the same "
            "type as the selector expression.",
            case_stmt->source());
        return false;
      }

      auto v = selector->value_as_u32();
      if (selector_set.find(v) != selector_set.end()) {
        AddError(
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
    AddError("switch statement must have a default clause", s->source());
    return false;
  }

  if (!s->body().empty()) {
    auto* last_clause = s->body().back()->As<ast::CaseStatement>();
    auto* last_stmt = last_clause->body()->last();
    if (last_stmt && last_stmt->Is<ast::FallthroughStatement>()) {
      AddError(
          "a fallthrough statement must not appear as "
          "the last statement in last clause of a switch",
          last_stmt->source());
      return false;
    }
  }

  return true;
}

bool Resolver::SwitchStatement(ast::SwitchStatement* stmt) {
  auto* sem =
      builder_->create<sem::SwitchStatement>(stmt, current_compound_statement_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    Mark(stmt->condition());
    if (!Expression(stmt->condition())) {
      return false;
    }
    for (auto* case_stmt : stmt->body()) {
      Mark(case_stmt);
      if (!CaseStatement(case_stmt)) {
        return false;
      }
    }
    if (!ValidateSwitch(stmt)) {
      return false;
    }
    return true;
  });
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

  if (auto* ident = a->lhs()->As<ast::IdentifierExpression>()) {
    VariableInfo* var;
    if (variable_stack_.get(ident->symbol(), &var)) {
      if (var->kind == VariableKind::kParameter) {
        AddError("cannot assign to function parameter", a->lhs()->source());
        AddNote("'" + builder_->Symbols().NameFor(ident->symbol()) +
                    "' is declared here:",
                var->declaration->source());
        return false;
      }
      if (var->declaration->is_const()) {
        AddError("cannot assign to const", a->lhs()->source());
        AddNote("'" + builder_->Symbols().NameFor(ident->symbol()) +
                    "' is declared here:",
                var->declaration->source());
        return false;
      }
    }
  }

  auto* lhs_ref = lhs_type->As<sem::Reference>();
  if (!lhs_ref) {
    // LHS is not a reference, so it has no storage.
    AddError("cannot assign to value of type '" + TypeNameOf(a->lhs()) + "'",
             a->lhs()->source());
    return false;
  }

  auto* storage_type = lhs_ref->StoreType();
  auto* value_type = rhs_type->UnwrapRef();  // Implicit load of RHS

  // Value type has to match storage type
  if (storage_type != value_type) {
    AddError("cannot assign '" + TypeNameOf(a->rhs()) + "' to '" +
                 TypeNameOf(a->lhs()) + "'",
             a->source());
    return false;
  }
  if (lhs_ref->Access() == ast::Access::kRead) {
    AddError(
        "cannot store into a read-only type '" + TypeNameOf(a->lhs()) + "'",
        a->source());
    return false;
  }
  return true;
}

bool Resolver::ValidateNoDuplicateDefinition(Symbol sym,
                                             const Source& source,
                                             bool check_global_scope_only) {
  if (check_global_scope_only) {
    bool is_global = false;
    VariableInfo* var;
    if (variable_stack_.get(sym, &var, &is_global)) {
      if (is_global) {
        AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
                 source);
        AddNote("previous definition is here", var->declaration->source());
        return false;
      }
    }
    auto it = symbol_to_function_.find(sym);
    if (it != symbol_to_function_.end()) {
      AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
               source);
      AddNote("previous definition is here", it->second->declaration->source());
      return false;
    }
  } else {
    VariableInfo* var;
    if (variable_stack_.get(sym, &var)) {
      AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
               source);
      AddNote("previous definition is here", var->declaration->source());
      return false;
    }
  }
  return true;
}

bool Resolver::ValidateNoDuplicateDecorations(
    const ast::DecorationList& decorations) {
  std::unordered_map<const TypeInfo*, Source> seen;
  for (auto* d : decorations) {
    auto res = seen.emplace(&d->TypeInfo(), d->source());
    if (!res.second && !d->Is<ast::InternalDecoration>()) {
      AddError("duplicate " + d->name() + " decoration", d->source());
      AddNote("first decoration declared here", res.first->second);
      return false;
    }
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
        AddNote(err.str(), member->Declaration()->source());
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
    AddError(err.str(), usage);
    return false;
  }

  return true;
}

template <typename F>
bool Resolver::Scope(sem::CompoundStatement* stmt, F&& callback) {
  auto* prev_current_statement = current_statement_;
  auto* prev_current_compound_statement = current_compound_statement_;
  auto* prev_current_block = current_block_;
  current_statement_ = stmt;
  current_compound_statement_ = stmt;
  current_block_ = stmt->As<sem::BlockStatement>();
  variable_stack_.push_scope();

  TINT_DEFER({
    TINT_DEFER(variable_stack_.pop_scope());
    current_block_ = prev_current_block;
    current_compound_statement_ = prev_current_compound_statement;
    current_statement_ = prev_current_statement;
  });

  return callback();
}

std::string Resolver::VectorPretty(uint32_t size,
                                   const sem::Type* element_type) {
  sem::Vector vec_type(element_type, size);
  return vec_type.FriendlyName(builder_->Symbols());
}

void Resolver::Mark(const ast::Node* node) {
  if (node == nullptr) {
    TINT_ICE(Resolver, diagnostics_) << "Resolver::Mark() called with nullptr";
  }
  if (marked_.emplace(node).second) {
    return;
  }
  TINT_ICE(Resolver, diagnostics_)
      << "AST node '" << node->TypeInfo().name
      << "' was encountered twice in the same AST of a Program\n"
      << "At: " << node->source() << "\n"
      << "Content: " << builder_->str(node) << "\n"
      << "Pointer: " << node;
}

void Resolver::AddError(const std::string& msg, const Source& source) const {
  diagnostics_.add_error(diag::System::Resolver, msg, source);
}

void Resolver::AddWarning(const std::string& msg, const Source& source) const {
  diagnostics_.add_warning(diag::System::Resolver, msg, source);
}

void Resolver::AddNote(const std::string& msg, const Source& source) const {
  diagnostics_.add_note(diag::System::Resolver, msg, source);
}

Resolver::VariableInfo::VariableInfo(const ast::Variable* decl,
                                     sem::Type* ty,
                                     const std::string& tn,
                                     ast::StorageClass sc,
                                     ast::Access ac,
                                     VariableKind k)
    : declaration(decl),
      type(ty),
      type_name(tn),
      storage_class(sc),
      access(ac),
      kind(k) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}
Resolver::FunctionInfo::~FunctionInfo() = default;

}  // namespace resolver
}  // namespace tint
