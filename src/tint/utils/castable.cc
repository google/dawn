// Copyright 2020 The Tint Authors.
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

#include "src/tint/utils/castable.h"

namespace tint::utils {

/// The unique TypeInfo for the CastableBase type
/// @return doxygen-thinks-this-static-field-is-a-function :(
template <>
const TypeInfo detail::TypeInfoOf<CastableBase>::info{
    nullptr,
    "CastableBase",
    tint::utils::TypeInfo::HashCodeOf<CastableBase>(),
    tint::utils::TypeInfo::FullHashCodeOf<CastableBase>(),
};

CastableBase::CastableBase(const CastableBase&) = default;

CastableBase::~CastableBase() = default;

}  // namespace tint::utils
