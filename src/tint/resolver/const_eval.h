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

#ifndef SRC_TINT_RESOLVER_CONST_EVAL_H_
#define SRC_TINT_RESOLVER_CONST_EVAL_H_

#include <stddef.h>
#include <string>

#include "src/tint/utils/result.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
class Source;
}  // namespace tint
namespace tint::ast {
class LiteralExpression;
}  // namespace tint::ast
namespace tint::sem {
class Constant;
class Expression;
class StructMember;
class Type;
}  // namespace tint::sem

namespace tint::resolver {

/// ConstEval performs shader creation-time (const-expression) expression evaluation.
/// Methods are called from the resolver, either directly or via member-function pointers indexed by
/// the IntrinsicTable. All child-expression nodes are guaranteed to have been already resolved
/// before calling a method to evaluate an expression's value.
class ConstEval {
  public:
    /// The result type of a method that may raise a diagnostic error and the caller should abort
    /// resolving. Can be one of three distinct values:
    /// * A non-null sem::Constant pointer. Returned when a expression resolves to a creation time
    ///   value.
    /// * A null sem::Constant pointer. Returned when a expression cannot resolve to a creation time
    ///   value, but is otherwise legal.
    /// * `utils::Failure`. Returned when there was a resolver error. In this situation the method
    ///   will have already reported a diagnostic error message, and the caller should abort
    ///   resolving.
    using Result = utils::Result<const sem::Constant*>;

    /// Typedef for a constant evaluation function
    using Function = Result (ConstEval::*)(const sem::Type* result_ty,
                                           utils::VectorRef<const sem::Constant*>,
                                           const Source&);

    /// Constructor
    /// @param b the program builder
    explicit ConstEval(ProgramBuilder& b);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constant value evaluation methods, to be called directly from Resolver
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// @param ty the target type - must be an array or constructor
    /// @param args the input arguments
    /// @return the constructed value, or null if the value cannot be calculated
    Result ArrayOrStructCtor(const sem::Type* ty, utils::VectorRef<const sem::Expression*> args);

    /// @param ty the target type
    /// @param expr the input expression
    /// @return the bit-cast of the given expression to the given type, or null if the value cannot
    ///         be calculated
    Result Bitcast(const sem::Type* ty, const sem::Expression* expr);

    /// @param obj the object being indexed
    /// @param idx the index expression
    /// @return the result of the index, or null if the value cannot be calculated
    Result Index(const sem::Expression* obj, const sem::Expression* idx);

    /// @param ty the result type
    /// @param lit the literal AST node
    /// @return the constant value of the literal
    Result Literal(const sem::Type* ty, const ast::LiteralExpression* lit);

    /// @param obj the object being accessed
    /// @param member the member
    /// @return the result of the member access, or null if the value cannot be calculated
    Result MemberAccess(const sem::Expression* obj, const sem::StructMember* member);

    /// @param ty the result type
    /// @param vector the vector being swizzled
    /// @param indices the swizzle indices
    /// @return the result of the swizzle, or null if the value cannot be calculated
    Result Swizzle(const sem::Type* ty,
                   const sem::Expression* vector,
                   utils::VectorRef<uint32_t> indices);

    /// Convert the `value` to `target_type`
    /// @param ty the result type
    /// @param value the value being converted
    /// @param source the source location of the conversion
    /// @return the converted value, or null if the value cannot be calculated
    Result Convert(const sem::Type* ty, const sem::Constant* value, const Source& source);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constant value evaluation methods, to be indirectly called via the intrinsic table
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// Type conversion
    /// @param ty the result type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the converted value, or null if the value cannot be calculated
    Result Conv(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Zero value type constructor
    /// @param ty the result type
    /// @param args the input arguments (no arguments provided)
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result Zero(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Identity value type constructor
    /// @param ty the result type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result Identity(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector splat constructor
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecSplat(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector constructor using scalars
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecCtorS(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector constructor using a mix of scalars and smaller vectors
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecCtorM(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Matrix constructor using scalar values
    /// @param ty the matrix type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result MatCtorS(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Matrix constructor using column vectors
    /// @param ty the matrix type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the constructed value, or null if the value cannot be calculated
    Result MatCtorV(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    ////////////////////////////////////////////////////////////////////////////
    // Unary Operators
    ////////////////////////////////////////////////////////////////////////////

    /// Complement operator '~'
    /// @param ty the integer type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpComplement(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// Unary minus operator '-'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpUnaryMinus(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// Unary not operator '!'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpNot(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    ////////////////////////////////////////////////////////////////////////////
    // Binary Operators
    ////////////////////////////////////////////////////////////////////////////

    /// Plus operator '+'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpPlus(const sem::Type* ty,
                  utils::VectorRef<const sem::Constant*> args,
                  const Source& source);

    /// Minus operator '-'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpMinus(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// Multiply operator '*' for the same type on the LHS and RHS
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiply(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Multiply operator '*' for matCxR<T> * vecC<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyMatVec(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Multiply operator '*' for vecR<T> * matCxR<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyVecMat(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Multiply operator '*' for matKxR<T> * matCxK<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyMatMat(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Divide operator '/'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpDivide(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Equality operator '=='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpEqual(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// Inequality operator '!='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpNotEqual(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Less than operator '<'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpLessThan(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Greater than operator '>'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpGreaterThan(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

    /// Less than or equal operator '<='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpLessThanEqual(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// Greater than or equal operator '>='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpGreaterThanEqual(const sem::Type* ty,
                              utils::VectorRef<const sem::Constant*> args,
                              const Source& source);

    /// Bitwise and operator '&'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpAnd(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// Bitwise or operator '|'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpOr(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Bitwise xor operator '^'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpXor(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// Bitwise shift left operator '<<'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result OpShiftLeft(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    ////////////////////////////////////////////////////////////////////////////
    // Builtins
    ////////////////////////////////////////////////////////////////////////////

    /// atan2 builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result atan2(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// clamp builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result clamp(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// select builtin with single bool third arg
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result select_bool(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    /// select builtin with vector of bool third arg
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result select_boolvec(const sem::Type* ty,
                          utils::VectorRef<const sem::Constant*> args,
                          const Source& source);

  private:
    /// Adds the given error message to the diagnostics
    void AddError(const std::string& msg, const Source& source) const;

    /// Adds the given warning message to the diagnostics
    void AddWarning(const std::string& msg, const Source& source) const;

    /// Adds two Number<T>s
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Add(NumberT a, NumberT b);

    /// Multiplies two Number<T>s
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Mul(NumberT a, NumberT b);

    /// Returns the dot product of (a1,a2) with (b1,b2)
    /// @param a1 component 1 of lhs vector
    /// @param a2 component 2 of lhs vector
    /// @param b1 component 1 of rhs vector
    /// @param b2 component 2 of rhs vector
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Dot2(NumberT a1, NumberT a2, NumberT b1, NumberT b2);

    /// Returns the dot product of (a1,a2,a3) with (b1,b2,b3)
    /// @param a1 component 1 of lhs vector
    /// @param a2 component 2 of lhs vector
    /// @param a3 component 3 of lhs vector
    /// @param b1 component 1 of rhs vector
    /// @param b2 component 2 of rhs vector
    /// @param b3 component 3 of rhs vector
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Dot3(NumberT a1,
                                NumberT a2,
                                NumberT a3,
                                NumberT b1,
                                NumberT b2,
                                NumberT b3);

    /// Returns the dot product of (a1,b1,c1,d1) with (a2,b2,c2,d2)
    /// @param a1 component 1 of lhs vector
    /// @param a2 component 2 of lhs vector
    /// @param a3 component 3 of lhs vector
    /// @param a4 component 4 of lhs vector
    /// @param b1 component 1 of rhs vector
    /// @param b2 component 2 of rhs vector
    /// @param b3 component 3 of rhs vector
    /// @param b4 component 4 of rhs vector
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Dot4(NumberT a1,
                                NumberT a2,
                                NumberT a3,
                                NumberT a4,
                                NumberT b1,
                                NumberT b2,
                                NumberT b3,
                                NumberT b4);

    /// Returns a callable that calls Add, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto AddFunc(const sem::Type* elem_ty);

    /// Returns a callable that calls Mul, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto MulFunc(const sem::Type* elem_ty);

    /// Returns a callable that calls Dot2, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot2Func(const sem::Type* elem_ty);

    /// Returns a callable that calls Dot3, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot3Func(const sem::Type* elem_ty);

    /// Returns a callable that calls Dot4, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot4Func(const sem::Type* elem_ty);

    ProgramBuilder& builder;
    const Source* current_source = nullptr;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_CONST_EVAL_H_
