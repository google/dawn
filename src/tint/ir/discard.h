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

#ifndef SRC_TINT_IR_DISCARD_H_
#define SRC_TINT_IR_DISCARD_H_

#include "src/tint/debug.h"
#include "src/tint/ir/call.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A discard instruction in the IR.
class Discard : public utils::Castable<Discard, Call> {
  public:
    /// Constructor
    Discard();
    Discard(const Discard& inst) = delete;
    Discard(Discard&& inst) = delete;
    ~Discard() override;

    Discard& operator=(const Discard& inst) = delete;
    Discard& operator=(Discard&& inst) = delete;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_DISCARD_H_
