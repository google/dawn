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

#include "src/tint/sem/array.h"

#include <string>

#include "src/tint/debug.h"
#include "src/tint/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Array);

namespace tint::sem {

Array::Array(const Type* element,
             uint32_t count,
             uint32_t align,
             uint32_t size,
             uint32_t stride,
             uint32_t implicit_stride)
    : element_(element),
      count_(count),
      align_(align),
      size_(size),
      stride_(stride),
      implicit_stride_(implicit_stride),
      constructible_(count > 0  // Runtime-sized arrays are not constructible
                     && element->IsConstructible()) {
    TINT_ASSERT(Semantic, element_);
}

size_t Array::Hash() const {
    return utils::Hash(TypeInfo::Of<Array>().full_hashcode, count_, align_, size_, stride_);
}

bool Array::Equals(const sem::Type& other) const {
    if (auto* o = other.As<Array>()) {
        // Note: implicit_stride is not part of the type_name string as this is
        // derived from the element type
        return o->element_ == element_ && o->count_ == count_ && o->align_ == align_ &&
               o->size_ == size_ && o->stride_ == stride_;
    }
    return false;
}

bool Array::IsConstructible() const {
    return constructible_;
}

std::string Array::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    if (!IsStrideImplicit()) {
        out << "@stride(" << stride_ << ") ";
    }
    out << "array<" << element_->FriendlyName(symbols);
    if (!IsRuntimeSized()) {
        out << ", " << count_;
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

}  // namespace tint::sem
