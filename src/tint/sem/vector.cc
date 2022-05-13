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

#include "src/tint/sem/vector.h"

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Vector);

namespace tint::sem {

Vector::Vector(Type const* subtype, uint32_t width) : subtype_(subtype), width_(width) {
    TINT_ASSERT(Semantic, width_ > 1);
    TINT_ASSERT(Semantic, width_ < 5);
}

Vector::Vector(Vector&&) = default;

Vector::~Vector() = default;

size_t Vector::Hash() const {
    return utils::Hash(TypeInfo::Of<Vector>().full_hashcode, width_, subtype_);
}

bool Vector::Equals(const Type& other) const {
    if (auto* v = other.As<Vector>()) {
        return v->width_ == width_ && v->subtype_ == subtype_;
    }
    return false;
}

std::string Vector::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    out << "vec" << width_ << "<" << subtype_->FriendlyName(symbols) << ">";
    return out.str();
}

bool Vector::IsConstructible() const {
    return true;
}

uint32_t Vector::Size() const {
    return subtype_->Size() * width_;
}

uint32_t Vector::Align() const {
    switch (width_) {
        case 2:
            return subtype_->Size() * 2;
        case 3:
            return subtype_->Size() * 4;
        case 4:
            return subtype_->Size() * 4;
    }
    return 0;  // Unreachable
}

}  // namespace tint::sem
