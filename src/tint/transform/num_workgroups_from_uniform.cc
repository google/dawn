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

#include "src/tint/builtin/builtin_value.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/function.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::NumWorkgroupsFromUniform);
TINT_INSTANTIATE_TYPEINFO(tint::transform::NumWorkgroupsFromUniform::Config);

namespace tint::transform {
namespace {

bool ShouldRun(const Program* program) {
    for (auto* node : program->ASTNodes().Objects()) {
        if (auto* attr = node->As<ast::BuiltinAttribute>()) {
            if (program->Sem().Get(attr)->Value() == builtin::BuiltinValue::kNumWorkgroups) {
                return true;
            }
        }
    }
    return false;
}

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

Transform::ApplyResult NumWorkgroupsFromUniform::Apply(const Program* src,
                                                       const DataMap& inputs,
                                                       DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    auto* cfg = inputs.Get<Config>();
    if (cfg == nullptr) {
        b.Diagnostics().add_error(diag::System::Transform,
                                  "missing transform data for " + std::string(TypeInfo().name));
        return Program(std::move(b));
    }

    if (!ShouldRun(src)) {
        return SkipTransform;
    }

    const char* kNumWorkgroupsMemberName = "num_workgroups";

    // Find all entry point parameters that declare the num_workgroups builtin.
    std::unordered_set<Accessor, Accessor::Hasher> to_replace;
    for (auto* func : src->AST().Functions()) {
        // num_workgroups is only valid for compute stages.
        if (func->PipelineStage() != ast::PipelineStage::kCompute) {
            continue;
        }

        for (auto* param : src->Sem().Get(func)->Parameters()) {
            // Because the CanonicalizeEntryPointIO transform has been run, builtins
            // will only appear as struct members.
            auto* str = param->Type()->As<sem::Struct>();
            if (!str) {
                continue;
            }

            for (auto* member : str->Members()) {
                if (member->Attributes().builtin != builtin::BuiltinValue::kNumWorkgroups) {
                    continue;
                }

                // Capture the symbols that would be used to access this member, which
                // we will replace later. We currently have no way to get from the
                // parameter directly to the member accessor expressions that use it.
                to_replace.insert({param->Declaration()->name->symbol, member->Name()});

                // Remove the struct member.
                // The CanonicalizeEntryPointIO transform will have generated this
                // struct uniquely for this particular entry point, so we know that
                // there will be no other uses of this struct in the module and that we
                // can safely modify it here.
                ctx.Remove(str->Declaration()->members, member->Declaration());

                // If this is the only member, remove the struct and parameter too.
                if (str->Members().Length() == 1) {
                    ctx.Remove(func->params, param->Declaration());
                    ctx.Remove(src->AST().GlobalDeclarations(), str->Declaration());
                }
            }
        }
    }

    // Get (or create, on first call) the uniform buffer that will receive the
    // number of workgroups.
    const ast::Variable* num_workgroups_ubo = nullptr;
    auto get_ubo = [&]() {
        if (!num_workgroups_ubo) {
            auto* num_workgroups_struct =
                b.Structure(b.Sym(), utils::Vector{
                                         b.Member(kNumWorkgroupsMemberName, b.ty.vec3(b.ty.u32())),
                                     });

            uint32_t group, binding;
            if (cfg->ubo_binding.has_value()) {
                // If cfg->ubo_binding holds a value, use the specified binding point.
                group = cfg->ubo_binding->group;
                binding = cfg->ubo_binding->binding;
            } else {
                // If cfg->ubo_binding holds no value, use the binding 0 of the largest used group
                // plus 1, or group 0 if no resource bound.
                group = 0;

                for (auto* global : src->AST().GlobalVariables()) {
                    auto* global_sem = src->Sem().Get<sem::GlobalVariable>(global);
                    if (auto bp = global_sem->BindingPoint()) {
                        if (bp->group >= group) {
                            group = bp->group + 1;
                        }
                    }
                }

                binding = 0;
            }

            num_workgroups_ubo = b.GlobalVar(b.Sym(), b.ty.Of(num_workgroups_struct),
                                             builtin::AddressSpace::kUniform, b.Group(AInt(group)),
                                             b.Binding(AInt(binding)));
        }
        return num_workgroups_ubo;
    };

    // Now replace all the places where the builtins are accessed with the value
    // loaded from the uniform buffer.
    for (auto* node : src->ASTNodes().Objects()) {
        auto* accessor = node->As<ast::MemberAccessorExpression>();
        if (!accessor) {
            continue;
        }
        auto* ident = accessor->object->As<ast::IdentifierExpression>();
        if (!ident) {
            continue;
        }

        if (to_replace.count({ident->identifier->symbol, accessor->member->symbol})) {
            ctx.Replace(accessor,
                        b.MemberAccessor(get_ubo()->name->symbol, kNumWorkgroupsMemberName));
        }
    }

    ctx.Clone();
    return Program(std::move(b));
}

NumWorkgroupsFromUniform::Config::Config(std::optional<sem::BindingPoint> ubo_bp)
    : ubo_binding(ubo_bp) {}
NumWorkgroupsFromUniform::Config::Config(const Config&) = default;
NumWorkgroupsFromUniform::Config::~Config() = default;

}  // namespace tint::transform
