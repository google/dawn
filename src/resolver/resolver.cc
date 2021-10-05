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
#include "src/sem/variable.h"
#include "src/utils/defer.h"
#include "src/utils/get_or_create.h"
#include "src/utils/math.h"
#include "src/utils/reverse.h"
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

  AllocateOverridableConstantIds();

  if (!ValidatePipelineStages()) {
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
    if (auto* t = ty->As<ast::DepthMultisampledTexture>()) {
      return builder_->create<sem::DepthMultisampledTexture>(t->dim());
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
    AddError("atomic only supports i32 or u32 types",
             a->type() ? a->type()->source() : a->source());
    return false;
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
                                           VariableKind kind,
                                           uint32_t index /* = 0 */) {
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
    AddError("let declaration must have an initializer", var->source());
    return nullptr;
  } else if (!var->type()) {
    AddError(
        (kind == VariableKind::kGlobal)
            ? "module scope var declaration requires a type and initializer"
            : "function scope var declaration requires a type or initializer",
        var->source());
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
  if (storage_class == ast::StorageClass::kNone && !var->is_const()) {
    // No declared storage class. Infer from usage / type.
    if (kind == VariableKind::kLocal) {
      storage_class = ast::StorageClass::kFunction;
    } else if (storage_type->UnwrapRef()->is_handle()) {
      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // If the store type is a texture type or a sampler type, then the
      // variable declaration must not have a storage class decoration. The
      // storage class will always be handle.
      storage_class = ast::StorageClass::kUniformConstant;
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

  if (rhs_type &&
      !ValidateVariableConstructor(var, storage_class, storage_type, type_name,
                                   rhs_type, rhs_type_name)) {
    return nullptr;
  }

  auto* info =
      variable_infos_.Create(var, const_cast<sem::Type*>(type), type_name,
                             storage_class, access, kind, index);
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
        ast::GetDecoration<ast::OverrideDecoration>(var->decorations());
    if (!override_deco) {
      continue;
    }

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

    variable_to_info_[var]->constant_id = constant_id;
  }
}

bool Resolver::ValidateVariableConstructor(const ast::Variable* var,
                                           ast::StorageClass storage_class,
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

  if (!var->is_const()) {
    switch (storage_class) {
      case ast::StorageClass::kPrivate:
      case ast::StorageClass::kFunction:
        break;  // Allowed an initializer
      default:
        // https://gpuweb.github.io/gpuweb/wgsl/#var-and-let
        // Optionally has an initializer expression, if the variable is in the
        // private or function storage classes.
        AddError("var of storage class '" +
                     std::string(ast::str(storage_class)) +
                     "' cannot have an initializer. var initializers are only "
                     "supported for the storage classes "
                     "'private' and 'function'",
                 var->source());
        return false;
    }
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

  // TODO(bclayton): Call this at the end of resolve on all uniform and storage
  // referenced structs
  if (!ValidateStorageClassLayout(info)) {
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
    return builder_->Symbols().NameFor(sm->Declaration()->symbol());
  };

  auto type_name_of = [this](const sem::StructMember* sm) {
    return sm->Declaration()->type()->FriendlyName(builder_->Symbols());
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

    print_struct_begin_line(st->Align(), st->Size(),
                            st->FriendlyName(builder_->Symbols()));

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
                   "' in storage class '" + ast::str(sc) +
                   "' must be a multiple of " + std::to_string(required_align) +
                   " bytes, but '" + member_name_of(m) +
                   "' is currently at offset " + std::to_string(m->Offset()) +
                   ". Consider setting [[align(" +
                   std::to_string(required_align) + ")]] on this member",
               m->Declaration()->source());

      AddNote("see layout of struct:\n" + get_struct_layout_string(str),
              str->Declaration()->source());

      if (auto* member_str = m->Type()->As<sem::Struct>()) {
        AddNote("and layout of struct member:\n" +
                    get_struct_layout_string(member_str),
                member_str->Declaration()->source());
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
            m->Declaration()->source());

        AddNote("see layout of struct:\n" + get_struct_layout_string(str),
                str->Declaration()->source());

        auto* prev_member_str = prev_member->Type()->As<sem::Struct>();
        AddNote("and layout of previous member struct:\n" +
                    get_struct_layout_string(prev_member_str),
                prev_member_str->Declaration()->source());
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
              m->Declaration()->type()->source());
          AddNote("see layout of struct:\n" + get_struct_layout_string(str),
                  str->Declaration()->source());
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

bool Resolver::ValidateStorageClassLayout(const VariableInfo* info) {
  if (auto* str = info->type->UnwrapRef()->As<sem::Struct>()) {
    if (!ValidateStorageClassLayout(str, info->storage_class)) {
      AddNote("see declaration of variable", info->declaration->source());
      return false;
    }
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
        "only variables in <storage> storage class may declare an access mode",
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

  if (!info->declaration->is_const()) {
    if (!ValidateAtomicVariable(info)) {
      return false;
    }
  }

  return ValidateVariable(info);
}

// https://gpuweb.github.io/gpuweb/wgsl/#atomic-types
// Atomic types may only be instantiated by variables in the workgroup storage
// class or by storage buffer variables with a read_write access mode.
bool Resolver::ValidateAtomicVariable(const VariableInfo* info) {
  auto sc = info->storage_class;
  auto access = info->access;
  auto* type = info->type->UnwrapRef();
  auto source = info->declaration->type() ? info->declaration->type()->source()
                                          : info->declaration->source();

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
        AddNote("atomic sub-type of '" +
                    type->FriendlyName(builder_->Symbols()) +
                    "' is declared here",
                found->second);
        return false;
      } else if (sc == ast::StorageClass::kStorage &&
                 access != ast::Access::kReadWrite) {
        AddError(
            "atomic variables in <storage> storage class must have read_write "
            "access mode",
            source);
        AddNote("atomic sub-type of '" +
                    type->FriendlyName(builder_->Symbols()) +
                    "' is declared here",
                found->second);
        return false;
      }
    }
  }

  return true;
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

  if (var->is_const() && info->kind != VariableKind::kParameter &&
      !(storage_type->IsConstructible() || storage_type->Is<sem::Pointer>())) {
    AddError(storage_type->FriendlyName(builder_->Symbols()) +
                 " cannot be used as the type of a let",
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
    } else if (!deco->IsAnyOf<ast::BuiltinDecoration, ast::InvariantDecoration,
                              ast::LocationDecoration,
                              ast::InterpolateDecoration,
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
    if (!info->type->IsConstructible() &&
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
  bool is_output = !is_input;
  switch (deco->value()) {
    case ast::Builtin::kPosition:
      if (stage != ast::PipelineStage::kNone &&
          !((is_input && stage == ast::PipelineStage::kFragment) ||
            (is_output && stage == ast::PipelineStage::kVertex))) {
        is_stage_mismatch = true;
      }
      if (!(type->is_float_vector() && type->As<sem::Vector>()->Width() == 4)) {
        AddError("store type of " + deco_to_str(deco) + " must be 'vec4<f32>'",
                 deco->source());
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

  if (func->params().size() > 255) {
    AddError("functions may declare at most 255 parameters", func->source());
    return false;
  }

  for (auto* param : func->params()) {
    if (!ValidateFunctionParameter(func, variable_to_info_.at(param))) {
      return false;
    }
  }

  if (!info->return_type->Is<sem::Void>()) {
    if (!info->return_type->IsConstructible()) {
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
      if (!deco->IsAnyOf<ast::BuiltinDecoration, ast::InternalDecoration,
                         ast::LocationDecoration, ast::InterpolateDecoration,
                         ast::InvariantDecoration>() &&
          (IsValidationEnabled(info->declaration->decorations(),
                               ast::DisabledValidation::kEntryPointParameter) &&
           IsValidationEnabled(info->declaration->decorations(),
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
  auto validate_entry_point_decorations_inner =
      [&](const ast::DecorationList& decos, sem::Type* ty, Source source,
          ParamOrRetType param_or_ret, bool is_struct_member) {
        // Scan decorations for pipeline IO attributes.
        // Check for overlap with attributes that have been seen previously.
        ast::Decoration* pipeline_io_attribute = nullptr;
        ast::InvariantDecoration* invariant_attribute = nullptr;
        for (auto* deco : decos) {
          auto is_invalid_compute_shader_decoration = false;
          if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
            if (pipeline_io_attribute) {
              AddError("multiple entry point IO attributes", deco->source());
              AddNote(
                  "previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
              return false;
            }
            pipeline_io_attribute = deco;

            if (builtins.count(builtin->value())) {
              AddError(
                  deco_to_str(builtin) +
                      " attribute appears multiple times as pipeline " +
                      (param_or_ret == ParamOrRetType::kParameter ? "input"
                                                                  : "output"),
                  func->source());
              return false;
            }

            if (!ValidateBuiltinDecoration(builtin, ty,
                                           /* is_input */ param_or_ret ==
                                               ParamOrRetType::kParameter)) {
              return false;
            }
            builtins.emplace(builtin->value());
          } else if (auto* location = deco->As<ast::LocationDecoration>()) {
            if (pipeline_io_attribute) {
              AddError("multiple entry point IO attributes", deco->source());
              AddNote(
                  "previously consumed " + deco_to_str(pipeline_io_attribute),
                  pipeline_io_attribute->source());
              return false;
            }
            pipeline_io_attribute = deco;

            bool is_input = param_or_ret == ParamOrRetType::kParameter;
            if (!ValidateLocationDecoration(location, ty, locations, source,
                                            is_input)) {
              return false;
            }
          } else if (auto* interpolate =
                         deco->As<ast::InterpolateDecoration>()) {
            if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
              is_invalid_compute_shader_decoration = true;
            } else if (!ValidateInterpolateDecoration(interpolate, ty)) {
              return false;
            }
          } else if (auto* invariant = deco->As<ast::InvariantDecoration>()) {
            if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
              is_invalid_compute_shader_decoration = true;
            }
            invariant_attribute = invariant;
          }
          if (is_invalid_compute_shader_decoration) {
            std::string input_or_output =
                param_or_ret == ParamOrRetType::kParameter ? "inputs"
                                                           : "output";
            AddError(
                "decoration is not valid for compute shader " + input_or_output,
                deco->source());
            return false;
          }
        }

        if (IsValidationEnabled(
                decos, ast::DisabledValidation::kEntryPointParameter)) {
          if (!ty->Is<sem::Struct>() && !pipeline_io_attribute) {
            std::string err = "missing entry point IO attribute";
            if (!is_struct_member) {
              err += (param_or_ret == ParamOrRetType::kParameter
                          ? " on parameter"
                          : " on return type");
            }
            AddError(err, source);
            return false;
          }

          if (invariant_attribute) {
            bool has_position = false;
            if (pipeline_io_attribute) {
              if (auto* builtin =
                      pipeline_io_attribute->As<ast::BuiltinDecoration>()) {
                has_position = (builtin->value() == ast::Builtin::kPosition);
              }
            }
            if (!has_position) {
              AddError(
                  "invariant attribute must only be applied to a position "
                  "builtin",
                  invariant_attribute->source());
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
    if (!validate_entry_point_decorations_inner(decos, ty, source, param_or_ret,
                                                /*is_struct_member*/ false)) {
      return false;
    }

    if (auto* str = ty->As<sem::Struct>()) {
      for (auto* member : str->Members()) {
        if (!validate_entry_point_decorations_inner(
                member->Declaration()->decorations(), member->Type(),
                member->Declaration()->source(), param_or_ret,
                /*is_struct_member*/ true)) {
          AddNote("while analysing entry point '" +
                      builder_->Symbols().NameFor(func->symbol()) + "'",
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
  uint32_t parameter_index = 0;
  std::unordered_map<Symbol, Source> parameter_names;
  for (auto* param : func->params()) {
    Mark(param);

    {  // Check the parameter name is unique for the function
      auto emplaced = parameter_names.emplace(param->symbol(), param->source());
      if (!emplaced.second) {
        auto name = builder_->Symbols().NameFor(param->symbol());
        AddError("redefinition of parameter '" + name + "'", param->source());
        AddNote("previous definition is here", emplaced.first->second);
        return false;
      }
    }

    auto* param_info =
        Variable(param, VariableKind::kParameter, parameter_index++);
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

      Mark(expr);
      if (!Expression(expr)) {
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
        AddError(kErrBadType, expr->source());
        return false;
      }

      any_i32 = any_i32 || is_i32;
      any_u32 = any_u32 || is_u32;
      if (any_i32 && any_u32) {
        AddError(kErrInconsistentType, expr->source());
        return false;
      }

      if (auto* ident = expr->As<ast::IdentifierExpression>()) {
        // We have an identifier of a module-scope constant.
        VariableInfo* var = nullptr;
        if (!variable_stack_.get(ident->symbol(), &var) ||
            !(var->declaration->is_const())) {
          AddError(kErrBadType, expr->source());
          return false;
        }

        // Capture the constant if an [[override]] attribute is present.
        if (ast::HasDecoration<ast::OverrideDecoration>(
                var->declaration->decorations())) {
          info->workgroup_size[i].overridable_const = var->declaration;
        }

        expr = var->declaration->constructor();
        if (!expr) {
          // No constructor means this value must be overriden by the user.
          info->workgroup_size[i].value = 0;
          continue;
        }
      } else if (!expr->Is<ast::ScalarConstructorExpression>()) {
        AddError(
            "workgroup_size argument must be either a literal or a "
            "module-scope constant",
            values[i]->source());
        return false;
      }

      auto val = ConstantValueOf(expr);
      if (!val) {
        TINT_ICE(Resolver, diagnostics_)
            << "could not resolve constant workgroup_size constant value";
        continue;
      }
      // Validate and set the default value for this dimension.
      if (is_i32 ? val.Elements()[0].i32 < 1 : val.Elements()[0].u32 < 1) {
        AddError("workgroup_size argument must be at least 1",
                 values[i]->source());
        return false;
      }

      info->workgroup_size[i].value =
          is_i32 ? static_cast<uint32_t>(val.Elements()[0].i32)
                 : val.Elements()[0].u32;
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
  bool unreachable = false;
  for (auto* stmt : stmts) {
    if (unreachable) {
      AddError("code is unreachable", stmt->source());
      return false;
    }

    auto* nested_stmt = stmt;
    while (auto* block = nested_stmt->As<ast::BlockStatement>()) {
      if (block->empty()) {
        break;
      }
      nested_stmt = block->statements().back();
    }
    if (nested_stmt->IsAnyOf<ast::ReturnStatement, ast::BreakStatement,
                             ast::ContinueStatement, ast::DiscardStatement>()) {
      unreachable = true;
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
                 stmt->source());
        return false;
      }
    } else {
      AddError("continue statement must be in a loop", stmt->source());
      return false;
    }

    return true;
  }
  if (stmt->Is<ast::DiscardStatement>()) {
    if (auto* continuing =
            sem_statement
                ->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
      AddError("continuing blocks must not contain a discard statement",
               stmt->source());
      if (continuing != sem_statement->Parent()) {
        AddNote("see continuing block here",
                continuing->Declaration()->source());
      }
      return false;
    }
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

      if (!TypeOf(condition)->UnwrapRef()->Is<sem::Bool>()) {
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

bool Resolver::TraverseExpressions(ast::Expression* root,
                                   std::vector<ast::Expression*>& out) {
  std::vector<ast::Expression*> to_visit;
  to_visit.emplace_back(root);

  auto add = [&](ast::Expression* e) {
    Mark(e);
    to_visit.emplace_back(e);
  };

  while (!to_visit.empty()) {
    auto* expr = to_visit.back();
    to_visit.pop_back();

    out.emplace_back(expr);

    if (auto* array = expr->As<ast::ArrayAccessorExpression>()) {
      add(array->array());
      add(array->idx_expr());
    } else if (auto* bin_op = expr->As<ast::BinaryExpression>()) {
      add(bin_op->lhs());
      add(bin_op->rhs());
    } else if (auto* bitcast = expr->As<ast::BitcastExpression>()) {
      add(bitcast->expr());
    } else if (auto* call = expr->As<ast::CallExpression>()) {
      for (auto* arg : call->params()) {
        add(arg);
      }
    } else if (auto* type_ctor = expr->As<ast::TypeConstructorExpression>()) {
      for (auto* value : type_ctor->values()) {
        add(value);
      }
    } else if (auto* member = expr->As<ast::MemberAccessorExpression>()) {
      add(member->structure());
    } else if (auto* unary = expr->As<ast::UnaryOpExpression>()) {
      add(unary->expr());
    } else if (expr->IsAnyOf<ast::ScalarConstructorExpression,
                             ast::IdentifierExpression>()) {
      // Leaf expression
    } else {
      TINT_ICE(Resolver, diagnostics_)
          << "unhandled expression type: " << expr->TypeInfo().name;
      return false;
    }
  }

  return true;
}

bool Resolver::Expression(ast::Expression* root) {
  std::vector<ast::Expression*> sorted;
  if (!TraverseExpressions(root, sorted)) {
    return false;
  }

  for (auto* expr : utils::Reverse(sorted)) {
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
      TINT_ICE(Resolver, diagnostics_)
          << "unhandled expression type: " << expr->TypeInfo().name;
      return false;
    }
    if (!ok) {
      return false;
    }
  }

  return true;
}

bool Resolver::ArrayAccessor(ast::ArrayAccessorExpression* expr) {
  auto* idx = expr->idx_expr();
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
    AddError("cannot index type '" + TypeNameOf(expr->array()) + "'",
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

  return ValidateCall(call);
}

bool Resolver::ValidateCall(ast::CallExpression* call) {
  if (TypeOf(call)->Is<sem::Void>()) {
    bool is_call_statement = false;
    if (current_statement_) {
      if (auto* call_stmt =
              As<ast::CallStatement>(current_statement_->Declaration())) {
        if (call_stmt->expr() == call) {
          is_call_statement = true;
        }
      }
    }
    if (!is_call_statement) {
      // https://gpuweb.github.io/gpuweb/wgsl/#function-call-expr
      // If the called function does not return a value, a function call
      // statement should be used instead.
      auto* ident = call->func();
      auto name = builder_->Symbols().NameFor(ident->symbol());
      // A function call is made to either a user declared function or an
      // intrinsic. function_calls_ only maps CallExpression to user declared
      // functions
      bool is_function = function_calls_.count(call) != 0;
      AddError((is_function ? "function" : "intrinsic") + std::string(" '") +
                   name + "' does not return a value",
               call->source());
      return false;
    }
  }

  return true;
}

bool Resolver::ValidateCallStatement(ast::CallStatement* stmt) {
  const sem::Type* return_type = TypeOf(stmt->expr());
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

  auto* out = builder_->create<sem::Call>(call, result, current_statement_);
  builder_->Sem().Add(call, out);
  SetExprInfo(call, result->ReturnType());

  current_function_->intrinsic_calls.emplace_back(
      IntrinsicCallInfo{call, result});

  if (IsTextureIntrinsic(intrinsic_type) &&
      !ValidateTextureIntrinsicFunction(call, out)) {
    return false;
  }

  return true;
}

bool Resolver::ValidateTextureIntrinsicFunction(
    const ast::CallExpression* ast_call,
    const sem::Call* sem_call) {
  auto* intrinsic = sem_call->Target()->As<sem::Intrinsic>();
  if (!intrinsic) {
    return false;
  }
  std::string func_name = intrinsic->str();
  auto index =
      sem::IndexOf(intrinsic->Parameters(), sem::ParameterUsage::kOffset);
  if (index > -1) {
    auto* param = ast_call->params()[index];
    if (param->Is<ast::TypeConstructorExpression>()) {
      auto values = ConstantValueOf(param);
      if (!values.IsValid()) {
        AddError(
            "'" + func_name + "' offset parameter must be a const_expression",
            param->source());
        return false;
      }
      if (!values.Type()->Is<sem::Vector>() ||
          !values.ElementType()->Is<sem::I32>()) {
        TINT_ICE(Resolver, diagnostics_)
            << "failed to resolve '" + func_name + "' offset parameter type";
        return false;
      }
      for (auto offset : values.Elements()) {
        auto offset_value = offset.i32;
        if (offset_value < -8 || offset_value > 7) {
          AddError("each offset component of '" + func_name +
                       "' must be at least -8 and at most 7. "
                       "found: '" +
                       std::to_string(offset_value) + "'",
                   param->source());
          return false;
        }
      }
    } else {
      AddError(
          "'" + func_name + "' offset parameter must be a const_expression",
          param->source());
      return false;
    }
  }
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

  function_calls_.emplace(call,
                          FunctionCallInfo{callee_func, current_statement_});
  SetExprInfo(call, callee_func->return_type, callee_func->return_type_name);

  if (!ValidateFunctionCall(call, callee_func)) {
    return false;
  }
  return true;
}

bool Resolver::ValidateFunctionCall(const ast::CallExpression* call,
                                    const FunctionInfo* target) {
  auto* ident = call->func();
  auto name = builder_->Symbols().NameFor(ident->symbol());

  if (target->declaration->IsEntryPoint()) {
    // https://www.w3.org/TR/WGSL/#function-restriction
    // An entry point must never be the target of a function call.
    AddError("entry point functions cannot be the target of a function call",
             call->source());
    return false;
  }

  if (call->params().size() != target->parameters.size()) {
    bool more = call->params().size() > target->parameters.size();
    AddError("too " + (more ? std::string("many") : std::string("few")) +
                 " arguments in call to '" + name + "', expected " +
                 std::to_string(target->parameters.size()) + ", got " +
                 std::to_string(call->params().size()),
             call->source());
    return false;
  }

  for (size_t i = 0; i < call->params().size(); ++i) {
    const VariableInfo* param = target->parameters[i];
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

    if (param->declaration->type()->Is<ast::Pointer>()) {
      auto is_valid = false;
      if (auto* ident_expr = arg_expr->As<ast::IdentifierExpression>()) {
        VariableInfo* var;
        if (!variable_stack_.get(ident_expr->symbol(), &var)) {
          TINT_ICE(Resolver, diagnostics_) << "failed to resolve identifier";
          return false;
        }
        if (var->kind == VariableKind::kParameter) {
          is_valid = true;
        }
      } else if (auto* unary = arg_expr->As<ast::UnaryOpExpression>()) {
        if (unary->op() == ast::UnaryOp::kAddressOf) {
          if (auto* ident_unary =
                  unary->expr()->As<ast::IdentifierExpression>()) {
            VariableInfo* var;
            if (!variable_stack_.get(ident_unary->symbol(), &var)) {
              TINT_ICE(Resolver, diagnostics_)
                  << "failed to resolve identifier";
              return false;
            }
            if (var->declaration->is_const()) {
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
              param->declaration->decorations(),
              ast::DisabledValidation::kIgnoreInvalidPointerArgument)) {
        AddError(
            "expected an address-of expression of a variable identifier "
            "expression or a function parameter",
            arg_expr->source());
        return false;
      }
    }
  }
  return true;
}

bool Resolver::Constructor(ast::ConstructorExpression* expr) {
  if (auto* type_ctor = expr->As<ast::TypeConstructorExpression>()) {
    auto* type = Type(type_ctor->type());
    if (!type) {
      return false;
    }

    auto type_name = type_ctor->type()->FriendlyName(builder_->Symbols());

    // Now that the argument types have been determined, make sure that they
    // obey the constructor type rules laid out in
    // https://gpuweb.github.io/gpuweb/wgsl.html#type-constructor-expr.
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
    } else {
      AddError("type is not constructible", type_ctor->source());
      return false;
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
  if (!struct_type->IsConstructible()) {
    AddError("struct constructor has non-constructible type", ctor->source());
    return false;
  }

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
  } else if (!elem_type->IsConstructible()) {
    AddError("array constructor has non-constructible element type",
             ctor->type()->As<ast::Array>()->type()->source());
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
      if (elem_type != value_elem_type && values.size() > 1u) {
        AddError(
            "type in vector constructor does not match vector type: "
            "expected '" +
                elem_type->FriendlyName(builder_->Symbols()) + "', found '" +
                value_elem_type->FriendlyName(builder_->Symbols()) + "'",
            value->source());
        return false;
      }

      value_cardinality_sum += value_vec->Width();
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
  if (value_cardinality_sum > 1 && value_cardinality_sum != vec_type->Width()) {
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

    if (!value_vec || value_vec->Width() != matrix_type->rows() ||
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

  const bool is_valid = (type->Is<Bool>() && value_type->is_scalar()) ||
                        (type->Is<I32>() && value_type->is_scalar()) ||
                        (type->Is<U32>() && value_type->is_scalar()) ||
                        (type->Is<F32>() && value_type->is_scalar());
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
          auto iter = std::find_if(
              decls.begin(), decls.end(),
              [&symbol](auto* v) { return v->symbol() == symbol; });
          if (iter != decls.end()) {
            auto var_decl_index =
                static_cast<size_t>(std::distance(decls.begin(), iter));
            if (var_decl_index >= loop_block->NumDeclsAtFirstContinue()) {
              AddError("continue statement bypasses declaration of '" +
                           builder_->Symbols().NameFor(symbol) + "'",
                       loop_block->FirstContinue()->source());
              AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                          "' declared here",
                      (*iter)->source());
              AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                          "' referenced in continuing block here",
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
  auto* structure = TypeOf(expr->structure());
  auto* storage_type = structure->UnwrapRef();

  sem::Type* ret = nullptr;
  std::vector<uint32_t> swizzle;

  if (auto* str = storage_type->As<sem::Struct>()) {
    Mark(expr->member());
    auto symbol = expr->member()->symbol();

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

      if (swizzle.back() >= vec->Width()) {
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
    AddError(
        "invalid member accessor expression. Expected vector or struct, got '" +
            TypeNameOf(expr->structure()) + "'",
        expr->structure()->source());
    return false;
  }

  SetExprInfo(expr, ret);

  return true;
}

bool Resolver::Binary(ast::BinaryExpression* expr) {
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
      (lhs_vec->Width() == rhs_vec->Width());

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
        (lhs_vec->Width() == rhs_mat->rows())) {
      SetExprInfo(expr, builder_->create<sem::Vector>(lhs_vec->type(),
                                                      rhs_mat->columns()));
      return true;
    }

    // Matrix times vector
    if (lhs_mat_elem_type && lhs_mat_elem_type->Is<F32>() &&
        rhs_vec_elem_type && rhs_vec_elem_type->Is<F32>() &&
        (lhs_mat->columns() == rhs_vec->Width())) {
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
                              builder_->create<sem::Bool>(), lhs_vec->Width()));
        return true;
      }

      if (lhs_vec_elem_type->is_numeric_scalar()) {
        SetExprInfo(expr, builder_->create<sem::Vector>(
                              builder_->create<sem::Bool>(), lhs_vec->Width()));
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
        if (ref->StoreType()->UnwrapRef()->is_handle()) {
          AddError(
              "cannot take the address of expression in handle storage class",
              unary->expr()->source());
          return false;
        }
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
    Mark(deco);
    if (!deco->Is<ast::InternalDecoration>()) {
      AddError("decorations are not valid on local variables", deco->source());
      return false;
    }
  }

  variable_stack_.set(var->symbol(), info);
  if (current_block_) {  // Not all statements are inside a block
    current_block_->AddDecl(var);
  }

  if (!ValidateVariable(info)) {
    return false;
  }

  if (!var->is_const() &&
      IsValidationEnabled(var->decorations(),
                          ast::DisabledValidation::kIgnoreStorageClass)) {
    if (!info->type->UnwrapRef()->IsConstructible()) {
      AddError("function variable must have a constructible type",
               var->type() ? var->type()->source() : var->source());
      return false;
    }
    if (info->storage_class != ast::StorageClass::kFunction) {
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

  // Create semantic nodes for all ast::Variables
  std::unordered_map<const tint::ast::Variable*, sem::Parameter*> sem_params;
  for (auto it : variable_to_info_) {
    auto* var = it.first;
    auto* info = it.second;

    sem::Variable* sem_var = nullptr;

    if (ast::HasDecoration<ast::OverrideDecoration>(var->decorations())) {
      // Create a pipeline overridable constant.
      sem_var = builder_->create<sem::GlobalVariable>(var, info->type,
                                                      info->constant_id);
    } else {
      switch (info->kind) {
        case VariableKind::kGlobal:
          sem_var = builder_->create<sem::GlobalVariable>(
              var, info->type, info->storage_class, info->access,
              info->binding_point);
          break;
        case VariableKind::kLocal:
          sem_var = builder_->create<sem::LocalVariable>(
              var, info->type, info->storage_class, info->access);
          break;
        case VariableKind::kParameter: {
          auto* param = builder_->create<sem::Parameter>(
              var, info->index, info->type, info->storage_class, info->access);
          sem_var = param;
          sem_params.emplace(var, param);
          break;
        }
      }
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

    std::vector<sem::Parameter*> parameters;
    parameters.reserve(info->parameters.size());
    for (auto* p : info->parameters) {
      parameters.emplace_back(sem_params.at(p->declaration));
    }

    auto* sem_func = builder_->create<sem::Function>(
        info->declaration, const_cast<sem::Type*>(info->return_type),
        parameters, remap_vars(info->referenced_module_vars),
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

sem::Array* Resolver::Array(const ast::Array* arr) {
  auto source = arr->source();

  auto* elem_type = Type(arr->type());
  if (!elem_type) {
    return nullptr;
  }

  if (!IsPlain(elem_type)) {  // Check must come before GetDefaultAlignAndSize()
    AddError(elem_type->FriendlyName(builder_->Symbols()) +
                 " cannot be used as an element type of an array",
             source);
    return nullptr;
  }

  uint32_t el_align = elem_type->Align();
  uint32_t el_size = elem_type->Size();

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
  uint64_t implicit_stride = utils::RoundUp<uint64_t>(el_align, el_size);

  uint64_t stride = explicit_stride ? explicit_stride : implicit_stride;

  // Evaluate the constant array size expression.
  // sem::Array uses a size of 0 for a runtime-sized array.
  uint32_t count = 0;
  if (auto* count_expr = arr->Size()) {
    Mark(count_expr);
    if (!Expression(count_expr)) {
      return nullptr;
    }

    auto size_source = count_expr->source();

    auto* ty = TypeOf(count_expr)->UnwrapRef();
    if (!ty->is_integer_scalar()) {
      AddError("array size must be integer scalar", size_source);
      return nullptr;
    }

    if (auto* ident = count_expr->As<ast::IdentifierExpression>()) {
      // Make sure the identifier is a non-overridable module-scope constant.
      VariableInfo* var = nullptr;
      bool is_global = false;
      if (!variable_stack_.get(ident->symbol(), &var, &is_global) ||
          !is_global || !var->declaration->is_const()) {
        AddError("array size identifier must be a module-scope constant",
                 size_source);
        return nullptr;
      }
      if (ast::HasDecoration<ast::OverrideDecoration>(
              var->declaration->decorations())) {
        AddError("array size expression must not be pipeline-overridable",
                 size_source);
        return nullptr;
      }

      count_expr = var->declaration->constructor();
    } else if (!count_expr->Is<ast::ScalarConstructorExpression>()) {
      AddError(
          "array size expression must be either a literal or a module-scope "
          "constant",
          size_source);
      return nullptr;
    }

    auto count_val = ConstantValueOf(count_expr);
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
    AddError(msg.str(), arr->source());
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
    atomic_composite_info_.emplace(out, arr->type()->source());
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

  std::unordered_set<uint32_t> locations;
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

    auto has_position = false;
    ast::InvariantDecoration* invariant_attribute = nullptr;
    for (auto* deco : member->Declaration()->decorations()) {
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
                member->Declaration()->decorations(),
                ast::DisabledValidation::kIgnoreStrideDecoration)) {
          continue;
        }
        AddError("decoration is not valid for structure members",
                 deco->source());
        return false;
      }

      if (auto* invariant = deco->As<ast::InvariantDecoration>()) {
        invariant_attribute = invariant;
      } else if (auto* location = deco->As<ast::LocationDecoration>()) {
        if (!ValidateLocationDecoration(location, member->Type(), locations,
                                        member->Declaration()->source())) {
          return false;
        }
      } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        if (!ValidateBuiltinDecoration(builtin, member->Type(),
                                       /* is_input */ false)) {
          return false;
        }
        if (builtin->value() == ast::Builtin::kPosition) {
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
               invariant_attribute->source());
      return false;
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

bool Resolver::ValidateLocationDecoration(
    const ast::LocationDecoration* location,
    const sem::Type* type,
    std::unordered_set<uint32_t>& locations,
    const Source& source,
    const bool is_input) {
  std::string inputs_or_output = is_input ? "inputs" : "output";
  if (current_function_ && current_function_->declaration->pipeline_stage() ==
                               ast::PipelineStage::kCompute) {
    AddError("decoration is not valid for compute shader " + inputs_or_output,
             location->source());
    return false;
  }

  if (!type->is_numeric_scalar_or_vector()) {
    std::string invalid_type = type->FriendlyName(builder_->Symbols());
    AddError("cannot apply 'location' attribute to declaration of type '" +
                 invalid_type + "'",
             source);
    AddNote(
        "'location' attribute must only be applied to declarations of "
        "numeric scalar or numeric vector type",
        location->source());
    return false;
  }

  if (locations.count(location->value())) {
    AddError(deco_to_str(location) + " attribute appears multiple times",
             location->source());
    return false;
  }
  locations.emplace(location->value());

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
  uint64_t struct_size = 0;
  uint64_t struct_align = 1;
  std::unordered_map<Symbol, ast::StructMember*> member_map;

  for (auto* member : str->members()) {
    Mark(member);
    auto result = member_map.emplace(member->symbol(), member);
    if (!result.second) {
      AddError("redefinition of '" +
                   builder_->Symbols().NameFor(member->symbol()) + "'",
               member->source());
      AddNote("previous definition is here", result.first->second->source());
      return nullptr;
    }

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

    uint64_t offset = struct_size;
    uint64_t align = type->Align();
    uint64_t size = type->Size();

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
    if (offset > std::numeric_limits<uint32_t>::max()) {
      std::stringstream msg;
      msg << "struct member has byte offset 0x" << std::hex << offset
          << ", but must not exceed 0x" << std::hex
          << std::numeric_limits<uint32_t>::max();
      AddError(msg.str(), member->source());
      return nullptr;
    }

    auto* sem_member = builder_->create<sem::StructMember>(
        member, member->symbol(), const_cast<sem::Type*>(type),
        static_cast<uint32_t>(sem_members.size()), offset, align, size);
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
    AddError(msg.str(), str->source());
    return nullptr;
  }
  if (struct_align > std::numeric_limits<uint32_t>::max()) {
    TINT_ICE(Resolver, diagnostics_)
        << "calculated struct stride exceeds uint32";
    return nullptr;
  }

  auto* out = builder_->create<sem::Struct>(
      str, str->name(), sem_members, static_cast<uint32_t>(struct_align),
      static_cast<uint32_t>(struct_size),
      static_cast<uint32_t>(size_no_padding));

  for (size_t i = 0; i < sem_members.size(); i++) {
    auto* mem_type = sem_members[i]->Type();
    if (mem_type->Is<sem::Atomic>()) {
      atomic_composite_info_.emplace(out,
                                     sem_members[i]->Declaration()->source());
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

  auto* sem = builder_->Sem().Get(ret);
  if (auto* continuing =
          sem->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
    AddError("continuing blocks must not contain a return statement",
             ret->source());
    if (continuing != sem->Parent()) {
      AddNote("see continuing block here", continuing->Declaration()->source());
    }
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
  if (!storage_type->IsConstructible()) {
    AddError("storage type of assignment must be constructible", a->source());
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
                                     VariableKind k,
                                     uint32_t idx)
    : declaration(decl),
      type(ty),
      type_name(tn),
      storage_class(sc),
      access(ac),
      kind(k),
      index(idx) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}
Resolver::FunctionInfo::~FunctionInfo() = default;

}  // namespace resolver
}  // namespace tint
