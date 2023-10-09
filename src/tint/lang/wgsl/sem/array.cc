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

#include "src/tint/lang/wgsl/sem/array.h"

#include "src/tint/lang/wgsl/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Array);

namespace tint::sem {

Array::Array(core::type::Type const* element,
             const core::type::ArrayCount* count,
             uint32_t align,
             uint32_t size,
             uint32_t stride,
             uint32_t implicit_stride)
    : Base(element, count, align, size, stride, implicit_stride) {}

Array::~Array() = default;

void Array::AddTransitivelyReferencedOverride(const GlobalVariable* var) {
    transitively_referenced_overrides_.Add(var);
    for (auto* ref : var->TransitivelyReferencedOverrides()) {
        AddTransitivelyReferencedOverride(ref);
    }
}

}  // namespace tint::sem
