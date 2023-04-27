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

#ifndef SRC_TINT_IR_INSTRUCTION_H_
#define SRC_TINT_IR_INSTRUCTION_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// An instruction in the IR.
class Instruction : public utils::Castable<Instruction, Value> {
  public:
    Instruction(const Instruction& inst) = delete;
    Instruction(Instruction&& inst) = delete;
    /// Destructor
    ~Instruction() override;

    Instruction& operator=(const Instruction& inst) = delete;
    Instruction& operator=(Instruction&& inst) = delete;

    /// @returns the type of the value
    const type::Type* Type() const override { return type_; }

    /// Write the value to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    utils::StringStream& ToValue(utils::StringStream& out) const override {
        out << "%" << std::to_string(id_);
        if (type_ != nullptr) {
            out << "(" << Type()->FriendlyName() << ")";
        }
        return out;
    }

    /// Write the instruction to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    virtual utils::StringStream& ToInstruction(utils::StringStream& out) const = 0;

  protected:
    /// Constructor
    Instruction();
    /// Constructor
    /// @param id the instruction id
    /// @param type the result type
    Instruction(uint32_t id, const type::Type* type);

  private:
    uint32_t id_ = 0;
    const type::Type* type_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_H_
