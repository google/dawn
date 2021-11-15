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
#include <cmath>
#include <iomanip>
#include <limits>
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
#include "src/ast/traverse_expressions.h"
#include "src/ast/type_name.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/vector.h"
#include "src/ast/workgroup_decoration.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/call.h"
#include "src/sem/depth_multisampled_texture_type.h"
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
#include "src/sem/type_constructor.h"
#include "src/sem/type_conversion.h"
#include "src/sem/variable.h"
#include "src/utils/defer.h"
#include "src/utils/get_or_create.h"
#include "src/utils/math.h"
#include "src/utils/reverse.h"
#include "src/utils/scoped_assignment.h"
#include "src/utils/transform.h"

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
      if (dv->validation == validation) {
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
    str << "builtin(" << builtin->builtin << ")";
  } else if (auto* location = deco->As<ast::LocationDecoration>()) {
    str << "location(" << location->value << ")";
  }
  return str.str();
}
}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder),
      diagnostics_(builder->Diagnostics()),
      intrinsic_table_(IntrinsicTable::Create(*builder)) {}

Resolver::~Resolver() = default;

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

  return result;
}

// https://gpuweb.github.io/gpuweb/wgsl/#plain-types-section
bool Resolver::IsPlain(const sem::Type* type) const {
  return type->is_scalar() ||
         type->IsAnyOf<sem::Atomic, sem::Vector, sem::Matrix, sem::Array,
                       sem::Struct>();
}

// https://gpuweb.github.io/gpuweb/wgsl.html#storable-types
bool Resolver::IsStorable(const sem::Type* type) const {
  return IsPlain(type) || type->IsAnyOf<sem::Texture, sem::Sampler>();
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

  AllocateOverridableConstantIds();

  if (!ValidatePipelineStages()) {
    return false;
  }

  bool result = true;
  for (auto* node : builder_->ASTNodes().Objects()) {
    if (marked_.count(node) == 0) {
      TINT_ICE(Resolver, diagnostics_) << "AST node '" << node->TypeInfo().name
                                       << "' was not reached by the resolver\n"
                                       << "At: " << node->source << "\n"
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
      if (auto* el = Type(t->type)) {
        if (auto* vector = builder_->create<sem::Vector>(el, t->width)) {
          if (ValidateVector(vector, t->source)) {
            return vector;
          }
        }
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Matrix>()) {
      if (auto* el = Type(t->type)) {
        if (auto* column_type = builder_->create<sem::Vector>(el, t->rows)) {
          if (auto* matrix =
                  builder_->create<sem::Matrix>(column_type, t->columns)) {
            if (ValidateMatrix(matrix, t->source)) {
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
      if (auto* el = Type(t->type)) {
        auto* a = builder_->create<sem::Atomic>(el);
        if (!ValidateAtomic(t, a)) {
          return nullptr;
        }
        return a;
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Pointer>()) {
      if (auto* el = Type(t->type)) {
        auto access = t->access;
        if (access == ast::kUndefined) {
          access = DefaultAccessForStorageClass(t->storage_class);
        }
        return builder_->create<sem::Pointer>(el, t->storage_class, access);
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Sampler>()) {
      return builder_->create<sem::Sampler>(t->kind);
    }
    if (auto* t = ty->As<ast::SampledTexture>()) {
      if (auto* el = Type(t->type)) {
        return builder_->create<sem::SampledTexture>(t->dim, el);
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::MultisampledTexture>()) {
      if (auto* el = Type(t->type)) {
        return builder_->create<sem::MultisampledTexture>(t->dim, el);
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::DepthTexture>()) {
      return builder_->create<sem::DepthTexture>(t->dim);
    }
    if (auto* t = ty->As<ast::DepthMultisampledTexture>()) {
      return builder_->create<sem::DepthMultisampledTexture>(t->dim);
    }
    if (auto* t = ty->As<ast::StorageTexture>()) {
      if (auto* el = Type(t->type)) {
        if (!ValidateStorageTexture(t)) {
          return nullptr;
        }
        return builder_->create<sem::StorageTexture>(t->dim, t->format,
                                                     t->access, el);
      }
      return nullptr;
    }
    if (ty->As<ast::ExternalTexture>()) {
      return builder_->create<sem::ExternalTexture>();
    }
    if (auto* t = ty->As<ast::TypeName>()) {
      auto it = named_type_info_.find(t->name);
      if (it == named_type_info_.end()) {
        AddError("unknown type '" + builder_->Symbols().NameFor(t->name) + "'",
                 t->source);
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
    AddError("atomic only supports i32 or u32 types",
             a->type ? a->type->source : a->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateStorageTexture(const ast::StorageTexture* t) {
  switch (t->access) {
    case ast::Access::kWrite:
      break;
    case ast::Access::kUndefined:
      AddError("storage texture missing access control", t->source);
      return false;
    default:
      AddError("storage textures currently only support 'write' access control",
               t->source);
      return false;
  }

  if (!IsValidStorageTextureDimension(t->dim)) {
    AddError("cube dimensions for storage textures are not supported",
             t->source);
    return false;
  }

  if (!IsValidStorageTextureImageFormat(t->format)) {
    AddError(
        "image format must be one of the texel formats specified for storage "
        "textues in https://gpuweb.github.io/gpuweb/wgsl/#texel-formats",
        t->source);
    return false;
  }
  return true;
}

sem::Variable* Resolver::Variable(const ast::Variable* var,
                                  VariableKind kind,
                                  uint32_t index /* = 0 */) {
  const sem::Type* storage_ty = nullptr;

  // If the variable has a declared type, resolve it.
  if (auto* ty = var->type) {
    storage_ty = Type(ty);
    if (!storage_ty) {
      return nullptr;
    }
  }

  const sem::Expression* rhs = nullptr;

  // Does the variable have a constructor?
  if (var->constructor) {
    rhs = Expression(var->constructor);
    if (!rhs) {
      return nullptr;
    }

    // If the variable has no declared type, infer it from the RHS
    if (!storage_ty) {
      if (!var->is_const && kind == VariableKind::kGlobal) {
        AddError("global var declaration must specify a type", var->source);
        return nullptr;
      }

      storage_ty = rhs->Type()->UnwrapRef();  // Implicit load of RHS
    }
  } else if (var->is_const && kind != VariableKind::kParameter &&
             !ast::HasDecoration<ast::OverrideDecoration>(var->decorations)) {
    AddError("let declaration must have an initializer", var->source);
    return nullptr;
  } else if (!var->type) {
    AddError(
        (kind == VariableKind::kGlobal)
            ? "module scope var declaration requires a type and initializer"
            : "function scope var declaration requires a type or initializer",
        var->source);
    return nullptr;
  }

  if (!storage_ty) {
    TINT_ICE(Resolver, diagnostics_)
        << "failed to determine storage type for variable '" +
               builder_->Symbols().NameFor(var->symbol) + "'\n"
        << "Source: " << var->source;
    return nullptr;
  }

  auto storage_class = var->declared_storage_class;
  if (storage_class == ast::StorageClass::kNone && !var->is_const) {
    // No declared storage class. Infer from usage / type.
    if (kind == VariableKind::kLocal) {
      storage_class = ast::StorageClass::kFunction;
    } else if (storage_ty->UnwrapRef()->is_handle()) {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // If the store type is a texture type or a sampler type, then the
      // variable declaration must not have a storage class decoration. The
      // storage class will always be handle.
      storage_class = ast::StorageClass::kUniformConstant;
    }
  }

  if (kind == VariableKind::kLocal && !var->is_const &&
      storage_class != ast::StorageClass::kFunction &&
      IsValidationEnabled(var->decorations,
                          ast::DisabledValidation::kIgnoreStorageClass)) {
    AddError("function variable has a non-function storage class", var->source);
    return nullptr;
  }

  auto access = var->declared_access;
  if (access == ast::Access::kUndefined) {
    access = DefaultAccessForStorageClass(storage_class);
  }

  auto* var_ty = storage_ty;
  if (!var->is_const) {
    // Variable declaration. Unlike `let`, `var` has storage.
    // Variables are always of a reference type to the declared storage type.
    var_ty =
        builder_->create<sem::Reference>(storage_ty, storage_class, access);
  }

  if (rhs && !ValidateVariableConstructorOrCast(var, storage_class, storage_ty,
                                                rhs->Type())) {
    return nullptr;
  }

  if (!ApplyStorageClassUsageToType(
          storage_class, const_cast<sem::Type*>(var_ty), var->source)) {
    AddNote(
        std::string("while instantiating ") +
            ((kind == VariableKind::kParameter) ? "parameter " : "variable ") +
            builder_->Symbols().NameFor(var->symbol),
        var->source);
    return nullptr;
  }

  if (kind == VariableKind::kParameter) {
    if (auto* ptr = var_ty->As<sem::Pointer>()) {
      // For MSL, we push module-scope variables into the entry point as pointer
      // parameters, so we also need to handle their store type.
      if (!ApplyStorageClassUsageToType(
              ptr->StorageClass(), const_cast<sem::Type*>(ptr->StoreType()),
              var->source)) {
        AddNote("while instantiating parameter " +
                    builder_->Symbols().NameFor(var->symbol),
                var->source);
        return nullptr;
      }
    }
  }

  switch (kind) {
    case VariableKind::kGlobal: {
      sem::BindingPoint binding_point;
      if (auto bp = var->BindingPoint()) {
        binding_point = {bp.group->value, bp.binding->value};
      }

      auto* override =
          ast::GetDecoration<ast::OverrideDecoration>(var->decorations);
      bool has_const_val = rhs && var->is_const && !override;

      auto* global = builder_->create<sem::GlobalVariable>(
          var, var_ty, storage_class, access,
          has_const_val ? rhs->ConstantValue() : sem::Constant{},
          binding_point);

      if (override) {
        global->SetIsOverridable();
        if (override->has_value) {
          global->SetConstantId(static_cast<uint16_t>(override->value));
        }
      }

      builder_->Sem().Add(var, global);
      return global;
    }
    case VariableKind::kLocal: {
      auto* local = builder_->create<sem::LocalVariable>(
          var, var_ty, storage_class, access,
          (rhs && var->is_const) ? rhs->ConstantValue() : sem::Constant{});
      builder_->Sem().Add(var, local);
      return local;
    }
    case VariableKind::kParameter: {
      auto* param = builder_->create<sem::Parameter>(var, index, var_ty,
                                                     storage_class, access);
      builder_->Sem().Add(var, param);
      return param;
    }
  }

  TINT_UNREACHABLE(Resolver, diagnostics_)
      << "unhandled VariableKind " << static_cast<int>(kind);
  return nullptr;
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

void Resolver::AllocateOverridableConstantIds() {
  // The next pipeline constant ID to try to allocate.
  uint16_t next_constant_id = 0;

  // Allocate constant IDs in global declaration order, so that they are
  // deterministic.
  // TODO(crbug.com/tint/1192): If a transform changes the order or removes an
  // unused constant, the allocation may change on the next Resolver pass.
  for (auto* decl : builder_->AST().GlobalDeclarations()) {
    auto* var = decl->As<ast::Variable>();
    if (!var) {
      continue;
    }
    auto* override_deco =
        ast::GetDecoration<ast::OverrideDecoration>(var->decorations);
    if (!override_deco) {
      continue;
    }

    uint16_t constant_id;
    if (override_deco->has_value) {
      constant_id = static_cast<uint16_t>(override_deco->value);
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

    auto* sem = Sem<sem::GlobalVariable>(var);
    const_cast<sem::GlobalVariable*>(sem)->SetConstantId(constant_id);
  }
}

bool Resolver::ValidateVariableConstructorOrCast(
    const ast::Variable* var,
    ast::StorageClass storage_class,
    const sem::Type* storage_ty,
    const sem::Type* rhs_ty) {
  auto* value_type = rhs_ty->UnwrapRef();  // Implicit load of RHS

  // Value type has to match storage type
  if (storage_ty != value_type) {
    std::string decl = var->is_const ? "let" : "var";
    AddError("cannot initialize " + decl + " of type '" +
                 TypeNameOf(storage_ty) + "' with value of type '" +
                 TypeNameOf(rhs_ty) + "'",
             var->source);
    return false;
  }

  if (!var->is_const) {
    switch (storage_class) {
      case ast::StorageClass::kPrivate:
      case ast::StorageClass::kFunction:
        break;  // Allowed an initializer
      default:
        // https://gpuweb.github.io/gpuweb/wgsl/#var-and-let
        // Optionally has an initializer expression, if the variable is in the
        // private or function storage classes.
        AddError("var of storage class '" +
                     std::string(ast::ToString(storage_class)) +
                     "' cannot have an initializer. var initializers are only "
                     "supported for the storage classes "
                     "'private' and 'function'",
                 var->source);
        return false;
    }
  }

  return true;
}

bool Resolver::GlobalVariable(const ast::Variable* var) {
  if (!ValidateNoDuplicateDefinition(var->symbol, var->source,
                                     /* check_global_scope_only */ true)) {
    return false;
  }

  auto* sem = Variable(var, VariableKind::kGlobal);
  if (!sem) {
    return false;
  }
  variable_stack_.Set(var->symbol, sem);

  auto storage_class = sem->StorageClass();
  if (!var->is_const && storage_class == ast::StorageClass::kNone) {
    AddError("global variables must have a storage class", var->source);
    return false;
  }
  if (var->is_const && storage_class != ast::StorageClass::kNone) {
    AddError("global constants shouldn't have a storage class", var->source);
    return false;
  }

  for (auto* deco : var->decorations) {
    Mark(deco);

    if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
      // Track the constant IDs that are specified in the shader.
      if (override_deco->has_value) {
        constant_ids_.emplace(override_deco->value, sem);
      }
    }
  }

  if (!ValidateNoDuplicateDecorations(var->decorations)) {
    return false;
  }

  if (!ValidateGlobalVariable(sem)) {
    return false;
  }

  // TODO(bclayton): Call this at the end of resolve on all uniform and storage
  // referenced structs
  if (!ValidateStorageClassLayout(sem)) {
    return false;
  }

  return true;
}

bool Resolver::ValidateStorageClassLayout(const sem::Struct* str,
                                          ast::StorageClass sc) {
  // https://gpuweb.github.io/gpuweb/wgsl/#storage-class-layout-constraints

  auto is_uniform_struct_or_array = [sc](const sem::Type* ty) {
    return sc == ast::StorageClass::kUniform &&
           ty->IsAnyOf<sem::Array, sem::Struct>();
  };

  auto is_uniform_struct = [sc](const sem::Type* ty) {
    return sc == ast::StorageClass::kUniform && ty->Is<sem::Struct>();
  };

  auto required_alignment_of = [&](const sem::Type* ty) {
    uint32_t actual_align = ty->Align();
    uint32_t required_align = actual_align;
    if (is_uniform_struct_or_array(ty)) {
      required_align = utils::RoundUp(16u, actual_align);
    }
    return required_align;
  };

  auto member_name_of = [this](const sem::StructMember* sm) {
    return builder_->Symbols().NameFor(sm->Declaration()->symbol);
  };

  auto type_name_of = [this](const sem::StructMember* sm) {
    return TypeNameOf(sm->Type());
  };

  // TODO(amaiorano): Output struct and member decorations so that this output
  // can be copied verbatim back into source
  auto get_struct_layout_string = [&](const sem::Struct* st) -> std::string {
    std::stringstream ss;

    if (st->Members().empty()) {
      TINT_ICE(Resolver, diagnostics_) << "Validation should have ensured that "
                                          "structs have at least one member";
      return {};
    }
    const auto* const last_member = st->Members().back();
    const uint32_t last_member_struct_padding_offset =
        last_member->Offset() + last_member->Size();

    // Compute max widths to align output
    const auto offset_w =
        static_cast<int>(::log10(last_member_struct_padding_offset)) + 1;
    const auto size_w = static_cast<int>(::log10(st->Size())) + 1;
    const auto align_w = static_cast<int>(::log10(st->Align())) + 1;

    auto print_struct_begin_line = [&](size_t align, size_t size,
                                       std::string struct_name) {
      ss << "/*          " << std::setw(offset_w) << " "
         << "align(" << std::setw(align_w) << align << ") size("
         << std::setw(size_w) << size << ") */ struct " << struct_name
         << " {\n";
    };

    auto print_struct_end_line = [&]() {
      ss << "/*                         "
         << std::setw(offset_w + size_w + align_w) << " "
         << "*/ };";
    };

    auto print_member_line = [&](size_t offset, size_t align, size_t size,
                                 std::string s) {
      ss << "/* offset(" << std::setw(offset_w) << offset << ") align("
         << std::setw(align_w) << align << ") size(" << std::setw(size_w)
         << size << ") */   " << s << ";\n";
    };

    print_struct_begin_line(st->Align(), st->Size(), TypeNameOf(st));

    for (size_t i = 0; i < st->Members().size(); ++i) {
      auto* const m = st->Members()[i];

      // Output field alignment padding, if any
      auto* const prev_member = (i == 0) ? nullptr : str->Members()[i - 1];
      if (prev_member) {
        uint32_t padding =
            m->Offset() - (prev_member->Offset() + prev_member->Size());
        if (padding > 0) {
          size_t padding_offset = m->Offset() - padding;
          print_member_line(padding_offset, 1, padding,
                            "// -- implicit field alignment padding --");
        }
      }

      // Output member
      std::string member_name = member_name_of(m);
      print_member_line(m->Offset(), m->Align(), m->Size(),
                        member_name_of(m) + " : " + type_name_of(m));
    }

    // Output struct size padding, if any
    uint32_t struct_padding = st->Size() - last_member_struct_padding_offset;
    if (struct_padding > 0) {
      print_member_line(last_member_struct_padding_offset, 1, struct_padding,
                        "// -- implicit struct size padding --");
    }

    print_struct_end_line();

    return ss.str();
  };

  if (!ast::IsHostShareable(sc)) {
    return true;
  }

  for (size_t i = 0; i < str->Members().size(); ++i) {
    auto* const m = str->Members()[i];
    uint32_t required_align = required_alignment_of(m->Type());

    // Validate that member is at a valid byte offset
    if (m->Offset() % required_align != 0) {
      AddError("the offset of a struct member of type '" + type_name_of(m) +
                   "' in storage class '" + ast::ToString(sc) +
                   "' must be a multiple of " + std::to_string(required_align) +
                   " bytes, but '" + member_name_of(m) +
                   "' is currently at offset " + std::to_string(m->Offset()) +
                   ". Consider setting [[align(" +
                   std::to_string(required_align) + ")]] on this member",
               m->Declaration()->source);

      AddNote("see layout of struct:\n" + get_struct_layout_string(str),
              str->Declaration()->source);

      if (auto* member_str = m->Type()->As<sem::Struct>()) {
        AddNote("and layout of struct member:\n" +
                    get_struct_layout_string(member_str),
                member_str->Declaration()->source);
      }

      return false;
    }

    // For uniform buffers, validate that the number of bytes between the
    // previous member of type struct and the current is a multiple of 16 bytes.
    auto* const prev_member = (i == 0) ? nullptr : str->Members()[i - 1];
    if (prev_member && is_uniform_struct(prev_member->Type())) {
      const uint32_t prev_to_curr_offset = m->Offset() - prev_member->Offset();
      if (prev_to_curr_offset % 16 != 0) {
        AddError(
            "uniform storage requires that the number of bytes between the "
            "start of the previous member of type struct and the current "
            "member be a multiple of 16 bytes, but there are currently " +
                std::to_string(prev_to_curr_offset) + " bytes between '" +
                member_name_of(prev_member) + "' and '" + member_name_of(m) +
                "'. Consider setting [[align(16)]] on this member",
            m->Declaration()->source);

        AddNote("see layout of struct:\n" + get_struct_layout_string(str),
                str->Declaration()->source);

        auto* prev_member_str = prev_member->Type()->As<sem::Struct>();
        AddNote("and layout of previous member struct:\n" +
                    get_struct_layout_string(prev_member_str),
                prev_member_str->Declaration()->source);
        return false;
      }
    }

    // For uniform buffer array members, validate that array elements are
    // aligned to 16 bytes
    if (auto* arr = m->Type()->As<sem::Array>()) {
      if (sc == ast::StorageClass::kUniform) {
        // We already validated that this array member is itself aligned to 16
        // bytes above, so we only need to validate that stride is a multiple of
        // 16 bytes.
        if (arr->Stride() % 16 != 0) {
          AddError(
              "uniform storage requires that array elements be aligned to 16 "
              "bytes, but array stride of '" +
                  member_name_of(m) + "' is currently " +
                  std::to_string(arr->Stride()) +
                  ". Consider setting [[stride(" +
                  std::to_string(
                      utils::RoundUp(required_align, arr->Stride())) +
                  ")]] on the array type",
              m->Declaration()->type->source);
          AddNote("see layout of struct:\n" + get_struct_layout_string(str),
                  str->Declaration()->source);
          return false;
        }
      }
    }

    // If member is struct, recurse
    if (auto* str_member = m->Type()->As<sem::Struct>()) {
      // Cache result of struct + storage class pair
      if (valid_struct_storage_layouts_.emplace(str_member, sc).second) {
        if (!ValidateStorageClassLayout(str_member, sc)) {
          return false;
        }
      }
    }
  }

  return true;
}

bool Resolver::ValidateStorageClassLayout(const sem::Variable* var) {
  if (auto* str = var->Type()->UnwrapRef()->As<sem::Struct>()) {
    if (!ValidateStorageClassLayout(str, var->StorageClass())) {
      AddNote("see declaration of variable", var->Declaration()->source);
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateGlobalVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  if (!ValidateNoDuplicateDecorations(decl->decorations)) {
    return false;
  }

  for (auto* deco : decl->decorations) {
    if (decl->is_const) {
      if (auto* override_deco = deco->As<ast::OverrideDecoration>()) {
        if (override_deco->has_value) {
          uint32_t id = override_deco->value;
          auto it = constant_ids_.find(id);
          if (it != constant_ids_.end() && it->second != var) {
            AddError("pipeline constant IDs must be unique", deco->source);
            AddNote("a pipeline constant with an ID of " + std::to_string(id) +
                        " was previously declared "
                        "here:",
                    ast::GetDecoration<ast::OverrideDecoration>(
                        it->second->Declaration()->decorations)
                        ->source);
            return false;
          }
          if (id > 65535) {
            AddError("pipeline constant IDs must be between 0 and 65535",
                     deco->source);
            return false;
          }
        }
      } else {
        AddError("decoration is not valid for constants", deco->source);
        return false;
      }
    } else {
      bool is_shader_io_decoration =
          deco->IsAnyOf<ast::BuiltinDecoration, ast::InterpolateDecoration,
                        ast::InvariantDecoration, ast::LocationDecoration>();
      bool has_io_storage_class =
          var->StorageClass() == ast::StorageClass::kInput ||
          var->StorageClass() == ast::StorageClass::kOutput;
      if (!(deco->IsAnyOf<ast::BindingDecoration, ast::GroupDecoration,
                          ast::InternalDecoration>()) &&
          (!is_shader_io_decoration || !has_io_storage_class)) {
        AddError("decoration is not valid for variables", deco->source);
        return false;
      }
    }
  }

  auto binding_point = decl->BindingPoint();
  switch (var->StorageClass()) {
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
            decl->source);
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
            decl->source);
        return false;
      }
  }

  // https://gpuweb.github.io/gpuweb/wgsl/#variable-declaration
  // The access mode always has a default, and except for variables in the
  // storage storage class, must not be written.
  if (var->StorageClass() != ast::StorageClass::kStorage &&
      decl->declared_access != ast::Access::kUndefined) {
    AddError(
        "only variables in <storage> storage class may declare an access mode",
        decl->source);
    return false;
  }

  switch (var->StorageClass()) {
    case ast::StorageClass::kStorage: {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // A variable in the storage storage class is a storage buffer variable.
      // Its store type must be a host-shareable structure type with block
      // attribute, satisfying the storage class constraints.

      auto* str = var->Type()->UnwrapRef()->As<sem::Struct>();

      if (!str) {
        AddError(
            "variables declared in the <storage> storage class must be of a "
            "structure type",
            decl->source);
        return false;
      }

      if (!str->IsBlockDecorated()) {
        AddError(
            "structure used as a storage buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source);
        if (decl->source.range.begin.line) {
          AddNote("structure used as storage buffer here", decl->source);
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
      auto* str = var->Type()->UnwrapRef()->As<sem::Struct>();
      if (!str) {
        AddError(
            "variables declared in the <uniform> storage class must be of a "
            "structure type",
            decl->source);
        return false;
      }

      if (!str->IsBlockDecorated()) {
        AddError(
            "structure used as a uniform buffer must be declared with the "
            "[[block]] decoration",
            str->Declaration()->source);
        if (decl->source.range.begin.line) {
          AddNote("structure used as uniform buffer here", decl->source);
        }
        return false;
      }

      for (auto* member : str->Members()) {
        if (auto* arr = member->Type()->As<sem::Array>()) {
          if (arr->IsRuntimeSized()) {
            AddError(
                "structure containing a runtime sized array "
                "cannot be used as a uniform buffer",
                decl->source);
            AddNote("structure is declared here", str->Declaration()->source);
            return false;
          }
        }
      }

      break;
    }
    default:
      break;
  }

  if (!decl->is_const) {
    if (!ValidateAtomicVariable(var)) {
      return false;
    }
  }

  return ValidateVariable(var);
}

// https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
// Atomic types may only be instantiated by variables in the workgroup storage
// class or by storage buffer variables with a read_write access mode.
bool Resolver::ValidateAtomicVariable(const sem::Variable* var) {
  auto sc = var->StorageClass();
  auto* decl = var->Declaration();
  auto access = var->Access();
  auto* type = var->Type()->UnwrapRef();
  auto source = decl->type ? decl->type->source : decl->source;

  if (type->Is<sem::Atomic>()) {
    if (sc != ast::StorageClass::kWorkgroup) {
      AddError(
          "atomic variables must have <storage> or <workgroup> storage class",
          source);
      return false;
    }
  } else if (type->IsAnyOf<sem::Struct, sem::Array>()) {
    auto found = atomic_composite_info_.find(type);
    if (found != atomic_composite_info_.end()) {
      if (sc != ast::StorageClass::kStorage &&
          sc != ast::StorageClass::kWorkgroup) {
        AddError(
            "atomic variables must have <storage> or <workgroup> storage class",
            source);
        AddNote(
            "atomic sub-type of '" + TypeNameOf(type) + "' is declared here",
            found->second);
        return false;
      } else if (sc == ast::StorageClass::kStorage &&
                 access != ast::Access::kReadWrite) {
        AddError(
            "atomic variables in <storage> storage class must have read_write "
            "access mode",
            source);
        AddNote(
            "atomic sub-type of '" + TypeNameOf(type) + "' is declared here",
            found->second);
        return false;
      }
    }
  }

  return true;
}

bool Resolver::ValidateVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* storage_ty = var->Type()->UnwrapRef();

  if (!decl->is_const && !IsStorable(storage_ty)) {
    AddError(TypeNameOf(storage_ty) + " cannot be used as the type of a var",
             decl->source);
    return false;
  }

  if (decl->is_const && !var->Is<sem::Parameter>() &&
      !(storage_ty->IsConstructible() || storage_ty->Is<sem::Pointer>())) {
    AddError(TypeNameOf(storage_ty) + " cannot be used as the type of a let",
             decl->source);
    return false;
  }

  if (auto* r = storage_ty->As<sem::Array>()) {
    if (r->IsRuntimeSized()) {
      AddError("runtime arrays may only appear as the last member of a struct",
               decl->source);
      return false;
    }
  }

  if (auto* r = storage_ty->As<sem::MultisampledTexture>()) {
    if (r->dim() != ast::TextureDimension::k2d) {
      AddError("only 2d multisampled textures are supported", decl->source);
      return false;
    }

    if (!r->type()->UnwrapRef()->is_numeric_scalar()) {
      AddError("texture_multisampled_2d<type>: type must be f32, i32 or u32",
               decl->source);
      return false;
    }
  }

  if (var->Is<sem::LocalVariable>() && !decl->is_const &&
      IsValidationEnabled(decl->decorations,
                          ast::DisabledValidation::kIgnoreStorageClass)) {
    if (!var->Type()->UnwrapRef()->IsConstructible()) {
      AddError("function variable must have a constructible type",
               decl->type ? decl->type->source : decl->source);
      return false;
    }
  }

  if (storage_ty->is_handle() &&
      decl->declared_storage_class != ast::StorageClass::kNone) {
    // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
    // If the store type is a texture type or a sampler type, then the
    // variable declaration must not have a storage class decoration. The
    // storage class will always be handle.
    AddError("variables of type '" + TypeNameOf(storage_ty) +
                 "' must not have a storage class",
             decl->source);
    return false;
  }

  if (IsValidationEnabled(decl->decorations,
                          ast::DisabledValidation::kIgnoreStorageClass) &&
      (decl->declared_storage_class == ast::StorageClass::kInput ||
       decl->declared_storage_class == ast::StorageClass::kOutput)) {
    AddError("invalid use of input/output storage class", decl->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateFunctionParameter(const ast::Function* func,
                                         const sem::Variable* var) {
  if (!ValidateVariable(var)) {
    return false;
  }

  auto* decl = var->Declaration();

  for (auto* deco : decl->decorations) {
    if (!func->IsEntryPoint() && !deco->Is<ast::InternalDecoration>()) {
      AddError(
          "decoration is not valid for non-entry point function parameters",
          deco->source);
      return false;
    } else if (!deco->IsAnyOf<ast::BuiltinDecoration, ast::InvariantDecoration,
                              ast::LocationDecoration,
                              ast::InterpolateDecoration,
                              ast::InternalDecoration>() &&
               (IsValidationEnabled(
                    decl->decorations,
                    ast::DisabledValidation::kEntryPointParameter) &&
                IsValidationEnabled(
                    decl->decorations,
                    ast::DisabledValidation::
                        kIgnoreConstructibleFunctionParameter))) {
      AddError("decoration is not valid for function parameters", deco->source);
      return false;
    }
  }

  if (auto* ref = var->Type()->As<sem::Pointer>()) {
    auto sc = ref->StorageClass();
    if (!(sc == ast::StorageClass::kFunction ||
          sc == ast::StorageClass::kPrivate ||
          sc == ast::StorageClass::kWorkgroup) &&
        IsValidationEnabled(decl->decorations,
                            ast::DisabledValidation::kIgnoreStorageClass)) {
      std::stringstream ss;
      ss << "function parameter of pointer type cannot be in '" << sc
         << "' storage class";
      AddError(ss.str(), decl->source);
      return false;
    }
  }

  if (IsPlain(var->Type())) {
    if (!var->Type()->IsConstructible() &&
        IsValidationEnabled(
            decl->decorations,
            ast::DisabledValidation::kIgnoreConstructibleFunctionParameter)) {
      AddError("store type of function parameter must be a constructible type",
               decl->source);
      return false;
    }
  } else if (!var->Type()
                  ->IsAnyOf<sem::Texture, sem::Sampler, sem::Pointer>()) {
    AddError(
        "store type of function parameter cannot be " + TypeNameOf(var->Type()),
        decl->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateBuiltinDecoration(const ast::BuiltinDecoration* deco,
                                         const sem::Type* storage_ty,
                                         const bool is_input) {
  auto* type = storage_ty->UnwrapRef();
  const auto stage = current_function_
                         ? current_function_->Declaration()->PipelineStage()
                         : ast::PipelineStage::kNone;
  std::stringstream stage_name;
  stage_name << stage;
  bool is_stage_mismatch = false;
  bool is_output = !is_input;
  switch (deco->builtin) {
    case ast::Builtin::kPosition:
      if (stage != ast::PipelineStage::kNone &&
          !((is_input && stage == ast::PipelineStage::kFragment) ||
            (is_output && stage == ast::PipelineStage::kVertex))) {
        is_stage_mismatch = true;
      }
      if (!(type->is_float_vector() && type->As<sem::Vector>()->Width() == 4)) {
        AddError("store type of " + deco_to_str(deco) + " must be 'vec4<f32>'",
                 deco->source);
        return false;
      }
      break;
    case ast::Builtin::kGlobalInvocationId:
    case ast::Builtin::kLocalInvocationId:
    case ast::Builtin::kNumWorkgroups:
    case ast::Builtin::kWorkgroupId:
      if (stage != ast::PipelineStage::kNone &&
          !(stage == ast::PipelineStage::kCompute && is_input)) {
        is_stage_mismatch = true;
      }
      if (!(type->is_unsigned_integer_vector() &&
            type->As<sem::Vector>()->Width() == 3)) {
        AddError("store type of " + deco_to_str(deco) + " must be 'vec3<u32>'",
                 deco->source);
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
                 deco->source);
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
                 deco->source);
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
                 deco->source);
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
                 deco->source);
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
                 deco->source);
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
                 deco->source);
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
             deco->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateInterpolateDecoration(
    const ast::InterpolateDecoration* deco,
    const sem::Type* storage_ty) {
  auto* type = storage_ty->UnwrapRef();

  if (type->is_integer_scalar_or_vector() &&
      deco->type != ast::InterpolationType::kFlat) {
    AddError(
        "interpolation type must be 'flat' for integral user-defined IO types",
        deco->source);
    return false;
  }

  if (deco->type == ast::InterpolationType::kFlat &&
      deco->sampling != ast::InterpolationSampling::kNone) {
    AddError("flat interpolation attribute must not have a sampling parameter",
             deco->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateFunction(const sem::Function* func) {
  auto* decl = func->Declaration();
  if (!ValidateNoDuplicateDefinition(decl->symbol, decl->source,
                                     /* check_global_scope_only */ true)) {
    return false;
  }

  auto workgroup_deco_count = 0;
  for (auto* deco : decl->decorations) {
    if (deco->Is<ast::WorkgroupDecoration>()) {
      workgroup_deco_count++;
      if (decl->PipelineStage() != ast::PipelineStage::kCompute) {
        AddError(
            "the workgroup_size attribute is only valid for compute stages",
            deco->source);
        return false;
      }
    } else if (!deco->IsAnyOf<ast::StageDecoration,
                              ast::InternalDecoration>()) {
      AddError("decoration is not valid for functions", deco->source);
      return false;
    }
  }

  if (decl->params.size() > 255) {
    AddError("functions may declare at most 255 parameters", decl->source);
    return false;
  }

  for (size_t i = 0; i < decl->params.size(); i++) {
    if (!ValidateFunctionParameter(decl, func->Parameters()[i])) {
      return false;
    }
  }

  if (!func->ReturnType()->Is<sem::Void>()) {
    if (!func->ReturnType()->IsConstructible()) {
      AddError("function return type must be a constructible type",
               decl->return_type->source);
      return false;
    }

    if (decl->body) {
      if (!decl->body->Last() ||
          !decl->body->Last()->Is<ast::ReturnStatement>()) {
        AddError("non-void function must end with a return statement",
                 decl->source);
        return false;
      }
    } else if (IsValidationEnabled(
                   decl->decorations,
                   ast::DisabledValidation::kFunctionHasNoBody)) {
      TINT_ICE(Resolver, diagnostics_)
          << "Function " << builder_->Symbols().NameFor(decl->symbol)
          << " has no body";
    }

    for (auto* deco : decl->return_type_decorations) {
      if (!decl->IsEntryPoint()) {
        AddError(
            "decoration is not valid for non-entry point function return types",
            deco->source);
        return false;
      }
      if (!deco->IsAnyOf<ast::BuiltinDecoration, ast::InternalDecoration,
                         ast::LocationDecoration, ast::InterpolateDecoration,
                         ast::InvariantDecoration>() &&
          (IsValidationEnabled(decl->decorations,
                               ast::DisabledValidation::kEntryPointParameter) &&
           IsValidationEnabled(decl->decorations,
                               ast::DisabledValidation::
                                   kIgnoreConstructibleFunctionParameter))) {
        AddError("decoration is not valid for entry point return types",
                 deco->source);
        return false;
      }
    }
  }

  if (decl->IsEntryPoint()) {
    if (!ValidateEntryPoint(func)) {
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateEntryPoint(const sem::Function* func) {
  auto* decl = func->Declaration();

  // Use a lambda to validate the entry point decorations for a type.
  // Persistent state is used to track which builtins and locations have
  // already been seen, in order to catch conflicts.
  // TODO(jrprice): This state could be stored in sem::Function instead, and
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
                                                    const sem::Type* ty,
                                                    Source source,
                                                    ParamOrRetType param_or_ret,
                                                    bool is_struct_member) {
    // Scan decorations for pipeline IO attributes.
    // Check for overlap with attributes that have been seen previously.
    const ast::Decoration* pipeline_io_attribute = nullptr;
    const ast::InterpolateDecoration* interpolate_attribute = nullptr;
    const ast::InvariantDecoration* invariant_attribute = nullptr;
    for (auto* deco : decos) {
      auto is_invalid_compute_shader_decoration = false;
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (pipeline_io_attribute) {
          AddError("multiple entry point IO attributes", deco->source);
          AddNote("previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source);
          return false;
        }
        pipeline_io_attribute = deco;

        if (builtins.count(builtin->builtin)) {
          AddError(deco_to_str(builtin) +
                       " attribute appears multiple times as pipeline " +
                       (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                   : "output"),
                   decl->source);
          return false;
        }

        if (!ValidateBuiltinDecoration(
                builtin, ty,
                /* is_input */ param_or_ret == ParamOrRetType::kParameter)) {
          return false;
        }
        builtins.emplace(builtin->builtin);
      } else if (auto* location = deco->As<ast::LocationDecoration>()) {
        if (pipeline_io_attribute) {
          AddError("multiple entry point IO attributes", deco->source);
          AddNote("previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source);
          return false;
        }
        pipeline_io_attribute = deco;

        bool is_input = param_or_ret == ParamOrRetType::kParameter;
        if (!ValidateLocationDecoration(location, ty, locations, source,
                                        is_input)) {
          return false;
        }
      } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
        if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
          is_invalid_compute_shader_decoration = true;
        } else if (!ValidateInterpolateDecoration(interpolate, ty)) {
          return false;
        }
        interpolate_attribute = interpolate;
      } else if (auto* invariant = deco->As<ast::InvariantDecoration>()) {
        if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
          is_invalid_compute_shader_decoration = true;
        }
        invariant_attribute = invariant;
      }
      if (is_invalid_compute_shader_decoration) {
        std::string input_or_output =
            param_or_ret == ParamOrRetType::kParameter ? "inputs" : "output";
        AddError(
            "decoration is not valid for compute shader " + input_or_output,
            deco->source);
        return false;
      }
    }

    if (IsValidationEnabled(decos,
                            ast::DisabledValidation::kEntryPointParameter)) {
      if (is_struct_member && ty->Is<sem::Struct>()) {
        AddError("nested structures cannot be used for entry point IO", source);
        return false;
      }

      if (!ty->Is<sem::Struct>() && !pipeline_io_attribute) {
        std::string err = "missing entry point IO attribute";
        if (!is_struct_member) {
          err +=
              (param_or_ret == ParamOrRetType::kParameter ? " on parameter"
                                                          : " on return type");
        }
        AddError(err, source);
        return false;
      }

      if (pipeline_io_attribute &&
          pipeline_io_attribute->Is<ast::LocationDecoration>()) {
        if (ty->is_integer_scalar_or_vector() && !interpolate_attribute) {
          // TODO(crbug.com/tint/1224): Make these errors once downstream
          // usages have caught up (no sooner than M99).
          if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
              param_or_ret == ParamOrRetType::kReturnType) {
            AddWarning(
                "integral user-defined vertex outputs must have a flat "
                "interpolation attribute",
                source);
          }
          if (decl->PipelineStage() == ast::PipelineStage::kFragment &&
              param_or_ret == ParamOrRetType::kParameter) {
            AddWarning(
                "integral user-defined fragment inputs must have a flat "
                "interpolation attribute",
                source);
          }
        }
      }

      if (invariant_attribute) {
        bool has_position = false;
        if (pipeline_io_attribute) {
          if (auto* builtin =
                  pipeline_io_attribute->As<ast::BuiltinDecoration>()) {
            has_position = (builtin->builtin == ast::Builtin::kPosition);
          }
        }
        if (!has_position) {
          AddError(
              "invariant attribute must only be applied to a position "
              "builtin",
              invariant_attribute->source);
          return false;
        }
      }
    }
    return true;
  };

  // Outer lambda for validating the entry point decorations for a type.
  auto validate_entry_point_decorations = [&](const ast::DecorationList& decos,
                                              const sem::Type* ty,
                                              Source source,
                                              ParamOrRetType param_or_ret) {
    if (!validate_entry_point_decorations_inner(decos, ty, source, param_or_ret,
                                                /*is_struct_member*/ false)) {
      return false;
    }

    if (auto* str = ty->As<sem::Struct>()) {
      for (auto* member : str->Members()) {
        if (!validate_entry_point_decorations_inner(
                member->Declaration()->decorations, member->Type(),
                member->Declaration()->source, param_or_ret,
                /*is_struct_member*/ true)) {
          AddNote("while analysing entry point '" +
                      builder_->Symbols().NameFor(decl->symbol) + "'",
                  decl->source);
          return false;
        }
      }
    }

    return true;
  };

  for (auto* param : func->Parameters()) {
    auto* param_decl = param->Declaration();
    if (!validate_entry_point_decorations(param_decl->decorations,
                                          param->Type(), param_decl->source,
                                          ParamOrRetType::kParameter)) {
      return false;
    }
  }

  // Clear IO sets after parameter validation. Builtin and location attributes
  // in return types should be validated independently from those used in
  // parameters.
  builtins.clear();
  locations.clear();

  if (!func->ReturnType()->Is<sem::Void>()) {
    if (!validate_entry_point_decorations(decl->return_type_decorations,
                                          func->ReturnType(), decl->source,
                                          ParamOrRetType::kReturnType)) {
      return false;
    }
  }

  if (decl->PipelineStage() == ast::PipelineStage::kVertex &&
      builtins.count(ast::Builtin::kPosition) == 0) {
    // Check module-scope variables, as the SPIR-V sanitizer generates these.
    bool found = false;
    for (auto* global : func->TransitivelyReferencedGlobals()) {
      if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
              global->Declaration()->decorations)) {
        if (builtin->builtin == ast::Builtin::kPosition) {
          found = true;
          break;
        }
      }
    }
    if (!found) {
      AddError(
          "a vertex shader must include the 'position' builtin in its return "
          "type",
          decl->source);
      return false;
    }
  }

  if (decl->PipelineStage() == ast::PipelineStage::kCompute) {
    if (!ast::HasDecoration<ast::WorkgroupDecoration>(decl->decorations)) {
      AddError(
          "a compute shader must include 'workgroup_size' in its "
          "attributes",
          decl->source);
      return false;
    }
  }

  // Validate there are no resource variable binding collisions
  std::unordered_map<sem::BindingPoint, const ast::Variable*> binding_points;
  for (auto* var : func->TransitivelyReferencedGlobals()) {
    auto* var_decl = var->Declaration();
    if (!var_decl->BindingPoint()) {
      continue;
    }
    auto bp = var->BindingPoint();
    auto res = binding_points.emplace(bp, var_decl);
    if (!res.second &&
        IsValidationEnabled(decl->decorations,
                            ast::DisabledValidation::kBindingPointCollision) &&
        IsValidationEnabled(res.first->second->decorations,
                            ast::DisabledValidation::kBindingPointCollision)) {
      // https://gpuweb.github.io/gpuweb/wgsl/#resource-interface
      // Bindings must not alias within a shader stage: two different
      // variables in the resource interface of a given shader must not have
      // the same group and binding values, when considered as a pair of
      // values.
      auto func_name = builder_->Symbols().NameFor(decl->symbol);
      AddError("entry point '" + func_name +
                   "' references multiple variables that use the "
                   "same resource binding [[group(" +
                   std::to_string(bp.group) + "), binding(" +
                   std::to_string(bp.binding) + ")]]",
               var_decl->source);
      AddNote("first resource binding usage declared here",
              res.first->second->source);
      return false;
    }
  }

  return true;
}

sem::Function* Resolver::Function(const ast::Function* decl) {
  variable_stack_.Push();
  TINT_DEFER(variable_stack_.Pop());

  uint32_t parameter_index = 0;
  std::unordered_map<Symbol, Source> parameter_names;
  std::vector<sem::Parameter*> parameters;

  // Resolve all the parameters
  for (auto* param : decl->params) {
    Mark(param);

    {  // Check the parameter name is unique for the function
      auto emplaced = parameter_names.emplace(param->symbol, param->source);
      if (!emplaced.second) {
        auto name = builder_->Symbols().NameFor(param->symbol);
        AddError("redefinition of parameter '" + name + "'", param->source);
        AddNote("previous definition is here", emplaced.first->second);
        return nullptr;
      }
    }

    auto* var = As<sem::Parameter>(
        Variable(param, VariableKind::kParameter, parameter_index++));
    if (!var) {
      return nullptr;
    }

    for (auto* deco : param->decorations) {
      Mark(deco);
    }
    if (!ValidateNoDuplicateDecorations(param->decorations)) {
      return nullptr;
    }

    variable_stack_.Set(param->symbol, var);
    parameters.emplace_back(var);

    auto* var_ty = const_cast<sem::Type*>(var->Type());
    if (auto* str = var_ty->As<sem::Struct>()) {
      switch (decl->PipelineStage()) {
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

  // Resolve the return type
  sem::Type* return_type = nullptr;
  if (auto* ty = decl->return_type) {
    return_type = Type(ty);
    if (!return_type) {
      return nullptr;
    }
  } else {
    return_type = builder_->create<sem::Void>();
  }

  if (auto* str = return_type->As<sem::Struct>()) {
    if (!ApplyStorageClassUsageToType(ast::StorageClass::kNone, str,
                                      decl->source)) {
      AddNote("while instantiating return type for " +
                  builder_->Symbols().NameFor(decl->symbol),
              decl->source);
      return nullptr;
    }

    switch (decl->PipelineStage()) {
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

  sem::WorkgroupSize ws{};
  if (!WorkgroupSizeFor(decl, ws)) {
    return nullptr;
  }

  auto* func =
      builder_->create<sem::Function>(decl, return_type, parameters, ws);
  builder_->Sem().Add(decl, func);

  if (decl->IsEntryPoint()) {
    entry_points_.emplace_back(func);
  }

  TINT_SCOPED_ASSIGNMENT(current_function_, func);

  if (decl->body) {
    Mark(decl->body);
    if (current_compound_statement_) {
      TINT_ICE(Resolver, diagnostics_)
          << "Resolver::Function() called with a current compound statement";
      return nullptr;
    }
    auto* sem_block = builder_->create<sem::FunctionBlockStatement>(func);
    builder_->Sem().Add(decl->body, sem_block);
    if (!Scope(sem_block, [&] { return Statements(decl->body->statements); })) {
      return nullptr;
    }
  }

  for (auto* deco : decl->decorations) {
    Mark(deco);
  }
  if (!ValidateNoDuplicateDecorations(decl->decorations)) {
    return nullptr;
  }

  for (auto* deco : decl->return_type_decorations) {
    Mark(deco);
  }
  if (!ValidateNoDuplicateDecorations(decl->return_type_decorations)) {
    return nullptr;
  }

  if (!ValidateFunction(func)) {
    return nullptr;
  }

  // Register the function information _after_ processing the statements. This
  // allows us to catch a function calling itself when determining the call
  // information as this function doesn't exist until it's finished.
  symbol_to_function_[decl->symbol] = func;

  // If this is an entry point, mark all transitively called functions as being
  // used by this entry point.
  if (decl->IsEntryPoint()) {
    for (auto* f : func->TransitivelyCalledFunctions()) {
      const_cast<sem::Function*>(f)->AddAncestorEntryPoint(func);
    }
  }

  return func;
}

bool Resolver::WorkgroupSizeFor(const ast::Function* func,
                                sem::WorkgroupSize& ws) {
  // Set work-group size defaults.
  for (int i = 0; i < 3; i++) {
    ws[i].value = 1;
    ws[i].overridable_const = nullptr;
  }

  auto* deco = ast::GetDecoration<ast::WorkgroupDecoration>(func->decorations);
  if (!deco) {
    return true;
  }

  auto values = deco->Values();
  auto any_i32 = false;
  auto any_u32 = false;
  for (int i = 0; i < 3; i++) {
    // Each argument to this decoration can either be a literal, an
    // identifier for a module-scope constants, or nullptr if not specified.

    auto* expr = values[i];
    if (!expr) {
      // Not specified, just use the default.
      continue;
    }

    auto* expr_sem = Expression(expr);
    if (!expr_sem) {
      return false;
    }

    constexpr const char* kErrBadType =
        "workgroup_size argument must be either literal or module-scope "
        "constant of type i32 or u32";
    constexpr const char* kErrInconsistentType =
        "workgroup_size arguments must be of the same type, either i32 "
        "or u32";

    auto* ty = TypeOf(expr);
    bool is_i32 = ty->UnwrapRef()->Is<sem::I32>();
    bool is_u32 = ty->UnwrapRef()->Is<sem::U32>();
    if (!is_i32 && !is_u32) {
      AddError(kErrBadType, expr->source);
      return false;
    }

    any_i32 = any_i32 || is_i32;
    any_u32 = any_u32 || is_u32;
    if (any_i32 && any_u32) {
      AddError(kErrInconsistentType, expr->source);
      return false;
    }

    sem::Constant value;

    if (auto* user = Sem(expr)->As<sem::VariableUser>()) {
      // We have an variable of a module-scope constant.
      auto* decl = user->Variable()->Declaration();
      if (!decl->is_const) {
        AddError(kErrBadType, expr->source);
        return false;
      }
      // Capture the constant if an [[override]] attribute is present.
      if (ast::HasDecoration<ast::OverrideDecoration>(decl->decorations)) {
        ws[i].overridable_const = decl;
      }

      if (decl->constructor) {
        value = Sem(decl->constructor)->ConstantValue();
      } else {
        // No constructor means this value must be overriden by the user.
        ws[i].value = 0;
        continue;
      }
    } else if (expr->Is<ast::LiteralExpression>()) {
      value = Sem(expr)->ConstantValue();
    } else {
      AddError(
          "workgroup_size argument must be either a literal or a "
          "module-scope constant",
          values[i]->source);
      return false;
    }

    if (!value) {
      TINT_ICE(Resolver, diagnostics_)
          << "could not resolve constant workgroup_size constant value";
      continue;
    }
    // Validate and set the default value for this dimension.
    if (is_i32 ? value.Elements()[0].i32 < 1 : value.Elements()[0].u32 < 1) {
      AddError("workgroup_size argument must be at least 1", values[i]->source);
      return false;
    }

    ws[i].value = is_i32 ? static_cast<uint32_t>(value.Elements()[0].i32)
                         : value.Elements()[0].u32;
  }
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
  bool unreachable = false;
  for (auto* stmt : stmts) {
    if (unreachable) {
      AddError("code is unreachable", stmt->source);
      return false;
    }

    auto* nested_stmt = stmt;
    while (auto* block = nested_stmt->As<ast::BlockStatement>()) {
      if (block->Empty()) {
        break;
      }
      nested_stmt = block->statements.back();
    }
    if (nested_stmt->IsAnyOf<ast::ReturnStatement, ast::BreakStatement,
                             ast::ContinueStatement, ast::DiscardStatement>()) {
      unreachable = true;
    }
  }
  return true;
}

bool Resolver::Statement(const ast::Statement* stmt) {
  if (stmt->Is<ast::CaseStatement>()) {
    AddError("case statement can only be used inside a switch statement",
             stmt->source);
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
  sem::Statement* sem_statement = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem_statement);
  TINT_SCOPED_ASSIGNMENT(current_statement_, sem_statement);
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return Assignment(a);
  }
  if (stmt->Is<ast::BreakStatement>()) {
    if (!sem_statement->FindFirstParent<sem::LoopBlockStatement>() &&
        !sem_statement->FindFirstParent<sem::SwitchCaseBlockStatement>()) {
      AddError("break statement must be in a loop or switch case",
               stmt->source);
      return false;
    }
    return true;
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    if (!Expression(c->expr)) {
      return false;
    }
    return true;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    // Set if we've hit the first continue statement in our parent loop
    if (auto* block =
            current_block_->FindFirstParent<
                sem::LoopBlockStatement, sem::LoopContinuingBlockStatement>()) {
      if (auto* loop_block = block->As<sem::LoopBlockStatement>()) {
        if (!loop_block->FirstContinue()) {
          const_cast<sem::LoopBlockStatement*>(loop_block)
              ->SetFirstContinue(c, loop_block->Decls().size());
        }
      } else {
        AddError("continuing blocks must not contain a continue statement",
                 stmt->source);
        return false;
      }
    } else {
      AddError("continue statement must be in a loop", stmt->source);
      return false;
    }

    return true;
  }
  if (stmt->Is<ast::DiscardStatement>()) {
    if (auto* continuing =
            sem_statement
                ->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
      AddError("continuing blocks must not contain a discard statement",
               stmt->source);
      if (continuing != sem_statement->Parent()) {
        AddNote("see continuing block here", continuing->Declaration()->source);
      }
      return false;
    }
    current_function_->SetHasDiscard();
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

  AddError("unknown statement type for type determination: " +
               std::string(stmt->TypeInfo().name),
           stmt->source);
  return false;
}

bool Resolver::CaseStatement(const ast::CaseStatement* stmt) {
  auto* sem = builder_->create<sem::SwitchCaseBlockStatement>(
      stmt->body, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  builder_->Sem().Add(stmt->body, sem);
  Mark(stmt->body);
  for (auto* sel : stmt->selectors) {
    Mark(sel);
  }
  return Scope(sem, [&] { return Statements(stmt->body->statements); });
}

bool Resolver::IfStatement(const ast::IfStatement* stmt) {
  auto* sem = builder_->create<sem::IfStatement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (!Expression(stmt->condition)) {
      return false;
    }

    auto* cond_type = TypeOf(stmt->condition)->UnwrapRef();
    if (!cond_type->Is<sem::Bool>()) {
      AddError(
          "if statement condition must be bool, got " + TypeNameOf(cond_type),
          stmt->condition->source);
      return false;
    }

    Mark(stmt->body);
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    builder_->Sem().Add(stmt->body, body);
    if (!Scope(body, [&] { return Statements(stmt->body->statements); })) {
      return false;
    }

    for (auto* else_stmt : stmt->else_statements) {
      Mark(else_stmt);
      if (!ElseStatement(else_stmt)) {
        return false;
      }
    }
    return true;
  });
}

bool Resolver::ElseStatement(const ast::ElseStatement* stmt) {
  auto* sem = builder_->create<sem::ElseStatement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (auto* cond = stmt->condition) {
      if (!Expression(cond)) {
        return false;
      }

      auto* else_cond_type = TypeOf(cond)->UnwrapRef();
      if (!else_cond_type->Is<sem::Bool>()) {
        AddError("else statement condition must be bool, got " +
                     TypeNameOf(else_cond_type),
                 cond->source);
        return false;
      }
    }

    Mark(stmt->body);
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    builder_->Sem().Add(stmt->body, body);
    return Scope(body, [&] { return Statements(stmt->body->statements); });
  });
}

bool Resolver::BlockStatement(const ast::BlockStatement* stmt) {
  auto* sem = builder_->create<sem::BlockStatement>(
      stmt->As<ast::BlockStatement>(), current_compound_statement_,
      current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] { return Statements(stmt->statements); });
}

bool Resolver::LoopStatement(const ast::LoopStatement* stmt) {
  auto* sem = builder_->create<sem::LoopStatement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    Mark(stmt->body);

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    builder_->Sem().Add(stmt->body, body);
    return Scope(body, [&] {
      if (!Statements(stmt->body->statements)) {
        return false;
      }
      if (stmt->continuing) {
        Mark(stmt->continuing);
        if (!stmt->continuing->Empty()) {
          auto* continuing =
              builder_->create<sem::LoopContinuingBlockStatement>(
                  stmt->continuing, current_compound_statement_,
                  current_function_);
          builder_->Sem().Add(stmt->continuing, continuing);
          if (!Scope(continuing, [&] {
                return Statements(stmt->continuing->statements);
              })) {
            return false;
          }
        }
      }
      return true;
    });
  });
}

bool Resolver::ForLoopStatement(const ast::ForLoopStatement* stmt) {
  auto* sem = builder_->create<sem::ForLoopStatement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (auto* initializer = stmt->initializer) {
      Mark(initializer);
      if (!Statement(initializer)) {
        return false;
      }
    }

    if (auto* condition = stmt->condition) {
      if (!Expression(condition)) {
        return false;
      }

      auto* cond_ty = TypeOf(condition)->UnwrapRef();
      if (!cond_ty->Is<sem::Bool>()) {
        AddError("for-loop condition must be bool, got " + TypeNameOf(cond_ty),
                 condition->source);
        return false;
      }
    }

    if (auto* continuing = stmt->continuing) {
      Mark(continuing);
      if (!Statement(continuing)) {
        return false;
      }
    }

    Mark(stmt->body);

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    builder_->Sem().Add(stmt->body, body);
    return Scope(body, [&] { return Statements(stmt->body->statements); });
  });
}

sem::Expression* Resolver::Expression(const ast::Expression* root) {
  std::vector<const ast::Expression*> sorted;
  if (!ast::TraverseExpressions<ast::TraverseOrder::RightToLeft>(
          root, diagnostics_, [&](const ast::Expression* expr) {
            Mark(expr);
            sorted.emplace_back(expr);
            return ast::TraverseAction::Descend;
          })) {
    return nullptr;
  }

  for (auto* expr : utils::Reverse(sorted)) {
    sem::Expression* sem_expr = nullptr;
    if (auto* array = expr->As<ast::IndexAccessorExpression>()) {
      sem_expr = IndexAccessor(array);
    } else if (auto* bin_op = expr->As<ast::BinaryExpression>()) {
      sem_expr = Binary(bin_op);
    } else if (auto* bitcast = expr->As<ast::BitcastExpression>()) {
      sem_expr = Bitcast(bitcast);
    } else if (auto* call = expr->As<ast::CallExpression>()) {
      sem_expr = Call(call);
    } else if (auto* ident = expr->As<ast::IdentifierExpression>()) {
      sem_expr = Identifier(ident);
    } else if (auto* literal = expr->As<ast::LiteralExpression>()) {
      sem_expr = Literal(literal);
    } else if (auto* member = expr->As<ast::MemberAccessorExpression>()) {
      sem_expr = MemberAccessor(member);
    } else if (auto* unary = expr->As<ast::UnaryOpExpression>()) {
      sem_expr = UnaryOp(unary);
    } else if (expr->Is<ast::PhonyExpression>()) {
      sem_expr = builder_->create<sem::Expression>(
          expr, builder_->create<sem::Void>(), current_statement_,
          sem::Constant{});
    } else {
      TINT_ICE(Resolver, diagnostics_)
          << "unhandled expression type: " << expr->TypeInfo().name;
      return nullptr;
    }
    if (!sem_expr) {
      return nullptr;
    }
    builder_->Sem().Add(expr, sem_expr);
    if (expr == root) {
      return sem_expr;
    }
  }

  TINT_ICE(Resolver, diagnostics_) << "Expression() did not find root node";
  return nullptr;
}

sem::Expression* Resolver::IndexAccessor(
    const ast::IndexAccessorExpression* expr) {
  auto* idx = expr->index;
  auto* parent_raw_ty = TypeOf(expr->object);
  auto* parent_ty = parent_raw_ty->UnwrapRef();
  const sem::Type* ty = nullptr;
  if (auto* arr = parent_ty->As<sem::Array>()) {
    ty = arr->ElemType();
  } else if (auto* vec = parent_ty->As<sem::Vector>()) {
    ty = vec->type();
  } else if (auto* mat = parent_ty->As<sem::Matrix>()) {
    ty = builder_->create<sem::Vector>(mat->type(), mat->rows());
  } else {
    AddError("cannot index type '" + TypeNameOf(parent_ty) + "'", expr->source);
    return nullptr;
  }

  auto* idx_ty = TypeOf(idx)->UnwrapRef();
  if (!idx_ty->IsAnyOf<sem::I32, sem::U32>()) {
    AddError("index must be of type 'i32' or 'u32', found: '" +
                 TypeNameOf(idx_ty) + "'",
             idx->source);
    return nullptr;
  }

  if (parent_ty->IsAnyOf<sem::Array, sem::Matrix>()) {
    if (!parent_raw_ty->Is<sem::Reference>()) {
      // TODO(bclayton): expand this to allow any const_expr expression
      // https://github.com/gpuweb/gpuweb/issues/1272
      if (!idx->As<ast::IntLiteralExpression>()) {
        AddError("index must be signed or unsigned integer literal",
                 idx->source);
        return nullptr;
      }
    }
  }

  // If we're extracting from a reference, we return a reference.
  if (auto* ref = parent_raw_ty->As<sem::Reference>()) {
    ty = builder_->create<sem::Reference>(ty, ref->StorageClass(),
                                          ref->Access());
  }

  auto val = EvaluateConstantValue(expr, ty);
  return builder_->create<sem::Expression>(expr, ty, current_statement_, val);
}

sem::Expression* Resolver::Bitcast(const ast::BitcastExpression* expr) {
  auto* ty = Type(expr->type);
  if (!ty) {
    return nullptr;
  }
  if (ty->Is<sem::Pointer>()) {
    AddError("cannot cast to a pointer", expr->source);
    return nullptr;
  }

  auto val = EvaluateConstantValue(expr, ty);
  return builder_->create<sem::Expression>(expr, ty, current_statement_, val);
}

sem::Call* Resolver::Call(const ast::CallExpression* expr) {
  std::vector<const sem::Expression*> args(expr->args.size());
  std::vector<const sem::Type*> arg_tys(args.size());
  for (size_t i = 0; i < expr->args.size(); i++) {
    auto* arg = Sem(expr->args[i]);
    if (!arg) {
      return nullptr;
    }
    args[i] = arg;
    arg_tys[i] = args[i]->Type();
  }

  auto type_ctor_or_conv = [&](const sem::Type* ty) -> sem::Call* {
    // The call has resolved to a type constructor or cast.
    if (args.size() == 1) {
      auto* target = ty;
      auto* source = args[0]->Type()->UnwrapRef();
      if ((source != target) &&  //
          ((source->is_scalar() && target->is_scalar()) ||
           (source->Is<sem::Vector>() && target->Is<sem::Vector>()) ||
           (source->Is<sem::Matrix>() && target->Is<sem::Matrix>()))) {
        // Note: Matrix types currently cannot be converted (the element type
        // must only be f32). We implement this for the day we support other
        // matrix element types.
        return TypeConversion(expr, ty, args[0], arg_tys[0]);
      }
    }
    return TypeConstructor(expr, ty, std::move(args), std::move(arg_tys));
  };

  // Resolve the target of the CallExpression to determine whether this is a
  // function call, cast or type constructor expression.
  if (expr->target.type) {
    auto* ty = Type(expr->target.type);
    if (!ty) {
      return nullptr;
    }
    return type_ctor_or_conv(ty);
  }

  auto* ident = expr->target.name;
  Mark(ident);

  auto it = named_type_info_.find(ident->symbol);
  if (it != named_type_info_.end()) {
    // We have a type.
    return type_ctor_or_conv(it->second.sem);
  }

  // Not a type, treat as a intrinsic / function call.
  auto name = builder_->Symbols().NameFor(ident->symbol);
  auto intrinsic_type = sem::ParseIntrinsicType(name);
  auto* call = (intrinsic_type != IntrinsicType::kNone)
                   ? IntrinsicCall(expr, intrinsic_type, std::move(args),
                                   std::move(arg_tys))
                   : FunctionCall(expr, std::move(args));

  current_function_->AddDirectCall(call);
  return call;
}

sem::Call* Resolver::IntrinsicCall(
    const ast::CallExpression* expr,
    sem::IntrinsicType intrinsic_type,
    const std::vector<const sem::Expression*> args,
    const std::vector<const sem::Type*> arg_tys) {
  auto* intrinsic = intrinsic_table_->Lookup(intrinsic_type, std::move(arg_tys),
                                             expr->source);
  if (!intrinsic) {
    return nullptr;
  }

  if (intrinsic->IsDeprecated()) {
    AddWarning("use of deprecated intrinsic", expr->source);
  }

  auto* call = builder_->create<sem::Call>(expr, intrinsic, std::move(args),
                                           current_statement_, sem::Constant{});

  current_function_->AddDirectlyCalledIntrinsic(intrinsic);

  if (IsTextureIntrinsic(intrinsic_type) &&
      !ValidateTextureIntrinsicFunction(call)) {
    return nullptr;
  }

  if (!ValidateIntrinsicCall(call)) {
    return nullptr;
  }

  return call;
}

bool Resolver::ValidateIntrinsicCall(const sem::Call* call) {
  if (call->Type()->Is<sem::Void>()) {
    bool is_call_statement = false;
    if (auto* call_stmt = As<ast::CallStatement>(call->Stmt()->Declaration())) {
      if (call_stmt->expr == call->Declaration()) {
        is_call_statement = true;
      }
    }
    if (!is_call_statement) {
      // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
      // If the called function does not return a value, a function call
      // statement should be used instead.
      auto* ident = call->Declaration()->target.name;
      auto name = builder_->Symbols().NameFor(ident->symbol);
      AddError("intrinsic '" + name + "' does not return a value",
               call->Declaration()->source);
      return false;
    }
  }

  return true;
}

sem::Call* Resolver::FunctionCall(
    const ast::CallExpression* expr,
    const std::vector<const sem::Expression*> args) {
  auto sym = expr->target.name->symbol;
  auto name = builder_->Symbols().NameFor(sym);

  auto target_it = symbol_to_function_.find(sym);
  if (target_it == symbol_to_function_.end()) {
    if (current_function_ && current_function_->Declaration()->symbol == sym) {
      AddError("recursion is not permitted. '" + name +
                   "' attempted to call itself.",
               expr->source);
    } else {
      AddError("unable to find called function: " + name, expr->source);
    }
    return nullptr;
  }
  auto* target = target_it->second;
  auto* call = builder_->create<sem::Call>(expr, target, std::move(args),
                                           current_statement_, sem::Constant{});

  if (current_function_) {
    target->AddCallSite(call);

    // Note: Requires called functions to be resolved first.
    // This is currently guaranteed as functions must be declared before
    // use.
    current_function_->AddTransitivelyCalledFunction(target);
    for (auto* transitive_call : target->TransitivelyCalledFunctions()) {
      current_function_->AddTransitivelyCalledFunction(transitive_call);
    }

    // We inherit any referenced variables from the callee.
    for (auto* var : target->TransitivelyReferencedGlobals()) {
      current_function_->AddTransitivelyReferencedGlobal(var);
    }
  }

  if (!ValidateFunctionCall(call)) {
    return nullptr;
  }

  return call;
}

bool Resolver::ValidateTextureIntrinsicFunction(const sem::Call* call) {
  auto* intrinsic = call->Target()->As<sem::Intrinsic>();
  if (!intrinsic) {
    return false;
  }
  std::string func_name = intrinsic->str();
  auto& signature = intrinsic->Signature();
  auto index = signature.IndexOf(sem::ParameterUsage::kOffset);
  if (index > -1) {
    auto* arg = call->Arguments()[index];
    if (auto values = arg->ConstantValue()) {
      // Assert that the constant values are of the expected type.
      if (!values.Type()->Is<sem::Vector>() ||
          !values.ElementType()->Is<sem::I32>()) {
        TINT_ICE(Resolver, diagnostics_)
            << "failed to resolve '" + func_name + "' offset parameter type";
        return false;
      }

      // Currently const_expr is restricted to literals and type constructors.
      // Check that that's all we have for the offset parameter.
      bool is_const_expr = true;
      ast::TraverseExpressions(
          arg->Declaration(), diagnostics_, [&](const ast::Expression* e) {
            if (e->IsAnyOf<ast::LiteralExpression, ast::CallExpression>()) {
              return ast::TraverseAction::Descend;
            }
            is_const_expr = false;
            return ast::TraverseAction::Stop;
          });
      if (is_const_expr) {
        for (auto offset : values.Elements()) {
          auto offset_value = offset.i32;
          if (offset_value < -8 || offset_value > 7) {
            AddError("each offset component of '" + func_name +
                         "' must be at least -8 and at most 7. "
                         "found: '" +
                         std::to_string(offset_value) + "'",
                     arg->Declaration()->source);
            return false;
          }
        }
        return true;
      }
    }
    AddError("'" + func_name + "' offset parameter must be a const_expression",
             arg->Declaration()->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateFunctionCall(const sem::Call* call) {
  auto* decl = call->Declaration();
  auto* target = call->Target()->As<sem::Function>();
  auto sym = decl->target.name->symbol;
  auto name = builder_->Symbols().NameFor(sym);

  if (target->Declaration()->IsEntryPoint()) {
    // https://www.w3.org/TR/WGSL/#function-restriction
    // An entry point must never be the target of a function call.
    AddError("entry point functions cannot be the target of a function call",
             decl->source);
    return false;
  }

  if (decl->args.size() != target->Parameters().size()) {
    bool more = decl->args.size() > target->Parameters().size();
    AddError("too " + (more ? std::string("many") : std::string("few")) +
                 " arguments in call to '" + name + "', expected " +
                 std::to_string(target->Parameters().size()) + ", got " +
                 std::to_string(call->Arguments().size()),
             decl->source);
    return false;
  }

  for (size_t i = 0; i < call->Arguments().size(); ++i) {
    const sem::Variable* param = target->Parameters()[i];
    const ast::Expression* arg_expr = decl->args[i];
    auto* param_type = param->Type();
    auto* arg_type = TypeOf(arg_expr)->UnwrapRef();

    if (param_type != arg_type) {
      AddError("type mismatch for argument " + std::to_string(i + 1) +
                   " in call to '" + name + "', expected '" +
                   TypeNameOf(param_type) + "', got '" + TypeNameOf(arg_type) +
                   "'",
               arg_expr->source);
      return false;
    }

    if (param_type->Is<sem::Pointer>()) {
      auto is_valid = false;
      if (auto* ident_expr = arg_expr->As<ast::IdentifierExpression>()) {
        auto* var = variable_stack_.Get(ident_expr->symbol);
        if (!var) {
          TINT_ICE(Resolver, diagnostics_) << "failed to resolve identifier";
          return false;
        }
        if (var->Is<sem::Parameter>()) {
          is_valid = true;
        }
      } else if (auto* unary = arg_expr->As<ast::UnaryOpExpression>()) {
        if (unary->op == ast::UnaryOp::kAddressOf) {
          if (auto* ident_unary =
                  unary->expr->As<ast::IdentifierExpression>()) {
            auto* var = variable_stack_.Get(ident_unary->symbol);
            if (!var) {
              TINT_ICE(Resolver, diagnostics_)
                  << "failed to resolve identifier";
              return false;
            }
            if (var->Declaration()->is_const) {
              TINT_ICE(Resolver, diagnostics_)
                  << "Resolver::FunctionCall() encountered an address-of "
                     "expression of a constant identifier expression";
              return false;
            }
            is_valid = true;
          }
        }
      }

      if (!is_valid &&
          IsValidationEnabled(
              param->Declaration()->decorations,
              ast::DisabledValidation::kIgnoreInvalidPointerArgument)) {
        AddError(
            "expected an address-of expression of a variable identifier "
            "expression or a function parameter",
            arg_expr->source);
        return false;
      }
    }
  }

  if (call->Type()->Is<sem::Void>()) {
    bool is_call_statement = false;
    if (auto* call_stmt = As<ast::CallStatement>(call->Stmt()->Declaration())) {
      if (call_stmt->expr == call->Declaration()) {
        is_call_statement = true;
      }
    }
    if (!is_call_statement) {
      // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
      // If the called function does not return a value, a function call
      // statement should be used instead.
      AddError("function '" + name + "' does not return a value", decl->source);
      return false;
    }
  }
  return true;
}

sem::Call* Resolver::TypeConversion(const ast::CallExpression* expr,
                                    const sem::Type* target,
                                    const sem::Expression* arg,
                                    const sem::Type* source) {
  // It is not valid to have a type-cast call expression inside a call
  // statement.
  if (current_statement_) {
    if (auto* stmt =
            current_statement_->Declaration()->As<ast::CallStatement>()) {
      if (stmt->expr == expr) {
        AddError("type cast evaluated but not used", expr->source);
        return nullptr;
      }
    }
  }

  auto* call_target = utils::GetOrCreate(
      type_conversions_, TypeConversionSig{target, source},
      [&]() -> sem::TypeConversion* {
        // Now that the argument types have been determined, make sure that they
        // obey the conversion rules laid out in
        // https://gpuweb.github.io/gpuweb/wgsl/#conversion-expr.
        bool ok = true;
        if (auto* vec_type = target->As<sem::Vector>()) {
          ok = ValidateVectorConstructorOrCast(expr, vec_type);
        } else if (auto* mat_type = target->As<sem::Matrix>()) {
          // Note: Matrix types currently cannot be converted (the element type
          // must only be f32). We implement this for the day we support other
          // matrix element types.
          ok = ValidateMatrixConstructorOrCast(expr, mat_type);
        } else if (target->is_scalar()) {
          ok = ValidateScalarConstructorOrCast(expr, target);
        } else if (auto* arr_type = target->As<sem::Array>()) {
          ok = ValidateArrayConstructorOrCast(expr, arr_type);
        } else if (auto* struct_type = target->As<sem::Struct>()) {
          ok = ValidateStructureConstructorOrCast(expr, struct_type);
        } else {
          AddError("type is not constructible", expr->source);
          return nullptr;
        }
        if (!ok) {
          return nullptr;
        }

        auto* param = builder_->create<sem::Parameter>(
            nullptr,                   // declaration
            0,                         // index
            source->UnwrapRef(),       // type
            ast::StorageClass::kNone,  // storage_class
            ast::Access::kUndefined);  // access
        return builder_->create<sem::TypeConversion>(target, param);
      });

  if (!call_target) {
    return nullptr;
  }

  auto val = EvaluateConstantValue(expr, target);
  return builder_->create<sem::Call>(expr, call_target,
                                     std::vector<const sem::Expression*>{arg},
                                     current_statement_, val);
}

sem::Call* Resolver::TypeConstructor(
    const ast::CallExpression* expr,
    const sem::Type* ty,
    const std::vector<const sem::Expression*> args,
    const std::vector<const sem::Type*> arg_tys) {
  // It is not valid to have a type-constructor call expression as a call
  // statement.
  if (current_statement_) {
    if (auto* stmt =
            current_statement_->Declaration()->As<ast::CallStatement>()) {
      if (stmt->expr == expr) {
        AddError("type constructor evaluated but not used", expr->source);
        return nullptr;
      }
    }
  }

  auto* call_target = utils::GetOrCreate(
      type_ctors_, TypeConstructorSig{ty, arg_tys},
      [&]() -> sem::TypeConstructor* {
        // Now that the argument types have been determined, make sure that they
        // obey the constructor type rules laid out in
        // https://gpuweb.github.io/gpuweb/wgsl/#type-constructor-expr.
        bool ok = true;
        if (auto* vec_type = ty->As<sem::Vector>()) {
          ok = ValidateVectorConstructorOrCast(expr, vec_type);
        } else if (auto* mat_type = ty->As<sem::Matrix>()) {
          ok = ValidateMatrixConstructorOrCast(expr, mat_type);
        } else if (ty->is_scalar()) {
          ok = ValidateScalarConstructorOrCast(expr, ty);
        } else if (auto* arr_type = ty->As<sem::Array>()) {
          ok = ValidateArrayConstructorOrCast(expr, arr_type);
        } else if (auto* struct_type = ty->As<sem::Struct>()) {
          ok = ValidateStructureConstructorOrCast(expr, struct_type);
        } else {
          AddError("type is not constructible", expr->source);
          return nullptr;
        }
        if (!ok) {
          return nullptr;
        }

        return builder_->create<sem::TypeConstructor>(
            ty, utils::Transform(
                    arg_tys,
                    [&](const sem::Type* t, size_t i) -> const sem::Parameter* {
                      return builder_->create<sem::Parameter>(
                          nullptr,                   // declaration
                          i,                         // index
                          t->UnwrapRef(),            // type
                          ast::StorageClass::kNone,  // storage_class
                          ast::Access::kUndefined);  // access
                    }));
      });

  if (!call_target) {
    return nullptr;
  }

  auto val = EvaluateConstantValue(expr, ty);
  return builder_->create<sem::Call>(expr, call_target, std::move(args),
                                     current_statement_, val);
}

sem::Expression* Resolver::Literal(const ast::LiteralExpression* literal) {
  auto* ty = TypeOf(literal);
  if (!ty) {
    return nullptr;
  }

  auto val = EvaluateConstantValue(literal, ty);
  return builder_->create<sem::Expression>(literal, ty, current_statement_,
                                           val);
}

bool Resolver::ValidateStructureConstructorOrCast(
    const ast::CallExpression* ctor,
    const sem::Struct* struct_type) {
  if (!struct_type->IsConstructible()) {
    AddError("struct constructor has non-constructible type", ctor->source);
    return false;
  }

  if (ctor->args.size() > 0) {
    if (ctor->args.size() != struct_type->Members().size()) {
      std::string fm =
          ctor->args.size() < struct_type->Members().size() ? "few" : "many";
      AddError("struct constructor has too " + fm + " inputs: expected " +
                   std::to_string(struct_type->Members().size()) + ", found " +
                   std::to_string(ctor->args.size()),
               ctor->source);
      return false;
    }
    for (auto* member : struct_type->Members()) {
      auto* value = ctor->args[member->Index()];
      auto* value_ty = TypeOf(value);
      if (member->Type() != value_ty->UnwrapRef()) {
        AddError(
            "type in struct constructor does not match struct member type: "
            "expected '" +
                TypeNameOf(member->Type()) + "', found '" +
                TypeNameOf(value_ty) + "'",
            value->source);
        return false;
      }
    }
  }
  return true;
}

bool Resolver::ValidateArrayConstructorOrCast(const ast::CallExpression* ctor,
                                              const sem::Array* array_type) {
  auto& values = ctor->args;
  auto* elem_ty = array_type->ElemType();
  for (auto* value : values) {
    auto* value_ty = TypeOf(value)->UnwrapRef();
    if (value_ty != elem_ty) {
      AddError(
          "type in array constructor does not match array type: "
          "expected '" +
              TypeNameOf(elem_ty) + "', found '" + TypeNameOf(value_ty) + "'",
          value->source);
      return false;
    }
  }

  if (array_type->IsRuntimeSized()) {
    AddError("cannot init a runtime-sized array", ctor->source);
    return false;
  } else if (!elem_ty->IsConstructible()) {
    AddError("array constructor has non-constructible element type",
             ctor->source);
    return false;
  } else if (!values.empty() && (values.size() != array_type->Count())) {
    std::string fm = values.size() < array_type->Count() ? "few" : "many";
    AddError("array constructor has too " + fm + " elements: expected " +
                 std::to_string(array_type->Count()) + ", found " +
                 std::to_string(values.size()),
             ctor->source);
    return false;
  } else if (values.size() > array_type->Count()) {
    AddError("array constructor has too many elements: expected " +
                 std::to_string(array_type->Count()) + ", found " +
                 std::to_string(values.size()),
             ctor->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateVectorConstructorOrCast(const ast::CallExpression* ctor,
                                               const sem::Vector* vec_type) {
  auto& values = ctor->args;
  auto* elem_ty = vec_type->type();
  size_t value_cardinality_sum = 0;
  for (auto* value : values) {
    auto* value_ty = TypeOf(value)->UnwrapRef();
    if (value_ty->is_scalar()) {
      if (elem_ty != value_ty) {
        AddError(
            "type in vector constructor does not match vector type: "
            "expected '" +
                TypeNameOf(elem_ty) + "', found '" + TypeNameOf(value_ty) + "'",
            value->source);
        return false;
      }

      value_cardinality_sum++;
    } else if (auto* value_vec = value_ty->As<sem::Vector>()) {
      auto* value_elem_ty = value_vec->type();
      // A mismatch of vector type parameter T is only an error if multiple
      // arguments are present. A single argument constructor constitutes a
      // type conversion expression.
      if (elem_ty != value_elem_ty && values.size() > 1u) {
        AddError(
            "type in vector constructor does not match vector type: "
            "expected '" +
                TypeNameOf(elem_ty) + "', found '" + TypeNameOf(value_elem_ty) +
                "'",
            value->source);
        return false;
      }

      value_cardinality_sum += value_vec->Width();
    } else {
      // A vector constructor can only accept vectors and scalars.
      AddError("expected vector or scalar type in vector constructor; found: " +
                   TypeNameOf(value_ty),
               value->source);
      return false;
    }
  }

  // A correct vector constructor must either be a zero-value expression,
  // a single-value initializer (splat) expression, or the number of components
  // of all constructor arguments must add up to the vector cardinality.
  if (value_cardinality_sum > 1 && value_cardinality_sum != vec_type->Width()) {
    if (values.empty()) {
      TINT_ICE(Resolver, diagnostics_)
          << "constructor arguments expected to be non-empty!";
    }
    const Source& values_start = values[0]->source;
    const Source& values_end = values[values.size() - 1]->source;
    AddError("attempted to construct '" + TypeNameOf(vec_type) + "' with " +
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

bool Resolver::ValidateMatrixConstructorOrCast(const ast::CallExpression* ctor,
                                               const sem::Matrix* matrix_ty) {
  auto& values = ctor->args;
  // Zero Value expression
  if (values.empty()) {
    return true;
  }

  if (!ValidateMatrix(matrix_ty, ctor->source)) {
    return false;
  }

  auto* elem_type = matrix_ty->type();
  auto num_elements = matrix_ty->columns() * matrix_ty->rows();

  // Print a generic error for an invalid matrix constructor, showing the
  // available overloads.
  auto print_error = [&]() {
    const Source& values_start = values[0]->source;
    const Source& values_end = values[values.size() - 1]->source;
    auto type_name = TypeNameOf(matrix_ty);
    auto elem_type_name = TypeNameOf(elem_type);
    std::stringstream ss;
    ss << "invalid constructor for " + type_name << std::endl << std::endl;
    ss << "3 candidates available:" << std::endl;
    ss << "  " << type_name << "()" << std::endl;
    ss << "  " << type_name << "(" << elem_type_name << ",...,"
       << elem_type_name << ")"
       << " // " << std::to_string(num_elements) << " arguments" << std::endl;
    ss << "  " << type_name << "(";
    for (uint32_t c = 0; c < matrix_ty->columns(); c++) {
      if (c > 0) {
        ss << ", ";
      }
      ss << VectorPretty(matrix_ty->rows(), elem_type);
    }
    ss << ")" << std::endl;
    AddError(ss.str(), Source::Combine(values_start, values_end));
  };

  const sem::Type* expected_arg_type = nullptr;
  if (num_elements == values.size()) {
    // Column-major construction from scalar elements.
    expected_arg_type = matrix_ty->type();
  } else if (matrix_ty->columns() == values.size()) {
    // Column-by-column construction from vectors.
    expected_arg_type = matrix_ty->ColumnType();
  } else {
    print_error();
    return false;
  }

  for (auto* value : values) {
    if (TypeOf(value)->UnwrapRef() != expected_arg_type) {
      print_error();
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateScalarConstructorOrCast(const ast::CallExpression* ctor,
                                               const sem::Type* ty) {
  if (ctor->args.size() == 0) {
    return true;
  }
  if (ctor->args.size() > 1) {
    AddError("expected zero or one value in constructor, got " +
                 std::to_string(ctor->args.size()),
             ctor->source);
    return false;
  }

  // Validate constructor
  auto* value = ctor->args[0];
  auto* value_ty = TypeOf(value)->UnwrapRef();

  using Bool = sem::Bool;
  using I32 = sem::I32;
  using U32 = sem::U32;
  using F32 = sem::F32;

  const bool is_valid = (ty->Is<Bool>() && value_ty->is_scalar()) ||
                        (ty->Is<I32>() && value_ty->is_scalar()) ||
                        (ty->Is<U32>() && value_ty->is_scalar()) ||
                        (ty->Is<F32>() && value_ty->is_scalar());
  if (!is_valid) {
    AddError("cannot construct '" + TypeNameOf(ty) +
                 "' with a value of type '" + TypeNameOf(value_ty) + "'",
             ctor->source);

    return false;
  }

  return true;
}

sem::Expression* Resolver::Identifier(const ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol;
  if (auto* var = variable_stack_.Get(symbol)) {
    auto* user =
        builder_->create<sem::VariableUser>(expr, current_statement_, var);

    if (current_statement_) {
      // If identifier is part of a loop continuing block, make sure it
      // doesn't refer to a variable that is bypassed by a continue statement
      // in the loop's body block.
      if (auto* continuing_block =
              current_statement_
                  ->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
        auto* loop_block =
            continuing_block->FindFirstParent<sem::LoopBlockStatement>();
        if (loop_block->FirstContinue()) {
          auto& decls = loop_block->Decls();
          // If our identifier is in loop_block->decls, make sure its index is
          // less than first_continue
          auto iter =
              std::find_if(decls.begin(), decls.end(),
                           [&symbol](auto* v) { return v->symbol == symbol; });
          if (iter != decls.end()) {
            auto var_decl_index =
                static_cast<size_t>(std::distance(decls.begin(), iter));
            if (var_decl_index >= loop_block->NumDeclsAtFirstContinue()) {
              AddError("continue statement bypasses declaration of '" +
                           builder_->Symbols().NameFor(symbol) + "'",
                       loop_block->FirstContinue()->source);
              AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                          "' declared here",
                      (*iter)->source);
              AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                          "' referenced in continuing block here",
                      expr->source);
              return nullptr;
            }
          }
        }
      }
    }

    if (current_function_) {
      if (auto* global = var->As<sem::GlobalVariable>()) {
        current_function_->AddDirectlyReferencedGlobal(global);
      }
    }

    var->AddUser(user);
    return user;
  }

  if (symbol_to_function_.count(symbol)) {
    AddError("missing '(' for function call", expr->source.End());
    return nullptr;
  }

  std::string name = builder_->Symbols().NameFor(symbol);
  if (sem::ParseIntrinsicType(name) != IntrinsicType::kNone) {
    AddError("missing '(' for intrinsic call", expr->source.End());
    return nullptr;
  }

  AddError("identifier must be declared before use: " + name, expr->source);
  return nullptr;
}

sem::Expression* Resolver::MemberAccessor(
    const ast::MemberAccessorExpression* expr) {
  auto* structure = TypeOf(expr->structure);
  auto* storage_ty = structure->UnwrapRef();

  const sem::Type* ret = nullptr;
  std::vector<uint32_t> swizzle;

  if (auto* str = storage_ty->As<sem::Struct>()) {
    Mark(expr->member);
    auto symbol = expr->member->symbol;

    const sem::StructMember* member = nullptr;
    for (auto* m : str->Members()) {
      if (m->Name() == symbol) {
        ret = m->Type();
        member = m;
        break;
      }
    }

    if (ret == nullptr) {
      AddError(
          "struct member " + builder_->Symbols().NameFor(symbol) + " not found",
          expr->source);
      return nullptr;
    }

    // If we're extracting from a reference, we return a reference.
    if (auto* ref = structure->As<sem::Reference>()) {
      ret = builder_->create<sem::Reference>(ret, ref->StorageClass(),
                                             ref->Access());
    }

    return builder_->create<sem::StructMemberAccess>(
        expr, ret, current_statement_, member);
  }

  if (auto* vec = storage_ty->As<sem::Vector>()) {
    Mark(expr->member);
    std::string s = builder_->Symbols().NameFor(expr->member->symbol);
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
                   expr->member->source.Begin() + swizzle.size());
          return nullptr;
      }

      if (swizzle.back() >= vec->Width()) {
        AddError("invalid vector swizzle member", expr->member->source);
        return nullptr;
      }
    }

    if (size < 1 || size > 4) {
      AddError("invalid vector swizzle size", expr->member->source);
      return nullptr;
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
               expr->member->source);
      return nullptr;
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
    return builder_->create<sem::Swizzle>(expr, ret, current_statement_,
                                          std::move(swizzle));
  }

  AddError(
      "invalid member accessor expression. Expected vector or struct, got '" +
          TypeNameOf(storage_ty) + "'",
      expr->structure->source);
  return nullptr;
}

sem::Expression* Resolver::Binary(const ast::BinaryExpression* expr) {
  using Bool = sem::Bool;
  using F32 = sem::F32;
  using I32 = sem::I32;
  using U32 = sem::U32;
  using Matrix = sem::Matrix;
  using Vector = sem::Vector;

  auto* lhs_ty = TypeOf(expr->lhs)->UnwrapRef();
  auto* rhs_ty = TypeOf(expr->rhs)->UnwrapRef();

  auto* lhs_vec = lhs_ty->As<Vector>();
  auto* lhs_vec_elem_type = lhs_vec ? lhs_vec->type() : nullptr;
  auto* rhs_vec = rhs_ty->As<Vector>();
  auto* rhs_vec_elem_type = rhs_vec ? rhs_vec->type() : nullptr;

  const bool matching_vec_elem_types =
      lhs_vec_elem_type && rhs_vec_elem_type &&
      (lhs_vec_elem_type == rhs_vec_elem_type) &&
      (lhs_vec->Width() == rhs_vec->Width());

  const bool matching_types = matching_vec_elem_types || (lhs_ty == rhs_ty);

  auto build = [&](const sem::Type* ty) {
    auto val = EvaluateConstantValue(expr, ty);
    return builder_->create<sem::Expression>(expr, ty, current_statement_, val);
  };

  // Binary logical expressions
  if (expr->IsLogicalAnd() || expr->IsLogicalOr()) {
    if (matching_types && lhs_ty->Is<Bool>()) {
      return build(lhs_ty);
    }
  }
  if (expr->IsOr() || expr->IsAnd()) {
    if (matching_types && lhs_ty->Is<Bool>()) {
      return build(lhs_ty);
    }
    if (matching_types && lhs_vec_elem_type && lhs_vec_elem_type->Is<Bool>()) {
      return build(lhs_ty);
    }
  }

  // Arithmetic expressions
  if (expr->IsArithmetic()) {
    // Binary arithmetic expressions over scalars
    if (matching_types && lhs_ty->is_numeric_scalar()) {
      return build(lhs_ty);
    }

    // Binary arithmetic expressions over vectors
    if (matching_types && lhs_vec_elem_type &&
        lhs_vec_elem_type->is_numeric_scalar()) {
      return build(lhs_ty);
    }

    // Binary arithmetic expressions with mixed scalar and vector operands
    if (lhs_vec_elem_type && (lhs_vec_elem_type == rhs_ty)) {
      if (expr->IsModulo()) {
        if (rhs_ty->is_integer_scalar()) {
          return build(lhs_ty);
        }
      } else if (rhs_ty->is_numeric_scalar()) {
        return build(lhs_ty);
      }
    }
    if (rhs_vec_elem_type && (rhs_vec_elem_type == lhs_ty)) {
      if (expr->IsModulo()) {
        if (lhs_ty->is_integer_scalar()) {
          return build(rhs_ty);
        }
      } else if (lhs_ty->is_numeric_scalar()) {
        return build(rhs_ty);
      }
    }
  }

  // Matrix arithmetic
  auto* lhs_mat = lhs_ty->As<Matrix>();
  auto* lhs_mat_elem_type = lhs_mat ? lhs_mat->type() : nullptr;
  auto* rhs_mat = rhs_ty->As<Matrix>();
  auto* rhs_mat_elem_type = rhs_mat ? rhs_mat->type() : nullptr;
  // Addition and subtraction of float matrices
  if ((expr->IsAdd() || expr->IsSubtract()) && lhs_mat_elem_type &&
      lhs_mat_elem_type->Is<F32>() && rhs_mat_elem_type &&
      rhs_mat_elem_type->Is<F32>() &&
      (lhs_mat->columns() == rhs_mat->columns()) &&
      (lhs_mat->rows() == rhs_mat->rows())) {
    return build(rhs_ty);
  }
  if (expr->IsMultiply()) {
    // Multiplication of a matrix and a scalar
    if (lhs_ty->Is<F32>() && rhs_mat_elem_type &&
        rhs_mat_elem_type->Is<F32>()) {
      return build(rhs_ty);
    }
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_ty->Is<F32>()) {
      return build(lhs_ty);
    }

    // Vector times matrix
    if (lhs_vec_elem_type && lhs_vec_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_vec->Width() == rhs_mat->rows())) {
      return build(
          builder_->create<sem::Vector>(lhs_vec->type(), rhs_mat->columns()));
    }

    // Matrix times vector
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_vec->Width())) {
      return build(
          builder_->create<sem::Vector>(rhs_vec->type(), lhs_mat->rows()));
    }

    // Matrix times matrix
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_mat_elem_type && rhs_mat_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_mat->rows())) {
      return build(builder_->create<sem::Matrix>(
          builder_->create<sem::Vector>(lhs_mat_elem_type, lhs_mat->rows()),
          rhs_mat->columns()));
    }
  }

  // Comparison expressions
  if (expr->IsComparison()) {
    if (matching_types) {
      // Special case for bools: only == and !=
      if (lhs_ty->Is<Bool>() && (expr->IsEqual() || expr->IsNotEqual())) {
        return build(builder_->create<sem::Bool>());
      }

      // For the rest, we can compare i32, u32, and f32
      if (lhs_ty->IsAnyOf<I32, U32, F32>()) {
        return build(builder_->create<sem::Bool>());
      }
    }

    // Same for vectors
    if (matching_vec_elem_types) {
      if (lhs_vec_elem_type->Is<Bool>() &&
          (expr->IsEqual() || expr->IsNotEqual())) {
        return build(builder_->create<sem::Vector>(
            builder_->create<sem::Bool>(), lhs_vec->Width()));
      }

      if (lhs_vec_elem_type->is_numeric_scalar()) {
        return build(builder_->create<sem::Vector>(
            builder_->create<sem::Bool>(), lhs_vec->Width()));
      }
    }
  }

  // Binary bitwise operations
  if (expr->IsBitwise()) {
    if (matching_types && lhs_ty->is_integer_scalar_or_vector()) {
      return build(lhs_ty);
    }
  }

  // Bit shift expressions
  if (expr->IsBitshift()) {
    // Type validation rules are the same for left or right shift, despite
    // differences in computation rules (i.e. right shift can be arithmetic or
    // logical depending on lhs type).

    if (lhs_ty->IsAnyOf<I32, U32>() && rhs_ty->Is<U32>()) {
      return build(lhs_ty);
    }

    if (lhs_vec_elem_type && lhs_vec_elem_type->IsAnyOf<I32, U32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<U32>()) {
      return build(lhs_ty);
    }
  }

  AddError("Binary expression operand types are invalid for this operation: " +
               TypeNameOf(lhs_ty) + " " + FriendlyName(expr->op) + " " +
               TypeNameOf(rhs_ty),
           expr->source);
  return nullptr;
}

sem::Expression* Resolver::UnaryOp(const ast::UnaryOpExpression* unary) {
  auto* expr_ty = TypeOf(unary->expr);
  if (!expr_ty) {
    return nullptr;
  }

  const sem::Type* ty = nullptr;

  switch (unary->op) {
    case ast::UnaryOp::kNot:
      // Result type matches the deref'd inner type.
      ty = expr_ty->UnwrapRef();
      if (!ty->Is<sem::Bool>() && !ty->is_bool_vector()) {
        AddError(
            "cannot logical negate expression of type '" + TypeNameOf(expr_ty),
            unary->expr->source);
        return nullptr;
      }
      break;

    case ast::UnaryOp::kComplement:
      // Result type matches the deref'd inner type.
      ty = expr_ty->UnwrapRef();
      if (!ty->is_integer_scalar_or_vector()) {
        AddError("cannot bitwise complement expression of type '" +
                     TypeNameOf(expr_ty),
                 unary->expr->source);
        return nullptr;
      }
      break;

    case ast::UnaryOp::kNegation:
      // Result type matches the deref'd inner type.
      ty = expr_ty->UnwrapRef();
      if (!(ty->IsAnyOf<sem::F32, sem::I32>() ||
            ty->is_signed_integer_vector() || ty->is_float_vector())) {
        AddError("cannot negate expression of type '" + TypeNameOf(expr_ty),
                 unary->expr->source);
        return nullptr;
      }
      break;

    case ast::UnaryOp::kAddressOf:
      if (auto* ref = expr_ty->As<sem::Reference>()) {
        if (ref->StoreType()->UnwrapRef()->is_handle()) {
          AddError(
              "cannot take the address of expression in handle storage class",
              unary->expr->source);
          return nullptr;
        }

        auto* array = unary->expr->As<ast::IndexAccessorExpression>();
        auto* member = unary->expr->As<ast::MemberAccessorExpression>();
        if ((array && TypeOf(array->object)->UnwrapRef()->Is<sem::Vector>()) ||
            (member &&
             TypeOf(member->structure)->UnwrapRef()->Is<sem::Vector>())) {
          AddError("cannot take the address of a vector component",
                   unary->expr->source);
          return nullptr;
        }

        ty = builder_->create<sem::Pointer>(ref->StoreType(),
                                            ref->StorageClass(), ref->Access());
      } else {
        AddError("cannot take the address of expression", unary->expr->source);
        return nullptr;
      }
      break;

    case ast::UnaryOp::kIndirection:
      if (auto* ptr = expr_ty->As<sem::Pointer>()) {
        ty = builder_->create<sem::Reference>(
            ptr->StoreType(), ptr->StorageClass(), ptr->Access());
      } else {
        AddError("cannot dereference expression of type '" +
                     TypeNameOf(expr_ty) + "'",
                 unary->expr->source);
        return nullptr;
      }
      break;
  }

  auto val = EvaluateConstantValue(unary, ty);
  return builder_->create<sem::Expression>(unary, ty, current_statement_, val);
}

bool Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
  Mark(stmt->variable);

  if (!ValidateNoDuplicateDefinition(stmt->variable->symbol,
                                     stmt->variable->source)) {
    return false;
  }

  auto* var = Variable(stmt->variable, VariableKind::kLocal);
  if (!var) {
    return false;
  }

  for (auto* deco : stmt->variable->decorations) {
    Mark(deco);
    if (!deco->Is<ast::InternalDecoration>()) {
      AddError("decorations are not valid on local variables", deco->source);
      return false;
    }
  }

  variable_stack_.Set(stmt->variable->symbol, var);
  if (current_block_) {  // Not all statements are inside a block
    current_block_->AddDecl(stmt->variable);
  }

  if (!ValidateVariable(var)) {
    return false;
  }

  return true;
}

sem::Type* Resolver::TypeDecl(const ast::TypeDecl* named_type) {
  sem::Type* result = nullptr;
  if (auto* alias = named_type->As<ast::Alias>()) {
    result = Type(alias->type);
  } else if (auto* str = named_type->As<ast::Struct>()) {
    result = Structure(str);
  } else {
    TINT_UNREACHABLE(Resolver, diagnostics_) << "Unhandled TypeDecl";
  }

  if (!result) {
    return nullptr;
  }

  named_type_info_.emplace(named_type->name, TypeDeclInfo{named_type, result});

  if (!ValidateTypeDecl(named_type)) {
    return nullptr;
  }

  builder_->Sem().Add(named_type, result);
  return result;
}

bool Resolver::ValidateTypeDecl(const ast::TypeDecl* named_type) const {
  auto iter = named_type_info_.find(named_type->name);
  if (iter == named_type_info_.end()) {
    TINT_ICE(Resolver, diagnostics_)
        << "ValidateTypeDecl called() before TypeDecl()";
  }
  if (iter->second.ast != named_type) {
    AddError("type with the name '" +
                 builder_->Symbols().NameFor(named_type->name) +
                 "' was already declared",
             named_type->source);
    AddNote("first declared here", iter->second.ast->source);
    return false;
  }
  return true;
}

sem::Type* Resolver::TypeOf(const ast::Expression* expr) {
  auto* sem = Sem(expr);
  return sem ? const_cast<sem::Type*>(sem->Type()) : nullptr;
}

std::string Resolver::TypeNameOf(const sem::Type* ty) {
  return RawTypeNameOf(ty->UnwrapRef());
}

std::string Resolver::RawTypeNameOf(const sem::Type* ty) {
  return ty->FriendlyName(builder_->Symbols());
}

sem::Type* Resolver::TypeOf(const ast::LiteralExpression* lit) {
  if (lit->Is<ast::SintLiteralExpression>()) {
    return builder_->create<sem::I32>();
  }
  if (lit->Is<ast::UintLiteralExpression>()) {
    return builder_->create<sem::U32>();
  }
  if (lit->Is<ast::FloatLiteralExpression>()) {
    return builder_->create<sem::F32>();
  }
  if (lit->Is<ast::BoolLiteralExpression>()) {
    return builder_->create<sem::Bool>();
  }
  TINT_UNREACHABLE(Resolver, diagnostics_)
      << "Unhandled literal type: " << lit->TypeInfo().name;
  return nullptr;
}

bool Resolver::ValidatePipelineStages() {
  auto check_workgroup_storage = [&](const sem::Function* func,
                                     const sem::Function* entry_point) {
    auto stage = entry_point->Declaration()->PipelineStage();
    if (stage != ast::PipelineStage::kCompute) {
      for (auto* var : func->DirectlyReferencedGlobals()) {
        if (var->StorageClass() == ast::StorageClass::kWorkgroup) {
          std::stringstream stage_name;
          stage_name << stage;
          for (auto* user : var->Users()) {
            if (func == user->Stmt()->Function()) {
              AddError("workgroup memory cannot be used by " +
                           stage_name.str() + " pipeline stage",
                       user->Declaration()->source);
              break;
            }
          }
          AddNote("variable is declared here", var->Declaration()->source);
          if (func != entry_point) {
            TraverseCallChain(entry_point, func, [&](const sem::Function* f) {
              AddNote(
                  "called by function '" +
                      builder_->Symbols().NameFor(f->Declaration()->symbol) +
                      "'",
                  f->Declaration()->source);
            });
            AddNote("called by entry point '" +
                        builder_->Symbols().NameFor(
                            entry_point->Declaration()->symbol) +
                        "'",
                    entry_point->Declaration()->source);
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
    for (auto* func : entry_point->TransitivelyCalledFunctions()) {
      if (!check_workgroup_storage(func, entry_point)) {
        return false;
      }
    }
  }

  auto check_intrinsic_calls = [&](const sem::Function* func,
                                   const sem::Function* entry_point) {
    auto stage = entry_point->Declaration()->PipelineStage();
    for (auto* intrinsic : func->DirectlyCalledIntrinsics()) {
      if (!intrinsic->SupportedStages().Contains(stage)) {
        auto* call = func->FindDirectCallTo(intrinsic);
        std::stringstream err;
        err << "built-in cannot be used by " << stage << " pipeline stage";
        AddError(err.str(), call ? call->Declaration()->source
                                 : func->Declaration()->source);
        if (func != entry_point) {
          TraverseCallChain(entry_point, func, [&](const sem::Function* f) {
            AddNote("called by function '" +
                        builder_->Symbols().NameFor(f->Declaration()->symbol) +
                        "'",
                    f->Declaration()->source);
          });
          AddNote("called by entry point '" +
                      builder_->Symbols().NameFor(
                          entry_point->Declaration()->symbol) +
                      "'",
                  entry_point->Declaration()->source);
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
    for (auto* func : entry_point->TransitivelyCalledFunctions()) {
      if (!check_intrinsic_calls(func, entry_point)) {
        return false;
      }
    }
  }
  return true;
}

template <typename CALLBACK>
void Resolver::TraverseCallChain(const sem::Function* from,
                                 const sem::Function* to,
                                 CALLBACK&& callback) const {
  for (auto* f : from->TransitivelyCalledFunctions()) {
    if (f == to) {
      callback(f);
      return;
    }
    if (f->TransitivelyCalledFunctions().contains(to)) {
      TraverseCallChain(f, to, callback);
      callback(f);
      return;
    }
  }
  TINT_ICE(Resolver, diagnostics_)
      << "TraverseCallChain() 'from' does not transitively call 'to'";
}

sem::Array* Resolver::Array(const ast::Array* arr) {
  auto source = arr->source;

  auto* elem_type = Type(arr->type);
  if (!elem_type) {
    return nullptr;
  }

  if (!IsPlain(elem_type)) {  // Check must come before GetDefaultAlignAndSize()
    AddError(TypeNameOf(elem_type) +
                 " cannot be used as an element type of an array",
             source);
    return nullptr;
  }

  uint32_t el_align = elem_type->Align();
  uint32_t el_size = elem_type->Size();

  if (!ValidateNoDuplicateDecorations(arr->decorations)) {
    return nullptr;
  }

  // Look for explicit stride via [[stride(n)]] decoration
  uint32_t explicit_stride = 0;
  for (auto* deco : arr->decorations) {
    Mark(deco);
    if (auto* sd = deco->As<ast::StrideDecoration>()) {
      explicit_stride = sd->stride;
      if (!ValidateArrayStrideDecoration(sd, el_size, el_align, source)) {
        return nullptr;
      }
      continue;
    }

    AddError("decoration is not valid for array types", deco->source);
    return nullptr;
  }

  // Calculate implicit stride
  uint64_t implicit_stride = utils::RoundUp<uint64_t>(el_align, el_size);

  uint64_t stride = explicit_stride ? explicit_stride : implicit_stride;

  // Evaluate the constant array size expression.
  // sem::Array uses a size of 0 for a runtime-sized array.
  uint32_t count = 0;
  if (auto* count_expr = arr->count) {
    auto* count_sem = Expression(count_expr);
    if (!count_sem) {
      return nullptr;
    }

    auto size_source = count_expr->source;

    auto* ty = count_sem->Type()->UnwrapRef();
    if (!ty->is_integer_scalar()) {
      AddError("array size must be integer scalar", size_source);
      return nullptr;
    }

    if (auto* ident = count_expr->As<ast::IdentifierExpression>()) {
      // Make sure the identifier is a non-overridable module-scope constant.
      auto* var = variable_stack_.Get(ident->symbol);
      if (!var || !var->Is<sem::GlobalVariable>() ||
          !var->Declaration()->is_const) {
        AddError("array size identifier must be a module-scope constant",
                 size_source);
        return nullptr;
      }
      if (ast::HasDecoration<ast::OverrideDecoration>(
              var->Declaration()->decorations)) {
        AddError("array size expression must not be pipeline-overridable",
                 size_source);
        return nullptr;
      }

      count_expr = var->Declaration()->constructor;
    } else if (!count_expr->Is<ast::LiteralExpression>()) {
      AddError(
          "array size expression must be either a literal or a module-scope "
          "constant",
          size_source);
      return nullptr;
    }

    auto count_val = count_sem->ConstantValue();
    if (!count_val) {
      TINT_ICE(Resolver, diagnostics_)
          << "could not resolve array size expression";
      return nullptr;
    }

    if (ty->is_signed_integer_scalar() ? count_val.Elements()[0].i32 < 1
                                       : count_val.Elements()[0].u32 < 1u) {
      AddError("array size must be at least 1", size_source);
      return nullptr;
    }

    count = count_val.Elements()[0].u32;
  }

  auto size = std::max<uint64_t>(count, 1) * stride;
  if (size > std::numeric_limits<uint32_t>::max()) {
    std::stringstream msg;
    msg << "array size in bytes must not exceed 0x" << std::hex
        << std::numeric_limits<uint32_t>::max() << ", but is 0x" << std::hex
        << size;
    AddError(msg.str(), arr->source);
    return nullptr;
  }
  if (stride > std::numeric_limits<uint32_t>::max() ||
      implicit_stride > std::numeric_limits<uint32_t>::max()) {
    TINT_ICE(Resolver, diagnostics_)
        << "calculated array stride exceeds uint32";
    return nullptr;
  }
  auto* out = builder_->create<sem::Array>(
      elem_type, count, el_align, static_cast<uint32_t>(size),
      static_cast<uint32_t>(stride), static_cast<uint32_t>(implicit_stride));

  if (!ValidateArray(out, source)) {
    return nullptr;
  }

  if (elem_type->Is<sem::Atomic>()) {
    atomic_composite_info_.emplace(out, arr->type->source);
  } else {
    auto found = atomic_composite_info_.find(elem_type);
    if (found != atomic_composite_info_.end()) {
      atomic_composite_info_.emplace(out, found->second);
    }
  }

  return out;
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
  auto stride = deco->stride;
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
             str->Declaration()->source);
    return false;
  }

  std::unordered_set<uint32_t> locations;
  for (auto* member : str->Members()) {
    if (auto* r = member->Type()->As<sem::Array>()) {
      if (r->IsRuntimeSized()) {
        if (member != str->Members().back()) {
          AddError(
              "runtime arrays may only appear as the last member of a struct",
              member->Declaration()->source);
          return false;
        }
        if (!str->IsBlockDecorated()) {
          AddError(
              "a struct containing a runtime-sized array "
              "requires the [[block]] attribute: '" +
                  builder_->Symbols().NameFor(str->Declaration()->name) + "'",
              member->Declaration()->source);
          return false;
        }
      }
    }

    auto has_position = false;
    const ast::InvariantDecoration* invariant_attribute = nullptr;
    for (auto* deco : member->Declaration()->decorations) {
      if (!deco->IsAnyOf<ast::BuiltinDecoration,             //
                         ast::InternalDecoration,            //
                         ast::InterpolateDecoration,         //
                         ast::InvariantDecoration,           //
                         ast::LocationDecoration,            //
                         ast::StructMemberOffsetDecoration,  //
                         ast::StructMemberSizeDecoration,    //
                         ast::StructMemberAlignDecoration>()) {
        if (deco->Is<ast::StrideDecoration>() &&
            IsValidationDisabled(
                member->Declaration()->decorations,
                ast::DisabledValidation::kIgnoreStrideDecoration)) {
          continue;
        }
        AddError("decoration is not valid for structure members", deco->source);
        return false;
      }

      if (auto* invariant = deco->As<ast::InvariantDecoration>()) {
        invariant_attribute = invariant;
      } else if (auto* location = deco->As<ast::LocationDecoration>()) {
        if (!ValidateLocationDecoration(location, member->Type(), locations,
                                        member->Declaration()->source)) {
          return false;
        }
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (!ValidateBuiltinDecoration(builtin, member->Type(),
                                       /* is_input */ false)) {
          return false;
        }
        if (builtin->builtin == ast::Builtin::kPosition) {
          has_position = true;
        }
      } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
        if (!ValidateInterpolateDecoration(interpolate, member->Type())) {
          return false;
        }
      }
    }

    if (invariant_attribute && !has_position) {
      AddError("invariant attribute must only be applied to a position builtin",
               invariant_attribute->source);
      return false;
    }

    if (auto* member_struct_type = member->Type()->As<sem::Struct>()) {
      if (auto* member_struct_type_block_decoration =
              ast::GetDecoration<ast::StructBlockDecoration>(
                  member_struct_type->Declaration()->decorations)) {
        AddError("structs must not contain [[block]] decorated struct members",
                 member->Declaration()->source);
        AddNote("see member's struct decoration here",
                member_struct_type_block_decoration->source);
        return false;
      }
    }
  }

  for (auto* deco : str->Declaration()->decorations) {
    if (!(deco->Is<ast::StructBlockDecoration>())) {
      AddError("decoration is not valid for struct declarations", deco->source);
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateLocationDecoration(
    const ast::LocationDecoration* location,
    const sem::Type* type,
    std::unordered_set<uint32_t>& locations,
    const Source& source,
    const bool is_input) {
  std::string inputs_or_output = is_input ? "inputs" : "output";
  if (current_function_ && current_function_->Declaration()->PipelineStage() ==
                               ast::PipelineStage::kCompute) {
    AddError("decoration is not valid for compute shader " + inputs_or_output,
             location->source);
    return false;
  }

  if (!type->is_numeric_scalar_or_vector()) {
    std::string invalid_type = TypeNameOf(type);
    AddError("cannot apply 'location' attribute to declaration of type '" +
                 invalid_type + "'",
             source);
    AddNote(
        "'location' attribute must only be applied to declarations of "
        "numeric scalar or numeric vector type",
        location->source);
    return false;
  }

  if (locations.count(location->value)) {
    AddError(deco_to_str(location) + " attribute appears multiple times",
             location->source);
    return false;
  }
  locations.emplace(location->value);

  return true;
}

sem::Struct* Resolver::Structure(const ast::Struct* str) {
  if (!ValidateNoDuplicateDecorations(str->decorations)) {
    return nullptr;
  }
  for (auto* deco : str->decorations) {
    Mark(deco);
  }

  sem::StructMemberList sem_members;
  sem_members.reserve(str->members.size());

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
  uint64_t struct_size = 0;
  uint64_t struct_align = 1;
  std::unordered_map<Symbol, const ast::StructMember*> member_map;

  for (auto* member : str->members) {
    Mark(member);
    auto result = member_map.emplace(member->symbol, member);
    if (!result.second) {
      AddError("redefinition of '" +
                   builder_->Symbols().NameFor(member->symbol) + "'",
               member->source);
      AddNote("previous definition is here", result.first->second->source);
      return nullptr;
    }

    // Resolve member type
    auto* type = Type(member->type);
    if (!type) {
      return nullptr;
    }

    // Validate member type
    if (!IsPlain(type)) {
      AddError(TypeNameOf(type) +
                   " cannot be used as the type of a structure member",
               member->source);
      return nullptr;
    }

    uint64_t offset = struct_size;
    uint64_t align = type->Align();
    uint64_t size = type->Size();

    if (!ValidateNoDuplicateDecorations(member->decorations)) {
      return nullptr;
    }

    bool has_offset_deco = false;
    bool has_align_deco = false;
    bool has_size_deco = false;
    for (auto* deco : member->decorations) {
      Mark(deco);
      if (auto* o = deco->As<ast::StructMemberOffsetDecoration>()) {
        // Offset decorations are not part of the WGSL spec, but are emitted
        // by the SPIR-V reader.
        if (o->offset < struct_size) {
          AddError("offsets must be in ascending order", o->source);
          return nullptr;
        }
        offset = o->offset;
        align = 1;
        has_offset_deco = true;
      } else if (auto* a = deco->As<ast::StructMemberAlignDecoration>()) {
        if (a->align <= 0 || !utils::IsPowerOfTwo(a->align)) {
          AddError("align value must be a positive, power-of-two integer",
                   a->source);
          return nullptr;
        }
        align = a->align;
        has_align_deco = true;
      } else if (auto* s = deco->As<ast::StructMemberSizeDecoration>()) {
        if (s->size < size) {
          AddError("size must be at least as big as the type's size (" +
                       std::to_string(size) + ")",
                   s->source);
          return nullptr;
        }
        size = s->size;
        has_size_deco = true;
      }
    }

    if (has_offset_deco && (has_align_deco || has_size_deco)) {
      AddError(
          "offset decorations cannot be used with align or size decorations",
          member->source);
      return nullptr;
    }

    offset = utils::RoundUp(align, offset);
    if (offset > std::numeric_limits<uint32_t>::max()) {
      std::stringstream msg;
      msg << "struct member has byte offset 0x" << std::hex << offset
          << ", but must not exceed 0x" << std::hex
          << std::numeric_limits<uint32_t>::max();
      AddError(msg.str(), member->source);
      return nullptr;
    }

    auto* sem_member = builder_->create<sem::StructMember>(
        member, member->symbol, type, static_cast<uint32_t>(sem_members.size()),
        static_cast<uint32_t>(offset), static_cast<uint32_t>(align),
        static_cast<uint32_t>(size));
    builder_->Sem().Add(member, sem_member);
    sem_members.emplace_back(sem_member);

    struct_size = offset + size;
    struct_align = std::max(struct_align, align);
  }

  uint64_t size_no_padding = struct_size;
  struct_size = utils::RoundUp(struct_align, struct_size);

  if (struct_size > std::numeric_limits<uint32_t>::max()) {
    std::stringstream msg;
    msg << "struct size in bytes must not exceed 0x" << std::hex
        << std::numeric_limits<uint32_t>::max() << ", but is 0x" << std::hex
        << struct_size;
    AddError(msg.str(), str->source);
    return nullptr;
  }
  if (struct_align > std::numeric_limits<uint32_t>::max()) {
    TINT_ICE(Resolver, diagnostics_)
        << "calculated struct stride exceeds uint32";
    return nullptr;
  }

  auto* out = builder_->create<sem::Struct>(
      str, str->name, sem_members, static_cast<uint32_t>(struct_align),
      static_cast<uint32_t>(struct_size),
      static_cast<uint32_t>(size_no_padding));

  for (size_t i = 0; i < sem_members.size(); i++) {
    auto* mem_type = sem_members[i]->Type();
    if (mem_type->Is<sem::Atomic>()) {
      atomic_composite_info_.emplace(out,
                                     sem_members[i]->Declaration()->source);
      break;
    } else {
      auto found = atomic_composite_info_.find(mem_type);
      if (found != atomic_composite_info_.end()) {
        atomic_composite_info_.emplace(out, found->second);
        break;
      }
    }
  }

  if (!ValidateStructure(out)) {
    return nullptr;
  }

  return out;
}

bool Resolver::ValidateReturn(const ast::ReturnStatement* ret) {
  auto* func_type = current_function_->ReturnType();

  auto* ret_type = ret->value ? TypeOf(ret->value)->UnwrapRef()
                              : builder_->create<sem::Void>();

  if (func_type->UnwrapRef() != ret_type) {
    AddError(
        "return statement type must match its function "
        "return type, returned '" +
            TypeNameOf(ret_type) + "', expected '" + TypeNameOf(func_type) +
            "'",
        ret->source);
    return false;
  }

  auto* sem = Sem(ret);
  if (auto* continuing =
          sem->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
    AddError("continuing blocks must not contain a return statement",
             ret->source);
    if (continuing != sem->Parent()) {
      AddNote("see continuing block here", continuing->Declaration()->source);
    }
    return false;
  }

  return true;
}

bool Resolver::Return(const ast::ReturnStatement* ret) {
  if (auto* value = ret->value) {
    if (!Expression(value)) {
      return false;
    }
  }

  // Validate after processing the return value expression so that its type is
  // available for validation.
  return ValidateReturn(ret);
}

bool Resolver::ValidateSwitch(const ast::SwitchStatement* s) {
  auto* cond_type = TypeOf(s->condition)->UnwrapRef();
  if (!cond_type->is_integer_scalar()) {
    AddError(
        "switch statement selector expression must be of a "
        "scalar integer type",
        s->condition->source);
    return false;
  }

  bool has_default = false;
  std::unordered_map<uint32_t, Source> selectors;

  for (auto* case_stmt : s->body) {
    if (case_stmt->IsDefault()) {
      if (has_default) {
        // More than one default clause
        AddError("switch statement must have exactly one default clause",
                 case_stmt->source);
        return false;
      }
      has_default = true;
    }

    for (auto* selector : case_stmt->selectors) {
      if (cond_type != TypeOf(selector)) {
        AddError(
            "the case selector values must have the same "
            "type as the selector expression.",
            case_stmt->source);
        return false;
      }

      auto v = selector->ValueAsU32();
      auto it = selectors.find(v);
      if (it != selectors.end()) {
        auto val = selector->Is<ast::IntLiteralExpression>()
                       ? std::to_string(selector->ValueAsI32())
                       : std::to_string(selector->ValueAsU32());
        AddError("duplicate switch case '" + val + "'", selector->source);
        AddNote("previous case declared here", it->second);
        return false;
      }
      selectors.emplace(v, selector->source);
    }
  }

  if (!has_default) {
    // No default clause
    AddError("switch statement must have a default clause", s->source);
    return false;
  }

  if (!s->body.empty()) {
    auto* last_clause = s->body.back()->As<ast::CaseStatement>();
    auto* last_stmt = last_clause->body->Last();
    if (last_stmt && last_stmt->Is<ast::FallthroughStatement>()) {
      AddError(
          "a fallthrough statement must not appear as "
          "the last statement in last clause of a switch",
          last_stmt->source);
      return false;
    }
  }

  return true;
}

bool Resolver::SwitchStatement(const ast::SwitchStatement* stmt) {
  auto* sem = builder_->create<sem::SwitchStatement>(
      stmt, current_compound_statement_, current_function_);
  builder_->Sem().Add(stmt, sem);
  return Scope(sem, [&] {
    if (!Expression(stmt->condition)) {
      return false;
    }
    for (auto* case_stmt : stmt->body) {
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

bool Resolver::Assignment(const ast::AssignmentStatement* a) {
  if (!Expression(a->lhs) || !Expression(a->rhs)) {
    return false;
  }

  return ValidateAssignment(a);
}

bool Resolver::ValidateAssignment(const ast::AssignmentStatement* a) {
  auto const* rhs_ty = TypeOf(a->rhs);

  if (a->lhs->Is<ast::PhonyExpression>()) {
    // https://www.w3.org/TR/WGSL/#phony-assignment-section
    auto* ty = rhs_ty->UnwrapRef();
    if (!ty->IsConstructible() &&
        !ty->IsAnyOf<sem::Pointer, sem::Texture, sem::Sampler>()) {
      AddError(
          "cannot assign '" + TypeNameOf(rhs_ty) +
              "' to '_'. '_' can only be assigned a constructible, pointer, "
              "texture or sampler type",
          a->rhs->source);
      return false;
    }
    return true;  // RHS can be anything.
  }

  // https://gpuweb.github.io/gpuweb/wgsl/#assignment-statement
  auto const* lhs_ty = TypeOf(a->lhs);

  if (auto* ident = a->lhs->As<ast::IdentifierExpression>()) {
    if (auto* var = variable_stack_.Get(ident->symbol)) {
      if (var->Is<sem::Parameter>()) {
        AddError("cannot assign to function parameter", a->lhs->source);
        AddNote("'" + builder_->Symbols().NameFor(ident->symbol) +
                    "' is declared here:",
                var->Declaration()->source);
        return false;
      }
      if (var->Declaration()->is_const) {
        AddError("cannot assign to const", a->lhs->source);
        AddNote("'" + builder_->Symbols().NameFor(ident->symbol) +
                    "' is declared here:",
                var->Declaration()->source);
        return false;
      }
    }
  }

  auto* lhs_ref = lhs_ty->As<sem::Reference>();
  if (!lhs_ref) {
    // LHS is not a reference, so it has no storage.
    AddError("cannot assign to value of type '" + TypeNameOf(lhs_ty) + "'",
             a->lhs->source);
    return false;
  }

  auto* storage_ty = lhs_ref->StoreType();
  auto* value_type = rhs_ty->UnwrapRef();  // Implicit load of RHS

  // Value type has to match storage type
  if (storage_ty != value_type) {
    AddError("cannot assign '" + TypeNameOf(rhs_ty) + "' to '" +
                 TypeNameOf(lhs_ty) + "'",
             a->source);
    return false;
  }
  if (!storage_ty->IsConstructible()) {
    AddError("storage type of assignment must be constructible", a->source);
    return false;
  }
  if (lhs_ref->Access() == ast::Access::kRead) {
    AddError(
        "cannot store into a read-only type '" + RawTypeNameOf(lhs_ty) + "'",
        a->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateNoDuplicateDefinition(Symbol sym,
                                             const Source& source,
                                             bool check_global_scope_only) {
  if (check_global_scope_only) {
    if (auto* var = variable_stack_.Get(sym)) {
      if (var->Is<sem::GlobalVariable>()) {
        AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
                 source);
        AddNote("previous definition is here", var->Declaration()->source);
        return false;
      }
    }
    auto it = symbol_to_function_.find(sym);
    if (it != symbol_to_function_.end()) {
      AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
               source);
      AddNote("previous definition is here", it->second->Declaration()->source);
      return false;
    }
  } else {
    if (auto* var = variable_stack_.Get(sym)) {
      AddError("redefinition of '" + builder_->Symbols().NameFor(sym) + "'",
               source);
      AddNote("previous definition is here", var->Declaration()->source);
      return false;
    }
  }
  return true;
}

bool Resolver::ValidateNoDuplicateDecorations(
    const ast::DecorationList& decorations) {
  std::unordered_map<const TypeInfo*, Source> seen;
  for (auto* d : decorations) {
    auto res = seen.emplace(&d->TypeInfo(), d->source);
    if (!res.second && !d->Is<ast::InternalDecoration>()) {
      AddError("duplicate " + d->Name() + " decoration", d->source);
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
        err << "while analysing structure member " << TypeNameOf(str) << "."
            << builder_->Symbols().NameFor(member->Declaration()->symbol);
        AddNote(err.str(), member->Declaration()->source);
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
    err << "Type '" << TypeNameOf(ty) << "' cannot be used in storage class '"
        << sc << "' as it is non-host-shareable";
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
  variable_stack_.Push();

  TINT_DEFER({
    current_block_ = prev_current_block;
    current_compound_statement_ = prev_current_compound_statement;
    current_statement_ = prev_current_statement;
    variable_stack_.Pop();
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
      << "At: " << node->source << "\n"
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

template <typename SEM, typename AST_OR_TYPE>
const sem::Info::GetResultType<SEM, AST_OR_TYPE>* Resolver::Sem(
    const AST_OR_TYPE* ast) {
  auto* sem = builder_->Sem().Get<SEM>(ast);
  if (!sem) {
    TINT_ICE(Resolver, diagnostics_)
        << "AST node '" << ast->TypeInfo().name << "' had no semantic info\n"
        << "At: " << ast->source << "\n"
        << "Pointer: " << ast;
  }
  return sem;
}

////////////////////////////////////////////////////////////////////////////////
// Resolver::TypeConversionSig
////////////////////////////////////////////////////////////////////////////////
bool Resolver::TypeConversionSig::operator==(
    const TypeConversionSig& rhs) const {
  return target == rhs.target && source == rhs.source;
}
std::size_t Resolver::TypeConversionSig::Hasher::operator()(
    const TypeConversionSig& sig) const {
  return utils::Hash(sig.target, sig.source);
}

////////////////////////////////////////////////////////////////////////////////
// Resolver::TypeConstructorSig
////////////////////////////////////////////////////////////////////////////////
Resolver::TypeConstructorSig::TypeConstructorSig(
    const sem::Type* ty,
    const std::vector<const sem::Type*> params)
    : type(ty), parameters(params) {}
Resolver::TypeConstructorSig::TypeConstructorSig(const TypeConstructorSig&) =
    default;
Resolver::TypeConstructorSig::~TypeConstructorSig() = default;

bool Resolver::TypeConstructorSig::operator==(
    const TypeConstructorSig& rhs) const {
  return type == rhs.type && parameters == rhs.parameters;
}
std::size_t Resolver::TypeConstructorSig::Hasher::operator()(
    const TypeConstructorSig& sig) const {
  return utils::Hash(sig.type, sig.parameters);
}

}  // namespace resolver
}  // namespace tint
