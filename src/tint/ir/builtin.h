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

#ifndef SRC_TINT_IR_BUILTIN_H_
#define SRC_TINT_IR_BUILTIN_H_

#include "src/tint/builtin/function.h"
#include "src/tint/ir/call.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A value conversion instruction in the IR.
class Builtin : public utils::Castable<Builtin, Call> {
  public:
    /// Constructor
    /// @param res_type the result type
    /// @param func the builtin function
    /// @param args the conversion arguments
    Builtin(const type::Type* res_type, builtin::Function func, utils::VectorRef<Value*> args);
    Builtin(const Builtin& inst) = delete;
    Builtin(Builtin&& inst) = delete;
    ~Builtin() override;

    Builtin& operator=(const Builtin& inst) = delete;
    Builtin& operator=(Builtin&& inst) = delete;

    /// @returns the builtin function
    builtin::Function Func() const { return func_; }

  private:
    const builtin::Function func_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BUILTIN_H_
