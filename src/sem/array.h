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

#ifndef SRC_SEM_ARRAY_H_
#define SRC_SEM_ARRAY_H_

#include <stdint.h>

#include "src/sem/node.h"

namespace tint {

// Forward declarations
namespace type {
class ArrayType;
}  // namespace type

namespace sem {

/// Array holds the semantic information for Array nodes.
class Array : public Castable<Array, Node> {
 public:
  /// Constructor
  /// @param type the Array type
  /// @param align the byte alignment of the structure
  /// @param size the byte size of the structure
  /// @param stride the number of bytes from the start of one element of the
  /// array to the start of the next element
  Array(type::ArrayType* type, uint32_t align, uint32_t size, uint32_t stride);

  /// @return the resolved type of the Array
  type::ArrayType* Type() const { return type_; }

  /// @returns the byte alignment of the array
  /// @note this may differ from the alignment of a structure member of this
  /// array type, if the member is annotated with the `[[align(n)]]` decoration.
  uint32_t Align() const { return align_; }

  /// @returns the byte size of the array
  /// @note this may differ from the size of a structure member of this array
  /// type, if the member is annotated with the `[[size(n)]]` decoration.
  uint32_t Size() const { return size_; }

  /// @returns the number of bytes from the start of one element of the
  /// array to the start of the next element
  uint32_t Stride() const { return stride_; }

 private:
  type::ArrayType* const type_;
  uint32_t const align_;
  uint32_t const size_;
  uint32_t const stride_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_ARRAY_H_
