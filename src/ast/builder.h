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

#include "src/ast/array_accessor_expression.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/expression.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// TypesBuilder holds basic `tint` types and methods for constructing
/// complex types.
class TypesBuilder {
 public:
  /// Constructor
  /// @param mod the module
  explicit TypesBuilder(Module* mod);

  /// A boolean type
  type::Bool* const bool_;
  /// A f32 type
  type::F32* const f32;
  /// A i32 type
  type::I32* const i32;
  /// A u32 type
  type::U32* const u32;
  /// A void type
  type::Void* const void_;

  /// @return the tint AST type for the C type `T`.
  template <typename T>
  type::Type* Of() const {
    return CToAST<T>::get(this);
  }

  /// @return the tint AST type for a 2-element vector of the C type `T`.
  template <typename T>
  type::Vector* vec2() const {
    return mod_->create<type::Vector>(Of<T>(), 2);
  }

  /// @return the tint AST type for a 3-element vector of the C type `T`.
  template <typename T>
  type::Vector* vec3() const {
    return mod_->create<type::Vector>(Of<T>(), 3);
  }

  /// @return the tint AST type for a 4-element vector of the C type `T`.
  template <typename T>
  type::Type* vec4() const {
    return mod_->create<type::Vector>(Of<T>(), 4);
  }

  /// @return the tint AST type for a 2x3 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat2x2() const {
    return mod_->create<type::Matrix>(Of<T>(), 2, 2);
  }

  /// @return the tint AST type for a 2x3 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat2x3() const {
    return mod_->create<type::Matrix>(Of<T>(), 3, 2);
  }

  /// @return the tint AST type for a 2x4 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat2x4() const {
    return mod_->create<type::Matrix>(Of<T>(), 4, 2);
  }

  /// @return the tint AST type for a 3x2 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat3x2() const {
    return mod_->create<type::Matrix>(Of<T>(), 2, 3);
  }

  /// @return the tint AST type for a 3x3 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat3x3() const {
    return mod_->create<type::Matrix>(Of<T>(), 3, 3);
  }

  /// @return the tint AST type for a 3x4 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat3x4() const {
    return mod_->create<type::Matrix>(Of<T>(), 4, 3);
  }

  /// @return the tint AST type for a 4x2 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat4x2() const {
    return mod_->create<type::Matrix>(Of<T>(), 2, 4);
  }

  /// @return the tint AST type for a 4x3 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat4x3() const {
    return mod_->create<type::Matrix>(Of<T>(), 3, 4);
  }

  /// @return the tint AST type for a 4x4 matrix of the C type `T`.
  template <typename T>
  type::Matrix* mat4x4() const {
    return mod_->create<type::Matrix>(Of<T>(), 4, 4);
  }

  /// @param subtype the array element type
  /// @param n the array size. 0 represents unbounded
  /// @return the tint AST type for a array of size `n` of type `T`
  type::Array* array(type::Type* subtype, uint32_t n) const {
    return mod_->create<type::Array>(subtype, n, ArrayDecorationList{});
  }

  /// @return the tint AST type for an array of size `N` of type `T`
  template <typename T, int N = 0>
  type::Array* array() const {
    return array(Of<T>(), N);
  }

  /// @return the tint AST pointer to type `T` with the given StorageClass.
  /// @param storage_class the storage class of the pointer
  template <typename T>
  type::Pointer* pointer(StorageClass storage_class) const {
    return mod_->create<type::Pointer>(Of<T>(), storage_class);
  }

 private:
  /// CToAST<T> is specialized for various `T` types and each specialization
  /// contains a single static `get()` method for obtaining the corresponding
  /// AST type for the C type `T`.
  /// `get()` has the signature:
  ///    `static type::Type* get(Types* t)`
  template <typename T>
  struct CToAST {};

  Module* const mod_;
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
  /// @param mod the module to use in the builder
  explicit Builder(Module* mod);
  virtual ~Builder();

  /// @param expr the expression
  /// @return expr
  Expression* Expr(Expression* expr) { return expr; }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  IdentifierExpression* Expr(const std::string& name) {
    return create<IdentifierExpression>(Source{}, mod->RegisterSymbol(name),
                                        name);
  }

  /// @param name the identifier name
  /// @return an IdentifierExpression with the given name
  IdentifierExpression* Expr(const char* name) {
    return create<IdentifierExpression>(Source{}, mod->RegisterSymbol(name),
                                        name);
  }

  /// @param value the boolean value
  /// @return a Scalar constructor for the given value
  ScalarConstructorExpression* Expr(bool value) {
    return create<ScalarConstructorExpression>(Source{}, Literal(value));
  }

  /// @param value the float value
  /// @return a Scalar constructor for the given value
  ScalarConstructorExpression* Expr(f32 value) {
    return create<ScalarConstructorExpression>(Source{}, Literal(value));
  }

  /// @param value the integer value
  /// @return a Scalar constructor for the given value
  ScalarConstructorExpression* Expr(i32 value) {
    return create<ScalarConstructorExpression>(Source{}, Literal(value));
  }

  /// @param value the unsigned int value
  /// @return a Scalar constructor for the given value
  ScalarConstructorExpression* Expr(u32 value) {
    return create<ScalarConstructorExpression>(Source{}, Literal(value));
  }

  /// Converts `arg` to an `Expression` using `Expr()`, then appends it to
  /// `list`.
  /// @param list the list to append too
  /// @param arg the arg to create
  template <typename ARG>
  void Append(ExpressionList& list, ARG&& arg) {
    list.emplace_back(Expr(std::forward<ARG>(arg)));
  }

  /// Converts `arg0` and `args` to `Expression`s using `Expr()`,
  /// then appends them to `list`.
  /// @param list the list to append too
  /// @param arg0 the first argument
  /// @param args the rest of the arguments
  template <typename ARG0, typename... ARGS>
  void Append(ExpressionList& list, ARG0&& arg0, ARGS&&... args) {
    Append(list, std::forward<ARG0>(arg0));
    Append(list, std::forward<ARGS>(args)...);
  }

  /// @return an empty list of expressions,
  ExpressionList ExprList() { return {}; }

  /// @param args the list of expressions
  /// @return the list of expressions converted to `Expression`s using
  /// `Expr()`,
  template <typename... ARGS>
  ExpressionList ExprList(ARGS&&... args) {
    ExpressionList list;
    list.reserve(sizeof...(args));
    Append(list, std::forward<ARGS>(args)...);
    return list;
  }

  /// @param val the boolan value
  /// @return a boolean literal with the given value
  BoolLiteral* Literal(bool val) {
    return create<BoolLiteral>(Source{}, ty.bool_, val);
  }

  /// @param val the float value
  /// @return a float literal with the given value
  FloatLiteral* Literal(f32 val) {
    return create<FloatLiteral>(Source{}, ty.f32, val);
  }

  /// @param val the unsigned int value
  /// @return a UintLiteral with the given value
  UintLiteral* Literal(u32 val) {
    return create<UintLiteral>(Source{}, ty.u32, val);
  }

  /// @param val the integer value
  /// @return the SintLiteral with the given value
  SintLiteral* Literal(i32 val) {
    return create<SintLiteral>(Source{}, ty.i32, val);
  }

  /// @param args the arguments for the type constructor
  /// @return an `TypeConstructorExpression` of type `ty`, with the values
  /// of `args` converted to `Expression`s using `Expr()`
  template <typename T, typename... ARGS>
  TypeConstructorExpression* Construct(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.Of<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param type the type to construct
  /// @param args the arguments for the constructor
  /// @return an `TypeConstructorExpression` of `type` constructed with the
  /// values `args`.
  template <typename... ARGS>
  TypeConstructorExpression* Construct(type::Type* type, ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, type, ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `TypeConstructorExpression` of a 2-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* vec2(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.vec2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `TypeConstructorExpression` of a 3-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* vec3(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.vec3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the vector constructor
  /// @return an `TypeConstructorExpression` of a 4-element vector of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* vec4(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.vec4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 2x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat2x2(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat2x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 2x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat2x3(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat2x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 2x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat2x4(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat2x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 3x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat3x2(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat3x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 3x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat3x3(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat3x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 3x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat3x4(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat3x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 4x2 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat4x2(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat4x2<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 4x3 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat4x3(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat4x3<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the matrix constructor
  /// @return an `TypeConstructorExpression` of a 4x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, typename... ARGS>
  TypeConstructorExpression* mat4x4(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.mat4x4<T>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param args the arguments for the array constructor
  /// @return an `TypeConstructorExpression` of a 4x4 matrix of type
  /// `T`, constructed with the values `args`.
  template <typename T, int N = 0, typename... ARGS>
  TypeConstructorExpression* array(ARGS&&... args) {
    return create<TypeConstructorExpression>(
        Source{}, ty.array<T, N>(), ExprList(std::forward<ARGS>(args)...));
  }

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @returns a `Variable` with the given name, storage and type. The variable
  /// will be built with a nullptr constructor and no decorations.
  Variable* Var(const std::string& name,
                StorageClass storage,
                type::Type* type);

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @param constructor constructor expression
  /// @param decorations variable decorations
  /// @returns a `Variable` with the given name, storage and type
  Variable* Var(const std::string& name,
                StorageClass storage,
                type::Type* type,
                Expression* constructor,
                VariableDecorationList decorations);

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @returns a constant `Variable` with the given name, storage and type. The
  /// variable will be built with a nullptr constructor and no decorations.
  Variable* Const(const std::string& name,
                  StorageClass storage,
                  type::Type* type);

  /// @param name the variable name
  /// @param storage the variable storage class
  /// @param type the variable type
  /// @param constructor optional constructor expression
  /// @param decorations optional variable decorations
  /// @returns a constant `Variable` with the given name, storage and type
  Variable* Const(const std::string& name,
                  StorageClass storage,
                  type::Type* type,
                  Expression* constructor,
                  VariableDecorationList decorations);

  /// @param func the function name
  /// @param args the function call arguments
  /// @returns a `CallExpression` to the function `func`, with the
  /// arguments of `args` converted to `Expression`s using `Expr()`.
  template <typename... ARGS>
  CallExpression Call(const std::string& func, ARGS&&... args) {
    return CallExpression(Source{}, Expr(func),
                          ExprList(std::forward<ARGS>(args)...));
  }

  /// @param lhs the left hand argument to the addition operation
  /// @param rhs the right hand argument to the addition operation
  /// @returns a `BinaryExpression` summing the arguments `lhs` and `rhs`
  template <typename LHS, typename RHS>
  Expression* Add(LHS&& lhs, RHS&& rhs) {
    return create<BinaryExpression>(Source{}, ast::BinaryOp::kAdd,
                                    Expr(std::forward<LHS>(lhs)),
                                    Expr(std::forward<RHS>(rhs)));
  }

  /// @param lhs the left hand argument to the subtraction operation
  /// @param rhs the right hand argument to the subtraction operation
  /// @returns a `BinaryExpression` subtracting `rhs` from `lhs`
  template <typename LHS, typename RHS>
  Expression* Sub(LHS&& lhs, RHS&& rhs) {
    return create<BinaryExpression>(Source{}, ast::BinaryOp::kSubtract,
                                    Expr(std::forward<LHS>(lhs)),
                                    Expr(std::forward<RHS>(rhs)));
  }

  /// @param arr the array argument for the array accessor expression
  /// @param idx the index argument for the array accessor expression
  /// @returns a `ArrayAccessorExpression` that indexes `arr` with `idx`
  template <typename ARR, typename IDX>
  Expression* Index(ARR&& arr, IDX&& idx) {
    return create<ArrayAccessorExpression>(
        Source{}, Expr(std::forward<ARR>(arr)), Expr(std::forward<IDX>(idx)));
  }

  /// Creates a new `Node` owned by the Module. When the Module is
  /// destructed, the `Node` will also be destructed.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) {
    return mod->create<T>(std::forward<ARGS>(args)...);
  }

  /// The builder module
  Module* const mod;
  /// The builder types
  const TypesBuilder ty;

 protected:
  /// Called whenever a new variable is built with `Var()`.
  virtual void OnVariableBuilt(Variable*) {}
};

/// BuilderWithModule is a `Builder` that constructs and owns its `Module`.
class BuilderWithModule : public Builder {
 public:
  BuilderWithModule();
  ~BuilderWithModule() override;
};

//! @cond Doxygen_Suppress
// Various template specializations for TypesBuilder::CToAST.
template <>
struct TypesBuilder::CToAST<Builder::i32> {
  static type::Type* get(const TypesBuilder* t) { return t->i32; }
};
template <>
struct TypesBuilder::CToAST<Builder::u32> {
  static type::Type* get(const TypesBuilder* t) { return t->u32; }
};
template <>
struct TypesBuilder::CToAST<Builder::f32> {
  static type::Type* get(const TypesBuilder* t) { return t->f32; }
};
template <>
struct TypesBuilder::CToAST<bool> {
  static type::Type* get(const TypesBuilder* t) { return t->bool_; }
};
template <>
struct TypesBuilder::CToAST<void> {
  static type::Type* get(const TypesBuilder* t) { return t->void_; }
};
//! @endcond

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_BUILDER_H_
