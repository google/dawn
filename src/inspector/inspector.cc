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
#include "src/ast/float_literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/uint_literal.h"
#include "src/semantic/function.h"
#include "src/semantic/struct.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace inspector {

namespace {

void AppendResourceBindings(std::vector<ResourceBinding>* dest,
                            const std::vector<ResourceBinding>& orig) {
  assert(dest);
  if (!dest) {
    return;
  }

  dest->reserve(dest->size() + orig.size());
  dest->insert(dest->end(), orig.begin(), orig.end());
}

ResourceBinding::TextureDimension
TypeTextureDimensionToResourceBindingTextureDimension(
    const type::TextureDimension& type_dim) {
  switch (type_dim) {
    case type::TextureDimension::k1d:
      return ResourceBinding::TextureDimension::k1d;
    case type::TextureDimension::k2d:
      return ResourceBinding::TextureDimension::k2d;
    case type::TextureDimension::k2dArray:
      return ResourceBinding::TextureDimension::k2dArray;
    case type::TextureDimension::k3d:
      return ResourceBinding::TextureDimension::k3d;
    case type::TextureDimension::kCube:
      return ResourceBinding::TextureDimension::kCube;
    case type::TextureDimension::kCubeArray:
      return ResourceBinding::TextureDimension::kCubeArray;
    case type::TextureDimension::kNone:
      return ResourceBinding::TextureDimension::kNone;
  }
  return ResourceBinding::TextureDimension::kNone;
}

ResourceBinding::SampledKind BaseTypeToSampledKind(type::Type* base_type) {
  if (!base_type) {
    return ResourceBinding::SampledKind::kUnknown;
  }

  if (auto* at = base_type->As<type::Array>()) {
    base_type = at->type();
  } else if (auto* mt = base_type->As<type::Matrix>()) {
    base_type = mt->type();
  } else if (auto* vt = base_type->As<type::Vector>()) {
    base_type = vt->type();
  }

  if (base_type->Is<type::F32>()) {
    return ResourceBinding::SampledKind::kFloat;
  } else if (base_type->Is<type::U32>()) {
    return ResourceBinding::SampledKind::kUInt;
  } else if (base_type->Is<type::I32>()) {
    return ResourceBinding::SampledKind::kSInt;
  } else {
    return ResourceBinding::SampledKind::kUnknown;
  }
}

ResourceBinding::ImageFormat TypeImageFormatToResourceBindingImageFormat(
    const type::ImageFormat& image_format) {
  switch (image_format) {
    case type::ImageFormat::kR8Unorm:
      return ResourceBinding::ImageFormat::kR8Unorm;
    case type::ImageFormat::kR8Snorm:
      return ResourceBinding::ImageFormat::kR8Snorm;
    case type::ImageFormat::kR8Uint:
      return ResourceBinding::ImageFormat::kR8Uint;
    case type::ImageFormat::kR8Sint:
      return ResourceBinding::ImageFormat::kR8Sint;
    case type::ImageFormat::kR16Uint:
      return ResourceBinding::ImageFormat::kR16Uint;
    case type::ImageFormat::kR16Sint:
      return ResourceBinding::ImageFormat::kR16Sint;
    case type::ImageFormat::kR16Float:
      return ResourceBinding::ImageFormat::kR16Float;
    case type::ImageFormat::kRg8Unorm:
      return ResourceBinding::ImageFormat::kRg8Unorm;
    case type::ImageFormat::kRg8Snorm:
      return ResourceBinding::ImageFormat::kRg8Snorm;
    case type::ImageFormat::kRg8Uint:
      return ResourceBinding::ImageFormat::kRg8Uint;
    case type::ImageFormat::kRg8Sint:
      return ResourceBinding::ImageFormat::kRg8Sint;
    case type::ImageFormat::kR32Uint:
      return ResourceBinding::ImageFormat::kR32Uint;
    case type::ImageFormat::kR32Sint:
      return ResourceBinding::ImageFormat::kR32Sint;
    case type::ImageFormat::kR32Float:
      return ResourceBinding::ImageFormat::kR32Float;
    case type::ImageFormat::kRg16Uint:
      return ResourceBinding::ImageFormat::kRg16Uint;
    case type::ImageFormat::kRg16Sint:
      return ResourceBinding::ImageFormat::kRg16Sint;
    case type::ImageFormat::kRg16Float:
      return ResourceBinding::ImageFormat::kRg16Float;
    case type::ImageFormat::kRgba8Unorm:
      return ResourceBinding::ImageFormat::kRgba8Unorm;
    case type::ImageFormat::kRgba8UnormSrgb:
      return ResourceBinding::ImageFormat::kRgba8UnormSrgb;
    case type::ImageFormat::kRgba8Snorm:
      return ResourceBinding::ImageFormat::kRgba8Snorm;
    case type::ImageFormat::kRgba8Uint:
      return ResourceBinding::ImageFormat::kRgba8Uint;
    case type::ImageFormat::kRgba8Sint:
      return ResourceBinding::ImageFormat::kRgba8Sint;
    case type::ImageFormat::kBgra8Unorm:
      return ResourceBinding::ImageFormat::kBgra8Unorm;
    case type::ImageFormat::kBgra8UnormSrgb:
      return ResourceBinding::ImageFormat::kBgra8UnormSrgb;
    case type::ImageFormat::kRgb10A2Unorm:
      return ResourceBinding::ImageFormat::kRgb10A2Unorm;
    case type::ImageFormat::kRg11B10Float:
      return ResourceBinding::ImageFormat::kRg11B10Float;
    case type::ImageFormat::kRg32Uint:
      return ResourceBinding::ImageFormat::kRg32Uint;
    case type::ImageFormat::kRg32Sint:
      return ResourceBinding::ImageFormat::kRg32Sint;
    case type::ImageFormat::kRg32Float:
      return ResourceBinding::ImageFormat::kRg32Float;
    case type::ImageFormat::kRgba16Uint:
      return ResourceBinding::ImageFormat::kRgba16Uint;
    case type::ImageFormat::kRgba16Sint:
      return ResourceBinding::ImageFormat::kRgba16Sint;
    case type::ImageFormat::kRgba16Float:
      return ResourceBinding::ImageFormat::kRgba16Float;
    case type::ImageFormat::kRgba32Uint:
      return ResourceBinding::ImageFormat::kRgba32Uint;
    case type::ImageFormat::kRgba32Sint:
      return ResourceBinding::ImageFormat::kRgba32Sint;
    case type::ImageFormat::kRgba32Float:
      return ResourceBinding::ImageFormat::kRgba32Float;
    case type::ImageFormat::kNone:
      return ResourceBinding::ImageFormat::kNone;
  }
  return ResourceBinding::ImageFormat::kNone;
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

    EntryPoint entry_point;
    entry_point.name = program_->Symbols().NameFor(func->symbol());
    entry_point.remapped_name = program_->Symbols().NameFor(func->symbol());
    entry_point.stage = func->pipeline_stage();
    std::tie(entry_point.workgroup_size_x, entry_point.workgroup_size_y,
             entry_point.workgroup_size_z) = func->workgroup_size();

    for (auto* var : program_->Sem().Get(func)->ReferencedModuleVariables()) {
      auto* decl = var->Declaration();

      auto name = program_->Symbols().NameFor(decl->symbol());
      if (decl->HasBuiltinDecoration()) {
        continue;
      }

      StageVariable stage_variable;
      stage_variable.name = name;

      stage_variable.component_type = ComponentType::kUnknown;
      auto* type = var->Declaration()->type()->UnwrapAll();
      if (type->is_float_scalar_or_vector() || type->is_float_matrix()) {
        stage_variable.component_type = ComponentType::kFloat;
      } else if (type->is_unsigned_scalar_or_vector()) {
        stage_variable.component_type = ComponentType::kUInt;
      } else if (type->is_signed_scalar_or_vector()) {
        stage_variable.component_type = ComponentType::kSInt;
      }

      auto* location_decoration = decl->GetLocationDecoration();
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
  AppendResourceBindings(&result, GetDepthTextureResourceBindings(entry_point));

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
    auto* decl = var->Declaration();
    auto binding_info = ruv.second;

    if (!decl->type()->Is<type::AccessControl>()) {
      continue;
    }
    auto* unwrapped_type = decl->type()->UnwrapIfNeeded();

    auto* str = unwrapped_type->As<type::Struct>();
    if (str == nullptr) {
      continue;
    }

    if (!str->IsBlockDecorated()) {
      continue;
    }

    auto* sem = program_->Sem().Get(str);
    if (!sem) {
      error_ = "Missing semantic information for structure " +
               program_->Symbols().NameFor(str->symbol());
      continue;
    }

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kUniformBuffer;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.size = sem->Size();

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
    auto* decl = var->Declaration();
    auto binding_info = ref.second;

    ResourceBinding entry;
    entry.resource_type = ResourceBinding::ResourceType::kDepthTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = decl->type()->UnwrapIfNeeded()->As<type::Texture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    result.push_back(entry);
  }

  return result;
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

  auto* func_sem = program_->Sem().Get(func);
  std::vector<ResourceBinding> result;
  for (auto& rsv : func_sem->ReferencedStorageBufferVariables()) {
    auto* var = rsv.first;
    auto* decl = var->Declaration();
    auto binding_info = rsv.second;

    auto* ac_type = decl->type()->As<type::AccessControl>();
    if (ac_type == nullptr) {
      continue;
    }

    if (read_only != ac_type->IsReadOnly()) {
      continue;
    }

    auto* str = decl->type()->UnwrapIfNeeded()->As<type::Struct>();
    if (!str) {
      continue;
    }

    auto* sem = program_->Sem().Get(str);
    if (!sem) {
      error_ = "Missing semantic information for structure " +
               program_->Symbols().NameFor(str->symbol());
      continue;
    }

    ResourceBinding entry;
    entry.resource_type =
        read_only ? ResourceBinding::ResourceType::kReadOnlyStorageBuffer
                  : ResourceBinding::ResourceType::kStorageBuffer;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();
    entry.size = sem->Size();

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
    auto* decl = var->Declaration();
    auto binding_info = ref.second;

    ResourceBinding entry;
    entry.resource_type =
        multisampled_only ? ResourceBinding::ResourceType::kMulitsampledTexture
                          : ResourceBinding::ResourceType::kSampledTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type = decl->type()->UnwrapIfNeeded()->As<type::Texture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    type::Type* base_type = nullptr;
    if (multisampled_only) {
      base_type = texture_type->As<type::MultisampledTexture>()
                      ->type()
                      ->UnwrapIfNeeded();
    } else {
      base_type =
          texture_type->As<type::SampledTexture>()->type()->UnwrapIfNeeded();
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
    auto* decl = var->Declaration();
    auto binding_info = ref.second;

    auto* ac_type = decl->type()->As<type::AccessControl>();
    if (ac_type == nullptr) {
      continue;
    }

    if (read_only != ac_type->IsReadOnly()) {
      continue;
    }

    ResourceBinding entry;
    entry.resource_type =
        read_only ? ResourceBinding::ResourceType::kReadOnlyStorageTexture
                  : ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
    entry.bind_group = binding_info.group->value();
    entry.binding = binding_info.binding->value();

    auto* texture_type =
        decl->type()->UnwrapIfNeeded()->As<type::StorageTexture>();
    entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(
        texture_type->dim());

    type::Type* base_type = texture_type->type()->UnwrapIfNeeded();
    entry.sampled_kind = BaseTypeToSampledKind(base_type);
    entry.image_format = TypeImageFormatToResourceBindingImageFormat(
        texture_type->image_format());

    result.push_back(entry);
  }

  return result;
}

}  // namespace inspector
}  // namespace tint
