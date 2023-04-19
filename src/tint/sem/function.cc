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

#include "src/tint/sem/function.h"

#include "src/tint/ast/function.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/must_use_attribute.h"
#include "src/tint/sem/variable.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/utils/transform.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Function);

namespace tint::sem {

Function::Function(const ast::Function* declaration)
    : Base(EvaluationStage::kRuntime,
           ast::HasAttribute<ast::MustUseAttribute>(declaration->attributes)),
      declaration_(declaration),
      workgroup_size_{1, 1, 1} {}

Function::~Function() = default;

std::vector<std::pair<const Variable*, const ast::LocationAttribute*>>
Function::TransitivelyReferencedLocationVariables() const {
    std::vector<std::pair<const Variable*, const ast::LocationAttribute*>> ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        for (auto* attr : global->Declaration()->attributes) {
            if (auto* location = attr->As<ast::LocationAttribute>()) {
                ret.push_back({global, location});
                break;
            }
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedUniformVariables() const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        if (global->AddressSpace() != builtin::AddressSpace::kUniform) {
            continue;
        }

        if (auto bp = global->BindingPoint()) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedStorageBufferVariables() const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        if (global->AddressSpace() != builtin::AddressSpace::kStorage) {
            continue;
        }

        if (auto bp = global->BindingPoint()) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

std::vector<std::pair<const Variable*, const ast::BuiltinAttribute*>>
Function::TransitivelyReferencedBuiltinVariables() const {
    std::vector<std::pair<const Variable*, const ast::BuiltinAttribute*>> ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        for (auto* attr : global->Declaration()->attributes) {
            if (auto* builtin = attr->As<ast::BuiltinAttribute>()) {
                ret.push_back({global, builtin});
                break;
            }
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariables() const {
    return TransitivelyReferencedSamplerVariablesImpl(type::SamplerKind::kSampler);
}

Function::VariableBindings Function::TransitivelyReferencedComparisonSamplerVariables() const {
    return TransitivelyReferencedSamplerVariablesImpl(type::SamplerKind::kComparisonSampler);
}

Function::VariableBindings Function::TransitivelyReferencedSampledTextureVariables() const {
    return TransitivelyReferencedSampledTextureVariablesImpl(false);
}

Function::VariableBindings Function::TransitivelyReferencedMultisampledTextureVariables() const {
    return TransitivelyReferencedSampledTextureVariablesImpl(true);
}

Function::VariableBindings Function::TransitivelyReferencedVariablesOfType(
    const tint::utils::TypeInfo* type) const {
    VariableBindings ret;
    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        if (unwrapped_type->TypeInfo().Is(type)) {
            if (auto bp = global->BindingPoint()) {
                ret.push_back({global, *bp});
            }
        }
    }
    return ret;
}

bool Function::HasAncestorEntryPoint(Symbol symbol) const {
    for (const auto* point : ancestor_entry_points_) {
        if (point->Declaration()->name->symbol == symbol) {
            return true;
        }
    }
    return false;
}

Function::VariableBindings Function::TransitivelyReferencedSamplerVariablesImpl(
    type::SamplerKind kind) const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        auto* sampler = unwrapped_type->As<type::Sampler>();
        if (sampler == nullptr || sampler->kind() != kind) {
            continue;
        }

        if (auto bp = global->BindingPoint()) {
            ret.push_back({global, *bp});
        }
    }
    return ret;
}

Function::VariableBindings Function::TransitivelyReferencedSampledTextureVariablesImpl(
    bool multisampled) const {
    VariableBindings ret;

    for (auto* global : TransitivelyReferencedGlobals()) {
        auto* unwrapped_type = global->Type()->UnwrapRef();
        auto* texture = unwrapped_type->As<type::Texture>();
        if (texture == nullptr) {
            continue;
        }

        auto is_multisampled = texture->Is<type::MultisampledTexture>();
        auto is_sampled = texture->Is<type::SampledTexture>();

        if ((multisampled && !is_multisampled) || (!multisampled && !is_sampled)) {
            continue;
        }

        if (auto bp = global->BindingPoint()) {
            ret.push_back({global, *bp});
        }
    }

    return ret;
}

}  // namespace tint::sem
