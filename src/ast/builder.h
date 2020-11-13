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

#ifndef SRC_AST_BUILDER_H_
#define SRC_AST_BUILDER_H_

#include <assert.h>

#include <memory>
#include <string>
#include <utility>

#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"
#include "src/context.h"

namespace tint {
namespace ast {

/// Helper for building common AST constructs
class Builder {
 public:
  /// Constructor
  /// Note, the context _must_ be set with |set_context| before the builder
  /// is used.
  Builder();
  /// Constructor
  /// @param ctx the context to use in the builder
  explicit Builder(Context* ctx);
  virtual ~Builder();

  /// Sets the given context into the builder
  /// @param ctx the context to set
  void set_context(Context* ctx) { ctx_ = ctx; }

  /// Creates a new type
  /// @param args the arguments to pass to the type constructor
  /// @returns a registered pointer to the requested type
  template <typename T, typename... ARGS>
  ast::type::Type* type(ARGS&&... args) {
    assert(ctx_);
    return ctx_->type_mgr().Get(
        std::make_unique<T>(std::forward<ARGS>(args)...));
  }

  /// @returns a pointer to the bool type
  ast::type::BoolType* bool_type() {
    return type<ast::type::BoolType>()->AsBool();
  }

  /// @returns a pointer to the f32 type
  ast::type::F32Type* f32() { return type<ast::type::F32Type>()->AsF32(); }

  /// @returns a pointer to the i32 type
  ast::type::I32Type* i32() { return type<ast::type::I32Type>()->AsI32(); }

  /// @param ty the type of the matrix components
  /// @param rows the number of rows
  /// @param cols the number of columns
  /// @returns a pointer to the u32 type
  ast::type::MatrixType* mat(ast::type::Type* ty,
                             uint32_t rows,
                             uint32_t cols) {
    return type<ast::type::MatrixType>(ty, rows, cols)->AsMatrix();
  }

  /// @returns a pointer to the u32 type
  ast::type::U32Type* u32() { return type<ast::type::U32Type>()->AsU32(); }

  /// @param ty the type of the vector components
  /// @param size the size of the vector
  /// @returns a pointer to the vector type
  ast::type::VectorType* vec(ast::type::Type* ty, uint32_t size) {
    return type<ast::type::VectorType>(ty, size)->AsVector();
  }

  /// @returns a pointer to the void type
  ast::type::VoidType* void_type() {
    return type<ast::type::VoidType>()->AsVoid();
  }

  /// @param expr the expression
  /// @return expr
  std::unique_ptr<ast::Expression> make_expr(
      std::unique_ptr<ast::Expression> expr) {
    return expr;
  }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  std::unique_ptr<ast::IdentifierExpression> make_expr(
      const std::string& name) {
    return create<ast::IdentifierExpression>(name);
  }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  std::unique_ptr<ast::IdentifierExpression> make_expr(const char* name) {
    return create<ast::IdentifierExpression>(name);
  }

  /// @param value the float value
  /// @return a Scalar constructor for the given value
  std::unique_ptr<ast::ScalarConstructorExpression> make_expr(float value) {
    return create<ast::ScalarConstructorExpression>(make_literal(value));
  }

  /// @param value the int value
  /// @return a Scalar constructor for the given value
  std::unique_ptr<ast::ScalarConstructorExpression> make_expr(int32_t value) {
    return create<ast::ScalarConstructorExpression>(make_literal(value));
  }

  /// @param value the unsigned int value
  /// @return a Scalar constructor for the given value
  std::unique_ptr<ast::ScalarConstructorExpression> make_expr(uint32_t value) {
    return create<ast::ScalarConstructorExpression>(make_literal(value));
  }

  /// @param val the boolan value
  /// @return a boolean literal with the given value
  std::unique_ptr<ast::BoolLiteral> make_literal(bool val) {
    return create<ast::BoolLiteral>(bool_type(), val);
  }

  /// @param val the float value
  /// @return a float literal with the given value
  std::unique_ptr<ast::FloatLiteral> make_literal(float val) {
    return create<ast::FloatLiteral>(f32(), val);
  }

  /// @param val the unsigned int value
  /// @return a UintLiteral with the given value
  std::unique_ptr<ast::UintLiteral> make_literal(uint32_t val) {
    return create<ast::UintLiteral>(u32(), val);
  }

  /// @param val the integer value
  /// @return the SintLiteral with the given value
  std::unique_ptr<ast::SintLiteral> make_literal(int32_t val) {
    return create<ast::SintLiteral>(i32(), val);
  }

  /// Converts `arg` to an `ast::Expression` using `make_expr()`, then appends
  /// it to `list`.
  /// @param list the list to append too
  /// @param arg the arg to create
  template <typename ARG>
  void append_expr(ast::ExpressionList& list, ARG&& arg) {
    list.emplace_back(make_expr(std::forward<ARG>(arg)));
  }

  /// Converts `arg0` and `args` to `ast::Expression`s using `make_expr()`,
  /// then appends them to `list`.
  /// @param list the list to append too
  /// @param arg0 the first argument
  /// @param args the rest of the arguments
  template <typename ARG0, typename... ARGS>
  void append_expr(ast::ExpressionList& list, ARG0&& arg0, ARGS&&... args) {
    append_expr(list, std::forward<ARG0>(arg0));
    append_expr(list, std::forward<ARGS>(args)...);
  }

  /// @param ty the type
  /// @param args the arguments for the type constructor
  /// @return an `ast::TypeConstructorExpression` of type `ty`, with the values
  /// of `args` converted to `ast::Expression`s using `make_expr()`
  template <typename... ARGS>
  std::unique_ptr<ast::TypeConstructorExpression> construct(ast::type::Type* ty,
                                                            ARGS&&... args) {
    ast::ExpressionList vals;
    append_expr(vals, std::forward<ARGS>(args)...);
    return create<ast::TypeConstructorExpression>(ty, std::move(vals));
  }

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @returns a `ast::Variable` with the given name, storage and type
  virtual std::unique_ptr<ast::Variable> make_var(const std::string& name,
                                                  ast::StorageClass storage,
                                                  ast::type::Type* type);

  /// @param func the function name
  /// @param args the function call arguments
  /// @returns a `ast::CallExpression` to the function `func`, with the
  /// arguments of `args` converted to `ast::Expression`s using `make_expr()`.
  template <typename... ARGS>
  ast::CallExpression call_expr(const std::string& func, ARGS&&... args) {
    ast::ExpressionList params;
    append_expr(params, std::forward<ARGS>(args)...);
    return ast::CallExpression{make_expr(func), std::move(params)};
  }

  /// @return a `std::unique_ptr` to a new `T` constructed with `args`
  /// @param args the arguments to forward to the constructor for `T`
  template <typename T, typename... ARGS>
  std::unique_ptr<T> create(ARGS&&... args) {
    return std::make_unique<T>(std::forward<ARGS>(args)...);
  }

 private:
  Context* ctx_ = nullptr;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BUILDER_H_
