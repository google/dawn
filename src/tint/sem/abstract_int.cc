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

#include "src/tint/sem/abstract_int.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::AbstractInt);

namespace tint::sem {

AbstractInt::AbstractInt() = default;
AbstractInt::AbstractInt(AbstractInt&&) = default;
AbstractInt::~AbstractInt() = default;

size_t AbstractInt::Hash() const {
    return utils::Hash(TypeInfo::Of<AbstractInt>().full_hashcode);
}

bool AbstractInt::Equals(const sem::Type& other) const {
    return other.Is<AbstractInt>();
}

std::string AbstractInt::FriendlyName(const SymbolTable&) const {
    return "AbstractInt";
}

}  // namespace tint::sem
