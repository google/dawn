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

#ifndef SRC_AST_TYPE_STRUCT_TYPE_H_
#define SRC_AST_TYPE_STRUCT_TYPE_H_

#include <memory>
#include <string>

#include "src/ast/struct.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A structure type
class StructType : public Type {
 public:
  /// Constructor
  /// @param name the name of the struct
  /// @param impl the struct data
  StructType(const std::string& name, std::unique_ptr<Struct> impl);
  /// Move constructor
  StructType(StructType&&);
  ~StructType() override;

  /// @returns the struct name
  const std::string& name() const { return name_; }

  /// @returns true if the struct has a block decoration
  bool IsBlockDecorated() const { return struct_->IsBlockDecorated(); }

  /// @returns true if the type is a struct type
  bool IsStruct() const override;

  /// @returns the struct name
  Struct* impl() const { return struct_.get(); }

  /// @returns the name for the type
  std::string type_name() const override;

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns minimum size required for this type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t MinBufferBindingSize(MemoryLayout mem_layout) const override;

  /// @param mem_layout type of memory layout to use in calculation.
  /// @returns base alignment for the type, in bytes.
  ///          0 for non-host shareable types.
  uint64_t BaseAlignment(MemoryLayout mem_layout) const override;

 private:
  std::string name_;
  std::unique_ptr<Struct> struct_;

  uint64_t LargestMemberBaseAlignment(MemoryLayout mem_layout) const;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_STRUCT_TYPE_H_
