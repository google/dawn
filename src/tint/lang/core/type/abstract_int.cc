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

#include "src/tint/lang/core/type/abstract_int.h"

#include "src/tint/lang/core/type/manager.h"
#include "src/tint/utils/math/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::AbstractInt);

namespace tint::core::type {

AbstractInt::AbstractInt() : Base(Hash(tint::TypeInfo::Of<AbstractInt>().full_hashcode)) {}

AbstractInt::~AbstractInt() = default;

std::string AbstractInt::FriendlyName() const {
    return "abstract-int";
}

AbstractInt* AbstractInt::Clone(CloneContext& ctx) const {
    return ctx.dst.mgr->Get<AbstractInt>();
}

}  // namespace tint::core::type
