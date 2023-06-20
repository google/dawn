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

#ifndef SRC_TINT_IR_BUILDER_H_
#define SRC_TINT_IR_BUILDER_H_

#include <utility>

#include "src/tint/constant/scalar.h"
#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/block_param.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin_call.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/convert.h"
#include "src/tint/ir/discard.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/function_param.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/instruction_result.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/multi_in_block.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/swizzle.h"
#include "src/tint/ir/unary.h"
#include "src/tint/ir/unreachable.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/value.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/pointer.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/type/void.h"

namespace tint::ir {

/// Builds an ir::Module
class Builder {
    /// A helper used to enable overloads if the first type in `TYPES` is a utils::Vector or
    /// utils::VectorRef.
    template <typename... TYPES>
    using EnableIfVectorLike = utils::traits::EnableIf<
        utils::IsVectorLike<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    /// A helper used to disable overloads if the first type in `TYPES` is a utils::Vector or
    /// utils::VectorRef.
    template <typename... TYPES>
    using DisableIfVectorLike = utils::traits::EnableIf<
        !utils::IsVectorLike<utils::traits::Decay<utils::traits::NthTypeOf<0, TYPES..., void>>>>;

    template <typename T>
    T* Append(T* val) {
        if (current_block_) {
            current_block_->Append(val);
        }
        return val;
    }

    /// If set, any created instruction will be auto-appended to the block.
    ir::Block* current_block_ = nullptr;

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

    /// Creates a new builder wrapping the given block
    /// @param b the block to set as the current block
    /// @returns the builder
    Builder With(Block* b) { return Builder(ir, b); }

    /// @returns a new block
    ir::Block* Block();

    /// @returns a new multi-in block
    ir::MultiInBlock* MultiInBlock();

    /// Creates a function flow node
    /// @param name the function name
    /// @param return_type the function return type
    /// @param stage the function stage
    /// @param wg_size the workgroup_size
    /// @returns the flow node
    ir::Function* Function(std::string_view name,
                           const type::Type* return_type,
                           Function::PipelineStage stage = Function::PipelineStage::kUndefined,
                           std::optional<std::array<uint32_t, 3>> wg_size = {});

    /// Creates an if flow node
    /// @param condition the if condition
    /// @returns the flow node
    template <typename T>
    ir::If* If(T&& condition) {
        return Append(
            ir.instructions.Create<ir::If>(Value(std::forward<T>(condition)), Block(), Block()));
    }

    /// Creates a loop flow node
    /// @returns the flow node
    ir::Loop* Loop();

    /// Creates a switch flow node
    /// @param condition the switch condition
    /// @returns the flow node
    template <typename T>
    ir::Switch* Switch(T&& condition) {
        return Append(ir.instructions.Create<ir::Switch>(Value(std::forward<T>(condition))));
    }

    /// Creates a case flow node for the given case branch.
    /// @param s the switch to create the case into
    /// @param selectors the case selectors for the case statement
    /// @returns the start block for the case flow node
    ir::Block* Case(ir::Switch* s, utils::VectorRef<Switch::CaseSelector> selectors);

    /// Creates a case flow node for the given case branch.
    /// @param s the switch to create the case into
    /// @param selectors the case selectors for the case statement
    /// @returns the start block for the case flow node
    ir::Block* Case(ir::Switch* s, std::initializer_list<Switch::CaseSelector> selectors);

    /// Creates a new ir::Constant
    /// @param val the constant value
    /// @returns the new constant
    ir::Constant* Constant(const constant::Value* val) {
        return ir.constants.GetOrCreate(val, [&]() { return ir.values.Create<ir::Constant>(val); });
    }

    /// Creates a ir::Constant for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(i32 v) { return Constant(ir.constant_values.Get(v)); }

    /// Creates a ir::Constant for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(u32 v) { return Constant(ir.constant_values.Get(v)); }

    /// Creates a ir::Constant for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f32 v) { return Constant(ir.constant_values.Get(v)); }

    /// Creates a ir::Constant for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f16 v) { return Constant(ir.constant_values.Get(v)); }

    /// Creates a ir::Constant for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(bool v) { return Constant(ir.constant_values.Get(v)); }

    /// Creates a ir::Constant for the given number
    /// @param number the number value
    /// @returns the new constant
    template <typename T, typename = std::enable_if_t<IsNumeric<T>>>
    ir::Constant* Value(T&& number) {
        return Constant(std::forward<T>(number));
    }

    /// Pass-through overload for nullptr values
    /// @returns nullptr
    ir::Value* Value(std::nullptr_t) { return nullptr; }

    /// Pass-through overload for Value()
    /// @param v the ir::Value pointer
    /// @returns @p v
    ir::Value* Value(ir::Value* v) { return v; }

    /// Extract the first result from the instruction
    /// @param inst the instruction
    /// @returns the result value
    ir::Value* Value(ir::Instruction* inst) {
        TINT_ASSERT(IR, inst->HasResults() && !inst->HasMultiResults());
        return inst->Result();
    }

    /// Creates a value from the given number
    /// @param n the number
    /// @returns the value
    template <typename T>
    ir::Value* Value(Number<T> n) {
        return Constant(n);
    }

    /// Pass-through overload for Values() with vector-like argument
    /// @param vec the vector of ir::Value*
    /// @return @p vec
    template <typename VEC, typename = EnableIfVectorLike<utils::traits::Decay<VEC>>>
    auto Values(VEC&& vec) {
        return std::forward<VEC>(vec);
    }

    /// Overload for Values() with utils::Empty argument
    /// @return utils::Empty
    utils::EmptyType Values(utils::EmptyType) { return utils::Empty; }

    /// Overload for Values() with no arguments
    /// @return utils::Empty
    utils::EmptyType Values() { return utils::Empty; }

    /// @param args the arguments to pass to Value()
    /// @returns a vector of ir::Value* built from transforming the arguments with Value()
    template <typename... ARGS, typename = DisableIfVectorLike<ARGS...>>
    auto Values(ARGS&&... args) {
        return utils::Vector{Value(std::forward<ARGS>(args))...};
    }

    /// Creates an op for `lhs kind rhs`
    /// @param kind the kind of operation
    /// @param type the result type of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    template <typename LHS, typename RHS>
    ir::Binary* Binary(enum Binary::Kind kind, const type::Type* type, LHS&& lhs, RHS&& rhs) {
        return Append(ir.instructions.Create<ir::Binary>(InstructionResult(type), kind,
                                                         Value(std::forward<LHS>(lhs)),
                                                         Value(std::forward<RHS>(rhs))));
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
        return Append(ir.instructions.Create<ir::Unary>(InstructionResult(type), kind,
                                                        Value(std::forward<VAL>(val))));
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
        return Equal(type, std::forward<VAL>(val), Constant(false));
    }

    /// Creates a bitcast instruction
    /// @param type the result type of the bitcast
    /// @param val the value being bitcast
    /// @returns the instruction
    template <typename VAL>
    ir::Bitcast* Bitcast(const type::Type* type, VAL&& val) {
        return Append(ir.instructions.Create<ir::Bitcast>(InstructionResult(type),
                                                          Value(std::forward<VAL>(val))));
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

    /// Creates a builtin call instruction
    /// @param type the return type of the call
    /// @param func the builtin function to call
    /// @param args the call arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::BuiltinCall* Call(const type::Type* type, builtin::Function func, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::BuiltinCall>(InstructionResult(type), func,
                                                              Values(std::forward<ARGS>(args)...)));
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
        auto* val = Value(std::forward<VAL>(from));
        return Append(
            ir.instructions.Create<ir::Load>(InstructionResult(val->Type()->UnwrapPtr()), val));
    }

    /// Creates a store instruction
    /// @param to the expression being stored too
    /// @param from the expression being stored
    /// @returns the instruction
    template <typename TO, typename ARG>
    ir::Store* Store(TO&& to, ARG&& from) {
        return Append(ir.instructions.Create<ir::Store>(Value(std::forward<TO>(to)),
                                                        Value(std::forward<ARG>(from))));
    }

    /// Creates a new `var` declaration
    /// @param type the var type
    /// @returns the instruction
    ir::Var* Var(const type::Pointer* type);

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
        return Append(ir.instructions.Create<ir::Return>(func, Value(std::forward<ARG>(value))));
    }

    /// Creates a loop next iteration instruction
    /// @param loop the loop being iterated
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::NextIteration* NextIteration(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::NextIteration>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a loop break-if instruction
    /// @param condition the break condition
    /// @param loop the loop being iterated
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename CONDITION, typename... ARGS>
    ir::BreakIf* BreakIf(CONDITION&& condition, ir::Loop* loop, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::BreakIf>(
            Value(std::forward<CONDITION>(condition)), loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates a continue instruction
    /// @param loop the loop being continued
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::Continue* Continue(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::Continue>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit switch instruction
    /// @param sw the switch being exited
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitSwitch* ExitSwitch(ir::Switch* sw, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::ExitSwitch>(sw, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit loop instruction
    /// @param loop the loop being exited
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitLoop* ExitLoop(ir::Loop* loop, ARGS&&... args) {
        return Append(
            ir.instructions.Create<ir::ExitLoop>(loop, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit if instruction
    /// @param i the if being exited
    /// @param args the branch arguments
    /// @returns the instruction
    template <typename... ARGS>
    ir::ExitIf* ExitIf(ir::If* i, ARGS&&... args) {
        return Append(ir.instructions.Create<ir::ExitIf>(i, Values(std::forward<ARGS>(args)...)));
    }

    /// Creates an exit instruction for the given control instruction
    /// @param inst the control instruction being exited
    /// @param args the branch arguments
    /// @returns the exit instruction, or nullptr if the control instruction is not supported.
    template <typename... ARGS>
    ir::Branch* Exit(ir::ControlInstruction* inst, ARGS&&... args) {
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

    /// Creates a new `FunctionParam`
    /// @param type the parameter type
    /// @returns the value
    ir::FunctionParam* FunctionParam(const type::Type* type);

    /// Creates a new `Access`
    /// @param type the return type
    /// @param object the object being accessed
    /// @param indices the access indices
    /// @returns the instruction
    template <typename OBJ, typename... ARGS>
    ir::Access* Access(const type::Type* type, OBJ&& object, ARGS&&... indices) {
        return Append(ir.instructions.Create<ir::Access>(InstructionResult(type),
                                                         Value(std::forward<OBJ>(object)),
                                                         Values(std::forward<ARGS>(indices)...)));
    }

    /// Creates a new `Swizzle`
    /// @param type the return type
    /// @param object the object being swizzled
    /// @param indices the swizzle indices
    /// @returns the instruction
    template <typename OBJ>
    ir::Swizzle* Swizzle(const type::Type* type, OBJ&& object, utils::VectorRef<uint32_t> indices) {
        return Append(ir.instructions.Create<ir::Swizzle>(
            InstructionResult(type), Value(std::forward<OBJ>(object)), std::move(indices)));
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
        return Append(ir.instructions.Create<ir::Swizzle>(InstructionResult(type),
                                                          Value(std::forward<OBJ>(object)),
                                                          utils::Vector<uint32_t, 4>(indices)));
    }

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

    /// The IR module.
    Module& ir;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BUILDER_H_
