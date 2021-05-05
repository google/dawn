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

#include "src/sem/u32_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::U32);

namespace tint {
namespace sem {

U32::U32() = default;

U32::~U32() = default;

U32::U32(U32&&) = default;

std::string U32::type_name() const {
  return "__u32";
}

std::string U32::FriendlyName(const SymbolTable&) const {
  return "u32";
}

}  // namespace sem
}  // namespace tint
