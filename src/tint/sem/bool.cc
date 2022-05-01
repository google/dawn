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

#include "src/tint/sem/bool.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Bool);

namespace tint::sem {

Bool::Bool() = default;

Bool::Bool(Bool&&) = default;

Bool::~Bool() = default;

size_t Bool::Hash() const {
    return static_cast<size_t>(TypeInfo::Of<Bool>().full_hashcode);
}

bool Bool::Equals(const Type& other) const {
    return other.Is<Bool>();
}

std::string Bool::FriendlyName(const SymbolTable&) const {
    return "bool";
}

bool Bool::IsConstructible() const {
    return true;
}

uint32_t Bool::Size() const {
    return 4;
}

uint32_t Bool::Align() const {
    return 4;
}

}  // namespace tint::sem
