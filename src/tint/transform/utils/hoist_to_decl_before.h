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

#ifndef SRC_TINT_TRANSFORM_UTILS_HOIST_TO_DECL_BEFORE_H_
#define SRC_TINT_TRANSFORM_UTILS_HOIST_TO_DECL_BEFORE_H_

#include <functional>
#include <memory>

#include "src/tint/sem/value_expression.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Utility class that can be used to hoist expressions before other
/// expressions, possibly converting 'for-loop's to 'loop's and 'else-if's to
// 'else {if}'s.
class HoistToDeclBefore {
  public:
    /// Constructor
    /// @param ctx the clone context
    explicit HoistToDeclBefore(CloneContext& ctx);

    /// Destructor
    ~HoistToDeclBefore();

    /// StmtBuilder is a builder of an AST statement
    using StmtBuilder = std::function<const ast::Statement*()>;

    /// VariableKind is either a var, let or const
    enum class VariableKind {
        kVar,
        kLet,
        kConst,
    };

    /// Hoists @p expr to a `let` or `var` with optional `decl_name`, inserting it
    /// before @p before_expr.
    /// @param before_expr expression to insert `expr` before
    /// @param expr expression to hoist
    /// @param kind variable kind to hoist to
    /// @param decl_name optional name to use for the variable/constant name
    /// @return true on success
    bool Add(const sem::ValueExpression* before_expr,
             const ast::Expression* expr,
             VariableKind kind,
             const char* decl_name = "");

    /// Inserts @p stmt before @p before_stmt, possibly converting 'for-loop's to 'loop's if
    /// necessary.
    /// @warning If the container of @p before_stmt is cloned multiple times, then the resolver will
    /// ICE as the same statement cannot be shared.
    /// @param before_stmt statement to insert @p stmt before
    /// @param stmt statement to insert
    /// @return true on success
    bool InsertBefore(const sem::Statement* before_stmt, const ast::Statement* stmt);

    /// Inserts the returned statement of @p builder before @p before_stmt, possibly converting
    /// 'for-loop's to 'loop's if necessary.
    /// @note If the container of @p before_stmt is cloned multiple times, then @p builder will be
    /// called for each clone.
    /// @param before_stmt the preceding statement that the statement of @p builder will be inserted
    /// before
    /// @param builder the statement builder used to create the new statement
    /// @return true on success
    bool InsertBefore(const sem::Statement* before_stmt, const StmtBuilder& builder);

    /// Replaces the statement @p what with the statement @p stmt, possibly converting 'for-loop's
    /// to 'loop's if necessary.
    /// @param what the statement to replace
    /// @param with the replacement statement
    /// @return true on success
    bool Replace(const sem::Statement* what, const ast::Statement* with);

    /// Replaces the statement @p what with the statement returned by @p stmt, possibly converting
    /// 'for-loop's to 'loop's if necessary.
    /// @param what the statement to replace
    /// @param with the replacement statement builder
    /// @return true on success
    bool Replace(const sem::Statement* what, const StmtBuilder& with);

    /// Use to signal that we plan on hoisting a decl before `before_expr`. This
    /// will convert 'for-loop's to 'loop's and 'else-if's to 'else {if}'s if
    /// needed.
    /// @param before_expr expression we would hoist a decl before
    /// @return true on success
    bool Prepare(const sem::ValueExpression* before_expr);

  private:
    struct State;
    std::unique_ptr<State> state_;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_UTILS_HOIST_TO_DECL_BEFORE_H_
