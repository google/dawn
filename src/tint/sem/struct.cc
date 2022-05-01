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

#include "src/tint/sem/struct.h"

#include <cmath>
#include <iomanip>
#include <string>
#include <utility>

#include "src/tint/ast/struct_member.h"
#include "src/tint/symbol_table.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::sem::StructMember);

namespace tint::sem {

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

size_t Struct::Hash() const {
    return utils::Hash(TypeInfo::Of<Struct>().full_hashcode, name_);
}

bool Struct::Equals(const sem::Type& other) const {
    if (auto* o = other.As<Struct>()) {
        return o->name_ == name_;
    }
    return false;
}

const StructMember* Struct::FindMember(Symbol name) const {
    for (auto* member : members_) {
        if (member->Declaration()->symbol == name) {
            return member;
        }
    }
    return nullptr;
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

std::string Struct::Layout(const tint::SymbolTable& symbols) const {
    std::stringstream ss;

    auto member_name_of = [&](const sem::StructMember* sm) {
        return symbols.NameFor(sm->Declaration()->symbol);
    };

    if (Members().empty()) {
        return {};
    }
    const auto* const last_member = Members().back();
    const uint32_t last_member_struct_padding_offset = last_member->Offset() + last_member->Size();

    // Compute max widths to align output
    const auto offset_w = static_cast<int>(::log10(last_member_struct_padding_offset)) + 1;
    const auto size_w = static_cast<int>(::log10(Size())) + 1;
    const auto align_w = static_cast<int>(::log10(Align())) + 1;

    auto print_struct_begin_line = [&](size_t align, size_t size, std::string struct_name) {
        ss << "/*          " << std::setw(offset_w) << " "
           << "align(" << std::setw(align_w) << align << ") size(" << std::setw(size_w) << size
           << ") */ struct " << struct_name << " {\n";
    };

    auto print_struct_end_line = [&]() {
        ss << "/*                         " << std::setw(offset_w + size_w + align_w) << " "
           << "*/ };";
    };

    auto print_member_line = [&](size_t offset, size_t align, size_t size, std::string s) {
        ss << "/* offset(" << std::setw(offset_w) << offset << ") align(" << std::setw(align_w)
           << align << ") size(" << std::setw(size_w) << size << ") */   " << s << ";\n";
    };

    print_struct_begin_line(Align(), Size(), UnwrapRef()->FriendlyName(symbols));

    for (size_t i = 0; i < Members().size(); ++i) {
        auto* const m = Members()[i];

        // Output field alignment padding, if any
        auto* const prev_member = (i == 0) ? nullptr : Members()[i - 1];
        if (prev_member) {
            uint32_t padding = m->Offset() - (prev_member->Offset() + prev_member->Size());
            if (padding > 0) {
                size_t padding_offset = m->Offset() - padding;
                print_member_line(padding_offset, 1, padding,
                                  "// -- implicit field alignment padding --");
            }
        }

        // Output member
        std::string member_name = member_name_of(m);
        print_member_line(m->Offset(), m->Align(), m->Size(),
                          member_name + " : " + m->Type()->UnwrapRef()->FriendlyName(symbols));
    }

    // Output struct size padding, if any
    uint32_t struct_padding = Size() - last_member_struct_padding_offset;
    if (struct_padding > 0) {
        print_member_line(last_member_struct_padding_offset, 1, struct_padding,
                          "// -- implicit struct size padding --");
    }

    print_struct_end_line();

    return ss.str();
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

}  // namespace tint::sem
