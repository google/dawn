// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_IR_BITCAST_H_
#define SRC_TINT_IR_BITCAST_H_

#include <ostream>

#include "src/tint/castable.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/type.h"

namespace tint::ir {

/// A bitcast instruction in the IR.
class Bitcast : public Castable<Bitcast, Instruction> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param val the value being bitcast
    Bitcast(Value* result, Value* val);
    Bitcast(const Bitcast& instr) = delete;
    Bitcast(Bitcast&& instr) = delete;
    ~Bitcast() override;

    Bitcast& operator=(const Bitcast& instr) = delete;
    Bitcast& operator=(Bitcast&& instr) = delete;

    /// @returns the left-hand-side value for the instruction
    const Value* Val() const { return val_; }

    /// Write the instruction to the given stream
    /// @param out the stream to write to
    /// @param st the symbol table
    /// @returns the stream
    std::ostream& ToString(std::ostream& out, const SymbolTable& st) const override;

  private:
    Value* val_ = nullptr;
};

std::ostream& operator<<(std::ostream& out, const Bitcast&);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BITCAST_H_
