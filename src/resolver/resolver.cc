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
#include "src/sem/access_control_type.h"
#include "src/sem/array.h"
#include "src/sem/call.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/pointer_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/sampler_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"
#include "src/utils/math.h"

namespace tint {
namespace resolver {
namespace {

using IntrinsicType = tint::sem::IntrinsicType;

// Helper class that temporarily assigns a value to a reference for the scope of
// the object. Once the ScopedAssignment is destructed, the original value is
// restored.
template <typename T>
class ScopedAssignment {
 public:
  ScopedAssignment(T& ref, T val) : ref_(ref) {
    old_value_ = ref;
    ref = val;
  }
  ~ScopedAssignment() { ref_ = old_value_; }

 private:
  T& ref_;
  T old_value_;
};

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

}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder), intrinsic_table_(IntrinsicTable::Create()) {}

Resolver::~Resolver() = default;

Resolver::BlockInfo::BlockInfo(const ast::BlockStatement* b,
                               Resolver::BlockInfo::Type ty,
                               Resolver::BlockInfo* p)
    : block(b), type(ty), parent(p) {}

Resolver::BlockInfo::~BlockInfo() = default;

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

  if (result && diagnostics_.contains_errors()) {
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
  type = type->UnwrapIfNeeded();
  if (type->is_scalar() || type->Is<sem::Vector>() || type->Is<sem::Matrix>()) {
    return true;
  }
  if (auto* arr = type->As<sem::ArrayType>()) {
    return IsStorable(arr->type());
  }
  if (auto* str_ty = type->As<sem::StructType>()) {
    if (auto* str = Structure(str_ty)) {
      for (const auto* member : str->members) {
        if (!IsStorable(member->Type())) {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

// https://gpuweb.github.io/gpuweb/wgsl.html#host-shareable-types
bool Resolver::IsHostShareable(const sem::Type* type) {
  type = type->UnwrapIfNeeded();
  if (type->IsAnyOf<sem::I32, sem::U32, sem::F32>()) {
    return true;
  }
  if (auto* vec = type->As<sem::Vector>()) {
    return IsHostShareable(vec->type());
  }
  if (auto* mat = type->As<sem::Matrix>()) {
    return IsHostShareable(mat->type());
  }
  if (auto* arr = type->As<sem::ArrayType>()) {
    return IsHostShareable(arr->type());
  }
  if (auto* str = type->As<sem::StructType>()) {
    auto* info = Structure(str);
    if (!info) {
      return false;
    }
    for (auto* member : info->members) {
      if (!IsHostShareable(member->Type())) {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Resolver::IsValidAssignment(const sem::Type* lhs, const sem::Type* rhs) {
  // TODO(crbug.com/tint/659): This is a rough approximation, and is missing
  // checks for writability of pointer storage class, access control, etc.
  // This will need to be fixed after WGSL agrees the behavior of pointers /
  // references.
  // Check:
  if (lhs->UnwrapIfNeeded() != rhs->UnwrapIfNeeded()) {
    // Try RHS dereference
    if (lhs->UnwrapIfNeeded() != rhs->UnwrapAll()) {
      return false;
    }
  }

  return true;
}

bool Resolver::ResolveInternal() {
  Mark(&builder_->AST());

  auto register_named_type = [this](Symbol name, const sem::Type* type,
                                    const Source& source) {
    auto added = named_types_.emplace(name, type).second;
    if (!added) {
      diagnostics_.add_error("type with the name '" +
                                 builder_->Symbols().NameFor(name) +
                                 "' was already declared",
                             source);
      return false;
    }
    return true;
  };

  // Process everything else in the order they appear in the module. This is
  // necessary for validation of use-before-declaration.
  for (auto* decl : builder_->AST().GlobalDeclarations()) {
    if (auto* ty = decl->As<ast::NamedType>()) {
      auto* sem_ty = Type(ty);
      if (sem_ty == nullptr) {
        return false;
      }
      if (!register_named_type(ty->name(), sem_ty, ty->source())) {
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
        // ast::StrideDecoration are used to build a sem::ArrayTypes, but
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

const sem::Type* Resolver::Type(const ast::Type* ty) {
  Mark(ty);
  auto* s = [&]() -> const sem::Type* {
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
    if (auto* t = ty->As<ast::Alias>()) {
      return Type(t->type());
    }
    if (auto* t = ty->As<ast::AccessControl>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::AccessControl>(t->access_control(),
                                                    const_cast<sem::Type*>(el));
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
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::ArrayType>(const_cast<sem::Type*>(el),
                                                t->size(), t->decorations());
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Pointer>()) {
      if (auto* el = Type(t->type())) {
        return builder_->create<sem::Pointer>(const_cast<sem::Type*>(el),
                                              t->storage_class());
      }
      return nullptr;
    }
    if (auto* t = ty->As<ast::Struct>()) {
      return builder_->create<sem::StructType>(const_cast<ast::Struct*>(t));
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
        return builder_->create<sem::StorageTexture>(
            t->dim(), t->image_format(), const_cast<sem::Type*>(el));
      }
      return nullptr;
    }
    if (ty->As<ast::ExternalTexture>()) {
      return builder_->create<sem::ExternalTexture>();
    }
    if (auto* t = ty->As<ast::TypeName>()) {
      auto it = named_types_.find(t->name());
      if (it == named_types_.end()) {
        diagnostics_.add_error(
            "unknown type '" + builder_->Symbols().NameFor(t->name()) + "'",
            t->source());
        return nullptr;
      }
      return it->second;
    }
    TINT_UNREACHABLE(diagnostics_)
        << "Unhandled ast::Type: " << ty->TypeInfo().name;
    return nullptr;
  }();

  if (s == nullptr) {
    return nullptr;
  }
  if (!Type(s, ty->source())) {
    return nullptr;
  }
  builder_->Sem().Add(ty, s);
  return s;
}

// TODO(crbug.com/tint/724): This method should be merged into Type(ast::Type*)
bool Resolver::Type(const sem::Type* ty, const Source& source /* = {} */) {
  ty = ty->UnwrapAliasIfNeeded();
  if (auto* str = ty->As<sem::StructType>()) {
    if (!Structure(str)) {
      return false;
    }
  } else if (auto* arr = ty->As<sem::ArrayType>()) {
    if (!Array(arr, source)) {
      return false;
    }
  }
  return true;
}

Resolver::VariableInfo* Resolver::Variable(
    ast::Variable* var,
    const sem::Type* type, /* = nullptr */
    std::string type_name /* = "" */) {
  auto it = variable_to_info_.find(var);
  if (it != variable_to_info_.end()) {
    return it->second;
  }

  if (type == nullptr && var->type()) {
    type = Type(var->type());
    type_name = var->type()->FriendlyName(builder_->Symbols());
  }
  if (type == nullptr) {
    return nullptr;
  }

  auto* ctype = Canonical(type);
  auto* info = variable_infos_.Create(var, ctype, type_name);
  variable_to_info_.emplace(var, info);

  // TODO(bclayton): Why is this here? Needed?
  // Resolve variable's type
  if (auto* arr = info->type->As<sem::ArrayType>()) {
    if (!Array(arr, var->source())) {
      return nullptr;
    }
  }

  return info;
}

bool Resolver::GlobalVariable(ast::Variable* var) {
  if (variable_stack_.has(var->symbol())) {
    diagnostics_.add_error("v-0011",
                           "redeclared global identifier '" +
                               builder_->Symbols().NameFor(var->symbol()) + "'",
                           var->source());
    return false;
  }

  auto* info = Variable(var);
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
    if (var->is_const()) {
      if (!deco->Is<ast::OverrideDecoration>()) {
        diagnostics_.add_error("decoration is not valid for constants",
                               deco->source());
        return false;
      }
    } else if (!(deco->Is<ast::BindingDecoration>() ||
                 deco->Is<ast::BuiltinDecoration>() ||
                 deco->Is<ast::GroupDecoration>() ||
                 deco->Is<ast::LocationDecoration>())) {
      diagnostics_.add_error("decoration is not valid for variables",
                             deco->source());
      return false;
    }
  }

  if (var->has_constructor()) {
    Mark(var->constructor());
    if (!Expression(var->constructor())) {
      return false;
    }
  }

  if (!ValidateGlobalVariable(info)) {
    return false;
  }

  if (!ApplyStorageClassUsageToType(var->declared_storage_class(), info->type,
                                    var->source())) {
    diagnostics_.add_note("while instantiating variable " +
                              builder_->Symbols().NameFor(var->symbol()),
                          var->source());
    return false;
  }

  return true;
}

bool Resolver::ValidateGlobalVariable(const VariableInfo* info) {
  switch (info->storage_class) {
    case ast::StorageClass::kStorage: {
      // https://gpuweb.github.io/gpuweb/wgsl/#variable-declaration
      // Variables in the storage storage class and variables with a storage
      // texture type must have an access attribute applied to the store type.

      // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
      // A variable in the storage storage class is a storage buffer variable.
      // Its store type must be a host-shareable structure type with block
      // attribute, satisfying the storage class constraints.

      auto* access = info->type->As<sem::AccessControl>();
      auto* str = access ? access->type()->As<sem::StructType>() : nullptr;
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
            str->impl()->source());
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
      auto* str = info->type->As<sem::StructType>();
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
            str->impl()->source());
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

  return ValidateVariable(info->declaration);
}

bool Resolver::ValidateVariable(const ast::Variable* var) {
  auto* type = variable_to_info_[var]->type;
  if (auto* r = type->As<sem::ArrayType>()) {
    if (r->IsRuntimeArray()) {
      diagnostics_.add_error(
          "v-0015",
          "runtime arrays may only appear as the last member of a struct",
          var->source());
      return false;
    }
  }

  if (auto* r = type->As<sem::MultisampledTexture>()) {
    if (r->dim() != ast::TextureDimension::k2d) {
      diagnostics_.add_error("Only 2d multisampled textures are supported",
                             var->source());
      return false;
    }

    auto* data_type = r->type()->UnwrapAll();
    if (!data_type->is_numeric_scalar()) {
      diagnostics_.add_error(
          "texture_multisampled_2d<type>: type must be f32, i32 or u32",
          var->source());
      return false;
    }
  }

  if (type->As<sem::StorageTexture>()) {
    diagnostics_.add_error("Storage Textures must have access control.",
                           var->source());
    return false;
  }

  if (auto* ac = type->As<sem::AccessControl>()) {
    if (auto* r = ac->type()->As<sem::StorageTexture>()) {
      if (ac->IsReadWrite()) {
        diagnostics_.add_error(
            "Storage Textures only support Read-Only and Write-Only access "
            "control.",
            var->source());
        return false;
      }

      if (!IsValidStorageTextureDimension(r->dim())) {
        diagnostics_.add_error(
            "Cube dimensions for storage textures are not "
            "supported.",
            var->source());
        return false;
      }

      if (!IsValidStorageTextureImageFormat(r->image_format())) {
        diagnostics_.add_error(
            "image format must be one of the texel formats specified for "
            "storage textues in "
            "https://gpuweb.github.io/gpuweb/wgsl/#texel-formats",
            var->source());
        return false;
      }
    }
  }

  return true;
}

bool Resolver::ValidateParameter(const ast::Variable* param) {
  return ValidateVariable(param);
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

  for (auto* param : func->params()) {
    if (!ValidateParameter(param)) {
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
    } else if (!ast::HasDecoration<ast::InternalDecoration>(
                   func->decorations())) {
      TINT_ICE(diagnostics_)
          << "Function " << builder_->Symbols().NameFor(func->symbol())
          << " has no body and does not have the [[internal]] decoration";
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
  auto stage_deco_count = 0;
  for (auto* deco : func->decorations()) {
    if (deco->Is<ast::StageDecoration>()) {
      stage_deco_count++;
    } else if (!deco->Is<ast::WorkgroupDecoration>()) {
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

  // Use a lambda to validate the entry point decorations for a type.
  // Persistent state is used to track which builtins and locations have already
  // been seen, in order to catch conflicts.
  // TODO(jrprice): This state could be stored in FunctionInfo instead, and then
  // passed to sem::Function since it would be useful there too.
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
      [&](const ast::DecorationList& decos, const sem::Type* ty, Source source,
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
        if (Canonical(ty)->Is<sem::StructType>()) {
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
        }

        return true;
      };

  // Outer lambda for validating the entry point decorations for a type.
  auto validate_entry_point_decorations = [&](const ast::DecorationList& decos,
                                              const sem::Type* ty,
                                              Source source,
                                              ParamOrRetType param_or_ret) {
    // Validate the decorations for the type.
    if (!validate_entry_point_decorations_inner(decos, ty, source, param_or_ret,
                                                false)) {
      return false;
    }

    if (auto* struct_ty = Canonical(ty)->As<sem::StructType>()) {
      // Validate the decorations for each struct members, and also check for
      // invalid member types.
      for (auto* member : Structure(struct_ty)->members) {
        auto* member_ty = Canonical(member->Type());
        if (member_ty->Is<sem::StructType>()) {
          diagnostics_.add_error(
              "entry point IO types cannot contain nested structures",
              member->Declaration()->source());
          diagnostics_.add_note("while analysing entry point " +
                                    builder_->Symbols().NameFor(func->symbol()),
                                func->source());
          return false;
        } else if (auto* arr = member_ty->As<sem::ArrayType>()) {
          if (arr->IsRuntimeArray()) {
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
                member->Declaration()->decorations(), member_ty,
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

  return true;
}

bool Resolver::Function(ast::Function* func) {
  auto* info = function_infos_.Create<FunctionInfo>(func);

  ScopedAssignment<FunctionInfo*> sa(current_function_, info);

  variable_stack_.push_scope();
  for (auto* param : func->params()) {
    Mark(param);
    auto* param_info = Variable(param);
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

    if (auto* str = param_info->type->As<sem::StructType>()) {
      auto* str_info = Structure(str);
      if (!str_info) {
        return false;
      }
      switch (func->pipeline_stage()) {
        case ast::PipelineStage::kVertex:
          str_info->pipeline_stage_uses.emplace(
              sem::PipelineStageUsage::kVertexInput);
          break;
        case ast::PipelineStage::kFragment:
          str_info->pipeline_stage_uses.emplace(
              sem::PipelineStageUsage::kFragmentInput);
          break;
        case ast::PipelineStage::kCompute:
          str_info->pipeline_stage_uses.emplace(
              sem::PipelineStageUsage::kComputeInput);
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

  info->return_type = Canonical(info->return_type);

  if (auto* str = info->return_type->As<sem::StructType>()) {
    if (!ApplyStorageClassUsageToType(ast::StorageClass::kNone, str,
                                      func->source())) {
      diagnostics_.add_note("while instantiating return type for " +
                                builder_->Symbols().NameFor(func->symbol()),
                            func->source());
      return false;
    }

    auto* str_info = Structure(str);
    if (!str_info) {
      return false;
    }
    switch (func->pipeline_stage()) {
      case ast::PipelineStage::kVertex:
        str_info->pipeline_stage_uses.emplace(
            sem::PipelineStageUsage::kVertexOutput);
        break;
      case ast::PipelineStage::kFragment:
        str_info->pipeline_stage_uses.emplace(
            sem::PipelineStageUsage::kFragmentOutput);
        break;
      case ast::PipelineStage::kCompute:
        str_info->pipeline_stage_uses.emplace(
            sem::PipelineStageUsage::kComputeOutput);
        break;
      case ast::PipelineStage::kNone:
        break;
    }
  }

  if (func->body()) {
    Mark(func->body());
    if (!BlockStatement(func->body())) {
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

bool Resolver::BlockStatement(const ast::BlockStatement* stmt) {
  return BlockScope(stmt, BlockInfo::Type::kGeneric,
                    [&] { return Statements(stmt->list()); });
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
  auto* sem_statement =
      builder_->create<sem::Statement>(stmt, current_block_->block);
  builder_->Sem().Add(stmt, sem_statement);

  ScopedAssignment<sem::Statement*> sa(current_statement_, sem_statement);

  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return Assignment(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return BlockStatement(b);
  }
  if (stmt->Is<ast::BreakStatement>()) {
    if (!current_block_->FindFirstParent(BlockInfo::Type::kLoop) &&
        !current_block_->FindFirstParent(BlockInfo::Type::kSwitchCase)) {
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
            current_block_->FindFirstParent(BlockInfo::Type::kLoop)) {
      if (loop_block->first_continue == size_t(~0)) {
        loop_block->first_continue = loop_block->decls.size();
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
    // We don't call DetermineBlockStatement on the body and continuing block as
    // these would make their BlockInfo siblings as in the AST, but we want the
    // body BlockInfo to parent the continuing BlockInfo for semantics and
    // validation. Also, we need to set their types differently.
    Mark(l->body());
    return BlockScope(l->body(), BlockInfo::Type::kLoop, [&] {
      if (!Statements(l->body()->list())) {
        return false;
      }

      if (l->continuing()) {  // has_continuing() also checks for empty()
        Mark(l->continuing());
      }
      if (l->has_continuing()) {
        if (!BlockScope(l->continuing(), BlockInfo::Type::kLoopContinuing,
                        [&] { return Statements(l->continuing()->list()); })) {
          return false;
        }
      }

      return true;
    });
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
  return BlockScope(stmt->body(), BlockInfo::Type::kSwitchCase,
                    [&] { return Statements(stmt->body()->list()); });
}

bool Resolver::IfStatement(ast::IfStatement* stmt) {
  Mark(stmt->condition());
  if (!Expression(stmt->condition())) {
    return false;
  }

  auto* cond_type = TypeOf(stmt->condition())->UnwrapAll();
  if (cond_type != builder_->ty.bool_()) {
    diagnostics_.add_error("if statement condition must be bool, got " +
                               cond_type->FriendlyName(builder_->Symbols()),
                           stmt->condition()->source());
    return false;
  }

  Mark(stmt->body());
  if (!BlockStatement(stmt->body())) {
    return false;
  }

  for (auto* else_stmt : stmt->else_statements()) {
    Mark(else_stmt);
    // Else statements are a bit unusual - they're owned by the if-statement,
    // not a BlockStatement.
    constexpr ast::BlockStatement* no_block_statement = nullptr;
    auto* sem_else_stmt =
        builder_->create<sem::Statement>(else_stmt, no_block_statement);
    builder_->Sem().Add(else_stmt, sem_else_stmt);
    ScopedAssignment<sem::Statement*> sa(current_statement_, sem_else_stmt);
    if (auto* cond = else_stmt->condition()) {
      Mark(cond);
      if (!Expression(cond)) {
        return false;
      }
    }
    Mark(else_stmt->body());
    if (!BlockStatement(else_stmt->body())) {
      return false;
    }
  }
  return true;
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
  auto* parent_type = res->UnwrapAll();
  sem::Type* ret = nullptr;
  if (auto* arr = parent_type->As<sem::ArrayType>()) {
    ret = arr->type();
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

  // If we're extracting from a pointer, we return a pointer.
  if (auto* ptr = res->As<sem::Pointer>()) {
    ret = builder_->create<sem::Pointer>(ret, ptr->storage_class());
  } else if (auto* arr = parent_type->As<sem::ArrayType>()) {
    if (!arr->type()->is_scalar()) {
      // If we extract a non-scalar from an array then we also get a pointer. We
      // will generate a Function storage class variable to store this into.
      ret = builder_->create<sem::Pointer>(ret, ast::StorageClass::kFunction);
    }
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

  // The expression has to be an identifier as you can't store function pointers
  // but, if it isn't we'll just use the normal result determination to be on
  // the safe side.
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
      // This is currently guaranteed as functions must be declared before use.
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
  auto* elem_type = vec_type->type()->UnwrapAll();
  size_t value_cardinality_sum = 0;
  for (auto* value : values) {
    auto* value_type = TypeOf(value)->UnwrapAll();
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
      sem::Type* value_elem_type = value_vec->type()->UnwrapAll();
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

  auto* elem_type = matrix_type->type()->UnwrapAll();
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
    auto* value_type = TypeOf(value)->UnwrapAll();
    auto* value_vec = value_type->As<sem::Vector>();

    if (!value_vec || value_vec->size() != matrix_type->rows() ||
        elem_type != value_vec->type()->UnwrapAll()) {
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
    // A constant is the type, but a variable is always a pointer so synthesize
    // the pointer around the variable type.
    if (var->declaration->is_const()) {
      SetType(expr, var->type, var->type_name);
    } else if (var->type->Is<sem::Pointer>()) {
      SetType(expr, var->type, var->type_name);
    } else {
      SetType(expr,
              builder_->create<sem::Pointer>(const_cast<sem::Type*>(var->type),
                                             var->storage_class),
              var->type_name);
    }

    var->users.push_back(expr);
    set_referenced_from_function_if_needed(var, true);

    if (current_block_) {
      // If identifier is part of a loop continuing block, make sure it doesn't
      // refer to a variable that is bypassed by a continue statement in the
      // loop's body block.
      if (auto* continuing_block = current_block_->FindFirstParent(
              BlockInfo::Type::kLoopContinuing)) {
        auto* loop_block =
            continuing_block->FindFirstParent(BlockInfo::Type::kLoop);
        if (loop_block->first_continue != size_t(~0)) {
          auto& decls = loop_block->decls;
          // If our identifier is in loop_block->decls, make sure its index is
          // less than first_continue
          auto iter = std::find_if(
              decls.begin(), decls.end(),
              [&symbol](auto* v) { return v->symbol() == symbol; });
          if (iter != decls.end()) {
            auto var_decl_index =
                static_cast<size_t>(std::distance(decls.begin(), iter));
            if (var_decl_index >= loop_block->first_continue) {
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

  auto* res = TypeOf(expr->structure());
  auto* data_type = res->UnwrapPtrIfNeeded()->UnwrapIfNeeded();

  sem::Type* ret = nullptr;
  std::vector<uint32_t> swizzle;

  if (auto* ty = data_type->As<sem::StructType>()) {
    Mark(expr->member());
    auto symbol = expr->member()->symbol();
    auto* str = Structure(ty);

    const sem::StructMember* member = nullptr;
    for (auto* m : str->members) {
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

    // If we're extracting from a pointer, we return a pointer.
    if (auto* ptr = res->As<sem::Pointer>()) {
      ret = builder_->create<sem::Pointer>(ret, ptr->storage_class());
    }

    builder_->Sem().Add(expr, builder_->create<sem::StructMemberAccess>(
                                  expr, ret, current_statement_, member));
  } else if (auto* vec = data_type->As<sem::Vector>()) {
    Mark(expr->member());
    std::string str = builder_->Symbols().NameFor(expr->member()->symbol());
    auto size = str.size();
    swizzle.reserve(str.size());

    for (auto c : str) {
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
    if (!std::all_of(str.begin(), str.end(), is_rgba) &&
        !std::all_of(str.begin(), str.end(), is_xyzw)) {
      diagnostics_.add_error(
          "invalid mixing of vector swizzle characters rgba with xyzw",
          expr->member()->source());
      return false;
    }

    if (size == 1) {
      // A single element swizzle is just the type of the vector.
      ret = vec->type();
      // If we're extracting from a pointer, we return a pointer.
      if (auto* ptr = res->As<sem::Pointer>()) {
        ret = builder_->create<sem::Pointer>(ret, ptr->storage_class());
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
            data_type->type_name(),
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

  auto* lhs_declared_type = TypeOf(expr->lhs())->UnwrapAll();
  auto* rhs_declared_type = TypeOf(expr->rhs())->UnwrapAll();

  auto* lhs_type = Canonical(lhs_declared_type);
  auto* rhs_type = Canonical(rhs_declared_type);

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
          lhs_declared_type->FriendlyName(builder_->Symbols()) + " " +
          FriendlyName(expr->op()) + " " +
          rhs_declared_type->FriendlyName(builder_->Symbols()),
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
    SetType(expr, TypeOf(expr->lhs())->UnwrapPtrIfNeeded());
    return true;
  }
  // Result type is a scalar or vector of boolean type
  if (expr->IsLogicalAnd() || expr->IsLogicalOr() || expr->IsEqual() ||
      expr->IsNotEqual() || expr->IsLessThan() || expr->IsGreaterThan() ||
      expr->IsLessThanEqual() || expr->IsGreaterThanEqual()) {
    auto* bool_type = builder_->create<sem::Bool>();
    auto* param_type = TypeOf(expr->lhs())->UnwrapAll();
    sem::Type* result_type = bool_type;
    if (auto* vec = param_type->As<sem::Vector>()) {
      result_type = builder_->create<sem::Vector>(bool_type, vec->size());
    }
    SetType(expr, result_type);
    return true;
  }
  if (expr->IsMultiply()) {
    auto* lhs_type = TypeOf(expr->lhs())->UnwrapAll();
    auto* rhs_type = TypeOf(expr->rhs())->UnwrapAll();

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

bool Resolver::UnaryOp(ast::UnaryOpExpression* expr) {
  Mark(expr->expr());

  // Result type matches the parameter type.
  if (!Expression(expr->expr())) {
    return false;
  }

  auto* result_type = TypeOf(expr->expr())->UnwrapPtrIfNeeded();
  SetType(expr, result_type);
  return true;
}

bool Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
  ast::Variable* var = stmt->variable();
  Mark(var);

  // If the variable has a declared type, resolve it.
  std::string type_name;
  const sem::Type* type = nullptr;
  if (auto* ast_ty = var->type()) {
    type_name = ast_ty->FriendlyName(builder_->Symbols());
    type = Type(ast_ty);
    if (!type) {
      return false;
    }
  }

  bool is_global = false;
  if (variable_stack_.get(var->symbol(), nullptr, &is_global)) {
    const char* error_code = is_global ? "v-0013" : "v-0014";
    diagnostics_.add_error(error_code,
                           "redeclared identifier '" +
                               builder_->Symbols().NameFor(var->symbol()) + "'",
                           stmt->source());
    return false;
  }

  if (auto* ctor = stmt->variable()->constructor()) {
    Mark(ctor);
    if (!Expression(ctor)) {
      return false;
    }
    auto* rhs_type = TypeOf(ctor);

    // If the variable has no type, infer it from the rhs
    if (type == nullptr) {
      type_name = TypeNameOf(ctor);
      type = rhs_type->UnwrapPtrIfNeeded();
    }

    if (!IsValidAssignment(type, rhs_type)) {
      diagnostics_.add_error(
          "variable of type '" + type_name +
              "' cannot be initialized with a value of type '" +
              TypeNameOf(ctor) + "'",
          stmt->source());
      return false;
    }
  }

  for (auto* deco : var->decorations()) {
    // TODO(bclayton): Validate decorations
    Mark(deco);
  }

  auto* info = Variable(var, type, type_name);
  if (!info) {
    return false;
  }
  // TODO(bclayton): Remove this and fix tests. We're overriding the semantic
  // type stored in info->type here with a possibly non-canonicalized type.
  info->type = type;
  variable_stack_.set(var->symbol(), info);
  current_block_->decls.push_back(var);

  if (!ValidateVariable(var)) {
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

const sem::Type* Resolver::TypeOf(const ast::Expression* expr) {
  auto it = expr_info_.find(expr);
  if (it != expr_info_.end()) {
    return it->second.type;
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

const sem::Type* Resolver::TypeOf(const ast::Literal* lit) {
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

void Resolver::SetType(ast::Expression* expr, typ::Type type) {
  SetType(expr, type,
          type.sem ? type.sem->FriendlyName(builder_->Symbols())
                   : type.ast->FriendlyName(builder_->Symbols()));
}

void Resolver::SetType(ast::Expression* expr,
                       typ::Type type,
                       const std::string& type_name) {
  if (!type.sem) {
    type.sem = Type(type.ast);
    TINT_ASSERT(type.sem);
  }
  if (expr_info_.count(expr)) {
    TINT_ICE(builder_->Diagnostics())
        << "SetType() called twice for the same expression";
  }
  type = Canonical(type);
  expr_info_.emplace(expr, ExpressionInfo{type, type_name, current_statement_});
}

void Resolver::CreateSemanticNodes() const {
  auto& sem = builder_->Sem();

  // Collate all the 'ancestor_entry_points' - this is a map of function symbol
  // to all the entry points that transitively call the function.
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

  // Create semantic nodes for all ast::Variables
  for (auto it : variable_to_info_) {
    auto* var = it.first;
    auto* info = it.second;
    auto* sem_var =
        builder_->create<sem::Variable>(var, info->type, info->storage_class);
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
          TINT_ICE(builder_->Diagnostics())
              << "expected sem::VariableUser, got "
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
        ancestor_entry_points[func->symbol()]);
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

  // Create semantic nodes for all structs
  for (auto it : struct_info_) {
    auto* str = it.first;
    auto* info = it.second;
    builder_->Sem().Add(
        str, builder_->create<sem::Struct>(
                 const_cast<sem::StructType*>(str), std::move(info->members),
                 info->align, info->size, info->size_no_padding,
                 info->storage_class_usage, info->pipeline_stage_uses));
  }
}

bool Resolver::DefaultAlignAndSize(const sem::Type* ty,
                                   uint32_t& align,
                                   uint32_t& size,
                                   const Source& source) {
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

  auto* cty = Canonical(ty);
  if (cty->is_scalar()) {
    // Note: Also captures booleans, but these are not host-shareable.
    align = 4;
    size = 4;
    return true;
  } else if (auto* vec = cty->As<sem::Vector>()) {
    if (vec->size() < 2 || vec->size() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid vector size: vec" << vec->size();
      return false;
    }
    align = vector_align[vec->size()];
    size = vector_size[vec->size()];
    return true;
  } else if (auto* mat = cty->As<sem::Matrix>()) {
    if (mat->columns() < 2 || mat->columns() > 4 || mat->rows() < 2 ||
        mat->rows() > 4) {
      TINT_UNREACHABLE(diagnostics_)
          << "Invalid matrix size: mat" << mat->columns() << "x" << mat->rows();
      return false;
    }
    align = vector_align[mat->rows()];
    size = vector_align[mat->rows()] * mat->columns();
    return true;
  } else if (auto* s = cty->As<sem::StructType>()) {
    if (auto* si = Structure(s)) {
      align = si->align;
      size = si->size;
      return true;
    }
    return false;
  } else if (cty->Is<sem::ArrayType>()) {
    if (auto* sem =
            Array(ty->UnwrapAliasIfNeeded()->As<sem::ArrayType>(), source)) {
      align = sem->Align();
      size = sem->Size();
      return true;
    }
    return false;
  }
  TINT_UNREACHABLE(diagnostics_) << "Invalid type " << ty->TypeInfo().name;
  return false;
}

const sem::Array* Resolver::Array(const sem::ArrayType* arr,
                                  const Source& source) {
  if (auto* sem = builder_->Sem().Get(arr)) {
    // Semantic info already constructed for this array type
    return sem;
  }

  if (!ValidateArray(arr, source)) {
    return nullptr;
  }

  auto* el_ty = arr->type();

  uint32_t el_align = 0;
  uint32_t el_size = 0;
  if (!DefaultAlignAndSize(el_ty, el_align, el_size, source)) {
    return nullptr;
  }

  auto create_semantic = [&](uint32_t stride) -> sem::Array* {
    auto align = el_align;
    // WebGPU requires runtime arrays have at least one element, but the AST
    // records an element count of 0 for it.
    auto size = std::max<uint32_t>(arr->size(), 1) * stride;
    auto* sem = builder_->create<sem::Array>(const_cast<sem::ArrayType*>(arr),
                                             align, size, stride);
    builder_->Sem().Add(arr, sem);
    return sem;
  };

  // Look for explicit stride via [[stride(n)]] decoration
  uint32_t explicit_stride = 0;
  for (auto* deco : arr->decorations()) {
    Mark(deco);
    if (auto* stride = deco->As<ast::StrideDecoration>()) {
      if (explicit_stride) {
        diagnostics_.add_error(
            "array must have at most one [[stride]] decoration", source);
        return nullptr;
      }
      explicit_stride = stride->stride();
      if (!ValidateArrayStrideDecoration(stride, el_size, el_align, source)) {
        return nullptr;
      }
    }
  }
  if (explicit_stride) {
    return create_semantic(explicit_stride);
  }

  // Calculate implicit stride
  auto implicit_stride = utils::RoundUp(el_align, el_size);
  return create_semantic(implicit_stride);
}

bool Resolver::ValidateArray(const sem::ArrayType* arr, const Source& source) {
  auto* el_ty = arr->type();

  if (!IsStorable(el_ty)) {
    builder_->Diagnostics().add_error(
        el_ty->FriendlyName(builder_->Symbols()) +
            " cannot be used as an element type of an array",
        source);
    return false;
  }

  if (auto* el_str = el_ty->As<sem::StructType>()) {
    if (el_str->impl()->IsBlockDecorated()) {
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

bool Resolver::ValidateStructure(const StructInfo* st) {
  for (auto* member : st->members) {
    if (auto* r = member->Type()->UnwrapAll()->As<sem::ArrayType>()) {
      if (r->IsRuntimeArray()) {
        if (member != st->members.back()) {
          diagnostics_.add_error(
              "v-0015",
              "runtime arrays may only appear as the last member of a struct",
              member->Declaration()->source());
          return false;
        }
        if (!st->type->impl()->IsBlockDecorated()) {
          diagnostics_.add_error(
              "v-0015",
              "a struct containing a runtime-sized array "
              "requires the [[block]] attribute: '" +
                  builder_->Symbols().NameFor(st->type->impl()->name()) + "'",
              member->Declaration()->source());
          return false;
        }

        for (auto* deco : r->decorations()) {
          if (!deco->Is<ast::StrideDecoration>()) {
            diagnostics_.add_error("decoration is not valid for array types",
                                   deco->source());
            return false;
          }
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

  for (auto* deco : st->type->impl()->decorations()) {
    if (!(deco->Is<ast::StructBlockDecoration>())) {
      diagnostics_.add_error("decoration is not valid for struct declarations",
                             deco->source());
      return false;
    }
  }

  return true;
}

Resolver::StructInfo* Resolver::Structure(const sem::StructType* str) {
  auto info_it = struct_info_.find(str);
  if (info_it != struct_info_.end()) {
    // StructInfo already resolved for this structure type
    return info_it->second;
  }

  for (auto* deco : str->impl()->decorations()) {
    Mark(deco);
  }

  sem::StructMemberList sem_members;
  sem_members.reserve(str->impl()->members().size());

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

  for (auto* member : str->impl()->members()) {
    Mark(member);

    // Resolve member type
    auto* type = Type(member->type());
    if (!type) {
      return nullptr;
    }

    // Validate member type
    if (!IsStorable(type)) {
      builder_->Diagnostics().add_error(
          type->FriendlyName(builder_->Symbols()) +
          " cannot be used as the type of a structure member");
      return nullptr;
    }

    uint32_t offset = struct_size;
    uint32_t align = 0;
    uint32_t size = 0;
    if (!DefaultAlignAndSize(type, align, size, member->source())) {
      return nullptr;
    }

    bool has_offset_deco = false;
    bool has_align_deco = false;
    bool has_size_deco = false;
    for (auto* deco : member->decorations()) {
      Mark(deco);
      if (auto* o = deco->As<ast::StructMemberOffsetDecoration>()) {
        // Offset decorations are not part of the WGSL spec, but are emitted by
        // the SPIR-V reader.
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
        member, const_cast<sem::Type*>(type), offset, align, size);
    builder_->Sem().Add(member, sem_member);
    sem_members.emplace_back(sem_member);

    struct_size = offset + size;
    struct_align = std::max(struct_align, align);
  }

  auto size_no_padding = struct_size;
  struct_size = utils::RoundUp(struct_align, struct_size);

  auto* info = struct_infos_.Create();
  info->type = str;
  info->members = std::move(sem_members);
  info->align = struct_align;
  info->size = struct_size;
  info->size_no_padding = size_no_padding;
  struct_info_.emplace(str, info);

  if (!ValidateStructure(info)) {
    return nullptr;
  }

  return info;
}

bool Resolver::ValidateReturn(const ast::ReturnStatement* ret) {
  auto* func_type = current_function_->return_type;

  auto* ret_type = ret->has_value() ? TypeOf(ret->value())->UnwrapAll()
                                    : builder_->ty.void_();

  if (func_type->UnwrapAll() != ret_type) {
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

    // Validate after processing the return value expression so that its type is
    // available for validation
    return Expression(value) && ValidateReturn(ret);
  }

  return true;
}

bool Resolver::ValidateSwitch(const ast::SwitchStatement* s) {
  auto* cond_type = TypeOf(s->condition())->UnwrapAll();
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
    if (!CaseStatement(case_stmt)) {
      return false;
    }
  }
  if (!ValidateSwitch(s)) {
    return false;
  }
  return true;
}

bool Resolver::ValidateAssignment(const ast::AssignmentStatement* a) {
  auto* lhs = a->lhs();
  auto* rhs = a->rhs();

  // TODO(crbug.com/tint/659): This logic needs updating once pointers are
  // pinned down in the WGSL spec.
  auto* lhs_type = TypeOf(lhs)->UnwrapAll();
  auto* rhs_type = TypeOf(rhs);
  if (!IsValidAssignment(lhs_type, rhs_type)) {
    diagnostics_.add_error("invalid assignment: cannot assign value of type '" +
                               rhs_type->FriendlyName(builder_->Symbols()) +
                               "' to a variable of type '" +
                               lhs_type->FriendlyName(builder_->Symbols()) +
                               "'",
                           a->source());
    return false;
  }

  // Pointers are not storable in WGSL, but the right-hand side must be
  // storable. The raw right-hand side might be a pointer value which must be
  // loaded (dereferenced) to provide the value to be stored.
  auto* rhs_result_type = TypeOf(rhs)->UnwrapAll();
  if (!IsStorable(rhs_result_type)) {
    diagnostics_.add_error(
        "v-000x",
        "invalid assignment: right-hand-side is not storable: " +
            TypeOf(rhs)->FriendlyName(builder_->Symbols()),
        a->source());
    return false;
  }

  // lhs must be a pointer or a constant
  auto* lhs_result_type = TypeOf(lhs)->UnwrapIfNeeded();
  if (!lhs_result_type->Is<sem::Pointer>()) {
    // In case lhs is a constant identifier, output a nicer message as it's
    // likely to be a common programmer error.
    if (auto* ident = lhs->As<ast::IdentifierExpression>()) {
      VariableInfo* var;
      if (variable_stack_.get(ident->symbol(), &var) &&
          var->declaration->is_const()) {
        diagnostics_.add_error(
            "v-0021",
            "cannot re-assign a constant: '" +
                builder_->Symbols().NameFor(ident->symbol()) + "'",
            a->source());
        return false;
      }
    }

    // Issue a generic error.
    diagnostics_.add_error(
        "v-000x",
        "invalid assignment: left-hand-side does not reference storage: " +
            TypeOf(lhs)->FriendlyName(builder_->Symbols()),
        a->source());
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

bool Resolver::ApplyStorageClassUsageToType(ast::StorageClass sc,
                                            const sem::Type* ty,
                                            const Source& usage) {
  ty = ty->UnwrapIfNeeded();

  if (auto* str = ty->As<sem::StructType>()) {
    auto* info = Structure(str);
    if (!info) {
      return false;
    }
    if (info->storage_class_usage.count(sc)) {
      return true;  // Already applied
    }
    info->storage_class_usage.emplace(sc);
    for (auto* member : info->members) {
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

  if (auto* arr = ty->As<sem::ArrayType>()) {
    return ApplyStorageClassUsageToType(sc, arr->type(), usage);
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

template <typename F>
bool Resolver::BlockScope(const ast::BlockStatement* block,
                          BlockInfo::Type type,
                          F&& callback) {
  BlockInfo block_info(block, type, current_block_);
  ScopedAssignment<BlockInfo*> sa(current_block_, &block_info);
  variable_stack_.push_scope();
  bool result = callback();
  variable_stack_.pop_scope();
  return result;
}

std::string Resolver::VectorPretty(uint32_t size, sem::Type* element_type) {
  sem::Vector vec_type(element_type, size);
  return vec_type.FriendlyName(builder_->Symbols());
}

const sem::Type* Resolver::Canonical(const sem::Type* type) {
  using AccessControl = sem::AccessControl;
  using Alias = sem::Alias;
  using Matrix = sem::Matrix;
  using Type = sem::Type;
  using Vector = sem::Vector;

  if (!type) {
    TINT_ICE(diagnostics_) << "Canonical() called with nullptr";
    return nullptr;
  }

  std::function<const Type*(const Type*)> make_canonical;
  make_canonical = [&](const Type* t) -> const sem::Type* {
    // Unwrap alias sequence
    const Type* ct = t;
    while (auto* p = ct->As<Alias>()) {
      ct = p->type();
    }

    if (auto* v = ct->As<Vector>()) {
      return builder_->create<Vector>(
          const_cast<sem::Type*>(make_canonical(v->type())), v->size());
    }
    if (auto* m = ct->As<Matrix>()) {
      auto* column_type =
          builder_->create<sem::Vector>(make_canonical(m->type()), m->rows());
      return builder_->create<Matrix>(column_type, m->columns());
    }
    if (auto* ac = ct->As<AccessControl>()) {
      return builder_->create<AccessControl>(ac->access_control(),
                                             make_canonical(ac->type()));
    }
    return ct;
  };

  return utils::GetOrCreate(type_to_canonical_, type,
                            [&] { return make_canonical(type); });
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
                                     const sem::Type* ctype,
                                     const std::string& tn)
    : declaration(decl),
      type(ctype),
      type_name(tn),
      storage_class(decl->declared_storage_class()) {}

Resolver::VariableInfo::~VariableInfo() = default;

Resolver::FunctionInfo::FunctionInfo(ast::Function* decl) : declaration(decl) {}
Resolver::FunctionInfo::~FunctionInfo() = default;

Resolver::StructInfo::StructInfo() = default;
Resolver::StructInfo::~StructInfo() = default;

}  // namespace resolver
}  // namespace tint
