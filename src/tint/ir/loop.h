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

#ifndef SRC_TINT_IR_LOOP_H_
#define SRC_TINT_IR_LOOP_H_

#include "src/tint/ir/block.h"
#include "src/tint/ir/branch.h"

namespace tint::ir {

/// Flow node describing a loop.
class Loop : public utils::Castable<Loop, Branch> {
  public:
    /// Constructor
    /// @param s the start block
    /// @param c the continuing block
    /// @param m the merge block
    Loop(Block* s, Block* c, Block* m);
    ~Loop() override;

    /// @returns the switch start branch
    const Block* Start() const { return start_; }
    /// @returns the switch start branch
    Block* Start() { return start_; }

    /// @returns the switch continuing branch
    const Block* Continuing() const { return continuing_; }
    /// @returns the switch continuing branch
    Block* Continuing() { return continuing_; }

    /// @returns the switch merge branch
    const Block* Merge() const { return merge_; }
    /// @returns the switch merge branch
    Block* Merge() { return merge_; }

  private:
    Block* start_ = nullptr;
    Block* continuing_ = nullptr;
    Block* merge_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_LOOP_H_
