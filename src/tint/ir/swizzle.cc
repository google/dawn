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

#include "src/tint/ir/swizzle.h"

#include <utility>

#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Swizzle);

namespace tint::ir {

Swizzle::Swizzle(const type::Type* ty, Value* object, utils::VectorRef<uint32_t> indices)
    : result_type_(ty), indices_(std::move(indices)) {
    TINT_ASSERT(IR, object != nullptr);
    TINT_ASSERT(IR, result_type_ != nullptr);
    TINT_ASSERT(IR, !indices.IsEmpty());
    TINT_ASSERT(IR, indices.Length() <= 4);

    AddOperand(object);

    for (auto idx : indices_) {
        TINT_ASSERT(IR, idx < 4);
    }
}

Swizzle::~Swizzle() = default;

}  // namespace tint::ir
