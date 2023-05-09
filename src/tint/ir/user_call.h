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

#ifndef SRC_TINT_IR_USER_CALL_H_
#define SRC_TINT_IR_USER_CALL_H_

#include "src/tint/ir/call.h"
#include "src/tint/symbol.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A user call instruction in the IR.
class UserCall : public utils::Castable<UserCall, Call> {
  public:
    /// Constructor
    /// @param type the result type
    /// @param name the function name
    /// @param args the function arguments
    UserCall(const type::Type* type, Symbol name, utils::VectorRef<Value*> args);
    UserCall(const UserCall& inst) = delete;
    UserCall(UserCall&& inst) = delete;
    ~UserCall() override;

    UserCall& operator=(const UserCall& inst) = delete;
    UserCall& operator=(UserCall&& inst) = delete;

    /// The function name
    Symbol name;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_USER_CALL_H_
