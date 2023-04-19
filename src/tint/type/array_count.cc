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

#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::ArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::type::ConstantArrayCount);
TINT_INSTANTIATE_TYPEINFO(tint::type::RuntimeArrayCount);

namespace tint::type {

ArrayCount::ArrayCount(size_t hash) : Base(hash) {}
ArrayCount::~ArrayCount() = default;

ConstantArrayCount::ConstantArrayCount(uint32_t val)
    : Base(static_cast<size_t>(utils::TypeInfo::Of<ConstantArrayCount>().full_hashcode)),
      value(val) {}
ConstantArrayCount::~ConstantArrayCount() = default;

bool ConstantArrayCount::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<ConstantArrayCount>()) {
        return value == v->value;
    }
    return false;
}

std::string ConstantArrayCount::FriendlyName() const {
    return std::to_string(value);
}

ConstantArrayCount* ConstantArrayCount::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<ConstantArrayCount>(value);
}

RuntimeArrayCount::RuntimeArrayCount()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<RuntimeArrayCount>().full_hashcode)) {}
RuntimeArrayCount::~RuntimeArrayCount() = default;

bool RuntimeArrayCount::Equals(const UniqueNode& other) const {
    return other.Is<RuntimeArrayCount>();
}

std::string RuntimeArrayCount::FriendlyName() const {
    return "";
}

RuntimeArrayCount* RuntimeArrayCount::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<RuntimeArrayCount>();
}

}  // namespace tint::type
