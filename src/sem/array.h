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
#include <string>

#include "src/sem/node.h"
#include "src/sem/type.h"

// Forward declarations
namespace tint {
namespace ast {
class Array;
}  // namespace ast
}  // namespace tint

namespace tint {
namespace sem {

/// Array holds the semantic information for Array nodes.
class Array : public Castable<Array, Type> {
 public:
  /// Constructor
  /// @param element the array element type
  /// @param count the number of elements in the array. 0 represents a
  /// runtime-sized array.
  /// @param align the byte alignment of the array
  /// @param size the byte size of the array
  /// @param stride the number of bytes from the start of one element of the
  /// array to the start of the next element
  /// @param stride_implicit is true if the value of `stride` matches the
  /// element's natural stride.
  Array(Type const* element,
        uint32_t count,
        uint32_t align,
        uint32_t size,
        uint32_t stride,
        bool stride_implicit);

  /// @return the array element type
  Type const* ElemType() const { return element_; }

  /// @returns the number of elements in the array. 0 represents a runtime-sized
  /// array.
  uint32_t Count() const { return count_; }

  /// @returns the byte alignment of the array
  /// @note this may differ from the alignment of a structure member of this
  /// array type, if the member is annotated with the `[[align(n)]]` decoration.
  uint32_t Align() const { return align_; }

  /// @returns the byte size of the array
  /// @note this may differ from the size of a structure member of this array
  /// type, if the member is annotated with the `[[size(n)]]` decoration.
  uint32_t SizeInBytes() const { return size_; }

  /// @returns the number of bytes from the start of one element of the
  /// array to the start of the next element
  uint32_t Stride() const { return stride_; }

  /// @returns true if the value returned by Stride() does matches the
  /// element's natural stride
  bool IsStrideImplicit() const { return stride_implicit_; }

  /// @returns true if this array is runtime sized
  bool IsRuntimeSized() const { return count_ == 0; }

  /// @returns the name for the type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  Type const* const element_;
  uint32_t const count_;
  uint32_t const align_;
  uint32_t const size_;
  uint32_t const stride_;
  bool const stride_implicit_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_ARRAY_H_
