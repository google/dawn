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
#include "src/ast/group_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decoration.h"
#include "src/semantic/variable.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/texture_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::semantic::Function);

namespace tint {
namespace semantic {

Function::Function(std::vector<const Variable*> referenced_module_vars,
                   std::vector<const Variable*> local_referenced_module_vars,
                   std::vector<Symbol> ancestor_entry_points)
    : referenced_module_vars_(std::move(referenced_module_vars)),
      local_referenced_module_vars_(std::move(local_referenced_module_vars)),
      ancestor_entry_points_(std::move(ancestor_entry_points)) {}

Function::~Function() = default;

const std::vector<std::pair<const Variable*, ast::LocationDecoration*>>
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

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedUniformVariables() const {
  std::vector<std::pair<const Variable*, Function::BindingInfo>> ret;

  for (auto* var : ReferencedModuleVariables()) {
    if (var->StorageClass() != ast::StorageClass::kUniform) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* b = deco->As<ast::BindingDecoration>()) {
        binding = b;
      } else if (auto* g = deco->As<ast::GroupDecoration>()) {
        group = g;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedStoragebufferVariables() const {
  std::vector<std::pair<const Variable*, Function::BindingInfo>> ret;

  for (auto* var : ReferencedModuleVariables()) {
    if (var->StorageClass() != ast::StorageClass::kStorage) {
      continue;
    }

    ast::BindingDecoration* binding = nullptr;
    ast::GroupDecoration* group = nullptr;
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* b = deco->As<ast::BindingDecoration>()) {
        binding = b;
      } else if (auto* s = deco->As<ast::GroupDecoration>()) {
        group = s;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
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

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedSamplerVariables() const {
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kSampler);
}

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedComparisonSamplerVariables() const {
  return ReferencedSamplerVariablesImpl(type::SamplerKind::kComparisonSampler);
}

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedSampledTextureVariables() const {
  return ReferencedSampledTextureVariablesImpl(false);
}

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedMultisampledTextureVariables() const {
  return ReferencedSampledTextureVariablesImpl(true);
}

const std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
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

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedSamplerVariablesImpl(type::SamplerKind kind) const {
  std::vector<std::pair<const Variable*, Function::BindingInfo>> ret;

  for (auto* var : ReferencedModuleVariables()) {
    auto* unwrapped_type = var->Declaration()->type()->UnwrapIfNeeded();
    auto* sampler = unwrapped_type->As<type::Sampler>();
    if (sampler == nullptr || sampler->kind() != kind) {
      continue;
    }

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
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }
  return ret;
}

const std::vector<std::pair<const Variable*, Function::BindingInfo>>
Function::ReferencedSampledTextureVariablesImpl(bool multisampled) const {
  std::vector<std::pair<const Variable*, Function::BindingInfo>> ret;

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
    for (auto* deco : var->Declaration()->decorations()) {
      if (auto* b = deco->As<ast::BindingDecoration>()) {
        binding = b;
      } else if (auto* s = deco->As<ast::GroupDecoration>()) {
        group = s;
      }
    }
    if (binding == nullptr || group == nullptr) {
      continue;
    }

    ret.push_back({var, BindingInfo{binding, group}});
  }

  return ret;
}

}  // namespace semantic
}  // namespace tint
