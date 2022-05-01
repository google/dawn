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

#include "src/tint/transform/module_scope_var_to_entry_point_param.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::ModuleScopeVarToEntryPointParam);

namespace tint::transform {
namespace {
// Returns `true` if `type` is or contains a matrix type.
bool ContainsMatrix(const sem::Type* type) {
    type = type->UnwrapRef();
    if (type->Is<sem::Matrix>()) {
        return true;
    } else if (auto* ary = type->As<sem::Array>()) {
        return ContainsMatrix(ary->ElemType());
    } else if (auto* str = type->As<sem::Struct>()) {
        for (auto* member : str->Members()) {
            if (ContainsMatrix(member->Type())) {
                return true;
            }
        }
    }
    return false;
}
}  // namespace

/// State holds the current transform state.
struct ModuleScopeVarToEntryPointParam::State {
    /// The clone context.
    CloneContext& ctx;

    /// Constructor
    /// @param context the clone context
    explicit State(CloneContext& context) : ctx(context) {}

    /// Clone any struct types that are contained in `ty` (including `ty` itself),
    /// and add it to the global declarations now, so that they precede new global
    /// declarations that need to reference them.
    /// @param ty the type to clone
    void CloneStructTypes(const sem::Type* ty) {
        if (auto* str = ty->As<sem::Struct>()) {
            if (!cloned_structs_.emplace(str).second) {
                // The struct has already been cloned.
                return;
            }

            // Recurse into members.
            for (auto* member : str->Members()) {
                CloneStructTypes(member->Type());
            }

            // Clone the struct and add it to the global declaration list.
            // Remove the old declaration.
            auto* ast_str = str->Declaration();
            ctx.dst->AST().AddTypeDecl(ctx.Clone(ast_str));
            ctx.Remove(ctx.src->AST().GlobalDeclarations(), ast_str);
        } else if (auto* arr = ty->As<sem::Array>()) {
            CloneStructTypes(arr->ElemType());
        }
    }

    /// Process the module.
    void Process() {
        // Predetermine the list of function calls that need to be replaced.
        using CallList = std::vector<const ast::CallExpression*>;
        std::unordered_map<const ast::Function*, CallList> calls_to_replace;

        std::vector<const ast::Function*> functions_to_process;

        // Build a list of functions that transitively reference any module-scope
        // variables.
        for (auto* func_ast : ctx.src->AST().Functions()) {
            auto* func_sem = ctx.src->Sem().Get(func_ast);

            bool needs_processing = false;
            for (auto* var : func_sem->TransitivelyReferencedGlobals()) {
                if (var->StorageClass() != ast::StorageClass::kNone) {
                    needs_processing = true;
                    break;
                }
            }
            if (needs_processing) {
                functions_to_process.push_back(func_ast);

                // Find all of the calls to this function that will need to be replaced.
                for (auto* call : func_sem->CallSites()) {
                    calls_to_replace[call->Stmt()->Function()->Declaration()].push_back(
                        call->Declaration());
                }
            }
        }

        // Build a list of `&ident` expressions. We'll use this later to avoid
        // generating expressions of the form `&*ident`, which break WGSL validation
        // rules when this expression is passed to a function.
        // TODO(jrprice): We should add support for bidirectional SEM tree traversal
        // so that we can do this on the fly instead.
        std::unordered_map<const ast::IdentifierExpression*, const ast::UnaryOpExpression*>
            ident_to_address_of;
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            auto* address_of = node->As<ast::UnaryOpExpression>();
            if (!address_of || address_of->op != ast::UnaryOp::kAddressOf) {
                continue;
            }
            if (auto* ident = address_of->expr->As<ast::IdentifierExpression>()) {
                ident_to_address_of[ident] = address_of;
            }
        }

        for (auto* func_ast : functions_to_process) {
            auto* func_sem = ctx.src->Sem().Get(func_ast);
            bool is_entry_point = func_ast->IsEntryPoint();

            // Map module-scope variables onto their replacement.
            struct NewVar {
                Symbol symbol;
                bool is_pointer;
                bool is_wrapped;
            };
            const char* kWrappedArrayMemberName = "arr";
            std::unordered_map<const sem::Variable*, NewVar> var_to_newvar;

            // We aggregate all workgroup variables into a struct to avoid hitting
            // MSL's limit for threadgroup memory arguments.
            Symbol workgroup_parameter_symbol;
            ast::StructMemberList workgroup_parameter_members;
            auto workgroup_param = [&]() {
                if (!workgroup_parameter_symbol.IsValid()) {
                    workgroup_parameter_symbol = ctx.dst->Sym();
                }
                return workgroup_parameter_symbol;
            };

            for (auto* var : func_sem->TransitivelyReferencedGlobals()) {
                auto sc = var->StorageClass();
                auto* ty = var->Type()->UnwrapRef();
                if (sc == ast::StorageClass::kNone) {
                    continue;
                }
                if (sc != ast::StorageClass::kPrivate && sc != ast::StorageClass::kStorage &&
                    sc != ast::StorageClass::kUniform && sc != ast::StorageClass::kHandle &&
                    sc != ast::StorageClass::kWorkgroup) {
                    TINT_ICE(Transform, ctx.dst->Diagnostics())
                        << "unhandled module-scope storage class (" << sc << ")";
                }

                // This is the symbol for the variable that replaces the module-scope
                // var.
                auto new_var_symbol = ctx.dst->Sym();

                // Helper to create an AST node for the store type of the variable.
                auto store_type = [&]() { return CreateASTTypeFor(ctx, ty); };

                // Track whether the new variable is a pointer or not.
                bool is_pointer = false;

                // Track whether the new variable was wrapped in a struct or not.
                bool is_wrapped = false;

                if (is_entry_point) {
                    if (var->Type()->UnwrapRef()->is_handle()) {
                        // For a texture or sampler variable, redeclare it as an entry point
                        // parameter. Disable entry point parameter validation.
                        auto* disable_validation =
                            ctx.dst->Disable(ast::DisabledValidation::kEntryPointParameter);
                        auto attrs = ctx.Clone(var->Declaration()->attributes);
                        attrs.push_back(disable_validation);
                        auto* param = ctx.dst->Param(new_var_symbol, store_type(), attrs);
                        ctx.InsertFront(func_ast->params, param);
                    } else if (sc == ast::StorageClass::kStorage ||
                               sc == ast::StorageClass::kUniform) {
                        // Variables into the Storage and Uniform storage classes are
                        // redeclared as entry point parameters with a pointer type.
                        auto attributes = ctx.Clone(var->Declaration()->attributes);
                        attributes.push_back(
                            ctx.dst->Disable(ast::DisabledValidation::kEntryPointParameter));
                        attributes.push_back(
                            ctx.dst->Disable(ast::DisabledValidation::kIgnoreStorageClass));

                        auto* param_type = store_type();
                        if (auto* arr = ty->As<sem::Array>(); arr && arr->IsRuntimeSized()) {
                            // Wrap runtime-sized arrays in structures, so that we can declare
                            // pointers to them. Ideally we'd just emit the array itself as a
                            // pointer, but this is not representable in Tint's AST.
                            CloneStructTypes(ty);
                            auto* wrapper = ctx.dst->Structure(
                                ctx.dst->Sym(),
                                {ctx.dst->Member(kWrappedArrayMemberName, param_type)});
                            param_type = ctx.dst->ty.Of(wrapper);
                            is_wrapped = true;
                        }

                        param_type = ctx.dst->ty.pointer(param_type, sc,
                                                         var->Declaration()->declared_access);
                        auto* param = ctx.dst->Param(new_var_symbol, param_type, attributes);
                        ctx.InsertFront(func_ast->params, param);
                        is_pointer = true;
                    } else if (sc == ast::StorageClass::kWorkgroup && ContainsMatrix(var->Type())) {
                        // Due to a bug in the MSL compiler, we use a threadgroup memory
                        // argument for any workgroup allocation that contains a matrix.
                        // See crbug.com/tint/938.
                        // TODO(jrprice): Do this for all other workgroup variables too.

                        // Create a member in the workgroup parameter struct.
                        auto member = ctx.Clone(var->Declaration()->symbol);
                        workgroup_parameter_members.push_back(
                            ctx.dst->Member(member, store_type()));
                        CloneStructTypes(var->Type()->UnwrapRef());

                        // Create a function-scope variable that is a pointer to the member.
                        auto* member_ptr = ctx.dst->AddressOf(
                            ctx.dst->MemberAccessor(ctx.dst->Deref(workgroup_param()), member));
                        auto* local_var = ctx.dst->Let(
                            new_var_symbol,
                            ctx.dst->ty.pointer(store_type(), ast::StorageClass::kWorkgroup),
                            member_ptr);
                        ctx.InsertFront(func_ast->body->statements, ctx.dst->Decl(local_var));
                        is_pointer = true;
                    } else {
                        // Variables in the Private and Workgroup storage classes are
                        // redeclared at function scope. Disable storage class validation on
                        // this variable.
                        auto* disable_validation =
                            ctx.dst->Disable(ast::DisabledValidation::kIgnoreStorageClass);
                        auto* constructor = ctx.Clone(var->Declaration()->constructor);
                        auto* local_var =
                            ctx.dst->Var(new_var_symbol, store_type(), sc, constructor,
                                         ast::AttributeList{disable_validation});
                        ctx.InsertFront(func_ast->body->statements, ctx.dst->Decl(local_var));
                    }
                } else {
                    // For a regular function, redeclare the variable as a parameter.
                    // Use a pointer for non-handle types.
                    auto* param_type = store_type();
                    ast::AttributeList attributes;
                    if (!var->Type()->UnwrapRef()->is_handle()) {
                        param_type = ctx.dst->ty.pointer(param_type, sc,
                                                         var->Declaration()->declared_access);
                        is_pointer = true;

                        // Disable validation of the parameter's storage class and of
                        // arguments passed it.
                        attributes.push_back(
                            ctx.dst->Disable(ast::DisabledValidation::kIgnoreStorageClass));
                        attributes.push_back(ctx.dst->Disable(
                            ast::DisabledValidation::kIgnoreInvalidPointerArgument));
                    }
                    ctx.InsertBack(func_ast->params,
                                   ctx.dst->Param(new_var_symbol, param_type, attributes));
                }

                // Replace all uses of the module-scope variable.
                // For non-entry points, dereference non-handle pointer parameters.
                for (auto* user : var->Users()) {
                    if (user->Stmt()->Function()->Declaration() == func_ast) {
                        const ast::Expression* expr = ctx.dst->Expr(new_var_symbol);
                        if (is_pointer) {
                            // If this identifier is used by an address-of operator, just
                            // remove the address-of instead of adding a deref, since we
                            // already have a pointer.
                            auto* ident = user->Declaration()->As<ast::IdentifierExpression>();
                            if (ident_to_address_of.count(ident)) {
                                ctx.Replace(ident_to_address_of[ident], expr);
                                continue;
                            }

                            expr = ctx.dst->Deref(expr);
                        }
                        if (is_wrapped) {
                            // Get the member from the wrapper structure.
                            expr = ctx.dst->MemberAccessor(expr, kWrappedArrayMemberName);
                        }
                        ctx.Replace(user->Declaration(), expr);
                    }
                }

                var_to_newvar[var] = {new_var_symbol, is_pointer, is_wrapped};
            }

            if (!workgroup_parameter_members.empty()) {
                // Create the workgroup memory parameter.
                // The parameter is a struct that contains members for each workgroup
                // variable.
                auto* str =
                    ctx.dst->Structure(ctx.dst->Sym(), std::move(workgroup_parameter_members));
                auto* param_type =
                    ctx.dst->ty.pointer(ctx.dst->ty.Of(str), ast::StorageClass::kWorkgroup);
                auto* disable_validation =
                    ctx.dst->Disable(ast::DisabledValidation::kEntryPointParameter);
                auto* param = ctx.dst->Param(workgroup_param(), param_type, {disable_validation});
                ctx.InsertFront(func_ast->params, param);
            }

            // Pass the variables as pointers to any functions that need them.
            for (auto* call : calls_to_replace[func_ast]) {
                auto* target = ctx.src->AST().Functions().Find(call->target.name->symbol);
                auto* target_sem = ctx.src->Sem().Get(target);

                // Add new arguments for any variables that are needed by the callee.
                // For entry points, pass non-handle types as pointers.
                for (auto* target_var : target_sem->TransitivelyReferencedGlobals()) {
                    auto sc = target_var->StorageClass();
                    if (sc == ast::StorageClass::kNone) {
                        continue;
                    }

                    auto new_var = var_to_newvar[target_var];
                    bool is_handle = target_var->Type()->UnwrapRef()->is_handle();
                    const ast::Expression* arg = ctx.dst->Expr(new_var.symbol);
                    if (new_var.is_wrapped) {
                        // The variable is wrapped in a struct, so we need to pass a pointer
                        // to the struct member instead.
                        arg = ctx.dst->AddressOf(
                            ctx.dst->MemberAccessor(ctx.dst->Deref(arg), kWrappedArrayMemberName));
                    } else if (is_entry_point && !is_handle && !new_var.is_pointer) {
                        // We need to pass a pointer and we don't already have one, so take
                        // the address of the new variable.
                        arg = ctx.dst->AddressOf(arg);
                    }
                    ctx.InsertBack(call->args, arg);
                }
            }
        }

        // Now remove all module-scope variables with these storage classes.
        for (auto* var_ast : ctx.src->AST().GlobalVariables()) {
            auto* var_sem = ctx.src->Sem().Get(var_ast);
            if (var_sem->StorageClass() != ast::StorageClass::kNone) {
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), var_ast);
            }
        }
    }

  private:
    std::unordered_set<const sem::Struct*> cloned_structs_;
};

ModuleScopeVarToEntryPointParam::ModuleScopeVarToEntryPointParam() = default;

ModuleScopeVarToEntryPointParam::~ModuleScopeVarToEntryPointParam() = default;

bool ModuleScopeVarToEntryPointParam::ShouldRun(const Program* program, const DataMap&) const {
    for (auto* decl : program->AST().GlobalDeclarations()) {
        if (decl->Is<ast::Variable>()) {
            return true;
        }
    }
    return false;
}

void ModuleScopeVarToEntryPointParam::Run(CloneContext& ctx, const DataMap&, DataMap&) const {
    State state{ctx};
    state.Process();
    ctx.Clone();
}

}  // namespace tint::transform
