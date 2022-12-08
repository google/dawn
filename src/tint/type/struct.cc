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

#include "src/tint/type/struct.h"

#include <cmath>
#include <iomanip>
#include <string>
#include <utility>

#include "src/tint/symbol_table.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::StructBase);
TINT_INSTANTIATE_TYPEINFO(tint::type::StructMemberBase);

namespace tint::type {
namespace {

TypeFlags FlagsFrom(utils::VectorRef<const StructMemberBase*> members) {
    TypeFlags flags{
        TypeFlag::kConstructable,
        TypeFlag::kCreationFixedFootprint,
        TypeFlag::kFixedFootprint,
    };
    for (auto* member : members) {
        if (!member->Type()->IsConstructible()) {
            flags.Remove(TypeFlag::kConstructable);
        }
        if (!member->Type()->HasFixedFootprint()) {
            flags.Remove(TypeFlag::kFixedFootprint);
        }
        if (!member->Type()->HasCreationFixedFootprint()) {
            flags.Remove(TypeFlag::kCreationFixedFootprint);
        }
    }
    return flags;
}

}  // namespace

StructBase::StructBase(tint::Source source,
                       Symbol name,
                       utils::VectorRef<const StructMemberBase*> members,
                       uint32_t align,
                       uint32_t size,
                       uint32_t size_no_padding)
    : Base(FlagsFrom(members)),
      source_(source),
      name_(name),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding) {}

StructBase::~StructBase() = default;

size_t StructBase::Hash() const {
    return utils::Hash(TypeInfo::Of<StructBase>().full_hashcode, name_);
}

bool StructBase::Equals(const Type& other) const {
    if (auto* o = other.As<StructBase>()) {
        return o->name_ == name_;
    }
    return false;
}

const StructMemberBase* StructBase::FindMember(Symbol name) const {
    for (auto* member : members_) {
        if (member->Name() == name) {
            return member;
        }
    }
    return nullptr;
}

uint32_t StructBase::Align() const {
    return align_;
}

uint32_t StructBase::Size() const {
    return size_;
}

std::string StructBase::FriendlyName(const SymbolTable& symbols) const {
    return symbols.NameFor(name_);
}

std::string StructBase::Layout(const tint::SymbolTable& symbols) const {
    std::stringstream ss;

    auto member_name_of = [&](const StructMemberBase* sm) { return symbols.NameFor(sm->Name()); };

    if (Members().IsEmpty()) {
        return {};
    }
    const auto* const last_member = Members().Back();
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

    for (size_t i = 0; i < Members().Length(); ++i) {
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

StructMemberBase::StructMemberBase(tint::Source source,
                                   Symbol name,
                                   const type::Type* type,
                                   uint32_t index,
                                   uint32_t offset,
                                   uint32_t align,
                                   uint32_t size,
                                   std::optional<uint32_t> location)
    : source_(source),
      name_(name),
      type_(type),
      index_(index),
      offset_(offset),
      align_(align),
      size_(size),
      location_(location) {}

StructMemberBase::~StructMemberBase() = default;

}  // namespace tint::type
