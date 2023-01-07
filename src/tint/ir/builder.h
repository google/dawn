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
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/module.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/temp.h"
#include "src/tint/ir/terminator.h"
#include "src/tint/ir/value.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/u32.h"

namespace tint::ir {

/// Builds an ir::Module
class Builder {
  public:
    /// Constructor
    Builder();
    /// Constructor
    /// @param mod the ir::Module to wrap with this builder
    explicit Builder(Module&& mod);
    /// Destructor
    ~Builder();

    /// @returns a new block flow node
    Block* CreateBlock();

    /// @returns a new terminator flow node
    Terminator* CreateTerminator();

    /// Creates a function flow node
    /// @returns the flow node
    Function* CreateFunction();

    /// Creates an if flow node
    /// @returns the flow node
    If* CreateIf();

    /// Creates a loop flow node
    /// @returns the flow node
    Loop* CreateLoop();

    /// Creates a switch flow node
    /// @returns the flow node
    Switch* CreateSwitch();

    /// Creates a case flow node for the given case branch.
    /// @param s the switch to create the case into
    /// @param selectors the case selectors for the case statement
    /// @returns the start block for the case flow node
    Block* CreateCase(Switch* s, utils::VectorRef<Switch::CaseSelector> selectors);

    /// Branches the given block to the given flow node.
    /// @param from the block to branch from
    /// @param to the node to branch too
    /// @param args arguments to the branch
    void Branch(Block* from, FlowNode* to, utils::VectorRef<Value*> args);

    /// Creates a constant::Value
    /// @param args the arguments
    /// @returns the new constant value
    template <typename T, typename... ARGS>
    traits::EnableIf<traits::IsTypeOrDerived<T, constant::Value>, const T>* create(ARGS&&... args) {
        return ir.constants.Create<T>(std::forward<ARGS>(args)...);
    }

    /// Creates a new ir::Constant
    /// @param val the constant value
    /// @returns the new constant
    ir::Constant* Constant(const constant::Value* val) {
        return ir.values.Create<ir::Constant>(val);
    }

    /// Creates a ir::Constant for an i32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(i32 v) {
        return Constant(create<constant::Scalar<i32>>(ir.types.Get<type::I32>(), v));
    }

    /// Creates a ir::Constant for a u32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(u32 v) {
        return Constant(create<constant::Scalar<u32>>(ir.types.Get<type::U32>(), v));
    }

    /// Creates a ir::Constant for a f32 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f32 v) {
        return Constant(create<constant::Scalar<f32>>(ir.types.Get<type::F32>(), v));
    }

    /// Creates a ir::Constant for a f16 Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(f16 v) {
        return Constant(create<constant::Scalar<f16>>(ir.types.Get<type::F16>(), v));
    }

    /// Creates a ir::Constant for a bool Scalar
    /// @param v the value
    /// @returns the new constant
    ir::Constant* Constant(bool v) {
        return Constant(create<constant::Scalar<bool>>(ir.types.Get<type::Bool>(), v));
    }

    /// Creates a new Temporary
    /// @param type the type of the temporary
    /// @returns the new temporary
    ir::Temp* Temp(const type::Type* type) {
        return ir.values.Create<ir::Temp>(type, AllocateTempId());
    }

    /// Creates an op for `lhs kind rhs`
    /// @param kind the kind of operation
    /// @param type the result type of the binary expression
    /// @param lhs the left-hand-side of the operation
    /// @param rhs the right-hand-side of the operation
    /// @returns the operation
    Binary* CreateBinary(Binary::Kind kind, const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an And operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* And(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Or operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Or(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Xor operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Xor(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an LogicalAnd operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* LogicalAnd(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an LogicalOr operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* LogicalOr(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Equal operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Equal(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an NotEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* NotEqual(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an LessThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* LessThan(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an GreaterThan operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* GreaterThan(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an LessThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* LessThanEqual(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an GreaterThanEqual operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* GreaterThanEqual(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an ShiftLeft operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* ShiftLeft(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an ShiftRight operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* ShiftRight(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Add operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Add(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Subtract operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Subtract(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Multiply operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Multiply(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Divide operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Divide(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates an Modulo operation
    /// @param type the result type of the expression
    /// @param lhs the lhs of the add
    /// @param rhs the rhs of the add
    /// @returns the operation
    Binary* Modulo(const type::Type* type, Value* lhs, Value* rhs);

    /// Creates a bitcast instruction
    /// @param type the result type of the bitcast
    /// @param val the value being bitcast
    /// @returns the instruction
    ir::Bitcast* Bitcast(const type::Type* type, Value* val);

    /// @returns a unique temp id
    Temp::Id AllocateTempId();

    /// The IR module.
    Module ir;

    /// The next temporary number to allocate
    Temp::Id next_temp_id = 1;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BUILDER_H_
