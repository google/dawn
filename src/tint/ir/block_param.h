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

#ifndef SRC_TINT_IR_BLOCK_PARAM_H_
#define SRC_TINT_IR_BLOCK_PARAM_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class BlockParam : public utils::Castable<BlockParam, Value> {
  public:
    /// Constructor
    /// @param type the type of the var
    explicit BlockParam(const type::Type* type);
    BlockParam(const BlockParam& inst) = delete;
    BlockParam(BlockParam&& inst) = delete;
    ~BlockParam() override;

    BlockParam& operator=(const BlockParam& inst) = delete;
    BlockParam& operator=(BlockParam&& inst) = delete;

    /// @returns the type of the var
    const type::Type* Type() const override { return type; }

    /// the result type of the instruction
    const type::Type* type = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_BLOCK_PARAM_H_
