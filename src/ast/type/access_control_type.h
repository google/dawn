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

#ifndef SRC_AST_TYPE_ACCESS_CONTROL_TYPE_H_
#define SRC_AST_TYPE_ACCESS_CONTROL_TYPE_H_

#include <string>

#include "src/ast/access_control.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// An access control type. Holds an access setting and pointer to another type.
class AccessControlType : public Castable<AccessControlType, Type> {
 public:
  /// Constructor
  /// @param access the access control setting
  /// @param subtype the access controlled type
  AccessControlType(AccessControl access, Type* subtype);
  /// Move constructor
  AccessControlType(AccessControlType&&);
  ~AccessControlType() override;

  /// @returns true if the access control is read only
  bool IsReadOnly() const { return access_ == AccessControl::kReadOnly; }
  /// @returns true if the access control is write only
  bool IsWriteOnly() const { return access_ == AccessControl::kWriteOnly; }
  /// @returns true if the access control is read/write
  bool IsReadWrite() const { return access_ == AccessControl::kReadWrite; }

  /// @returns the access control value
  AccessControl access_control() const { return access_; }
  /// @returns the subtype type
  Type* type() const { return subtype_; }

  /// @returns the name for this type
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
  AccessControl access_ = AccessControl::kReadOnly;
  Type* subtype_ = nullptr;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ACCESS_CONTROL_TYPE_H_
