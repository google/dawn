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

#include "src/tint/type/scalar.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Scalar);

namespace tint::type {

Scalar::Scalar(size_t hash, type::Flags flags) : Base(hash, flags) {}

Scalar::~Scalar() = default;

bool Scalar::Equals(const UniqueNode& other) const {
    return &other.TypeInfo() == &TypeInfo();
}

}  // namespace tint::type
