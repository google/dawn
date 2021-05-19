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

#include "src/sem/function.h"

#include "src/ast/function.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Function);

namespace tint {
namespace sem {

namespace {

ParameterList GetParameters(const std::vector<const Variable*>& params) {
  ParameterList parameters;
  parameters.reserve(params.size());
  for (auto* param : params) {
    parameters.emplace_back(Parameter{param->Type(), Parameter::Usage::kNone});
  }
  return parameters;
}

}  // namespace

Function::Function(ast::Function* declaration,
                   Type* return_type,
                   std::vector<const Variable*> parameters,
                   std::vector<const Variable*> referenced_module_vars,
                   std::vector<const Variable*> local_referenced_module_vars,
                   std::vector<const ast::ReturnStatement*> return_statements,
                   std::vector<Symbol> ancestor_entry_points,
                   std::array<WorkgroupDimension, 3> workgroup_size)
    : Base(return_type, GetParameters(parameters)),
      declaration_(declaration),
      parameters_(std::move(parameters)),
      referenced_module_vars_(std::move(referenced_module_vars)),
      local_referenced_module_vars_(std::move(local_referenced_module_vars)),
      return_statements_(std::move(return_statements)),
      ancestor_entry_points_(std::move(ancestor_entry_points)),
      workgroup_size_(std::move(workgroup_size)) {}

Function::~Function() = default;

std::vector<std::pair<const Variable*, ast::LocationDecoration*>>
Function::ReferencedLocationVariables() const {
  std::vector<std::pair<const Variable*, ast::LocationDecoration*>> ret;

  for (auto* var : ReferencedModuleVariables()) {
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* location = deco->As<ast::LocationDecoration>()) {
        ret.push_back({var, location});
        break;
      }
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedUniformVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    if (var->StorageClass() != ast::StorageClass::kUniform) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedStorageBufferVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    if (var->StorageClass() != ast::StorageClass::kStorage) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
Function::ReferencedBuiltinVariables() const {
  std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>> ret;

  for (auto* var : ReferencedModuleVariables()) {
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        ret.push_back({var, builtin});
        break;
      }
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedSamplerVariables() const {
  return ReferencedSamplerVariablesImpl(ast::SamplerKind::kSampler);
}

Function::VariableBindings Function::ReferencedComparisonSamplerVariables()
    const {
  return ReferencedSamplerVariablesImpl(ast::SamplerKind::kComparisonSampler);
}

Function::VariableBindings Function::ReferencedSampledTextureVariables() const {
  return ReferencedSampledTextureVariablesImpl(false);
}

Function::VariableBindings Function::ReferencedMultisampledTextureVariables()
    const {
  return ReferencedSampledTextureVariablesImpl(true);
}

Function::VariableBindings Function::ReferencedStorageTextureVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* storage_texture = unwrapped_type->As<sem::StorageTexture>();
    if (storage_texture == nullptr) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedDepthTextureVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* storage_texture = unwrapped_type->As<sem::DepthTexture>();
    if (storage_texture == nullptr) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedExternalTextureVariables()
    const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* external_texture = unwrapped_type->As<sem::ExternalTexture>();
    if (external_texture == nullptr) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

bool Function::HasAncestorEntryPoint(Symbol symbol) const {
  for (const auto& point : ancestor_entry_points_) {
    if (point == symbol) {
      return true;
    }
  }
  return false;
}

Function::VariableBindings Function::ReferencedSamplerVariablesImpl(
    ast::SamplerKind kind) const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* sampler = unwrapped_type->As<sem::Sampler>();
    if (sampler == nullptr || sampler->kind() != kind) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings Function::ReferencedSampledTextureVariablesImpl(
    bool multisampled) const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* texture = unwrapped_type->As<sem::Texture>();
    if (texture == nullptr) {
      continue;
    }

    auto is_multisampled = texture->Is<sem::MultisampledTexture>();
    auto is_sampled = texture->Is<sem::SampledTexture>();

    if ((multisampled && !is_multisampled) || (!multisampled && !is_sampled)) {
      continue;
    }

    if (auto binding_point = var->Declaration()->binding_point()) {
      ret.push_back({var, binding_point});
    }
  }

  return ret;
}

}  // namespace sem
}  // namespace tint
