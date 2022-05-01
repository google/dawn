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
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/location_attribute.h"
#include "src/tint/ast/module.h"
#include "src/tint/ast/sint_literal_expression.h"
#include "src/tint/ast/uint_literal_expression.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/f32.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/u32.h"
#include "src/tint/sem/variable.h"
#include "src/tint/sem/vector.h"
#include "src/tint/sem/void.h"
#include "src/tint/utils/math.h"
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

std::tuple<ComponentType, CompositionType> CalculateComponentAndComposition(const sem::Type* type) {
    if (type->is_float_scalar()) {
        return {ComponentType::kFloat, CompositionType::kScalar};
    } else if (type->is_float_vector()) {
        auto* vec = type->As<sem::Vector>();
        if (vec->Width() == 2) {
            return {ComponentType::kFloat, CompositionType::kVec2};
        } else if (vec->Width() == 3) {
            return {ComponentType::kFloat, CompositionType::kVec3};
        } else if (vec->Width() == 4) {
            return {ComponentType::kFloat, CompositionType::kVec4};
        }
    } else if (type->is_unsigned_integer_scalar()) {
        return {ComponentType::kUInt, CompositionType::kScalar};
    } else if (type->is_unsigned_integer_vector()) {
        auto* vec = type->As<sem::Vector>();
        if (vec->Width() == 2) {
            return {ComponentType::kUInt, CompositionType::kVec2};
        } else if (vec->Width() == 3) {
            return {ComponentType::kUInt, CompositionType::kVec3};
        } else if (vec->Width() == 4) {
            return {ComponentType::kUInt, CompositionType::kVec4};
        }
    } else if (type->is_signed_integer_scalar()) {
        return {ComponentType::kSInt, CompositionType::kScalar};
    } else if (type->is_signed_integer_vector()) {
        auto* vec = type->As<sem::Vector>();
        if (vec->Width() == 2) {
            return {ComponentType::kSInt, CompositionType::kVec2};
        } else if (vec->Width() == 3) {
            return {ComponentType::kSInt, CompositionType::kVec3};
        } else if (vec->Width() == 4) {
            return {ComponentType::kSInt, CompositionType::kVec4};
        }
    }
    return {ComponentType::kUnknown, CompositionType::kUnknown};
}

std::tuple<InterpolationType, InterpolationSampling> CalculateInterpolationData(
    const sem::Type* type,
    const ast::AttributeList& attributes) {
    auto* interpolation_attribute = ast::GetAttribute<ast::InterpolateAttribute>(attributes);
    if (type->is_integer_scalar_or_vector()) {
        return {InterpolationType::kFlat, InterpolationSampling::kNone};
    }

    if (!interpolation_attribute) {
        return {InterpolationType::kPerspective, InterpolationSampling::kCenter};
    }

    auto interpolation_type = interpolation_attribute->type;
    auto sampling = interpolation_attribute->sampling;
    if (interpolation_type != ast::InterpolationType::kFlat &&
        sampling == ast::InterpolationSampling::kNone) {
        sampling = ast::InterpolationSampling::kCenter;
    }
    return {ASTToInspectorInterpolationType(interpolation_type),
            ASTToInspectorInterpolationSampling(sampling)};
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
        entry_point.name = program_->Symbols().NameFor(func->symbol);
        entry_point.remapped_name = program_->Symbols().NameFor(func->symbol);
        entry_point.stage = func->PipelineStage();

        auto wgsize = sem->WorkgroupSize();
        entry_point.workgroup_size_x = wgsize[0].value;
        entry_point.workgroup_size_y = wgsize[1].value;
        entry_point.workgroup_size_z = wgsize[2].value;
        if (wgsize[0].overridable_const || wgsize[1].overridable_const ||
            wgsize[2].overridable_const) {
            // TODO(crbug.com/tint/713): Handle overridable constants.
            TINT_ASSERT(Inspector, false);
        }

        for (auto* param : sem->Parameters()) {
            AddEntryPointInOutVariables(program_->Symbols().NameFor(param->Declaration()->symbol),
                                        param->Type(), param->Declaration()->attributes,
                                        entry_point.input_variables);

            entry_point.input_position_used |= ContainsBuiltin(
                ast::Builtin::kPosition, param->Type(), param->Declaration()->attributes);
            entry_point.front_facing_used |= ContainsBuiltin(
                ast::Builtin::kFrontFacing, param->Type(), param->Declaration()->attributes);
            entry_point.sample_index_used |= ContainsBuiltin(
                ast::Builtin::kSampleIndex, param->Type(), param->Declaration()->attributes);
            entry_point.input_sample_mask_used |= ContainsBuiltin(
                ast::Builtin::kSampleMask, param->Type(), param->Declaration()->attributes);
            entry_point.num_workgroups_used |= ContainsBuiltin(
                ast::Builtin::kNumWorkgroups, param->Type(), param->Declaration()->attributes);
        }

        if (!sem->ReturnType()->Is<sem::Void>()) {
            AddEntryPointInOutVariables("<retval>", sem->ReturnType(), func->return_type_attributes,
                                        entry_point.output_variables);

            entry_point.output_sample_mask_used = ContainsBuiltin(
                ast::Builtin::kSampleMask, sem->ReturnType(), func->return_type_attributes);
        }

        for (auto* var : sem->TransitivelyReferencedGlobals()) {
            auto* decl = var->Declaration();

            auto name = program_->Symbols().NameFor(decl->symbol);

            auto* global = var->As<sem::GlobalVariable>();
            if (global && global->IsOverridable()) {
                OverridableConstant overridable_constant;
                overridable_constant.name = name;
                overridable_constant.numeric_id = global->ConstantId();
                auto* type = var->Type();
                TINT_ASSERT(Inspector, type->is_scalar());
                if (type->is_bool_scalar_or_vector()) {
                    overridable_constant.type = OverridableConstant::Type::kBool;
                } else if (type->is_float_scalar()) {
                    overridable_constant.type = OverridableConstant::Type::kFloat32;
                } else if (type->is_signed_integer_scalar()) {
                    overridable_constant.type = OverridableConstant::Type::kInt32;
                } else if (type->is_unsigned_integer_scalar()) {
                    overridable_constant.type = OverridableConstant::Type::kUint32;
                } else {
                    TINT_UNREACHABLE(Inspector, diagnostics_);
                }

                overridable_constant.is_initialized = global->Declaration()->constructor;
                overridable_constant.is_numeric_id_specified =
                    ast::HasAttribute<ast::IdAttribute>(global->Declaration()->attributes);

                entry_point.overridable_constants.push_back(overridable_constant);
            }
        }

        result.push_back(std::move(entry_point));
    }

    return result;
}

std::map<uint32_t, Scalar> Inspector::GetConstantIDs() {
    std::map<uint32_t, Scalar> result;
    for (auto* var : program_->AST().GlobalVariables()) {
        auto* global = program_->Sem().Get<sem::GlobalVariable>(var);
        if (!global || !global->IsOverridable()) {
            continue;
        }

        // If there are conflicting defintions for a constant id, that is invalid
        // WGSL, so the resolver should catch it. Thus here the inspector just
        // assumes all definitions of the constant id are the same, so only needs
        // to find the first reference to constant id.
        uint32_t constant_id = global->ConstantId();
        if (result.find(constant_id) != result.end()) {
            continue;
        }

        if (!var->constructor) {
            result[constant_id] = Scalar();
            continue;
        }

        auto* literal = var->constructor->As<ast::LiteralExpression>();
        if (!literal) {
            // This is invalid WGSL, but handling gracefully.
            result[constant_id] = Scalar();
            continue;
        }

        if (auto* l = literal->As<ast::BoolLiteralExpression>()) {
            result[constant_id] = Scalar(l->value);
            continue;
        }

        if (auto* l = literal->As<ast::UintLiteralExpression>()) {
            result[constant_id] = Scalar(l->value);
            continue;
        }

        if (auto* l = literal->As<ast::SintLiteralExpression>()) {
            result[constant_id] = Scalar(l->value);
            continue;
        }

        if (auto* l = literal->As<ast::FloatLiteralExpression>()) {
            result[constant_id] = Scalar(l->value);
            continue;
        }

        result[constant_id] = Scalar();
    }

    return result;
}

std::map<std::string, uint32_t> Inspector::GetConstantNameToIdMap() {
    std::map<std::string, uint32_t> result;
    for (auto* var : program_->AST().GlobalVariables()) {
        auto* global = program_->Sem().Get<sem::GlobalVariable>(var);
        if (global && global->IsOverridable()) {
            auto name = program_->Symbols().NameFor(var->symbol);
            result[name] = global->ConstantId();
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
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;
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
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;

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
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;

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
    const tint::TypeInfo* texture_type,
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
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;

        auto* tex = var->Type()->UnwrapRef()->As<sem::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(tex->dim());

        result.push_back(entry);
    }

    return result;
}

std::vector<ResourceBinding> Inspector::GetDepthTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &TypeInfo::Of<sem::DepthTexture>(),
                                      ResourceBinding::ResourceType::kDepthTexture);
}

std::vector<ResourceBinding> Inspector::GetDepthMultisampledTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &TypeInfo::Of<sem::DepthMultisampledTexture>(),
                                      ResourceBinding::ResourceType::kDepthMultisampledTexture);
}

std::vector<ResourceBinding> Inspector::GetExternalTextureResourceBindings(
    const std::string& entry_point) {
    return GetTextureResourceBindings(entry_point, &TypeInfo::Of<sem::ExternalTexture>(),
                                      ResourceBinding::ResourceType::kExternalTexture);
}

std::vector<sem::SamplerTexturePair> Inspector::GetSamplerTextureUses(
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

std::vector<sem::SamplerTexturePair> Inspector::GetSamplerTextureUses(
    const std::string& entry_point,
    const sem::BindingPoint& placeholder) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }
    auto* func_sem = program_->Sem().Get(func);

    std::vector<sem::SamplerTexturePair> new_pairs;
    for (auto pair : func_sem->TextureSamplerPairs()) {
        auto* texture = pair.first->As<sem::GlobalVariable>();
        auto* sampler = pair.second ? pair.second->As<sem::GlobalVariable>() : nullptr;
        SamplerTexturePair new_pair;
        new_pair.sampler_binding_point = sampler ? sampler->BindingPoint() : placeholder;
        new_pair.texture_binding_point = texture->BindingPoint();
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
        if (var->StorageClass() == ast::StorageClass::kWorkgroup) {
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
    std::vector<std::string> result;

    ast::ExtensionSet set = program_->AST().Extensions();
    result.reserve(set.size());
    for (auto kind : set) {
        std::string name = ast::Enable::KindToName(kind);
        result.push_back(name);
    }

    return result;
}

std::vector<std::pair<std::string, Source>> Inspector::GetEnableDirectives() {
    std::vector<std::pair<std::string, Source>> result;

    // Ast nodes for enable directive are stored within global declarations list
    auto global_decls = program_->AST().GlobalDeclarations();
    for (auto node : global_decls) {
        if (auto ext = node->As<ast::Enable>()) {
            result.push_back({ext->name, ext->source});
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
                                            const sem::Type* type,
                                            const ast::AttributeList& attributes,
                                            std::vector<StageVariable>& variables) const {
    // Skip builtins.
    if (ast::HasAttribute<ast::BuiltinAttribute>(attributes)) {
        return;
    }

    auto* unwrapped_type = type->UnwrapRef();

    if (auto* struct_ty = unwrapped_type->As<sem::Struct>()) {
        // Recurse into members.
        for (auto* member : struct_ty->Members()) {
            AddEntryPointInOutVariables(
                name + "." + program_->Symbols().NameFor(member->Declaration()->symbol),
                member->Type(), member->Declaration()->attributes, variables);
        }
        return;
    }

    // Base case: add the variable.

    StageVariable stage_variable;
    stage_variable.name = name;
    std::tie(stage_variable.component_type, stage_variable.composition_type) =
        CalculateComponentAndComposition(type);

    auto* location = ast::GetAttribute<ast::LocationAttribute>(attributes);
    TINT_ASSERT(Inspector, location != nullptr);
    stage_variable.has_location_attribute = true;
    stage_variable.location_attribute = location->value;

    std::tie(stage_variable.interpolation_type, stage_variable.interpolation_sampling) =
        CalculateInterpolationData(type, attributes);

    variables.push_back(stage_variable);
}

bool Inspector::ContainsBuiltin(ast::Builtin builtin,
                                const sem::Type* type,
                                const ast::AttributeList& attributes) const {
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
    if (!builtin_declaration || builtin_declaration->builtin != builtin) {
        return false;
    }

    return true;
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

        if (read_only != (var->Access() == ast::Access::kRead)) {
            continue;
        }

        auto* unwrapped_type = var->Type()->UnwrapRef();

        ResourceBinding entry;
        entry.resource_type = read_only ? ResourceBinding::ResourceType::kReadOnlyStorageBuffer
                                        : ResourceBinding::ResourceType::kStorageBuffer;
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;
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
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;

        auto* texture_type = var->Type()->UnwrapRef()->As<sem::Texture>();
        entry.dim = TypeTextureDimensionToResourceBindingTextureDimension(texture_type->dim());

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
    const std::string& entry_point) {
    auto* func = FindEntryPointByName(entry_point);
    if (!func) {
        return {};
    }

    auto* func_sem = program_->Sem().Get(func);
    std::vector<ResourceBinding> result;
    for (auto& ref : func_sem->TransitivelyReferencedVariablesOfType<sem::StorageTexture>()) {
        auto* var = ref.first;
        auto binding_info = ref.second;

        auto* texture_type = var->Type()->UnwrapRef()->As<sem::StorageTexture>();

        ResourceBinding entry;
        entry.resource_type = ResourceBinding::ResourceType::kWriteOnlyStorageTexture;
        entry.bind_group = binding_info.group->value;
        entry.binding = binding_info.binding->value;

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
        std::unordered_map<std::string, utils::UniqueVector<sem::SamplerTexturePair>>>();

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

        auto* t = c->args[texture_index];
        auto* s = c->args[sampler_index];

        GetOriginatingResources(std::array<const ast::Expression*, 2>{t, s},
                                [&](std::array<const sem::GlobalVariable*, 2> globals) {
                                    auto* texture = globals[0];
                                    sem::BindingPoint texture_binding_point = {
                                        texture->Declaration()->BindingPoint().group->value,
                                        texture->Declaration()->BindingPoint().binding->value};

                                    auto* sampler = globals[1];
                                    sem::BindingPoint sampler_binding_point = {
                                        sampler->Declaration()->BindingPoint().group->value,
                                        sampler->Declaration()->BindingPoint().binding->value};

                                    for (auto* entry_point : entry_points) {
                                        const auto& ep_name = program_->Symbols().NameFor(
                                            entry_point->Declaration()->symbol);
                                        (*sampler_targets_)[ep_name].add(
                                            {sampler_binding_point, texture_binding_point});
                                    }
                                });
    }
}

template <size_t N, typename F>
void Inspector::GetOriginatingResources(std::array<const ast::Expression*, N> exprs, F&& callback) {
    if (!program_->IsValid()) {
        TINT_ICE(Inspector, diagnostics_)
            << "attempting to get originating resources in invalid program";
        return;
    }

    auto& sem = program_->Sem();

    std::array<const sem::GlobalVariable*, N> globals{};
    std::array<const sem::Parameter*, N> parameters{};
    utils::UniqueVector<const ast::CallExpression*> callsites;

    for (size_t i = 0; i < N; i++) {
        const sem::Variable* source_var = sem.Get(exprs[i])->SourceVariable();
        if (auto* global = source_var->As<sem::GlobalVariable>()) {
            globals[i] = global;
        } else if (auto* param = source_var->As<sem::Parameter>()) {
            auto* func = tint::As<sem::Function>(param->Owner());
            if (func->CallSites().empty()) {
                // One or more of the expressions is a parameter, but this function
                // is not called. Ignore.
                return;
            }
            for (auto* call : func->CallSites()) {
                callsites.add(call->Declaration());
            }
            parameters[i] = param;
        } else {
            TINT_ICE(Inspector, diagnostics_)
                << "cannot resolve originating resource with expression type "
                << exprs[i]->TypeInfo().name;
            return;
        }
    }

    if (callsites.size()) {
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
