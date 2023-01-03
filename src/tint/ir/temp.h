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

#ifndef SRC_TINT_IR_TEMP_H_
#define SRC_TINT_IR_TEMP_H_

#include <ostream>

#include "src/tint/ir/value.h"
#include "src/tint/symbol_table.h"

namespace tint::ir {

/// Temporary value in the IR.
class Temp : public Castable<Temp, Value> {
  public:
    /// A value id.
    using Id = uint32_t;

    /// Constructor
    /// @param type the type of the temporary
    /// @param id the id for the value
    Temp(const type::Type* type, Id id);

    /// Destructor
    ~Temp() override;

    Temp(const Temp&) = delete;
    Temp(Temp&&) = delete;

    Temp& operator=(const Temp&) = delete;
    Temp& operator=(Temp&&) = delete;

    /// @returns the value data as an `Id`.
    Id AsId() const { return id_; }

    /// @returns the type of the temporary
    const type::Type* Type() const override { return type_; }

    /// Write the temp to the given stream
    /// @param out the stream to write to
    /// @param st the symbol table
    /// @returns the stream
    std::ostream& ToString(std::ostream& out, const SymbolTable& st) const override;

  private:
    const type::Type* type_ = nullptr;
    Id id_ = 0;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_TEMP_H_
