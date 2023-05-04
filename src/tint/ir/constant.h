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

#ifndef SRC_TINT_IR_CONSTANT_H_
#define SRC_TINT_IR_CONSTANT_H_

#include "src/tint/constant/value.h"
#include "src/tint/ir/value.h"

namespace tint::ir {

/// Constant in the IR.
class Constant : public utils::Castable<Constant, Value> {
  public:
    /// Constructor
    /// @param val the value stored in the constant
    explicit Constant(const constant::Value* val);
    ~Constant() override;

    /// @returns the type of the constant
    const type::Type* Type() const override { return value->Type(); }

    /// The constants value
    const constant::Value* const value;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CONSTANT_H_
