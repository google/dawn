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

#include <string>
#include <utility>

#include "src/ast/struct_member.h"
#include "src/symbol_table.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMember);

namespace tint {
namespace sem {

Struct::Struct(const ast::Struct* declaration,
               Symbol name,
               StructMemberList members,
               uint32_t align,
               uint32_t size,
               uint32_t size_no_padding)
    : declaration_(declaration),
      name_(name),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding) {
  constructible_ = true;
  for (auto* member : members_) {
    if (!member->Type()->IsConstructible()) {
      constructible_ = false;
      break;
    }
  }
}

Struct::~Struct() = default;

const StructMember* Struct::FindMember(Symbol name) const {
  for (auto* member : members_) {
    if (member->Declaration()->symbol == name) {
      return member;
    }
  }
  return nullptr;
}

std::string Struct::type_name() const {
  return "__struct_" + name_.to_str();
}

uint32_t Struct::Align() const {
  return align_;
}

uint32_t Struct::Size() const {
  return size_;
}

std::string Struct::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(name_);
}

bool Struct::IsConstructible() const {
  return constructible_;
}

StructMember::StructMember(const ast::StructMember* declaration,
                           Symbol name,
                           sem::Type* type,
                           uint32_t index,
                           uint32_t offset,
                           uint32_t align,
                           uint32_t size)
    : declaration_(declaration),
      name_(name),
      type_(type),
      index_(index),
      offset_(offset),
      align_(align),
      size_(size) {}

StructMember::~StructMember() = default;

}  // namespace sem
}  // namespace tint
