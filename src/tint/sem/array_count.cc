// Copyright 2021 The Tint Authors.
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

#include "src/tint/sem/array_count.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::ArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::sem::ConstantArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::sem::RuntimeArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::sem::NamedOverrideArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::sem::UnnamedOverrideArrayCount);

namespace tint::sem {

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

RuntimeArrayCount::RuntimeArrayCount() : Base() {}
RuntimeArrayCount::~RuntimeArrayCount() = default;

size_t RuntimeArrayCount::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<RuntimeArrayCount>().full_hashcode);
}

bool RuntimeArrayCount::Equals(const ArrayCount& other) const {
    return other.Is<RuntimeArrayCount>();
}

NamedOverrideArrayCount::NamedOverrideArrayCount(const GlobalVariable* var)
    : Base(), variable(var) {}
NamedOverrideArrayCount::~NamedOverrideArrayCount() = default;

size_t NamedOverrideArrayCount::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<NamedOverrideArrayCount>().full_hashcode);
}

bool NamedOverrideArrayCount::Equals(const ArrayCount& other) const {
    if (auto* v = other.As<NamedOverrideArrayCount>()) {
        return variable == v->variable;
    }
    return false;
}

UnnamedOverrideArrayCount::UnnamedOverrideArrayCount(const Expression* e) : Base(), expr(e) {}
UnnamedOverrideArrayCount::~UnnamedOverrideArrayCount() = default;

size_t UnnamedOverrideArrayCount::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<UnnamedOverrideArrayCount>().full_hashcode);
}

bool UnnamedOverrideArrayCount::Equals(const ArrayCount& other) const {
    if (auto* v = other.As<UnnamedOverrideArrayCount>()) {
        return expr == v->expr;
    }
    return false;
}

}  // namespace tint::sem
