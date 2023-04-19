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

#ifndef SRC_TINT_IR_CONVERT_H_
#define SRC_TINT_IR_CONVERT_H_

#include "src/tint/ir/call.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// A value conversion instruction in the IR.
class Convert : public utils::Castable<Convert, Call> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param from the type being converted from
    /// @param args the conversion arguments
    Convert(Value* result, const type::Type* from, utils::VectorRef<Value*> args);
    Convert(const Convert& instr) = delete;
    Convert(Convert&& instr) = delete;
    ~Convert() override;

    Convert& operator=(const Convert& instr) = delete;
    Convert& operator=(Convert&& instr) = delete;

    /// @returns the from type
    const type::Type* From() const { return from_; }
    /// @returns the to type
    const type::Type* To() const { return Result()->Type(); }

    /// Write the instruction to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    utils::StringStream& ToString(utils::StringStream& out) const override;

  private:
    const type::Type* from_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CONVERT_H_
