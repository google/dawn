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
#include "src/utils/to_const_ptr_vec.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Function);

namespace tint {
namespace sem {

Function::Function(const ast::Function* declaration,
                   Type* return_type,
                   std::vector<Parameter*> parameters)
    : Base(return_type, utils::ToConstPtrVec(parameters)),
      declaration_(declaration),
      workgroup_size_{WorkgroupDimension{1}, WorkgroupDimension{1},
                      WorkgroupDimension{1}} {
  for (auto* parameter : parameters) {
    parameter->SetOwner(this);
  }
}  // namespace sem

Function::~Function() = default;

std::vector<std::pair<const Variable*, const ast::LocationDecoration*>>
Function::TransitivelyReferencedLocationVariables() const {
  std::vector<std::pair<const Variable*, const ast::LocationDecoration*>> ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
    for (auto* deco : var->Declaration()->decorations) {
      if (auto* location = deco->As<ast::LocationDecoration>()) {
        ret.push_back({var, location});
        break;
      }
    }
  }
  return ret;
}

Function::VariableBindings Function::TransitivelyReferencedUniformVariables()
    const {
  VariableBindings ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
    if (var->StorageClass() != ast::StorageClass::kUniform) {
      continue;
    }

    if (auto binding_point = var->Declaration()->BindingPoint()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings
Function::TransitivelyReferencedStorageBufferVariables() const {
  VariableBindings ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
    if (var->StorageClass() != ast::StorageClass::kStorage) {
      continue;
    }

    if (auto binding_point = var->Declaration()->BindingPoint()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

std::vector<std::pair<const Variable*, const ast::BuiltinDecoration*>>
Function::TransitivelyReferencedBuiltinVariables() const {
  std::vector<std::pair<const Variable*, const ast::BuiltinDecoration*>> ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
    for (auto* deco : var->Declaration()->decorations) {
      if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
        ret.push_back({var, builtin});
        break;
      }
    }
  }
  return ret;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariables()
    const {
  return TransitivelyReferencedSamplerVariablesImpl(ast::SamplerKind::kSampler);
}

Function::VariableBindings
Function::TransitivelyReferencedComparisonSamplerVariables() const {
  return TransitivelyReferencedSamplerVariablesImpl(
      ast::SamplerKind::kComparisonSampler);
}

Function::VariableBindings
Function::TransitivelyReferencedSampledTextureVariables() const {
  return TransitivelyReferencedSampledTextureVariablesImpl(false);
}

Function::VariableBindings
Function::TransitivelyReferencedMultisampledTextureVariables() const {
  return TransitivelyReferencedSampledTextureVariablesImpl(true);
}

Function::VariableBindings Function::TransitivelyReferencedVariablesOfType(
    const tint::TypeInfo& type_info) const {
  VariableBindings ret;
  for (auto* var : TransitivelyReferencedGlobals()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    if (unwrapped_type->TypeInfo().Is(type_info)) {
      if (auto binding_point = var->Declaration()->BindingPoint()) {
        ret.push_back({var, binding_point});
      }
    }
  }
  return ret;
}

bool Function::HasAncestorEntryPoint(Symbol symbol) const {
  for (const auto* point : ancestor_entry_points_) {
    if (point->Declaration()->symbol == symbol) {
      return true;
    }
  }
  return false;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariablesImpl(
    ast::SamplerKind kind) const {
  VariableBindings ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
    auto* unwrapped_type = var->Type()->UnwrapRef();
    auto* sampler = unwrapped_type->As<sem::Sampler>();
    if (sampler == nullptr || sampler->kind() != kind) {
      continue;
    }

    if (auto binding_point = var->Declaration()->BindingPoint()) {
      ret.push_back({var, binding_point});
    }
  }
  return ret;
}

Function::VariableBindings
Function::TransitivelyReferencedSampledTextureVariablesImpl(
    bool multisampled) const {
  VariableBindings ret;

  for (auto* var : TransitivelyReferencedGlobals()) {
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

    if (auto binding_point = var->Declaration()->BindingPoint()) {
      ret.push_back({var, binding_point});
    }
  }

  return ret;
}

}  // namespace sem
}  // namespace tint
