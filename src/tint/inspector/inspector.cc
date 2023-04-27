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

#include "src/tint/inspector/inspector.h"

#include <limits>
#include <utility>

#include "src/tint/ast/bool_literal_expression.h"
#include "src/tint/ast/call_expression.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/location_attribute.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/var.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/builtin/extension.h"
#include "src/tint/builtin/interpolation_sampling.h"
#include "src/tint/builtin/interpolation_type.h"
#include "src/tint/sem/builtin_enum_expression.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/variable.h"
#include "src/tint/switch.h"
#include "src/tint/type/array.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/unique_vector.h"

namespace tint::inspector {

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

std::tuple<ComponentType, CompositionType> CalculateComponentAndComposition(
    const type::Type* type) {
    // entry point in/out variables must of numeric scalar or vector types.
    TINT_ASSERT(Inspector, type->is_numeric_scalar_or_vector());

    ComponentType componentType = Switch(
        type::Type::DeepestElementOf(type),  //
        [&](const type::F32*) { return ComponentType::kF32; },
        [&](const type::F16*) { return ComponentType::kF16; },
        [&](const type::I32*) { return ComponentType::kI32; },
        [&](const type::U32*) { return ComponentType::kU32; },
        [&](Default) {
            tint::diag::List diagnostics;
            TINT_UNREACHABLE(Inspector, diagnostics) << "unhandled component type";
            return ComponentType::kUnknown;
        });

    CompositionType compositionType;
    if (auto* vec = type->As<type::Vector>()) {
        switch (vec->Width()) {
            case 2: {
                compositionType = CompositionType::kVec2;
                break;
            }
            case 3: {
                compositionType = CompositionType::kVec3;
                break;
            }
            case 4: {
                compositionType = CompositionType::kVec4;
                break;
            }
            default: {
                tint::diag::List diagnostics;
                TINT_UNREACHABLE(Inspector, diagnostics) << "unhandled composition type";
                compositionType = CompositionType::kUnknown;
                break;
            }
        }
    } else {
        compositionType = CompositionType::kScalar;
    }

    return {componentType, compositionType};
}

}  // namespace

Inspector::Inspector(const Program* program) : program_(program) {}

Inspector::~Inspector() = default;

EntryPoint Inspector::GetEntryPoint(const tint::ast::Function* func) {
    EntryPoint entry_point;
    TINT_ASSERT(Inspector, func != nullptr);
    TINT_ASSERT(Inspector, func->IsEntryPoint());

    auto* sem = program_->Sem().Get(func);

    entry_point.name = func->name->symbol.Name();
    entry_point.remapped_name = func->name->symbol.Name();

    switch (func->PipelineStage()) {
        case ast::PipelineStage::kCompute: {
            entry_point.stage = PipelineStage::kCompute;

            auto wgsize = sem->WorkgroupSize();
            if (wgsize[0].has_value() && wgsize[1].has_value() && wgsize[2].has_value()) {
                entry_point.workgroup_size = {wgsize[0].value(), wgsize[1].value(),
                                              wgsize[2].value()};
            }
            break;
        }
        case ast::PipelineStage::kFragment: {
            entry_point.stage = PipelineStage::kFragment;
            break;
        }
        case ast::PipelineStage::kVertex: {
            entry_point.stage = PipelineStage::kVertex;
            break;
        }
        default: {
            TINT_UNREACHABLE(Inspector, diagnostics_)
                << "invalid pipeline stage for entry point '" << entry_point.name << "'";
            break;
        }
    }

    for (auto* param : sem->Parameters()) {
        AddEntryPointInOutVariables(param->Declaration()->name->symbol.Name(), param->Type(),
                                    param->Declaration()->attributes, param->Location(),
                                    entry_point.input_variables);

        entry_point.input_position_used |= ContainsBuiltin(
            builtin::BuiltinValue::kPosition, param->Type(), param->Declaration()->attributes);
        entry_point.front_facing_used |= ContainsBuiltin(
            builtin::BuiltinValue::kFrontFacing, param->Type(), param->Declaration()->attributes);
        entry_point.sample_index_used |= ContainsBuiltin(
            builtin::BuiltinValue::kSampleIndex, param->Type(), param->Declaration()->attributes);
        entry_point.input_sample_mask_used |= ContainsBuiltin(
            builtin::BuiltinValue::kSampleMask, param->Type(), param->Declaration()->attributes);
        entry_point.num_workgroups_used |= ContainsBuiltin(
            builtin::BuiltinValue::kNumWorkgroups, param->Type(), param->Declaration()->attributes);
    }

    if (!sem->ReturnType()->Is<type::Void>()) {
        AddEntryPointInOutVariables("<retval>", sem->ReturnType(), func->return_type_attributes,
                                    sem->ReturnLocation(), entry_point.output_variables);

        entry_point.output_sample_mask_used = ContainsBuiltin(
            builtin::BuiltinValue::kSampleMask, sem->ReturnType(), func->return_type_attributes);
        entry_point.frag_depth_used = ContainsBuiltin(
            builtin::BuiltinValue::kFragDepth, sem->ReturnType(), func->return_type_attributes);
    }

    for (auto* var : sem->TransitivelyReferencedGlobals()) {
        auto* decl = var->Declaration();

        auto name = decl->name->symbol.Name();

        auto* global = var->As<sem::GlobalVariable>();
        if (global && global->Declaration()->Is<ast::Override>()) {
            Override override;
            override.name = name;
            override.id = global->OverrideId();
            auto* type = var->Type();
            TINT_ASSERT(Inspector, type->is_scalar());
            if (type->is_bool_scalar_or_vector()) {
                override.type = Override::Type::kBool;
            } else if (type->is_float_scalar()) {
                if (type->Is<type::F16>()) {
                    override.type = Override::Type::kFloat16;
                } else {
                    override.type = Override::Type::kFloat32;
                }
            } else if (type->is_signed_integer_scalar()) {
                override.type = Override::Type::kInt32;
            } else if (type->is_unsigned_integer_scalar()) {
                override.type = Override::Type::kUint32;
            } else {
                TINT_UNREACHABLE(Inspector, diagnostics_);
            }

            override.is_initialized = global->Declaration()->initializer;
            override.is_id_specified =
                ast::HasAttribute<ast::IdAttribute>(global->Declaration()->attributes);

            entry_point.overrides.push_back(override);
        }
    }

    return entry_point;
}

EntryPoint Inspector::GetEntryPoint(const std::string& entry_point_name) {
    auto* func = FindEntryPointByName(entry_point_name);
    if (!func) {
        return EntryPoint();
    }
    return GetEntryPoint(func);
}

std::vector<EntryPoint> Inspector::GetEntryPoints() {
    std::vector<EntryPoint> result;

    for (auto* func : program_->AST().Functions()) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        result.push_back(GetEntryPoint(func));
    }

    return result;
}

std::map<OverrideId, Scalar> Inspector::GetOverrideDefaultValues() {
    std::map<OverrideId, Scalar> result;
    for (auto* var : program_->AST().GlobalVariables()) {
        auto* global = program_->Sem().Get<sem::GlobalVariable>(var);
        if (!global || !global->Declaration()->Is<ast::Override>()) {
            continue;
        }

        // If there are conflicting defintions for an override id, that is invalid
        // WGSL, so the resolver should catch it. Thus here the inspector just
        // assumes all definitions of the override id are the same, so only needs
        // to find the first reference to override id.
        OverrideId override_id = global->OverrideId();
        if (result.find(override_id) != result.end()) {
            continue;
        }

        if (global->Initializer()) {
            if (auto* value = global->Initializer()->ConstantValue()) {
                result[override_id] = Switch(
                    value->Type(),  //
                    [&](const type::I32*) { return Scalar(value->ValueAs<i32>()); },
                    [&](const type::U32*) { return Scalar(value->ValueAs<u32>()); },
                    [&](const type::F32*) { return Scalar(value->ValueAs<f32>()); },
                    [&](const type::F16*) {
                        // Default value of f16 override is also stored as float scalar.
                        return Scalar(static_cast<float>(value->ValueAs<f16>()));
                    },
                    [&](const type::Bool*) { return Scalar(value->ValueAs<bool>()); });
                continue;
            }
        }

        // No const-expression initializer for the override
        result[override_id] = Scalar();
    }

    return result;
}

std::map<std::string, OverrideId> Inspector::GetNamedOverrideIds() {
    std::map<std::string, OverrideId> result;
    for (auto* var : program_->AST().GlobalVariables()) {
        auto* global = program_->Sem().Get<sem::GlobalVariable>(var);
        if (global && global->Declaration()->Is<ast::Override>()) {
            auto name = var->name->symbol.Name();
            result[name] = global->OverrideId();
        }
    }
    return result;
}

uint32_t Inspector::GetStorageSize(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return 0;
    }

    size_t size = 0;
    auto* func_sem = program_->Sem().Get(func);
    for (auto& ruv : func_sem->TransitivelyReferencedUniformVariables()) {
        size += ruv.first->Type()->UnwrapRef()->Size();
    }
    for (auto& rsv : func_sem->TransitivelyReferencedStorageBufferVariables()) {
        size += rsv.first->Type()->UnwrapRef()->Size();
    }

    if (static_cast<uint64_t>(size) > static_cast<uint64_t>(std::numeric_limits<uint32_t>::max())) {
        return std::numeric_limits<uint32_t>::max();
    }
    return static_cast<uint32_t>(size);
}

std::vector<ResourceBinding> Inspector::GetResourceBindings(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    for (auto fn : {
             &Inspector::GetUniformBufferResourceBindings,
             &Inspector::GetStorageBufferResourceBindings,
             &Inspector::GetReadOnlyStorageBufferResourceBindings,
             &Inspector::GetSamplerResourceBindings,
             &Inspector::GetComparisonSamplerResourceBindings,
             &Inspector::GetSampledTextureResourceBindings,
             &Inspector::GetMultisampledTextureResourceBindings,
             &Inspector::GetWriteOnlyStorageTextureResourceBindings,
             &Inspector::GetDepthTextureResourceBindings,
             &Inspector::GetDepthMultisampledTextureResourceBindings,
             &Inspector::GetExternalTextureResourceBindings,
         }) {
        AppendResourceBindings(&result, (this->*fn)(entry_point));
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

    auto* func_sem = program_->Sem().Get(func);
    for (auto& ruv : func_sem->TransitivelyReferencedUniformVariables()) {
        auto* var = ruv.first;
        auto binding_info = ruv.second;

        auto* unwrapped_type = var->Type()->UnwrapRef();

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kUniformBuffer;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.size = unwrapped_type->Size();
        entry.size_no_padding = entry.size;
        if (auto* str = unwrapped_type->As<sem::Struct>()) {
            entry.size_no_padding = str->SizeNoPadding();
        } else {
            entry.size_no_padding = entry.size;
        }

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetStorageBufferResourceBindings(
    const std::string& entry_point) {
    return GetStorageBufferResourceBindingsImpl(entry_point, false);
}

std::vector<ResourceBinding> Inspector::GetReadOnlyStorageBufferResourceBindings(
    const std::string& entry_point) {
    return GetStorageBufferResourceBindingsImpl(entry_point, true);
}

std::vector<ResourceBinding> Inspector::GetSamplerResourceBindings(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;

    auto* func_sem = program_->Sem().Get(func);
    for (auto& rs : func_sem->TransitivelyReferencedSamplerVariables()) {
        auto binding_info = rs.second;

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kSampler;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

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
    for (auto& rcs : func_sem->TransitivelyReferencedComparisonSamplerVariables()) {
        auto binding_info = rcs.second;

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kComparisonSampler;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

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

std::vector<ResourceBinding> Inspector::GetWriteOnlyStorageTextureResourceBindings(
    const std::string& entry_point) {
    return GetStorageTextureResourceBindingsImpl(entry_point);
}

std::vector<ResourceBinding> Inspector::GetTextureResourceBindings(
    const std::string& entry_point,
    const tint::utils::TypeInfo* texture_type,
    ResourceBinding::ResourceType resource_type) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    std::vector<ResourceBinding> result;
    auto* func_sem = program_->Sem().Get(func);
    for (auto& ref : func_sem->TransitivelyReferencedVariablesOfType(texture_type)) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        ResourceBinding entry;
        entry.resource_type = resource_type;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

        auto* tex = var->Type()->UnwrapRef()->As<type::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->dim());

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetDepthTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &utils::TypeInfo::Of<type::DepthTexture>(),
                                      ResourceBinding::ResourceType::kDepthTexture);
}

std::vector<ResourceBinding> Inspector::GetDepthMultisampledTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point,
                                      &utils::TypeInfo::Of<type::DepthMultisampledTexture>(),
                                      ResourceBinding::ResourceType::kDepthMultisampledTexture);
}

std::vector<ResourceBinding> Inspector::GetExternalTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &utils::TypeInfo::Of<type::ExternalTexture>(),
                                      ResourceBinding::ResourceType::kExternalTexture);
}

utils::VectorRef<SamplerTexturePair> Inspector::GetSamplerTextureUses(
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

std::vector<SamplerTexturePair> Inspector::GetSamplerTextureUses(
    const std::string& entry_point,
    const sem::BindingPoint& placeholder) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }
    auto* func_sem = program_->Sem().Get(func);

    std::vector<SamplerTexturePair> new_pairs;
    for (auto pair : func_sem->TextureSamplerPairs()) {
        auto* texture = pair.first->As<sem::GlobalVariable>();
        auto* sampler = pair.second ? pair.second->As<sem::GlobalVariable>() : nullptr;
        SamplerTexturePair new_pair;
        new_pair.sampler_binding_point = sampler ? *sampler->BindingPoint() : placeholder;
        new_pair.texture_binding_point = *texture->BindingPoint();
        new_pairs.push_back(new_pair);
    }
    return new_pairs;
}

uint32_t Inspector::GetWorkgroupStorageSize(const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return 0;
    }

    uint32_t total_size = 0;
    auto* func_sem = program_->Sem().Get(func);
    for (const sem::Variable* var : func_sem->TransitivelyReferencedGlobals()) {
        if (var->AddressSpace() == builtin::AddressSpace::kWorkgroup) {
            auto* ty = var->Type()->UnwrapRef();
            uint32_t align = ty->Align();
            uint32_t size = ty->Size();

            // This essentially matches std430 layout rules from GLSL, which are in
            // turn specified as an upper bound for Vulkan layout sizing. Since D3D
            // and Metal are even less specific, we assume Vulkan behavior as a
            // good-enough approximation everywhere.
            total_size += utils::RoundUp(align, size);
        }
    }

    return total_size;
}

std::vector<std::string> Inspector::GetUsedExtensionNames() {
    auto& extensions = program_->Sem().Module()->Extensions();
    std::vector<std::string> out;
    out.reserve(extensions.Length());
    for (auto ext : extensions) {
        out.push_back(utils::ToString(ext));
    }
    return out;
}

std::vector<std::pair<std::string, Source>> Inspector::GetEnableDirectives() {
    std::vector<std::pair<std::string, Source>> result;

    // Ast nodes for enable directive are stored within global declarations list
    auto global_decls = program_->AST().GlobalDeclarations();
    for (auto* node : global_decls) {
        if (auto* enable = node->As<ast::Enable>()) {
            for (auto* ext : enable->extensions) {
                result.push_back({utils::ToString(ext->name), ext->source});
            }
        }
    }

    return result;
}

const ast::Function* Inspector::FindEntryPointByName(const std::string& name) {
    auto* func = program_->AST().Functions().Find(program_->Symbols().Get(name));
    if (!func) {
        diagnostics_.add_error(diag::System::Inspector, name + " was not found!");
        return nullptr;
    }

    if (!func->IsEntryPoint()) {
        diagnostics_.add_error(diag::System::Inspector, name + " is not an entry point!");
        return nullptr;
    }

    return func;
}

void Inspector::AddEntryPointInOutVariables(std::string name,
                                            const type::Type* type,
                                            utils::VectorRef<const ast::Attribute*> attributes,
                                            std::optional<uint32_t> location,
                                            std::vector<StageVariable>& variables) const {
    // Skip builtins.
    if (ast::HasAttribute<ast::BuiltinAttribute>(attributes)) {
        return;
    }

    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        // Recurse into members.
        for (auto* member : struct_ty->Members()) {
            AddEntryPointInOutVariables(name + "." + member->Name().Name(), member->Type(),
                                        member->Declaration()->attributes,
                                        member->Attributes().location, variables);
        }
        return;
    }

    // Base case: add the variable.

    StageVariable stage_variable;
    stage_variable.name = name;
    std::tie(stage_variable.component_type, stage_variable.composition_type) =
        CalculateComponentAndComposition(type);

    TINT_ASSERT(Inspector, location.has_value());
    stage_variable.has_location_attribute = true;
    stage_variable.location_attribute = location.value();

    std::tie(stage_variable.interpolation_type, stage_variable.interpolation_sampling) =
        CalculateInterpolationData(type, attributes);

    variables.push_back(stage_variable);
}

bool Inspector::ContainsBuiltin(builtin::BuiltinValue builtin,
                                const type::Type* type,
                                utils::VectorRef<const ast::Attribute*> attributes) const {
    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        // Recurse into members.
        for (auto* member : struct_ty->Members()) {
            if (ContainsBuiltin(builtin, member->Type(), member->Declaration()->attributes)) {
                return true;
            }
        }
        return false;
    }

    // Base case: check for builtin
    auto* builtin_declaration = ast::GetAttribute<ast::BuiltinAttribute>(attributes);
    if (!builtin_declaration) {
        return false;
    }
    return program_->Sem().Get(builtin_declaration)->Value() == builtin;
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
    for (auto& rsv : func_sem->TransitivelyReferencedStorageBufferVariables()) {
        auto* var = rsv.first;
        auto binding_info = rsv.second;

        if (read_only != (var->Access() == builtin::Access::kRead)) {
            continue;
        }

        auto* unwrapped_type = var->Type()->UnwrapRef();

        ResourceBinding entry;
        entry.resource_type = read_only ? ResourceBinding::ResourceType::kReadOnlyStorageBuffer
                                        : ResourceBinding::ResourceType::kStorageBuffer;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;
        entry.size = unwrapped_type->Size();
        if (auto* str = unwrapped_type->As<sem::Struct>()) {
            entry.size_no_padding = str->SizeNoPadding();
        } else {
            entry.size_no_padding = entry.size;
        }

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
    auto referenced_variables = multisampled_only
                                    ? func_sem->TransitivelyReferencedMultisampledTextureVariables()
                                    : func_sem->TransitivelyReferencedSampledTextureVariables();
    for (auto& ref : referenced_variables) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        ResourceBinding entry;
        entry.resource_type = multisampled_only
                                  ? ResourceBinding::ResourceType::kMultisampledTexture
                                  : ResourceBinding::ResourceType::kSampledTexture;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

        auto* texture_type = var->Type()->UnwrapRef()->As<type::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(texture_type->dim());

        const type::Type* base_type = nullptr;
        if (multisampled_only) {
            base_type = texture_type->As<type::MultisampledTexture>()->type();
        } else {
            base_type = texture_type->As<type::SampledTexture>()->type();
        }
        entry.sampled_kind = BaseTypeToSampledKind(base_type);

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetStorageTextureResourceBindingsImpl(
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    auto* func_sem = program_->Sem().Get(func);
    std::vector<ResourceBinding> result;
    for (auto& ref : func_sem->TransitivelyReferencedVariablesOfType<type::StorageTexture>()) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        auto* texture_type = var->Type()->UnwrapRef()->As<type::StorageTexture>();

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
        entry.bind_group = binding_info.group;
        entry.binding = binding_info.binding;

        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(texture_type->dim());

        auto* base_type = texture_type->type();
        entry.sampled_kind = BaseTypeToSampledKind(base_type);
        entry.image_format =
            TypeTexelFormatToResourceBindingTexelFormat(texture_type->texel_format());

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
        std::unordered_map<std::string, utils::UniqueVector<SamplerTexturePair, 4>>>();

    auto& sem = program_->Sem();

    for (auto* node : program_->ASTNodes().Objects()) {
        auto* c = node->As<ast::CallExpression>();
        if (!c) {
            continue;
        }

        auto* call = sem.Get(c)->UnwrapMaterialize()->As<sem::Call>();
        if (!call) {
            continue;
        }

        auto* i = call->Target()->As<sem::Builtin>();
        if (!i) {
            continue;
        }

        const auto& signature = i->Signature();
        int sampler_index = signature.IndexOf(sem::ParameterUsage::kSampler);
        if (sampler_index == -1) {
            continue;
        }

        int texture_index = signature.IndexOf(sem::ParameterUsage::kTexture);
        if (texture_index == -1) {
            continue;
        }

        auto* call_func = call->Stmt()->Function();
        std::vector<const sem::Function*> entry_points;
        if (call_func->Declaration()->IsEntryPoint()) {
            entry_points = {call_func};
        } else {
            entry_points = call_func->AncestorEntryPoints();
        }

        if (entry_points.empty()) {
            continue;
        }

        auto* t = c->args[static_cast<size_t>(texture_index)];
        auto* s = c->args[static_cast<size_t>(sampler_index)];

        GetOriginatingResources(std::array<const ast::Expression*, 2>{t, s},
                                [&](std::array<const sem::GlobalVariable*, 2> globals) {
                                    auto texture_binding_point = *globals[0]->BindingPoint();
                                    auto sampler_binding_point = *globals[1]->BindingPoint();

                                    for (auto* entry_point : entry_points) {
                                        const auto& ep_name =
                                            entry_point->Declaration()->name->symbol.Name();
                                        (*sampler_targets_)[ep_name].Add(
                                            {sampler_binding_point, texture_binding_point});
                                    }
                                });
    }
}

std::tuple<InterpolationType, InterpolationSampling> Inspector::CalculateInterpolationData(
    const type::Type* type,
    utils::VectorRef<const ast::Attribute*> attributes) const {
    auto* interpolation_attribute = ast::GetAttribute<ast::InterpolateAttribute>(attributes);
    if (type->is_integer_scalar_or_vector()) {
        return {InterpolationType::kFlat, InterpolationSampling::kNone};
    }

    if (!interpolation_attribute) {
        return {InterpolationType::kPerspective, InterpolationSampling::kCenter};
    }

    auto& sem = program_->Sem();

    auto ast_interpolation_type = sem.Get<sem::BuiltinEnumExpression<builtin::InterpolationType>>(
                                         interpolation_attribute->type)
                                      ->Value();

    auto ast_sampling_type = builtin::InterpolationSampling::kUndefined;
    if (interpolation_attribute->sampling) {
        ast_sampling_type = sem.Get<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>(
                                   interpolation_attribute->sampling)
                                ->Value();
    }

    if (ast_interpolation_type != builtin::InterpolationType::kFlat &&
        ast_sampling_type == builtin::InterpolationSampling::kUndefined) {
        ast_sampling_type = builtin::InterpolationSampling::kCenter;
    }

    auto interpolation_type = InterpolationType::kUnknown;
    switch (ast_interpolation_type) {
        case builtin::InterpolationType::kPerspective:
            interpolation_type = InterpolationType::kPerspective;
            break;
        case builtin::InterpolationType::kLinear:
            interpolation_type = InterpolationType::kLinear;
            break;
        case builtin::InterpolationType::kFlat:
            interpolation_type = InterpolationType::kFlat;
            break;
        case builtin::InterpolationType::kUndefined:
            break;
    }

    auto sampling_type = InterpolationSampling::kUnknown;
    switch (ast_sampling_type) {
        case builtin::InterpolationSampling::kUndefined:
            sampling_type = InterpolationSampling::kNone;
            break;
        case builtin::InterpolationSampling::kCenter:
            sampling_type = InterpolationSampling::kCenter;
            break;
        case builtin::InterpolationSampling::kCentroid:
            sampling_type = InterpolationSampling::kCentroid;
            break;
        case builtin::InterpolationSampling::kSample:
            sampling_type = InterpolationSampling::kSample;
            break;
    }

    return {interpolation_type, sampling_type};
}

template <size_t N, typename F>
void Inspector::GetOriginatingResources(std::array<const ast::Expression*, N> exprs, F&& callback) {
    if (TINT_UNLIKELY(!program_->IsValid())) {
        TINT_ICE(Inspector, diagnostics_)
            << "attempting to get originating resources in invalid program";
        return;
    }

    auto& sem = program_->Sem();

    std::array<const sem::GlobalVariable*, N> globals{};
    std::array<const sem::Parameter*, N> parameters{};
    utils::UniqueVector<const ast::CallExpression*, 8> callsites;

    for (size_t i = 0; i < N; i++) {
        const sem::Variable* root_ident = sem.GetVal(exprs[i])->RootIdentifier();
        if (auto* global = root_ident->As<sem::GlobalVariable>()) {
            globals[i] = global;
        } else if (auto* param = root_ident->As<sem::Parameter>()) {
            auto* func = tint::As<sem::Function>(param->Owner());
            if (func->CallSites().empty()) {
                // One or more of the expressions is a parameter, but this function
                // is not called. Ignore.
                return;
            }
            for (auto* call : func->CallSites()) {
                callsites.Add(call->Declaration());
            }
            parameters[i] = param;
        } else {
            TINT_ICE(Inspector, diagnostics_)
                << "cannot resolve originating resource with expression type "
                << exprs[i]->TypeInfo().name;
            return;
        }
    }

    if (callsites.Length()) {
        for (auto* call_expr : callsites) {
            // Make a copy of the expressions for this callsite
            std::array<const ast::Expression*, N> call_exprs = exprs;
            // Patch all the parameter expressions with their argument
            for (size_t i = 0; i < N; i++) {
                if (auto* param = parameters[i]) {
                    call_exprs[i] = call_expr->args[param->Index()];
                }
            }
            // Now call GetOriginatingResources() with from the callsite
            GetOriginatingResources(call_exprs, callback);
        }
    } else {
        // All the expressions resolved to globals
        callback(globals);
    }
}

}  // namespace tint::inspector
