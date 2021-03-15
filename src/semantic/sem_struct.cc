// Copyright 2021 The Tint Authors.
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

#include "src/semantic/struct.h"

TINT_INSTANTIATE_TYPEINFO(tint::semantic::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::semantic::StructMember);

namespace tint {
namespace semantic {

Struct::Struct(type::Struct* type,
               StructMemberList members,
               uint32_t align,
               uint32_t size)
    : type_(type), members_(std::move(members)), align_(align), size_(size) {}

Struct::~Struct() = default;

StructMember::StructMember(ast::StructMember* declaration,
                           uint32_t offset,
                           uint32_t size)
    : declaration_(declaration), offset_(offset), size_(size) {}

StructMember::~StructMember() = default;

}  // namespace semantic
}  // namespace tint
