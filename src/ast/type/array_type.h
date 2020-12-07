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

#ifndef SRC_AST_TYPE_ARRAY_TYPE_H_
#define SRC_AST_TYPE_ARRAY_TYPE_H_

#include <assert.h>

#include <string>
#include <utility>

#include "src/ast/array_decoration.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// An array type. If size is zero then it is a runtime array.
class Array : public Castable<Array, Type> {
 public:
  /// Constructor
  /// @param subtype the type of the array elements
  /// @param size the number of elements in the array. `0` represents a
  /// runtime-sized array.
  /// @param decorations the array decorations
  Array(Type* subtype, uint32_t size, ArrayDecorationList decorations);
  /// Move constructor
  Array(Array&&);
  ~Array() override;

  /// @returns true if this is a runtime array.
  /// i.e. the size is determined at runtime
  bool IsRuntimeArray() const { return size_ == 0; }

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns minimum size required for this type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t MinBufferBindingSize(MemoryLayout mem_layout) const override;

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns base alignment for the type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t BaseAlignment(MemoryLayout mem_layout) const override;

  /// @returns the array decorations
  const ArrayDecorationList& decorations() const { return decos_; }

  /// @returns the array stride or 0 if none set.
  uint32_t array_stride() const;
  /// @returns true if the array has a stride set
  bool has_array_stride() const;

  /// @returns the array type
  Type* type() const { return subtype_; }
  /// @returns the array size. Size is 0 for a runtime array
  uint32_t size() const { return size_; }

  /// @returns the name for the type
  std::string type_name() const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Array* Clone(CloneContext* ctx) const override;

 private:
  Type* subtype_ = nullptr;
  uint32_t size_ = 0;
  ArrayDecorationList decos_;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ARRAY_TYPE_H_
