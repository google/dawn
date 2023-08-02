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

#ifndef SRC_TINT_LANG_CORE_IR_BUILDER_H_
#define SRC_TINT_LANG_CORE_IR_BUILDER_H_

#include <utility>

#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/intrinsic_call.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::ir {

/// Builds an ir::Module
class Builder {
    /// Evaluates to true if T is a non-reference instruction pointer.
    template <typename T>
    static constexpr bool IsNonRefInstPtr =
        std::is_pointer_v<T> && std::is_base_of_v<ir::Instruction, std::remove_pointer_t<T>>;

    /// static_assert()s that ARGS contains no more than one non-reference instruction pointer.
    /// This is used to detect patterns where C++ non-deterministic evaluation order may cause
    /// instruction ordering bugs.
    template <typename... ARGS>
    static constexpr void CheckForNonDeterministicEvaluation() {
        constexpr bool possibly_non_deterministic_eval =
            ((IsNonRefInstPtr<ARGS> ? 1 : 0) + ...) > 1;
        static_assert(!possibly_non_deterministic_eval,
                      "Detected possible non-deterministic ordering of instructions. "
                      "Consider hoisting Builder call arguments to separate statements.");
    }

    /// A helper used to enable overloads if the first type in `TYPES` is a Vector or
    /// VectorRef.
    template <typename... TYPES>
    using EnableIfVectorLike = tint::traits::EnableIf<
        tint::IsVectorLike<tint::traits::Decay<tint::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is a Vector or
    /// VectorRef.
    template <typename... TYPES>
    using DisableIfVectorLike = tint::traits::EnableIf<
        !tint::IsVectorLike<tint::traits::Decay<tint::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// If set, any created instruction will be auto-appended to the block.
    ir::Block* current_block_ = nullptr;

    /// If set, any created instruction will be auto-inserted before this instruction.
    ir::Instruction* current_insertion_point_ = nullptr;

  public:
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    explicit Builder(Module& mod);
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    /// @param block the block to insert too
    Builder(Module& mod, ir::Block* block);
    /// Destructor
    ~Builder();

    /// Creates a new builder that will append to the given block
    /// @param b the block to append new instructions to
    /// @returns the builder
    Builder Append(ir::Block* b) { return Builder(ir, b); }

    /// Calls @p cb with the builder appending to block @p b
    /// @param b the block to set as the block to append to
    /// @param cb the function to call with the builder appending to block @p b
    template <typename FUNCTION>
    void Append(ir::Block* b, FUNCTION&& cb) {
        TINT_SCOPED_ASSIGNMENT(current_block_, b);
        TINT_SCOPED_ASSIGNMENT(current_insertion_point_, nullptr);
        cb();
    }

    /// Calls @p cb with the builder inserting before @p ip
    /// @param ip the insertion point for new instructions
    /// @param cb the function to call with the builder inserting new instructions before @p ip
    template <typename FUNCTION>
    void InsertBefore(ir::Instruction* ip, FUNCTION&& cb) {
        TINT_SCOPED_ASSIGNMENT(current_block_, nullptr);
        TINT_SCOPED_ASSIGNMENT(current_insertion_point_, ip);
        cb();
    }

    /// Appends and returns the instruction @p val to the current insertion point. If there is no
    /// current insertion point set, then @p val is just returned.
    /// @param val the instruction to append
    /// @returns the instruction
    template <typename T>
    T* Append(T* val) {
        if (current_insertion_point_) {
            val->InsertBefore(current_insertion_point_);
        } else if (current_block_) {
            current_block_->Append(val);
        }
        return val;
    }

    /// @returns a new block
    ir::Block* Block();

    /// @returns a new multi-in block
    ir::MultiInBlock* MultiInBlock();

    /// Creates a function instruction
    /// @param name the function name
    /// @param return_type the function return type
    /// @param stage the function stage
    /// @param wg_size the workgroup_size
    /// @returns the instruction
    ir::Function* Function(std::string_view name,
                           const type::Type* return_type,
                           Function::PipelineStage stage = Function::PipelineStage::kUndefined,
                           std::optional<std::array<uint32_t, 3>> wg_size = {});

    /// Creates an if instruction
    /// @param condition the if condition
    /// @returns the instruction
    template <typename T>
    ir::If* If(T&& condition) {
        auto* cond_val = Value(std::forward<T>(condition));
        return Append(ir.instructions.Create<ir::If>(cond_val, Block(), Block()));
    }

    /// Creates a loop instruction
    /// @returns the instruction
    ir::Loop* Loop();

    /// Creates a switch instruction
    /// @param condition the switch condition
    /// @returns the instruction
    template <typename T>
    ir::Switch* Switch(T&& condition) {
        auto* cond_val = Value(std::forward<T>(condition));
        return Append(ir.instructions.Create<ir::Switch>(cond_val));
    }

    /// Creates a case for the switch @p s with the given selectors
    /// @param s the switch to create the case into
    /// @param selectors the case selectors for the case statement
    /// @returns the start block for the case instruction
    ir::Block* Case(ir::Switch* s, VectorRef<Switch::CaseSelector> selectors);

    /// Creates a case for the switch @p s with the given selectors
    /// @param s the switch to create the case into
    /// @param selectors the case selectors for the case statement
    /// @returns the start block for the case instruction
    ir::Block* Case(ir::Switch* s, std::initializer_list<Switch::CaseSelector> selectors);

    /// Creates a new ir::Constant
    /// @param val the constant value
    /// @returns the new constant
    ir::Constant* Constant(const constant::Value* val) {
        return ir.constants.GetOrCreate(val, [&] { return ir.values.Create<ir::Constant>(val); });
    }

    /// Creates a ir::Constant for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(i32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(u32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f32 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f16 v) { return Constant(ConstantValue(v)); }

    /// Creates a ir::Constant for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    template <typename BOOL, typename = std::enable_if_t<std::is_same_v<BOOL, bool>>>
    ir::Constant* Constant(BOOL v) {
        return Constant(ConstantValue(v));
    }

    /// Retrieves the inner constant from an ir::Constant
    /// @param constant the ir constant
    /// @returns the constant::Value inside the constant
    const constant::Value* ConstantValue(ir::Constant* constant) { return constant->Value(); }

    /// Creates a constant::Value for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    const constant::Value* ConstantValue(i32 v) { return ir.constant_values.Get(v); }

    /// Creates a constant::Value for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    const constant::Value* ConstantValue(u32 v) { return ir.constant_values.Get(v); }

    /// Creates a constant::Value for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    const constant::Value* ConstantValue(f32 v) { return ir.constant_values.Get(v); }

    /// Creates a constant::Value for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    const constant::Value* ConstantValue(f16 v) { return ir.constant_values.Get(v); }

    /// Creates a constant::Value for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    template <typename BOOL, typename = std::enable_if_t<std::is_same_v<BOOL, bool>>>
    const constant::Value* ConstantValue(BOOL v) {
        return ir.constant_values.Get(v);
    }

    /// Creates a new ir::Constant
    /// @param ty the constant type
    /// @param values the composite values
    /// @returns the new constant
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    ir::Constant* Composite(const type::Type* ty, ARGS&&... values) {
        return Constant(
            ir.constant_values.Composite(ty, Vector{ConstantValue(std::forward<ARGS>(values))...}));
    }

    /// @param in the input value. One of: nullptr, ir::Value*, ir::Instruction* or a numeric value.
    /// @returns an ir::Value* from the given argument.
    template <typename T>
    ir::Value* Value(T&& in) {
        using D = std::decay_t<T>;
        constexpr bool is_null = std::is_same_v<T, std::nullptr_t>;
        constexpr bool is_ptr = std::is_pointer_v<D>;
        constexpr bool is_numeric = IsNumeric<D>;
        static_assert(is_null || is_ptr || is_numeric, "invalid argument type for Value()");

        if constexpr (is_null) {
            return nullptr;
        } else if constexpr (is_ptr) {
            using P = std::remove_pointer_t<D>;
            constexpr bool is_value = std::is_base_of_v<ir::Value, P>;
            constexpr bool is_instruction = std::is_base_of_v<ir::Instruction, P>;
            static_assert(is_value || is_instruction, "invalid pointer type for Value()");

            if constexpr (is_value) {
                return in;  /// Pass-through
            } else if constexpr (is_instruction) {
                /// Extract the first result from the instruction
                TINT_ASSERT(in->HasResults() && !in->HasMultiResults());
                return in->Result();
            }
        } else if constexpr (is_numeric) {
            /// Creates a value from the given number
            return Constant(in);
        }
    }

    /// Pass-through overload for Values() with vector-like argument
    /// @param vec the vector of ir::Value*
    /// @return @p vec
    template <typename VEC, typename = EnableIfVectorLike<tint::traits::Decay<VEC>>>
    auto Values(VEC&& vec) {
        return std::forward<VEC>(vec);
    }

    /// Overload for Values() with tint::Empty argument
    /// @return tint::Empty
    tint::EmptyType Values(tint::EmptyType) { return tint::Empty; }

    /// Overload for Values() with no arguments
    /// @return tint::Empty
    tint::EmptyType Values() { return tint::Empty; }

    /// @param args the arguments to pass to Value()
    /// @returns a vector of ir::Value* built from transforming the arguments with Value()
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    auto Values(ARGS&&... args) {
        CheckForNonDeterministicEvaluation<ARGS...>();
        return Vector{Value(std::forward<ARGS>(args))...};
    }

    /// Creates an op for `lhs kind rhs`
    /// @param kind the kind of operation
    /// @param type the result type of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Binary(enum Binary::Kind kind, const type::Type* type, LHS&& lhs, RHS&& rhs) {
        CheckForNonDeterministicEvaluation<LHS, RHS>();
        auto* lhs_val = Value(std::forward<LHS>(lhs));
        auto* rhs_val = Value(std::forward<RHS>(rhs));
        return Append(
            ir.instructions.Create<ir::Binary>(InstructionResult(type), kind, lhs_val, rhs_val));
    }

    /// Creates an And operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* And(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kAnd, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Or operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Or(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kOr, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Xor operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Xor(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kXor, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Equal operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Equal(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an NotEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* NotEqual(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kNotEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an LessThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* LessThan(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kLessThan, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* GreaterThan(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kGreaterThan, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an LessThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* LessThanEqual(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kLessThanEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an GreaterThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* GreaterThanEqual(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kGreaterThanEqual, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an ShiftLeft operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* ShiftLeft(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kShiftLeft, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an ShiftRight operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* ShiftRight(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kShiftRight, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an Add operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Add(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kAdd, type, std::forward<LHS>(lhs), std::forward<RHS>(rhs));
    }

    /// Creates an Subtract operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Subtract(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kSubtract, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an Multiply operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Multiply(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kMultiply, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an Divide operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Divide(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kDivide, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an Modulo operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Modulo(const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Binary(ir::Binary::Kind::kModulo, type, std::forward<LHS>(lhs),
                      std::forward<RHS>(rhs));
    }

    /// Creates an op for `kind val`
    /// @param kind the kind of operation
    /// @param type the result type of the binary expression
    /// @param val the value of the operation
    /// @returns the operation
    template <typename VAL>
    ir::Unary* Unary(enum Unary::Kind kind, const type::Type* type, VAL&& val) {
        auto* value = Value(std::forward<VAL>(val));
        return Append(ir.instructions.Create<ir::Unary>(InstructionResult(type), kind, value));
    }

    /// Creates a Complement operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::Unary* Complement(const type::Type* type, VAL&& val) {
        return Unary(ir::Unary::Kind::kComplement, type, std::forward<VAL>(val));
    }

    /// Creates a Negation operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::Unary* Negation(const type::Type* type, VAL&& val) {
        return Unary(ir::Unary::Kind::kNegation, type, std::forward<VAL>(val));
    }

    /// Creates a Not operation
    /// @param type the result type of the expression
    /// @param val the value
    /// @returns the operation
    template <typename VAL>
    ir::Binary* Not(const type::Type* type, VAL&& val) {
        if (auto* vec = type->As<type::Vector>()) {
            return Equal(type, std::forward<VAL>(val),
                         Constant(ir.constant_values.Splat(vec, ir.constant_values.Get(false),
                                                           vec->Width())));
        } else {
            return Equal(type, std::forward<VAL>(val), Constant(false));
        }
    }

    /// Creates a bitcast instruction
    /// @param type the result type of the bitcast
    /// @param val the value being bitcast
    /// @returns the instruction
    template <typename VAL>
    ir::Bitcast* Bitcast(const type::Type* type, VAL&& val) {
        auto* value = Value(std::forward<VAL>(val));
        return Append(ir.instructions.Create<ir::Bitcast>(InstructionResult(type), value));
    }

    /// Creates a discard instruction
    /// @returns the instruction
    ir::Discard* Discard();

    /// Creates a user function call instruction
    /// @param type the return type of the call
    /// @param func the function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::UserCall* Call(const type::Type* type, ir::Function* func, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::UserCall>(InstructionResult(type), func,
                                                           Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a core builtin call instruction
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::CoreBuiltinCall* Call(const type::Type* type, builtin::Function func, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::CoreBuiltinCall>(
            InstructionResult(type), func, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an intrinsic call instruction
    /// @param type the return type of the call
    /// @param kind the intrinsic function to call
    /// @param args the call arguments
    /// @returns the intrinsic call instruction
    template <typename... ARGS>
    ir::IntrinsicCall* Call(const type::Type* type, enum IntrinsicCall::Kind kind, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::IntrinsicCall>(
            InstructionResult(type), kind, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a value conversion instruction
    /// @param to the type converted to
    /// @param val the value to be converted
    /// @returns the instruction
    template <typename VAL>
    ir::Convert* Convert(const type::Type* to, VAL&& val) {
        return Append(ir.instructions.Create<ir::Convert>(InstructionResult(to),
                                                          Value(std::forward<VAL>(val))));
    }

    /// Creates a value constructor instruction
    /// @param type the type to constructed
    /// @param args the arguments to the constructor
    /// @returns the instruction
    template <typename... ARGS>
    ir::Construct* Construct(const type::Type* type, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::Construct>(InstructionResult(type),
                                                            Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a load instruction
    /// @param from the expression being loaded from
    /// @returns the instruction
    template <typename VAL>
    ir::Load* Load(VAL&& from) {
        auto* value = Value(std::forward<VAL>(from));
        return Append(
            ir.instructions.Create<ir::Load>(InstructionResult(value->Type()->UnwrapPtr()), value));
    }

    /// Creates a store instruction
    /// @param to the expression being stored too
    /// @param from the expression being stored
    /// @returns the instruction
    template <typename TO, typename FROM>
    ir::Store* Store(TO&& to, FROM&& from) {
        CheckForNonDeterministicEvaluation<TO, FROM>();
        auto* to_val = Value(std::forward<TO>(to));
        auto* from_val = Value(std::forward<FROM>(from));
        return Append(ir.instructions.Create<ir::Store>(to_val, from_val));
    }

    /// Creates a store vector element instruction
    /// @param to the vector pointer expression being stored too
    /// @param index the new vector element index
    /// @param value the new vector element expression
    /// @returns the instruction
    template <typename TO, typename INDEX, typename VALUE>
    ir::StoreVectorElement* StoreVectorElement(TO&& to, INDEX&& index, VALUE&& value) {
        CheckForNonDeterministicEvaluation<TO, INDEX, VALUE>();
        auto* to_val = Value(std::forward<TO>(to));
        auto* index_val = Value(std::forward<INDEX>(index));
        auto* value_val = Value(std::forward<VALUE>(value));
        return Append(ir.instructions.Create<ir::StoreVectorElement>(to_val, index_val, value_val));
    }

    /// Creates a load vector element instruction
    /// @param from the vector pointer expression being loaded from
    /// @param index the new vector element index
    /// @returns the instruction
    template <typename FROM, typename INDEX>
    ir::LoadVectorElement* LoadVectorElement(FROM&& from, INDEX&& index) {
        CheckForNonDeterministicEvaluation<FROM, INDEX>();
        auto* from_val = Value(std::forward<FROM>(from));
        auto* index_val = Value(std::forward<INDEX>(index));
        auto* res = InstructionResult(VectorPtrElementType(from_val->Type()));
        return Append(ir.instructions.Create<ir::LoadVectorElement>(res, from_val, index_val));
    }

    /// Creates a new `var` declaration
    /// @param type the var type
    /// @returns the instruction
    ir::Var* Var(const type::Pointer* type);

    /// Creates a new `var` declaration with a name
    /// @param name the var name
    /// @param type the var type
    /// @returns the instruction
    ir::Var* Var(std::string_view name, const type::Pointer* type);

    /// Creates a new `let` declaration
    /// @param name the let name
    /// @param value the let value
    /// @returns the instruction
    template <typename VALUE>
    ir::Let* Let(std::string_view name, VALUE&& value) {
        auto* val = Value(std::forward<VALUE>(value));
        if (TINT_UNLIKELY(!val)) {
            TINT_ASSERT(val);
            return nullptr;
        }
        auto* let = Append(ir.instructions.Create<ir::Let>(InstructionResult(val->Type()), val));
        ir.SetName(let, name);
        ir.SetName(let->Result(), name);
        return let;
    }

    /// Creates a return instruction
    /// @param func the function being returned
    /// @returns the instruction
    ir::Return* Return(ir::Function* func) {
        return Append(ir.instructions.Create<ir::Return>(func));
    }

    /// Creates a return instruction
    /// @param func the function being returned
    /// @param value the return value
    /// @returns the instruction
    template <typename ARG>
    ir::Return* Return(ir::Function* func, ARG&& value) {
        if constexpr (std::is_same_v<std::decay_t<ARG>, ir::Value*>) {
            if (value == nullptr) {
                return Append(ir.instructions.Create<ir::Return>(func));
            }
        }
        auto* val = Value(std::forward<ARG>(value));
        return Append(ir.instructions.Create<ir::Return>(func, val));
    }

    /// Creates a loop next iteration instruction
    /// @param loop the loop being iterated
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::NextIteration* NextIteration(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::NextIteration>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a loop break-if instruction
    /// @param condition the break condition
    /// @param loop the loop being iterated
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename CONDITION, typename... ARGS>
    ir::BreakIf* BreakIf(ir::Loop* loop, CONDITION&& condition, ARGS&&... args) {
        CheckForNonDeterministicEvaluation<CONDITION, ARGS...>();
        auto* cond_val = Value(std::forward<CONDITION>(condition));
        return Append(ir.instructions.Create<ir::BreakIf>(cond_val, loop,
                                                          Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a continue instruction
    /// @param loop the loop being continued
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::Continue* Continue(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::Continue>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit switch instruction
    /// @param sw the switch being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitSwitch* ExitSwitch(ir::Switch* sw, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::ExitSwitch>(sw, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit loop instruction
    /// @param loop the loop being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitLoop* ExitLoop(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::ExitLoop>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit if instruction
    /// @param i the if being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitIf* ExitIf(ir::If* i, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::ExitIf>(i, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit instruction for the given control instruction
    /// @param inst the control instruction being exited
    /// @param args the arguments for the target MultiInBlock
    /// @returns the exit instruction, or nullptr if the control instruction is not supported.
    template <typename... ARGS>
    ir::Exit* Exit(ir::ControlInstruction* inst, ARGS&&... args) {
        return tint::Switch(
            inst,  //
            [&](ir::If* i) { return ExitIf(i, std::forward<ARGS>(args)...); },
            [&](ir::Loop* i) { return ExitLoop(i, std::forward<ARGS>(args)...); },
            [&](ir::Switch* i) { return ExitSwitch(i, std::forward<ARGS>(args)...); });
    }

    /// Creates a new `BlockParam`
    /// @param type the parameter type
    /// @returns the value
    ir::BlockParam* BlockParam(const type::Type* type);

    /// Creates a new `BlockParam` with a name.
    /// @param name the parameter name
    /// @param type the parameter type
    /// @returns the value
    ir::BlockParam* BlockParam(std::string_view name, const type::Type* type);

    /// Creates a new `FunctionParam`
    /// @param type the parameter type
    /// @returns the value
    ir::FunctionParam* FunctionParam(const type::Type* type);

    /// Creates a new `FunctionParam` with a name.
    /// @param name the parameter name
    /// @param type the parameter type
    /// @returns the value
    ir::FunctionParam* FunctionParam(std::string_view name, const type::Type* type);

    /// Creates a new `Access`
    /// @param type the return type
    /// @param object the object being accessed
    /// @param indices the access indices
    /// @returns the instruction
    template <typename OBJ, typename... ARGS>
    ir::Access* Access(const type::Type* type, OBJ&& object, ARGS&&... indices) {
        CheckForNonDeterministicEvaluation<OBJ, ARGS...>();
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.instructions.Create<ir::Access>(InstructionResult(type), obj_val,
                                                         Values(std::forward<ARGS>(indices)...)));
    }

    /// Creates a new `Swizzle`
    /// @param type the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename OBJ>
    ir::Swizzle* Swizzle(const type::Type* type, OBJ&& object, VectorRef<uint32_t> indices) {
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.instructions.Create<ir::Swizzle>(InstructionResult(type), obj_val,
                                                          std::move(indices)));
    }

    /// Creates a new `Swizzle`
    /// @param type the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename OBJ>
    ir::Swizzle* Swizzle(const type::Type* type,
                         OBJ&& object,
                         std::initializer_list<uint32_t> indices) {
        auto* obj_val = Value(std::forward<OBJ>(object));
        return Append(ir.instructions.Create<ir::Swizzle>(InstructionResult(type), obj_val,
                                                          Vector<uint32_t, 4>(indices)));
    }

    /// Creates a terminate invocation instruction
    /// @returns the instruction
    ir::TerminateInvocation* TerminateInvocation();

    /// Creates an unreachable instruction
    /// @returns the instruction
    ir::Unreachable* Unreachable();

    /// Retrieves the root block for the module, creating if necessary
    /// @returns the root block
    ir::Block* RootBlock();

    /// Creates a new runtime value
    /// @param type the return type
    /// @returns the value
    ir::InstructionResult* InstructionResult(const type::Type* type) {
        return ir.values.Create<ir::InstructionResult>(type);
    }

    /// Create a ranged loop with a callback to build the loop body.
    /// @param ty the type manager to use for new types
    /// @param start the first loop index
    /// @param end one past the last loop index
    /// @param step the loop index step amount
    /// @param cb the callback to call for the loop body
    template <typename START, typename END, typename STEP, typename FUNCTION>
    void LoopRange(type::Manager& ty, START&& start, END&& end, STEP&& step, FUNCTION&& cb) {
        auto* start_value = Value(std::forward<START>(start));
        auto* end_value = Value(std::forward<END>(end));
        auto* step_value = Value(std::forward<STEP>(step));

        auto* loop = Loop();
        auto* idx = BlockParam("idx", start_value->Type());
        loop->Body()->SetParams({idx});
        Append(loop->Initializer(), [&] {
            // Start the loop with `idx = start`.
            NextIteration(loop, start_value);
        });
        Append(loop->Body(), [&] {
            // Loop until `idx == end`.
            auto* breakif = If(GreaterThanEqual(ty.bool_(), idx, end_value));
            Append(breakif->True(), [&] {  //
                ExitLoop(loop);
            });

            cb(idx);

            Continue(loop);
        });
        Append(loop->Continuing(), [&] {
            // Update the index with `idx += step` and go to the next iteration.
            auto* new_idx = Add(idx->Type(), idx, step_value);
            NextIteration(loop, new_idx);
        });
    }

    /// The IR module.
    Module& ir;

  private:
    /// @returns the element type of the vector-pointer type
    /// Asserts and return i32 if @p type is not a pointer to a vector
    const type::Type* VectorPtrElementType(const type::Type* type);
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_BUILDER_H_
