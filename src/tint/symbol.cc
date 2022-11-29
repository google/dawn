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

#include "src/tint/symbol.h"

#include <utility>

namespace tint {

Symbol::Symbol() = default;

Symbol::Symbol(uint32_t val, tint::ProgramID program_id) : val_(val), program_id_(program_id) {}

#if TINT_SYMBOL_STORE_DEBUG_NAME
Symbol::Symbol(uint32_t val, tint::ProgramID pid, std::string debug_name)
    : val_(val), program_id_(pid), debug_name_(std::move(debug_name)) {}
#endif

Symbol::Symbol(const Symbol& o) = default;

Symbol::Symbol(Symbol&& o) = default;

Symbol::~Symbol() = default;

Symbol& Symbol::operator=(const Symbol& o) = default;

Symbol& Symbol::operator=(Symbol&& o) = default;

bool Symbol::operator==(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ == other.val_;
}

bool Symbol::operator!=(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ != other.val_;
}

bool Symbol::operator<(const Symbol& other) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(Symbol, program_id_, other.program_id_);
    return val_ < other.val_;
}

std::string Symbol::to_str() const {
    return "$" + std::to_string(val_);
}

}  // namespace tint
