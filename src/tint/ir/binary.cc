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

#include "src/tint/ir/binary.h"
#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Binary);

namespace tint::ir {

Binary::Binary(Kind kind, const type::Type* ty, Value* lhs, Value* rhs)
    : Base(ty), kind_(kind), lhs_(lhs), rhs_(rhs) {
    TINT_ASSERT(IR, lhs_);
    TINT_ASSERT(IR, rhs_);
    lhs_->AddUsage(this);
    rhs_->AddUsage(this);
}

Binary::~Binary() = default;

}  // namespace tint::ir
