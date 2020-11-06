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

#include <algorithm>
#include <map>
#include <utility>

#include "src/ast/bool_literal.h"
#include "src/ast/constructor_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/null_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/type.h"
#include "src/ast/uint_literal.h"

namespace tint {
namespace inspector {

Inspector::Inspector(const ast::Module& module) : module_(module) {}

Inspector::~Inspector() = default;

std::vector<EntryPoint> Inspector::GetEntryPoints() {
  std::vector<EntryPoint> result;

  for (const auto& func : module_.functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    EntryPoint entry_point;
    entry_point.name = func->name();
    entry_point.stage = func->pipeline_stage();
    std::tie(entry_point.workgroup_size_x, entry_point.workgroup_size_y,
             entry_point.workgroup_size_z) = func->workgroup_size();

    for (auto* var : func->referenced_module_variables()) {
      if (var->storage_class() == ast::StorageClass::kInput) {
        entry_point.input_variables.push_back(var->name());
      } else {
        entry_point.output_variables.push_back(var->name());
      }
    }
    result.push_back(std::move(entry_point));
  }

  return result;
}

std::map<uint32_t, Scalar> Inspector::GetConstantIDs() {
  std::map<uint32_t, Scalar> result;
  for (auto& var : module_.global_variables()) {
    if (!var->IsDecorated()) {
      continue;
    }

    auto* decorated = var->AsDecorated();
    if (!decorated->HasConstantIdDecoration()) {
      continue;
    }

    // If there are conflicting defintions for a constant id, that is invalid
    // WGSL, so the validator should catch it. Thus here the inspector just
    // assumes all definitians of the constant id are the same, so only needs
    // to find the first reference to constant id.
    uint32_t constant_id = decorated->constant_id();
    if (result.find(constant_id) != result.end()) {
      continue;
    }

    if (!var->has_constructor()) {
      result[constant_id] = Scalar();
      continue;
    }

    auto* expression = var->constructor();
    if (!expression->IsConstructor()) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    auto* constructor = expression->AsConstructor();
    if (!constructor->IsScalarConstructor()) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    auto* literal = constructor->AsScalarConstructor()->literal();
    if (!literal) {
      // This is invalid WGSL, but handling gracefully.
      result[constant_id] = Scalar();
      continue;
    }

    if (literal->IsBool()) {
      result[constant_id] = Scalar(literal->AsBool()->IsTrue());
      continue;
    }

    if (literal->IsUint()) {
      result[constant_id] = Scalar(literal->AsUint()->value());
      continue;
    }

    if (literal->IsSint()) {
      result[constant_id] = Scalar(literal->AsSint()->value());
      continue;
    }

    if (literal->IsFloat()) {
      result[constant_id] = Scalar(literal->AsFloat()->value());
      continue;
    }

    result[constant_id] = Scalar();
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetUniformBufferResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;

  for (auto& ruv : func->referenced_uniform_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = ruv;
    if (!var->type()->IsAccessControl()) {
      continue;
    }
    auto* unwrapped_type = var->type()->UnwrapIfNeeded();

    if (!unwrapped_type->IsStruct()) {
      continue;
    }

    if (!unwrapped_type->AsStruct()->IsBlockDecorated()) {
      continue;
    }

    entry.bind_group = binding_info.set->value();
    entry.binding = binding_info.binding->value();
    entry.min_buffer_binding_size = var->type()->MinBufferBindingSize(
        ast::type::MemoryLayout::kUniformBuffer);

    result.push_back(std::move(entry));
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

  std::vector<ResourceBinding> result;

  for (auto& rs : func->referenced_sampler_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = rs;

    entry.bind_group = binding_info.set->value();
    entry.binding = binding_info.binding->value();

    result.push_back(std::move(entry));
  }

  return result;
}

std::vector<ResourceBinding> Inspector::GetComparisonSamplerResourceBindings(
    const std::string& entry_point) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;

  for (auto& rcs : func->referenced_comparison_sampler_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = rcs;

    entry.bind_group = binding_info.set->value();
    entry.binding = binding_info.binding->value();

    result.push_back(std::move(entry));
  }

  return result;
}

ast::Function* Inspector::FindEntryPointByName(const std::string& name) {
  auto* func = module_.FindFunctionByName(name);
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

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindingsImpl(
    const std::string& entry_point,
    bool read_only) {
  auto* func = FindEntryPointByName(entry_point);
  if (!func) {
    return {};
  }

  std::vector<ResourceBinding> result;
  for (auto& rsv : func->referenced_storagebuffer_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = rsv;
    if (!var->type()->IsAccessControl()) {
      continue;
    }

    auto* ac_type = var->type()->AsAccessControl();
    if (read_only != ac_type->IsReadOnly()) {
      continue;
    }

    if (!var->type()->UnwrapIfNeeded()->IsStruct()) {
      continue;
    }

    entry.bind_group = binding_info.set->value();
    entry.binding = binding_info.binding->value();
    entry.min_buffer_binding_size = var->type()->MinBufferBindingSize(
        ast::type::MemoryLayout::kStorageBuffer);

    result.push_back(std::move(entry));
  }

  return result;
}

}  // namespace inspector
}  // namespace tint
