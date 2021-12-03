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
#include "src/utils/math.h"
#include "src/utils/reverse.h"
#include "src/utils/scoped_assignment.h"
#include "src/utils/transform.h"

namespace tint {
namespace resolver {

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder),
      diagnostics_(builder->Diagnostics()),
      intrinsic_table_(IntrinsicTable::Create(*builder)) {}

Resolver::~Resolver() = default;

bool Resolver::Resolve() {
  if (builder_->Diagnostics().contains_errors()) {
    return false;
  }

  if (!DependencyGraph::Build(builder_->AST(), builder_->Symbols(),
                              builder_->Diagnostics(), dependencies_,
                              /* allow_out_of_order_decls*/ false)) {
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

  SetShadows();

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
    if (auto* type = ResolvedSymbol<sem::Type>(ty)) {
      return type;
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

      global->SetConstructor(rhs);

      builder_->Sem().Add(var, global);
      return global;
    }
    case VariableKind::kLocal: {
      auto* local = builder_->create<sem::LocalVariable>(
          var, var_ty, storage_class, access, current_statement_,
          (rhs && var->is_const) ? rhs->ConstantValue() : sem::Constant{});
      builder_->Sem().Add(var, local);
      local->SetConstructor(rhs);
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

void Resolver::SetShadows() {
  for (auto it : dependencies_.shadows) {
    auto* var = Sem(it.first);
    if (auto* local = var->As<sem::LocalVariable>()) {
      local->SetShadows(Sem(it.second));
    }
    if (auto* param = var->As<sem::Parameter>()) {
      param->SetShadows(Sem(it.second));
    }
  }
}  // namespace resolver

bool Resolver::GlobalVariable(const ast::Variable* var) {
  auto* sem = Variable(var, VariableKind::kGlobal);
  if (!sem) {
    return false;
  }

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

sem::Function* Resolver::Function(const ast::Function* decl) {
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

  auto* func = builder_->create<sem::Function>(decl, return_type, parameters);
  builder_->Sem().Add(decl, func);

  TINT_SCOPED_ASSIGNMENT(current_function_, func);

  if (!WorkgroupSize(decl)) {
    return nullptr;
  }

  if (decl->IsEntryPoint()) {
    entry_points_.emplace_back(func);
  }

  if (decl->body) {
    Mark(decl->body);
    if (current_compound_statement_) {
      TINT_ICE(Resolver, diagnostics_)
          << "Resolver::Function() called with a current compound statement";
      return nullptr;
    }
    auto* body = StatementScope(
        decl->body, builder_->create<sem::FunctionBlockStatement>(func),
        [&] { return Statements(decl->body->statements); });
    if (!body) {
      return nullptr;
    }
    func->Behaviors() = body->Behaviors();
    if (func->Behaviors().Contains(sem::Behavior::kReturn)) {
      // https://www.w3.org/TR/WGSL/#behaviors-rules
      // We assign a behavior to each function: it is its body’s behavior
      // (treating the body as a regular statement), with any "Return" replaced
      // by "Next".
      func->Behaviors().Remove(sem::Behavior::kReturn);
      func->Behaviors().Add(sem::Behavior::kNext);
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

  // If this is an entry point, mark all transitively called functions as being
  // used by this entry point.
  if (decl->IsEntryPoint()) {
    for (auto* f : func->TransitivelyCalledFunctions()) {
      const_cast<sem::Function*>(f)->AddAncestorEntryPoint(func);
    }
  }

  return func;
}

bool Resolver::WorkgroupSize(const ast::Function* func) {
  // Set work-group size defaults.
  sem::WorkgroupSize ws;
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

  current_function_->SetWorkgroupSize(std::move(ws));
  return true;
}

bool Resolver::Statements(const ast::StatementList& stmts) {
  sem::Behaviors behaviors{sem::Behavior::kNext};

  bool reachable = true;
  for (auto* stmt : stmts) {
    Mark(stmt);
    auto* sem = Statement(stmt);
    if (!sem) {
      return false;
    }
    // s1 s2:(B1∖{Next}) ∪ B2
    sem->SetIsReachable(reachable);
    if (reachable) {
      behaviors = (behaviors - sem::Behavior::kNext) + sem->Behaviors();
    }
    reachable = reachable && sem->Behaviors().Contains(sem::Behavior::kNext);
  }

  current_statement_->Behaviors() = behaviors;

  if (!ValidateStatements(stmts)) {
    return false;
  }

  return true;
}

sem::Statement* Resolver::Statement(const ast::Statement* stmt) {
  if (stmt->Is<ast::CaseStatement>()) {
    AddError("case statement can only be used inside a switch statement",
             stmt->source);
    return nullptr;
  }
  if (stmt->Is<ast::ElseStatement>()) {
    TINT_ICE(Resolver, diagnostics_)
        << "Resolver::Statement() encountered an Else statement. Else "
           "statements are embedded in If statements, so should never be "
           "encountered as top-level statements";
    return nullptr;
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
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return AssignmentStatement(a);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return BreakStatement(b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    return CallStatement(c);
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return ContinueStatement(c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return DiscardStatement(d);
  }
  if (auto* f = stmt->As<ast::FallthroughStatement>()) {
    return FallthroughStatement(f);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return ReturnStatement(r);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return VariableDeclStatement(v);
  }

  AddError("unknown statement type: " + std::string(stmt->TypeInfo().name),
           stmt->source);
  return nullptr;
}

sem::CaseStatement* Resolver::CaseStatement(const ast::CaseStatement* stmt) {
  auto* sem = builder_->create<sem::CaseStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    for (auto* sel : stmt->selectors) {
      Mark(sel);
    }
    Mark(stmt->body);
    auto* body = BlockStatement(stmt->body);
    if (!body) {
      return false;
    }
    sem->SetBlock(body);
    sem->Behaviors() = body->Behaviors();
    return true;
  });
}

sem::IfStatement* Resolver::IfStatement(const ast::IfStatement* stmt) {
  auto* sem = builder_->create<sem::IfStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    auto* cond = Expression(stmt->condition);
    if (!cond) {
      return false;
    }
    sem->SetCondition(cond);
    sem->Behaviors() = cond->Behaviors();
    sem->Behaviors().Remove(sem::Behavior::kNext);

    Mark(stmt->body);
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    if (!StatementScope(stmt->body, body,
                        [&] { return Statements(stmt->body->statements); })) {
      return false;
    }
    sem->Behaviors().Add(body->Behaviors());

    for (auto* else_stmt : stmt->else_statements) {
      Mark(else_stmt);
      auto* else_sem = ElseStatement(else_stmt);
      if (!else_sem) {
        return false;
      }
      sem->Behaviors().Add(else_sem->Behaviors());
    }

    if (stmt->else_statements.empty() ||
        stmt->else_statements.back()->condition != nullptr) {
      // https://www.w3.org/TR/WGSL/#behaviors-rules
      // if statements without an else branch are treated as if they had an
      // empty else branch (which adds Next to their behavior)
      sem->Behaviors().Add(sem::Behavior::kNext);
    }

    return ValidateIfStatement(sem);
  });
}

sem::ElseStatement* Resolver::ElseStatement(const ast::ElseStatement* stmt) {
  auto* sem = builder_->create<sem::ElseStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    if (auto* cond_expr = stmt->condition) {
      auto* cond = Expression(cond_expr);
      if (!cond) {
        return false;
      }
      sem->SetCondition(cond);
      // https://www.w3.org/TR/WGSL/#behaviors-rules
      // if statements with else if branches are treated as if they were nested
      // simple if/else statements
      sem->Behaviors() = cond->Behaviors();
    }
    sem->Behaviors().Remove(sem::Behavior::kNext);

    Mark(stmt->body);
    auto* body = builder_->create<sem::BlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    if (!StatementScope(stmt->body, body,
                        [&] { return Statements(stmt->body->statements); })) {
      return false;
    }
    sem->Behaviors().Add(body->Behaviors());

    return ValidateElseStatement(sem);
  });
}

sem::BlockStatement* Resolver::BlockStatement(const ast::BlockStatement* stmt) {
  auto* sem = builder_->create<sem::BlockStatement>(
      stmt->As<ast::BlockStatement>(), current_compound_statement_,
      current_function_);
  return StatementScope(stmt, sem,
                        [&] { return Statements(stmt->statements); });
}

sem::LoopStatement* Resolver::LoopStatement(const ast::LoopStatement* stmt) {
  auto* sem = builder_->create<sem::LoopStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    Mark(stmt->body);

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    return StatementScope(stmt->body, body, [&] {
      if (!Statements(stmt->body->statements)) {
        return false;
      }
      auto& behaviors = sem->Behaviors();
      behaviors = body->Behaviors();

      if (stmt->continuing) {
        Mark(stmt->continuing);
        if (!stmt->continuing->Empty()) {
          auto* continuing = StatementScope(
              stmt->continuing,
              builder_->create<sem::LoopContinuingBlockStatement>(
                  stmt->continuing, current_compound_statement_,
                  current_function_),
              [&] { return Statements(stmt->continuing->statements); });
          if (!continuing) {
            return false;
          }
          behaviors.Add(continuing->Behaviors());
        }
      }

      if (behaviors.Contains(sem::Behavior::kBreak)) {  // Does the loop exit?
        behaviors.Add(sem::Behavior::kNext);
      } else {
        behaviors.Remove(sem::Behavior::kNext);
      }
      behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kContinue);

      return true;
    });
  });
}

sem::ForLoopStatement* Resolver::ForLoopStatement(
    const ast::ForLoopStatement* stmt) {
  auto* sem = builder_->create<sem::ForLoopStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    auto& behaviors = sem->Behaviors();
    if (auto* initializer = stmt->initializer) {
      Mark(initializer);
      auto* init = Statement(initializer);
      if (!init) {
        return false;
      }
      behaviors.Add(init->Behaviors());
    }

    if (auto* cond_expr = stmt->condition) {
      auto* cond = Expression(cond_expr);
      if (!cond) {
        return false;
      }
      sem->SetCondition(cond);
      behaviors.Add(cond->Behaviors());
    }

    if (auto* continuing = stmt->continuing) {
      Mark(continuing);
      auto* cont = Statement(continuing);
      if (!cont) {
        return false;
      }
      behaviors.Add(cont->Behaviors());
    }

    Mark(stmt->body);

    auto* body = builder_->create<sem::LoopBlockStatement>(
        stmt->body, current_compound_statement_, current_function_);
    if (!StatementScope(stmt->body, body,
                        [&] { return Statements(stmt->body->statements); })) {
      return false;
    }

    behaviors.Add(body->Behaviors());
    if (stmt->condition ||
        behaviors.Contains(sem::Behavior::kBreak)) {  // Does the loop exit?
      behaviors.Add(sem::Behavior::kNext);
    } else {
      behaviors.Remove(sem::Behavior::kNext);
    }
    behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kContinue);

    return ValidateForLoopStatement(sem);
  });
}

sem::Expression* Resolver::Expression(const ast::Expression* root) {
  std::vector<const ast::Expression*> sorted;
  bool mark_failed = false;
  if (!ast::TraverseExpressions<ast::TraverseOrder::RightToLeft>(
          root, diagnostics_, [&](const ast::Expression* expr) {
            if (!Mark(expr)) {
              mark_failed = true;
              return ast::TraverseAction::Stop;
            }
            sorted.emplace_back(expr);
            return ast::TraverseAction::Descend;
          })) {
    return nullptr;
  }

  if (mark_failed) {
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

    // https://www.w3.org/TR/WGSL/#behaviors-rules
    // an expression behavior is always either {Next} or {Next, Discard}
    if (sem_expr->Behaviors() != sem::Behavior::kNext &&
        sem_expr->Behaviors() != sem::Behaviors{sem::Behavior::kNext,  // NOLINT
                                                sem::Behavior::kDiscard} &&
        !IsCallStatement(expr)) {
      TINT_ICE(Resolver, diagnostics_)
          << expr->TypeInfo().name
          << " behaviors are: " << sem_expr->Behaviors();
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
  auto* idx = Sem(expr->index);
  auto* obj = Sem(expr->object);
  auto* obj_raw_ty = obj->Type();
  auto* obj_ty = obj_raw_ty->UnwrapRef();
  const sem::Type* ty = nullptr;
  if (auto* arr = obj_ty->As<sem::Array>()) {
    ty = arr->ElemType();
  } else if (auto* vec = obj_ty->As<sem::Vector>()) {
    ty = vec->type();
  } else if (auto* mat = obj_ty->As<sem::Matrix>()) {
    ty = builder_->create<sem::Vector>(mat->type(), mat->rows());
  } else {
    AddError("cannot index type '" + TypeNameOf(obj_ty) + "'", expr->source);
    return nullptr;
  }

  auto* idx_ty = idx->Type()->UnwrapRef();
  if (!idx_ty->IsAnyOf<sem::I32, sem::U32>()) {
    AddError("index must be of type 'i32' or 'u32', found: '" +
                 TypeNameOf(idx_ty) + "'",
             idx->Declaration()->source);
    return nullptr;
  }

  if (obj_ty->IsAnyOf<sem::Array, sem::Matrix>()) {
    if (!obj_raw_ty->Is<sem::Reference>()) {
      // TODO(bclayton): expand this to allow any const_expr expression
      // https://github.com/gpuweb/gpuweb/issues/1272
      if (!idx->Declaration()->As<ast::IntLiteralExpression>()) {
        AddError("index must be signed or unsigned integer literal",
                 idx->Declaration()->source);
        return nullptr;
      }
    }
  }

  // If we're extracting from a reference, we return a reference.
  if (auto* ref = obj_raw_ty->As<sem::Reference>()) {
    ty = builder_->create<sem::Reference>(ty, ref->StorageClass(),
                                          ref->Access());
  }

  auto val = EvaluateConstantValue(expr, ty);
  auto* sem =
      builder_->create<sem::Expression>(expr, ty, current_statement_, val);
  sem->Behaviors() = idx->Behaviors() + obj->Behaviors();
  return sem;
}

sem::Expression* Resolver::Bitcast(const ast::BitcastExpression* expr) {
  auto* inner = Sem(expr->expr);
  auto* ty = Type(expr->type);
  if (!ty) {
    return nullptr;
  }

  auto val = EvaluateConstantValue(expr, ty);
  auto* sem =
      builder_->create<sem::Expression>(expr, ty, current_statement_, val);

  sem->Behaviors() = inner->Behaviors();

  if (!ValidateBitcast(expr, ty)) {
    return nullptr;
  }

  return sem;
}

sem::Call* Resolver::Call(const ast::CallExpression* expr) {
  std::vector<const sem::Expression*> args(expr->args.size());
  std::vector<const sem::Type*> arg_tys(args.size());
  sem::Behaviors arg_behaviors;

  for (size_t i = 0; i < expr->args.size(); i++) {
    auto* arg = Sem(expr->args[i]);
    if (!arg) {
      return nullptr;
    }
    args[i] = arg;
    arg_tys[i] = args[i]->Type();
    arg_behaviors.Add(arg->Behaviors());
  }

  arg_behaviors.Remove(sem::Behavior::kNext);

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

  auto* resolved = ResolvedSymbol(ident);
  if (auto* ty = As<sem::Type>(resolved)) {
    return type_ctor_or_conv(ty);
  }

  if (auto* fn = As<sem::Function>(resolved)) {
    return FunctionCall(expr, fn, std::move(args), arg_behaviors);
  }

  auto name = builder_->Symbols().NameFor(ident->symbol);
  auto intrinsic_type = sem::ParseIntrinsicType(name);
  if (intrinsic_type != sem::IntrinsicType::kNone) {
    return IntrinsicCall(expr, intrinsic_type, std::move(args),
                         std::move(arg_tys));
  }

  TINT_ICE(Resolver, diagnostics_)
      << expr->source << " unresolved CallExpression target:\n"
      << "resolved: " << (resolved ? resolved->TypeInfo().name : "<null>")
      << "\n"
      << "name: " << builder_->Symbols().NameFor(ident->symbol);
  return nullptr;
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

  current_function_->AddDirectCall(call);

  return call;
}

sem::Call* Resolver::FunctionCall(
    const ast::CallExpression* expr,
    sem::Function* target,
    const std::vector<const sem::Expression*> args,
    sem::Behaviors arg_behaviors) {
  auto sym = expr->target.name->symbol;
  auto name = builder_->Symbols().NameFor(sym);

  auto* call = builder_->create<sem::Call>(expr, target, std::move(args),
                                           current_statement_, sem::Constant{});

  if (current_function_) {
    // Note: Requires called functions to be resolved first.
    // This is currently guaranteed as functions must be declared before
    // use.
    current_function_->AddTransitivelyCalledFunction(target);
    current_function_->AddDirectCall(call);
    for (auto* transitive_call : target->TransitivelyCalledFunctions()) {
      current_function_->AddTransitivelyCalledFunction(transitive_call);
    }

    // We inherit any referenced variables from the callee.
    for (auto* var : target->TransitivelyReferencedGlobals()) {
      current_function_->AddTransitivelyReferencedGlobal(var);
    }
  }

  target->AddCallSite(call);

  call->Behaviors() = arg_behaviors + target->Behaviors();

  if (!ValidateFunctionCall(call)) {
    return nullptr;
  }

  return call;
}

sem::Call* Resolver::TypeConversion(const ast::CallExpression* expr,
                                    const sem::Type* target,
                                    const sem::Expression* arg,
                                    const sem::Type* source) {
  // It is not valid to have a type-cast call expression inside a call
  // statement.
  if (IsCallStatement(expr)) {
    AddError("type cast evaluated but not used", expr->source);
    return nullptr;
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
  if (IsCallStatement(expr)) {
    AddError("type constructor evaluated but not used", expr->source);
    return nullptr;
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

sem::Expression* Resolver::Identifier(const ast::IdentifierExpression* expr) {
  auto symbol = expr->symbol;
  auto* resolved = ResolvedSymbol(expr);
  if (auto* var = As<sem::Variable>(resolved)) {
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

  if (Is<sem::Function>(resolved)) {
    AddError("missing '(' for function call", expr->source.End());
    return nullptr;
  }

  if (IsIntrinsic(symbol)) {
    AddError("missing '(' for intrinsic call", expr->source.End());
    return nullptr;
  }

  if (resolved->Is<sem::Type>()) {
    AddError("missing '(' for type constructor or cast", expr->source.End());
    return nullptr;
  }

  TINT_ICE(Resolver, diagnostics_)
      << expr->source << " unresolved identifier:\n"
      << "resolved: " << (resolved ? resolved->TypeInfo().name : "<null>")
      << "\n"
      << "name: " << builder_->Symbols().NameFor(symbol);
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

  auto* lhs = Sem(expr->lhs);
  auto* rhs = Sem(expr->rhs);

  auto* lhs_ty = lhs->Type()->UnwrapRef();
  auto* rhs_ty = rhs->Type()->UnwrapRef();

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
    auto* sem =
        builder_->create<sem::Expression>(expr, ty, current_statement_, val);
    sem->Behaviors() = lhs->Behaviors() + rhs->Behaviors();
    return sem;
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
  auto* expr = Sem(unary->expr);
  auto* expr_ty = expr->Type();
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
  auto* sem =
      builder_->create<sem::Expression>(unary, ty, current_statement_, val);
  sem->Behaviors() = expr->Behaviors();
  return sem;
}

sem::Type* Resolver::TypeDecl(const ast::TypeDecl* named_type) {
  sem::Type* result = nullptr;
  if (auto* alias = named_type->As<ast::Alias>()) {
    result = Alias(alias);
  } else if (auto* str = named_type->As<ast::Struct>()) {
    result = Structure(str);
  } else {
    TINT_UNREACHABLE(Resolver, diagnostics_) << "Unhandled TypeDecl";
  }

  if (!result) {
    return nullptr;
  }

  builder_->Sem().Add(named_type, result);
  return result;
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
      auto* var = ResolvedSymbol<sem::Variable>(ident);
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

sem::Type* Resolver::Alias(const ast::Alias* alias) {
  auto* ty = Type(alias->type);
  if (!ty) {
    return nullptr;
  }
  if (!ValidateAlias(alias)) {
    return nullptr;
  }
  return ty;
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

sem::Statement* Resolver::ReturnStatement(const ast::ReturnStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    auto& behaviors = current_statement_->Behaviors();
    behaviors = sem::Behavior::kReturn;

    if (auto* value = stmt->value) {
      auto* expr = Expression(value);
      if (!expr) {
        return false;
      }
      behaviors.Add(expr->Behaviors() - sem::Behavior::kNext);
    }

    // Validate after processing the return value expression so that its type is
    // available for validation.
    return ValidateReturn(stmt);
  });
}

sem::SwitchStatement* Resolver::SwitchStatement(
    const ast::SwitchStatement* stmt) {
  auto* sem = builder_->create<sem::SwitchStatement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    auto& behaviors = sem->Behaviors();

    auto* cond = Expression(stmt->condition);
    if (!cond) {
      return false;
    }
    behaviors = cond->Behaviors() - sem::Behavior::kNext;

    for (auto* case_stmt : stmt->body) {
      Mark(case_stmt);
      auto* c = CaseStatement(case_stmt);
      if (!c) {
        return false;
      }
      behaviors.Add(c->Behaviors());
    }

    if (behaviors.Contains(sem::Behavior::kBreak)) {
      behaviors.Add(sem::Behavior::kNext);
    }
    behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kFallthrough);

    return ValidateSwitch(stmt);
  });
}

sem::Statement* Resolver::VariableDeclStatement(
    const ast::VariableDeclStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    Mark(stmt->variable);

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

    if (current_block_) {  // Not all statements are inside a block
      current_block_->AddDecl(stmt->variable);
    }

    if (auto* ctor = var->Constructor()) {
      sem->Behaviors() = ctor->Behaviors();
    }

    return ValidateVariable(var);
  });
}

sem::Statement* Resolver::AssignmentStatement(
    const ast::AssignmentStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    auto* lhs = Expression(stmt->lhs);
    if (!lhs) {
      return false;
    }

    auto* rhs = Expression(stmt->rhs);
    if (!rhs) {
      return false;
    }

    auto& behaviors = sem->Behaviors();
    behaviors = rhs->Behaviors();
    if (!stmt->lhs->Is<ast::PhonyExpression>()) {
      behaviors.Add(lhs->Behaviors());
    }

    return ValidateAssignment(stmt);
  });
}

sem::Statement* Resolver::BreakStatement(const ast::BreakStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    sem->Behaviors() = sem::Behavior::kBreak;

    return ValidateBreakStatement(sem);
  });
}

sem::Statement* Resolver::CallStatement(const ast::CallStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    if (auto* expr = Expression(stmt->expr)) {
      sem->Behaviors() = expr->Behaviors();
      return true;
    }
    return false;
  });
}

sem::Statement* Resolver::ContinueStatement(
    const ast::ContinueStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    sem->Behaviors() = sem::Behavior::kContinue;

    // Set if we've hit the first continue statement in our parent loop
    if (auto* block = sem->FindFirstParent<sem::LoopBlockStatement>()) {
      if (!block->FirstContinue()) {
        const_cast<sem::LoopBlockStatement*>(block)->SetFirstContinue(
            stmt, block->Decls().size());
      }
    }

    return ValidateContinueStatement(sem);
  });
}

sem::Statement* Resolver::DiscardStatement(const ast::DiscardStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    sem->Behaviors() = sem::Behavior::kDiscard;
    current_function_->SetHasDiscard();

    return ValidateDiscardStatement(sem);
  });
}

sem::Statement* Resolver::FallthroughStatement(
    const ast::FallthroughStatement* stmt) {
  auto* sem = builder_->create<sem::Statement>(
      stmt, current_compound_statement_, current_function_);
  return StatementScope(stmt, sem, [&] {
    sem->Behaviors() = sem::Behavior::kFallthrough;

    return ValidateFallthroughStatement(sem);
  });
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

template <typename SEM, typename F>
SEM* Resolver::StatementScope(const ast::Statement* ast,
                              SEM* sem,
                              F&& callback) {
  builder_->Sem().Add(ast, sem);

  auto* as_compound =
      As<sem::CompoundStatement, CastFlags::kDontErrorOnImpossibleCast>(sem);
  auto* as_block =
      As<sem::BlockStatement, CastFlags::kDontErrorOnImpossibleCast>(sem);

  TINT_SCOPED_ASSIGNMENT(current_statement_, sem);
  TINT_SCOPED_ASSIGNMENT(
      current_compound_statement_,
      as_compound ? as_compound : current_compound_statement_);
  TINT_SCOPED_ASSIGNMENT(current_block_, as_block ? as_block : current_block_);

  if (!callback()) {
    return nullptr;
  }

  return sem;
}

std::string Resolver::VectorPretty(uint32_t size,
                                   const sem::Type* element_type) {
  sem::Vector vec_type(element_type, size);
  return vec_type.FriendlyName(builder_->Symbols());
}

bool Resolver::Mark(const ast::Node* node) {
  if (node == nullptr) {
    TINT_ICE(Resolver, diagnostics_) << "Resolver::Mark() called with nullptr";
    return false;
  }
  if (marked_.emplace(node).second) {
    return true;
  }
  TINT_ICE(Resolver, diagnostics_)
      << "AST node '" << node->TypeInfo().name
      << "' was encountered twice in the same AST of a Program\n"
      << "At: " << node->source << "\n"
      << "Pointer: " << node;
  return false;
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

bool Resolver::IsIntrinsic(Symbol symbol) const {
  std::string name = builder_->Symbols().NameFor(symbol);
  return sem::ParseIntrinsicType(name) != sem::IntrinsicType::kNone;
}

bool Resolver::IsCallStatement(const ast::Expression* expr) const {
  return current_statement_ &&
         Is<ast::CallStatement>(current_statement_->Declaration(),
                                [&](auto* stmt) { return stmt->expr == expr; });
}

const ast::Statement* Resolver::ClosestContinuing(bool stop_at_loop) const {
  for (const auto* s = current_statement_; s != nullptr; s = s->Parent()) {
    if (stop_at_loop && s->Is<sem::LoopStatement>()) {
      break;
    }
    if (s->Is<sem::LoopContinuingBlockStatement>()) {
      return s->Declaration();
    }
    if (auto* f = As<sem::ForLoopStatement>(s->Parent())) {
      if (f->Declaration()->continuing == s->Declaration()) {
        return s->Declaration();
      }
      if (stop_at_loop) {
        break;
      }
    }
  }
  return nullptr;
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
