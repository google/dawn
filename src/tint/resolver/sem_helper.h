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

#ifndef SRC_TINT_RESOLVER_SEM_HELPER_H_
#define SRC_TINT_RESOLVER_SEM_HELPER_H_

#include <string>

#include "src/tint/builtin/builtin_value.h"
#include "src/tint/builtin/interpolation_sampling.h"
#include "src/tint/builtin/interpolation_type.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/sem/builtin_enum_expression.h"
#include "src/tint/sem/function_expression.h"
#include "src/tint/sem/type_expression.h"
#include "src/tint/utils/map.h"

namespace tint::resolver {

/// Helper class to retrieve sem information.
class SemHelper {
  public:
    /// Constructor
    /// @param builder the program builder
    explicit SemHelper(ProgramBuilder* builder);
    ~SemHelper();

    /// Get is a helper for obtaining the semantic node for the given AST node.
    /// Raises an ICE and returns `nullptr` if there is no semantic node associated with the AST
    /// node.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename SEM = sem::Info::InferFromAST, typename AST = ast::Node>
    auto* Get(const AST* ast) const {
        using T = sem::Info::GetResultType<SEM, AST>;
        auto* sem = builder_->Sem().Get(ast);
        if (TINT_UNLIKELY(!sem)) {
            TINT_ICE(Resolver, builder_->Diagnostics())
                << "AST node '" << ast->TypeInfo().name << "' had no semantic info\n"
                << "At: " << ast->source << "\n"
                << "Pointer: " << ast;
        }
        return const_cast<T*>(As<T>(sem));
    }

    /// GetVal is a helper for obtaining the semantic sem::ValueExpression for the given AST node.
    /// Raises an error diagnostic and returns `nullptr` if the semantic node is not a
    /// sem::ValueExpression.
    /// @param ast the ast node to get the sem for
    /// @returns the sem node for @p ast
    template <typename AST = ast::Node>
    auto* GetVal(const AST* ast) const {
        return AsValueExpression(Get(ast));
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to sem::ValueExpression if the cast
    /// is successful, otherwise an error diagnostic is raised.
    sem::ValueExpression* AsValueExpression(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            if (auto* val_expr = expr->As<sem::ValueExpression>(); TINT_LIKELY(val_expr)) {
                return val_expr;
            }
            ErrorExpectedValueExpr(expr);
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to type::Type if the cast is
    /// successful, otherwise an error diagnostic is raised.
    sem::TypeExpression* AsTypeExpression(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            if (auto* ty_expr = expr->As<sem::TypeExpression>(); TINT_LIKELY(ty_expr)) {
                return ty_expr;
            }
            ErrorUnexpectedExprKind(expr, "type");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to sem::Function if the cast is
    /// successful, otherwise an error diagnostic is raised.
    sem::FunctionExpression* AsFunctionExpression(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* fn_expr = expr->As<sem::FunctionExpression>();
            if (TINT_LIKELY(fn_expr)) {
                return fn_expr;
            }
            ErrorUnexpectedExprKind(expr, "function");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<builtin::AddressSpace> if the cast is successful, otherwise an
    /// error diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::AddressSpace>* AsAddressSpace(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::AddressSpace>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "address space");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<builtin::BuiltinValue> if the cast is successful, otherwise an
    /// error diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::BuiltinValue>* AsBuiltinValue(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::BuiltinValue>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "builtin value");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<type::TexelFormat> if the cast is successful, otherwise an error
    /// diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::TexelFormat>* AsTexelFormat(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::TexelFormat>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "texel format");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<builtin::Access> if the cast is successful, otherwise an error
    /// diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::Access>* AsAccess(sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::Access>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "access");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<builtin::InterpolationSampling> if the cast is successful,
    /// otherwise an error diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::InterpolationSampling>* AsInterpolationSampling(
        sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr =
                expr->As<sem::BuiltinEnumExpression<builtin::InterpolationSampling>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "interpolation sampling");
        }
        return nullptr;
    }

    /// @param expr the semantic node
    /// @returns nullptr if @p expr is nullptr, or @p expr cast to
    /// sem::BuiltinEnumExpression<builtin::InterpolationType> if the cast is successful, otherwise
    /// an error diagnostic is raised.
    sem::BuiltinEnumExpression<builtin::InterpolationType>* AsInterpolationType(
        sem::Expression* expr) const {
        if (TINT_LIKELY(expr)) {
            auto* enum_expr = expr->As<sem::BuiltinEnumExpression<builtin::InterpolationType>>();
            if (TINT_LIKELY(enum_expr)) {
                return enum_expr;
            }
            ErrorUnexpectedExprKind(expr, "interpolation type");
        }
        return nullptr;
    }

    /// @returns the resolved type of the ast::Expression @p expr
    /// @param expr the expression
    type::Type* TypeOf(const ast::Expression* expr) const;

    /// @returns the type name of the given semantic type, unwrapping references.
    /// @param ty the type to look up
    std::string TypeNameOf(const type::Type* ty) const;

    /// @returns the type name of the given semantic type, without unwrapping references.
    /// @param ty the type to look up
    std::string RawTypeNameOf(const type::Type* ty) const;

    /// Raises an error diagnostic that the expression @p got was expected to be a
    /// sem::ValueExpression, but the expression evaluated to something different.
    /// @param expr the expression
    void ErrorExpectedValueExpr(const sem::Expression* expr) const;

    /// Raises an error diagnostic that the expression @p got was not of the kind @p wanted.
    /// @param expr the expression
    /// @param wanted the expected expression kind
    void ErrorUnexpectedExprKind(const sem::Expression* expr, std::string_view wanted) const;

    /// If @p node is a module-scope type, variable or function declaration, then appends a note
    /// diagnostic where this declaration was declared, otherwise the function does nothing.
    /// @param node the AST node.
    void NoteDeclarationSource(const ast::Node* node) const;

    /// @param expr the expression to describe
    /// @return a string that describes @p expr. Useful for diagnostics.
    std::string Describe(const sem::Expression* expr) const;

  private:
    /// Adds the given error message to the diagnostics
    void AddError(const std::string& msg, const Source& source) const;

    /// Adds the given warning message to the diagnostics
    void AddWarning(const std::string& msg, const Source& source) const;

    /// Adds the given note message to the diagnostics
    void AddNote(const std::string& msg, const Source& source) const;

    ProgramBuilder* builder_;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_SEM_HELPER_H_
