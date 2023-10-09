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

#include "src/tint/lang/wgsl/resolver/incomplete_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::resolver::IncompleteType);

namespace tint::resolver {

IncompleteType::IncompleteType(core::BuiltinType b)
    : Base(static_cast<size_t>(tint::TypeInfo::Of<IncompleteType>().full_hashcode),
           core::type::Flags{}),
      builtin(b) {}

IncompleteType::~IncompleteType() = default;

std::string IncompleteType::FriendlyName() const {
    return "<incomplete-type>";
}

uint32_t IncompleteType::Size() const {
    return 0;
}

uint32_t IncompleteType::Align() const {
    return 0;
}

core::type::Type* IncompleteType::Clone(core::type::CloneContext&) const {
    TINT_ICE() << "IncompleteType does not support cloning";
    return nullptr;
}

core::type::TypeAndCount IncompleteType::Elements(const Type*, uint32_t) const {
    return {};
}

const core::type::Type* IncompleteType::Element(uint32_t) const {
    return nullptr;
}

bool IncompleteType::Equals(const core::type::UniqueNode& other) const {
    if (auto* o = other.As<IncompleteType>()) {
        return o->builtin == builtin;
    }
    return false;
}

}  // namespace tint::resolver
