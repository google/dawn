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

#include "src/tint/lang/core/ir/multi_in_block.h"

#include "src/tint/utils/containers/predicates.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::MultiInBlock);

namespace tint::ir {

MultiInBlock::MultiInBlock() : Base() {}

MultiInBlock::~MultiInBlock() = default;

void MultiInBlock::SetParams(VectorRef<BlockParam*> params) {
    params_ = std::move(params);
}

void MultiInBlock::SetParams(std::initializer_list<BlockParam*> params) {
    params_ = std::move(params);
}

void MultiInBlock::AddInboundSiblingBranch(ir::Terminator* node) {
    TINT_ASSERT(node != nullptr);

    if (node) {
        inbound_sibling_branches_.Push(node);
    }
}

}  // namespace tint::ir
