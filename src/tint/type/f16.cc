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

#include "src/tint/type/f16.h"

#include "src/tint/type/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::F16);

namespace tint::type {

F16::F16()
    : Base(static_cast<size_t>(utils::TypeInfo::Of<F16>().full_hashcode),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }) {}

F16::~F16() = default;

bool F16::Equals(const UniqueNode& other) const {
    return other.Is<F16>();
}

std::string F16::FriendlyName() const {
    return "f16";
}

uint32_t F16::Size() const {
    return 2;
}

uint32_t F16::Align() const {
    return 2;
}

F16* F16::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<F16>();
}

}  // namespace tint::type
