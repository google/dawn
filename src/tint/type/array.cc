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

#include "src/tint/type/array.h"

#include <string>

#include "src/tint/debug.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/string_stream.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Array);

namespace tint::type {

namespace {

type::Flags FlagsFrom(const Type* element, const ArrayCount* count) {
    type::Flags flags;
    // Only constant-expression sized arrays are constructible
    if (count->Is<ConstantArrayCount>()) {
        if (element->IsConstructible()) {
            flags.Add(Flag::kConstructable);
        }
        if (element->HasCreationFixedFootprint()) {
            flags.Add(Flag::kCreationFixedFootprint);
        }
    }
    if (!count->Is<RuntimeArrayCount>()) {
        if (element->HasFixedFootprint()) {
            flags.Add(Flag::kFixedFootprint);
        }
    }
    return flags;
}

}  // namespace

const char* const Array::kErrExpectedConstantCount =
    "array size is an override-expression, when expected a constant-expression.\n"
    "Was the SubstituteOverride transform run?";

Array::Array(const Type* element,
             const ArrayCount* count,
             uint32_t align,
             uint32_t size,
             uint32_t stride,
             uint32_t implicit_stride)
    : Base(utils::Hash(utils::TypeInfo::Of<Array>().full_hashcode, count, align, size, stride),
           FlagsFrom(element, count)),
      element_(element),
      count_(count),
      align_(align),
      size_(size),
      stride_(stride),
      implicit_stride_(implicit_stride) {
    TINT_ASSERT(Type, element_);
}

bool Array::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<Array>()) {
        // Note: implicit_stride is not part of the type_name string as this is
        // derived from the element type
        return o->element_ == element_ && o->count_ == count_ && o->align_ == align_ &&
               o->size_ == size_ && o->stride_ == stride_;
    }
    return false;
}

std::string Array::FriendlyName() const {
    utils::StringStream out;
    if (!IsStrideImplicit()) {
        out << "@stride(" << stride_ << ") ";
    }
    out << "array<" << element_->FriendlyName();

    auto count_str = count_->FriendlyName();
    if (!count_str.empty()) {
        out << ", " << count_str;
    }

    out << ">";
    return out.str();
}

uint32_t Array::Align() const {
    return align_;
}

uint32_t Array::Size() const {
    return size_;
}

Array* Array::Clone(CloneContext& ctx) const {
    auto* elem_ty = element_->Clone(ctx);
    auto* count = count_->Clone(ctx);

    return ctx.dst.mgr->Get<Array>(elem_ty, count, align_, size_, stride_, implicit_stride_);
}

}  // namespace tint::type
