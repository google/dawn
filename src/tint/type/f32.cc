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

#include "src/tint/type/f32.h"

#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::F32);

namespace tint::type {

F32::F32()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<F32>().full_hashcode),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }) {}

F32::~F32() = default;

bool F32::Equals(const UniqueNode& other) const {
    return other.Is<F32>();
}

std::string F32::FriendlyName() const {
    return "f32";
}

uint32_t F32::Size() const {
    return 4;
}

uint32_t F32::Align() const {
    return 4;
}

F32* F32::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<F32>();
}

}  // namespace tint::type
