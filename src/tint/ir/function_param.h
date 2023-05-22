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

#ifndef SRC_TINT_IR_FUNCTION_PARAM_H_
#define SRC_TINT_IR_FUNCTION_PARAM_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A function parameter in the IR.
class FunctionParam : public utils::Castable<FunctionParam, Value> {
  public:
    /// Constructor
    /// @param type the type of the var
    explicit FunctionParam(const type::Type* type);
    ~FunctionParam() override;

    /// @returns the type of the var
    const type::Type* Type() const override { return type_; }

  private:
    /// The type of the parameter
    const type::Type* type_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_FUNCTION_PARAM_H_
