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
#include "src/utils/map.h"
#include "src/utils/math.h"
#include "src/utils/reverse.h"
#include "src/utils/scoped_assignment.h"
#include "src/utils/transform.h"

namespace tint {
namespace resolver {
namespace {

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

template <typename CALLBACK>
void TraverseCallChain(diag::List& diagnostics,
                       const sem::Function* from,
                       const sem::Function* to,
                       CALLBACK&& callback) {
  for (auto* f : from->TransitivelyCalledFunctions()) {
    if (f == to) {
      callback(f);
      return;
    }
    if (f->TransitivelyCalledFunctions().contains(to)) {
      TraverseCallChain(diagnostics, f, to, callback);
      callback(f);
      return;
    }
  }
  TINT_ICE(Resolver, diagnostics)
      << "TraverseCallChain() 'from' does not transitively call 'to'";
}

}  // namespace

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

  if (var->Is<sem::GlobalVariable>()) {
    auto name = builder_->Symbols().NameFor(decl->symbol);
    if (sem::ParseIntrinsicType(name) != sem::IntrinsicType::kNone) {
      auto* kind = var->Declaration()->is_const ? "let" : "var";
      AddError(
          "'" + name +
              "' is a builtin and cannot be redeclared as a module-scope " +
              kind,
          decl->source);
      return false;
    }
  }

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

  auto name = builder_->Symbols().NameFor(decl->symbol);
  if (sem::ParseIntrinsicType(name) != sem::IntrinsicType::kNone) {
    AddError(
        "'" + name + "' is a builtin and cannot be redeclared as a function",
        decl->source);
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
      sem::Behaviors behaviors{sem::Behavior::kNext};
      if (auto* last = decl->body->Last()) {
        behaviors = Sem(last)->Behaviors();
      }
      if (behaviors.Contains(sem::Behavior::kNext)) {
        AddError("missing return at end of function", decl->source);
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

  // https://www.w3.org/TR/WGSL/#behaviors-rules
  // a function behavior is always one of {}, {Next}, {Discard}, or
  // {Next, Discard}.
  if (func->Behaviors() != sem::Behaviors{} &&  // NOLINT: bad warning
      func->Behaviors() != sem::Behavior::kNext &&
      func->Behaviors() != sem::Behavior::kDiscard &&
      func->Behaviors() != sem::Behaviors{sem::Behavior::kNext,  //
                                          sem::Behavior::kDiscard}) {
    TINT_ICE(Resolver, diagnostics_)
        << "function '" << name << "' behaviors are: " << func->Behaviors();
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

      if (interpolate_attribute) {
        if (!pipeline_io_attribute ||
            !pipeline_io_attribute->Is<ast::LocationDecoration>()) {
          AddError("interpolate attribute must only be used with [[location]]",
                   interpolate_attribute->source);
          return false;
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

bool Resolver::ValidateStatements(const ast::StatementList& stmts) {
  for (auto* stmt : stmts) {
    if (!Sem(stmt)->IsReachable()) {
      /// TODO(https://github.com/gpuweb/gpuweb/issues/2378): This may need to
      /// become an error.
      AddWarning("code is unreachable", stmt->source);
      break;
    }
  }
  return true;
}

bool Resolver::ValidateBitcast(const ast::BitcastExpression* cast,
                               const sem::Type* to) {
  auto* from = TypeOf(cast->expr)->UnwrapRef();
  if (!from->is_numeric_scalar_or_vector()) {
    AddError("'" + TypeNameOf(from) + "' cannot be bitcast",
             cast->expr->source);
    return false;
  }
  if (!to->is_numeric_scalar_or_vector()) {
    AddError("cannot bitcast to '" + TypeNameOf(to) + "'", cast->type->source);
    return false;
  }

  auto width = [&](const sem::Type* ty) {
    if (auto* vec = ty->As<sem::Vector>()) {
      return vec->Width();
    }
    return 1u;
  };

  if (width(from) != width(to)) {
    AddError("cannot bitcast from '" + TypeNameOf(from) + "' to '" +
                 TypeNameOf(to) + "'",
             cast->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateBreakStatement(const sem::Statement* stmt) {
  if (!stmt->FindFirstParent<sem::LoopBlockStatement, sem::CaseStatement>()) {
    AddError("break statement must be in a loop or switch case",
             stmt->Declaration()->source);
    return false;
  }
  return true;
}

bool Resolver::ValidateContinueStatement(const sem::Statement* stmt) {
  if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ true)) {
    AddError("continuing blocks must not contain a continue statement",
             stmt->Declaration()->source);
    if (continuing != stmt->Declaration() &&
        continuing != stmt->Parent()->Declaration()) {
      AddNote("see continuing block here", continuing->source);
    }
    return false;
  }

  if (!stmt->FindFirstParent<sem::LoopBlockStatement>()) {
    AddError("continue statement must be in a loop",
             stmt->Declaration()->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateDiscardStatement(const sem::Statement* stmt) {
  if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ false)) {
    AddError("continuing blocks must not contain a discard statement",
             stmt->Declaration()->source);
    if (continuing != stmt->Declaration() &&
        continuing != stmt->Parent()->Declaration()) {
      AddNote("see continuing block here", continuing->source);
    }
    return false;
  }
  return true;
}

bool Resolver::ValidateFallthroughStatement(const sem::Statement* stmt) {
  if (auto* block = As<sem::BlockStatement>(stmt->Parent())) {
    if (auto* c = As<sem::CaseStatement>(block->Parent())) {
      if (block->Declaration()->Last() == stmt->Declaration()) {
        if (auto* s = As<sem::SwitchStatement>(c->Parent())) {
          if (c->Declaration() != s->Declaration()->body.back()) {
            return true;
          }
          AddError(
              "a fallthrough statement must not be used in the last switch "
              "case",
              stmt->Declaration()->source);
          return false;
        }
      }
    }
  }
  AddError(
      "fallthrough must only be used as the last statement of a case block",
      stmt->Declaration()->source);
  return false;
}

bool Resolver::ValidateElseStatement(const sem::ElseStatement* stmt) {
  if (auto* cond = stmt->Condition()) {
    auto* cond_ty = cond->Type()->UnwrapRef();
    if (!cond_ty->Is<sem::Bool>()) {
      AddError(
          "else statement condition must be bool, got " + TypeNameOf(cond_ty),
          stmt->Condition()->Declaration()->source);
      return false;
    }
  }
  return true;
}

bool Resolver::ValidateForLoopStatement(const sem::ForLoopStatement* stmt) {
  if (auto* cond = stmt->Condition()) {
    auto* cond_ty = cond->Type()->UnwrapRef();
    if (!cond_ty->Is<sem::Bool>()) {
      AddError("for-loop condition must be bool, got " + TypeNameOf(cond_ty),
               stmt->Condition()->Declaration()->source);
      return false;
    }
  }
  return true;
}

bool Resolver::ValidateIfStatement(const sem::IfStatement* stmt) {
  auto* cond_ty = stmt->Condition()->Type()->UnwrapRef();
  if (!cond_ty->Is<sem::Bool>()) {
    AddError("if statement condition must be bool, got " + TypeNameOf(cond_ty),
             stmt->Condition()->Declaration()->source);
    return false;
  }
  return true;
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

bool Resolver::ValidateTextureIntrinsicFunction(const sem::Call* call) {
  auto* intrinsic = call->Target()->As<sem::Intrinsic>();
  if (!intrinsic) {
    return false;
  }

  std::string func_name = intrinsic->str();
  auto& signature = intrinsic->Signature();

  auto check_arg_is_constexpr = [&](sem::ParameterUsage usage, int min,
                                    int max) {
    auto index = signature.IndexOf(usage);
    if (index < 0) {
      return true;
    }
    std::string name = sem::str(usage);
    auto* arg = call->Arguments()[index];
    if (auto values = arg->ConstantValue()) {
      // Assert that the constant values are of the expected type.
      if (!values.Type()->IsAnyOf<sem::I32, sem::Vector>() ||
          !values.ElementType()->Is<sem::I32>()) {
        TINT_ICE(Resolver, diagnostics_)
            << "failed to resolve '" + func_name + "' " << name
            << " parameter type";
        return false;
      }

      // Currently const_expr is restricted to literals and type constructors.
      // Check that that's all we have for the parameter.
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
        auto vector = intrinsic->Parameters()[index]->Type()->Is<sem::Vector>();
        for (size_t i = 0; i < values.Elements().size(); i++) {
          auto value = values.Elements()[i].i32;
          if (value < min || value > max) {
            if (vector) {
              AddError("each component of the " + name +
                           " argument must be at least " + std::to_string(min) +
                           " and at most " + std::to_string(max) + ". " + name +
                           " component " + std::to_string(i) + " is " +
                           std::to_string(value),
                       arg->Declaration()->source);
            } else {
              AddError("the " + name + " argument must be at least " +
                           std::to_string(min) + " and at most " +
                           std::to_string(max) + ". " + name + " is " +
                           std::to_string(value),
                       arg->Declaration()->source);
            }
            return false;
          }
        }
        return true;
      }
    }
    AddError("the " + name + " argument must be a const_expression",
             arg->Declaration()->source);
    return false;
  };

  return check_arg_is_constexpr(sem::ParameterUsage::kOffset, -8, 7) &&
         check_arg_is_constexpr(sem::ParameterUsage::kComponent, 0, 3);
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
        auto* var = ResolvedSymbol<sem::Variable>(ident_expr);
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
            auto* var = ResolvedSymbol<sem::Variable>(ident_unary);
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

  if (call->Behaviors().Contains(sem::Behavior::kDiscard)) {
    if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ false)) {
      AddError(
          "cannot call a function that may discard inside a continuing block",
          call->Declaration()->source);
      if (continuing != call->Stmt()->Declaration() &&
          continuing != call->Stmt()->Parent()->Declaration()) {
        AddNote("see continuing block here", continuing->source);
      }
      return false;
    }
  }

  return true;
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
            TraverseCallChain(diagnostics_, entry_point, func,
                              [&](const sem::Function* f) {
                                AddNote("called by function '" +
                                            builder_->Symbols().NameFor(
                                                f->Declaration()->symbol) +
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
          TraverseCallChain(
              diagnostics_, entry_point, func, [&](const sem::Function* f) {
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

bool Resolver::ValidateAlias(const ast::Alias* alias) {
  auto name = builder_->Symbols().NameFor(alias->name);
  if (sem::ParseIntrinsicType(name) != sem::IntrinsicType::kNone) {
    AddError("'" + name + "' is a builtin and cannot be redeclared as an alias",
             alias->source);
    return false;
  }

  return true;
}

bool Resolver::ValidateStructure(const sem::Struct* str) {
  auto name = builder_->Symbols().NameFor(str->Declaration()->name);
  if (sem::ParseIntrinsicType(name) != sem::IntrinsicType::kNone) {
    AddError("'" + name + "' is a builtin and cannot be redeclared as a struct",
             str->Declaration()->source);
    return false;
  }

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

    auto has_location = false;
    auto has_position = false;
    const ast::InvariantDecoration* invariant_attribute = nullptr;
    const ast::InterpolateDecoration* interpolate_attribute = nullptr;
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
        has_location = true;
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
        interpolate_attribute = interpolate;
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

    if (interpolate_attribute && !has_location) {
      AddError("interpolate attribute must only be used with [[location]]",
               interpolate_attribute->source);
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
  if (auto* continuing = ClosestContinuing(/*stop_at_loop*/ false)) {
    AddError("continuing blocks must not contain a return statement",
             ret->source);
    if (continuing != sem->Declaration() &&
        continuing != sem->Parent()->Declaration()) {
      AddNote("see continuing block here", continuing->source);
    }
    return false;
  }

  return true;
}

bool Resolver::ValidateSwitch(const ast::SwitchStatement* s) {
  auto* cond_ty = TypeOf(s->condition)->UnwrapRef();
  if (!cond_ty->is_integer_scalar()) {
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
      if (cond_ty != TypeOf(selector)) {
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

  return true;
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

  if (auto* var = ResolvedSymbol<sem::Variable>(a->lhs)) {
    auto* decl = var->Declaration();
    if (var->Is<sem::Parameter>()) {
      AddError("cannot assign to function parameter", a->lhs->source);
      AddNote("'" + builder_->Symbols().NameFor(decl->symbol) +
                  "' is declared here:",
              decl->source);
      return false;
    }
    if (decl->is_const) {
      AddError("cannot assign to const", a->lhs->source);
      AddNote("'" + builder_->Symbols().NameFor(decl->symbol) +
                  "' is declared here:",
              decl->source);
      return false;
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

bool Resolver::IsValidationDisabled(const ast::DecorationList& decorations,
                                    ast::DisabledValidation validation) const {
  for (auto* decoration : decorations) {
    if (auto* dv = decoration->As<ast::DisableValidationDecoration>()) {
      if (dv->validation == validation) {
        return true;
      }
    }
  }
  return false;
}

bool Resolver::IsValidationEnabled(const ast::DecorationList& decorations,
                                   ast::DisabledValidation validation) const {
  return !IsValidationDisabled(decorations, validation);
}

}  // namespace resolver
}  // namespace tint
