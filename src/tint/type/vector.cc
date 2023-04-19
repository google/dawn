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

#include "src/tint/type/vector.h"

#include "src/tint/debug.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/type/manager.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Vector);

namespace tint::type {

Vector::Vector(Type const* subtype, uint32_t width, bool packed /* = false */)
    : Base(utils::Hash(utils::TypeInfo::Of<Vector>().full_hashcode, width, subtype, packed),
           type::Flags{
               Flag::kConstructable,
               Flag::kCreationFixedFootprint,
               Flag::kFixedFootprint,
           }),
      subtype_(subtype),
      width_(width),
      packed_(packed) {
    TINT_ASSERT(Type, width_ > 1);
    TINT_ASSERT(Type, width_ < 5);
}

Vector::~Vector() = default;

bool Vector::Equals(const UniqueNode& other) const {
    if (auto* v = other.As<Vector>()) {
        return v->width_ == width_ && v->subtype_ == subtype_ && v->packed_ == packed_;
    }
    return false;
}

std::string Vector::FriendlyName() const {
    utils::StringStream out;
    if (packed_) {
        out << "__packed_";
    }
    out << "vec" << width_ << "<" << subtype_->FriendlyName() << ">";
    return out.str();
}

uint32_t Vector::Size() const {
    return subtype_->Size() * width_;
}

uint32_t Vector::Align() const {
    switch (width_) {
        case 2:
            return subtype_->Size() * 2;
        case 3:
            return subtype_->Size() * (packed_ ? 1 : 4);
        case 4:
            return subtype_->Size() * 4;
    }
    return 0;  // Unreachable
}

Vector* Vector::Clone(CloneContext& ctx) const {
    auto* subtype = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<Vector>(subtype, width_, packed_);
}

}  // namespace tint::type
