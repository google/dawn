// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_LANG_WGSL_SEM_ARRAY_H_
#define SRC_TINT_LANG_WGSL_SEM_ARRAY_H_

#include "src/tint/lang/core/type/array.h"
#include "src/tint/utils/containers/unique_vector.h"

/// Forward declarations
namespace tint::sem {
class GlobalVariable;
}

namespace tint::sem {

/// Array holds the semantic information for Arrays.
class Array final : public Castable<Array, core::type::Array> {
  public:
    /// Constructor
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param align the byte alignment of the array
    /// @param size the byte size of the array. The size will be 0 if the array element count is
    ///        pipeline overridable.
    /// @param stride the number of bytes from the start of one element of the
    ///        array to the start of the next element
    /// @param implicit_stride the number of bytes from the start of one element
    /// of the array to the start of the next element, if there was no `@stride`
    /// attribute applied.
    Array(core::type::Type const* element,
          const core::type::ArrayCount* count,
          uint32_t align,
          uint32_t size,
          uint32_t stride,
          uint32_t implicit_stride);

    /// Destructor
    ~Array() override;
};

}  // namespace tint::sem

#endif  // SRC_TINT_LANG_WGSL_SEM_ARRAY_H_
