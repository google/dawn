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

#include "src/tint/resolver/resolver.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/array.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/attribute.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/depth_texture.h"
#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/matrix.h"
#include "src/tint/ast/pointer.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/sampled_texture.h"
#include "src/tint/ast/sampler.h"
#include "src/tint/ast/storage_texture.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/ast/type_name.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/vector.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/resolver/uniformity.h"
#include "src/tint/sem/abstract_float.h"
#include "src/tint/sem/abstract_int.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/break_if_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/loop_statement.h"
#include "src/tint/sem/materialize.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/pointer.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/sampler.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/type_initializer.h"
#include "src/tint/sem/variable.h"
#include "src/tint/sem/while_statement.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/reverse.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/transform.h"
#include "src/tint/utils/vector.h"

namespace tint::resolver {
namespace {

constexpr int64_t kMaxArrayElementCount = 65536;
constexpr uint32_t kMaxStatementDepth = 127;

}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : builder_(builder),
      diagnostics_(builder->Diagnostics()),
      const_eval_(*builder),
      intrinsic_table_(IntrinsicTable::Create(*builder)),
      sem_(builder, dependencies_),
      validator_(builder, sem_) {}

Resolver::~Resolver() = default;

bool Resolver::Resolve() {
    if (builder_->Diagnostics().contains_errors()) {
        return false;
    }

    builder_->Sem().Reserve(builder_->LastAllocatedNodeID());

    // Pre-allocate the marked bitset with the total number of AST nodes.
    marked_.Resize(builder_->ASTNodes().Count());

    if (!DependencyGraph::Build(builder_->AST(), builder_->Symbols(), builder_->Diagnostics(),
                                dependencies_)) {
        return false;
    }

    bool result = ResolveInternal();

    if (!result && !diagnostics_.contains_errors()) {
        TINT_ICE(Resolver, diagnostics_) << "resolving failed, but no error was raised";
        return false;
    }

    // Create the semantic module
    builder_->Sem().SetModule(builder_->create<sem::Module>(
        std::move(dependencies_.ordered_globals), std::move(enabled_extensions_)));

    return result;
}

bool Resolver::ResolveInternal() {
    Mark(&builder_->AST());

    // Process all module-scope declarations in dependency order.
    for (auto* decl : dependencies_.ordered_globals) {
        Mark(decl);
        if (!Switch<bool>(
                decl,  //
                [&](const ast::Enable* e) { return Enable(e); },
                [&](const ast::TypeDecl* td) { return TypeDecl(td); },
                [&](const ast::Function* func) { return Function(func); },
                [&](const ast::Variable* var) { return GlobalVariable(var); },
                [&](const ast::StaticAssert* sa) { return StaticAssert(sa); },
                [&](Default) {
                    TINT_UNREACHABLE(Resolver, diagnostics_)
                        << "unhandled global declaration: " << decl->TypeInfo().name;
                    return false;
                })) {
            return false;
        }
    }

    if (!AllocateOverridableConstantIds()) {
        return false;
    }

    SetShadows();

    if (!validator_.PipelineStages(entry_points_)) {
        return false;
    }

    if (!validator_.PushConstants(entry_points_)) {
        return false;
    }

    if (!enabled_extensions_.Contains(ast::Extension::kChromiumDisableUniformityAnalysis)) {
        if (!AnalyzeUniformity(builder_, dependencies_)) {
            if (kUniformityFailuresAsError) {
                return false;
            }
        }
    }

    bool result = true;
    for (auto* node : builder_->ASTNodes().Objects()) {
        if (!marked_[node->node_id.value]) {
            TINT_ICE(Resolver, diagnostics_)
                << "AST node '" << node->TypeInfo().name << "' was not reached by the resolver\n"
                << "At: " << node->source << "\n"
                << "Pointer: " << node;
            result = false;
        }
    }

    return result;
}

sem::Type* Resolver::Type(const ast::Type* ty) {
    Mark(ty);
    auto* s = Switch(
        ty,  //
        [&](const ast::Void*) { return builder_->create<sem::Void>(); },
        [&](const ast::Bool*) { return builder_->create<sem::Bool>(); },
        [&](const ast::I32*) { return builder_->create<sem::I32>(); },
        [&](const ast::U32*) { return builder_->create<sem::U32>(); },
        [&](const ast::F16* t) -> sem::F16* {
            // Validate if f16 type is allowed.
            if (!enabled_extensions_.Contains(ast::Extension::kF16)) {
                AddError("f16 used without 'f16' extension enabled", t->source);
                return nullptr;
            }
            return builder_->create<sem::F16>();
        },
        [&](const ast::F32*) { return builder_->create<sem::F32>(); },
        [&](const ast::Vector* t) -> sem::Vector* {
            if (!t->type) {
                AddError("missing vector element type", t->source.End());
                return nullptr;
            }
            if (auto* el = Type(t->type)) {
                if (auto* vector = builder_->create<sem::Vector>(el, t->width)) {
                    if (validator_.Vector(vector, t->source)) {
                        return vector;
                    }
                }
            }
            return nullptr;
        },
        [&](const ast::Matrix* t) -> sem::Matrix* {
            if (!t->type) {
                AddError("missing matrix element type", t->source.End());
                return nullptr;
            }
            if (auto* el = Type(t->type)) {
                if (auto* column_type = builder_->create<sem::Vector>(el, t->rows)) {
                    if (auto* matrix = builder_->create<sem::Matrix>(column_type, t->columns)) {
                        if (validator_.Matrix(matrix, t->source)) {
                            return matrix;
                        }
                    }
                }
            }
            return nullptr;
        },
        [&](const ast::Array* t) { return Array(t); },
        [&](const ast::Atomic* t) -> sem::Atomic* {
            if (auto* el = Type(t->type)) {
                auto* a = builder_->create<sem::Atomic>(el);
                if (!validator_.Atomic(t, a)) {
                    return nullptr;
                }
                return a;
            }
            return nullptr;
        },
        [&](const ast::Pointer* t) -> sem::Pointer* {
            if (auto* el = Type(t->type)) {
                auto access = t->access;
                if (access == ast::Access::kUndefined) {
                    access = DefaultAccessForAddressSpace(t->address_space);
                }
                return builder_->create<sem::Pointer>(el, t->address_space, access);
            }
            return nullptr;
        },
        [&](const ast::Sampler* t) { return builder_->create<sem::Sampler>(t->kind); },
        [&](const ast::SampledTexture* t) -> sem::SampledTexture* {
            if (auto* el = Type(t->type)) {
                auto* sem = builder_->create<sem::SampledTexture>(t->dim, el);
                if (!validator_.SampledTexture(sem, t->source)) {
                    return nullptr;
                }
                return sem;
            }
            return nullptr;
        },
        [&](const ast::MultisampledTexture* t) -> sem::MultisampledTexture* {
            if (auto* el = Type(t->type)) {
                auto* sem = builder_->create<sem::MultisampledTexture>(t->dim, el);
                if (!validator_.MultisampledTexture(sem, t->source)) {
                    return nullptr;
                }
                return sem;
            }
            return nullptr;
        },
        [&](const ast::DepthTexture* t) { return builder_->create<sem::DepthTexture>(t->dim); },
        [&](const ast::DepthMultisampledTexture* t) {
            return builder_->create<sem::DepthMultisampledTexture>(t->dim);
        },
        [&](const ast::StorageTexture* t) -> sem::StorageTexture* {
            if (auto* el = Type(t->type)) {
                if (!validator_.StorageTexture(t)) {
                    return nullptr;
                }
                return builder_->create<sem::StorageTexture>(t->dim, t->format, t->access, el);
            }
            return nullptr;
        },
        [&](const ast::ExternalTexture*) { return builder_->create<sem::ExternalTexture>(); },
        [&](Default) {
            auto* resolved = sem_.ResolvedSymbol(ty);
            return Switch(
                resolved,  //
                [&](sem::Type* type) { return type; },
                [&](sem::Variable* var) {
                    auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                    AddError("cannot use variable '" + name + "' as type", ty->source);
                    AddNote("'" + name + "' declared here", var->Declaration()->source);
                    return nullptr;
                },
                [&](sem::Function* func) {
                    auto name = builder_->Symbols().NameFor(func->Declaration()->symbol);
                    AddError("cannot use function '" + name + "' as type", ty->source);
                    AddNote("'" + name + "' declared here", func->Declaration()->source);
                    return nullptr;
                },
                [&](Default) {
                    if (auto* tn = ty->As<ast::TypeName>()) {
                        if (IsBuiltin(tn->name)) {
                            auto name = builder_->Symbols().NameFor(tn->name);
                            AddError("cannot use builtin '" + name + "' as type", ty->source);
                            return nullptr;
                        }
                    }
                    TINT_UNREACHABLE(Resolver, diagnostics_)
                        << "Unhandled resolved type '"
                        << (resolved ? resolved->TypeInfo().name : "<null>")
                        << "' resolved from ast::Type '" << ty->TypeInfo().name << "'";
                    return nullptr;
                });
        });

    if (s) {
        builder_->Sem().Add(ty, s);
    }
    return s;
}

sem::Variable* Resolver::Variable(const ast::Variable* v, bool is_global) {
    return Switch(
        v,  //
        [&](const ast::Var* var) { return Var(var, is_global); },
        [&](const ast::Let* let) { return Let(let, is_global); },
        [&](const ast::Override* override) { return Override(override); },
        [&](const ast::Const* const_) { return Const(const_, is_global); },
        [&](Default) {
            TINT_ICE(Resolver, diagnostics_)
                << "Resolver::GlobalVariable() called with a unknown variable type: "
                << v->TypeInfo().name;
            return nullptr;
        });
}

sem::Variable* Resolver::Let(const ast::Let* v, bool is_global) {
    const sem::Type* ty = nullptr;

    // If the variable has a declared type, resolve it.
    if (v->type) {
        ty = Type(v->type);
        if (!ty) {
            return nullptr;
        }
    }

    if (!v->initializer) {
        AddError("'let' declaration must have an initializer", v->source);
        return nullptr;
    }

    auto* rhs = Materialize(Expression(v->initializer), ty);
    if (!rhs) {
        return nullptr;
    }

    // If the variable has no declared type, infer it from the RHS
    if (!ty) {
        ty = rhs->Type()->UnwrapRef();  // Implicit load of RHS
    }

    if (rhs && !validator_.VariableInitializer(v, ast::AddressSpace::kNone, ty, rhs)) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(ast::AddressSpace::kNone, const_cast<sem::Type*>(ty),
                                      v->source)) {
        AddNote("while instantiating 'let' " + builder_->Symbols().NameFor(v->symbol), v->source);
        return nullptr;
    }

    sem::Variable* sem = nullptr;
    if (is_global) {
        sem = builder_->create<sem::GlobalVariable>(
            v, ty, sem::EvaluationStage::kRuntime, ast::AddressSpace::kNone,
            ast::Access::kUndefined,
            /* constant_value */ nullptr, sem::BindingPoint{}, std::nullopt);
    } else {
        sem = builder_->create<sem::LocalVariable>(v, ty, sem::EvaluationStage::kRuntime,
                                                   ast::AddressSpace::kNone,
                                                   ast::Access::kUndefined, current_statement_,
                                                   /* constant_value */ nullptr);
    }

    sem->SetInitializer(rhs);
    builder_->Sem().Add(v, sem);
    return sem;
}

sem::Variable* Resolver::Override(const ast::Override* v) {
    const sem::Type* ty = nullptr;

    // If the variable has a declared type, resolve it.
    if (v->type) {
        ty = Type(v->type);
        if (!ty) {
            return nullptr;
        }
    }

    const sem::Expression* rhs = nullptr;

    // Does the variable have a initializer?
    if (v->initializer) {
        ExprEvalStageConstraint constraint{sem::EvaluationStage::kOverride, "override initializer"};
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
        rhs = Materialize(Expression(v->initializer), ty);
        if (!rhs) {
            return nullptr;
        }

        // If the variable has no declared type, infer it from the RHS
        if (!ty) {
            ty = rhs->Type()->UnwrapRef();  // Implicit load of RHS
        }
    } else if (!ty) {
        AddError("override declaration requires a type or initializer", v->source);
        return nullptr;
    }

    if (rhs && !validator_.VariableInitializer(v, ast::AddressSpace::kNone, ty, rhs)) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(ast::AddressSpace::kNone, const_cast<sem::Type*>(ty),
                                      v->source)) {
        AddNote("while instantiating 'override' " + builder_->Symbols().NameFor(v->symbol),
                v->source);
        return nullptr;
    }

    auto* sem = builder_->create<sem::GlobalVariable>(
        v, ty, sem::EvaluationStage::kOverride, ast::AddressSpace::kNone, ast::Access::kUndefined,
        /* constant_value */ nullptr, sem::BindingPoint{}, std::nullopt);
    sem->SetInitializer(rhs);

    if (auto* id_attr = ast::GetAttribute<ast::IdAttribute>(v->attributes)) {
        ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@id"};
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

        auto* materialized = Materialize(Expression(id_attr->expr));
        if (!materialized) {
            return nullptr;
        }
        if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
            AddError("@id must be an i32 or u32 value", id_attr->source);
            return nullptr;
        }

        auto const_value = materialized->ConstantValue();
        auto value = const_value->As<AInt>();
        if (value < 0) {
            AddError("@id value must be non-negative", id_attr->source);
            return nullptr;
        }
        if (value > std::numeric_limits<decltype(OverrideId::value)>::max()) {
            AddError("@id value must be between 0 and " +
                         std::to_string(std::numeric_limits<decltype(OverrideId::value)>::max()),
                     id_attr->source);
            return nullptr;
        }

        auto o = OverrideId{static_cast<decltype(OverrideId::value)>(value)};
        sem->SetOverrideId(o);

        // Track the constant IDs that are specified in the shader.
        override_ids_.Add(o, sem);
    }

    builder_->Sem().Add(v, sem);
    return sem;
}

sem::Variable* Resolver::Const(const ast::Const* c, bool is_global) {
    const sem::Type* ty = nullptr;

    // If the variable has a declared type, resolve it.
    if (c->type) {
        ty = Type(c->type);
        if (!ty) {
            return nullptr;
        }
    }

    if (!c->initializer) {
        AddError("'const' declaration must have an initializer", c->source);
        return nullptr;
    }

    const sem::Expression* rhs = nullptr;
    {
        ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "const initializer"};
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
        rhs = Expression(c->initializer);
        if (!rhs) {
            return nullptr;
        }
    }

    if (ty) {
        // If an explicit type was specified, materialize to that type
        rhs = Materialize(rhs, ty);
        if (!rhs) {
            return nullptr;
        }
    } else {
        // If no type was specified, infer it from the RHS
        ty = rhs->Type();
    }

    if (!validator_.VariableInitializer(c, ast::AddressSpace::kNone, ty, rhs)) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(ast::AddressSpace::kNone, const_cast<sem::Type*>(ty),
                                      c->source)) {
        AddNote("while instantiating 'const' " + builder_->Symbols().NameFor(c->symbol), c->source);
        return nullptr;
    }

    const auto value = rhs->ConstantValue();
    auto* sem = is_global ? static_cast<sem::Variable*>(builder_->create<sem::GlobalVariable>(
                                c, ty, sem::EvaluationStage::kConstant, ast::AddressSpace::kNone,
                                ast::Access::kUndefined, value, sem::BindingPoint{}, std::nullopt))
                          : static_cast<sem::Variable*>(builder_->create<sem::LocalVariable>(
                                c, ty, sem::EvaluationStage::kConstant, ast::AddressSpace::kNone,
                                ast::Access::kUndefined, current_statement_, value));

    sem->SetInitializer(rhs);
    builder_->Sem().Add(c, sem);
    return sem;
}

sem::Variable* Resolver::Var(const ast::Var* var, bool is_global) {
    const sem::Type* storage_ty = nullptr;

    // If the variable has a declared type, resolve it.
    if (auto* ty = var->type) {
        storage_ty = Type(ty);
        if (!storage_ty) {
            return nullptr;
        }
    }

    const sem::Expression* rhs = nullptr;

    // Does the variable have a initializer?
    if (var->initializer) {
        ExprEvalStageConstraint constraint{
            is_global ? sem::EvaluationStage::kOverride : sem::EvaluationStage::kRuntime,
            "var initializer",
        };
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

        rhs = Materialize(Expression(var->initializer), storage_ty);
        if (!rhs) {
            return nullptr;
        }
        // If the variable has no declared type, infer it from the RHS
        if (!storage_ty) {
            storage_ty = rhs->Type()->UnwrapRef();  // Implicit load of RHS
        }
    }

    if (!storage_ty) {
        AddError("var declaration requires a type or initializer", var->source);
        return nullptr;
    }

    auto address_space = var->declared_address_space;
    if (address_space == ast::AddressSpace::kNone) {
        // No declared address space. Infer from usage / type.
        if (!is_global) {
            address_space = ast::AddressSpace::kFunction;
        } else if (storage_ty->UnwrapRef()->is_handle()) {
            // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
            // If the store type is a texture type or a sampler type, then the
            // variable declaration must not have a address space attribute. The
            // address space will always be handle.
            address_space = ast::AddressSpace::kHandle;
        }
    }

    if (!is_global && address_space != ast::AddressSpace::kFunction &&
        validator_.IsValidationEnabled(var->attributes,
                                       ast::DisabledValidation::kIgnoreAddressSpace)) {
        AddError("function-scope 'var' declaration must use 'function' address space", var->source);
        return nullptr;
    }

    auto access = var->declared_access;
    if (access == ast::Access::kUndefined) {
        access = DefaultAccessForAddressSpace(address_space);
    }

    if (rhs && !validator_.VariableInitializer(var, address_space, storage_ty, rhs)) {
        return nullptr;
    }

    auto* var_ty = builder_->create<sem::Reference>(storage_ty, address_space, access);

    if (!ApplyAddressSpaceUsageToType(address_space, var_ty, var->source)) {
        AddNote("while instantiating 'var' " + builder_->Symbols().NameFor(var->symbol),
                var->source);
        return nullptr;
    }

    sem::Variable* sem = nullptr;
    if (is_global) {
        sem::BindingPoint binding_point;
        if (var->HasBindingPoint()) {
            uint32_t binding = 0;
            {
                ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@binding"};
                TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                auto* attr = ast::GetAttribute<ast::BindingAttribute>(var->attributes);
                auto* materialized = Materialize(Expression(attr->expr));
                if (!materialized) {
                    return nullptr;
                }
                if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
                    AddError("@binding must be an i32 or u32 value", attr->source);
                    return nullptr;
                }

                auto const_value = materialized->ConstantValue();
                auto value = const_value->As<AInt>();
                if (value < 0) {
                    AddError("@binding value must be non-negative", attr->source);
                    return nullptr;
                }
                binding = u32(value);
            }

            uint32_t group = 0;
            {
                ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@group"};
                TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                auto* attr = ast::GetAttribute<ast::GroupAttribute>(var->attributes);
                auto* materialized = Materialize(Expression(attr->expr));
                if (!materialized) {
                    return nullptr;
                }
                if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
                    AddError("@group must be an i32 or u32 value", attr->source);
                    return nullptr;
                }

                auto const_value = materialized->ConstantValue();
                auto value = const_value->As<AInt>();
                if (value < 0) {
                    AddError("@group value must be non-negative", attr->source);
                    return nullptr;
                }
                group = u32(value);
            }
            binding_point = {group, binding};
        }

        std::optional<uint32_t> location;
        if (auto* attr = ast::GetAttribute<ast::LocationAttribute>(var->attributes)) {
            auto value = LocationAttribute(attr);
            if (!value) {
                return nullptr;
            }
            location = value.Get();
        }

        sem = builder_->create<sem::GlobalVariable>(
            var, var_ty, sem::EvaluationStage::kRuntime, address_space, access,
            /* constant_value */ nullptr, binding_point, location);

    } else {
        sem = builder_->create<sem::LocalVariable>(var, var_ty, sem::EvaluationStage::kRuntime,
                                                   address_space, access, current_statement_,
                                                   /* constant_value */ nullptr);
    }

    sem->SetInitializer(rhs);
    builder_->Sem().Add(var, sem);
    return sem;
}

sem::Parameter* Resolver::Parameter(const ast::Parameter* param, uint32_t index) {
    auto add_note = [&] {
        AddNote("while instantiating parameter " + builder_->Symbols().NameFor(param->symbol),
                param->source);
    };

    for (auto* attr : param->attributes) {
        Mark(attr);
    }
    if (!validator_.NoDuplicateAttributes(param->attributes)) {
        return nullptr;
    }

    sem::Type* ty = Type(param->type);
    if (!ty) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(ast::AddressSpace::kNone, ty, param->source)) {
        add_note();
        return nullptr;
    }

    if (auto* ptr = ty->As<sem::Pointer>()) {
        // For MSL, we push module-scope variables into the entry point as pointer
        // parameters, so we also need to handle their store type.
        if (!ApplyAddressSpaceUsageToType(
                ptr->AddressSpace(), const_cast<sem::Type*>(ptr->StoreType()), param->source)) {
            add_note();
            return nullptr;
        }
    }

    sem::BindingPoint binding_point;
    if (param->HasBindingPoint()) {
        {
            ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@binding value"};
            TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

            auto* attr = ast::GetAttribute<ast::BindingAttribute>(param->attributes);
            auto* materialized = Materialize(Expression(attr->expr));
            if (!materialized) {
                return nullptr;
            }
            binding_point.binding = materialized->ConstantValue()->As<uint32_t>();
        }
        {
            ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@group value"};
            TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

            auto* attr = ast::GetAttribute<ast::GroupAttribute>(param->attributes);
            auto* materialized = Materialize(Expression(attr->expr));
            if (!materialized) {
                return nullptr;
            }
            binding_point.group = materialized->ConstantValue()->As<uint32_t>();
        }
    }

    std::optional<uint32_t> location;
    if (auto* attr = ast::GetAttribute<ast::LocationAttribute>(param->attributes)) {
        auto value = LocationAttribute(attr);
        if (!value) {
            return nullptr;
        }
        location = value.Get();
    }

    auto* sem = builder_->create<sem::Parameter>(
        param, index, ty, ast::AddressSpace::kNone, ast::Access::kUndefined,
        sem::ParameterUsage::kNone, binding_point, location);
    builder_->Sem().Add(param, sem);
    return sem;
}

utils::Result<uint32_t> Resolver::LocationAttribute(const ast::LocationAttribute* attr) {
    ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@location value"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

    auto* materialized = Materialize(Expression(attr->expr));
    if (!materialized) {
        return utils::Failure;
    }

    if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
        AddError("@location must be an i32 or u32 value", attr->source);
        return utils::Failure;
    }

    auto const_value = materialized->ConstantValue();
    auto value = const_value->As<AInt>();
    if (value < 0) {
        AddError("@location value must be non-negative", attr->source);
        return utils::Failure;
    }

    return static_cast<uint32_t>(value);
}

ast::Access Resolver::DefaultAccessForAddressSpace(ast::AddressSpace address_space) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class
    switch (address_space) {
        case ast::AddressSpace::kStorage:
        case ast::AddressSpace::kUniform:
        case ast::AddressSpace::kHandle:
            return ast::Access::kRead;
        default:
            break;
    }
    return ast::Access::kReadWrite;
}

bool Resolver::AllocateOverridableConstantIds() {
    constexpr size_t kLimit = std::numeric_limits<decltype(OverrideId::value)>::max();
    // The next pipeline constant ID to try to allocate.
    OverrideId next_id;
    bool ids_exhausted = false;

    auto increment_next_id = [&] {
        if (next_id.value == kLimit) {
            ids_exhausted = true;
        } else {
            next_id.value = next_id.value + 1;
        }
    };

    // Allocate constant IDs in global declaration order, so that they are
    // deterministic.
    // TODO(crbug.com/tint/1192): If a transform changes the order or removes an
    // unused constant, the allocation may change on the next Resolver pass.
    for (auto* decl : builder_->AST().GlobalDeclarations()) {
        auto* override = decl->As<ast::Override>();
        if (!override) {
            continue;
        }

        OverrideId id;
        if (ast::HasAttribute<ast::IdAttribute>(override->attributes)) {
            id = builder_->Sem().Get<sem::GlobalVariable>(override)->OverrideId();
        } else {
            // No ID was specified, so allocate the next available ID.
            while (!ids_exhausted && override_ids_.Contains(next_id)) {
                increment_next_id();
            }
            if (ids_exhausted) {
                AddError(
                    "number of 'override' variables exceeded limit of " + std::to_string(kLimit),
                    decl->source);
                return false;
            }
            id = next_id;
            increment_next_id();
        }

        auto* sem = sem_.Get<sem::GlobalVariable>(override);
        const_cast<sem::GlobalVariable*>(sem)->SetOverrideId(id);
    }
    return true;
}

void Resolver::SetShadows() {
    for (auto it : dependencies_.shadows) {
        Switch(
            sem_.Get(it.key),  //
            [&](sem::LocalVariable* local) { local->SetShadows(sem_.Get(it.value)); },
            [&](sem::Parameter* param) { param->SetShadows(sem_.Get(it.value)); });
    }
}

sem::GlobalVariable* Resolver::GlobalVariable(const ast::Variable* v) {
    auto* sem = As<sem::GlobalVariable>(Variable(v, /* is_global */ true));
    if (!sem) {
        return nullptr;
    }

    for (auto* attr : v->attributes) {
        Mark(attr);
    }

    if (!validator_.NoDuplicateAttributes(v->attributes)) {
        return nullptr;
    }

    if (!validator_.GlobalVariable(sem, override_ids_, atomic_composite_info_)) {
        return nullptr;
    }

    // TODO(bclayton): Call this at the end of resolve on all uniform and storage
    // referenced structs
    if (!validator_.AddressSpaceLayout(sem, enabled_extensions_, valid_type_storage_layouts_)) {
        return nullptr;
    }

    return sem;
}

sem::Statement* Resolver::StaticAssert(const ast::StaticAssert* assertion) {
    ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "static assertion"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
    auto* expr = Expression(assertion->condition);
    if (!expr) {
        return nullptr;
    }
    auto* cond = expr->ConstantValue();
    if (auto* ty = cond->Type(); !ty->Is<sem::Bool>()) {
        AddError(
            "static assertion condition must be a bool, got '" + builder_->FriendlyName(ty) + "'",
            assertion->condition->source);
        return nullptr;
    }
    if (!cond->As<bool>()) {
        AddError("static assertion failed", assertion->source);
        return nullptr;
    }
    auto* sem =
        builder_->create<sem::Statement>(assertion, current_compound_statement_, current_function_);
    builder_->Sem().Add(assertion, sem);
    return sem;
}

sem::Function* Resolver::Function(const ast::Function* decl) {
    uint32_t parameter_index = 0;
    utils::Hashmap<Symbol, Source, 8> parameter_names;
    utils::Vector<sem::Parameter*, 8> parameters;

    // Resolve all the parameters
    for (auto* param : decl->params) {
        Mark(param);

        {  // Check the parameter name is unique for the function
            if (auto added = parameter_names.Add(param->symbol, param->source); !added) {
                auto name = builder_->Symbols().NameFor(param->symbol);
                AddError("redefinition of parameter '" + name + "'", param->source);
                AddNote("previous definition is here", *added.value);
                return nullptr;
            }
        }

        auto* p = Parameter(param, parameter_index++);
        if (!p) {
            return nullptr;
        }

        if (!validator_.Parameter(decl, p)) {
            return nullptr;
        }

        parameters.Push(p);

        auto* p_ty = const_cast<sem::Type*>(p->Type());
        if (auto* str = p_ty->As<sem::Struct>()) {
            switch (decl->PipelineStage()) {
                case ast::PipelineStage::kVertex:
                    str->AddUsage(sem::PipelineStageUsage::kVertexInput);
                    break;
                case ast::PipelineStage::kFragment:
                    str->AddUsage(sem::PipelineStageUsage::kFragmentInput);
                    break;
                case ast::PipelineStage::kCompute:
                    str->AddUsage(sem::PipelineStageUsage::kComputeInput);
                    break;
                case ast::PipelineStage::kNone:
                    break;
            }
        }
    }

    // Resolve the return type
    sem::Type* return_type = nullptr;
    if (auto* ty = decl->return_type) {
        return_type = Type(ty);
        if (!return_type) {
            return nullptr;
        }
    } else {
        return_type = builder_->create<sem::Void>();
    }

    // Determine if the return type has a location
    std::optional<uint32_t> return_location;
    for (auto* attr : decl->return_type_attributes) {
        Mark(attr);

        if (auto* loc_attr = attr->As<ast::LocationAttribute>()) {
            auto value = LocationAttribute(loc_attr);
            if (!value) {
                return nullptr;
            }
            return_location = value.Get();
        }
    }
    if (!validator_.NoDuplicateAttributes(decl->attributes)) {
        return nullptr;
    }

    if (auto* str = return_type->As<sem::Struct>()) {
        if (!ApplyAddressSpaceUsageToType(ast::AddressSpace::kNone, str, decl->source)) {
            AddNote(
                "while instantiating return type for " + builder_->Symbols().NameFor(decl->symbol),
                decl->source);
            return nullptr;
        }

        switch (decl->PipelineStage()) {
            case ast::PipelineStage::kVertex:
                str->AddUsage(sem::PipelineStageUsage::kVertexOutput);
                break;
            case ast::PipelineStage::kFragment:
                str->AddUsage(sem::PipelineStageUsage::kFragmentOutput);
                break;
            case ast::PipelineStage::kCompute:
                str->AddUsage(sem::PipelineStageUsage::kComputeOutput);
                break;
            case ast::PipelineStage::kNone:
                break;
        }
    }

    auto* func =
        builder_->create<sem::Function>(decl, return_type, return_location, std::move(parameters));
    builder_->Sem().Add(decl, func);

    TINT_SCOPED_ASSIGNMENT(current_function_, func);

    if (!WorkgroupSize(decl)) {
        return nullptr;
    }

    if (decl->IsEntryPoint()) {
        entry_points_.Push(func);
    }

    if (decl->body) {
        Mark(decl->body);
        if (current_compound_statement_) {
            TINT_ICE(Resolver, diagnostics_)
                << "Resolver::Function() called with a current compound statement";
            return nullptr;
        }
        auto* body = StatementScope(decl->body, builder_->create<sem::FunctionBlockStatement>(func),
                                    [&] { return Statements(decl->body->statements); });
        if (!body) {
            return nullptr;
        }
        func->Behaviors() = body->Behaviors();
        if (func->Behaviors().Contains(sem::Behavior::kReturn)) {
            // https://www.w3.org/TR/WGSL/#behaviors-rules
            // We assign a behavior to each function: it is its body’s behavior
            // (treating the body as a regular statement), with any "Return" replaced
            // by "Next".
            func->Behaviors().Remove(sem::Behavior::kReturn);
            func->Behaviors().Add(sem::Behavior::kNext);
        }
    }

    for (auto* attr : decl->attributes) {
        Mark(attr);
    }

    if (!validator_.NoDuplicateAttributes(decl->return_type_attributes)) {
        return nullptr;
    }

    auto stage = current_function_ ? current_function_->Declaration()->PipelineStage()
                                   : ast::PipelineStage::kNone;
    if (!validator_.Function(func, stage)) {
        return nullptr;
    }

    // If this is an entry point, mark all transitively called functions as being
    // used by this entry point.
    if (decl->IsEntryPoint()) {
        for (auto* f : func->TransitivelyCalledFunctions()) {
            const_cast<sem::Function*>(f)->AddAncestorEntryPoint(func);
        }
    }

    return func;
}

bool Resolver::WorkgroupSize(const ast::Function* func) {
    // Set work-group size defaults.
    sem::WorkgroupSize ws;
    for (size_t i = 0; i < 3; i++) {
        ws[i] = 1;
    }

    auto* attr = ast::GetAttribute<ast::WorkgroupAttribute>(func->attributes);
    if (!attr) {
        return true;
    }

    auto values = attr->Values();
    utils::Vector<const sem::Expression*, 3> args;
    utils::Vector<const sem::Type*, 3> arg_tys;

    constexpr const char* kErrBadExpr =
        "workgroup_size argument must be a constant or override-expression of type "
        "abstract-integer, i32 or u32";

    for (size_t i = 0; i < 3; i++) {
        // Each argument to this attribute can either be a literal, an identifier for a module-scope
        // constants, a const-expression, or nullptr if not specified.
        auto* value = values[i];
        if (!value) {
            break;
        }
        const auto* expr = Expression(value);
        if (!expr) {
            return false;
        }
        auto* ty = expr->Type();
        if (!ty->IsAnyOf<sem::I32, sem::U32, sem::AbstractInt>()) {
            AddError(kErrBadExpr, value->source);
            return false;
        }

        if (expr->Stage() != sem::EvaluationStage::kConstant &&
            expr->Stage() != sem::EvaluationStage::kOverride) {
            AddError(kErrBadExpr, value->source);
            return false;
        }

        args.Push(expr);
        arg_tys.Push(ty);
    }

    auto* common_ty = sem::Type::Common(arg_tys);
    if (!common_ty) {
        AddError("workgroup_size arguments must be of the same type, either i32 or u32",
                 attr->source);
        return false;
    }

    // If all arguments are abstract-integers, then materialize to i32.
    if (common_ty->Is<sem::AbstractInt>()) {
        common_ty = builder_->create<sem::I32>();
    }

    for (size_t i = 0; i < args.Length(); i++) {
        auto* materialized = Materialize(args[i], common_ty);
        if (!materialized) {
            return false;
        }
        if (auto* value = materialized->ConstantValue()) {
            if (value->As<AInt>() < 1) {
                AddError("workgroup_size argument must be at least 1", values[i]->source);
                return false;
            }
            ws[i] = value->As<uint32_t>();
        } else {
            ws[i] = std::nullopt;
        }
    }

    uint64_t total_size = static_cast<uint64_t>(ws[0].value_or(1));
    for (size_t i = 1; i < 3; i++) {
        total_size *= static_cast<uint64_t>(ws[i].value_or(1));
        if (total_size > 0xffffffff) {
            AddError("total workgroup grid size cannot exceed 0xffffffff", values[i]->source);
            return false;
        }
    }

    current_function_->SetWorkgroupSize(std::move(ws));
    return true;
}

bool Resolver::Statements(utils::VectorRef<const ast::Statement*> stmts) {
    sem::Behaviors behaviors{sem::Behavior::kNext};

    bool reachable = true;
    for (auto* stmt : stmts) {
        Mark(stmt);
        auto* sem = Statement(stmt);
        if (!sem) {
            return false;
        }
        // s1 s2:(B1∖{Next}) ∪ B2
        sem->SetIsReachable(reachable);
        if (reachable) {
            behaviors = (behaviors - sem::Behavior::kNext) + sem->Behaviors();
        }
        reachable = reachable && sem->Behaviors().Contains(sem::Behavior::kNext);
    }

    current_statement_->Behaviors() = behaviors;

    if (!validator_.Statements(stmts)) {
        return false;
    }

    return true;
}

sem::Statement* Resolver::Statement(const ast::Statement* stmt) {
    return Switch(
        stmt,
        // Compound statements. These create their own sem::CompoundStatement
        // bindings.
        [&](const ast::BlockStatement* b) { return BlockStatement(b); },
        [&](const ast::ForLoopStatement* l) { return ForLoopStatement(l); },
        [&](const ast::LoopStatement* l) { return LoopStatement(l); },
        [&](const ast::WhileStatement* w) { return WhileStatement(w); },
        [&](const ast::IfStatement* i) { return IfStatement(i); },
        [&](const ast::SwitchStatement* s) { return SwitchStatement(s); },

        // Non-Compound statements
        [&](const ast::AssignmentStatement* a) { return AssignmentStatement(a); },
        [&](const ast::BreakStatement* b) { return BreakStatement(b); },
        [&](const ast::BreakIfStatement* b) { return BreakIfStatement(b); },
        [&](const ast::CallStatement* c) { return CallStatement(c); },
        [&](const ast::CompoundAssignmentStatement* c) { return CompoundAssignmentStatement(c); },
        [&](const ast::ContinueStatement* c) { return ContinueStatement(c); },
        [&](const ast::DiscardStatement* d) { return DiscardStatement(d); },
        [&](const ast::FallthroughStatement* f) { return FallthroughStatement(f); },
        [&](const ast::IncrementDecrementStatement* i) { return IncrementDecrementStatement(i); },
        [&](const ast::ReturnStatement* r) { return ReturnStatement(r); },
        [&](const ast::VariableDeclStatement* v) { return VariableDeclStatement(v); },
        [&](const ast::StaticAssert* sa) { return StaticAssert(sa); },

        // Error cases
        [&](const ast::CaseStatement*) {
            AddError("case statement can only be used inside a switch statement", stmt->source);
            return nullptr;
        },
        [&](Default) {
            AddError("unknown statement type: " + std::string(stmt->TypeInfo().name), stmt->source);
            return nullptr;
        });
}

sem::CaseStatement* Resolver::CaseStatement(const ast::CaseStatement* stmt, const sem::Type* ty) {
    auto* sem =
        builder_->create<sem::CaseStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Selectors().reserve(stmt->selectors.Length());
        for (auto* sel : stmt->selectors) {
            Mark(sel);

            ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "case selector"};
            TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

            const sem::Constant* const_value = nullptr;
            if (!sel->IsDefault()) {
                // The sem statement was created in the switch when attempting to determine the
                // common type.
                auto* materialized = Materialize(sem_.Get(sel->expr), ty);
                if (!materialized) {
                    return false;
                }
                if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
                    AddError("case selector must be an i32 or u32 value", sel->source);
                    return false;
                }
                const_value = materialized->ConstantValue();
                if (!const_value) {
                    AddError("case selector must be a constant expression", sel->source);
                    return false;
                }
            }

            sem->Selectors().emplace_back(builder_->create<sem::CaseSelector>(sel, const_value));
        }

        Mark(stmt->body);
        auto* body = BlockStatement(stmt->body);
        if (!body) {
            return false;
        }
        sem->SetBlock(body);
        sem->Behaviors() = body->Behaviors();
        return true;
    });
}

sem::IfStatement* Resolver::IfStatement(const ast::IfStatement* stmt) {
    auto* sem =
        builder_->create<sem::IfStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* cond = Expression(stmt->condition);
        if (!cond) {
            return false;
        }
        sem->SetCondition(cond);
        sem->Behaviors() = cond->Behaviors();
        sem->Behaviors().Remove(sem::Behavior::kNext);

        Mark(stmt->body);
        auto* body = builder_->create<sem::BlockStatement>(stmt->body, current_compound_statement_,
                                                           current_function_);
        if (!StatementScope(stmt->body, body, [&] { return Statements(stmt->body->statements); })) {
            return false;
        }
        sem->Behaviors().Add(body->Behaviors());

        if (stmt->else_statement) {
            Mark(stmt->else_statement);
            auto* else_sem = Statement(stmt->else_statement);
            if (!else_sem) {
                return false;
            }
            sem->Behaviors().Add(else_sem->Behaviors());
        } else {
            // https://www.w3.org/TR/WGSL/#behaviors-rules
            // if statements without an else branch are treated as if they had an
            // empty else branch (which adds Next to their behavior)
            sem->Behaviors().Add(sem::Behavior::kNext);
        }

        return validator_.IfStatement(sem);
    });
}

sem::BlockStatement* Resolver::BlockStatement(const ast::BlockStatement* stmt) {
    auto* sem = builder_->create<sem::BlockStatement>(
        stmt->As<ast::BlockStatement>(), current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] { return Statements(stmt->statements); });
}

sem::LoopStatement* Resolver::LoopStatement(const ast::LoopStatement* stmt) {
    auto* sem =
        builder_->create<sem::LoopStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        Mark(stmt->body);

        auto* body = builder_->create<sem::LoopBlockStatement>(
            stmt->body, current_compound_statement_, current_function_);
        return StatementScope(stmt->body, body, [&] {
            if (!Statements(stmt->body->statements)) {
                return false;
            }
            auto& behaviors = sem->Behaviors();
            behaviors = body->Behaviors();

            if (stmt->continuing) {
                Mark(stmt->continuing);
                auto* continuing = StatementScope(
                    stmt->continuing,
                    builder_->create<sem::LoopContinuingBlockStatement>(
                        stmt->continuing, current_compound_statement_, current_function_),
                    [&] { return Statements(stmt->continuing->statements); });
                if (!continuing) {
                    return false;
                }
                behaviors.Add(continuing->Behaviors());
            }

            if (behaviors.Contains(sem::Behavior::kBreak)) {  // Does the loop exit?
                behaviors.Add(sem::Behavior::kNext);
            } else {
                behaviors.Remove(sem::Behavior::kNext);
            }
            behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kContinue);

            return validator_.LoopStatement(sem);
        });
    });
}

sem::ForLoopStatement* Resolver::ForLoopStatement(const ast::ForLoopStatement* stmt) {
    auto* sem = builder_->create<sem::ForLoopStatement>(stmt, current_compound_statement_,
                                                        current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = sem->Behaviors();
        if (auto* initializer = stmt->initializer) {
            Mark(initializer);
            auto* init = Statement(initializer);
            if (!init) {
                return false;
            }
            behaviors.Add(init->Behaviors());
        }

        if (auto* cond_expr = stmt->condition) {
            auto* cond = Expression(cond_expr);
            if (!cond) {
                return false;
            }
            sem->SetCondition(cond);
            behaviors.Add(cond->Behaviors());
        }

        if (auto* continuing = stmt->continuing) {
            Mark(continuing);
            auto* cont = Statement(continuing);
            if (!cont) {
                return false;
            }
            behaviors.Add(cont->Behaviors());
        }

        Mark(stmt->body);

        auto* body = builder_->create<sem::LoopBlockStatement>(
            stmt->body, current_compound_statement_, current_function_);
        if (!StatementScope(stmt->body, body, [&] { return Statements(stmt->body->statements); })) {
            return false;
        }

        behaviors.Add(body->Behaviors());
        if (stmt->condition || behaviors.Contains(sem::Behavior::kBreak)) {  // Does the loop exit?
            behaviors.Add(sem::Behavior::kNext);
        } else {
            behaviors.Remove(sem::Behavior::kNext);
        }
        behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kContinue);

        return validator_.ForLoopStatement(sem);
    });
}

sem::WhileStatement* Resolver::WhileStatement(const ast::WhileStatement* stmt) {
    auto* sem =
        builder_->create<sem::WhileStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = sem->Behaviors();

        auto* cond = Expression(stmt->condition);
        if (!cond) {
            return false;
        }
        sem->SetCondition(cond);
        behaviors.Add(cond->Behaviors());

        Mark(stmt->body);

        auto* body = builder_->create<sem::LoopBlockStatement>(
            stmt->body, current_compound_statement_, current_function_);
        if (!StatementScope(stmt->body, body, [&] { return Statements(stmt->body->statements); })) {
            return false;
        }

        behaviors.Add(body->Behaviors());
        // Always consider the while as having a 'next' behaviour because it has
        // a condition. We don't check if the condition will terminate but it isn't
        // valid to have an infinite loop in a WGSL program, so a non-terminating
        // condition is already an invalid program.
        behaviors.Add(sem::Behavior::kNext);
        behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kContinue);

        return validator_.WhileStatement(sem);
    });
}

sem::Expression* Resolver::Expression(const ast::Expression* root) {
    utils::Vector<const ast::Expression*, 64> sorted;
    constexpr size_t kMaxExpressionDepth = 512U;
    bool failed = false;
    if (!ast::TraverseExpressions<ast::TraverseOrder::RightToLeft>(
            root, diagnostics_, [&](const ast::Expression* expr, size_t depth) {
                if (depth > kMaxExpressionDepth) {
                    AddError(
                        "reached max expression depth of " + std::to_string(kMaxExpressionDepth),
                        expr->source);
                    failed = true;
                    return ast::TraverseAction::Stop;
                }
                if (!Mark(expr)) {
                    failed = true;
                    return ast::TraverseAction::Stop;
                }
                sorted.Push(expr);
                return ast::TraverseAction::Descend;
            })) {
        return nullptr;
    }

    if (failed) {
        return nullptr;
    }

    for (auto* expr : utils::Reverse(sorted)) {
        auto* sem_expr = Switch(
            expr,
            [&](const ast::IndexAccessorExpression* array) -> sem::Expression* {
                return IndexAccessor(array);
            },
            [&](const ast::BinaryExpression* bin_op) -> sem::Expression* { return Binary(bin_op); },
            [&](const ast::BitcastExpression* bitcast) -> sem::Expression* {
                return Bitcast(bitcast);
            },
            [&](const ast::CallExpression* call) -> sem::Expression* { return Call(call); },
            [&](const ast::IdentifierExpression* ident) -> sem::Expression* {
                return Identifier(ident);
            },
            [&](const ast::LiteralExpression* literal) -> sem::Expression* {
                return Literal(literal);
            },
            [&](const ast::MemberAccessorExpression* member) -> sem::Expression* {
                return MemberAccessor(member);
            },
            [&](const ast::UnaryOpExpression* unary) -> sem::Expression* { return UnaryOp(unary); },
            [&](const ast::PhonyExpression*) -> sem::Expression* {
                return builder_->create<sem::Expression>(expr, builder_->create<sem::Void>(),
                                                         sem::EvaluationStage::kRuntime,
                                                         current_statement_,
                                                         /* constant_value */ nullptr,
                                                         /* has_side_effects */ false);
            },
            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << "unhandled expression type: " << expr->TypeInfo().name;
                return nullptr;
            });
        if (!sem_expr) {
            return nullptr;
        }

        if (auto* constraint = expr_eval_stage_constraint_.constraint) {
            if (!validator_.EvaluationStage(sem_expr, expr_eval_stage_constraint_.stage,
                                            constraint)) {
                return nullptr;
            }
        }

        builder_->Sem().Add(expr, sem_expr);
        if (expr == root) {
            return sem_expr;
        }
    }

    TINT_ICE(Resolver, diagnostics_) << "Expression() did not find root node";
    return nullptr;
}

const sem::Type* Resolver::ConcreteType(const sem::Type* ty,
                                        const sem::Type* target_ty,
                                        const Source& source) {
    auto i32 = [&] { return builder_->create<sem::I32>(); };
    auto f32 = [&] { return builder_->create<sem::F32>(); };
    auto i32v = [&](uint32_t width) { return builder_->create<sem::Vector>(i32(), width); };
    auto f32v = [&](uint32_t width) { return builder_->create<sem::Vector>(f32(), width); };
    auto f32m = [&](uint32_t columns, uint32_t rows) {
        return builder_->create<sem::Matrix>(f32v(rows), columns);
    };

    return Switch(
        ty,  //
        [&](const sem::AbstractInt*) { return target_ty ? target_ty : i32(); },
        [&](const sem::AbstractFloat*) { return target_ty ? target_ty : f32(); },
        [&](const sem::Vector* v) {
            return Switch(
                v->type(),  //
                [&](const sem::AbstractInt*) { return target_ty ? target_ty : i32v(v->Width()); },
                [&](const sem::AbstractFloat*) {
                    return target_ty ? target_ty : f32v(v->Width());
                });
        },
        [&](const sem::Matrix* m) {
            return Switch(m->type(),  //
                          [&](const sem::AbstractFloat*) {
                              return target_ty ? target_ty : f32m(m->columns(), m->rows());
                          });
        },
        [&](const sem::Array* a) -> const sem::Type* {
            const sem::Type* target_el_ty = nullptr;
            if (auto* target_arr_ty = As<sem::Array>(target_ty)) {
                target_el_ty = target_arr_ty->ElemType();
            }
            if (auto* el_ty = ConcreteType(a->ElemType(), target_el_ty, source)) {
                return Array(source, source, el_ty, a->Count(), /* explicit_stride */ 0);
            }
            return nullptr;
        });
}

const sem::Expression* Resolver::Materialize(const sem::Expression* expr,
                                             const sem::Type* target_type /* = nullptr */) {
    if (!expr) {
        // Allow for Materialize(Expression(blah)), where failures pass through Materialize()
        return nullptr;
    }

    auto* decl = expr->Declaration();

    auto* concrete_ty = ConcreteType(expr->Type(), target_type, decl->source);
    if (!concrete_ty) {
        return expr;  // Does not require materialization
    }

    auto* src_ty = expr->Type();
    if (!validator_.Materialize(concrete_ty, src_ty, decl->source)) {
        return nullptr;
    }

    auto expr_val = expr->ConstantValue();
    if (!expr_val) {
        TINT_ICE(Resolver, builder_->Diagnostics())
            << decl->source << "Materialize(" << decl->TypeInfo().name
            << ") called on expression with no constant value";
        return nullptr;
    }

    auto materialized_val = const_eval_.Convert(concrete_ty, expr_val, decl->source);
    if (!materialized_val) {
        // ConvertValue() has already failed and raised an diagnostic error.
        return nullptr;
    }

    if (!materialized_val.Get()) {
        TINT_ICE(Resolver, builder_->Diagnostics())
            << decl->source << "ConvertValue(" << builder_->FriendlyName(expr_val->Type()) << " -> "
            << builder_->FriendlyName(concrete_ty) << ") returned invalid value";
        return nullptr;
    }
    auto* m = builder_->create<sem::Materialize>(expr, current_statement_, materialized_val.Get());
    m->Behaviors() = expr->Behaviors();
    builder_->Sem().Replace(decl, m);
    return m;
}

template <size_t N>
bool Resolver::MaybeMaterializeArguments(utils::Vector<const sem::Expression*, N>& args,
                                         const sem::CallTarget* target) {
    for (size_t i = 0, n = std::min(args.Length(), target->Parameters().Length()); i < n; i++) {
        const auto* param_ty = target->Parameters()[i]->Type();
        if (ShouldMaterializeArgument(param_ty)) {
            auto* materialized = Materialize(args[i], param_ty);
            if (!materialized) {
                return false;
            }
            args[i] = materialized;
        }
    }
    return true;
}

bool Resolver::ShouldMaterializeArgument(const sem::Type* parameter_ty) const {
    const auto* param_el_ty = sem::Type::DeepestElementOf(parameter_ty);
    return param_el_ty && !param_el_ty->Is<sem::AbstractNumeric>();
}

bool Resolver::Convert(const sem::Constant*& c, const sem::Type* target_ty, const Source& source) {
    auto r = const_eval_.Convert(target_ty, c, source);
    if (!r) {
        return false;
    }
    c = r.Get();
    return true;
}

template <size_t N>
utils::Result<utils::Vector<const sem::Constant*, N>> Resolver::ConvertArguments(
    const utils::Vector<const sem::Expression*, N>& args,
    const sem::CallTarget* target) {
    auto const_args = utils::Transform(args, [](auto* arg) { return arg->ConstantValue(); });
    for (size_t i = 0, n = std::min(args.Length(), target->Parameters().Length()); i < n; i++) {
        if (!Convert(const_args[i], target->Parameters()[i]->Type(),
                     args[i]->Declaration()->source)) {
            return utils::Failure;
        }
    }
    return const_args;
}

sem::Expression* Resolver::IndexAccessor(const ast::IndexAccessorExpression* expr) {
    auto* idx = Materialize(sem_.Get(expr->index));
    if (!idx) {
        return nullptr;
    }
    const auto* obj = sem_.Get(expr->object);
    if (idx->Stage() != sem::EvaluationStage::kConstant) {
        // If the index is non-constant, then the resulting expression is non-constant, so we'll
        // have to materialize the object. For example, consider:
        //     vec2(1, 2)[runtime-index]
        obj = Materialize(obj);
    }
    if (!obj) {
        return nullptr;
    }
    auto* obj_raw_ty = obj->Type();
    auto* obj_ty = obj_raw_ty->UnwrapRef();
    auto* ty = Switch(
        obj_ty,  //
        [&](const sem::Array* arr) { return arr->ElemType(); },
        [&](const sem::Vector* vec) { return vec->type(); },
        [&](const sem::Matrix* mat) {
            return builder_->create<sem::Vector>(mat->type(), mat->rows());
        },
        [&](Default) {
            AddError("cannot index type '" + sem_.TypeNameOf(obj_ty) + "'", expr->source);
            return nullptr;
        });
    if (ty == nullptr) {
        return nullptr;
    }

    auto* idx_ty = idx->Type()->UnwrapRef();
    if (!idx_ty->IsAnyOf<sem::I32, sem::U32>()) {
        AddError("index must be of type 'i32' or 'u32', found: '" + sem_.TypeNameOf(idx_ty) + "'",
                 idx->Declaration()->source);
        return nullptr;
    }

    // If we're extracting from a reference, we return a reference.
    if (auto* ref = obj_raw_ty->As<sem::Reference>()) {
        ty = builder_->create<sem::Reference>(ty, ref->AddressSpace(), ref->Access());
    }

    auto stage = sem::EarliestStage(obj->Stage(), idx->Stage());
    const sem::Constant* val = nullptr;
    if (auto r = const_eval_.Index(obj, idx)) {
        val = r.Get();
    } else {
        return nullptr;
    }
    bool has_side_effects = idx->HasSideEffects() || obj->HasSideEffects();
    auto* sem = builder_->create<sem::IndexAccessorExpression>(
        expr, ty, stage, obj, idx, current_statement_, std::move(val), has_side_effects,
        obj->RootIdentifier());
    sem->Behaviors() = idx->Behaviors() + obj->Behaviors();
    return sem;
}

sem::Expression* Resolver::Bitcast(const ast::BitcastExpression* expr) {
    auto* inner = Materialize(sem_.Get(expr->expr));
    if (!inner) {
        return nullptr;
    }
    auto* ty = Type(expr->type);
    if (!ty) {
        return nullptr;
    }

    const sem::Constant* val = nullptr;
    if (auto r = const_eval_.Bitcast(ty, inner)) {
        val = r.Get();
    } else {
        return nullptr;
    }
    auto stage = sem::EvaluationStage::kRuntime;  // TODO(crbug.com/tint/1581)
    auto* sem = builder_->create<sem::Expression>(expr, ty, stage, current_statement_,
                                                  std::move(val), inner->HasSideEffects());

    sem->Behaviors() = inner->Behaviors();

    if (!validator_.Bitcast(expr, ty)) {
        return nullptr;
    }

    return sem;
}

sem::Call* Resolver::Call(const ast::CallExpression* expr) {
    // A CallExpression can resolve to one of:
    // * A function call.
    // * A builtin call.
    // * A type initializer.
    // * A type conversion.

    // Resolve all of the arguments, their types and the set of behaviors.
    utils::Vector<const sem::Expression*, 8> args;
    args.Reserve(expr->args.Length());
    auto args_stage = sem::EvaluationStage::kConstant;
    sem::Behaviors arg_behaviors;
    for (size_t i = 0; i < expr->args.Length(); i++) {
        auto* arg = sem_.Get(expr->args[i]);
        if (!arg) {
            return nullptr;
        }
        args.Push(arg);
        args_stage = sem::EarliestStage(args_stage, arg->Stage());
        arg_behaviors.Add(arg->Behaviors());
    }
    arg_behaviors.Remove(sem::Behavior::kNext);

    // Did any arguments have side effects?
    bool has_side_effects =
        std::any_of(args.begin(), args.end(), [](auto* e) { return e->HasSideEffects(); });

    // ct_init_or_conv is a helper for building either a sem::TypeInitializer or sem::TypeConversion
    // call for a InitConvIntrinsic with an optional template argument type.
    auto ct_init_or_conv = [&](InitConvIntrinsic ty, const sem::Type* template_arg) -> sem::Call* {
        auto arg_tys = utils::Transform(args, [](auto* arg) { return arg->Type(); });
        auto ctor_or_conv =
            intrinsic_table_->Lookup(ty, template_arg, arg_tys, args_stage, expr->source);
        if (!ctor_or_conv.target) {
            return nullptr;
        }
        if (!MaybeMaterializeArguments(args, ctor_or_conv.target)) {
            return nullptr;
        }
        const sem::Constant* value = nullptr;
        auto stage = sem::EarliestStage(ctor_or_conv.target->Stage(), args_stage);
        if (stage == sem::EvaluationStage::kConstant) {
            auto const_args = ConvertArguments(args, ctor_or_conv.target);
            if (!const_args) {
                return nullptr;
            }
            if (auto r = (const_eval_.*ctor_or_conv.const_eval_fn)(
                    ctor_or_conv.target->ReturnType(), const_args.Get(), expr->source)) {
                value = r.Get();
            } else {
                return nullptr;
            }
        }
        return builder_->create<sem::Call>(expr, ctor_or_conv.target, stage, std::move(args),
                                           current_statement_, value, has_side_effects);
    };

    // arr_or_str_init is a helper for building a sem::TypeInitializer for an array or structure
    // initializer call target.
    auto arr_or_str_init = [&](const sem::Type* ty,
                               const sem::CallTarget* call_target) -> sem::Call* {
        if (!MaybeMaterializeArguments(args, call_target)) {
            return nullptr;
        }

        auto stage = args_stage;               // The evaluation stage of the call
        const sem::Constant* value = nullptr;  // The constant value for the call
        if (stage == sem::EvaluationStage::kConstant) {
            if (auto r = const_eval_.ArrayOrStructInit(ty, args)) {
                value = r.Get();
            } else {
                return nullptr;
            }
            if (!value) {
                // Constant evaluation failed.
                // Can happen for expressions that will fail validation (later).
                // Use the kRuntime EvaluationStage, as kConstant will trigger an assertion in the
                // sem::Expression initializer, which checks that kConstant is paired with a
                // constant value.
                stage = sem::EvaluationStage::kRuntime;
            }
        }

        return builder_->create<sem::Call>(expr, call_target, stage, std::move(args),
                                           current_statement_, value, has_side_effects);
    };

    // ty_init_or_conv is a helper for building either a sem::TypeInitializer or sem::TypeConversion
    // call for the given semantic type.
    auto ty_init_or_conv = [&](const sem::Type* ty) {
        return Switch(
            ty,  //
            [&](const sem::Vector* v) {
                return ct_init_or_conv(VectorInitConvIntrinsic(v->Width()), v->type());
            },
            [&](const sem::Matrix* m) {
                return ct_init_or_conv(MatrixInitConvIntrinsic(m->columns(), m->rows()), m->type());
            },
            [&](const sem::I32*) { return ct_init_or_conv(InitConvIntrinsic::kI32, nullptr); },
            [&](const sem::U32*) { return ct_init_or_conv(InitConvIntrinsic::kU32, nullptr); },
            [&](const sem::F16*) { return ct_init_or_conv(InitConvIntrinsic::kF16, nullptr); },
            [&](const sem::F32*) { return ct_init_or_conv(InitConvIntrinsic::kF32, nullptr); },
            [&](const sem::Bool*) { return ct_init_or_conv(InitConvIntrinsic::kBool, nullptr); },
            [&](const sem::Array* arr) -> sem::Call* {
                auto* call_target = array_inits_.GetOrCreate(
                    ArrayInitializerSig{{arr, args.Length(), args_stage}},
                    [&]() -> sem::TypeInitializer* {
                        auto params = utils::Transform(args, [&](auto, size_t i) {
                            return builder_->create<sem::Parameter>(
                                nullptr,                   // declaration
                                static_cast<uint32_t>(i),  // index
                                arr->ElemType(),           // type
                                ast::AddressSpace::kNone,  // address_space
                                ast::Access::kUndefined);
                        });
                        return builder_->create<sem::TypeInitializer>(arr, std::move(params),
                                                                      args_stage);
                    });

                auto* call = arr_or_str_init(arr, call_target);
                if (!call) {
                    return nullptr;
                }

                // Validation must occur after argument materialization in arr_or_str_init().
                if (!validator_.ArrayInitializer(expr, arr)) {
                    return nullptr;
                }
                return call;
            },
            [&](const sem::Struct* str) -> sem::Call* {
                auto* call_target = struct_inits_.GetOrCreate(
                    StructInitializerSig{{str, args.Length(), args_stage}},
                    [&]() -> sem::TypeInitializer* {
                        utils::Vector<const sem::Parameter*, 8> params;
                        params.Resize(std::min(args.Length(), str->Members().size()));
                        for (size_t i = 0, n = params.Length(); i < n; i++) {
                            params[i] = builder_->create<sem::Parameter>(
                                nullptr,                    // declaration
                                static_cast<uint32_t>(i),   // index
                                str->Members()[i]->Type(),  // type
                                ast::AddressSpace::kNone,   // address_space
                                ast::Access::kUndefined);   // access
                        }
                        return builder_->create<sem::TypeInitializer>(str, std::move(params),
                                                                      args_stage);
                    });

                auto* call = arr_or_str_init(str, call_target);
                if (!call) {
                    return nullptr;
                }

                // Validation must occur after argument materialization in arr_or_str_init().
                if (!validator_.StructureInitializer(expr, str)) {
                    return nullptr;
                }
                return call;
            },
            [&](Default) {
                AddError("type is not constructible", expr->source);
                return nullptr;
            });
    };

    // ast::CallExpression has a target which is either an ast::Type or an ast::IdentifierExpression
    sem::Call* call = nullptr;
    if (expr->target.type) {
        // ast::CallExpression has an ast::Type as the target.
        // This call is either a type initializer or type conversion.
        call = Switch(
            expr->target.type,
            [&](const ast::Vector* v) -> sem::Call* {
                Mark(v);
                // vector element type must be inferred if it was not specified.
                sem::Type* template_arg = nullptr;
                if (v->type) {
                    template_arg = Type(v->type);
                    if (!template_arg) {
                        return nullptr;
                    }
                }
                if (auto* c = ct_init_or_conv(VectorInitConvIntrinsic(v->width), template_arg)) {
                    builder_->Sem().Add(expr->target.type, c->Target()->ReturnType());
                    return c;
                }
                return nullptr;
            },
            [&](const ast::Matrix* m) -> sem::Call* {
                Mark(m);
                // matrix element type must be inferred if it was not specified.
                sem::Type* template_arg = nullptr;
                if (m->type) {
                    template_arg = Type(m->type);
                    if (!template_arg) {
                        return nullptr;
                    }
                }
                if (auto* c = ct_init_or_conv(MatrixInitConvIntrinsic(m->columns, m->rows),
                                              template_arg)) {
                    builder_->Sem().Add(expr->target.type, c->Target()->ReturnType());
                    return c;
                }
                return nullptr;
            },
            [&](const ast::Array* a) -> sem::Call* {
                Mark(a);
                // array element type must be inferred if it was not specified.
                sem::ArrayCount el_count =
                    sem::ConstantArrayCount{static_cast<uint32_t>(args.Length())};
                const sem::Type* el_ty = nullptr;
                if (a->type) {
                    el_ty = Type(a->type);
                    if (!el_ty) {
                        return nullptr;
                    }
                    if (!a->count) {
                        AddError("cannot construct a runtime-sized array", expr->source);
                        return nullptr;
                    }
                    if (auto count = ArrayCount(a->count)) {
                        el_count = count.Get();
                    } else {
                        return nullptr;
                    }
                    // Note: validation later will detect any mismatches between explicit array
                    // size and number of initializer expressions.
                } else {
                    auto arg_tys =
                        utils::Transform(args, [](auto* arg) { return arg->Type()->UnwrapRef(); });
                    el_ty = sem::Type::Common(arg_tys);
                    if (!el_ty) {
                        AddError(
                            "cannot infer common array element type from initializer arguments",
                            expr->source);
                        utils::Hashset<const sem::Type*, 8> types;
                        for (size_t i = 0; i < args.Length(); i++) {
                            if (types.Add(args[i]->Type())) {
                                AddNote("argument " + std::to_string(i) + " is of type '" +
                                            sem_.TypeNameOf(args[i]->Type()) + "'",
                                        args[i]->Declaration()->source);
                            }
                        }
                        return nullptr;
                    }
                }
                uint32_t explicit_stride = 0;
                if (!ArrayAttributes(a->attributes, el_ty, explicit_stride)) {
                    return nullptr;
                }

                auto* arr = Array(a->type ? a->type->source : a->source,
                                  a->count ? a->count->source : a->source,  //
                                  el_ty, el_count, explicit_stride);
                if (!arr) {
                    return nullptr;
                }
                builder_->Sem().Add(a, arr);

                return ty_init_or_conv(arr);
            },
            [&](const ast::Type* ast) -> sem::Call* {
                // Handler for AST types that do not have an optional element type.
                if (auto* ty = Type(ast)) {
                    return ty_init_or_conv(ty);
                }
                return nullptr;
            },
            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << expr->source << " unhandled CallExpression target:\n"
                    << "type: "
                    << (expr->target.type ? expr->target.type->TypeInfo().name : "<null>");
                return nullptr;
            });
    } else {
        // ast::CallExpression has an ast::IdentifierExpression as the target.
        // This call is either a function call, builtin call, type initializer or type conversion.
        auto* ident = expr->target.name;
        Mark(ident);
        auto* resolved = sem_.ResolvedSymbol(ident);
        call = Switch<sem::Call*>(
            resolved,  //
            [&](sem::Type* ty) {
                // A type initializer or conversions.
                // Note: Unlike the code path where we're resolving the call target from an
                // ast::Type, all types must already have the element type explicitly specified, so
                // there's no need to infer element types.
                return ty_init_or_conv(ty);
            },
            [&](sem::Function* func) { return FunctionCall(expr, func, args, arg_behaviors); },
            [&](sem::Variable* var) {
                auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                AddError("cannot call variable '" + name + "'", ident->source);
                AddNote("'" + name + "' declared here", var->Declaration()->source);
                return nullptr;
            },
            [&](Default) -> sem::Call* {
                auto name = builder_->Symbols().NameFor(ident->symbol);
                auto builtin_type = sem::ParseBuiltinType(name);
                if (builtin_type != sem::BuiltinType::kNone) {
                    return BuiltinCall(expr, builtin_type, args);
                }

                TINT_ICE(Resolver, diagnostics_)
                    << expr->source << " unhandled CallExpression target:\n"
                    << "resolved: " << (resolved ? resolved->TypeInfo().name : "<null>") << "\n"
                    << "name: " << builder_->Symbols().NameFor(ident->symbol);
                return nullptr;
            });
    }

    if (!call) {
        return nullptr;
    }

    return validator_.Call(call, current_statement_) ? call : nullptr;
}

template <size_t N>
sem::Call* Resolver::BuiltinCall(const ast::CallExpression* expr,
                                 sem::BuiltinType builtin_type,
                                 utils::Vector<const sem::Expression*, N>& args) {
    auto arg_stage = sem::EvaluationStage::kConstant;
    for (auto* arg : args) {
        arg_stage = sem::EarliestStage(arg_stage, arg->Stage());
    }

    IntrinsicTable::Builtin builtin;
    {
        auto arg_tys = utils::Transform(args, [](auto* arg) { return arg->Type(); });
        builtin = intrinsic_table_->Lookup(builtin_type, arg_tys, arg_stage, expr->source);
        if (!builtin.sem) {
            return nullptr;
        }
    }

    if (builtin_type == sem::BuiltinType::kTintMaterialize) {
        args[0] = Materialize(args[0]);
        if (!args[0]) {
            return nullptr;
        }
    } else {
        // Materialize arguments if the parameter type is not abstract
        if (!MaybeMaterializeArguments(args, builtin.sem)) {
            return nullptr;
        }
    }

    if (builtin.sem->IsDeprecated()) {
        AddWarning("use of deprecated builtin", expr->source);
    }

    // If the builtin is @const, and all arguments have constant values, evaluate the builtin now.
    auto stage = sem::EarliestStage(arg_stage, builtin.sem->Stage());
    const sem::Constant* value = nullptr;
    if (stage == sem::EvaluationStage::kConstant) {
        auto const_args = ConvertArguments(args, builtin.sem);
        if (!const_args) {
            return nullptr;
        }
        if (auto r = (const_eval_.*builtin.const_eval_fn)(builtin.sem->ReturnType(),
                                                          const_args.Get(), expr->source)) {
            value = r.Get();
        } else {
            return nullptr;
        }
    }

    bool has_side_effects =
        builtin.sem->HasSideEffects() ||
        std::any_of(args.begin(), args.end(), [](auto* e) { return e->HasSideEffects(); });
    auto* call = builder_->create<sem::Call>(expr, builtin.sem, stage, std::move(args),
                                             current_statement_, value, has_side_effects);

    if (current_function_) {
        current_function_->AddDirectlyCalledBuiltin(builtin.sem);
        current_function_->AddDirectCall(call);
    }

    if (!validator_.RequiredExtensionForBuiltinFunction(call, enabled_extensions_)) {
        return nullptr;
    }

    if (IsTextureBuiltin(builtin_type)) {
        if (!validator_.TextureBuiltinFunction(call)) {
            return nullptr;
        }
        CollectTextureSamplerPairs(builtin.sem, call->Arguments());
    }

    if (!validator_.BuiltinCall(call)) {
        return nullptr;
    }

    return call;
}

void Resolver::CollectTextureSamplerPairs(const sem::Builtin* builtin,
                                          utils::VectorRef<const sem::Expression*> args) const {
    // Collect a texture/sampler pair for this builtin.
    const auto& signature = builtin->Signature();
    int texture_index = signature.IndexOf(sem::ParameterUsage::kTexture);
    if (texture_index == -1) {
        TINT_ICE(Resolver, diagnostics_) << "texture builtin without texture parameter";
    }
    if (auto* user = args[static_cast<size_t>(texture_index)]->As<sem::VariableUser>()) {
        auto* texture = user->Variable();
        if (!texture->Type()->UnwrapRef()->Is<sem::StorageTexture>()) {
            int sampler_index = signature.IndexOf(sem::ParameterUsage::kSampler);
            const sem::Variable* sampler =
                sampler_index != -1
                    ? args[static_cast<size_t>(sampler_index)]->As<sem::VariableUser>()->Variable()
                    : nullptr;
            current_function_->AddTextureSamplerPair(texture, sampler);
        }
    }
}

template <size_t N>
sem::Call* Resolver::FunctionCall(const ast::CallExpression* expr,
                                  sem::Function* target,
                                  utils::Vector<const sem::Expression*, N>& args,
                                  sem::Behaviors arg_behaviors) {
    auto sym = expr->target.name->symbol;
    auto name = builder_->Symbols().NameFor(sym);

    if (!MaybeMaterializeArguments(args, target)) {
        return nullptr;
    }

    // TODO(crbug.com/tint/1420): For now, assume all function calls have side
    // effects.
    bool has_side_effects = true;
    auto* call = builder_->create<sem::Call>(expr, target, sem::EvaluationStage::kRuntime,
                                             std::move(args), current_statement_,
                                             /* constant_value */ nullptr, has_side_effects);

    target->AddCallSite(call);

    call->Behaviors() = arg_behaviors + target->Behaviors();

    if (!validator_.FunctionCall(call, current_statement_)) {
        return nullptr;
    }

    if (current_function_) {
        // Note: Requires called functions to be resolved first.
        // This is currently guaranteed as functions must be declared before
        // use.
        current_function_->AddTransitivelyCalledFunction(target);
        current_function_->AddDirectCall(call);
        for (auto* transitive_call : target->TransitivelyCalledFunctions()) {
            current_function_->AddTransitivelyCalledFunction(transitive_call);
        }

        // We inherit any referenced variables from the callee.
        for (auto* var : target->TransitivelyReferencedGlobals()) {
            current_function_->AddTransitivelyReferencedGlobal(var);
        }

        // Note: Validation *must* be performed before calling this method.
        CollectTextureSamplerPairs(target, call->Arguments());
    }

    return call;
}

void Resolver::CollectTextureSamplerPairs(sem::Function* func,
                                          utils::VectorRef<const sem::Expression*> args) const {
    // Map all texture/sampler pairs from the target function to the
    // current function. These can only be global or parameter
    // variables. Resolve any parameter variables to the corresponding
    // argument passed to the current function. Leave global variables
    // as-is. Then add the mapped pair to the current function's list of
    // texture/sampler pairs.
    for (sem::VariablePair pair : func->TextureSamplerPairs()) {
        const sem::Variable* texture = pair.first;
        const sem::Variable* sampler = pair.second;
        if (auto* param = texture->As<sem::Parameter>()) {
            texture = args[param->Index()]->As<sem::VariableUser>()->Variable();
        }
        if (sampler) {
            if (auto* param = sampler->As<sem::Parameter>()) {
                sampler = args[param->Index()]->As<sem::VariableUser>()->Variable();
            }
        }
        current_function_->AddTextureSamplerPair(texture, sampler);
    }
}

sem::Expression* Resolver::Literal(const ast::LiteralExpression* literal) {
    auto* ty = Switch(
        literal,
        [&](const ast::IntLiteralExpression* i) -> sem::Type* {
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                    return builder_->create<sem::AbstractInt>();
                case ast::IntLiteralExpression::Suffix::kI:
                    return builder_->create<sem::I32>();
                case ast::IntLiteralExpression::Suffix::kU:
                    return builder_->create<sem::U32>();
            }
            return nullptr;
        },
        [&](const ast::FloatLiteralExpression* f) -> sem::Type* {
            switch (f->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                    return builder_->create<sem::AbstractFloat>();
                case ast::FloatLiteralExpression::Suffix::kF:
                    return builder_->create<sem::F32>();
                case ast::FloatLiteralExpression::Suffix::kH:
                    return builder_->create<sem::F16>();
            }
            return nullptr;
        },
        [&](const ast::BoolLiteralExpression*) { return builder_->create<sem::Bool>(); },
        [&](Default) { return nullptr; });

    if (ty == nullptr) {
        TINT_UNREACHABLE(Resolver, builder_->Diagnostics())
            << "Unhandled literal type: " << literal->TypeInfo().name;
        return nullptr;
    }

    if ((ty->Is<sem::F16>()) && (!enabled_extensions_.Contains(tint::ast::Extension::kF16))) {
        AddError("f16 literal used without 'f16' extension enabled", literal->source);
        return nullptr;
    }

    const sem::Constant* val = nullptr;
    if (auto r = const_eval_.Literal(ty, literal)) {
        val = r.Get();
    } else {
        return nullptr;
    }
    return builder_->create<sem::Expression>(literal, ty, sem::EvaluationStage::kConstant,
                                             current_statement_, std::move(val),
                                             /* has_side_effects */ false);
}

sem::Expression* Resolver::Identifier(const ast::IdentifierExpression* expr) {
    auto symbol = expr->symbol;
    auto* resolved = sem_.ResolvedSymbol(expr);
    if (auto* variable = As<sem::Variable>(resolved)) {
        auto* user = builder_->create<sem::VariableUser>(expr, current_statement_, variable);

        if (current_statement_) {
            // If identifier is part of a loop continuing block, make sure it
            // doesn't refer to a variable that is bypassed by a continue statement
            // in the loop's body block.
            if (auto* continuing_block =
                    current_statement_->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
                auto* loop_block = continuing_block->FindFirstParent<sem::LoopBlockStatement>();
                if (loop_block->FirstContinue()) {
                    // If our identifier is in loop_block->decls, make sure its index is
                    // less than first_continue
                    if (auto* decl = loop_block->Decls().Find(symbol)) {
                        if (decl->order >= loop_block->NumDeclsAtFirstContinue()) {
                            AddError("continue statement bypasses declaration of '" +
                                         builder_->Symbols().NameFor(symbol) + "'",
                                     loop_block->FirstContinue()->source);
                            AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                                        "' declared here",
                                    decl->variable->Declaration()->source);
                            AddNote("identifier '" + builder_->Symbols().NameFor(symbol) +
                                        "' referenced in continuing block here",
                                    expr->source);
                            return nullptr;
                        }
                    }
                }
            }
        }

        if (current_function_) {
            if (auto* global = variable->As<sem::GlobalVariable>()) {
                current_function_->AddDirectlyReferencedGlobal(global);
            }
        } else if (variable->Declaration()->Is<ast::Var>()) {
            // Use of a module-scope 'var' outside of a function.
            // Note: The spec is currently vague around the rules here. See
            // https://github.com/gpuweb/gpuweb/issues/3081. Remove this comment when resolved.
            std::string desc = "var '" + builder_->Symbols().NameFor(symbol) + "' ";
            AddError(desc + "cannot be referenced at module-scope", expr->source);
            AddNote(desc + "declared here", variable->Declaration()->source);
            return nullptr;
        }

        variable->AddUser(user);
        return user;
    }

    if (Is<sem::Function>(resolved)) {
        AddError("missing '(' for function call", expr->source.End());
        return nullptr;
    }

    if (IsBuiltin(symbol)) {
        AddError("missing '(' for builtin call", expr->source.End());
        return nullptr;
    }

    if (resolved->Is<sem::Type>()) {
        AddError("missing '(' for type initializer or cast", expr->source.End());
        return nullptr;
    }

    TINT_ICE(Resolver, diagnostics_)
        << expr->source << " unresolved identifier:\n"
        << "resolved: " << (resolved ? resolved->TypeInfo().name : "<null>") << "\n"
        << "name: " << builder_->Symbols().NameFor(symbol);
    return nullptr;
}

sem::Expression* Resolver::MemberAccessor(const ast::MemberAccessorExpression* expr) {
    auto* structure = sem_.TypeOf(expr->structure);
    auto* storage_ty = structure->UnwrapRef();
    auto* object = sem_.Get(expr->structure);
    auto* root_ident = object->RootIdentifier();

    const sem::Type* ty = nullptr;

    // Object may be a side-effecting expression (e.g. function call).
    bool has_side_effects = object && object->HasSideEffects();

    return Switch(
        storage_ty,  //
        [&](const sem::Struct* str) -> sem::Expression* {
            Mark(expr->member);
            auto symbol = expr->member->symbol;

            const sem::StructMember* member = nullptr;
            for (auto* m : str->Members()) {
                if (m->Name() == symbol) {
                    member = m;
                    break;
                }
            }

            // TODO(crbug.com/tint/1757): Remove
            if (utils::HasPrefix(builder_->Symbols().NameFor(str->Name()), "__frexp_result")) {
                if (builder_->Symbols().NameFor(symbol) == "sig") {
                    AddWarning(
                        "use of deprecated language feature: 'sig' has been renamed to 'fract'",
                        expr->member->source);
                    member = str->Members()[0];
                }
            }

            if (member == nullptr) {
                AddError("struct member " + builder_->Symbols().NameFor(symbol) + " not found",
                         expr->source);
                return nullptr;
            }

            ty = member->Type();

            // If we're extracting from a reference, we return a reference.
            if (auto* ref = structure->As<sem::Reference>()) {
                ty = builder_->create<sem::Reference>(ty, ref->AddressSpace(), ref->Access());
            }

            auto val = const_eval_.MemberAccess(object, member);
            if (!val) {
                return nullptr;
            }
            return builder_->create<sem::StructMemberAccess>(expr, ty, current_statement_,
                                                             val.Get(), object, member,
                                                             has_side_effects, root_ident);
        },

        [&](const sem::Vector* vec) -> sem::Expression* {
            Mark(expr->member);
            std::string s = builder_->Symbols().NameFor(expr->member->symbol);
            auto size = s.size();
            utils::Vector<uint32_t, 4> swizzle;
            swizzle.Reserve(s.size());

            for (auto c : s) {
                switch (c) {
                    case 'x':
                    case 'r':
                        swizzle.Push(0u);
                        break;
                    case 'y':
                    case 'g':
                        swizzle.Push(1u);
                        break;
                    case 'z':
                    case 'b':
                        swizzle.Push(2u);
                        break;
                    case 'w':
                    case 'a':
                        swizzle.Push(3u);
                        break;
                    default:
                        AddError("invalid vector swizzle character",
                                 expr->member->source.Begin() + swizzle.Length());
                        return nullptr;
                }

                if (swizzle.Back() >= vec->Width()) {
                    AddError("invalid vector swizzle member", expr->member->source);
                    return nullptr;
                }
            }

            if (size < 1 || size > 4) {
                AddError("invalid vector swizzle size", expr->member->source);
                return nullptr;
            }

            // All characters are valid, check if they're being mixed
            auto is_rgba = [](char c) { return c == 'r' || c == 'g' || c == 'b' || c == 'a'; };
            auto is_xyzw = [](char c) { return c == 'x' || c == 'y' || c == 'z' || c == 'w'; };
            if (!std::all_of(s.begin(), s.end(), is_rgba) &&
                !std::all_of(s.begin(), s.end(), is_xyzw)) {
                AddError("invalid mixing of vector swizzle characters rgba with xyzw",
                         expr->member->source);
                return nullptr;
            }

            if (size == 1) {
                // A single element swizzle is just the type of the vector.
                ty = vec->type();
                // If we're extracting from a reference, we return a reference.
                if (auto* ref = structure->As<sem::Reference>()) {
                    ty = builder_->create<sem::Reference>(ty, ref->AddressSpace(), ref->Access());
                }
            } else {
                // The vector will have a number of components equal to the length of
                // the swizzle.
                ty = builder_->create<sem::Vector>(vec->type(), static_cast<uint32_t>(size));
            }
            auto val = const_eval_.Swizzle(ty, object, swizzle);
            if (!val) {
                return nullptr;
            }
            return builder_->create<sem::Swizzle>(expr, ty, current_statement_, val.Get(), object,
                                                  std::move(swizzle), has_side_effects, root_ident);
        },

        [&](Default) {
            AddError("invalid member accessor expression. Expected vector or struct, got '" +
                         sem_.TypeNameOf(storage_ty) + "'",
                     expr->structure->source);
            return nullptr;
        });
}

sem::Expression* Resolver::Binary(const ast::BinaryExpression* expr) {
    const auto* lhs = sem_.Get(expr->lhs);
    const auto* rhs = sem_.Get(expr->rhs);
    auto* lhs_ty = lhs->Type()->UnwrapRef();
    auto* rhs_ty = rhs->Type()->UnwrapRef();

    auto stage = sem::EarliestStage(lhs->Stage(), rhs->Stage());
    auto op = intrinsic_table_->Lookup(expr->op, lhs_ty, rhs_ty, stage, expr->source, false);
    if (!op.result) {
        return nullptr;
    }
    if (ShouldMaterializeArgument(op.lhs)) {
        lhs = Materialize(lhs, op.lhs);
        if (!lhs) {
            return nullptr;
        }
    }
    if (ShouldMaterializeArgument(op.rhs)) {
        rhs = Materialize(rhs, op.rhs);
        if (!rhs) {
            return nullptr;
        }
    }

    const sem::Constant* value = nullptr;
    if (stage == sem::EvaluationStage::kConstant) {
        if (op.const_eval_fn) {
            auto const_args = utils::Vector{lhs->ConstantValue(), rhs->ConstantValue()};
            // Implicit conversion (e.g. AInt -> AFloat)
            if (!Convert(const_args[0], op.lhs, lhs->Declaration()->source)) {
                return nullptr;
            }
            if (!Convert(const_args[1], op.rhs, rhs->Declaration()->source)) {
                return nullptr;
            }

            if (auto r = (const_eval_.*op.const_eval_fn)(op.result, const_args, expr->source)) {
                value = r.Get();
            } else {
                return nullptr;
            }
        } else {
            stage = sem::EvaluationStage::kRuntime;
        }
    }

    bool has_side_effects = lhs->HasSideEffects() || rhs->HasSideEffects();
    auto* sem = builder_->create<sem::Expression>(expr, op.result, stage, current_statement_, value,
                                                  has_side_effects);
    sem->Behaviors() = lhs->Behaviors() + rhs->Behaviors();

    return sem;
}

sem::Expression* Resolver::UnaryOp(const ast::UnaryOpExpression* unary) {
    const auto* expr = sem_.Get(unary->expr);
    auto* expr_ty = expr->Type();
    if (!expr_ty) {
        return nullptr;
    }

    const sem::Type* ty = nullptr;
    const sem::Variable* root_ident = nullptr;
    const sem::Constant* value = nullptr;
    auto stage = sem::EvaluationStage::kRuntime;

    switch (unary->op) {
        case ast::UnaryOp::kAddressOf:
            if (auto* ref = expr_ty->As<sem::Reference>()) {
                if (ref->StoreType()->UnwrapRef()->is_handle()) {
                    AddError("cannot take the address of expression in handle address space",
                             unary->expr->source);
                    return nullptr;
                }

                auto* array = unary->expr->As<ast::IndexAccessorExpression>();
                auto* member = unary->expr->As<ast::MemberAccessorExpression>();
                if ((array && sem_.TypeOf(array->object)->UnwrapRef()->Is<sem::Vector>()) ||
                    (member && sem_.TypeOf(member->structure)->UnwrapRef()->Is<sem::Vector>())) {
                    AddError("cannot take the address of a vector component", unary->expr->source);
                    return nullptr;
                }

                ty = builder_->create<sem::Pointer>(ref->StoreType(), ref->AddressSpace(),
                                                    ref->Access());

                root_ident = expr->RootIdentifier();
            } else {
                AddError("cannot take the address of expression", unary->expr->source);
                return nullptr;
            }
            break;

        case ast::UnaryOp::kIndirection:
            if (auto* ptr = expr_ty->As<sem::Pointer>()) {
                ty = builder_->create<sem::Reference>(ptr->StoreType(), ptr->AddressSpace(),
                                                      ptr->Access());
                root_ident = expr->RootIdentifier();
            } else {
                AddError("cannot dereference expression of type '" + sem_.TypeNameOf(expr_ty) + "'",
                         unary->expr->source);
                return nullptr;
            }
            break;

        default: {
            stage = expr->Stage();
            auto op = intrinsic_table_->Lookup(unary->op, expr_ty, stage, unary->source);
            if (!op.result) {
                return nullptr;
            }
            ty = op.result;
            if (ShouldMaterializeArgument(op.parameter)) {
                expr = Materialize(expr, op.parameter);
                if (!expr) {
                    return nullptr;
                }
            }
            if (stage == sem::EvaluationStage::kConstant) {
                if (op.const_eval_fn) {
                    if (auto r = (const_eval_.*op.const_eval_fn)(
                            ty, utils::Vector{expr->ConstantValue()},
                            expr->Declaration()->source)) {
                        value = r.Get();
                    } else {
                        return nullptr;
                    }
                } else {
                    stage = sem::EvaluationStage::kRuntime;
                }
            }
            break;
        }
    }

    auto* sem = builder_->create<sem::Expression>(unary, ty, stage, current_statement_, value,
                                                  expr->HasSideEffects(), root_ident);
    sem->Behaviors() = expr->Behaviors();
    return sem;
}

bool Resolver::Enable(const ast::Enable* enable) {
    enabled_extensions_.Add(enable->extension);
    return true;
}

sem::Type* Resolver::TypeDecl(const ast::TypeDecl* named_type) {
    sem::Type* result = nullptr;
    if (auto* alias = named_type->As<ast::Alias>()) {
        result = Alias(alias);
    } else if (auto* str = named_type->As<ast::Struct>()) {
        result = Structure(str);
    } else {
        TINT_UNREACHABLE(Resolver, diagnostics_) << "Unhandled TypeDecl";
    }

    if (!result) {
        return nullptr;
    }

    builder_->Sem().Add(named_type, result);
    return result;
}

sem::Array* Resolver::Array(const ast::Array* arr) {
    if (!arr->type) {
        AddError("missing array element type", arr->source.End());
        return nullptr;
    }

    auto* el_ty = Type(arr->type);
    if (!el_ty) {
        return nullptr;
    }

    // Look for explicit stride via @stride(n) attribute
    uint32_t explicit_stride = 0;
    if (!ArrayAttributes(arr->attributes, el_ty, explicit_stride)) {
        return nullptr;
    }

    sem::ArrayCount el_count = sem::RuntimeArrayCount{};

    // Evaluate the constant array count expression.
    if (auto* count_expr = arr->count) {
        if (auto count = ArrayCount(count_expr)) {
            el_count = count.Get();
        } else {
            return nullptr;
        }
    }

    auto* out = Array(arr->type->source,                              //
                      arr->count ? arr->count->source : arr->source,  //
                      el_ty, el_count, explicit_stride);
    if (out == nullptr) {
        return nullptr;
    }

    if (el_ty->Is<sem::Atomic>()) {
        atomic_composite_info_.Add(out, &arr->type->source);
    } else {
        if (auto found = atomic_composite_info_.Get(el_ty)) {
            atomic_composite_info_.Add(out, *found);
        }
    }

    return out;
}

utils::Result<sem::ArrayCount> Resolver::ArrayCount(const ast::Expression* count_expr) {
    // Evaluate the constant array count expression.
    const auto* count_sem = Materialize(Expression(count_expr));
    if (!count_sem) {
        return utils::Failure;
    }

    if (count_sem->Stage() == sem::EvaluationStage::kOverride) {
        // array count is an override expression.
        // Is the count a named 'override'?
        if (auto* user = count_sem->UnwrapMaterialize()->As<sem::VariableUser>()) {
            if (auto* global = user->Variable()->As<sem::GlobalVariable>()) {
                return sem::ArrayCount{sem::NamedOverrideArrayCount{global}};
            }
        }
        return sem::ArrayCount{sem::UnnamedOverrideArrayCount{count_sem}};
    }

    auto* count_val = count_sem->ConstantValue();
    if (!count_val) {
        AddError("array count must evaluate to a constant integer expression or override variable",
                 count_expr->source);
        return utils::Failure;
    }

    if (auto* ty = count_val->Type(); !ty->is_integer_scalar()) {
        AddError("array count must evaluate to a constant integer expression, but is type '" +
                     builder_->FriendlyName(ty) + "'",
                 count_expr->source);
        return utils::Failure;
    }

    int64_t count = count_val->As<AInt>();
    if (count < 1) {
        AddError("array count (" + std::to_string(count) + ") must be greater than 0",
                 count_expr->source);
        return utils::Failure;
    }

    return sem::ArrayCount{sem::ConstantArrayCount{static_cast<uint32_t>(count)}};
}

bool Resolver::ArrayAttributes(utils::VectorRef<const ast::Attribute*> attributes,
                               const sem::Type* el_ty,
                               uint32_t& explicit_stride) {
    if (!validator_.NoDuplicateAttributes(attributes)) {
        return false;
    }

    for (auto* attr : attributes) {
        Mark(attr);
        if (auto* sd = attr->As<ast::StrideAttribute>()) {
            explicit_stride = sd->stride;
            if (!validator_.ArrayStrideAttribute(sd, el_ty->Size(), el_ty->Align())) {
                return false;
            }
            continue;
        }

        AddError("attribute is not valid for array types", attr->source);
        return false;
    }

    return true;
}

sem::Array* Resolver::Array(const Source& el_source,
                            const Source& count_source,
                            const sem::Type* el_ty,
                            sem::ArrayCount el_count,
                            uint32_t explicit_stride) {
    uint32_t el_align = el_ty->Align();
    uint32_t el_size = el_ty->Size();
    uint64_t implicit_stride = el_size ? utils::RoundUp<uint64_t>(el_align, el_size) : 0;
    uint64_t stride = explicit_stride ? explicit_stride : implicit_stride;
    uint64_t size = 0;

    if (auto const_count = std::get_if<sem::ConstantArrayCount>(&el_count)) {
        size = const_count->value * stride;
        if (size > std::numeric_limits<uint32_t>::max()) {
            std::stringstream msg;
            msg << "array byte size (0x" << std::hex << size
                << ") must not exceed 0xffffffff bytes";
            AddError(msg.str(), count_source);
            return nullptr;
        }
    } else if (std::holds_alternative<sem::RuntimeArrayCount>(el_count)) {
        size = stride;
    }
    auto* out = builder_->create<sem::Array>(el_ty, el_count, el_align, static_cast<uint32_t>(size),
                                             static_cast<uint32_t>(stride),
                                             static_cast<uint32_t>(implicit_stride));

    if (!validator_.Array(out, el_source)) {
        return nullptr;
    }

    return out;
}

sem::Type* Resolver::Alias(const ast::Alias* alias) {
    auto* ty = Type(alias->type);
    if (!ty) {
        return nullptr;
    }
    if (!validator_.Alias(alias)) {
        return nullptr;
    }
    return ty;
}

sem::Struct* Resolver::Structure(const ast::Struct* str) {
    if (!validator_.NoDuplicateAttributes(str->attributes)) {
        return nullptr;
    }
    for (auto* attr : str->attributes) {
        Mark(attr);
    }

    sem::StructMemberList sem_members;
    sem_members.reserve(str->members.Length());

    // Calculate the effective size and alignment of each field, and the overall
    // size of the structure.
    // For size, use the size attribute if provided, otherwise use the default
    // size for the type.
    // For alignment, use the alignment attribute if provided, otherwise use the
    // default alignment for the member type.
    // Diagnostic errors are raised if a basic rule is violated.
    // Validation of storage-class rules requires analyzing the actual variable
    // usage of the structure, and so is performed as part of the variable
    // validation.
    uint64_t struct_size = 0;
    uint64_t struct_align = 1;
    utils::Hashmap<Symbol, const ast::StructMember*, 8> member_map;

    for (auto* member : str->members) {
        Mark(member);
        if (auto added = member_map.Add(member->symbol, member); !added) {
            AddError("redefinition of '" + builder_->Symbols().NameFor(member->symbol) + "'",
                     member->source);
            AddNote("previous definition is here", (*added.value)->source);
            return nullptr;
        }

        // Resolve member type
        auto* type = Type(member->type);
        if (!type) {
            return nullptr;
        }

        // validator_.Validate member type
        if (!validator_.IsPlain(type)) {
            AddError(sem_.TypeNameOf(type) + " cannot be used as the type of a structure member",
                     member->source);
            return nullptr;
        }

        uint64_t offset = struct_size;
        uint64_t align = type->Align();
        uint64_t size = type->Size();

        if (!validator_.NoDuplicateAttributes(member->attributes)) {
            return nullptr;
        }

        bool has_offset_attr = false;
        bool has_align_attr = false;
        bool has_size_attr = false;
        std::optional<uint32_t> location;
        for (auto* attr : member->attributes) {
            Mark(attr);
            bool ok = Switch(
                attr,  //
                [&](const ast::StructMemberOffsetAttribute* o) {
                    // Offset attributes are not part of the WGSL spec, but are emitted
                    // by the SPIR-V reader.
                    ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant,
                                                       "@offset value"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(Expression(o->expr));
                    if (!materialized) {
                        return false;
                    }
                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@offset must be constant expression", o->expr->source);
                        return false;
                    }
                    offset = const_value->As<uint64_t>();

                    if (offset < struct_size) {
                        AddError("offsets must be in ascending order", o->source);
                        return false;
                    }
                    align = 1;
                    has_offset_attr = true;
                    return true;
                },
                [&](const ast::StructMemberAlignAttribute* a) {
                    ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@align"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(Expression(a->expr));
                    if (!materialized) {
                        return false;
                    }
                    if (!materialized->Type()->IsAnyOf<sem::I32, sem::U32>()) {
                        AddError("@align must be an i32 or u32 value", a->source);
                        return false;
                    }

                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@align must be constant expression", a->source);
                        return false;
                    }
                    auto value = const_value->As<AInt>();

                    if (value <= 0 || !utils::IsPowerOfTwo(value)) {
                        AddError("@align value must be a positive, power-of-two integer",
                                 a->source);
                        return false;
                    }
                    align = u32(value);
                    has_align_attr = true;
                    return true;
                },
                [&](const ast::StructMemberSizeAttribute* s) {
                    ExprEvalStageConstraint constraint{sem::EvaluationStage::kConstant, "@size"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(Expression(s->expr));
                    if (!materialized) {
                        return false;
                    }
                    if (!materialized->Type()->IsAnyOf<sem::U32, sem::I32>()) {
                        AddError("@size must be an i32 or u32 value", s->source);
                        return false;
                    }

                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@size must be constant expression", s->expr->source);
                        return false;
                    }
                    {
                        auto value = const_value->As<AInt>();
                        if (value <= 0) {
                            AddError("@size must be a positive integer", s->source);
                            return false;
                        }
                    }
                    auto value = const_value->As<uint64_t>();
                    if (value < size) {
                        AddError("@size must be at least as big as the type's size (" +
                                     std::to_string(size) + ")",
                                 s->source);
                        return false;
                    }
                    size = u32(value);
                    has_size_attr = true;
                    return true;
                },
                [&](const ast::LocationAttribute* loc_attr) {
                    auto value = LocationAttribute(loc_attr);
                    if (!value) {
                        return false;
                    }
                    location = value.Get();
                    return true;
                },
                [&](Default) {
                    // The validator will check attributes can be applied to the struct member.
                    return true;
                });
            if (!ok) {
                return nullptr;
            }
        }

        if (has_offset_attr && (has_align_attr || has_size_attr)) {
            AddError("@offset cannot be used with @align or @size", member->source);
            return nullptr;
        }

        offset = utils::RoundUp(align, offset);
        if (offset > std::numeric_limits<uint32_t>::max()) {
            std::stringstream msg;
            msg << "struct member offset (0x" << std::hex << offset << ") must not exceed 0x"
                << std::hex << std::numeric_limits<uint32_t>::max() << " bytes";
            AddError(msg.str(), member->source);
            return nullptr;
        }

        auto* sem_member = builder_->create<sem::StructMember>(
            member, member->symbol, type, static_cast<uint32_t>(sem_members.size()),
            static_cast<uint32_t>(offset), static_cast<uint32_t>(align),
            static_cast<uint32_t>(size), location);
        builder_->Sem().Add(member, sem_member);
        sem_members.emplace_back(sem_member);

        struct_size = offset + size;
        struct_align = std::max(struct_align, align);
    }

    uint64_t size_no_padding = struct_size;
    struct_size = utils::RoundUp(struct_align, struct_size);

    if (struct_size > std::numeric_limits<uint32_t>::max()) {
        std::stringstream msg;
        msg << "struct size (0x" << std::hex << struct_size << ") must not exceed 0xffffffff bytes";
        AddError(msg.str(), str->source);
        return nullptr;
    }
    if (struct_align > std::numeric_limits<uint32_t>::max()) {
        TINT_ICE(Resolver, diagnostics_) << "calculated struct stride exceeds uint32";
        return nullptr;
    }

    auto* out = builder_->create<sem::Struct>(
        str, str->name, sem_members, static_cast<uint32_t>(struct_align),
        static_cast<uint32_t>(struct_size), static_cast<uint32_t>(size_no_padding));

    for (size_t i = 0; i < sem_members.size(); i++) {
        auto* mem_type = sem_members[i]->Type();
        if (mem_type->Is<sem::Atomic>()) {
            atomic_composite_info_.Add(out, &sem_members[i]->Declaration()->source);
            break;
        } else {
            if (auto found = atomic_composite_info_.Get(mem_type)) {
                atomic_composite_info_.Add(out, *found);
                break;
            }
        }

        const_cast<sem::StructMember*>(sem_members[i])->SetStruct(out);
    }

    auto stage = current_function_ ? current_function_->Declaration()->PipelineStage()
                                   : ast::PipelineStage::kNone;
    if (!validator_.Structure(out, stage)) {
        return nullptr;
    }

    return out;
}

sem::Statement* Resolver::ReturnStatement(const ast::ReturnStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = current_statement_->Behaviors();
        behaviors = sem::Behavior::kReturn;

        const sem::Type* value_ty = nullptr;
        if (auto* value = stmt->value) {
            const auto* expr = Expression(value);
            if (!expr) {
                return false;
            }
            if (auto* ret_ty = current_function_->ReturnType(); !ret_ty->Is<sem::Void>()) {
                expr = Materialize(expr, ret_ty);
                if (!expr) {
                    return false;
                }
            }
            behaviors.Add(expr->Behaviors() - sem::Behavior::kNext);
            value_ty = expr->Type()->UnwrapRef();
        } else {
            value_ty = builder_->create<sem::Void>();
        }

        // Validate after processing the return value expression so that its type
        // is available for validation.
        return validator_.Return(stmt, current_function_->ReturnType(), value_ty,
                                 current_statement_);
    });
}

sem::SwitchStatement* Resolver::SwitchStatement(const ast::SwitchStatement* stmt) {
    auto* sem = builder_->create<sem::SwitchStatement>(stmt, current_compound_statement_,
                                                       current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = sem->Behaviors();

        const auto* cond = Expression(stmt->condition);
        if (!cond) {
            return false;
        }
        behaviors = cond->Behaviors() - sem::Behavior::kNext;

        auto* cond_ty = cond->Type()->UnwrapRef();

        // Determine the common type across all selectors and the switch expression
        // This must materialize to an integer scalar (non-abstract).
        utils::Vector<const sem::Type*, 8> types;
        types.Push(cond_ty);
        for (auto* case_stmt : stmt->body) {
            for (auto* sel : case_stmt->selectors) {
                if (sel->IsDefault()) {
                    continue;
                }
                auto* sem_expr = Expression(sel->expr);
                if (!sem_expr) {
                    return false;
                }
                types.Push(sem_expr->Type()->UnwrapRef());
            }
        }
        auto* common_ty = sem::Type::Common(types);
        if (!common_ty || !common_ty->is_integer_scalar()) {
            // No common type found or the common type was abstract.
            // Pick i32 and let validation deal with any mismatches.
            common_ty = builder_->create<sem::I32>();
        }
        cond = Materialize(cond, common_ty);
        if (!cond) {
            return false;
        }

        utils::Vector<sem::CaseStatement*, 4> cases;
        cases.Reserve(stmt->body.Length());
        for (auto* case_stmt : stmt->body) {
            Mark(case_stmt);
            auto* c = CaseStatement(case_stmt, common_ty);
            if (!c) {
                return false;
            }
            cases.Push(c);
            behaviors.Add(c->Behaviors());
            sem->Cases().emplace_back(c);
        }

        if (behaviors.Contains(sem::Behavior::kBreak)) {
            behaviors.Add(sem::Behavior::kNext);
        }
        behaviors.Remove(sem::Behavior::kBreak, sem::Behavior::kFallthrough);

        return validator_.SwitchStatement(stmt);
    });
}

sem::Statement* Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        Mark(stmt->variable);

        auto* variable = Variable(stmt->variable, /* is_global */ false);
        if (!variable) {
            return false;
        }

        for (auto* attr : stmt->variable->attributes) {
            Mark(attr);
            if (!attr->Is<ast::InternalAttribute>()) {
                AddError("attributes are not valid on local variables", attr->source);
                return false;
            }
        }

        current_compound_statement_->AddDecl(variable->As<sem::LocalVariable>());

        if (auto* ctor = variable->Initializer()) {
            sem->Behaviors() = ctor->Behaviors();
        }

        return validator_.LocalVariable(variable);
    });
}

sem::Statement* Resolver::AssignmentStatement(const ast::AssignmentStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = Expression(stmt->lhs);
        if (!lhs) {
            return false;
        }

        const bool is_phony_assignment = stmt->lhs->Is<ast::PhonyExpression>();

        const auto* rhs = Expression(stmt->rhs);
        if (!rhs) {
            return false;
        }

        if (!is_phony_assignment) {
            rhs = Materialize(rhs, lhs->Type()->UnwrapRef());
            if (!rhs) {
                return false;
            }
        }

        auto& behaviors = sem->Behaviors();
        behaviors = rhs->Behaviors();
        if (!is_phony_assignment) {
            behaviors.Add(lhs->Behaviors());
        }

        return validator_.Assignment(stmt, sem_.TypeOf(stmt->rhs));
    });
}

sem::Statement* Resolver::BreakStatement(const ast::BreakStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Behaviors() = sem::Behavior::kBreak;

        return validator_.BreakStatement(sem, current_statement_);
    });
}

sem::Statement* Resolver::BreakIfStatement(const ast::BreakIfStatement* stmt) {
    auto* sem = builder_->create<sem::BreakIfStatement>(stmt, current_compound_statement_,
                                                        current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* cond = Expression(stmt->condition);
        if (!cond) {
            return false;
        }
        sem->SetCondition(cond);
        sem->Behaviors() = cond->Behaviors();
        sem->Behaviors().Add(sem::Behavior::kBreak);

        return validator_.BreakIfStatement(sem, current_statement_);
    });
}

sem::Statement* Resolver::CallStatement(const ast::CallStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        if (auto* expr = Expression(stmt->expr)) {
            sem->Behaviors() = expr->Behaviors();
            return true;
        }
        return false;
    });
}

sem::Statement* Resolver::CompoundAssignmentStatement(
    const ast::CompoundAssignmentStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = Expression(stmt->lhs);
        if (!lhs) {
            return false;
        }

        auto* rhs = Expression(stmt->rhs);
        if (!rhs) {
            return false;
        }

        sem->Behaviors() = rhs->Behaviors() + lhs->Behaviors();

        auto* lhs_ty = lhs->Type()->UnwrapRef();
        auto* rhs_ty = rhs->Type()->UnwrapRef();
        auto stage = sem::EarliestStage(lhs->Stage(), rhs->Stage());
        auto* ty =
            intrinsic_table_->Lookup(stmt->op, lhs_ty, rhs_ty, stage, stmt->source, true).result;
        if (!ty) {
            return false;
        }
        return validator_.Assignment(stmt, ty);
    });
}

sem::Statement* Resolver::ContinueStatement(const ast::ContinueStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Behaviors() = sem::Behavior::kContinue;

        // Set if we've hit the first continue statement in our parent loop
        if (auto* block = sem->FindFirstParent<sem::LoopBlockStatement>()) {
            if (!block->FirstContinue()) {
                const_cast<sem::LoopBlockStatement*>(block)->SetFirstContinue(
                    stmt, block->Decls().Count());
            }
        }

        return validator_.ContinueStatement(sem, current_statement_);
    });
}

sem::Statement* Resolver::DiscardStatement(const ast::DiscardStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        current_function_->SetDiscardStatement(sem);
        return true;
    });
}

sem::Statement* Resolver::FallthroughStatement(const ast::FallthroughStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Behaviors() = sem::Behavior::kFallthrough;

        return validator_.FallthroughStatement(sem);
    });
}

sem::Statement* Resolver::IncrementDecrementStatement(
    const ast::IncrementDecrementStatement* stmt) {
    auto* sem =
        builder_->create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = Expression(stmt->lhs);
        if (!lhs) {
            return false;
        }
        sem->Behaviors() = lhs->Behaviors();

        return validator_.IncrementDecrementStatement(stmt);
    });
}

bool Resolver::ApplyAddressSpaceUsageToType(ast::AddressSpace address_space,
                                            sem::Type* ty,
                                            const Source& usage) {
    ty = const_cast<sem::Type*>(ty->UnwrapRef());

    if (auto* str = ty->As<sem::Struct>()) {
        if (str->AddressSpaceUsage().count(address_space)) {
            return true;  // Already applied
        }

        str->AddUsage(address_space);

        for (auto* member : str->Members()) {
            if (!ApplyAddressSpaceUsageToType(address_space, const_cast<sem::Type*>(member->Type()),
                                              usage)) {
                std::stringstream err;
                err << "while analyzing structure member " << sem_.TypeNameOf(str) << "."
                    << builder_->Symbols().NameFor(member->Declaration()->symbol);
                AddNote(err.str(), member->Declaration()->source);
                return false;
            }
        }
        return true;
    }

    if (auto* arr = ty->As<sem::Array>()) {
        if (address_space != ast::AddressSpace::kStorage) {
            if (arr->IsRuntimeSized()) {
                AddError("runtime-sized arrays can only be used in the <storage> address space",
                         usage);
                return false;
            }

            auto count = arr->ConstantCount();
            if (count.has_value() && count.value() >= kMaxArrayElementCount) {
                AddError("array count (" + std::to_string(count.value()) + ") must be less than " +
                             std::to_string(kMaxArrayElementCount),
                         usage);
                return false;
            }
        }
        return ApplyAddressSpaceUsageToType(address_space, const_cast<sem::Type*>(arr->ElemType()),
                                            usage);
    }

    if (ast::IsHostShareable(address_space) && !validator_.IsHostShareable(ty)) {
        std::stringstream err;
        err << "Type '" << sem_.TypeNameOf(ty) << "' cannot be used in address space '"
            << address_space << "' as it is non-host-shareable";
        AddError(err.str(), usage);
        return false;
    }

    return true;
}

template <typename SEM, typename F>
SEM* Resolver::StatementScope(const ast::Statement* ast, SEM* sem, F&& callback) {
    builder_->Sem().Add(ast, sem);

    auto* as_compound = As<sem::CompoundStatement, CastFlags::kDontErrorOnImpossibleCast>(sem);

    TINT_SCOPED_ASSIGNMENT(current_statement_, sem);
    TINT_SCOPED_ASSIGNMENT(current_compound_statement_,
                           as_compound ? as_compound : current_compound_statement_);
    TINT_SCOPED_ASSIGNMENT(current_scoping_depth_, current_scoping_depth_ + 1);

    if (current_scoping_depth_ > kMaxStatementDepth) {
        AddError("statement nesting depth / chaining length exceeds limit of " +
                     std::to_string(kMaxStatementDepth),
                 ast->source);
        return nullptr;
    }

    if (!callback()) {
        return nullptr;
    }

    return sem;
}

bool Resolver::Mark(const ast::Node* node) {
    if (node == nullptr) {
        TINT_ICE(Resolver, diagnostics_) << "Resolver::Mark() called with nullptr";
        return false;
    }
    auto marked_bit_ref = marked_[node->node_id.value];
    if (!marked_bit_ref) {
        marked_bit_ref = true;
        return true;
    }
    TINT_ICE(Resolver, diagnostics_) << "AST node '" << node->TypeInfo().name
                                     << "' was encountered twice in the same AST of a Program\n"
                                     << "At: " << node->source << "\n"
                                     << "Pointer: " << node;
    return false;
}

void Resolver::AddError(const std::string& msg, const Source& source) const {
    diagnostics_.add_error(diag::System::Resolver, msg, source);
}

void Resolver::AddWarning(const std::string& msg, const Source& source) const {
    diagnostics_.add_warning(diag::System::Resolver, msg, source);
}

void Resolver::AddNote(const std::string& msg, const Source& source) const {
    diagnostics_.add_note(diag::System::Resolver, msg, source);
}

bool Resolver::IsBuiltin(Symbol symbol) const {
    std::string name = builder_->Symbols().NameFor(symbol);
    return sem::ParseBuiltinType(name) != sem::BuiltinType::kNone;
}

}  // namespace tint::resolver
