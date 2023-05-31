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

#include "src/tint/ir/loop.h"

#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::ir::Loop);

namespace tint::ir {

Loop::Loop(ir::Block* b,
           ir::Block* c,
           ir::Block* m,
           utils::VectorRef<Value*> args /* = utils::Empty */)
    : Base(std::move(args)), body_(b), continuing_(c), merge_(m) {
    TINT_ASSERT(IR, body_);
    TINT_ASSERT(IR, continuing_);
    TINT_ASSERT(IR, merge_);
}

Loop::~Loop() = default;

}  // namespace tint::ir
