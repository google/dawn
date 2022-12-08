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

#include "src/tint/type/array_count.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::ArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::type::ConstantArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::type::RuntimeArrayCount);

namespace tint::type {

ArrayCount::ArrayCount() : Base() {}
ArrayCount::~ArrayCount() = default;

ConstantArrayCount::ConstantArrayCount(uint32_t val) : Base(), value(val) {}
ConstantArrayCount::~ConstantArrayCount() = default;

size_t ConstantArrayCount::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<ConstantArrayCount>().full_hashcode);
}

bool ConstantArrayCount::Equals(const ArrayCount& other) const {
    if (auto* v = other.As<ConstantArrayCount>()) {
        return value == v->value;
    }
    return false;
}

std::string ConstantArrayCount::FriendlyName(const SymbolTable&) const {
    return std::to_string(value);
}

RuntimeArrayCount::RuntimeArrayCount() : Base() {}
RuntimeArrayCount::~RuntimeArrayCount() = default;

size_t RuntimeArrayCount::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<RuntimeArrayCount>().full_hashcode);
}

bool RuntimeArrayCount::Equals(const ArrayCount& other) const {
    return other.Is<RuntimeArrayCount>();
}

std::string RuntimeArrayCount::FriendlyName(const SymbolTable&) const {
    return "";
}

}  // namespace tint::type
