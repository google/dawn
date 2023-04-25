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

#ifndef SRC_TINT_IR_RUNTIME_H_
#define SRC_TINT_IR_RUNTIME_H_

#include "src/tint/ir/value.h"
#include "src/tint/symbol_table.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// Runtime value in the IR.
class Runtime : public utils::Castable<Runtime, Value> {
  public:
    /// A value id.
    using Id = uint32_t;

    /// Constructor
    /// @param type the type of the value
    /// @param id the id for the value
    Runtime(const type::Type* type, Id id);

    /// Destructor
    ~Runtime() override;

    Runtime(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;

    Runtime& operator=(const Runtime&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    /// @returns the value data as an `Id`.
    Id AsId() const { return id_; }

    /// @returns the type of the value
    const type::Type* Type() const override { return type_; }

    /// Write the id to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    utils::StringStream& ToString(utils::StringStream& out) const override;

  private:
    const type::Type* type_ = nullptr;
    Id id_ = 0;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_RUNTIME_H_
