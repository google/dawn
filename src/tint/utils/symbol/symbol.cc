// Copyright 2020 The Tint Authors.
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

#include "src/tint/utils/symbol/symbol.h"

#include <utility>

namespace tint {

Symbol::Symbol() = default;

Symbol::Symbol(uint32_t val, tint::GenerationID pid, std::string_view name)
    : val_(val), generation_id_(pid), name_(name) {}

Symbol::Symbol(const Symbol& o) = default;

Symbol::Symbol(Symbol&& o) = default;

Symbol::~Symbol() = default;

Symbol& Symbol::operator=(const Symbol& o) = default;

Symbol& Symbol::operator=(Symbol&& o) = default;

bool Symbol::operator==(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ == other.val_;
}

bool Symbol::operator!=(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ != other.val_;
}

bool Symbol::operator<(const Symbol& other) const {
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(generation_id_, other.generation_id_);
    return val_ < other.val_;
}

std::string Symbol::to_str() const {
    return "$" + std::to_string(val_);
}

std::string_view Symbol::NameView() const {
    return name_;
}

std::string Symbol::Name() const {
    return std::string(name_);
}

}  // namespace tint
