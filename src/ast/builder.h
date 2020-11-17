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
#include "src/ast/type/array_type.h"
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

/// TypesBuilder holds basic `ast::tint` types and methods for constructing
/// complex types.
class TypesBuilder {
 public:
  /// Constructor
  /// @param tm the type manager
  explicit TypesBuilder(TypeManager* tm);

  /// A boolean type
  ast::type::BoolType* const bool_;
  /// A f32 type
  ast::type::F32Type* const f32;
  /// A i32 type
  ast::type::I32Type* const i32;
  /// A u32 type
  ast::type::U32Type* const u32;
  /// A void type
  ast::type::VoidType* const void_;

  /// @return the tint AST type for the C type `T`.
  template <typename T>
  ast::type::Type* Of() const {
    return CToAST<T>::get(this);
  }

  /// @return the tint AST type for a 2-element vector of the C type `T`.
  template <typename T>
  ast::type::VectorType* vec2() const {
    return tm_->Get<ast::type::VectorType>(Of<T>(), 2);
  }

  /// @return the tint AST type for a 3-element vector of the C type `T`.
  template <typename T>
  ast::type::VectorType* vec3() const {
    return tm_->Get<ast::type::VectorType>(Of<T>(), 3);
  }

  /// @return the tint AST type for a 4-element vector of the C type `T`.
  template <typename T>
  ast::type::Type* vec4() const {
    return tm_->Get<ast::type::VectorType>(Of<T>(), 4);
  }

  /// @return the tint AST type for a 2x3 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat2x2() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 2, 2);
  }

  /// @return the tint AST type for a 2x3 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat2x3() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 3, 2);
  }

  /// @return the tint AST type for a 2x4 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat2x4() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 4, 2);
  }

  /// @return the tint AST type for a 3x2 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat3x2() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 2, 3);
  }

  /// @return the tint AST type for a 3x3 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat3x3() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 3, 3);
  }

  /// @return the tint AST type for a 3x4 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat3x4() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 4, 3);
  }

  /// @return the tint AST type for a 4x2 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat4x2() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 2, 4);
  }

  /// @return the tint AST type for a 4x3 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat4x3() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 3, 4);
  }

  /// @return the tint AST type for a 4x4 matrix of the C type `T`.
  template <typename T>
  ast::type::MatrixType* mat4x4() const {
    return tm_->Get<ast::type::MatrixType>(Of<T>(), 4, 4);
  }

  /// @param subtype the array element type
  /// @param n the array size. 0 represents unbounded
  /// @return the tint AST type for a array of size `n` of type `T`
  ast::type::ArrayType* array(ast::type::Type* subtype, uint32_t n) const {
    return tm_->Get<ast::type::ArrayType>(subtype, n);
  }

  /// @return the tint AST type for an array of size `N` of type `T`
  template <typename T, int N = 0>
  ast::type::ArrayType* array() const {
    return array(Of<T>(), N);
  }

 private:
  /// CToAST<T> is specialized for various `T` types and each specialization
  /// contains a single static `get()` method for obtaining the corresponding
  /// AST type for the C type `T`.
  /// `get()` has the signature:
  ///    `static ast::type::Type* get(Types* t)`
  template <typename T>
  struct CToAST {};

  TypeManager* const tm_;
};

/// Helper for building common AST constructs.
class Builder {
 public:
  /// `i32` is a type alias to `int`.
  /// Useful for passing to template methods such as `vec2<i32>()` to imitate
  /// WGSL syntax.
  /// Note: this is intentionally not aliased to uint32_t as we want integer
  /// literals passed to the builder to match WGSL's integer literal types.
  using i32 = decltype(1);
  /// `u32` is a type alias to `unsigned int`.
  /// Useful for passing to template methods such as `vec2<u32>()` to imitate
  /// WGSL syntax.
  /// Note: this is intentionally not aliased to uint32_t as we want integer
  /// literals passed to the builder to match WGSL's integer literal types.
  using u32 = decltype(1u);
  /// `f32` is a type alias to `float`
  /// Useful for passing to template methods such as `vec2<f32>()` to imitate
  /// WGSL syntax.
  using f32 = float;

  /// Constructor
  /// @param ctx the context to use in the builder
  explicit Builder(tint::Context* ctx);
  virtual ~Builder();

  /// @param expr the expression
  /// @return expr
  ast::Expression* Expr(ast::Expression* expr) { return expr; }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const std::string& name) {
    return create<ast::IdentifierExpression>(name);
  }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  ast::IdentifierExpression* Expr(const char* name) {
    return create<ast::IdentifierExpression>(name);
  }

  /// @param value the boolean value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(bool value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param value the float value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(f32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param value the integer value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(i32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// @param value the unsigned int value
  /// @return a Scalar constructor for the given value
  ast::ScalarConstructorExpression* Expr(u32 value) {
    return create<ast::ScalarConstructorExpression>(Literal(value));
  }

  /// Converts `arg` to an `ast::Expression` using `Expr()`, then appends it to
  /// `list`.
  /// @param list the list to append too
  /// @param arg the arg to create
  template <typename ARG>
  void Append(ast::ExpressionList& list, ARG&& arg) {
    list.emplace_back(Expr(std::forward<ARG>(arg)));
  }

  /// Converts `arg0` and `args` to `ast::Expression`s using `Expr()`,
  /// then appends them to `list`.
  /// @param list the list to append too
  /// @param arg0 the first argument
  /// @param args the rest of the arguments
  template <typename ARG0, typename... ARGS>
  void Append(ast::ExpressionList& list, ARG0&& arg0, ARGS&&... args) {
    Append(list, std::forward<ARG0>(arg0));
    Append(list, std::forward<ARGS>(args)...);
  }

  /// @return an empty list of expressions,
  ast::ExpressionList ExprList() { return {}; }

  /// @param args the list of expressions
  /// @return the list of expressions converted to `ast::Expression`s using
  /// `Expr()`,
  template <typename... ARGS>
  ast::ExpressionList ExprList(ARGS&&... args) {
    ast::ExpressionList list;
    list.reserve(sizeof...(args));
    Append(list, std::forward<ARGS>(args)...);
    return list;
  }

  /// @param val the boolan value
  /// @return a boolean literal with the given value
  ast::BoolLiteral* Literal(bool val) {
    return create<ast::BoolLiteral>(ty.bool_, val);
  }

  /// @param val the float value
  /// @return a float literal with the given value
  ast::FloatLiteral* Literal(f32 val) {
    return create<ast::FloatLiteral>(ty.f32, val);
  }

  /// @param val the unsigned int value
  /// @return a UintLiteral with the given value
  ast::UintLiteral* Literal(u32 val) {
    return create<ast::UintLiteral>(ty.u32, val);
  }

  /// @param val the integer value
  /// @return the SintLiteral with the given value
  ast::SintLiteral* Literal(i32 val) {
    return create<ast::SintLiteral>(ty.i32, val);
  }

  /// @param args the arguments for the type constructor
  /// @return an `ast::TypeConstructorExpression` of type `ty`, with the values
  /// of `args` converted to `ast::Expression`s using `Expr()`
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* Construct(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.Of<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param type the type to construct
  /// @param args the arguments for the constructor
  /// @return an `ast::TypeConstructorExpression` of `type` constructed with the
  /// values `args`.
  template <typename... ARGS>
  ast::TypeConstructorExpression* Construct(ast::type::Type* type,
                                            ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        type, ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 2-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec2(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.vec2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 3-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec3(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.vec3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `ast::TypeConstructorExpression` of a 4-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* vec4(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.vec4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x2(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat2x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x3(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat2x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 2x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat2x4(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat2x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x2(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat3x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x3(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat3x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 3x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat3x4(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat3x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x2(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat4x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x3(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat4x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  ast::TypeConstructorExpression* mat4x4(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.mat4x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the array constructor
  /// @return an `ast::TypeConstructorExpression` of a 4x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, int N = 0, typename... ARGS>
  ast::TypeConstructorExpression* array(ARGS&&... args) {
    return create<ast::TypeConstructorExpression>(
        ty.array<T, N>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @returns a `ast::Variable` with the given name, storage and type
  ast::Variable* Var(const std::string& name,
                     ast::StorageClass storage,
                     ast::type::Type* type);

  /// @param func the function name
  /// @param args the function call arguments
  /// @returns a `ast::CallExpression` to the function `func`, with the
  /// arguments of `args` converted to `ast::Expression`s using `Expr()`.
  template <typename... ARGS>
  ast::CallExpression Call(const std::string& func, ARGS&&... args) {
    return ast::CallExpression{Expr(func),
                               ExprList(std::forward<ARGS>(args)...)};
  }

  /// Creates a new `ast::Node` owned by the Context. When the Context is
  /// destructed, the `ast::Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return ctx->create<T>(std::forward<ARGS>(args)...);
  }

  /// The builder context
  tint::Context* const ctx;
  /// The builder types
  const TypesBuilder ty;

 protected:
  /// Called whenever a new variable is built with `Var()`.
  virtual void OnVariableBuilt(ast::Variable*) {}
};

/// BuilderWithContext is a `Builder` that constructs and owns its `Context`.
class BuilderWithContext : public Builder {
 public:
  BuilderWithContext();
  ~BuilderWithContext() override;
};

//! @cond Doxygen_Suppress
// Various template specializations for TypesBuilder::CToAST.
template <>
struct TypesBuilder::CToAST<Builder::i32> {
  static ast::type::Type* get(const TypesBuilder* t) { return t->i32; }
};
template <>
struct TypesBuilder::CToAST<Builder::u32> {
  static ast::type::Type* get(const TypesBuilder* t) { return t->u32; }
};
template <>
struct TypesBuilder::CToAST<Builder::f32> {
  static ast::type::Type* get(const TypesBuilder* t) { return t->f32; }
};
template <>
struct TypesBuilder::CToAST<bool> {
  static ast::type::Type* get(const TypesBuilder* t) { return t->bool_; }
};
template <>
struct TypesBuilder::CToAST<void> {
  static ast::type::Type* get(const TypesBuilder* t) { return t->void_; }
};
//! @endcond

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BUILDER_H_
