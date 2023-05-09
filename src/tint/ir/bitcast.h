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

#include "src/tint/ir/call.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A bitcast instruction in the IR.
class Bitcast : public utils::Castable<Bitcast, Call> {
  public:
    /// Constructor
    /// @param type the result type
    /// @param val the value being bitcast
    Bitcast(const type::Type* type, Value* val);
    Bitcast(const Bitcast& inst) = delete;
    Bitcast(Bitcast&& inst) = delete;
    ~Bitcast() override;

    Bitcast& operator=(const Bitcast& inst) = delete;
    Bitcast& operator=(Bitcast&& inst) = delete;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BITCAST_H_
