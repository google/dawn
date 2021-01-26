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
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/module.h"
#include "src/ast/null_literal.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/program.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/texture_type.h"
#include "src/type/type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace inspector {

Inspector::Inspector(const Program* program) : program_(program) {}

Inspector::~Inspector() = default;

std::vector<EntryPoint> Inspector::GetEntryPoints() {
  std::vector<EntryPoint> result;

  for (auto* func : program_->AST().Functions()) {
    if (!func->IsEntryPoint()) {
      continue;
    }

    EntryPoint entry_point;
    entry_point.name = program_->Symbols().NameFor(func->symbol());
    entry_point.remapped_name = program_->Symbols().NameFor(func->symbol());
    entry_point.stage = func->pipeline_stage();
    std::tie(entry_point.workgroup_size_x, entry_point.workgroup_size_y,
             entry_point.workgroup_size_z) = func->workgroup_size();

    for (auto* var : func->referenced_module_variables()) {
      auto name = program_->Symbols().NameFor(var->symbol());
      if (var->HasBuiltinDecoration()) {
        continue;
      }

      StageVariable stage_variable;
      stage_variable.name = name;
      auto* location_decoration = var->GetLocationDecoration();
      if (location_decoration) {
        stage_variable.has_location_decoration = true;
        stage_variable.location_decoration = location_decoration->value();
      } else {
        stage_variable.has_location_decoration = false;
      }

      if (var->storage_class() == ast::StorageClass::kInput) {
        entry_point.input_variables.push_back(stage_variable);
      } else if (var->storage_class() == ast::StorageClass::kOutput) {
        entry_point.output_variables.push_back(stage_variable);
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
    if (!var->HasConstantIdDecoration()) {
      continue;
    }

    // If there are conflicting defintions for a constant id, that is invalid
    // WGSL, so the validator should catch it. Thus here the inspector just
    // assumes all definitians of the constant id are the same, so only needs
    // to find the first reference to constant id.
    uint32_t constant_id = var->constant_id();
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
    if (!var->type()->Is<type::AccessControl>()) {
      continue;
    }
    auto* unwrapped_type = var->type()->UnwrapIfNeeded();

    auto* str = unwrapped_type->As<type::Struct>();
    if (str == nullptr) {
      continue;
    }

    if (!str->IsBlockDecorated()) {
      continue;
    }

    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.min_buffer_binding_size =
        var->type()->MinBufferBindingSize(type::MemoryLayout::kUniformBuffer);

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

  std::vector<ResourceBinding> result;

  for (auto& rs : func->referenced_sampler_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = rs;

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

  std::vector<ResourceBinding> result;

  for (auto& rcs : func->referenced_comparison_sampler_variables()) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = rcs;

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

    auto* ac_type = var->type()->As<type::AccessControl>();
    if (ac_type == nullptr) {
      continue;
    }

    if (read_only != ac_type->IsReadOnly()) {
      continue;
    }

    if (!var->type()->UnwrapIfNeeded()->Is<type::Struct>()) {
      continue;
    }

    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.min_buffer_binding_size =
        var->type()->MinBufferBindingSize(type::MemoryLayout::kStorageBuffer);

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
  auto& referenced_variables =
      multisampled_only ? func->referenced_multisampled_texture_variables()
                        : func->referenced_sampled_texture_variables();
  for (auto& ref : referenced_variables) {
    ResourceBinding entry;
    ast::Variable* var = nullptr;
    ast::Function::BindingInfo binding_info;
    std::tie(var, binding_info) = ref;

    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = var->type()->UnwrapIfNeeded()->As<type::Texture>();
    switch (texture_type->dim()) {
      case type::TextureDimension::k1d:
        entry.dim = ResourceBinding::TextureDimension::k1d;
        break;
      case type::TextureDimension::k1dArray:
        entry.dim = ResourceBinding::TextureDimension::k1dArray;
        break;
      case type::TextureDimension::k2d:
        entry.dim = ResourceBinding::TextureDimension::k2d;
        break;
      case type::TextureDimension::k2dArray:
        entry.dim = ResourceBinding::TextureDimension::k2dArray;
        break;
      case type::TextureDimension::k3d:
        entry.dim = ResourceBinding::TextureDimension::k3d;
        break;
      case type::TextureDimension::kCube:
        entry.dim = ResourceBinding::TextureDimension::kCube;
        break;
      case type::TextureDimension::kCubeArray:
        entry.dim = ResourceBinding::TextureDimension::kCubeArray;
        break;
      default:
        entry.dim = ResourceBinding::TextureDimension::kNone;
        break;
    }

    type::Type* base_type = nullptr;
    if (multisampled_only) {
      base_type = texture_type->As<type::MultisampledTexture>()
                      ->type()
                      ->UnwrapIfNeeded();
    } else {
      base_type =
          texture_type->As<type::SampledTexture>()->type()->UnwrapIfNeeded();
    }

    if (auto* at = base_type->As<type::Array>()) {
      base_type = at->type();
    } else if (auto* mt = base_type->As<type::Matrix>()) {
      base_type = mt->type();
    } else if (auto* vt = base_type->As<type::Vector>()) {
      base_type = vt->type();
    }

    if (base_type->Is<type::F32>()) {
      entry.sampled_kind = ResourceBinding::SampledKind::kFloat;
    } else if (base_type->Is<type::U32>()) {
      entry.sampled_kind = ResourceBinding::SampledKind::kUInt;
    } else if (base_type->Is<type::I32>()) {
      entry.sampled_kind = ResourceBinding::SampledKind::kSInt;
    } else {
      entry.sampled_kind = ResourceBinding::SampledKind::kUnknown;
    }

    result.push_back(entry);
  }

  return result;
}

}  // namespace inspector
}  // namespace tint
