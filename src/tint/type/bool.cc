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

#include "src/tint/type/bool.h"

#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Bool);

namespace tint::type {

Bool::Bool()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<Bool>().full_hashcode),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }) {}

Bool::~Bool() = default;

bool Bool::Equals(const UniqueNode& other) const {
    return other.Is<Bool>();
}

std::string Bool::FriendlyName() const {
    return "bool";
}

uint32_t Bool::Size() const {
    return 4;
}

uint32_t Bool::Align() const {
    return 4;
}

Bool* Bool::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<Bool>();
}

}  // namespace tint::type
