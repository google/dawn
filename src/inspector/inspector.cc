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

#include "src/inspector/inspector.h"

#include <utility>

#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/module.h"
#include "src/ast/override_decoration.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"
#include "src/sem/array.h"
#include "src/sem/call.h"
#include "src/sem/f32_type.h"
#include "src/sem/function.h"
#include "src/sem/i32_type.h"
#include "src/sem/matrix_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/u32_type.h"
#include "src/sem/variable.h"
#include "src/sem/vector_type.h"
#include "src/sem/void_type.h"

namespace tint {
namespace inspector {

namespace {

void AppendResourceBindings(std::vector<ResourceBinding>* dest,
                            const std::vector<ResourceBinding>& orig) {
  TINT_ASSERT(Inspector, dest);
  if (!dest) {
    return;
  }

  dest->reserve(dest->size() + orig.size());
  dest->insert(dest->end(), orig.begin(), orig.end());
}

}  // namespace

Inspector::Inspector(const Program* program) : program_(program) {}

Inspector::~Inspector() = default;

std::vector<EntryPoint> Inspector::GetEntryPoints() {
  std::vector<EntryPoint> result;

  for (auto* func : program_->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    auto* sem = program_->Sem().Get(func);

    EntryPoint entry_point;
    entry_point.name = program_->Symbols().NameFor(func->symbol());
    entry_point.remapped_name = program_->Symbols().NameFor(func->symbol());
    entry_point.stage = func->pipeline_stage();

    auto wgsize = sem->workgroup_size();
    entry_point.workgroup_size_x = wgsize[0].value;
    entry_point.workgroup_size_y = wgsize[1].value;
    entry_point.workgroup_size_z = wgsize[2].value;
    if (wgsize[0].overridable_const || wgsize[1].overridable_const ||
        wgsize[2].overridable_const) {
      // TODO(crbug.com/tint/713): Handle overridable constants.
      TINT_ASSERT(Inspector, false);
    }

    for (auto* param : sem->Parameters()) {
      AddEntryPointInOutVariables(
          program_->Symbols().NameFor(param->Declaration()->symbol()),
          param->Type(), param->Declaration()->decorations(),
          entry_point.input_variables);
    }

    if (!sem->ReturnType()->Is<sem::Void>()) {
      AddEntryPointInOutVariables("<retval>", sem->ReturnType(),
                                  func->return_type_decorations(),
                                  entry_point.output_variables);
    }

    for (auto* var : sem->ReferencedModuleVariables()) {
      auto* decl = var->Declaration();

      auto name = program_->Symbols().NameFor(decl->symbol());
      if (ast::HasDecoration<ast::BuiltinDecoration>(decl->decorations())) {
        continue;
      }

      // TODO(crbug.com/tint/697): Remove this.
      {
        StageVariable stage_variable;
        stage_variable.name = name;

        stage_variable.component_type = ComponentType::kUnknown;
        auto* type = var->Type()->UnwrapRef();
        if (type->is_float_scalar_or_vector() || type->is_float_matrix()) {
          stage_variable.component_type = ComponentType::kFloat;
        } else if (type->is_unsigned_scalar_or_vector()) {
          stage_variable.component_type = ComponentType::kUInt;
        } else if (type->is_signed_scalar_or_vector()) {
          stage_variable.component_type = ComponentType::kSInt;
        }

        auto* location_decoration =
            ast::GetDecoration<ast::LocationDecoration>(decl->decorations());
        if (location_decoration) {
          stage_variable.has_location_decoration = true;
          stage_variable.location_decoration = location_decoration->value();
        } else {
          stage_variable.has_location_decoration = false;
        }

        if (var->StorageClass() == ast::StorageClass::kInput) {
          entry_point.input_variables.push_back(stage_variable);
        } else if (var->StorageClass() == ast::StorageClass::kOutput) {
          entry_point.output_variables.push_back(stage_variable);
        }
      }

      {
        if (var->IsPipelineConstant()) {
          OverridableConstant overridable_constant;
          overridable_constant.name = name;
          entry_point.overridable_constants.push_back(overridable_constant);
        }
      }
    }

    result.push_back(std::move(entry_point));
  }

  return result;
}

std::string Inspector::GetRemappedNameForEntryPoint(
    const std::string& entry_point) {
  // TODO(rharrison): Reenable once all of the backends are using the renamed
  //                  entry points.

  //  auto* func = FindEntryPointByName(entry_point);
  //  if (!func) {
  //    return {};
  //  }
  //  return func->name();
  return entry_point;
}

std::map<uint32_t, Scalar> Inspector::GetConstantIDs() {
  std::map<uint32_t, Scalar> result;
  for (auto* var : program_->AST().GlobalVariables()) {
    auto* sem_var = program_->Sem().Get(var);
    if (!sem_var->IsPipelineConstant()) {
      continue;
    }

    // If there are conflicting defintions for a constant id, that is invalid
    // WGSL, so the resolver should catch it. Thus here the inspector just
    // assumes all definitions of the constant id are the same, so only needs
    // to find the first reference to constant id.
    uint32_t constant_id = sem_var->ConstantId();
    if (result.find(constant_id) != result.end()) {
      continue;
    }

    if (!var->has_constructor()) {
      result[constant_id] = Scalar();
      continue;
    }

    auto* expression = var->constructor();
    auto* constructor = expression->As<ast::ConstructorExpression>();
    if (constructor == nullptr) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    auto* scalar_constructor =
        constructor->As<ast::ScalarConstructorExpression>();
    if (scalar_constructor == nullptr) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    auto* literal = scalar_constructor->literal();
    if (!literal) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    if (auto* l = literal->As<ast::BoolLiteral>()) {
      result[constant_id] = Scalar(l->IsTrue());
      continue;
    }

    if (auto* l = literal->As<ast::UintLiteral>()) {
      result[constant_id] = Scalar(l->value());
      continue;
    }

    if (auto* l = literal->As<ast::SintLiteral>()) {
      result[constant_id] = Scalar(l->value());
      continue;
    }

    if (auto* l = literal->As<ast::FloatLiteral>()) {
      result[constant_id] = Scalar(l->value());
      continue;
    }

    result[constant_id] = Scalar();
  }

  return result;
}

std::map<std::string, uint32_t> Inspector::GetConstantNameToIdMap() {
  std::map<std::string, uint32_t> result;
  for (auto* var : program_->AST().GlobalVariables()) {
    auto* sem_var = program_->Sem().Get(var);
    if (sem_var->IsPipelineConstant()) {
      auto name = program_->Symbols().NameFor(var->symbol());
      result[name] = sem_var->ConstantId();
    }
  }
  return result;
}

std::vector<ResourceBinding> Inspector::GetResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;

  AppendResourceBindings(&result,
                         GetUniformBufferResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetStorageBufferResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetReadOnlyStorageBufferResourceBindings(entry_point));
  AppendResourceBindings(&result, GetSamplerResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetComparisonSamplerResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetSampledTextureResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetMultisampledTextureResourceBindings(entry_point));
  AppendResourceBindings(
      &result, GetReadOnlyStorageTextureResourceBindings(entry_point));
  AppendResourceBindings(
      &result, GetWriteOnlyStorageTextureResourceBindings(entry_point));
  AppendResourceBindings(&result, GetDepthTextureResourceBindings(entry_point));
  AppendResourceBindings(&result,
                         GetExternalTextureResourceBindings(entry_point));
  return result;
}

std::vector<ResourceBinding> Inspector::GetUniformBufferResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;

  auto* func_sem = program_->Sem().Get(func);
  for (auto& ruv : func_sem->ReferencedUniformVariables()) {
    auto* var = ruv.first;
    auto binding_info = ruv.second;

    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* str = unwrapped_type->As<sem::Struct>();
    if (str == nullptr) {
      continue;
    }

    if (!str->IsBlockDecorated()) {
      continue;
    }

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kUniformBuffer;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.size = str->Size();
    entry.size_no_padding = str->SizeNoPadding();

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindings(
    const std::string& entry_point) {
  return GetStorageBufferResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding>
Inspector::GetReadOnlyStorageBufferResourceBindings(
    const std::string& entry_point) {
  return GetStorageBufferResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding> Inspector::GetSamplerResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  GenerateSamplerTargets();

  std::vector<ResourceBinding> result;

  auto* func_sem = program_->Sem().Get(func);
  for (auto& rs : func_sem->ReferencedSamplerVariables()) {
    auto binding_info = rs.second;

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kSampler;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetComparisonSamplerResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  GenerateSamplerTargets();

  std::vector<ResourceBinding> result;

  auto* func_sem = program_->Sem().Get(func);
  for (auto& rcs : func_sem->ReferencedComparisonSamplerVariables()) {
    auto binding_info = rcs.second;

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kComparisonSampler;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetSampledTextureResourceBindings(
    const std::string& entry_point) {
  return GetSampledTextureResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding> Inspector::GetMultisampledTextureResourceBindings(
    const std::string& entry_point) {
  return GetSampledTextureResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding>
Inspector::GetReadOnlyStorageTextureResourceBindings(
    const std::string& entry_point) {
  return GetStorageTextureResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding>
Inspector::GetWriteOnlyStorageTextureResourceBindings(
    const std::string& entry_point) {
  return GetStorageTextureResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding> Inspector::GetDepthTextureResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;
  auto* func_sem = program_->Sem().Get(func);
  for (auto& ref : func_sem->ReferencedDepthTextureVariables()) {
    auto* var = ref.first;
    auto binding_info = ref.second;

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kDepthTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = var->Type()->UnwrapRef()->As<sem::Texture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetExternalTextureResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;
  auto* func_sem = program_->Sem().Get(func);
  for (auto& ref : func_sem->ReferencedExternalTextureVariables()) {
    auto* var = ref.first;
    auto binding_info = ref.second;

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kExternalTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = var->Type()->UnwrapRef()->As<sem::Texture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    result.push_back(entry);
  }
  return result;
}

std::vector<SamplerTexturePair> Inspector::GetSamplerTextureUses(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  GenerateSamplerTargets();

  auto it = sampler_targets_->find(entry_point);
  if (it == sampler_targets_->end()) {
    return {};
  }
  return it->second;
}

ast::Function* Inspector::FindEntryPointByName(const std::string& name) {
  auto* func = program_->AST().Functions().Find(program_->Symbols().Get(name));
  if (!func) {
    error_ += name + " was not found!";
    return nullptr;
  }

  if (!func->IsEntryPoint()) {
    error_ += name + " is not an entry point!";
    return nullptr;
  }

  return func;
}

void Inspector::AddEntryPointInOutVariables(
    std::string name,
    sem::Type* type,
    const ast::DecorationList& decorations,
    std::vector<StageVariable>& variables) const {
  // Skip builtins.
  if (ast::HasDecoration<ast::BuiltinDecoration>(decorations)) {
    return;
  }

  auto* unwrapped_type = type->UnwrapRef();

  if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
    // Recurse into members.
    for (auto* member : struct_ty->Members()) {
      AddEntryPointInOutVariables(
          name + "." +
              program_->Symbols().NameFor(member->Declaration()->symbol()),
          member->Type(), member->Declaration()->decorations(), variables);
    }
    return;
  }

  // Base case: add the variable.

  StageVariable stage_variable;
  stage_variable.name = name;
  stage_variable.component_type = ComponentType::kUnknown;
  if (unwrapped_type->is_float_scalar_or_vector() ||
      unwrapped_type->is_float_matrix()) {
    stage_variable.component_type = ComponentType::kFloat;
  } else if (unwrapped_type->is_unsigned_scalar_or_vector()) {
    stage_variable.component_type = ComponentType::kUInt;
  } else if (unwrapped_type->is_signed_scalar_or_vector()) {
    stage_variable.component_type = ComponentType::kSInt;
  }

  auto* location = ast::GetDecoration<ast::LocationDecoration>(decorations);
  TINT_ASSERT(Inspector, location != nullptr);
  stage_variable.has_location_decoration = true;
  stage_variable.location_decoration = location->value();

  variables.push_back(stage_variable);
}

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindingsImpl(
    const std::string& entry_point,
    bool read_only) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  auto* func_sem = program_->Sem().Get(func);
  std::vector<ResourceBinding> result;
  for (auto& rsv : func_sem->ReferencedStorageBufferVariables()) {
    auto* var = rsv.first;
    auto binding_info = rsv.second;

    if (read_only != (var->Access() == ast::Access::kRead)) {
      continue;
    }

    auto* str = var->Type()->UnwrapRef()->As<sem::Struct>();
    if (!str) {
      continue;
    }

    ResourceBinding entry;
    entry.resource_type =
        read_only ? ResourceBinding::ResourceType::kReadOnlyStorageBuffer
                  : ResourceBinding::ResourceType::kStorageBuffer;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.size = str->Size();
    entry.size_no_padding = str->SizeNoPadding();

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetSampledTextureResourceBindingsImpl(
    const std::string& entry_point,
    bool multisampled_only) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;
  auto* func_sem = program_->Sem().Get(func);
  auto referenced_variables =
      multisampled_only ? func_sem->ReferencedMultisampledTextureVariables()
                        : func_sem->ReferencedSampledTextureVariables();
  for (auto& ref : referenced_variables) {
    auto* var = ref.first;
    auto binding_info = ref.second;

    ResourceBinding entry;
    entry.resource_type =
        multisampled_only ? ResourceBinding::ResourceType::kMultisampledTexture
                          : ResourceBinding::ResourceType::kSampledTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = var->Type()->UnwrapRef()->As<sem::Texture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    const sem::Type* base_type = nullptr;
    if (multisampled_only) {
      base_type = texture_type->As<sem::MultisampledTexture>()->type();
    } else {
      base_type = texture_type->As<sem::SampledTexture>()->type();
    }
    entry.sampled_kind = BaseTypeToSampledKind(base_type);

    result.push_back(entry);
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetStorageTextureResourceBindingsImpl(
    const std::string& entry_point,
    bool read_only) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  auto* func_sem = program_->Sem().Get(func);
  std::vector<ResourceBinding> result;
  for (auto& ref : func_sem->ReferencedStorageTextureVariables()) {
    auto* var = ref.first;
    auto binding_info = ref.second;

    auto* texture_type = var->Type()->UnwrapRef()->As<sem::StorageTexture>();

    if (read_only != (texture_type->access() == ast::Access::kRead)) {
      continue;
    }

    ResourceBinding entry;
    entry.resource_type =
        read_only ? ResourceBinding::ResourceType::kReadOnlyStorageTexture
                  : ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    auto* base_type = texture_type->type();
    entry.sampled_kind = BaseTypeToSampledKind(base_type);
    entry.image_format = TypeImageFormatToResourceBindingImageFormat(
        texture_type->image_format());

    result.push_back(entry);
  }

  return result;
}

void Inspector::GenerateSamplerTargets() {
  // Do not re-generate, since |program_| should not change during the lifetime
  // of the inspector.
  if (sampler_targets_ != nullptr) {
    return;
  }

  sampler_targets_ = std::make_unique<
      std::unordered_map<std::string, UniqueVector<SamplerTexturePair>>>();

  auto& sem = program_->Sem();

  for (auto* node : program_->ASTNodes().Objects()) {
    auto* c = node->As<ast::CallExpression>();
    if (!c) {
      continue;
    }

    auto* call = sem.Get(c);
    if (!call) {
      continue;
    }

    auto* i = call->Target()->As<sem::Intrinsic>();
    if (!i) {
      continue;
    }

    const auto& params = i->Parameters();
    int sampler_index = sem::IndexOf(params, sem::ParameterUsage::kSampler);
    if (sampler_index == -1) {
      continue;
    }

    int texture_index = sem::IndexOf(params, sem::ParameterUsage::kTexture);
    if (texture_index == -1) {
      continue;
    }

    auto* call_func = call->Stmt()->Function();
    std::vector<Symbol> entry_points;
    if (call_func->IsEntryPoint()) {
      entry_points = {call_func->symbol()};
    } else {
      entry_points = sem.Get(call_func)->AncestorEntryPoints();
    }

    if (entry_points.empty()) {
      continue;
    }

    auto* s = c->params()[sampler_index];
    auto* sampler = sem.Get<sem::VariableUser>(s)->Variable();
    sem::BindingPoint sampler_binding_point = {
        sampler->Declaration()->binding_point().group->value(),
        sampler->Declaration()->binding_point().binding->value()};

    auto* t = c->params()[texture_index];
    auto* texture = sem.Get<sem::VariableUser>(t)->Variable();
    sem::BindingPoint texture_binding_point = {
        texture->Declaration()->binding_point().group->value(),
        texture->Declaration()->binding_point().binding->value()};

    for (auto entry_point : entry_points) {
      const auto& ep_name = program_->Symbols().NameFor(entry_point);
      (*sampler_targets_)[ep_name].add(
          {sampler_binding_point, texture_binding_point});
    }
  }
}

}  // namespace inspector
}  // namespace tint
