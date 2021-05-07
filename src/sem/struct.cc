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

#include "src/sem/struct.h"
#include "src/ast/struct_member.h"

#include <string>
#include <utility>

TINT_INSTANTIATE_TYPEINFO(tint::sem::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMember);

namespace tint {
namespace sem {

Struct::Struct(const ast::Struct* declaration,
               StructMemberList members,
               uint32_t align,
               uint32_t size,
               uint32_t size_no_padding)
    : declaration_(declaration),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding) {}

Struct::~Struct() = default;

const StructMember* Struct::FindMember(Symbol name) const {
  for (auto* member : members_) {
    if (member->Declaration()->symbol() == name) {
      return member;
    }
  }
  return nullptr;
}

std::string Struct::type_name() const {
  return declaration_->type_name();
}

std::string Struct::FriendlyName(const SymbolTable& symbols) const {
  return declaration_->FriendlyName(symbols);
}

StructMember::StructMember(ast::StructMember* declaration,
                           sem::Type* type,
                           uint32_t index,
                           uint32_t offset,
                           uint32_t align,
                           uint32_t size)
    : declaration_(declaration),
      type_(type),
      index_(index),
      offset_(offset),
      align_(align),
      size_(size) {}

StructMember::~StructMember() = default;

}  // namespace sem
}  // namespace tint
