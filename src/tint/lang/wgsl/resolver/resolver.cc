// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/resolver/resolver.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>

#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/attribute.h"
#include "src/tint/lang/wgsl/ast/bitcast_expression.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/ast/unary_op_expression.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/intrinsic/ctor_conv.h"
#include "src/tint/lang/wgsl/intrinsic/dialect.h"
#include "src/tint/lang/wgsl/resolver/incomplete_type.h"
#include "src/tint/lang/wgsl/resolver/uniformity.h"
#include "src/tint/lang/wgsl/resolver/unresolved_identifier.h"
#include "src/tint/lang/wgsl/sem/array.h"
#include "src/tint/lang/wgsl/sem/break_if_statement.h"
#include "src/tint/lang/wgsl/sem/builtin_enum_expression.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/function_expression.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/index_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/loop_statement.h"
#include "src/tint/lang/wgsl/sem/materialize.h"
#include "src/tint/lang/wgsl/sem/member_accessor_expression.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/type_expression.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"
#include "src/tint/utils/containers/reverse.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::resolver {
namespace {

using CtorConvIntrinsic = wgsl::intrinsic::CtorConv;
using OverloadFlag = core::intrinsic::OverloadFlag;

constexpr int64_t kMaxArrayElementCount = 65536;
constexpr uint32_t kMaxStatementDepth = 127;
constexpr size_t kMaxNestDepthOfCompositeType = 255;

}  // namespace

Resolver::Resolver(ProgramBuilder* builder)
    : b(*builder),
      diagnostics_(builder->Diagnostics()),
      const_eval_(builder->constants, diagnostics_),
      intrinsic_table_{builder->Types(), builder->Symbols(), builder->Diagnostics()},
      sem_(builder),
      validator_(builder,
                 sem_,
                 enabled_extensions_,
                 atomic_composite_info_,
                 valid_type_storage_layouts_) {}

Resolver::~Resolver() = default;

bool Resolver::Resolve() {
    if (diagnostics_.contains_errors()) {
        return false;
    }

    b.Sem().Reserve(b.LastAllocatedNodeID());

    // Pre-allocate the marked bitset with the total number of AST nodes.
    marked_.Resize(b.ASTNodes().Count());

    if (!DependencyGraph::Build(b.AST(), diagnostics_, dependencies_)) {
        return false;
    }

    bool result = ResolveInternal();

    if (TINT_UNLIKELY(!result && !diagnostics_.contains_errors())) {
        AddICE("resolving failed, but no error was raised", {});
        return false;
    }

    // Create the semantic module. Don't be tempted to std::move() these, they're used below.
    auto* mod = b.create<sem::Module>(dependencies_.ordered_globals, enabled_extensions_);
    ApplyDiagnosticSeverities(mod);
    b.Sem().SetModule(mod);

    const bool disable_uniformity_analysis =
        enabled_extensions_.Contains(wgsl::Extension::kChromiumDisableUniformityAnalysis);
    if (result && !disable_uniformity_analysis) {
        // Run the uniformity analysis, which requires a complete semantic module.
        if (!AnalyzeUniformity(b, dependencies_)) {
            return false;
        }
    }

    return result;
}

bool Resolver::ResolveInternal() {
    Mark(&b.AST());

    // Process all module-scope declarations in dependency order.
    Vector<const ast::DiagnosticControl*, 4> diagnostic_controls;
    for (auto* decl : dependencies_.ordered_globals) {
        Mark(decl);
        if (!Switch<bool>(
                decl,  //
                [&](const ast::DiagnosticDirective* d) {
                    diagnostic_controls.Push(&d->control);
                    return DiagnosticControl(d->control);
                },
                [&](const ast::Enable* e) { return Enable(e); },
                [&](const ast::TypeDecl* td) { return TypeDecl(td); },
                [&](const ast::Function* func) { return Function(func); },
                [&](const ast::Variable* var) { return GlobalVariable(var); },
                [&](const ast::ConstAssert* ca) { return ConstAssert(ca); },  //
                TINT_ICE_ON_NO_MATCH)) {
            return false;
        }
    }

    if (!AllocateOverridableConstantIds()) {
        return false;
    }

    SetShadows();

    if (!validator_.DiagnosticControls(diagnostic_controls, "directive")) {
        return false;
    }

    if (!validator_.PipelineStages(entry_points_)) {
        return false;
    }

    if (!validator_.ModuleScopeVarUsages(entry_points_)) {
        return false;
    }

    bool result = true;
    for (auto* node : b.ASTNodes().Objects()) {
        if (TINT_UNLIKELY(!marked_[node->node_id.value])) {
            StringStream err;
            err << "AST node '" << node->TypeInfo().name << "' was not reached by the resolver\n"
                << "Pointer: " << node;
            AddICE(err.str(), node->source);
            result = false;
        }
    }

    return result;
}

sem::Variable* Resolver::Variable(const ast::Variable* v, bool is_global) {
    Mark(v->name);

    return Switch(
        v,  //
        [&](const ast::Var* var) { return Var(var, is_global); },
        [&](const ast::Let* let) { return Let(let); },
        [&](const ast::Override* override) { return Override(override); },
        [&](const ast::Const* const_) { return Const(const_, is_global); },  //
        TINT_ICE_ON_NO_MATCH);
}

sem::Variable* Resolver::Let(const ast::Let* v) {
    auto* sem = b.create<sem::LocalVariable>(v, current_statement_);
    sem->SetStage(core::EvaluationStage::kRuntime);
    b.Sem().Add(v, sem);

    // If the variable has a declared type, resolve it.
    if (v->type) {
        auto* ty = Type(v->type);
        if (TINT_UNLIKELY(!ty)) {
            return nullptr;
        }
        sem->SetType(ty);
    }

    for (auto* attribute : v->attributes) {
        Mark(attribute);
        bool ok = Switch(
            attribute,  //
            [&](const ast::InternalAttribute* attr) -> bool { return InternalAttribute(attr); },
            [&](Default) {
                ErrorInvalidAttribute(attribute, "'let' declaration");
                return false;
            });
        if (!ok) {
            return nullptr;
        }
    }

    if (TINT_UNLIKELY(!v->initializer)) {
        AddError("'let' declaration must have an initializer", v->source);
        return nullptr;
    }

    auto* rhs = Load(Materialize(ValueExpression(v->initializer), sem->Type()));
    if (TINT_UNLIKELY(!rhs)) {
        return nullptr;
    }
    sem->SetInitializer(rhs);

    // If the variable has no declared type, infer it from the RHS
    if (!sem->Type()) {
        sem->SetType(rhs->Type()->UnwrapRef());  // Implicit load of RHS
    }

    if (TINT_UNLIKELY(rhs && !validator_.VariableInitializer(v, sem->Type(), rhs))) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(core::AddressSpace::kUndefined,
                                      const_cast<core::type::Type*>(sem->Type()), v->source)) {
        AddNote("while instantiating 'let' " + v->name->symbol.Name(), v->source);
        return nullptr;
    }

    return sem;
}

sem::Variable* Resolver::Override(const ast::Override* v) {
    auto* sem = b.create<sem::GlobalVariable>(v);
    b.Sem().Add(v, sem);
    sem->SetStage(core::EvaluationStage::kOverride);

    on_transitively_reference_global_.Push([&](const sem::GlobalVariable* ref) {
        if (ref->Declaration()->Is<ast::Override>()) {
            sem->AddTransitivelyReferencedOverride(ref);
        }
    });
    TINT_DEFER(on_transitively_reference_global_.Pop());

    // If the variable has a declared type, resolve it.
    const core::type::Type* ty = nullptr;
    if (v->type) {
        ty = Type(v->type);
        if (!ty) {
            return nullptr;
        }
    }

    // Does the variable have an initializer?
    const sem::ValueExpression* init = nullptr;
    if (v->initializer) {
        // Note: RHS must be a const or override expression, which excludes references.
        // So there's no need to load or unwrap references here.
        ExprEvalStageConstraint constraint{core::EvaluationStage::kOverride,
                                           "override initializer"};
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
        init = Materialize(ValueExpression(v->initializer), ty);
        if (TINT_UNLIKELY(!init)) {
            return nullptr;
        }
        sem->SetInitializer(init);

        // If the variable has no declared type, infer it from the initializer
        if (!ty) {
            ty = init->Type();
        }
    } else if (!ty) {
        AddError("override declaration requires a type or initializer", v->source);
        return nullptr;
    }
    sem->SetType(ty);

    if (init && !validator_.VariableInitializer(v, ty, init)) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(core::AddressSpace::kUndefined,
                                      const_cast<core::type::Type*>(ty), v->source)) {
        AddNote("while instantiating 'override' " + v->name->symbol.Name(), v->source);
        return nullptr;
    }

    for (auto* attribute : v->attributes) {
        Mark(attribute);
        bool ok = Switch(
            attribute,  //
            [&](const ast::IdAttribute* attr) {
                ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@id"};
                TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                auto* materialized = Materialize(ValueExpression(attr->expr));
                if (!materialized) {
                    return false;
                }
                if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
                    AddError("@id must be an i32 or u32 value", attr->source);
                    return false;
                }

                auto const_value = materialized->ConstantValue();
                auto value = const_value->ValueAs<AInt>();
                if (value < 0) {
                    AddError("@id value must be non-negative", attr->source);
                    return false;
                }
                if (value > std::numeric_limits<decltype(OverrideId::value)>::max()) {
                    AddError(
                        "@id value must be between 0 and " +
                            std::to_string(std::numeric_limits<decltype(OverrideId::value)>::max()),
                        attr->source);
                    return false;
                }

                auto o = OverrideId{static_cast<decltype(OverrideId::value)>(value)};
                sem->SetOverrideId(o);

                // Track the constant IDs that are specified in the shader.
                override_ids_.Add(o, sem);
                return true;
            },
            [&](Default) {
                ErrorInvalidAttribute(attribute, "'override' declaration");
                return false;
            });
        if (!ok) {
            return nullptr;
        }
    }

    return sem;
}

sem::Variable* Resolver::Const(const ast::Const* c, bool is_global) {
    sem::Variable* sem = nullptr;
    sem::GlobalVariable* global = nullptr;
    if (is_global) {
        global = b.create<sem::GlobalVariable>(c);
        sem = global;
    } else {
        sem = b.create<sem::LocalVariable>(c, current_statement_);
    }
    b.Sem().Add(c, sem);

    for (auto* attribute : c->attributes) {
        Mark(attribute);
        bool ok = Switch(attribute,  //
                         [&](Default) {
                             ErrorInvalidAttribute(attribute, "'const' declaration");
                             return false;
                         });
        if (!ok) {
            return nullptr;
        }
    }

    if (TINT_UNLIKELY(!c->initializer)) {
        AddError("'const' declaration must have an initializer", c->source);
        return nullptr;
    }

    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "const initializer"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
    const auto* init = ValueExpression(c->initializer);
    if (TINT_UNLIKELY(!init)) {
        return nullptr;
    }

    // Note: RHS must be a const expression, which excludes references.
    // So there's no need to load or unwrap references here.

    // If the variable has a declared type, resolve it.
    const core::type::Type* ty = nullptr;
    if (c->type) {
        ty = Type(c->type);
        if (TINT_UNLIKELY(!ty)) {
            return nullptr;
        }
    }

    if (ty) {
        // If an explicit type was specified, materialize to that type
        init = Materialize(init, ty);
        if (TINT_UNLIKELY(!init)) {
            return nullptr;
        }
    } else {
        // If no type was specified, infer it from the RHS
        ty = init->Type();
    }

    sem->SetInitializer(init);
    sem->SetStage(core::EvaluationStage::kConstant);
    sem->SetConstantValue(init->ConstantValue());
    sem->SetType(ty);

    if (!validator_.VariableInitializer(c, ty, init)) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(core::AddressSpace::kUndefined,
                                      const_cast<core::type::Type*>(ty), c->source)) {
        AddNote("while instantiating 'const' " + c->name->symbol.Name(), c->source);
        return nullptr;
    }

    return sem;
}

sem::Variable* Resolver::Var(const ast::Var* var, bool is_global) {
    sem::Variable* sem = nullptr;
    sem::GlobalVariable* global = nullptr;
    if (is_global) {
        global = b.create<sem::GlobalVariable>(var);
        sem = global;
    } else {
        sem = b.create<sem::LocalVariable>(var, current_statement_);
    }
    sem->SetStage(core::EvaluationStage::kRuntime);
    b.Sem().Add(var, sem);

    if (is_global) {
        on_transitively_reference_global_.Push([&](const sem::GlobalVariable* ref) {
            if (ref->Declaration()->Is<ast::Override>()) {
                global->AddTransitivelyReferencedOverride(ref);
            }
        });
    }
    TINT_DEFER({
        if (is_global) {
            on_transitively_reference_global_.Pop();
        }
    });

    // If the variable has a declared type, resolve it.
    const core::type::Type* storage_ty = nullptr;
    if (auto ty = var->type) {
        storage_ty = Type(ty);
        if (TINT_UNLIKELY(!storage_ty)) {
            return nullptr;
        }
    }

    // Does the variable have a initializer?
    if (var->initializer) {
        ExprEvalStageConstraint constraint{
            is_global ? core::EvaluationStage::kOverride : core::EvaluationStage::kRuntime,
            "var initializer",
        };
        TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

        auto* init = Load(Materialize(ValueExpression(var->initializer), storage_ty));
        if (TINT_UNLIKELY(!init)) {
            return nullptr;
        }
        sem->SetInitializer(init);

        // If the variable has no declared type, infer it from the RHS
        if (!storage_ty) {
            storage_ty = init->Type();
        }
    }

    if (!storage_ty) {
        AddError("var declaration requires a type or initializer", var->source);
        return nullptr;
    }

    if (var->declared_address_space) {
        auto space = AddressSpaceExpression(var->declared_address_space);
        if (TINT_UNLIKELY(!space)) {
            return nullptr;
        }
        sem->SetAddressSpace(space->Value());
    } else {
        // No declared address space. Infer from usage / type.
        if (!is_global) {
            sem->SetAddressSpace(core::AddressSpace::kFunction);
        } else if (storage_ty->UnwrapRef()->is_handle()) {
            // https://gpuweb.github.io/gpuweb/wgsl/#module-scope-variables
            // If the store type is a texture type or a sampler type, then the
            // variable declaration must not have a address space attribute. The
            // address space will always be handle.
            sem->SetAddressSpace(core::AddressSpace::kHandle);
        }
    }

    if (!is_global && sem->AddressSpace() != core::AddressSpace::kFunction &&
        validator_.IsValidationEnabled(var->attributes,
                                       ast::DisabledValidation::kIgnoreAddressSpace)) {
        AddError("function-scope 'var' declaration must use 'function' address space", var->source);
        return nullptr;
    }

    if (var->declared_access) {
        auto expr = AccessExpression(var->declared_access);
        if (!expr) {
            return nullptr;
        }
        sem->SetAccess(expr->Value());
    } else {
        sem->SetAccess(DefaultAccessForAddressSpace(sem->AddressSpace()));
    }

    sem->SetType(b.create<core::type::Reference>(sem->AddressSpace(), storage_ty, sem->Access()));

    if (sem->Initializer() &&
        !validator_.VariableInitializer(var, storage_ty, sem->Initializer())) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(sem->AddressSpace(),
                                      const_cast<core::type::Type*>(sem->Type()),
                                      var->type ? var->type->source : var->source)) {
        AddNote("while instantiating 'var' " + var->name->symbol.Name(), var->source);
        return nullptr;
    }

    if (is_global) {
        bool has_io_address_space = sem->AddressSpace() == core::AddressSpace::kIn ||
                                    sem->AddressSpace() == core::AddressSpace::kOut;

        std::optional<uint32_t> group, binding;
        for (auto* attribute : var->attributes) {
            Mark(attribute);
            enum Status { kSuccess, kErrored, kInvalid };
            auto res = Switch(
                attribute,  //
                [&](const ast::BindingAttribute* attr) {
                    auto value = BindingAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    binding = value.Get();
                    return kSuccess;
                },
                [&](const ast::GroupAttribute* attr) {
                    auto value = GroupAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    group = value.Get();
                    return kSuccess;
                },
                [&](const ast::LocationAttribute* attr) {
                    if (!has_io_address_space) {
                        return kInvalid;
                    }
                    auto value = LocationAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    global->SetLocation(value.Get());
                    return kSuccess;
                },
                [&](const ast::IndexAttribute* attr) {
                    if (!has_io_address_space) {
                        return kInvalid;
                    }
                    auto value = IndexAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    global->SetIndex(value.Get());
                    return kSuccess;
                },
                [&](const ast::BuiltinAttribute* attr) {
                    if (!has_io_address_space) {
                        return kInvalid;
                    }
                    return BuiltinAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InterpolateAttribute* attr) {
                    if (!has_io_address_space) {
                        return kInvalid;
                    }
                    return InterpolateAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InvariantAttribute* attr) {
                    if (!has_io_address_space) {
                        return kInvalid;
                    }
                    return InvariantAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InternalAttribute* attr) {
                    return InternalAttribute(attr) ? kSuccess : kErrored;
                },
                [&](Default) { return kInvalid; });

            switch (res) {
                case kSuccess:
                    break;
                case kErrored:
                    return nullptr;
                case kInvalid:
                    ErrorInvalidAttribute(attribute, "module-scope 'var'");
                    return nullptr;
            }
        }

        if (group && binding) {
            global->SetBindingPoint(BindingPoint{group.value(), binding.value()});
        }

    } else {
        for (auto* attribute : var->attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,
                [&](const ast::InternalAttribute* attr) { return InternalAttribute(attr); },
                [&](Default) {
                    ErrorInvalidAttribute(attribute, "function-scope 'var'");
                    return false;
                });
            if (!ok) {
                return nullptr;
            }
        }
    }

    return sem;
}

sem::Parameter* Resolver::Parameter(const ast::Parameter* param,
                                    const ast::Function* func,
                                    uint32_t index) {
    Mark(param->name);

    auto* sem = b.create<sem::Parameter>(param, index);
    b.Sem().Add(param, sem);

    auto add_note = [&] {
        AddNote("while instantiating parameter " + param->name->symbol.Name(), param->source);
    };

    if (func->IsEntryPoint()) {
        std::optional<uint32_t> group, binding;
        for (auto* attribute : param->attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,  //
                [&](const ast::LocationAttribute* attr) {
                    auto value = LocationAttribute(attr);
                    if (TINT_UNLIKELY(!value)) {
                        return false;
                    }
                    sem->SetLocation(value.Get());
                    return true;
                },
                [&](const ast::BuiltinAttribute* attr) -> bool { return BuiltinAttribute(attr); },
                [&](const ast::InvariantAttribute* attr) -> bool {
                    return InvariantAttribute(attr);
                },
                [&](const ast::InterpolateAttribute* attr) -> bool {
                    return InterpolateAttribute(attr);
                },
                [&](const ast::InternalAttribute* attr) -> bool { return InternalAttribute(attr); },
                [&](const ast::GroupAttribute* attr) -> bool {
                    if (validator_.IsValidationEnabled(
                            param->attributes, ast::DisabledValidation::kEntryPointParameter)) {
                        ErrorInvalidAttribute(attribute, "function parameters");
                        return false;
                    }
                    auto value = GroupAttribute(attr);
                    if (TINT_UNLIKELY(!value)) {
                        return false;
                    }
                    group = value.Get();
                    return true;
                },
                [&](const ast::BindingAttribute* attr) -> bool {
                    if (validator_.IsValidationEnabled(
                            param->attributes, ast::DisabledValidation::kEntryPointParameter)) {
                        ErrorInvalidAttribute(attribute, "function parameters");
                        return false;
                    }
                    auto value = BindingAttribute(attr);
                    if (TINT_UNLIKELY(!value)) {
                        return false;
                    }
                    binding = value.Get();
                    return true;
                },
                [&](Default) {
                    ErrorInvalidAttribute(attribute, "function parameters");
                    return false;
                });
            if (!ok) {
                return nullptr;
            }
        }
        if (group && binding) {
            sem->SetBindingPoint(BindingPoint{group.value(), binding.value()});
        }
    } else {
        for (auto* attribute : param->attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,  //
                [&](const ast::InternalAttribute* attr) -> bool { return InternalAttribute(attr); },
                [&](Default) {
                    if (attribute->IsAnyOf<ast::LocationAttribute, ast::BuiltinAttribute,
                                           ast::InvariantAttribute, ast::InterpolateAttribute>()) {
                        ErrorInvalidAttribute(attribute, "non-entry point function parameters");
                    } else {
                        ErrorInvalidAttribute(attribute, "function parameters");
                    }
                    return false;
                });
            if (!ok) {
                return nullptr;
            }
        }
    }

    if (!validator_.NoDuplicateAttributes(param->attributes)) {
        return nullptr;
    }

    core::type::Type* ty = Type(param->type);
    if (TINT_UNLIKELY(!ty)) {
        return nullptr;
    }
    sem->SetType(ty);

    if (!ApplyAddressSpaceUsageToType(core::AddressSpace::kUndefined, ty, param->type->source)) {
        add_note();
        return nullptr;
    }

    if (auto* ptr = ty->As<core::type::Pointer>()) {
        // For MSL, we push module-scope variables into the entry point as pointer
        // parameters, so we also need to handle their store type.
        if (!ApplyAddressSpaceUsageToType(ptr->AddressSpace(),
                                          const_cast<core::type::Type*>(ptr->StoreType()),
                                          param->source)) {
            add_note();
            return nullptr;
        }
    }

    if (!validator_.Parameter(sem)) {
        return nullptr;
    }

    return sem;
}

core::Access Resolver::DefaultAccessForAddressSpace(core::AddressSpace address_space) {
    // https://gpuweb.github.io/gpuweb/wgsl/#storage-class
    switch (address_space) {
        case core::AddressSpace::kStorage:
        case core::AddressSpace::kUniform:
        case core::AddressSpace::kHandle:
            return core::Access::kRead;
        default:
            break;
    }
    return core::Access::kReadWrite;
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
    for (auto* decl : b.AST().GlobalDeclarations()) {
        auto* override = decl->As<ast::Override>();
        if (!override) {
            continue;
        }

        auto* sem = sem_.Get(override);

        OverrideId id;
        if (ast::HasAttribute<ast::IdAttribute>(override->attributes)) {
            id = sem->OverrideId();
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

        const_cast<sem::GlobalVariable*>(sem)->SetOverrideId(id);
    }
    return true;
}

void Resolver::SetShadows() {
    for (auto it : dependencies_.shadows) {
        CastableBase* shadowed = sem_.Get(it.value);
        if (TINT_UNLIKELY(!shadowed)) {
            StringStream err;
            err << "AST node '" << it.value->TypeInfo().name << "' had no semantic info\n"
                << "Pointer: " << it.value;
            AddICE(err.str(), it.value->source);
        }

        Switch(
            sem_.Get(it.key),  //
            [&](sem::LocalVariable* local) { local->SetShadows(shadowed); },
            [&](sem::Parameter* param) { param->SetShadows(shadowed); });
    }
}

sem::GlobalVariable* Resolver::GlobalVariable(const ast::Variable* v) {
    auto* sem = As<sem::GlobalVariable>(Variable(v, /* is_global */ true));
    if (!sem) {
        return nullptr;
    }

    if (!validator_.NoDuplicateAttributes(v->attributes)) {
        return nullptr;
    }

    if (!validator_.GlobalVariable(sem, override_ids_)) {
        return nullptr;
    }

    return sem;
}

sem::Statement* Resolver::ConstAssert(const ast::ConstAssert* assertion) {
    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "const assertion"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);
    auto* expr = ValueExpression(assertion->condition);
    if (!expr) {
        return nullptr;
    }
    auto* cond = expr->ConstantValue();
    if (auto* ty = cond->Type(); !ty->Is<core::type::Bool>()) {
        AddError("const assertion condition must be a bool, got '" + ty->FriendlyName() + "'",
                 assertion->condition->source);
        return nullptr;
    }
    if (!cond->ValueAs<bool>()) {
        AddError("const assertion failed", assertion->source);
        return nullptr;
    }
    auto* sem = b.create<sem::Statement>(assertion, current_compound_statement_, current_function_);
    b.Sem().Add(assertion, sem);
    return sem;
}

sem::Function* Resolver::Function(const ast::Function* decl) {
    Mark(decl->name);

    auto* func = b.create<sem::Function>(decl);
    b.Sem().Add(decl, func);
    TINT_SCOPED_ASSIGNMENT(current_function_, func);

    on_transitively_reference_global_.Push([&](const sem::GlobalVariable* ref) {  //
        func->AddDirectlyReferencedGlobal(ref);
    });
    TINT_DEFER(on_transitively_reference_global_.Pop());

    validator_.DiagnosticFilters().Push();
    TINT_DEFER(validator_.DiagnosticFilters().Pop());

    for (auto* attribute : decl->attributes) {
        Mark(attribute);
        bool ok = Switch(
            attribute,
            [&](const ast::DiagnosticAttribute* attr) { return DiagnosticAttribute(attr); },
            [&](const ast::StageAttribute* attr) { return StageAttribute(attr); },
            [&](const ast::MustUseAttribute* attr) { return MustUseAttribute(attr); },
            [&](const ast::WorkgroupAttribute* attr) {
                auto value = WorkgroupAttribute(attr);
                if (!value) {
                    return false;
                }
                func->SetWorkgroupSize(value.Get());
                return true;
            },
            [&](const ast::InternalAttribute* attr) { return InternalAttribute(attr); },
            [&](Default) {
                ErrorInvalidAttribute(attribute, "functions");
                return false;
            });
        if (!ok) {
            return nullptr;
        }
    }
    if (!validator_.NoDuplicateAttributes(decl->attributes)) {
        return nullptr;
    }

    // Resolve all the parameters
    uint32_t parameter_index = 0;
    Hashmap<Symbol, Source, 8> parameter_names;
    for (auto* param : decl->params) {
        Mark(param);

        {  // Check the parameter name is unique for the function
            if (auto added = parameter_names.Add(param->name->symbol, param->source); !added) {
                auto name = param->name->symbol.Name();
                AddError("redefinition of parameter '" + name + "'", param->source);
                AddNote("previous definition is here", *added.value);
                return nullptr;
            }
        }

        auto* p = Parameter(param, decl, parameter_index++);
        if (!p) {
            return nullptr;
        }

        func->AddParameter(p);

        auto* p_ty = const_cast<core::type::Type*>(p->Type());
        if (auto* str = p_ty->As<core::type::Struct>()) {
            switch (decl->PipelineStage()) {
                case ast::PipelineStage::kVertex:
                    str->AddUsage(core::type::PipelineStageUsage::kVertexInput);
                    break;
                case ast::PipelineStage::kFragment:
                    str->AddUsage(core::type::PipelineStageUsage::kFragmentInput);
                    break;
                case ast::PipelineStage::kCompute:
                    str->AddUsage(core::type::PipelineStageUsage::kComputeInput);
                    break;
                case ast::PipelineStage::kNone:
                    break;
            }
        }
    }

    // Resolve the return type
    core::type::Type* return_type = nullptr;
    if (auto ty = decl->return_type) {
        return_type = Type(ty);
        if (!return_type) {
            return nullptr;
        }
    } else {
        return_type = b.create<core::type::Void>();
    }
    func->SetReturnType(return_type);

    if (decl->IsEntryPoint()) {
        // Determine if the return type has a location
        bool permissive = validator_.IsValidationDisabled(
                              decl->attributes, ast::DisabledValidation::kEntryPointParameter) ||
                          validator_.IsValidationDisabled(
                              decl->attributes, ast::DisabledValidation::kFunctionParameter);
        for (auto* attribute : decl->return_type_attributes) {
            Mark(attribute);
            enum Status { kSuccess, kErrored, kInvalid };
            auto res = Switch(
                attribute,  //
                [&](const ast::LocationAttribute* attr) {
                    auto value = LocationAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    func->SetReturnLocation(value.Get());
                    return kSuccess;
                },
                [&](const ast::IndexAttribute* attr) {
                    auto value = IndexAttribute(attr);
                    if (!value) {
                        return kErrored;
                    }
                    func->SetReturnIndex(value.Get());
                    return kSuccess;
                },
                [&](const ast::BuiltinAttribute* attr) {
                    return BuiltinAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InternalAttribute* attr) {
                    return InternalAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InterpolateAttribute* attr) {
                    return InterpolateAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::InvariantAttribute* attr) {
                    return InvariantAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::BindingAttribute* attr) {
                    if (!permissive) {
                        return kInvalid;
                    }
                    return BindingAttribute(attr) ? kSuccess : kErrored;
                },
                [&](const ast::GroupAttribute* attr) {
                    if (!permissive) {
                        return kInvalid;
                    }
                    return GroupAttribute(attr) ? kSuccess : kErrored;
                },
                [&](Default) { return kInvalid; });

            switch (res) {
                case kSuccess:
                    break;
                case kErrored:
                    return nullptr;
                case kInvalid:
                    ErrorInvalidAttribute(attribute, "entry point return types");
                    return nullptr;
            }
        }
    } else {
        for (auto* attribute : decl->return_type_attributes) {
            Mark(attribute);
            bool ok = Switch(attribute,  //
                             [&](Default) {
                                 ErrorInvalidAttribute(attribute,
                                                       "non-entry point function return types");
                                 return false;
                             });
            if (!ok) {
                return nullptr;
            }
        }
    }

    if (auto* str = return_type->As<core::type::Struct>()) {
        if (!ApplyAddressSpaceUsageToType(core::AddressSpace::kUndefined, str, decl->source)) {
            AddNote("while instantiating return type for " + decl->name->symbol.Name(),
                    decl->source);
            return nullptr;
        }

        switch (decl->PipelineStage()) {
            case ast::PipelineStage::kVertex:
                str->AddUsage(core::type::PipelineStageUsage::kVertexOutput);
                break;
            case ast::PipelineStage::kFragment:
                str->AddUsage(core::type::PipelineStageUsage::kFragmentOutput);
                break;
            case ast::PipelineStage::kCompute:
                str->AddUsage(core::type::PipelineStageUsage::kComputeOutput);
                break;
            case ast::PipelineStage::kNone:
                break;
        }
    }

    ApplyDiagnosticSeverities(func);

    if (decl->IsEntryPoint()) {
        entry_points_.Push(func);
    }

    if (decl->body) {
        Mark(decl->body);
        if (TINT_UNLIKELY(current_compound_statement_)) {
            StringStream err;
            err << "Resolver::Function() called with a current compound statement";
            AddICE(err.str(), decl->body->source);
            return nullptr;
        }
        auto* body = StatementScope(decl->body, b.create<sem::FunctionBlockStatement>(func),
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

bool Resolver::Statements(VectorRef<const ast::Statement*> stmts) {
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
        [&](const ast::BlockStatement* s) { return BlockStatement(s); },
        [&](const ast::ForLoopStatement* s) { return ForLoopStatement(s); },
        [&](const ast::LoopStatement* s) { return LoopStatement(s); },
        [&](const ast::WhileStatement* s) { return WhileStatement(s); },
        [&](const ast::IfStatement* s) { return IfStatement(s); },
        [&](const ast::SwitchStatement* s) { return SwitchStatement(s); },

        // Non-Compound statements
        [&](const ast::AssignmentStatement* s) { return AssignmentStatement(s); },
        [&](const ast::BreakStatement* s) { return BreakStatement(s); },
        [&](const ast::BreakIfStatement* s) { return BreakIfStatement(s); },
        [&](const ast::CallStatement* s) { return CallStatement(s); },
        [&](const ast::CompoundAssignmentStatement* s) { return CompoundAssignmentStatement(s); },
        [&](const ast::ContinueStatement* s) { return ContinueStatement(s); },
        [&](const ast::DiscardStatement* s) { return DiscardStatement(s); },
        [&](const ast::IncrementDecrementStatement* s) { return IncrementDecrementStatement(s); },
        [&](const ast::ReturnStatement* s) { return ReturnStatement(s); },
        [&](const ast::VariableDeclStatement* s) { return VariableDeclStatement(s); },
        [&](const ast::ConstAssert* s) { return ConstAssert(s); },

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

sem::CaseStatement* Resolver::CaseStatement(const ast::CaseStatement* stmt,
                                            const core::type::Type* ty) {
    auto* sem = b.create<sem::CaseStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Selectors().reserve(stmt->selectors.Length());
        for (auto* sel : stmt->selectors) {
            Mark(sel);

            ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "case selector"};
            TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

            const core::constant::Value* const_value = nullptr;
            if (!sel->IsDefault()) {
                // The sem statement was created in the switch when attempting to determine the
                // common type.
                auto* materialized = Materialize(sem_.GetVal(sel->expr), ty);
                if (!materialized) {
                    return false;
                }
                if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
                    AddError("case selector must be an i32 or u32 value", sel->source);
                    return false;
                }
                const_value = materialized->ConstantValue();
                if (!const_value) {
                    AddError("case selector must be a constant expression", sel->source);
                    return false;
                }
            }

            sem->Selectors().emplace_back(b.create<sem::CaseSelector>(sel, const_value));
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
    auto* sem = b.create<sem::IfStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* cond = Load(ValueExpression(stmt->condition));
        if (!cond) {
            return false;
        }
        sem->SetCondition(cond);
        sem->Behaviors() = cond->Behaviors();
        sem->Behaviors().Remove(sem::Behavior::kNext);

        Mark(stmt->body);
        auto* body = b.create<sem::BlockStatement>(stmt->body, current_compound_statement_,
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
    auto* sem = b.create<sem::BlockStatement>(stmt->As<ast::BlockStatement>(),
                                              current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] { return Statements(stmt->statements); });
}

sem::LoopStatement* Resolver::LoopStatement(const ast::LoopStatement* stmt) {
    auto* sem = b.create<sem::LoopStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        Mark(stmt->body);

        auto* body = b.create<sem::LoopBlockStatement>(stmt->body, current_compound_statement_,
                                                       current_function_);
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
                    b.create<sem::LoopContinuingBlockStatement>(
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
    auto* sem =
        b.create<sem::ForLoopStatement>(stmt, current_compound_statement_, current_function_);
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
            auto* cond = Load(ValueExpression(cond_expr));
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

        auto* body = b.create<sem::LoopBlockStatement>(stmt->body, current_compound_statement_,
                                                       current_function_);
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
    auto* sem = b.create<sem::WhileStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = sem->Behaviors();

        auto* cond = Load(ValueExpression(stmt->condition));
        if (!cond) {
            return false;
        }
        sem->SetCondition(cond);
        behaviors.Add(cond->Behaviors());

        Mark(stmt->body);

        auto* body = b.create<sem::LoopBlockStatement>(stmt->body, current_compound_statement_,
                                                       current_function_);
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
    Vector<const ast::Expression*, 64> sorted;
    constexpr size_t kMaxExpressionDepth = 512U;
    bool failed = false;
    if (!ast::TraverseExpressions<ast::TraverseOrder::RightToLeft>(
            root, [&](const ast::Expression* expr, size_t depth) {
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
                if (auto* binary = expr->As<ast::BinaryExpression>();
                    binary && binary->IsLogical()) {
                    // Store potential const-eval short-circuit pair
                    logical_binary_lhs_to_parent_.Add(binary->lhs, binary);
                }
                sorted.Push(expr);
                return ast::TraverseAction::Descend;
            })) {
        AddError("TraverseExpressions failed", root->source);
        return nullptr;
    }

    if (failed) {
        return nullptr;
    }

    for (auto* expr : tint::Reverse(sorted)) {
        auto* sem_expr = Switch(
            expr,  //
            [&](const ast::IndexAccessorExpression* array) { return IndexAccessor(array); },
            [&](const ast::BinaryExpression* bin_op) { return Binary(bin_op); },
            [&](const ast::BitcastExpression* bitcast) { return Bitcast(bitcast); },
            [&](const ast::CallExpression* call) { return Call(call); },
            [&](const ast::IdentifierExpression* ident) { return Identifier(ident); },
            [&](const ast::LiteralExpression* literal) { return Literal(literal); },
            [&](const ast::MemberAccessorExpression* member) { return MemberAccessor(member); },
            [&](const ast::UnaryOpExpression* unary) { return UnaryOp(unary); },
            [&](const ast::PhonyExpression*) {
                return b.create<sem::ValueExpression>(expr, b.create<core::type::Void>(),
                                                      core::EvaluationStage::kRuntime,
                                                      current_statement_,
                                                      /* constant_value */ nullptr,
                                                      /* has_side_effects */ false);
            },  //
            TINT_ICE_ON_NO_MATCH);
        if (!sem_expr) {
            return nullptr;
        }

        auto* val = sem_expr->As<sem::ValueExpression>();

        if (val) {
            if (auto* constraint = expr_eval_stage_constraint_.constraint) {
                if (!validator_.EvaluationStage(val, expr_eval_stage_constraint_.stage,
                                                constraint)) {
                    return nullptr;
                }
            }
        }

        b.Sem().Add(expr, sem_expr);
        if (expr == root) {
            return sem_expr;
        }

        // If we just processed the lhs of a constexpr logical binary expression, mark the rhs for
        // short-circuiting.
        if (val && val->ConstantValue()) {
            if (auto binary = logical_binary_lhs_to_parent_.Find(expr)) {
                const bool lhs_is_true = val->ConstantValue()->ValueAs<bool>();
                if (((*binary)->IsLogicalAnd() && !lhs_is_true) ||
                    ((*binary)->IsLogicalOr() && lhs_is_true)) {
                    // Mark entire expression tree to not const-evaluate
                    auto r = ast::TraverseExpressions(  //
                        (*binary)->rhs, [&](const ast::Expression* e) {
                            skip_const_eval_.Add(e);
                            return ast::TraverseAction::Descend;
                        });
                    if (!r) {
                        AddError("TraverseExpressions failed", root->source);
                        return nullptr;
                    }
                }
            }
        }
    }

    AddICE("Expression() did not find root node", root->source);
    return nullptr;
}

sem::ValueExpression* Resolver::ValueExpression(const ast::Expression* expr) {
    return sem_.AsValueExpression(Expression(expr));
}

sem::TypeExpression* Resolver::TypeExpression(const ast::Expression* expr) {
    return sem_.AsTypeExpression(Expression(expr));
}

sem::FunctionExpression* Resolver::FunctionExpression(const ast::Expression* expr) {
    return sem_.AsFunctionExpression(Expression(expr));
}

core::type::Type* Resolver::Type(const ast::Expression* ast) {
    Vector<const sem::GlobalVariable*, 4> referenced_overrides;
    on_transitively_reference_global_.Push([&](const sem::GlobalVariable* ref) {
        if (ref->Declaration()->Is<ast::Override>()) {
            referenced_overrides.Push(ref);
        }
    });
    TINT_DEFER(on_transitively_reference_global_.Pop());

    auto* type_expr = TypeExpression(ast);
    if (TINT_UNLIKELY(!type_expr)) {
        return nullptr;
    }

    auto* type = const_cast<core::type::Type*>(type_expr->Type());
    if (TINT_UNLIKELY(!type)) {
        return nullptr;
    }

    if (auto* arr = type->As<sem::Array>()) {
        for (auto* ref : referenced_overrides) {
            arr->AddTransitivelyReferencedOverride(ref);
        }
    }

    return type;
}

sem::BuiltinEnumExpression<core::AddressSpace>* Resolver::AddressSpaceExpression(
    const ast::Expression* expr) {
    auto address_space_expr = sem_.AsAddressSpace(Expression(expr));
    if (TINT_UNLIKELY(!address_space_expr)) {
        return nullptr;
    }
    if (TINT_UNLIKELY(
            address_space_expr->Value() == core::AddressSpace::kPixelLocal &&
            !enabled_extensions_.Contains(wgsl::Extension::kChromiumExperimentalPixelLocal))) {
        StringStream err;
        err << "'pixel_local' address space requires the '"
            << wgsl::Extension::kChromiumExperimentalPixelLocal << "' extension enabled";
        AddError(err.str(), expr->source);
        return nullptr;
    }
    return address_space_expr;
}

sem::BuiltinEnumExpression<core::BuiltinValue>* Resolver::BuiltinValueExpression(
    const ast::Expression* expr) {
    return sem_.AsBuiltinValue(Expression(expr));
}

sem::BuiltinEnumExpression<core::TexelFormat>* Resolver::TexelFormatExpression(
    const ast::Expression* expr) {
    return sem_.AsTexelFormat(Expression(expr));
}

sem::BuiltinEnumExpression<core::Access>* Resolver::AccessExpression(const ast::Expression* expr) {
    return sem_.AsAccess(Expression(expr));
}

sem::BuiltinEnumExpression<core::InterpolationSampling>* Resolver::InterpolationSampling(
    const ast::Expression* expr) {
    return sem_.AsInterpolationSampling(Expression(expr));
}

sem::BuiltinEnumExpression<core::InterpolationType>* Resolver::InterpolationType(
    const ast::Expression* expr) {
    return sem_.AsInterpolationType(Expression(expr));
}

void Resolver::RegisterStore(const sem::ValueExpression* expr) {
    auto& info = alias_analysis_infos_[current_function_];
    Switch(
        expr->RootIdentifier(),
        [&](const sem::GlobalVariable* global) {
            info.module_scope_writes.insert({global, expr});
        },
        [&](const sem::Parameter* param) { info.parameter_writes.insert(param); });
}

bool Resolver::AliasAnalysis(const sem::Call* call) {
    auto* target = call->Target()->As<sem::Function>();
    if (!target) {
        return true;
    }
    if (validator_.IsValidationDisabled(target->Declaration()->attributes,
                                        ast::DisabledValidation::kIgnorePointerAliasing)) {
        return true;
    }

    // Helper to generate an aliasing error diagnostic.
    struct Alias {
        const sem::ValueExpression* expr;     // the "other expression"
        enum { Argument, ModuleScope } type;  // the type of the "other" expression
        std::string access;                   // the access performed for the "other" expression
    };
    auto make_error = [&](const sem::ValueExpression* arg, Alias&& var) {
        AddError("invalid aliased pointer argument", arg->Declaration()->source);
        switch (var.type) {
            case Alias::Argument:
                AddNote("aliases with another argument passed here",
                        var.expr->Declaration()->source);
                break;
            case Alias::ModuleScope: {
                auto* func = var.expr->Stmt()->Function();
                auto func_name = func->Declaration()->name->symbol.Name();
                AddNote(
                    "aliases with module-scope variable " + var.access + " in '" + func_name + "'",
                    var.expr->Declaration()->source);
                break;
            }
        }
        return false;
    };

    auto& args = call->Arguments();
    auto& target_info = alias_analysis_infos_[target];
    auto& caller_info = alias_analysis_infos_[current_function_];

    // Track the set of root identifiers that are read and written by arguments passed in this
    // call.
    std::unordered_map<const sem::Variable*, const sem::ValueExpression*> arg_reads;
    std::unordered_map<const sem::Variable*, const sem::ValueExpression*> arg_writes;
    for (size_t i = 0; i < args.Length(); i++) {
        auto* arg = args[i];
        if (!arg->Type()->Is<core::type::Pointer>()) {
            continue;
        }

        auto* root = arg->RootIdentifier();
        if (target_info.parameter_writes.count(target->Parameters()[i])) {
            // Arguments that are written to can alias with any other argument or module-scope
            // variable access.
            if (arg_writes.count(root)) {
                return make_error(arg, {arg_writes.at(root), Alias::Argument, "write"});
            }
            if (arg_reads.count(root)) {
                return make_error(arg, {arg_reads.at(root), Alias::Argument, "read"});
            }
            if (target_info.module_scope_reads.count(root)) {
                return make_error(
                    arg, {target_info.module_scope_reads.at(root), Alias::ModuleScope, "read"});
            }
            if (target_info.module_scope_writes.count(root)) {
                return make_error(
                    arg, {target_info.module_scope_writes.at(root), Alias::ModuleScope, "write"});
            }
            arg_writes.insert({root, arg});

            // Propagate the write access to the caller.
            Switch(
                root,
                [&](const sem::GlobalVariable* global) {
                    caller_info.module_scope_writes.insert({global, arg});
                },
                [&](const sem::Parameter* param) { caller_info.parameter_writes.insert(param); });
        } else if (target_info.parameter_reads.count(target->Parameters()[i])) {
            // Arguments that are read from can alias with arguments or module-scope variables
            // that are written to.
            if (arg_writes.count(root)) {
                return make_error(arg, {arg_writes.at(root), Alias::Argument, "write"});
            }
            if (target_info.module_scope_writes.count(root)) {
                return make_error(
                    arg, {target_info.module_scope_writes.at(root), Alias::ModuleScope, "write"});
            }
            arg_reads.insert({root, arg});

            // Propagate the read access to the caller.
            Switch(
                root,
                [&](const sem::GlobalVariable* global) {
                    caller_info.module_scope_reads.insert({global, arg});
                },
                [&](const sem::Parameter* param) { caller_info.parameter_reads.insert(param); });
        }
    }

    // Propagate module-scope variable uses to the caller.
    for (auto read : target_info.module_scope_reads) {
        caller_info.module_scope_reads.insert({read.first, read.second});
    }
    for (auto write : target_info.module_scope_writes) {
        caller_info.module_scope_writes.insert({write.first, write.second});
    }

    return true;
}

const core::type::Type* Resolver::ConcreteType(const core::type::Type* ty,
                                               const core::type::Type* target_ty,
                                               const Source& source) {
    auto i32 = [&] { return b.create<core::type::I32>(); };
    auto f32 = [&] { return b.create<core::type::F32>(); };
    auto i32v = [&](uint32_t width) { return b.create<core::type::Vector>(i32(), width); };
    auto f32v = [&](uint32_t width) { return b.create<core::type::Vector>(f32(), width); };
    auto f32m = [&](uint32_t columns, uint32_t rows) {
        return b.create<core::type::Matrix>(f32v(rows), columns);
    };

    return Switch(
        ty,  //
        [&](const core::type::AbstractInt*) { return target_ty ? target_ty : i32(); },
        [&](const core::type::AbstractFloat*) { return target_ty ? target_ty : f32(); },
        [&](const core::type::Vector* v) {
            return Switch(
                v->type(),  //
                [&](const core::type::AbstractInt*) {
                    return target_ty ? target_ty : i32v(v->Width());
                },
                [&](const core::type::AbstractFloat*) {
                    return target_ty ? target_ty : f32v(v->Width());
                });
        },
        [&](const core::type::Matrix* m) {
            return Switch(m->type(),  //
                          [&](const core::type::AbstractFloat*) {
                              return target_ty ? target_ty : f32m(m->columns(), m->rows());
                          });
        },
        [&](const sem::Array* a) -> const core::type::Type* {
            const core::type::Type* target_el_ty = nullptr;
            if (auto* target_arr_ty = As<sem::Array>(target_ty)) {
                target_el_ty = target_arr_ty->ElemType();
            }
            if (auto* el_ty = ConcreteType(a->ElemType(), target_el_ty, source)) {
                return Array(source, source, source, el_ty, a->Count(), /* explicit_stride */ 0);
            }
            return nullptr;
        },
        [&](const core::type::Struct* s) -> const core::type::Type* {
            if (auto tys = s->ConcreteTypes(); !tys.IsEmpty()) {
                return target_ty ? target_ty : tys[0];
            }
            return nullptr;
        });
}

const sem::ValueExpression* Resolver::Load(const sem::ValueExpression* expr) {
    if (!expr) {
        // Allow for Load(ValueExpression(blah)), where failures pass through Load()
        return nullptr;
    }

    if (!expr->Type()->Is<core::type::Reference>()) {
        // Expression is not a reference type, so cannot be loaded. Just return expr.
        return expr;
    }

    auto* load = b.create<sem::Load>(expr, current_statement_);
    load->Behaviors() = expr->Behaviors();
    b.Sem().Replace(expr->Declaration(), load);

    // Track the load for the alias analysis.
    auto& alias_info = alias_analysis_infos_[current_function_];
    Switch(
        expr->RootIdentifier(),
        [&](const sem::GlobalVariable* global) {
            alias_info.module_scope_reads.insert({global, expr});
        },
        [&](const sem::Parameter* param) { alias_info.parameter_reads.insert(param); });

    return load;
}

const sem::ValueExpression* Resolver::Materialize(
    const sem::ValueExpression* expr,
    const core::type::Type* target_type /* = nullptr */) {
    if (!expr) {
        // Allow for Materialize(ValueExpression(blah)), where failures pass through Materialize()
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

    const core::constant::Value* materialized_val = nullptr;
    if (!skip_const_eval_.Contains(decl)) {
        auto expr_val = expr->ConstantValue();
        if (TINT_UNLIKELY(!expr_val)) {
            StringStream err;
            err << decl->source << "Materialize(" << decl->TypeInfo().name
                << ") called on expression with no constant value";
            AddICE(err.str(), expr->Declaration()->source);
            return nullptr;
        }

        auto val = const_eval_.Convert(concrete_ty, expr_val, decl->source);
        if (!val) {
            // Convert() has already failed and raised an diagnostic error.
            return nullptr;
        }
        materialized_val = val.Get();
        if (TINT_UNLIKELY(!materialized_val)) {
            StringStream err;
            err << decl->source << "ConvertValue(" << expr_val->Type()->FriendlyName() << " -> "
                << concrete_ty->FriendlyName() << ") returned invalid value";
            AddICE(err.str(), expr->Declaration()->source);
            return nullptr;
        }
    }

    auto* m = b.create<sem::Materialize>(expr, current_statement_, concrete_ty, materialized_val);
    m->Behaviors() = expr->Behaviors();
    b.Sem().Replace(decl, m);
    return m;
}

template <size_t N>
bool Resolver::MaybeMaterializeAndLoadArguments(Vector<const sem::ValueExpression*, N>& args,
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
        if (!param_ty->Is<core::type::Reference>()) {
            auto* load = Load(args[i]);
            if (!load) {
                return false;
            }
            args[i] = load;
        }
    }
    return true;
}

bool Resolver::ShouldMaterializeArgument(const core::type::Type* parameter_ty) const {
    const auto* param_el_ty = parameter_ty->DeepestElement();
    return param_el_ty && !param_el_ty->Is<core::type::AbstractNumeric>();
}

bool Resolver::Convert(const core::constant::Value*& c,
                       const core::type::Type* target_ty,
                       const Source& source) {
    auto r = const_eval_.Convert(target_ty, c, source);
    if (!r) {
        return false;
    }
    c = r.Get();
    return true;
}

template <size_t N>
tint::Result<Vector<const core::constant::Value*, N>> Resolver::ConvertArguments(
    const Vector<const sem::ValueExpression*, N>& args,
    const sem::CallTarget* target) {
    auto const_args = tint::Transform(args, [](auto* arg) { return arg->ConstantValue(); });
    for (size_t i = 0, n = std::min(args.Length(), target->Parameters().Length()); i < n; i++) {
        if (!Convert(const_args[i], target->Parameters()[i]->Type(),
                     args[i]->Declaration()->source)) {
            return Failure{};
        }
    }
    return const_args;
}

sem::ValueExpression* Resolver::IndexAccessor(const ast::IndexAccessorExpression* expr) {
    auto* idx = Load(Materialize(sem_.GetVal(expr->index)));
    if (!idx) {
        return nullptr;
    }
    const auto* obj = sem_.GetVal(expr->object);
    if (idx->Stage() != core::EvaluationStage::kConstant) {
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
        [&](const core::type::Vector* vec) { return vec->type(); },
        [&](const core::type::Matrix* mat) {
            return b.create<core::type::Vector>(mat->type(), mat->rows());
        },
        [&](Default) {
            AddError("cannot index type '" + sem_.TypeNameOf(obj_ty) + "'", expr->source);
            return nullptr;
        });
    if (ty == nullptr) {
        return nullptr;
    }

    auto* idx_ty = idx->Type()->UnwrapRef();
    if (!idx_ty->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError("index must be of type 'i32' or 'u32', found: '" + sem_.TypeNameOf(idx_ty) + "'",
                 idx->Declaration()->source);
        return nullptr;
    }

    // If we're extracting from a reference, we return a reference.
    if (auto* ref = obj_raw_ty->As<core::type::Reference>()) {
        ty = b.create<core::type::Reference>(ref->AddressSpace(), ty, ref->Access());
    }

    const core::constant::Value* val = nullptr;
    auto stage = core::EarliestStage(obj->Stage(), idx->Stage());
    if (stage == core::EvaluationStage::kConstant && skip_const_eval_.Contains(expr)) {
        stage = core::EvaluationStage::kNotEvaluated;
    } else {
        if (auto* idx_val = idx->ConstantValue()) {
            auto res = const_eval_.Index(obj->ConstantValue(), obj->Type(), idx_val,
                                         idx->Declaration()->source);
            if (!res) {
                return nullptr;
            }
            val = res.Get();
        }
    }
    bool has_side_effects = idx->HasSideEffects() || obj->HasSideEffects();
    auto* sem = b.create<sem::IndexAccessorExpression>(expr, ty, stage, obj, idx,
                                                       current_statement_, std::move(val),
                                                       has_side_effects, obj->RootIdentifier());
    sem->Behaviors() = idx->Behaviors() + obj->Behaviors();
    return sem;
}

sem::ValueExpression* Resolver::Bitcast(const ast::BitcastExpression* expr) {
    auto* inner = Load(Materialize(sem_.GetVal(expr->expr)));
    if (!inner) {
        return nullptr;
    }
    auto* ty = Type(expr->type);
    if (!ty) {
        return nullptr;
    }
    if (!validator_.Bitcast(expr, ty)) {
        return nullptr;
    }

    auto stage = inner->Stage();
    if (stage == core::EvaluationStage::kConstant && skip_const_eval_.Contains(expr)) {
        stage = core::EvaluationStage::kNotEvaluated;
    }

    const core::constant::Value* value = nullptr;
    if (stage == core::EvaluationStage::kConstant) {
        if (auto r = const_eval_.Bitcast(ty, inner->ConstantValue(), expr->source)) {
            value = r.Get();
        } else {
            return nullptr;
        }
    }

    auto* sem = b.create<sem::ValueExpression>(expr, ty, stage, current_statement_,
                                               std::move(value), inner->HasSideEffects());
    sem->Behaviors() = inner->Behaviors();
    return sem;
}

sem::Call* Resolver::Call(const ast::CallExpression* expr) {
    // A CallExpression can resolve to one of:
    // * A function call.
    // * A builtin call.
    // * A value constructor.
    // * A value conversion.
    auto* target = sem_.Get(expr->target);
    if (TINT_UNLIKELY(!target)) {
        return nullptr;
    }

    // Resolve all of the arguments, their types and the set of behaviors.
    Vector<const sem::ValueExpression*, 8> args;
    args.Reserve(expr->args.Length());
    auto args_stage = core::EvaluationStage::kConstant;
    sem::Behaviors arg_behaviors;
    for (size_t i = 0; i < expr->args.Length(); i++) {
        auto* arg = sem_.GetVal(expr->args[i]);
        if (!arg) {
            return nullptr;
        }
        args.Push(arg);
        args_stage = core::EarliestStage(args_stage, arg->Stage());
        arg_behaviors.Add(arg->Behaviors());
    }
    arg_behaviors.Remove(sem::Behavior::kNext);

    // Did any arguments have side effects?
    bool has_side_effects =
        std::any_of(args.begin(), args.end(), [](auto* e) { return e->HasSideEffects(); });

    // ctor_or_conv is a helper for building either a sem::ValueConstructor or
    // sem::ValueConversion call for a CtorConvIntrinsic with an optional template argument type.
    auto ctor_or_conv = [&](CtorConvIntrinsic ty,
                            const core::type::Type* template_arg) -> sem::Call* {
        auto arg_tys = tint::Transform(args, [](auto* arg) { return arg->Type(); });
        auto match = intrinsic_table_.Lookup(ty, template_arg, arg_tys, args_stage, expr->source);
        if (!match) {
            return nullptr;
        }

        auto overload_stage = match->const_eval_fn ? core::EvaluationStage::kConstant
                                                   : core::EvaluationStage::kRuntime;

        sem::CallTarget* target_sem = nullptr;

        // Is this overload a constructor or conversion?
        if (match->info->flags.Contains(OverloadFlag::kIsConstructor)) {
            // Type constructor
            auto params = Transform(match->parameters, [&](auto& p, size_t i) {
                return b.create<sem::Parameter>(nullptr, static_cast<uint32_t>(i), p.type, p.usage);
            });
            target_sem = constructors_.GetOrCreate(match.Get(), [&] {
                return b.create<sem::ValueConstructor>(match->return_type, std::move(params),
                                                       overload_stage);
            });
        } else {
            // Type conversion
            target_sem = converters_.GetOrCreate(match.Get(), [&] {
                auto* param = b.create<sem::Parameter>(nullptr, 0u, match->parameters[0].type,
                                                       match->parameters[0].usage);
                return b.create<sem::ValueConversion>(match->return_type, param, overload_stage);
            });
        }

        if (!MaybeMaterializeAndLoadArguments(args, target_sem)) {
            return nullptr;
        }

        const core::constant::Value* value = nullptr;
        auto stage = core::EarliestStage(overload_stage, args_stage);
        if (stage == core::EvaluationStage::kConstant && skip_const_eval_.Contains(expr)) {
            stage = core::EvaluationStage::kNotEvaluated;
        }
        if (stage == core::EvaluationStage::kConstant) {
            auto const_args = ConvertArguments(args, target_sem);
            if (!const_args) {
                return nullptr;
            }
            auto const_eval_fn = match->const_eval_fn;
            if (auto r = (const_eval_.*const_eval_fn)(target_sem->ReturnType(), const_args.Get(),
                                                      expr->source)) {
                value = r.Get();
            } else {
                return nullptr;
            }
        }
        return b.create<sem::Call>(expr, target_sem, stage, std::move(args), current_statement_,
                                   value, has_side_effects);
    };

    // arr_or_str_init is a helper for building a sem::ValueConstructor for an array or structure
    // constructor call target.
    auto arr_or_str_init = [&](const core::type::Type* ty,
                               const sem::CallTarget* call_target) -> sem::Call* {
        auto stage = args_stage;                       // The evaluation stage of the call
        const core::constant::Value* value = nullptr;  // The constant value for the call
        if (stage == core::EvaluationStage::kConstant && skip_const_eval_.Contains(expr)) {
            stage = core::EvaluationStage::kNotEvaluated;
        }
        if (stage == core::EvaluationStage::kConstant) {
            auto const_args = ConvertArguments(args, call_target);
            if (!const_args) {
                return nullptr;
            }
            if (auto r = const_eval_.ArrayOrStructCtor(ty, std::move(const_args.Get()))) {
                value = r.Get();
            } else {
                return nullptr;
            }
            if (!value) {
                // Constant evaluation failed.
                // Can happen for expressions that will fail validation (later).
                // Use the kRuntime EvaluationStage, as kConstant will trigger an assertion in
                // the sem::ValueExpression constructor, which checks that kConstant is paired
                // with a constant value.
                stage = core::EvaluationStage::kRuntime;
            }
        }

        return b.create<sem::Call>(expr, call_target, stage, std::move(args), current_statement_,
                                   value, has_side_effects);
    };

    auto ty_init_or_conv = [&](const core::type::Type* type) {
        return Switch(
            type,  //
            [&](const core::type::I32*) { return ctor_or_conv(CtorConvIntrinsic::kI32, nullptr); },
            [&](const core::type::U32*) { return ctor_or_conv(CtorConvIntrinsic::kU32, nullptr); },
            [&](const core::type::F16*) {
                return validator_.CheckF16Enabled(expr->source)
                           ? ctor_or_conv(CtorConvIntrinsic::kF16, nullptr)
                           : nullptr;
            },
            [&](const core::type::F32*) { return ctor_or_conv(CtorConvIntrinsic::kF32, nullptr); },
            [&](const core::type::Bool*) {
                return ctor_or_conv(CtorConvIntrinsic::kBool, nullptr);
            },
            [&](const core::type::Vector* v) {
                if (v->Packed()) {
                    TINT_ASSERT(v->Width() == 3u);
                    return ctor_or_conv(CtorConvIntrinsic::kPackedVec3, v->type());
                }
                return ctor_or_conv(wgsl::intrinsic::VectorCtorConv(v->Width()), v->type());
            },
            [&](const core::type::Matrix* m) {
                return ctor_or_conv(wgsl::intrinsic::MatrixCtorConv(m->columns(), m->rows()),
                                    m->type());
            },
            [&](const sem::Array* arr) -> sem::Call* {
                auto* call_target = array_ctors_.GetOrCreate(
                    ArrayConstructorSig{{arr, args.Length(), args_stage}},
                    [&]() -> sem::ValueConstructor* {
                        auto params = tint::Transform(args, [&](auto, size_t i) {
                            return b.create<sem::Parameter>(nullptr,  // declaration
                                                            static_cast<uint32_t>(i),  // index
                                                            arr->ElemType());
                        });
                        return b.create<sem::ValueConstructor>(arr, std::move(params), args_stage);
                    });

                if (TINT_UNLIKELY(!MaybeMaterializeAndLoadArguments(args, call_target))) {
                    return nullptr;
                }

                if (TINT_UNLIKELY(!validator_.ArrayConstructor(expr, arr))) {
                    return nullptr;
                }

                return arr_or_str_init(arr, call_target);
            },
            [&](const core::type::Struct* str) -> sem::Call* {
                auto* call_target = struct_ctors_.GetOrCreate(
                    StructConstructorSig{{str, args.Length(), args_stage}},
                    [&]() -> sem::ValueConstructor* {
                        Vector<sem::Parameter*, 8> params;
                        params.Resize(std::min(args.Length(), str->Members().Length()));
                        for (size_t i = 0, n = params.Length(); i < n; i++) {
                            params[i] =
                                b.create<sem::Parameter>(nullptr,                     // declaration
                                                         static_cast<uint32_t>(i),    // index
                                                         str->Members()[i]->Type());  // type
                        }
                        return b.create<sem::ValueConstructor>(str, std::move(params), args_stage);
                    });

                if (TINT_UNLIKELY(!MaybeMaterializeAndLoadArguments(args, call_target))) {
                    return nullptr;
                }

                if (TINT_UNLIKELY(!validator_.StructureInitializer(expr, str))) {
                    return nullptr;
                }

                return arr_or_str_init(str, call_target);
            },
            [&](Default) {
                AddError("type is not constructible", expr->source);
                return nullptr;
            });
    };

    auto incomplete_type = [&](const IncompleteType* t) -> sem::Call* {
        // A type without template arguments.
        // Examples: vec3(...), array(...)
        switch (t->builtin) {
            case core::BuiltinType::kVec2:
                return ctor_or_conv(CtorConvIntrinsic::kVec2, nullptr);
            case core::BuiltinType::kVec3:
                return ctor_or_conv(CtorConvIntrinsic::kVec3, nullptr);
            case core::BuiltinType::kVec4:
                return ctor_or_conv(CtorConvIntrinsic::kVec4, nullptr);
            case core::BuiltinType::kMat2X2:
                return ctor_or_conv(CtorConvIntrinsic::kMat2x2, nullptr);
            case core::BuiltinType::kMat2X3:
                return ctor_or_conv(CtorConvIntrinsic::kMat2x3, nullptr);
            case core::BuiltinType::kMat2X4:
                return ctor_or_conv(CtorConvIntrinsic::kMat2x4, nullptr);
            case core::BuiltinType::kMat3X2:
                return ctor_or_conv(CtorConvIntrinsic::kMat3x2, nullptr);
            case core::BuiltinType::kMat3X3:
                return ctor_or_conv(CtorConvIntrinsic::kMat3x3, nullptr);
            case core::BuiltinType::kMat3X4:
                return ctor_or_conv(CtorConvIntrinsic::kMat3x4, nullptr);
            case core::BuiltinType::kMat4X2:
                return ctor_or_conv(CtorConvIntrinsic::kMat4x2, nullptr);
            case core::BuiltinType::kMat4X3:
                return ctor_or_conv(CtorConvIntrinsic::kMat4x3, nullptr);
            case core::BuiltinType::kMat4X4:
                return ctor_or_conv(CtorConvIntrinsic::kMat4x4, nullptr);
            case core::BuiltinType::kArray: {
                auto el_count =
                    b.create<core::type::ConstantArrayCount>(static_cast<uint32_t>(args.Length()));
                auto arg_tys =
                    tint::Transform(args, [](auto* arg) { return arg->Type()->UnwrapRef(); });
                auto el_ty = core::type::Type::Common(arg_tys);
                if (TINT_UNLIKELY(!el_ty)) {
                    AddError("cannot infer common array element type from constructor arguments",
                             expr->source);
                    Hashset<const core::type::Type*, 8> types;
                    for (size_t i = 0; i < args.Length(); i++) {
                        if (types.Add(args[i]->Type())) {
                            AddNote("argument " + std::to_string(i) + " is of type '" +
                                        sem_.TypeNameOf(args[i]->Type()) + "'",
                                    args[i]->Declaration()->source);
                        }
                    }
                    return nullptr;
                }
                auto* arr = Array(expr->source, expr->source, expr->source, el_ty, el_count,
                                  /* explicit_stride */ 0);
                if (TINT_UNLIKELY(!arr)) {
                    return nullptr;
                }
                return ty_init_or_conv(arr);
            }
            default: {
                TINT_ICE() << "unhandled IncompleteType builtin: " << t->builtin;
                return nullptr;
            }
        }
    };

    auto* call = Switch(
        target,  //
        [&](const sem::FunctionExpression* fn_expr) {
            return FunctionCall(expr, const_cast<sem::Function*>(fn_expr->Function()),
                                std::move(args), arg_behaviors);
        },
        [&](const sem::TypeExpression* ty_expr) {
            return Switch(
                ty_expr->Type(),  //
                [&](const IncompleteType* t) -> sem::Call* {
                    auto* ctor = incomplete_type(t);
                    if (TINT_UNLIKELY(!ctor)) {
                        return nullptr;
                    }
                    // Replace incomplete type with resolved type
                    const_cast<sem::TypeExpression*>(ty_expr)->SetType(ctor->Type());
                    return ctor;
                },
                [&](Default) { return ty_init_or_conv(ty_expr->Type()); });
        },
        [&](const sem::BuiltinEnumExpression<wgsl::BuiltinFn>* fn_expr) {
            return BuiltinCall(expr, fn_expr->Value(), args);
        },
        [&](Default) {
            sem_.ErrorUnexpectedExprKind(target, "call target");
            return nullptr;
        });

    if (!call) {
        return nullptr;
    }

    return validator_.Call(call, current_statement_) ? call : nullptr;
}

template <size_t N>
sem::Call* Resolver::BuiltinCall(const ast::CallExpression* expr,
                                 wgsl::BuiltinFn fn,
                                 Vector<const sem::ValueExpression*, N>& args) {
    auto arg_stage = core::EvaluationStage::kConstant;
    for (auto* arg : args) {
        arg_stage = core::EarliestStage(arg_stage, arg->Stage());
    }

    auto arg_tys = tint::Transform(args, [](auto* arg) { return arg->Type(); });
    auto overload = intrinsic_table_.Lookup(fn, arg_tys, arg_stage, expr->source);
    if (!overload) {
        return nullptr;
    }

    // De-duplicate builtins that are identical.
    auto* target = builtins_.GetOrCreate(std::make_pair(overload.Get(), fn), [&] {
        auto params = Transform(overload->parameters, [&](auto& p, size_t i) {
            return b.create<sem::Parameter>(nullptr, static_cast<uint32_t>(i), p.type, p.usage);
        });
        sem::PipelineStageSet supported_stages;
        auto flags = overload->info->flags;
        if (flags.Contains(OverloadFlag::kSupportsVertexPipeline)) {
            supported_stages.Add(ast::PipelineStage::kVertex);
        }
        if (flags.Contains(OverloadFlag::kSupportsFragmentPipeline)) {
            supported_stages.Add(ast::PipelineStage::kFragment);
        }
        if (flags.Contains(OverloadFlag::kSupportsComputePipeline)) {
            supported_stages.Add(ast::PipelineStage::kCompute);
        }
        auto eval_stage = overload->const_eval_fn ? core::EvaluationStage::kConstant
                                                  : core::EvaluationStage::kRuntime;
        return b.create<sem::BuiltinFn>(
            fn, overload->return_type, std::move(params), eval_stage, supported_stages,
            flags.Contains(OverloadFlag::kIsDeprecated), flags.Contains(OverloadFlag::kMustUse));
    });

    if (fn == wgsl::BuiltinFn::kTintMaterialize) {
        args[0] = Materialize(args[0]);
        if (!args[0]) {
            return nullptr;
        }
    } else {
        // Materialize arguments if the parameter type is not abstract
        if (!MaybeMaterializeAndLoadArguments(args, target)) {
            return nullptr;
        }
    }

    if (target->IsDeprecated()) {
        AddWarning("use of deprecated builtin", expr->source);
    }

    // If the builtin is @const, and all arguments have constant values, evaluate the builtin
    // now.
    const core::constant::Value* value = nullptr;
    auto stage = core::EarliestStage(arg_stage, target->Stage());
    if (stage == core::EvaluationStage::kConstant && skip_const_eval_.Contains(expr)) {
        stage = core::EvaluationStage::kNotEvaluated;
    }
    if (stage == core::EvaluationStage::kConstant) {
        auto const_args = ConvertArguments(args, target);
        if (!const_args) {
            return nullptr;
        }
        auto const_eval_fn = overload->const_eval_fn;
        if (auto r = (const_eval_.*const_eval_fn)(target->ReturnType(), const_args.Get(),
                                                  expr->source)) {
            value = r.Get();
        } else {
            return nullptr;
        }
    }

    bool has_side_effects =
        target->HasSideEffects() ||
        std::any_of(args.begin(), args.end(), [](auto* e) { return e->HasSideEffects(); });
    auto* call = b.create<sem::Call>(expr, target, stage, std::move(args), current_statement_,
                                     value, has_side_effects);

    if (current_function_) {
        current_function_->AddDirectlyCalledBuiltin(target);
        current_function_->AddDirectCall(call);
    }

    if (!validator_.RequiredExtensionForBuiltinFn(call)) {
        return nullptr;
    }

    if (IsTexture(fn)) {
        if (!validator_.TextureBuiltinFn(call)) {
            return nullptr;
        }
        CollectTextureSamplerPairs(target, call->Arguments());
    }

    if (fn == wgsl::BuiltinFn::kWorkgroupUniformLoad) {
        if (!validator_.WorkgroupUniformLoad(call)) {
            return nullptr;
        }
    }

    if (fn == wgsl::BuiltinFn::kSubgroupBroadcast) {
        if (!validator_.SubgroupBroadcast(call)) {
            return nullptr;
        }
    }

    if (!validator_.BuiltinCall(call)) {
        return nullptr;
    }

    return call;
}

core::type::Type* Resolver::BuiltinType(core::BuiltinType builtin_ty,
                                        const ast::Identifier* ident) {
    auto check_no_tmpl_args = [&](core::type::Type* ty) -> core::type::Type* {
        return TINT_LIKELY(CheckNotTemplated("type", ident)) ? ty : nullptr;
    };

    switch (builtin_ty) {
        case core::BuiltinType::kBool:
            return check_no_tmpl_args(b.create<core::type::Bool>());
        case core::BuiltinType::kI32:
            return check_no_tmpl_args(I32());
        case core::BuiltinType::kU32:
            return check_no_tmpl_args(U32());
        case core::BuiltinType::kF16:
            return check_no_tmpl_args(F16(ident));
        case core::BuiltinType::kF32:
            return check_no_tmpl_args(b.create<core::type::F32>());
        case core::BuiltinType::kVec2:
            return VecT(ident, builtin_ty, 2);
        case core::BuiltinType::kVec3:
            return VecT(ident, builtin_ty, 3);
        case core::BuiltinType::kVec4:
            return VecT(ident, builtin_ty, 4);
        case core::BuiltinType::kMat2X2:
            return MatT(ident, builtin_ty, 2, 2);
        case core::BuiltinType::kMat2X3:
            return MatT(ident, builtin_ty, 2, 3);
        case core::BuiltinType::kMat2X4:
            return MatT(ident, builtin_ty, 2, 4);
        case core::BuiltinType::kMat3X2:
            return MatT(ident, builtin_ty, 3, 2);
        case core::BuiltinType::kMat3X3:
            return MatT(ident, builtin_ty, 3, 3);
        case core::BuiltinType::kMat3X4:
            return MatT(ident, builtin_ty, 3, 4);
        case core::BuiltinType::kMat4X2:
            return MatT(ident, builtin_ty, 4, 2);
        case core::BuiltinType::kMat4X3:
            return MatT(ident, builtin_ty, 4, 3);
        case core::BuiltinType::kMat4X4:
            return MatT(ident, builtin_ty, 4, 4);
        case core::BuiltinType::kMat2X2F:
            return check_no_tmpl_args(Mat(ident, F32(), 2u, 2u));
        case core::BuiltinType::kMat2X3F:
            return check_no_tmpl_args(Mat(ident, F32(), 2u, 3u));
        case core::BuiltinType::kMat2X4F:
            return check_no_tmpl_args(Mat(ident, F32(), 2u, 4u));
        case core::BuiltinType::kMat3X2F:
            return check_no_tmpl_args(Mat(ident, F32(), 3u, 2u));
        case core::BuiltinType::kMat3X3F:
            return check_no_tmpl_args(Mat(ident, F32(), 3u, 3u));
        case core::BuiltinType::kMat3X4F:
            return check_no_tmpl_args(Mat(ident, F32(), 3u, 4u));
        case core::BuiltinType::kMat4X2F:
            return check_no_tmpl_args(Mat(ident, F32(), 4u, 2u));
        case core::BuiltinType::kMat4X3F:
            return check_no_tmpl_args(Mat(ident, F32(), 4u, 3u));
        case core::BuiltinType::kMat4X4F:
            return check_no_tmpl_args(Mat(ident, F32(), 4u, 4u));
        case core::BuiltinType::kMat2X2H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 2u, 2u));
        case core::BuiltinType::kMat2X3H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 2u, 3u));
        case core::BuiltinType::kMat2X4H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 2u, 4u));
        case core::BuiltinType::kMat3X2H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 3u, 2u));
        case core::BuiltinType::kMat3X3H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 3u, 3u));
        case core::BuiltinType::kMat3X4H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 3u, 4u));
        case core::BuiltinType::kMat4X2H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 4u, 2u));
        case core::BuiltinType::kMat4X3H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 4u, 3u));
        case core::BuiltinType::kMat4X4H:
            return check_no_tmpl_args(Mat(ident, F16(ident), 4u, 4u));
        case core::BuiltinType::kVec2F:
            return check_no_tmpl_args(Vec(ident, F32(), 2u));
        case core::BuiltinType::kVec3F:
            return check_no_tmpl_args(Vec(ident, F32(), 3u));
        case core::BuiltinType::kVec4F:
            return check_no_tmpl_args(Vec(ident, F32(), 4u));
        case core::BuiltinType::kVec2H:
            return check_no_tmpl_args(Vec(ident, F16(ident), 2u));
        case core::BuiltinType::kVec3H:
            return check_no_tmpl_args(Vec(ident, F16(ident), 3u));
        case core::BuiltinType::kVec4H:
            return check_no_tmpl_args(Vec(ident, F16(ident), 4u));
        case core::BuiltinType::kVec2I:
            return check_no_tmpl_args(Vec(ident, I32(), 2u));
        case core::BuiltinType::kVec3I:
            return check_no_tmpl_args(Vec(ident, I32(), 3u));
        case core::BuiltinType::kVec4I:
            return check_no_tmpl_args(Vec(ident, I32(), 4u));
        case core::BuiltinType::kVec2U:
            return check_no_tmpl_args(Vec(ident, U32(), 2u));
        case core::BuiltinType::kVec3U:
            return check_no_tmpl_args(Vec(ident, U32(), 3u));
        case core::BuiltinType::kVec4U:
            return check_no_tmpl_args(Vec(ident, U32(), 4u));
        case core::BuiltinType::kArray:
            return Array(ident);
        case core::BuiltinType::kAtomic:
            return Atomic(ident);
        case core::BuiltinType::kPtr:
            return Ptr(ident);
        case core::BuiltinType::kSampler:
            return check_no_tmpl_args(
                b.create<core::type::Sampler>(core::type::SamplerKind::kSampler));
        case core::BuiltinType::kSamplerComparison:
            return check_no_tmpl_args(
                b.create<core::type::Sampler>(core::type::SamplerKind::kComparisonSampler));
        case core::BuiltinType::kTexture1D:
            return SampledTexture(ident, core::type::TextureDimension::k1d);
        case core::BuiltinType::kTexture2D:
            return SampledTexture(ident, core::type::TextureDimension::k2d);
        case core::BuiltinType::kTexture2DArray:
            return SampledTexture(ident, core::type::TextureDimension::k2dArray);
        case core::BuiltinType::kTexture3D:
            return SampledTexture(ident, core::type::TextureDimension::k3d);
        case core::BuiltinType::kTextureCube:
            return SampledTexture(ident, core::type::TextureDimension::kCube);
        case core::BuiltinType::kTextureCubeArray:
            return SampledTexture(ident, core::type::TextureDimension::kCubeArray);
        case core::BuiltinType::kTextureDepth2D:
            return check_no_tmpl_args(
                b.create<core::type::DepthTexture>(core::type::TextureDimension::k2d));
        case core::BuiltinType::kTextureDepth2DArray:
            return check_no_tmpl_args(
                b.create<core::type::DepthTexture>(core::type::TextureDimension::k2dArray));
        case core::BuiltinType::kTextureDepthCube:
            return check_no_tmpl_args(
                b.create<core::type::DepthTexture>(core::type::TextureDimension::kCube));
        case core::BuiltinType::kTextureDepthCubeArray:
            return check_no_tmpl_args(
                b.create<core::type::DepthTexture>(core::type::TextureDimension::kCubeArray));
        case core::BuiltinType::kTextureDepthMultisampled2D:
            return check_no_tmpl_args(
                b.create<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d));
        case core::BuiltinType::kTextureExternal:
            return check_no_tmpl_args(b.create<core::type::ExternalTexture>());
        case core::BuiltinType::kTextureMultisampled2D:
            return MultisampledTexture(ident, core::type::TextureDimension::k2d);
        case core::BuiltinType::kTextureStorage1D:
            return StorageTexture(ident, core::type::TextureDimension::k1d);
        case core::BuiltinType::kTextureStorage2D:
            return StorageTexture(ident, core::type::TextureDimension::k2d);
        case core::BuiltinType::kTextureStorage2DArray:
            return StorageTexture(ident, core::type::TextureDimension::k2dArray);
        case core::BuiltinType::kTextureStorage3D:
            return StorageTexture(ident, core::type::TextureDimension::k3d);
        case core::BuiltinType::kPackedVec3:
            return PackedVec3T(ident);
        case core::BuiltinType::kAtomicCompareExchangeResultI32:
            return core::type::CreateAtomicCompareExchangeResult(b.Types(), b.Symbols(), I32());
        case core::BuiltinType::kAtomicCompareExchangeResultU32:
            return core::type::CreateAtomicCompareExchangeResult(b.Types(), b.Symbols(), U32());
        case core::BuiltinType::kFrexpResultAbstract:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), AF());
        case core::BuiltinType::kFrexpResultF16:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), F16(ident));
        case core::BuiltinType::kFrexpResultF32:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), F32());
        case core::BuiltinType::kFrexpResultVec2Abstract:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, AF(), 2));
        case core::BuiltinType::kFrexpResultVec2F16:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 2));
        case core::BuiltinType::kFrexpResultVec2F32:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F32(), 2));
        case core::BuiltinType::kFrexpResultVec3Abstract:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, AF(), 3));
        case core::BuiltinType::kFrexpResultVec3F16:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 3));
        case core::BuiltinType::kFrexpResultVec3F32:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F32(), 3));
        case core::BuiltinType::kFrexpResultVec4Abstract:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, AF(), 4));
        case core::BuiltinType::kFrexpResultVec4F16:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 4));
        case core::BuiltinType::kFrexpResultVec4F32:
            return core::type::CreateFrexpResult(b.Types(), b.Symbols(), Vec(ident, F32(), 4));
        case core::BuiltinType::kModfResultAbstract:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), AF());
        case core::BuiltinType::kModfResultF16:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), F16(ident));
        case core::BuiltinType::kModfResultF32:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), F32());
        case core::BuiltinType::kModfResultVec2Abstract:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, AF(), 2));
        case core::BuiltinType::kModfResultVec2F16:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 2));
        case core::BuiltinType::kModfResultVec2F32:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F32(), 2));
        case core::BuiltinType::kModfResultVec3Abstract:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, AF(), 3));
        case core::BuiltinType::kModfResultVec3F16:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 3));
        case core::BuiltinType::kModfResultVec3F32:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F32(), 3));
        case core::BuiltinType::kModfResultVec4Abstract:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, AF(), 4));
        case core::BuiltinType::kModfResultVec4F16:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F16(ident), 4));
        case core::BuiltinType::kModfResultVec4F32:
            return core::type::CreateModfResult(b.Types(), b.Symbols(), Vec(ident, F32(), 4));
        case core::BuiltinType::kUndefined:
            break;
    }

    auto name = ident->symbol.Name();
    StringStream err;
    err << " unhandled builtin type '" << name << "'";
    AddICE(err.str(), ident->source);
    return nullptr;
}

core::type::AbstractFloat* Resolver::AF() {
    return b.create<core::type::AbstractFloat>();
}

core::type::F32* Resolver::F32() {
    return b.create<core::type::F32>();
}

core::type::I32* Resolver::I32() {
    return b.create<core::type::I32>();
}

core::type::U32* Resolver::U32() {
    return b.create<core::type::U32>();
}

core::type::F16* Resolver::F16(const ast::Identifier* ident) {
    return validator_.CheckF16Enabled(ident->source) ? b.create<core::type::F16>() : nullptr;
}

core::type::Vector* Resolver::Vec(const ast::Identifier* ident, core::type::Type* el, uint32_t n) {
    if (TINT_UNLIKELY(!el)) {
        return nullptr;
    }
    if (TINT_UNLIKELY(!validator_.Vector(el, ident->source))) {
        return nullptr;
    }
    return b.create<core::type::Vector>(el, n);
}

core::type::Type* Resolver::VecT(const ast::Identifier* ident,
                                 core::BuiltinType builtin,
                                 uint32_t n) {
    auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>();
    if (!tmpl_ident) {
        // 'vecN' has no template arguments, so return an incomplete type.
        return b.create<IncompleteType>(builtin);
    }

    if (TINT_UNLIKELY(!CheckTemplatedIdentifierArgs(tmpl_ident, 1))) {
        return nullptr;
    }

    auto* ty = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!ty)) {
        return nullptr;
    }

    return Vec(ident, const_cast<core::type::Type*>(ty), n);
}

core::type::Matrix* Resolver::Mat(const ast::Identifier* ident,
                                  core::type::Type* el,
                                  uint32_t num_columns,
                                  uint32_t num_rows) {
    if (TINT_UNLIKELY(!el)) {
        return nullptr;
    }
    if (TINT_UNLIKELY(!validator_.Matrix(el, ident->source))) {
        return nullptr;
    }
    auto* column = Vec(ident, el, num_rows);
    if (!column) {
        return nullptr;
    }
    return b.create<core::type::Matrix>(column, num_columns);
}

core::type::Type* Resolver::MatT(const ast::Identifier* ident,
                                 core::BuiltinType builtin,
                                 uint32_t num_columns,
                                 uint32_t num_rows) {
    auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>();
    if (!tmpl_ident) {
        // 'vecN' has no template arguments, so return an incomplete type.
        return b.create<IncompleteType>(builtin);
    }

    if (TINT_UNLIKELY(!CheckTemplatedIdentifierArgs(tmpl_ident, 1))) {
        return nullptr;
    }

    auto* el_ty = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!el_ty)) {
        return nullptr;
    }

    return Mat(ident, const_cast<core::type::Type*>(el_ty), num_columns, num_rows);
}

core::type::Type* Resolver::Array(const ast::Identifier* ident) {
    auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>();
    if (!tmpl_ident) {
        // 'array' has no template arguments, so return an incomplete type.
        return b.create<IncompleteType>(core::BuiltinType::kArray);
    }

    if (TINT_UNLIKELY(!CheckTemplatedIdentifierArgs(tmpl_ident, 1, 2))) {
        return nullptr;
    }
    auto* ast_el_ty = tmpl_ident->arguments[0];
    auto* ast_count = (tmpl_ident->arguments.Length() > 1) ? tmpl_ident->arguments[1] : nullptr;

    auto* el_ty = sem_.GetType(ast_el_ty);
    if (!el_ty) {
        return nullptr;
    }

    const core::type::ArrayCount* el_count =
        ast_count ? ArrayCount(ast_count) : b.create<core::type::RuntimeArrayCount>();
    if (!el_count) {
        return nullptr;
    }

    // Look for explicit stride via @stride(n) attribute
    uint32_t explicit_stride = 0;
    if (!ArrayAttributes(tmpl_ident->attributes, el_ty, explicit_stride)) {
        return nullptr;
    }

    auto* out = Array(tmpl_ident->source,                             //
                      ast_el_ty->source,                              //
                      ast_count ? ast_count->source : ident->source,  //
                      el_ty, el_count, explicit_stride);
    if (!out) {
        return nullptr;
    }

    if (el_ty->Is<core::type::Atomic>()) {
        atomic_composite_info_.Add(out, &ast_el_ty->source);
    } else {
        if (auto found = atomic_composite_info_.Get(el_ty)) {
            atomic_composite_info_.Add(out, *found);
        }
    }

    return out;
}

core::type::Atomic* Resolver::Atomic(const ast::Identifier* ident) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 1);  // atomic<type>
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto* el_ty = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!el_ty)) {
        return nullptr;
    }

    auto* out = b.create<core::type::Atomic>(el_ty);
    if (TINT_UNLIKELY(!validator_.Atomic(tmpl_ident, out))) {
        return nullptr;
    }
    return out;
}

core::type::Pointer* Resolver::Ptr(const ast::Identifier* ident) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 2, 3);  // ptr<address, type [, access]>
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto address_space = sem_.GetAddressSpace(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(address_space == core::AddressSpace::kUndefined)) {
        return nullptr;
    }

    auto* store_ty = const_cast<core::type::Type*>(sem_.GetType(tmpl_ident->arguments[1]));
    if (TINT_UNLIKELY(!store_ty)) {
        return nullptr;
    }

    core::Access access = core::Access::kUndefined;
    if (tmpl_ident->arguments.Length() > 2) {
        access = sem_.GetAccess(tmpl_ident->arguments[2]);
        if (TINT_UNLIKELY(access == core::Access::kUndefined)) {
            return nullptr;
        }
    } else {
        access = DefaultAccessForAddressSpace(address_space);
    }

    auto* out = b.create<core::type::Pointer>(address_space, store_ty, access);
    if (TINT_UNLIKELY(!validator_.Pointer(tmpl_ident, out))) {
        return nullptr;
    }

    if (!ApplyAddressSpaceUsageToType(address_space, store_ty, tmpl_ident->arguments[1]->source)) {
        AddNote("while instantiating " + out->FriendlyName(), ident->source);
        return nullptr;
    }
    return out;
}

core::type::SampledTexture* Resolver::SampledTexture(const ast::Identifier* ident,
                                                     core::type::TextureDimension dim) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 1);
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto* ty_expr = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!ty_expr)) {
        return nullptr;
    }

    auto* out = b.create<core::type::SampledTexture>(dim, ty_expr);
    return validator_.SampledTexture(out, ident->source) ? out : nullptr;
}

core::type::MultisampledTexture* Resolver::MultisampledTexture(const ast::Identifier* ident,
                                                               core::type::TextureDimension dim) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 1);
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto* ty_expr = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!ty_expr)) {
        return nullptr;
    }

    auto* out = b.create<core::type::MultisampledTexture>(dim, ty_expr);
    return validator_.MultisampledTexture(out, ident->source) ? out : nullptr;
}

core::type::StorageTexture* Resolver::StorageTexture(const ast::Identifier* ident,
                                                     core::type::TextureDimension dim) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 2);
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto format = sem_.GetTexelFormat(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(format == core::TexelFormat::kUndefined)) {
        return nullptr;
    }

    auto access = sem_.GetAccess(tmpl_ident->arguments[1]);
    if (TINT_UNLIKELY(access == core::Access::kUndefined)) {
        return nullptr;
    }

    auto* subtype = core::type::StorageTexture::SubtypeFor(format, b.Types());
    auto* tex = b.create<core::type::StorageTexture>(dim, format, access, subtype);
    if (!validator_.StorageTexture(tex, ident->source)) {
        return nullptr;
    }

    return tex;
}

core::type::Vector* Resolver::PackedVec3T(const ast::Identifier* ident) {
    auto* tmpl_ident = TemplatedIdentifier(ident, 1);
    if (TINT_UNLIKELY(!tmpl_ident)) {
        return nullptr;
    }

    auto* el_ty = sem_.GetType(tmpl_ident->arguments[0]);
    if (TINT_UNLIKELY(!el_ty)) {
        return nullptr;
    }

    if (TINT_UNLIKELY(!validator_.Vector(el_ty, ident->source))) {
        return nullptr;
    }
    return b.create<core::type::Vector>(el_ty, 3u, true);
}

const ast::TemplatedIdentifier* Resolver::TemplatedIdentifier(const ast::Identifier* ident,
                                                              size_t min_args,
                                                              size_t max_args /* = use min 0 */) {
    auto* tmpl_ident = ident->As<ast::TemplatedIdentifier>();
    if (!tmpl_ident) {
        if (TINT_UNLIKELY(min_args != 0)) {
            AddError("expected '<' for '" + ident->symbol.Name() + "'",
                     Source{ident->source.range.end});
        }
        return nullptr;
    }
    return CheckTemplatedIdentifierArgs(tmpl_ident, min_args, max_args) ? tmpl_ident : nullptr;
}

bool Resolver::CheckTemplatedIdentifierArgs(const ast::TemplatedIdentifier* ident,
                                            size_t min_args,
                                            size_t max_args /* = use min 0 */) {
    if (max_args == 0) {
        max_args = min_args;
    }
    if (min_args == max_args) {
        if (TINT_UNLIKELY(ident->arguments.Length() != min_args)) {
            AddError("'" + ident->symbol.Name() + "' requires " + std::to_string(min_args) +
                         " template arguments",
                     ident->source);
            return false;
        }
    } else {
        if (TINT_UNLIKELY(ident->arguments.Length() < min_args)) {
            AddError("'" + ident->symbol.Name() + "' requires at least " +
                         std::to_string(min_args) + " template arguments",
                     ident->source);
            return false;
        }
        if (TINT_UNLIKELY(ident->arguments.Length() > max_args)) {
            AddError("'" + ident->symbol.Name() + "' requires at most " + std::to_string(max_args) +
                         " template arguments",
                     ident->source);
            return false;
        }
    }
    return ident;
}

size_t Resolver::NestDepth(const core::type::Type* ty) const {
    return Switch(
        ty,  //
        [](const core::type::Vector*) { return size_t{1}; },
        [](const core::type::Matrix*) { return size_t{2}; },
        [&](Default) {
            if (auto d = nest_depth_.Get(ty)) {
                return *d;
            }
            return size_t{0};
        });
}

void Resolver::CollectTextureSamplerPairs(const sem::BuiltinFn* builtin,
                                          VectorRef<const sem::ValueExpression*> args) const {
    // Collect a texture/sampler pair for this builtin.
    const auto& signature = builtin->Signature();
    int texture_index = signature.IndexOf(core::ParameterUsage::kTexture);
    if (TINT_UNLIKELY(texture_index == -1)) {
        StringStream err;
        err << "texture builtin without texture parameter";
        AddICE(err.str(), {});
        return;
    }
    if (auto* user =
            args[static_cast<size_t>(texture_index)]->UnwrapLoad()->As<sem::VariableUser>()) {
        auto* texture = user->Variable();
        if (!texture->Type()->UnwrapRef()->Is<core::type::StorageTexture>()) {
            int sampler_index = signature.IndexOf(core::ParameterUsage::kSampler);
            const sem::Variable* sampler = sampler_index != -1
                                               ? args[static_cast<size_t>(sampler_index)]
                                                     ->UnwrapLoad()
                                                     ->As<sem::VariableUser>()
                                                     ->Variable()
                                               : nullptr;
            current_function_->AddTextureSamplerPair(texture, sampler);
        }
    }
}

sem::Call* Resolver::FunctionCall(const ast::CallExpression* expr,
                                  sem::Function* target,
                                  VectorRef<const sem::ValueExpression*> args_in,
                                  sem::Behaviors arg_behaviors) {
    Vector<const sem::ValueExpression*, 8> args = std::move(args_in);
    if (!MaybeMaterializeAndLoadArguments(args, target)) {
        return nullptr;
    }

    // TODO(crbug.com/tint/1420): For now, assume all function calls have side effects.
    bool has_side_effects = true;
    auto* call = b.create<sem::Call>(expr, target, core::EvaluationStage::kRuntime, std::move(args),
                                     current_statement_,
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

        if (!AliasAnalysis(call)) {
            return nullptr;
        }

        // Note: Validation *must* be performed before calling this method.
        CollectTextureSamplerPairs(target, call->Arguments());
    }

    return call;
}

void Resolver::CollectTextureSamplerPairs(sem::Function* func,
                                          VectorRef<const sem::ValueExpression*> args) const {
    // Map all texture/sampler pairs from the target function to the
    // current function. These can only be global or parameter
    // variables. Resolve any parameter variables to the corresponding
    // argument passed to the current function. Leave global variables
    // as-is. Then add the mapped pair to the current function's list of
    // texture/sampler pairs.

    Hashset<const sem::Variable*, 4> texture_sampler_set;

    for (sem::VariablePair pair : func->TextureSamplerPairs()) {
        const sem::Variable* texture = pair.first;
        const sem::Variable* sampler = pair.second;
        if (auto* param = As<sem::Parameter>(texture)) {
            texture = args[param->Index()]->UnwrapLoad()->As<sem::VariableUser>()->Variable();
            texture_sampler_set.Add(texture);
        }
        if (auto* param = As<sem::Parameter>(sampler)) {
            sampler = args[param->Index()]->UnwrapLoad()->As<sem::VariableUser>()->Variable();
            texture_sampler_set.Add(sampler);
        }
        current_function_->AddTextureSamplerPair(texture, sampler);
    }

    // Add any possible texture/sampler not essentially passed to builtins from the function param.
    // This could be unused texture/sampler or texture/sampler passed to builtins that are emulated.

    const auto& signature = func->Signature();

    for (size_t i = 0; i < signature.parameters.Length(); i++) {
        auto* param = signature.parameters[i];
        if (param->Type()->Is<core::type::Texture>()) {
            auto* user = args[i]->UnwrapLoad()->As<sem::VariableUser>();
            auto* texture = user->Variable();
            if (!texture_sampler_set.Contains(texture)) {
                current_function_->AddTextureSamplerPair(texture, nullptr);
                func->AddTextureSamplerPair(texture, nullptr);
                texture_sampler_set.Add(texture);
            }
        } else if (param->Type()->Is<core::type::Sampler>()) {
            auto* user = args[i]->UnwrapLoad()->As<sem::VariableUser>();
            auto* sampler = user->Variable();
            if (!texture_sampler_set.Contains(sampler)) {
                current_function_->AddTextureSamplerPair(nullptr, sampler);
                func->AddTextureSamplerPair(nullptr, sampler);
                texture_sampler_set.Add(sampler);
            }
        }
    }
}

sem::ValueExpression* Resolver::Literal(const ast::LiteralExpression* literal) {
    auto* ty = Switch(
        literal,
        [&](const ast::IntLiteralExpression* i) -> core::type::Type* {
            switch (i->suffix) {
                case ast::IntLiteralExpression::Suffix::kNone:
                    return b.create<core::type::AbstractInt>();
                case ast::IntLiteralExpression::Suffix::kI:
                    return b.create<core::type::I32>();
                case ast::IntLiteralExpression::Suffix::kU:
                    return b.create<core::type::U32>();
            }
            TINT_UNREACHABLE() << "Unhandled integer literal suffix: " << i->suffix;
            return nullptr;
        },
        [&](const ast::FloatLiteralExpression* f) -> core::type::Type* {
            switch (f->suffix) {
                case ast::FloatLiteralExpression::Suffix::kNone:
                    return b.create<core::type::AbstractFloat>();
                case ast::FloatLiteralExpression::Suffix::kF:
                    return b.create<core::type::F32>();
                case ast::FloatLiteralExpression::Suffix::kH:
                    return validator_.CheckF16Enabled(literal->source) ? b.create<core::type::F16>()
                                                                       : nullptr;
            }
            TINT_UNREACHABLE() << "Unhandled float literal suffix: " << f->suffix;
            return nullptr;
        },
        [&](const ast::BoolLiteralExpression*) { return b.create<core::type::Bool>(); },  //
        TINT_ICE_ON_NO_MATCH);

    if (ty == nullptr) {
        return nullptr;
    }

    const core::constant::Value* val = nullptr;
    auto stage = core::EvaluationStage::kConstant;
    if (skip_const_eval_.Contains(literal)) {
        stage = core::EvaluationStage::kNotEvaluated;
    }
    if (stage == core::EvaluationStage::kConstant) {
        val = Switch(
            literal,
            [&](const ast::BoolLiteralExpression* lit) { return b.constants.Get(lit->value); },
            [&](const ast::IntLiteralExpression* lit) -> const core::constant::Value* {
                switch (lit->suffix) {
                    case ast::IntLiteralExpression::Suffix::kNone:
                        return b.constants.Get(AInt(lit->value));
                    case ast::IntLiteralExpression::Suffix::kI:
                        return b.constants.Get(i32(lit->value));
                    case ast::IntLiteralExpression::Suffix::kU:
                        return b.constants.Get(u32(lit->value));
                }
                return nullptr;
            },
            [&](const ast::FloatLiteralExpression* lit) -> const core::constant::Value* {
                switch (lit->suffix) {
                    case ast::FloatLiteralExpression::Suffix::kNone:
                        return b.constants.Get(AFloat(lit->value));
                    case ast::FloatLiteralExpression::Suffix::kF:
                        return b.constants.Get(f32(lit->value));
                    case ast::FloatLiteralExpression::Suffix::kH:
                        return b.constants.Get(f16(lit->value));
                }
                return nullptr;
            });
    }
    return b.create<sem::ValueExpression>(literal, ty, stage, current_statement_, std::move(val),
                                          /* has_side_effects */ false);
}

sem::Expression* Resolver::Identifier(const ast::IdentifierExpression* expr) {
    auto* ident = expr->identifier;
    Mark(ident);

    auto resolved = dependencies_.resolved_identifiers.Get(ident);
    if (!resolved) {
        StringStream err;
        err << "identifier '" << ident->symbol.Name() << "' was not resolved";
        AddICE(err.str(), expr->source);
        return nullptr;
    }

    if (auto* ast_node = resolved->Node()) {
        auto* resolved_node = sem_.Get(ast_node);
        return Switch(
            resolved_node,  //
            [&](sem::Variable* variable) -> sem::VariableUser* {
                if (!TINT_LIKELY(CheckNotTemplated("variable", ident))) {
                    return nullptr;
                }

                auto stage = variable->Stage();
                const core::constant::Value* value = variable->ConstantValue();
                if (skip_const_eval_.Contains(expr)) {
                    // This expression is short-circuited by an ancestor expression.
                    // Do not const-eval.
                    stage = core::EvaluationStage::kNotEvaluated;
                    value = nullptr;
                }
                auto* user =
                    b.create<sem::VariableUser>(expr, stage, current_statement_, value, variable);

                if (current_statement_) {
                    // If identifier is part of a loop continuing block, make sure it
                    // doesn't refer to a variable that is bypassed by a continue statement
                    // in the loop's body block.
                    if (auto* continuing_block =
                            current_statement_
                                ->FindFirstParent<sem::LoopContinuingBlockStatement>()) {
                        auto* loop_block =
                            continuing_block->FindFirstParent<sem::LoopBlockStatement>();
                        if (loop_block->FirstContinue()) {
                            // If our identifier is in loop_block->decls, make sure its index is
                            // less than first_continue
                            auto symbol = ident->symbol;
                            if (auto decl = loop_block->Decls().Find(symbol)) {
                                if (decl->order >= loop_block->NumDeclsAtFirstContinue()) {
                                    AddError("continue statement bypasses declaration of '" +
                                                 symbol.Name() + "'",
                                             loop_block->FirstContinue()->source);
                                    AddNote("identifier '" + symbol.Name() + "' declared here",
                                            decl->variable->Declaration()->source);
                                    AddNote("identifier '" + symbol.Name() +
                                                "' referenced in continuing block here",
                                            expr->source);
                                    return nullptr;
                                }
                            }
                        }
                    }
                }

                if (auto* global = variable->As<sem::GlobalVariable>()) {
                    for (auto& fn : on_transitively_reference_global_) {
                        fn(global);
                    }
                    if (!current_function_ && variable->Declaration()->Is<ast::Var>()) {
                        // Use of a module-scope 'var' outside of a function.
                        std::string desc = "var '" + ident->symbol.Name() + "' ";
                        AddError(desc + "cannot be referenced at module-scope", expr->source);
                        AddNote(desc + "declared here", variable->Declaration()->source);
                        return nullptr;
                    }
                }

                variable->AddUser(user);
                return user;
            },
            [&](const core::type::Type* ty) -> sem::TypeExpression* {
                // User declared types cannot be templated.
                if (!TINT_LIKELY(CheckNotTemplated("type", ident))) {
                    return nullptr;
                }

                // Notify callers of all transitively referenced globals.
                if (auto* arr = ty->As<sem::Array>()) {
                    for (auto& fn : on_transitively_reference_global_) {
                        for (auto* ref : arr->TransitivelyReferencedOverrides()) {
                            fn(ref);
                        }
                    }
                }

                return b.create<sem::TypeExpression>(expr, current_statement_, ty);
            },
            [&](const sem::Function* fn) -> sem::FunctionExpression* {
                if (!TINT_LIKELY(CheckNotTemplated("function", ident))) {
                    return nullptr;
                }
                return b.create<sem::FunctionExpression>(expr, current_statement_, fn);
            });
    }

    if (auto builtin_ty = resolved->BuiltinType(); builtin_ty != core::BuiltinType::kUndefined) {
        auto* ty = BuiltinType(builtin_ty, ident);
        if (!ty) {
            return nullptr;
        }
        return b.create<sem::TypeExpression>(expr, current_statement_, ty);
    }

    if (auto fn = resolved->BuiltinFn(); fn != wgsl::BuiltinFn::kNone) {
        return CheckNotTemplated("builtin function", ident)
                   ? b.create<sem::BuiltinEnumExpression<wgsl::BuiltinFn>>(expr, current_statement_,
                                                                           fn)
                   : nullptr;
    }

    if (auto access = resolved->Access(); access != core::Access::kUndefined) {
        return CheckNotTemplated("access", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::Access>>(expr, current_statement_,
                                                                        access)
                   : nullptr;
    }

    if (auto addr = resolved->AddressSpace(); addr != core::AddressSpace::kUndefined) {
        return CheckNotTemplated("address space", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::AddressSpace>>(
                         expr, current_statement_, addr)
                   : nullptr;
    }

    if (auto builtin = resolved->BuiltinValue(); builtin != core::BuiltinValue::kUndefined) {
        return CheckNotTemplated("builtin value", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::BuiltinValue>>(
                         expr, current_statement_, builtin)
                   : nullptr;
    }

    if (auto i_smpl = resolved->InterpolationSampling();
        i_smpl != core::InterpolationSampling::kUndefined) {
        return CheckNotTemplated("interpolation sampling", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::InterpolationSampling>>(
                         expr, current_statement_, i_smpl)
                   : nullptr;
    }

    if (auto i_type = resolved->InterpolationType();
        i_type != core::InterpolationType::kUndefined) {
        return CheckNotTemplated("interpolation type", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::InterpolationType>>(
                         expr, current_statement_, i_type)
                   : nullptr;
    }

    if (auto fmt = resolved->TexelFormat(); fmt != core::TexelFormat::kUndefined) {
        return CheckNotTemplated("texel format", ident)
                   ? b.create<sem::BuiltinEnumExpression<core::TexelFormat>>(
                         expr, current_statement_, fmt)
                   : nullptr;
    }

    if (resolved->Unresolved()) {
        return b.create<UnresolvedIdentifier>(expr, current_statement_);
    }

    TINT_UNREACHABLE() << "unhandled resolved identifier: " << resolved->String();
    return nullptr;
}

sem::ValueExpression* Resolver::MemberAccessor(const ast::MemberAccessorExpression* expr) {
    auto* object = sem_.GetVal(expr->object);
    if (!object) {
        return nullptr;
    }

    auto* object_ty = object->Type();
    auto* storage_ty = object_ty->UnwrapRef();

    auto* root_ident = object->RootIdentifier();

    const core::type::Type* ty = nullptr;

    // Object may be a side-effecting expression (e.g. function call).
    bool has_side_effects = object->HasSideEffects();

    Mark(expr->member);

    return Switch(
        storage_ty,  //
        [&](const core::type::Struct* str) -> sem::ValueExpression* {
            auto symbol = expr->member->symbol;

            const core::type::StructMember* member = nullptr;
            for (auto* m : str->Members()) {
                if (m->Name() == symbol) {
                    member = m;
                    break;
                }
            }

            if (member == nullptr) {
                AddError("struct member " + symbol.Name() + " not found", expr->source);
                return nullptr;
            }

            ty = member->Type();

            // If we're extracting from a reference, we return a reference.
            if (auto* ref = object_ty->As<core::type::Reference>()) {
                ty = b.create<core::type::Reference>(ref->AddressSpace(), ty, ref->Access());
            }

            const core::constant::Value* val = nullptr;
            if (auto* obj_val = object->ConstantValue()) {
                val = obj_val->Index(static_cast<size_t>(member->Index()));
            }
            return b.create<sem::StructMemberAccess>(expr, ty, current_statement_, val, object,
                                                     member, has_side_effects, root_ident);
        },

        [&](const core::type::Vector* vec) -> sem::ValueExpression* {
            std::string s = expr->member->symbol.Name();
            auto size = s.size();
            Vector<uint32_t, 4> swizzle;
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

            const sem::ValueExpression* obj_expr = object;
            if (size == 1) {
                // A single element swizzle is just the type of the vector.
                ty = vec->type();
                // If we're extracting from a reference, we return a reference.
                if (auto* ref = object_ty->As<core::type::Reference>()) {
                    ty = b.create<core::type::Reference>(ref->AddressSpace(), ty, ref->Access());
                }
            } else {
                // The vector will have a number of components equal to the length of
                // the swizzle.
                ty = b.create<core::type::Vector>(vec->type(), static_cast<uint32_t>(size));

                // The load rule is invoked before the swizzle, if necessary.
                obj_expr = Load(object);
            }
            const core::constant::Value* val = nullptr;
            if (auto* obj_val = object->ConstantValue()) {
                auto res = const_eval_.Swizzle(ty, obj_val, swizzle);
                if (!res) {
                    return nullptr;
                }
                val = res.Get();
            }
            return b.create<sem::Swizzle>(expr, ty, current_statement_, val, obj_expr,
                                          std::move(swizzle), has_side_effects, root_ident);
        },

        [&](Default) {
            AddError("invalid member accessor expression. Expected vector or struct, got '" +
                         sem_.TypeNameOf(storage_ty) + "'",
                     expr->member->source);
            return nullptr;
        });
}

sem::ValueExpression* Resolver::Binary(const ast::BinaryExpression* expr) {
    const auto* lhs = sem_.GetVal(expr->lhs);
    const auto* rhs = sem_.GetVal(expr->rhs);
    if (!lhs || !rhs) {
        return nullptr;
    }

    // Load arguments if they are references
    lhs = Load(lhs);
    if (!lhs) {
        return nullptr;
    }
    rhs = Load(rhs);
    if (!rhs) {
        return nullptr;
    }

    auto stage = core::EarliestStage(lhs->Stage(), rhs->Stage());
    auto overload =
        intrinsic_table_.Lookup(expr->op, lhs->Type(), rhs->Type(), stage, expr->source, false);
    if (!overload) {
        return nullptr;
    }

    auto* res_ty = overload->return_type;

    // Parameter types
    auto* lhs_ty = overload->parameters[0].type;
    auto* rhs_ty = overload->parameters[1].type;
    if (ShouldMaterializeArgument(lhs_ty)) {
        lhs = Materialize(lhs, lhs_ty);
        if (!lhs) {
            return nullptr;
        }
    }
    if (ShouldMaterializeArgument(rhs_ty)) {
        rhs = Materialize(rhs, rhs_ty);
        if (!rhs) {
            return nullptr;
        }
    }

    const core::constant::Value* value = nullptr;
    if (skip_const_eval_.Contains(expr)) {
        // This expression is short-circuited by an ancestor expression.
        // Do not const-eval.
        stage = core::EvaluationStage::kNotEvaluated;
    } else if (lhs->Stage() == core::EvaluationStage::kConstant &&
               rhs->Stage() == core::EvaluationStage::kNotEvaluated) {
        // Short-circuiting binary expression. Use the LHS value and stage.
        value = lhs->ConstantValue();
        stage = core::EvaluationStage::kConstant;
    } else if (stage == core::EvaluationStage::kConstant) {
        // Both LHS and RHS have expressions that are constant evaluation stage.
        auto const_eval_fn = overload->const_eval_fn;
        if (const_eval_fn) {  // Do we have a @const operator?
            // Yes. Perform any required abstract argument values implicit conversions to the
            // overload parameter types, and const-eval.
            Vector const_args{lhs->ConstantValue(), rhs->ConstantValue()};
            // Implicit conversion (e.g. AInt -> AFloat)
            if (!Convert(const_args[0], lhs_ty, lhs->Declaration()->source)) {
                return nullptr;
            }
            if (!Convert(const_args[1], rhs_ty, rhs->Declaration()->source)) {
                return nullptr;
            }
            if (auto r = (const_eval_.*const_eval_fn)(res_ty, const_args, expr->source)) {
                value = r.Get();
            } else {
                return nullptr;
            }
        } else {
            // The arguments have constant values, but the operator cannot be const-evaluated.
            // This can only be evaluated at runtime.
            stage = core::EvaluationStage::kRuntime;
        }
    }

    bool has_side_effects = lhs->HasSideEffects() || rhs->HasSideEffects();
    auto* sem = b.create<sem::ValueExpression>(expr, res_ty, stage, current_statement_, value,
                                               has_side_effects);
    sem->Behaviors() = lhs->Behaviors() + rhs->Behaviors();

    return sem;
}

sem::ValueExpression* Resolver::UnaryOp(const ast::UnaryOpExpression* unary) {
    const auto* expr = sem_.GetVal(unary->expr);
    if (!expr) {
        return nullptr;
    }
    auto* expr_ty = expr->Type();

    const core::type::Type* ty = nullptr;
    const sem::Variable* root_ident = nullptr;
    const core::constant::Value* value = nullptr;
    auto stage = core::EvaluationStage::kRuntime;

    switch (unary->op) {
        case core::UnaryOp::kAddressOf:
            if (auto* ref = expr_ty->As<core::type::Reference>()) {
                if (ref->StoreType()->UnwrapRef()->is_handle()) {
                    AddError("cannot take the address of expression in handle address space",
                             unary->expr->source);
                    return nullptr;
                }

                auto* array = unary->expr->As<ast::IndexAccessorExpression>();
                auto* member = unary->expr->As<ast::MemberAccessorExpression>();
                if ((array && sem_.TypeOf(array->object)->UnwrapRef()->Is<core::type::Vector>()) ||
                    (member &&
                     sem_.TypeOf(member->object)->UnwrapRef()->Is<core::type::Vector>())) {
                    AddError("cannot take the address of a vector component", unary->expr->source);
                    return nullptr;
                }

                ty = b.create<core::type::Pointer>(ref->AddressSpace(), ref->StoreType(),
                                                   ref->Access());

                root_ident = expr->RootIdentifier();
            } else {
                AddError("cannot take the address of expression", unary->expr->source);
                return nullptr;
            }
            break;

        case core::UnaryOp::kIndirection:
            if (auto* ptr = expr_ty->As<core::type::Pointer>()) {
                ty = b.create<core::type::Reference>(ptr->AddressSpace(), ptr->StoreType(),
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
            auto overload = intrinsic_table_.Lookup(unary->op, expr_ty, stage, unary->source);
            if (!overload) {
                return nullptr;
            }
            ty = overload->return_type;
            auto* param_ty = overload->parameters[0].type;
            if (ShouldMaterializeArgument(param_ty)) {
                expr = Materialize(expr, param_ty);
                if (!expr) {
                    return nullptr;
                }
            }

            // Load expr if it is a reference
            expr = Load(expr);
            if (!expr) {
                return nullptr;
            }

            stage = expr->Stage();
            if (stage == core::EvaluationStage::kConstant) {
                if (auto const_eval_fn = overload->const_eval_fn) {
                    if (auto r = (const_eval_.*const_eval_fn)(ty, Vector{expr->ConstantValue()},
                                                              expr->Declaration()->source)) {
                        value = r.Get();
                    } else {
                        return nullptr;
                    }
                } else {
                    stage = core::EvaluationStage::kRuntime;
                }
            }
            break;
        }
    }

    auto* sem = b.create<sem::ValueExpression>(unary, ty, stage, current_statement_, value,
                                               expr->HasSideEffects(), root_ident);
    sem->Behaviors() = expr->Behaviors();
    return sem;
}

tint::Result<uint32_t> Resolver::LocationAttribute(const ast::LocationAttribute* attr) {
    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@location value"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

    auto* materialized = Materialize(ValueExpression(attr->expr));
    if (!materialized) {
        return Failure{};
    }

    if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError("@location must be an i32 or u32 value", attr->source);
        return Failure{};
    }

    auto const_value = materialized->ConstantValue();
    auto value = const_value->ValueAs<AInt>();
    if (value < 0) {
        AddError("@location value must be non-negative", attr->source);
        return Failure{};
    }

    return static_cast<uint32_t>(value);
}

tint::Result<uint32_t> Resolver::IndexAttribute(const ast::IndexAttribute* attr) {
    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@index value"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

    auto* materialized = Materialize(ValueExpression(attr->expr));
    if (!materialized) {
        return Failure{};
    }

    if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError("@location must be an i32 or u32 value", attr->source);
        return Failure{};
    }

    auto const_value = materialized->ConstantValue();
    auto value = const_value->ValueAs<AInt>();
    if (value != 0 && value != 1) {
        AddError("@index value must be zero or one", attr->source);
        return Failure{};
    }

    return static_cast<uint32_t>(value);
}

tint::Result<uint32_t> Resolver::BindingAttribute(const ast::BindingAttribute* attr) {
    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@binding"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

    auto* materialized = Materialize(ValueExpression(attr->expr));
    if (!materialized) {
        return Failure{};
    }
    if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError("@binding must be an i32 or u32 value", attr->source);
        return Failure{};
    }

    auto const_value = materialized->ConstantValue();
    auto value = const_value->ValueAs<AInt>();
    if (value < 0) {
        AddError("@binding value must be non-negative", attr->source);
        return Failure{};
    }
    return static_cast<uint32_t>(value);
}

tint::Result<uint32_t> Resolver::GroupAttribute(const ast::GroupAttribute* attr) {
    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@group"};
    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

    auto* materialized = Materialize(ValueExpression(attr->expr));
    if (!materialized) {
        return Failure{};
    }
    if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
        AddError("@group must be an i32 or u32 value", attr->source);
        return Failure{};
    }

    auto const_value = materialized->ConstantValue();
    auto value = const_value->ValueAs<AInt>();
    if (value < 0) {
        AddError("@group value must be non-negative", attr->source);
        return Failure{};
    }
    return static_cast<uint32_t>(value);
}

tint::Result<sem::WorkgroupSize> Resolver::WorkgroupAttribute(const ast::WorkgroupAttribute* attr) {
    // Set work-group size defaults.
    sem::WorkgroupSize ws;
    for (size_t i = 0; i < 3; i++) {
        ws[i] = 1;
    }

    auto values = attr->Values();
    Vector<const sem::ValueExpression*, 3> args;
    Vector<const core::type::Type*, 3> arg_tys;

    constexpr const char* kErrBadExpr =
        "workgroup_size argument must be a constant or override-expression of type "
        "abstract-integer, i32 or u32";

    for (size_t i = 0; i < 3; i++) {
        // Each argument to this attribute can either be a literal, an identifier for a
        // module-scope constants, a const-expression, or nullptr if not specified.
        auto* value = values[i];
        if (!value) {
            break;
        }
        const auto* expr = ValueExpression(value);
        if (!expr) {
            return Failure{};
        }
        auto* ty = expr->Type();
        if (!ty->IsAnyOf<core::type::I32, core::type::U32, core::type::AbstractInt>()) {
            AddError(kErrBadExpr, value->source);
            return Failure{};
        }

        if (expr->Stage() != core::EvaluationStage::kConstant &&
            expr->Stage() != core::EvaluationStage::kOverride) {
            AddError(kErrBadExpr, value->source);
            return Failure{};
        }

        args.Push(expr);
        arg_tys.Push(ty);
    }

    auto* common_ty = core::type::Type::Common(arg_tys);
    if (!common_ty) {
        AddError("workgroup_size arguments must be of the same type, either i32 or u32",
                 attr->source);
        return Failure{};
    }

    // If all arguments are abstract-integers, then materialize to i32.
    if (common_ty->Is<core::type::AbstractInt>()) {
        common_ty = b.create<core::type::I32>();
    }

    for (size_t i = 0; i < args.Length(); i++) {
        auto* materialized = Materialize(args[i], common_ty);
        if (!materialized) {
            return Failure{};
        }
        if (auto* value = materialized->ConstantValue()) {
            if (value->ValueAs<AInt>() < 1) {
                AddError("workgroup_size argument must be at least 1", values[i]->source);
                return Failure{};
            }
            ws[i] = value->ValueAs<u32>();
        } else {
            ws[i] = std::nullopt;
        }
    }

    uint64_t total_size = static_cast<uint64_t>(ws[0].value_or(1));
    for (size_t i = 1; i < 3; i++) {
        total_size *= static_cast<uint64_t>(ws[i].value_or(1));
        if (total_size > 0xffffffff) {
            AddError("total workgroup grid size cannot exceed 0xffffffff", values[i]->source);
            return Failure{};
        }
    }

    return ws;
}

tint::Result<tint::core::BuiltinValue> Resolver::BuiltinAttribute(
    const ast::BuiltinAttribute* attr) {
    auto* builtin_expr = BuiltinValueExpression(attr->builtin);
    if (!builtin_expr) {
        return Failure{};
    }
    // Apply the resolved tint::sem::BuiltinEnumExpression<tint::core::BuiltinValue> to the
    // attribute.
    b.Sem().Add(attr, builtin_expr);
    return builtin_expr->Value();
}

bool Resolver::DiagnosticAttribute(const ast::DiagnosticAttribute* attr) {
    return DiagnosticControl(attr->control);
}

bool Resolver::StageAttribute(const ast::StageAttribute*) {
    return true;
}

bool Resolver::MustUseAttribute(const ast::MustUseAttribute*) {
    return true;
}

bool Resolver::InvariantAttribute(const ast::InvariantAttribute*) {
    return true;
}

bool Resolver::StrideAttribute(const ast::StrideAttribute*) {
    return true;
}

tint::Result<core::Interpolation> Resolver::InterpolateAttribute(
    const ast::InterpolateAttribute* attr) {
    core::Interpolation out;
    auto* type = InterpolationType(attr->type);
    if (!type) {
        return Failure{};
    }
    out.type = type->Value();
    if (attr->sampling) {
        auto* sampling = InterpolationSampling(attr->sampling);
        if (!sampling) {
            return Failure{};
        }
        out.sampling = sampling->Value();
    }
    return out;
}

bool Resolver::InternalAttribute(const ast::InternalAttribute* attr) {
    for (auto* dep : attr->dependencies) {
        if (!Expression(dep)) {
            return false;
        }
    }
    return true;
}

bool Resolver::DiagnosticControl(const ast::DiagnosticControl& control) {
    Mark(control.rule_name);
    Mark(control.rule_name->name);
    auto name = control.rule_name->name->symbol.Name();

    if (control.rule_name->category) {
        Mark(control.rule_name->category);
        if (control.rule_name->category->symbol.Name() == "chromium") {
            auto rule = wgsl::ParseChromiumDiagnosticRule(name);
            if (rule != wgsl::ChromiumDiagnosticRule::kUndefined) {
                validator_.DiagnosticFilters().Set(rule, control.severity);
            } else {
                StringStream ss;
                ss << "unrecognized diagnostic rule 'chromium." << name << "'\n";
                tint::SuggestAlternativeOptions opts;
                opts.prefix = "chromium.";
                tint::SuggestAlternatives(name, wgsl::kChromiumDiagnosticRuleStrings, ss, opts);
                AddWarning(ss.str(), control.rule_name->source);
            }
        }
        return true;
    }

    auto rule = wgsl::ParseCoreDiagnosticRule(name);
    if (rule != wgsl::CoreDiagnosticRule::kUndefined) {
        validator_.DiagnosticFilters().Set(rule, control.severity);
    } else {
        StringStream ss;
        ss << "unrecognized diagnostic rule '" << name << "'\n";
        tint::SuggestAlternatives(name, wgsl::kCoreDiagnosticRuleStrings, ss);
        AddWarning(ss.str(), control.rule_name->source);
    }
    return true;
}

bool Resolver::Enable(const ast::Enable* enable) {
    for (auto* ext : enable->extensions) {
        Mark(ext);
        enabled_extensions_.Add(ext->name);
    }
    return true;
}

core::type::Type* Resolver::TypeDecl(const ast::TypeDecl* named_type) {
    Mark(named_type->name);

    core::type::Type* result = nullptr;
    if (auto* alias = named_type->As<ast::Alias>()) {
        result = Alias(alias);
    } else if (auto* str = named_type->As<ast::Struct>()) {
        result = Structure(str);
    } else {
        TINT_UNREACHABLE() << "Unhandled TypeDecl";
    }

    if (!result) {
        return nullptr;
    }

    b.Sem().Add(named_type, result);
    return result;
}

const core::type::ArrayCount* Resolver::ArrayCount(const ast::Expression* count_expr) {
    // Evaluate the constant array count expression.
    const auto* count_sem = Materialize(sem_.GetVal(count_expr));
    if (!count_sem) {
        return nullptr;
    }

    switch (count_sem->Stage()) {
        case core::EvaluationStage::kNotEvaluated:
            // Happens in expressions like:
            //    false && array<T, N>()[i]
            // The end result will not be used, so just make N=1.
            return b.create<core::type::ConstantArrayCount>(static_cast<uint32_t>(1));

        case core::EvaluationStage::kOverride: {
            // array count is an override expression.
            // Is the count a named 'override'?
            if (auto* user = count_sem->UnwrapMaterialize()->As<sem::VariableUser>()) {
                if (auto* global = user->Variable()->As<sem::GlobalVariable>()) {
                    return b.create<sem::NamedOverrideArrayCount>(global);
                }
            }
            return b.create<sem::UnnamedOverrideArrayCount>(count_sem);
        }

        case core::EvaluationStage::kConstant: {
            auto* count_val = count_sem->ConstantValue();
            if (auto* ty = count_val->Type(); !ty->is_integer_scalar()) {
                AddError(
                    "array count must evaluate to a constant integer expression, but is type '" +
                        ty->FriendlyName() + "'",
                    count_expr->source);
                return nullptr;
            }

            int64_t count = count_val->ValueAs<AInt>();
            if (count < 1) {
                AddError("array count (" + std::to_string(count) + ") must be greater than 0",
                         count_expr->source);
                return nullptr;
            }

            return b.create<core::type::ConstantArrayCount>(static_cast<uint32_t>(count));
        }

        default: {
            AddError(
                "array count must evaluate to a constant integer expression or override variable",
                count_expr->source);
            return nullptr;
        }
    }
}

bool Resolver::ArrayAttributes(VectorRef<const ast::Attribute*> attributes,
                               const core::type::Type* el_ty,
                               uint32_t& explicit_stride) {
    if (!validator_.NoDuplicateAttributes(attributes)) {
        return false;
    }

    for (auto* attribute : attributes) {
        Mark(attribute);
        bool ok = Switch(
            attribute,  //
            [&](const ast::StrideAttribute* attr) {
                // If the element type is not plain, then el_ty->Align() may be 0, in which case we
                // could get a DBZ in ArrayStrideAttribute(). In this case, validation will error
                // about the invalid array element type (which is tested later), so this is just a
                // seatbelt.
                if (IsPlain(el_ty)) {
                    explicit_stride = attr->stride;
                    if (!validator_.ArrayStrideAttribute(attr, el_ty->Size(), el_ty->Align())) {
                        return false;
                    }
                }
                return true;
            },
            [&](Default) {
                ErrorInvalidAttribute(attribute, "array types");
                return false;
            });
        if (!ok) {
            return false;
        }
    }

    return true;
}

sem::Array* Resolver::Array(const Source& array_source,
                            const Source& el_source,
                            const Source& count_source,
                            const core::type::Type* el_ty,
                            const core::type::ArrayCount* el_count,
                            uint32_t explicit_stride) {
    uint32_t el_align = el_ty->Align();
    uint32_t el_size = el_ty->Size();
    uint64_t implicit_stride = el_size ? tint::RoundUp<uint64_t>(el_align, el_size) : 0;
    uint64_t stride = explicit_stride ? explicit_stride : implicit_stride;
    uint64_t size = 0;

    if (auto const_count = el_count->As<core::type::ConstantArrayCount>()) {
        size = const_count->value * stride;
        if (size > std::numeric_limits<uint32_t>::max()) {
            StringStream msg;
            msg << "array byte size (0x" << std::hex << size
                << ") must not exceed 0xffffffff bytes";
            AddError(msg.str(), count_source);
            return nullptr;
        }
    } else if (el_count->Is<core::type::RuntimeArrayCount>()) {
        size = stride;
    }
    auto* out =
        b.create<sem::Array>(el_ty, el_count, el_align, static_cast<uint32_t>(size),
                             static_cast<uint32_t>(stride), static_cast<uint32_t>(implicit_stride));

    // Maximum nesting depth of composite types
    //  https://gpuweb.github.io/gpuweb/wgsl/#limits
    const size_t nest_depth = 1 + NestDepth(el_ty);
    if (nest_depth > kMaxNestDepthOfCompositeType) {
        AddError("array has nesting depth of " + std::to_string(nest_depth) + ", maximum is " +
                     std::to_string(kMaxNestDepthOfCompositeType),
                 array_source);
        return nullptr;
    }
    nest_depth_.Add(out, nest_depth);

    if (!validator_.Array(out, el_source)) {
        return nullptr;
    }

    return out;
}

core::type::Type* Resolver::Alias(const ast::Alias* alias) {
    auto* ty = Type(alias->type);
    if (TINT_UNLIKELY(!ty)) {
        return nullptr;
    }
    if (TINT_UNLIKELY(!validator_.Alias(alias))) {
        return nullptr;
    }
    return ty;
}

sem::Struct* Resolver::Structure(const ast::Struct* str) {
    auto struct_name = [&] {  //
        return str->name->symbol.Name();
    };

    if (validator_.IsValidationEnabled(str->attributes,
                                       ast::DisabledValidation::kIgnoreStructMemberLimit)) {
        // Maximum number of members in a structure type
        // https://gpuweb.github.io/gpuweb/wgsl/#limits
        const size_t kMaxNumStructMembers = 16383;
        if (str->members.Length() > kMaxNumStructMembers) {
            AddError("struct '" + struct_name() + "' has " + std::to_string(str->members.Length()) +
                         " members, maximum is " + std::to_string(kMaxNumStructMembers),
                     str->source);
            return nullptr;
        }
    }

    if (!validator_.NoDuplicateAttributes(str->attributes)) {
        return nullptr;
    }

    for (auto* attribute : str->attributes) {
        Mark(attribute);
        bool ok = Switch(
            attribute, [&](const ast::InternalAttribute* attr) { return InternalAttribute(attr); },
            [&](Default) {
                ErrorInvalidAttribute(attribute, "struct declarations");
                return false;
            });
        if (!ok) {
            return nullptr;
        }
    }

    Vector<const sem::StructMember*, 8> sem_members;
    sem_members.Reserve(str->members.Length());

    // Calculate the effective size and alignment of each field, and the overall size of the
    // structure. For size, use the size attribute if provided, otherwise use the default size
    // for the type. For alignment, use the alignment attribute if provided, otherwise use the
    // default alignment for the member type. Diagnostic errors are raised if a basic rule is
    // violated. Validation of storage-class rules requires analyzing the actual variable usage
    // of the structure, and so is performed as part of the variable validation.
    uint64_t struct_size = 0;
    uint64_t struct_align = 1;
    Hashmap<Symbol, const ast::StructMember*, 8> member_map;

    size_t members_nest_depth = 0;
    for (auto* member : str->members) {
        Mark(member);
        Mark(member->name);
        if (auto added = member_map.Add(member->name->symbol, member); !added) {
            AddError("redefinition of '" + member->name->symbol.Name() + "'", member->source);
            AddNote("previous definition is here", (*added.value)->source);
            return nullptr;
        }

        // Resolve member type
        auto type = Type(member->type);
        if (!type) {
            return nullptr;
        }

        members_nest_depth = std::max(members_nest_depth, NestDepth(type));

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
        core::type::StructMemberAttributes attributes;
        for (auto* attribute : member->attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,  //
                [&](const ast::StructMemberOffsetAttribute* attr) {
                    // Offset attributes are not part of the WGSL spec, but are emitted by the
                    // SPIR-V reader.

                    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant,
                                                       "@offset value"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(ValueExpression(attr->expr));
                    if (!materialized) {
                        return false;
                    }
                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@offset must be constant expression", attr->expr->source);
                        return false;
                    }
                    offset = const_value->ValueAs<uint64_t>();

                    if (offset < struct_size) {
                        AddError("offsets must be in ascending order", attr->source);
                        return false;
                    }
                    has_offset_attr = true;
                    return true;
                },
                [&](const ast::StructMemberAlignAttribute* attr) {
                    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@align"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(ValueExpression(attr->expr));
                    if (!materialized) {
                        return false;
                    }
                    if (!materialized->Type()->IsAnyOf<core::type::I32, core::type::U32>()) {
                        AddError("@align must be an i32 or u32 value", attr->source);
                        return false;
                    }

                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@align must be constant expression", attr->source);
                        return false;
                    }
                    auto value = const_value->ValueAs<AInt>();

                    if (value <= 0 || !tint::IsPowerOfTwo(value)) {
                        AddError("@align value must be a positive, power-of-two integer",
                                 attr->source);
                        return false;
                    }
                    align = u32(value);
                    has_align_attr = true;
                    return true;
                },
                [&](const ast::StructMemberSizeAttribute* attr) {
                    ExprEvalStageConstraint constraint{core::EvaluationStage::kConstant, "@size"};
                    TINT_SCOPED_ASSIGNMENT(expr_eval_stage_constraint_, constraint);

                    auto* materialized = Materialize(ValueExpression(attr->expr));
                    if (!materialized) {
                        return false;
                    }
                    if (!materialized->Type()->IsAnyOf<core::type::U32, core::type::I32>()) {
                        AddError("@size must be an i32 or u32 value", attr->source);
                        return false;
                    }

                    auto const_value = materialized->ConstantValue();
                    if (!const_value) {
                        AddError("@size must be constant expression", attr->expr->source);
                        return false;
                    }
                    {
                        auto value = const_value->ValueAs<AInt>();
                        if (value <= 0) {
                            AddError("@size must be a positive integer", attr->source);
                            return false;
                        }
                    }
                    auto value = const_value->ValueAs<uint64_t>();
                    if (value < size) {
                        AddError("@size must be at least as big as the type's size (" +
                                     std::to_string(size) + ")",
                                 attr->source);
                        return false;
                    }
                    size = u32(value);
                    has_size_attr = true;
                    return true;
                },
                [&](const ast::LocationAttribute* attr) {
                    auto value = LocationAttribute(attr);
                    if (!value) {
                        return false;
                    }
                    attributes.location = value.Get();
                    return true;
                },
                [&](const ast::IndexAttribute* attr) {
                    auto value = IndexAttribute(attr);
                    if (!value) {
                        return false;
                    }
                    attributes.index = value.Get();
                    return true;
                },
                [&](const ast::BuiltinAttribute* attr) {
                    auto value = BuiltinAttribute(attr);
                    if (!value) {
                        return false;
                    }
                    attributes.builtin = value.Get();
                    return true;
                },
                [&](const ast::InterpolateAttribute* attr) {
                    auto value = InterpolateAttribute(attr);
                    if (!value) {
                        return false;
                    }
                    attributes.interpolation = value.Get();
                    return true;
                },
                [&](const ast::InvariantAttribute* attr) {
                    if (!InvariantAttribute(attr)) {
                        return false;
                    }
                    attributes.invariant = true;
                    return true;
                },
                [&](const ast::StrideAttribute* attr) {
                    if (validator_.IsValidationEnabled(
                            member->attributes, ast::DisabledValidation::kIgnoreStrideAttribute)) {
                        ErrorInvalidAttribute(attribute, "struct members");
                        return false;
                    }
                    return StrideAttribute(attr);
                },
                [&](const ast::InternalAttribute* attr) { return InternalAttribute(attr); },
                [&](Default) {
                    ErrorInvalidAttribute(attribute, "struct members");
                    return false;
                });
            if (!ok) {
                return nullptr;
            }
        }

        if (has_offset_attr && (has_align_attr || has_size_attr)) {
            AddError("@offset cannot be used with @align or @size", member->source);
            return nullptr;
        }

        offset = tint::RoundUp(align, offset);
        if (offset > std::numeric_limits<uint32_t>::max()) {
            StringStream msg;
            msg << "struct member offset (0x" << std::hex << offset << ") must not exceed 0x"
                << std::hex << std::numeric_limits<uint32_t>::max() << " bytes";
            AddError(msg.str(), member->source);
            return nullptr;
        }

        auto* sem_member = b.create<sem::StructMember>(
            member, member->name->symbol, type, static_cast<uint32_t>(sem_members.Length()),
            static_cast<uint32_t>(offset), static_cast<uint32_t>(align),
            static_cast<uint32_t>(size), attributes);
        b.Sem().Add(member, sem_member);
        sem_members.Push(sem_member);

        struct_size = offset + size;
        struct_align = std::max(struct_align, align);
    }

    uint64_t size_no_padding = struct_size;
    struct_size = tint::RoundUp(struct_align, struct_size);

    if (struct_size > std::numeric_limits<uint32_t>::max()) {
        StringStream msg;
        msg << "struct size (0x" << std::hex << struct_size << ") must not exceed 0xffffffff bytes";
        AddError(msg.str(), str->source);
        return nullptr;
    }
    if (TINT_UNLIKELY(struct_align > std::numeric_limits<uint32_t>::max())) {
        AddICE("calculated struct stride exceeds uint32", str->source);
        return nullptr;
    }

    auto* out = b.create<sem::Struct>(
        str, str->name->symbol, std::move(sem_members), static_cast<uint32_t>(struct_align),
        static_cast<uint32_t>(struct_size), static_cast<uint32_t>(size_no_padding));

    for (size_t i = 0; i < sem_members.Length(); i++) {
        auto* mem_type = sem_members[i]->Type();
        if (mem_type->Is<core::type::Atomic>()) {
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

    // Maximum nesting depth of composite types
    //  https://gpuweb.github.io/gpuweb/wgsl/#limits
    const size_t nest_depth = 1 + members_nest_depth;
    if (nest_depth > kMaxNestDepthOfCompositeType) {
        AddError("struct '" + struct_name() + "' has nesting depth of " +
                     std::to_string(nest_depth) + ", maximum is " +
                     std::to_string(kMaxNestDepthOfCompositeType),
                 str->source);
        return nullptr;
    }
    nest_depth_.Add(out, nest_depth);

    return out;
}

sem::Statement* Resolver::ReturnStatement(const ast::ReturnStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = current_statement_->Behaviors();
        behaviors = sem::Behavior::kReturn;

        const core::type::Type* value_ty = nullptr;
        if (auto* value = stmt->value) {
            const auto* expr = Load(ValueExpression(value));
            if (!expr) {
                return false;
            }
            if (auto* ret_ty = current_function_->ReturnType(); !ret_ty->Is<core::type::Void>()) {
                expr = Materialize(expr, ret_ty);
                if (!expr) {
                    return false;
                }
            }
            behaviors.Add(expr->Behaviors() - sem::Behavior::kNext);

            value_ty = expr->Type();
        } else {
            value_ty = b.create<core::type::Void>();
        }

        // Validate after processing the return value expression so that its type
        // is available for validation.
        return validator_.Return(stmt, current_function_->ReturnType(), value_ty,
                                 current_statement_);
    });
}

sem::SwitchStatement* Resolver::SwitchStatement(const ast::SwitchStatement* stmt) {
    auto* sem =
        b.create<sem::SwitchStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto& behaviors = sem->Behaviors();

        const auto* cond = Load(ValueExpression(stmt->condition));
        if (!cond) {
            return false;
        }
        behaviors = cond->Behaviors() - sem::Behavior::kNext;

        auto* cond_ty = cond->Type();

        // Determine the common type across all selectors and the switch expression
        // This must materialize to an integer scalar (non-abstract).
        Vector<const core::type::Type*, 8> types;
        types.Push(cond_ty);
        for (auto* case_stmt : stmt->body) {
            for (auto* sel : case_stmt->selectors) {
                if (sel->IsDefault()) {
                    continue;
                }
                auto* sem_expr = ValueExpression(sel->expr);
                if (!sem_expr) {
                    return false;
                }
                types.Push(sem_expr->Type()->UnwrapRef());
            }
        }
        auto* common_ty = core::type::Type::Common(types);
        if (!common_ty || !common_ty->is_integer_scalar()) {
            // No common type found or the common type was abstract.
            // Pick i32 and let validation deal with any mismatches.
            common_ty = b.create<core::type::I32>();
        }
        cond = Materialize(cond, common_ty);
        if (!cond) {
            return false;
        }

        // Handle switch body attributes.
        for (auto* attribute : stmt->body_attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,
                [&](const ast::DiagnosticAttribute* attr) { return DiagnosticAttribute(attr); },
                [&](Default) {
                    ErrorInvalidAttribute(attribute, "switch body");
                    return false;
                });
            if (!ok) {
                return false;
            }
        }
        if (!validator_.NoDuplicateAttributes(stmt->body_attributes)) {
            return false;
        }

        Vector<sem::CaseStatement*, 4> cases;
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

            ApplyDiagnosticSeverities(c);
        }

        if (behaviors.Contains(sem::Behavior::kBreak)) {
            behaviors.Add(sem::Behavior::kNext);
        }
        behaviors.Remove(sem::Behavior::kBreak);

        return validator_.SwitchStatement(stmt);
    });
}

sem::Statement* Resolver::VariableDeclStatement(const ast::VariableDeclStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        Mark(stmt->variable);

        auto* variable = Variable(stmt->variable, /* is_global */ false);
        if (!variable) {
            return false;
        }

        current_compound_statement_->AddDecl(variable->As<sem::LocalVariable>());

        if (auto* ctor = variable->Initializer()) {
            sem->Behaviors() = ctor->Behaviors();
        }

        return validator_.LocalVariable(variable);
    });
}

sem::Statement* Resolver::AssignmentStatement(const ast::AssignmentStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = ValueExpression(stmt->lhs);
        if (!lhs) {
            return false;
        }

        const bool is_phony_assignment = stmt->lhs->Is<ast::PhonyExpression>();

        const auto* rhs = ValueExpression(stmt->rhs);
        if (!rhs) {
            return false;
        }

        if (!is_phony_assignment) {
            rhs = Materialize(rhs, lhs->Type()->UnwrapRef());
            if (!rhs) {
                return false;
            }
        }

        rhs = Load(rhs);
        if (!rhs) {
            return false;
        }

        auto& behaviors = sem->Behaviors();
        behaviors = rhs->Behaviors();
        if (!is_phony_assignment) {
            behaviors.Add(lhs->Behaviors());
        }

        if (!is_phony_assignment) {
            RegisterStore(lhs);
        }

        return validator_.Assignment(stmt, sem_.TypeOf(stmt->rhs));
    });
}

sem::Statement* Resolver::BreakStatement(const ast::BreakStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        sem->Behaviors() = sem::Behavior::kBreak;

        return validator_.BreakStatement(sem, current_statement_);
    });
}

sem::Statement* Resolver::BreakIfStatement(const ast::BreakIfStatement* stmt) {
    auto* sem =
        b.create<sem::BreakIfStatement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* cond = Load(ValueExpression(stmt->condition));
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
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        if (auto* expr = ValueExpression(stmt->expr)) {
            sem->Behaviors() = expr->Behaviors();
            return true;
        }
        return false;
    });
}

sem::Statement* Resolver::CompoundAssignmentStatement(
    const ast::CompoundAssignmentStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = ValueExpression(stmt->lhs);
        if (!lhs) {
            return false;
        }

        const auto* rhs = ValueExpression(stmt->rhs);
        if (!rhs) {
            return false;
        }

        RegisterStore(lhs);

        sem->Behaviors() = rhs->Behaviors() + lhs->Behaviors();

        auto stage = core::EarliestStage(lhs->Stage(), rhs->Stage());

        auto overload =
            intrinsic_table_.Lookup(stmt->op, lhs->Type()->UnwrapRef(), rhs->Type()->UnwrapRef(),
                                    stage, stmt->source, true);
        if (!overload) {
            return false;
        }

        // Load or materialize the RHS if necessary.
        rhs = Load(Materialize(rhs, overload->parameters[1].type));
        if (!rhs) {
            return false;
        }

        return validator_.Assignment(stmt, overload->return_type);
    });
}

sem::Statement* Resolver::ContinueStatement(const ast::ContinueStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
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
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        current_function_->SetDiscardStatement(sem);
        return true;
    });
}

sem::Statement* Resolver::IncrementDecrementStatement(
    const ast::IncrementDecrementStatement* stmt) {
    auto* sem = b.create<sem::Statement>(stmt, current_compound_statement_, current_function_);
    return StatementScope(stmt, sem, [&] {
        auto* lhs = ValueExpression(stmt->lhs);
        if (!lhs) {
            return false;
        }
        sem->Behaviors() = lhs->Behaviors();

        RegisterStore(lhs);

        return validator_.IncrementDecrementStatement(stmt);
    });
}

bool Resolver::ApplyAddressSpaceUsageToType(core::AddressSpace address_space,
                                            core::type::Type* ty,
                                            const Source& usage) {
    ty = const_cast<core::type::Type*>(ty->UnwrapRef());

    if (auto* str = ty->As<sem::Struct>()) {
        if (str->AddressSpaceUsage().count(address_space)) {
            return true;  // Already applied
        }

        str->AddUsage(address_space);

        for (auto* member : str->Members()) {
            auto decl = member->Declaration();
            if (decl && !ApplyAddressSpaceUsageToType(address_space,
                                                      const_cast<core::type::Type*>(member->Type()),
                                                      decl->type->source)) {
                StringStream err;
                err << "while analyzing structure member " << sem_.TypeNameOf(str) << "."
                    << member->Name().Name();
                AddNote(err.str(), member->Declaration()->source);
                return false;
            }
        }
        return true;
    }

    if (auto* arr = ty->As<sem::Array>()) {
        if (address_space != core::AddressSpace::kStorage) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
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
        return ApplyAddressSpaceUsageToType(address_space,
                                            const_cast<core::type::Type*>(arr->ElemType()), usage);
    }

    if (core::IsHostShareable(address_space) && !validator_.IsHostShareable(ty)) {
        StringStream err;
        err << "Type '" << sem_.TypeNameOf(ty) << "' cannot be used in address space '"
            << address_space << "' as it is non-host-shareable";
        AddError(err.str(), usage);
        return false;
    }

    return true;
}

template <typename SEM, typename F>
SEM* Resolver::StatementScope(const ast::Statement* ast, SEM* sem, F&& callback) {
    b.Sem().Add(ast, sem);

    auto* as_compound =
        As<sem::CompoundStatement, tint::CastFlags::kDontErrorOnImpossibleCast>(sem);

    // Helper to handle attributes that are supported on certain types of statement.
    auto handle_attributes = [&](auto* stmt, sem::Statement* sem_stmt, const char* use) {
        for (auto* attribute : stmt->attributes) {
            Mark(attribute);
            bool ok = Switch(
                attribute,  //
                [&](const ast::DiagnosticAttribute* attr) { return DiagnosticAttribute(attr); },
                [&](Default) {
                    ErrorInvalidAttribute(attribute, use);
                    return false;
                });
            if (!ok) {
                return false;
            }
        }
        if (!validator_.NoDuplicateAttributes(stmt->attributes)) {
            return false;
        }
        ApplyDiagnosticSeverities(sem_stmt);
        return true;
    };

    // Handle attributes, if necessary.
    // Some statements can take diagnostic filtering attributes, so push a new diagnostic filter
    // scope to capture them.
    validator_.DiagnosticFilters().Push();
    TINT_DEFER(validator_.DiagnosticFilters().Pop());
    if (!Switch(
            ast,  //
            [&](const ast::BlockStatement* block) {
                return handle_attributes(block, sem, "block statements");
            },
            [&](const ast::ForLoopStatement* f) {
                return handle_attributes(f, sem, "for statements");
            },
            [&](const ast::IfStatement* i) { return handle_attributes(i, sem, "if statements"); },
            [&](const ast::LoopStatement* l) {
                return handle_attributes(l, sem, "loop statements");
            },
            [&](const ast::SwitchStatement* s) {
                return handle_attributes(s, sem, "switch statements");
            },
            [&](const ast::WhileStatement* w) {
                return handle_attributes(w, sem, "while statements");
            },
            [&](Default) { return true; })) {
        return nullptr;
    }

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
    if (TINT_UNLIKELY(node == nullptr)) {
        AddICE("Resolver::Mark() called with nullptr", {});
        return false;
    }
    auto marked_bit_ref = marked_[node->node_id.value];
    if (TINT_LIKELY(!marked_bit_ref)) {
        marked_bit_ref = true;
        return true;
    }
    StringStream err;
    err << "AST node '" << node->TypeInfo().name
        << "' was encountered twice in the same AST of a Program\n"
        << "Pointer: " << node;
    AddICE(err.str(), node->source);
    return false;
}

template <typename NODE>
void Resolver::ApplyDiagnosticSeverities(NODE* node) {
    for (auto itr : validator_.DiagnosticFilters().Top()) {
        node->SetDiagnosticSeverity(itr.key, itr.value);
    }
}

bool Resolver::CheckNotTemplated(const char* use, const ast::Identifier* ident) {
    if (TINT_UNLIKELY(ident->Is<ast::TemplatedIdentifier>())) {
        AddError(
            std::string(use) + " '" + ident->symbol.Name() + "' does not take template arguments",
            ident->source);
        if (auto resolved = dependencies_.resolved_identifiers.Get(ident)) {
            if (auto* ast_node = resolved->Node()) {
                sem_.NoteDeclarationSource(ast_node);
            }
        }
        return false;
    }
    return true;
}

void Resolver::ErrorInvalidAttribute(const ast::Attribute* attr, std::string_view use) {
    AddError("@" + attr->Name() + " is not valid for " + std::string(use), attr->source);
}

void Resolver::AddICE(const std::string& msg, const Source& source) const {
    if (source.file) {
        TINT_ICE() << source << ": " << msg;
    } else {
        TINT_ICE() << msg;
    }
    diag::Diagnostic err{};
    err.severity = diag::Severity::InternalCompilerError;
    err.system = diag::System::Resolver;
    err.source = source;
    err.message = msg;
    diagnostics_.add(std::move(err));
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

}  // namespace tint::resolver
