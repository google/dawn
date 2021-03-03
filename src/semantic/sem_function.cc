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

#include "src/semantic/function.h"

#include "src/ast/binding_decoration.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/function.h"
#include "src/ast/group_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/semantic/variable.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/texture_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::semantic::Function);

namespace tint {
namespace semantic {

namespace {

ParameterList GetParameters(ast::Function* ast) {
  ParameterList parameters;
  parameters.reserve(ast->params().size());
  for (auto* param : ast->params()) {
    parameters.emplace_back(Parameter{param->type(), Parameter::Usage::kNone});
  }
  return parameters;
}

std::tuple<ast::BindingDecoration*, ast::GroupDecoration*> GetBindingAndGroup(
    const Variable* var) {
  ast::BindingDecoration* binding = nullptr;
  ast::GroupDecoration* group = nullptr;
  for (auto* deco : var->Declaration()->decorations()) {
    if (auto* b = deco->As<ast::BindingDecoration>()) {
      binding = b;
    }
    if (auto* s = deco->As<ast::GroupDecoration>()) {
      group = s;
    }
  }
  return {binding, group};
}

}  // namespace

Function::Function(ast::Function* declaration,
                   std::vector<const Variable*> referenced_module_vars,
                   std::vector<const Variable*> local_referenced_module_vars,
                   std::vector<Symbol> ancestor_entry_points)
    : Base(declaration->return_type(), GetParameters(declaration)),
      declaration_(declaration),
      referenced_module_vars_(std::move(referenced_module_vars)),
      local_referenced_module_vars_(std::move(local_referenced_module_vars)),
      ancestor_entry_points_(std::move(ancestor_entry_points)) {}

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

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

Function::VariableBindings Function::ReferencedStorageBufferVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    if (var->StorageClass() != ast::StorageClass::kStorage) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
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
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kSampler);
}

Function::VariableBindings Function::ReferencedComparisonSamplerVariables()
    const {
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kComparisonSampler);
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
    auto* unwrapped_type = var->Declaration()->type()->UnwrapIfNeeded();
    auto* storage_texture = unwrapped_type->As<type::StorageTexture>();
    if (storage_texture == nullptr) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

Function::VariableBindings Function::ReferencedDepthTextureVariables() const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Declaration()->type()->UnwrapIfNeeded();
    auto* storage_texture = unwrapped_type->As<type::DepthTexture>();
    if (storage_texture == nullptr) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
Function::LocalReferencedBuiltinVariables() const {
  std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>> ret;

  for (auto* var : LocalReferencedModuleVariables()) {
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        ret.push_back({var, builtin});
        break;
      }
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
    type::SamplerKind kind) const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Declaration()->type()->UnwrapIfNeeded();
    auto* sampler = unwrapped_type->As<type::Sampler>();
    if (sampler == nullptr || sampler->kind() != kind) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

Function::VariableBindings Function::ReferencedSampledTextureVariablesImpl(
    bool multisampled) const {
  VariableBindings ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Declaration()->type()->UnwrapIfNeeded();
    auto* texture = unwrapped_type->As<type::Texture>();
    if (texture == nullptr) {
      continue;
    }

    auto is_multisampled = texture->Is<type::MultisampledTexture>();
    auto is_sampled = texture->Is<type::SampledTexture>();

    if ((multisampled && !is_multisampled) || (!multisampled && !is_sampled)) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    std::tie(binding, group) = GetBindingAndGroup(var);
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }

  return ret;
}

}  // namespace semantic
}  // namespace tint
