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

#include "src/tint/type/abstract_numeric.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::AbstractNumeric);

namespace tint::type {

AbstractNumeric::AbstractNumeric(size_t hash)
    : Base(hash,
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }) {}
AbstractNumeric::~AbstractNumeric() = default;

uint32_t AbstractNumeric::Size() const {
    return 0;
}

uint32_t AbstractNumeric::Align() const {
    return 0;
}

}  // namespace tint::type
