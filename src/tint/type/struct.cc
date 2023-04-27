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
#include "src/tint/type/manager.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Struct);
TINT_INSTANTIATE_TYPEINFO(tint::type::StructMember);

namespace tint::type {
namespace {

type::Flags FlagsFrom(utils::VectorRef<const StructMember*> members) {
    type::Flags flags{
        Flag::kConstructable,
        Flag::kCreationFixedFootprint,
        Flag::kFixedFootprint,
    };
    for (auto* member : members) {
        if (!member->Type()->IsConstructible()) {
            flags.Remove(Flag::kConstructable);
        }
        if (!member->Type()->HasFixedFootprint()) {
            flags.Remove(Flag::kFixedFootprint);
        }
        if (!member->Type()->HasCreationFixedFootprint()) {
            flags.Remove(Flag::kCreationFixedFootprint);
        }
    }
    return flags;
}

}  // namespace

Struct::Struct(Symbol name,
               utils::VectorRef<const StructMember*> members,
               uint32_t align,
               uint32_t size,
               uint32_t size_no_padding)
    : Base(utils::Hash(utils::TypeInfo::Of<Struct>().full_hashcode, name), FlagsFrom(members)),
      name_(name),
      members_(std::move(members)),
      align_(align),
      size_(size),
      size_no_padding_(size_no_padding) {}

Struct::~Struct() = default;

bool Struct::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Struct>()) {
        return o->name_ == name_;
    }
    return false;
}

const StructMember* Struct::FindMember(Symbol name) const {
    for (auto* member : members_) {
        if (member->Name() == name) {
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

std::string Struct::FriendlyName() const {
    return name_.Name();
}

std::string Struct::Layout() const {
    utils::StringStream ss;

    auto member_name_of = [&](const StructMember* sm) { return sm->Name().Name(); };

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

    print_struct_begin_line(Align(), Size(), UnwrapRef()->FriendlyName());

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
                          member_name + " : " + m->Type()->UnwrapRef()->FriendlyName());
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

Struct* Struct::Clone(CloneContext& ctx) const {
    auto sym = ctx.dst.st->Register(name_.Name());

    utils::Vector<const StructMember*, 4> members;
    for (const auto& mem : members_) {
        members.Push(mem->Clone(ctx));
    }
    return ctx.dst.mgr->Get<Struct>(sym, members, align_, size_, size_no_padding_);
}

StructMember::StructMember(Symbol name,
                           const type::Type* type,
                           uint32_t index,
                           uint32_t offset,
                           uint32_t align,
                           uint32_t size,
                           const StructMemberAttributes& attributes)
    : name_(name),
      type_(type),
      index_(index),
      offset_(offset),
      align_(align),
      size_(size),
      attributes_(attributes) {}

StructMember::~StructMember() = default;

StructMember* StructMember::Clone(CloneContext& ctx) const {
    auto sym = ctx.dst.st->Register(name_.Name());
    auto* ty = type_->Clone(ctx);
    return ctx.dst.mgr->Get<StructMember>(sym, ty, index_, offset_, align_, size_, attributes_);
}

}  // namespace tint::type
