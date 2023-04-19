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

#include "src/tint/type/abstract_int.h"

#include "src/tint/type/manager.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::AbstractInt);

namespace tint::type {

AbstractInt::AbstractInt() : Base(utils::Hash(utils::TypeInfo::Of<AbstractInt>().full_hashcode)) {}

AbstractInt::~AbstractInt() = default;

bool AbstractInt::Equals(const UniqueNode& other) const {
    return other.Is<AbstractInt>();
}

std::string AbstractInt::FriendlyName() const {
    return "abstract-int";
}

AbstractInt* AbstractInt::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<AbstractInt>();
}

}  // namespace tint::type
