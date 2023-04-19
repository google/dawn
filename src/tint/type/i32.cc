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

#include "src/tint/type/i32.h"

#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::I32);

namespace tint::type {

I32::I32()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<I32>().full_hashcode),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }) {}

I32::~I32() = default;

bool I32::Equals(const UniqueNode& other) const {
    return other.Is<I32>();
}

std::string I32::FriendlyName() const {
    return "i32";
}

uint32_t I32::Size() const {
    return 4;
}

uint32_t I32::Align() const {
    return 4;
}

I32* I32::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<I32>();
}

}  // namespace tint::type
