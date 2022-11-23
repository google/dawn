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

    /// @param ty the target type - must be an array or initializer
    /// @param args the input arguments
    /// @return the constructed value, or null if the value cannot be calculated
    Result ArrayOrStructInit(const sem::Type* ty, utils::VectorRef<const sem::Expression*> args);

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
    /// @param source the source location
    /// @return the converted value, or null if the value cannot be calculated
    Result Convert(const sem::Type* ty, const sem::Constant* value, const Source& source);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constant value evaluation methods, to be indirectly called via the intrinsic table
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// Type conversion
    /// @param ty the result type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the converted value, or null if the value cannot be calculated
    Result Conv(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Zero value type initializer
    /// @param ty the result type
    /// @param args the input arguments (no arguments provided)
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result Zero(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Identity value type initializer
    /// @param ty the result type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result Identity(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector splat initializer
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecSplat(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector initializer using scalars
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecInitS(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Vector initializer using a mix of scalars and smaller vectors
    /// @param ty the vector type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result VecInitM(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Matrix initializer using scalar values
    /// @param ty the matrix type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result MatInitS(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Matrix initializer using column vectors
    /// @param ty the matrix type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the constructed value, or null if the value cannot be calculated
    Result MatInitV(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    ////////////////////////////////////////////////////////////////////////////
    // Unary Operators
    ////////////////////////////////////////////////////////////////////////////

    /// Complement operator '~'
    /// @param ty the integer type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpComplement(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// Unary minus operator '-'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpUnaryMinus(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// Unary not operator '!'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
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
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpPlus(const sem::Type* ty,
                  utils::VectorRef<const sem::Constant*> args,
                  const Source& source);

    /// Minus operator '-'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpMinus(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// Multiply operator '*' for the same type on the LHS and RHS
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiply(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Multiply operator '*' for matCxR<T> * vecC<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyMatVec(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Multiply operator '*' for vecR<T> * matCxR<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyVecMat(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Multiply operator '*' for matKxR<T> * matCxK<T>
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpMultiplyMatMat(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// Divide operator '/'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpDivide(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// Equality operator '=='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpEqual(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// Inequality operator '!='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpNotEqual(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Less than operator '<'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpLessThan(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// Greater than operator '>'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpGreaterThan(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

    /// Less than or equal operator '<='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpLessThanEqual(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// Greater than or equal operator '>='
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpGreaterThanEqual(const sem::Type* ty,
                              utils::VectorRef<const sem::Constant*> args,
                              const Source& source);

    /// Bitwise and operator '&'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpAnd(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// Bitwise or operator '|'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpOr(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// Bitwise xor operator '^'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpXor(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// Bitwise shift left operator '<<'
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result OpShiftLeft(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    ////////////////////////////////////////////////////////////////////////////
    // Builtins
    ////////////////////////////////////////////////////////////////////////////

    /// abs builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result abs(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// acos builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result acos(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// acosh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result acosh(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// all builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result all(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// any builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result any(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// asin builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result asin(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// asinh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result asinh(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// atan builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result atan(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// atanh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result atanh(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// atan2 builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result atan2(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// ceil builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result ceil(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// clamp builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result clamp(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// cos builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result cos(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// cosh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result cosh(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// countLeadingZeros builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result countLeadingZeros(const sem::Type* ty,
                             utils::VectorRef<const sem::Constant*> args,
                             const Source& source);

    /// countOneBits builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result countOneBits(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// countTrailingZeros builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result countTrailingZeros(const sem::Type* ty,
                              utils::VectorRef<const sem::Constant*> args,
                              const Source& source);

    /// cross builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result cross(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result degrees(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// dot builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result dot(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// extractBits builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result extractBits(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    /// firstLeadingBit builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result firstLeadingBit(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// firstTrailingBit builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result firstTrailingBit(const sem::Type* ty,
                            utils::VectorRef<const sem::Constant*> args,
                            const Source& source);

    /// floor builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result floor(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// insertBits builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result insertBits(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// length builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result length(const sem::Type* ty,
                  utils::VectorRef<const sem::Constant*> args,
                  const Source& source);

    /// max builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result max(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// min builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result min(const sem::Type* ty,  // NOLINT(build/include_what_you_use)  -- confused by min
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// pack2x16float builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result pack2x16float(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

    /// pack2x16snorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result pack2x16snorm(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

    /// pack2x16unorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result pack2x16unorm(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

    /// pack4x8snorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result pack4x8snorm(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// pack4x8unorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result pack4x8unorm(const sem::Type* ty,
                        utils::VectorRef<const sem::Constant*> args,
                        const Source& source);

    /// radians builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location of the conversion
    /// @return the result value, or null if the value cannot be calculated
    Result radians(const sem::Type* ty,
                   utils::VectorRef<const sem::Constant*> args,
                   const Source& source);

    /// reverseBits builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result reverseBits(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    /// round builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result round(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// saturate builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result saturate(const sem::Type* ty,
                    utils::VectorRef<const sem::Constant*> args,
                    const Source& source);

    /// select builtin with single bool third arg
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result select_bool(const sem::Type* ty,
                       utils::VectorRef<const sem::Constant*> args,
                       const Source& source);

    /// select builtin with vector of bool third arg
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result select_boolvec(const sem::Type* ty,
                          utils::VectorRef<const sem::Constant*> args,
                          const Source& source);

    /// sign builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result sign(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// sin builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result sin(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// sinh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result sinh(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// smoothstep builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result smoothstep(const sem::Type* ty,
                      utils::VectorRef<const sem::Constant*> args,
                      const Source& source);

    /// step builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result step(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// sqrt builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result sqrt(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// tan builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result tan(const sem::Type* ty,
               utils::VectorRef<const sem::Constant*> args,
               const Source& source);

    /// tanh builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result tanh(const sem::Type* ty,
                utils::VectorRef<const sem::Constant*> args,
                const Source& source);

    /// trunc builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result trunc(const sem::Type* ty,
                 utils::VectorRef<const sem::Constant*> args,
                 const Source& source);

    /// unpack2x16float builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result unpack2x16float(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// unpack2x16snorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result unpack2x16snorm(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// unpack2x16unorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result unpack2x16unorm(const sem::Type* ty,
                           utils::VectorRef<const sem::Constant*> args,
                           const Source& source);

    /// unpack4x8snorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result unpack4x8snorm(const sem::Type* ty,
                          utils::VectorRef<const sem::Constant*> args,
                          const Source& source);

    /// unpack4x8unorm builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result unpack4x8unorm(const sem::Type* ty,
                          utils::VectorRef<const sem::Constant*> args,
                          const Source& source);

    /// quantizeToF16 builtin
    /// @param ty the expression type
    /// @param args the input arguments
    /// @param source the source location
    /// @return the result value, or null if the value cannot be calculated
    Result quantizeToF16(const sem::Type* ty,
                         utils::VectorRef<const sem::Constant*> args,
                         const Source& source);

  private:
    /// Adds the given error message to the diagnostics
    void AddError(const std::string& msg, const Source& source) const;

    /// Adds the given warning message to the diagnostics
    void AddWarning(const std::string& msg, const Source& source) const;

    /// Adds the given note message to the diagnostics
    void AddNote(const std::string& msg, const Source& source) const;

    /// Adds two Number<T>s
    /// @param source the source location
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Add(const Source& source, NumberT a, NumberT b);

    /// Subtracts two Number<T>s
    /// @param source the source location
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Sub(const Source& source, NumberT a, NumberT b);

    /// Multiplies two Number<T>s
    /// @param source the source location
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Mul(const Source& source, NumberT a, NumberT b);

    /// Divides two Number<T>s
    /// @param source the source location
    /// @param a the lhs number
    /// @param b the rhs number
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Div(const Source& source, NumberT a, NumberT b);

    /// Returns the dot product of (a1,a2) with (b1,b2)
    /// @param source the source location
    /// @param a1 component 1 of lhs vector
    /// @param a2 component 2 of lhs vector
    /// @param b1 component 1 of rhs vector
    /// @param b2 component 2 of rhs vector
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Dot2(const Source& source,
                                NumberT a1,
                                NumberT a2,
                                NumberT b1,
                                NumberT b2);

    /// Returns the dot product of (a1,a2,a3) with (b1,b2,b3)
    /// @param source the source location
    /// @param a1 component 1 of lhs vector
    /// @param a2 component 2 of lhs vector
    /// @param a3 component 3 of lhs vector
    /// @param b1 component 1 of rhs vector
    /// @param b2 component 2 of rhs vector
    /// @param b3 component 3 of rhs vector
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Dot3(const Source& source,
                                NumberT a1,
                                NumberT a2,
                                NumberT a3,
                                NumberT b1,
                                NumberT b2,
                                NumberT b3);

    /// Returns the dot product of (a1,b1,c1,d1) with (a2,b2,c2,d2)
    /// @param source the source location
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
    utils::Result<NumberT> Dot4(const Source& source,
                                NumberT a1,
                                NumberT a2,
                                NumberT a3,
                                NumberT a4,
                                NumberT b1,
                                NumberT b2,
                                NumberT b3,
                                NumberT b4);

    /// Returns the determinant of the 2x2 matrix [(a1, a2), (b1, b2)]
    /// @param source the source location
    /// @param a1 component 1 of the first column vector
    /// @param a2 component 2 of the first column vector
    /// @param b1 component 1 of the second column vector
    /// @param b2 component 2 of the second column vector
    template <typename NumberT>
    utils::Result<NumberT> Det2(const Source& source,
                                NumberT a1,
                                NumberT a2,
                                NumberT b1,
                                NumberT b2);

    template <typename NumberT>
    utils::Result<NumberT> Sqrt(const Source& source, NumberT v);

    /// Clamps e between low and high
    /// @param source the source location
    /// @param e the number to clamp
    /// @param low the lower bound
    /// @param high the upper bound
    /// @returns the result number on success, or logs an error and returns Failure
    template <typename NumberT>
    utils::Result<NumberT> Clamp(const Source& source, NumberT e, NumberT low, NumberT high);

    /// Returns a callable that calls Add, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto AddFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Sub, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto SubFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Mul, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto MulFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Div, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto DivFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Dot2, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot2Func(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Dot3, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot3Func(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Dot4, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Dot4Func(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Det2, and creates a Constant with its result of type `elem_ty`
    /// if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto Det2Func(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls Clamp, and creates a Constant with its result of type
    /// `elem_ty` if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto ClampFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns a callable that calls SqrtFunc, and creates a Constant with its
    /// result of type `elem_ty` if successful, or returns Failure otherwise.
    /// @param source the source location
    /// @param elem_ty the element type of the Constant to create on success
    /// @returns the callable function
    auto SqrtFunc(const Source& source, const sem::Type* elem_ty);

    /// Returns the dot product of v1 and v2.
    /// @param source the source location
    /// @param v1 the first vector
    /// @param v2 the second vector
    /// @returns the dot product
    Result Dot(const Source& source, const sem::Constant* v1, const sem::Constant* v2);

    ProgramBuilder& builder;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_CONST_EVAL_H_
