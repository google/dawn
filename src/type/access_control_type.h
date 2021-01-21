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

#ifndef SRC_TYPE_ACCESS_CONTROL_TYPE_H_
#define SRC_TYPE_ACCESS_CONTROL_TYPE_H_

#include <string>

#include "src/ast/access_control.h"
#include "src/type/type.h"

namespace tint {
namespace type {

/// An access control type. Holds an access setting and pointer to another type.
class AccessControl : public Castable<AccessControl, Type> {
 public:
  /// Constructor
  /// @param access the access control setting
  /// @param subtype the access controlled type
  AccessControl(ast::AccessControl access, Type* subtype);
  /// Move constructor
  AccessControl(AccessControl&&);
  ~AccessControl() override;

  /// @returns true if the access control is read only
  bool IsReadOnly() const { return access_ == ast::AccessControl::kReadOnly; }
  /// @returns true if the access control is write only
  bool IsWriteOnly() const { return access_ == ast::AccessControl::kWriteOnly; }
  /// @returns true if the access control is read/write
  bool IsReadWrite() const { return access_ == ast::AccessControl::kReadWrite; }

  /// @returns the access control value
  ast::AccessControl access_control() const { return access_; }
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

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  AccessControl* Clone(CloneContext* ctx) const override;

 private:
  ast::AccessControl const access_;
  Type* const subtype_;
};

}  // namespace type
}  // namespace tint

#endif  // SRC_TYPE_ACCESS_CONTROL_TYPE_H_
