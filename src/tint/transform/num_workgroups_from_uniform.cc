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

#include "src/tint/transform/num_workgroups_from_uniform.h"

#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::NumWorkgroupsFromUniform);
TINT_INSTANTIATE_TYPEINFO(tint::transform::NumWorkgroupsFromUniform::Config);

namespace tint::transform {
namespace {
/// Accessor describes the identifiers used in a member accessor that is being
/// used to retrieve the num_workgroups builtin from a parameter.
struct Accessor {
    Symbol param;
    Symbol member;

    /// Equality operator
    bool operator==(const Accessor& other) const {
        return param == other.param && member == other.member;
    }
    /// Hash function
    struct Hasher {
        size_t operator()(const Accessor& a) const { return utils::Hash(a.param, a.member); }
    };
};
}  // namespace

NumWorkgroupsFromUniform::NumWorkgroupsFromUniform() = default;
NumWorkgroupsFromUniform::~NumWorkgroupsFromUniform() = default;

bool NumWorkgroupsFromUniform::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* attr = node->As<ast::BuiltinAttribute>()) {
            if (attr->builtin == ast::Builtin::kNumWorkgroups) {
                return true;
            }
        }
    }
    return false;
}

void NumWorkgroupsFromUniform::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        ctx.dst->Diagnostics().add_error(
            diag::System::Transform, "missing transform data for " + std::string(TypeInfo().name));
        return;
    }

    const char* kNumWorkgroupsMemberName = "num_workgroups";

    // Find all entry point parameters that declare the num_workgroups builtin.
    std::unordered_set<Accessor, Accessor::Hasher> to_replace;
    for (auto* func : ctx.src->AST().Functions()) {
        // num_workgroups is only valid for compute stages.
        if (func->PipelineStage() != ast::PipelineStage::kCompute) {
            continue;
        }

        for (auto* param : ctx.src->Sem().Get(func)->Parameters()) {
            // Because the CanonicalizeEntryPointIO transform has been run, builtins
            // will only appear as struct members.
            auto* str = param->Type()->As<sem::Struct>();
            if (!str) {
                continue;
            }

            for (auto* member : str->Members()) {
                auto* builtin =
                    ast::GetAttribute<ast::BuiltinAttribute>(member->Declaration()->attributes);
                if (!builtin || builtin->builtin != ast::Builtin::kNumWorkgroups) {
                    continue;
                }

                // Capture the symbols that would be used to access this member, which
                // we will replace later. We currently have no way to get from the
                // parameter directly to the member accessor expressions that use it.
                to_replace.insert({param->Declaration()->symbol, member->Declaration()->symbol});

                // Remove the struct member.
                // The CanonicalizeEntryPointIO transform will have generated this
                // struct uniquely for this particular entry point, so we know that
                // there will be no other uses of this struct in the module and that we
                // can safely modify it here.
                ctx.Remove(str->Declaration()->members, member->Declaration());

                // If this is the only member, remove the struct and parameter too.
                if (str->Members().size() == 1) {
                    ctx.Remove(func->params, param->Declaration());
                    ctx.Remove(ctx.src->AST().GlobalDeclarations(), str->Declaration());
                }
            }
        }
    }

    // Get (or create, on first call) the uniform buffer that will receive the
    // number of workgroups.
    const ast::Variable* num_workgroups_ubo = nullptr;
    auto get_ubo = [&]() {
        if (!num_workgroups_ubo) {
            auto* num_workgroups_struct = ctx.dst->Structure(
                ctx.dst->Sym(),
                {ctx.dst->Member(kNumWorkgroupsMemberName, ctx.dst->ty.vec3(ctx.dst->ty.u32()))});
            num_workgroups_ubo = ctx.dst->Global(
                ctx.dst->Sym(), ctx.dst->ty.Of(num_workgroups_struct), ast::StorageClass::kUniform,
                ast::AttributeList{
                    ctx.dst->GroupAndBinding(cfg->ubo_binding.group, cfg->ubo_binding.binding)});
        }
        return num_workgroups_ubo;
    };

    // Now replace all the places where the builtins are accessed with the value
    // loaded from the uniform buffer.
    for (auto* node : ctx.src->ASTNodes().Objects()) {
        auto* accessor = node->As<ast::MemberAccessorExpression>();
        if (!accessor) {
            continue;
        }
        auto* ident = accessor->structure->As<ast::IdentifierExpression>();
        if (!ident) {
            continue;
        }

        if (to_replace.count({ident->symbol, accessor->member->symbol})) {
            ctx.Replace(accessor,
                        ctx.dst->MemberAccessor(get_ubo()->symbol, kNumWorkgroupsMemberName));
        }
    }

    ctx.Clone();
}

NumWorkgroupsFromUniform::Config::Config(sem::BindingPoint ubo_bp) : ubo_binding(ubo_bp) {}
NumWorkgroupsFromUniform::Config::Config(const Config&) = default;
NumWorkgroupsFromUniform::Config::~Config() = default;

}  // namespace tint::transform
