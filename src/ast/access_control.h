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

#ifndef SRC_AST_ACCESS_CONTROL_H_
#define SRC_AST_ACCESS_CONTROL_H_

#include <ostream>
#include <string>

#include "src/ast/type.h"

namespace tint {
namespace ast {

/// An access control type. Holds an access setting and pointer to another type.
class AccessControl : public Castable<AccessControl, Type> {
 public:
  /// The access control settings
  enum Access {
    /// Read only
    kReadOnly,
    /// Write only
    kWriteOnly,
    /// Read write
    kReadWrite
  };

  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param access the access control setting
  /// @param subtype the access controlled type
  AccessControl(ProgramID program_id,
                const Source& source,
                Access access,
                const Type* subtype);
  /// Move constructor
  AccessControl(AccessControl&&);
  ~AccessControl() override;

  /// @returns true if the access control is read only
  bool IsReadOnly() const { return access_ == Access::kReadOnly; }
  /// @returns true if the access control is write only
  bool IsWriteOnly() const { return access_ == Access::kWriteOnly; }
  /// @returns true if the access control is read/write
  bool IsReadWrite() const { return access_ == Access::kReadWrite; }

  /// @returns the access control value
  Access access_control() const { return access_; }
  /// @returns the subtype type
  Type* type() const { return const_cast<Type*>(subtype_); }

  /// @returns the name for this type
  std::string type_name() const override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  AccessControl* Clone(CloneContext* ctx) const override;

 private:
  Access const access_;
  const Type* const subtype_;
};

/// @param out the std::ostream to write to
/// @param access the AccessControl
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, AccessControl::Access access);

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ACCESS_CONTROL_H_
