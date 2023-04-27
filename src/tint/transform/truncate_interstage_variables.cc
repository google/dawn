// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/truncate_interstage_variables.h"

#include <memory>
#include <string>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/unicode.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::TruncateInterstageVariables);
TINT_INSTANTIATE_TYPEINFO(tint::transform::TruncateInterstageVariables::Config);

namespace tint::transform {

namespace {

struct TruncatedStructAndConverter {
    /// The symbol of the truncated structure.
    Symbol truncated_struct;
    /// The symbol of the helper function that takes the original structure as a single argument and
    /// returns the truncated structure type.
    Symbol truncate_fn;
};

}  // anonymous namespace

TruncateInterstageVariables::TruncateInterstageVariables() = default;
TruncateInterstageVariables::~TruncateInterstageVariables() = default;

Transform::ApplyResult TruncateInterstageVariables::Apply(const Program* src,
                                                          const DataMap& config,
                                                          DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    const auto* data = config.Get<Config>();
    if (data == nullptr) {
        b.Diagnostics().add_error(
            diag::System::Transform,
            "missing transform data for " +
                std::string(utils::TypeInfo::Of<TruncateInterstageVariables>().name));
        return Program(std::move(b));
    }

    auto& sem = ctx.src->Sem();

    bool should_run = false;

    utils::Hashmap<const sem::Function*, Symbol, 4u> entry_point_functions_to_truncate_functions;
    utils::Hashmap<const sem::Struct*, TruncatedStructAndConverter, 4u>
        old_shader_io_structs_to_new_struct_and_truncate_functions;

    for (auto* func_ast : ctx.src->AST().Functions()) {
        if (!func_ast->IsEntryPoint()) {
            continue;
        }

        if (func_ast->PipelineStage() != ast::PipelineStage::kVertex) {
            // Currently only vertex stage could have interstage output variables that need
            // truncated.
            continue;
        }

        auto* func_sem = sem.Get(func_ast);
        auto* str = func_sem->ReturnType()->As<sem::Struct>();

        // This transform is run after CanonicalizeEntryPointIO transform,
        // So it is guaranteed that entry point inputs are already grouped in a struct.
        if (TINT_UNLIKELY(!str)) {
            TINT_ICE(Transform, ctx.dst->Diagnostics())
                << "Entrypoint function return type is non-struct.\n"
                << "TruncateInterstageVariables transform needs to run after "
                   "CanonicalizeEntryPointIO transform.";
            continue;
        }

        // A prepass to check if any interstage variable locations in the entry point needs
        // truncating. If not we don't really need to handle this entry point.
        utils::Hashset<const sem::StructMember*, 16u> omit_members;

        for (auto* member : str->Members()) {
            if (auto location = member->Attributes().location) {
                if (!data->interstage_locations.test(location.value())) {
                    omit_members.Add(member);
                }
            }
        }

        if (omit_members.IsEmpty()) {
            continue;
        }

        // Now we are sure the transform needs to be run.
        should_run = true;

        // Get or create a new truncated struct/truncate function for the interstage inputs &
        // outputs.
        auto entry =
            old_shader_io_structs_to_new_struct_and_truncate_functions.GetOrCreate(str, [&] {
                auto new_struct_sym = b.Symbols().New();

                utils::Vector<const ast::StructMember*, 20> truncated_members;
                utils::Vector<const ast::Expression*, 20> initializer_exprs;

                for (auto* member : str->Members()) {
                    if (omit_members.Contains(member)) {
                        continue;
                    }

                    truncated_members.Push(ctx.Clone(member->Declaration()));
                    initializer_exprs.Push(b.MemberAccessor("io", ctx.Clone(member->Name())));
                }

                // Create the new shader io struct.
                b.Structure(new_struct_sym, std::move(truncated_members));

                // Create the mapping function to truncate the shader io.
                auto mapping_fn_sym = b.Symbols().New("truncate_shader_output");
                b.Func(mapping_fn_sym,
                       utils::Vector{b.Param("io", ctx.Clone(func_ast->return_type))},
                       b.ty(new_struct_sym),
                       utils::Vector{
                           b.Return(b.Call(new_struct_sym, std::move(initializer_exprs))),
                       });
                return TruncatedStructAndConverter{new_struct_sym, mapping_fn_sym};
            });

        ctx.Replace(func_ast->return_type.expr, b.Expr(entry.truncated_struct));

        entry_point_functions_to_truncate_functions.Add(func_sem, entry.truncate_fn);
    }

    if (!should_run) {
        return SkipTransform;
    }

    // Replace return statements with new truncated shader IO struct
    ctx.ReplaceAll(
        [&](const ast::ReturnStatement* return_statement) -> const ast::ReturnStatement* {
            auto* return_sem = sem.Get(return_statement);
            if (auto mapping_fn_sym =
                    entry_point_functions_to_truncate_functions.Find(return_sem->Function())) {
                return b.Return(return_statement->source,
                                b.Call(*mapping_fn_sym, ctx.Clone(return_statement->value)));
            }
            return nullptr;
        });

    // Remove IO attributes from old shader IO struct which is not used as entry point output
    // anymore.
    for (auto it : old_shader_io_structs_to_new_struct_and_truncate_functions) {
        const ast::Struct* struct_ty = it.key->Declaration();
        for (auto* member : struct_ty->members) {
            for (auto* attr : member->attributes) {
                if (attr->IsAnyOf<ast::BuiltinAttribute, ast::LocationAttribute,
                                  ast::InterpolateAttribute, ast::InvariantAttribute>()) {
                    ctx.Remove(member->attributes, attr);
                }
            }
        }
    }

    ctx.Clone();
    return Program(std::move(b));
}

TruncateInterstageVariables::Config::Config() = default;

TruncateInterstageVariables::Config::Config(const Config&) = default;

TruncateInterstageVariables::Config::~Config() = default;

TruncateInterstageVariables::Config& TruncateInterstageVariables::Config::operator=(const Config&) =
    default;

}  // namespace tint::transform
