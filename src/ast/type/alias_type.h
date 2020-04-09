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

#ifndef SRC_AST_TYPE_ALIAS_TYPE_H_
#define SRC_AST_TYPE_ALIAS_TYPE_H_

#include <string>

#include "src/ast/type/type.h"

namespace tint {
namespace ast {
namespace type {

/// A type alias type. Holds a name a pointer to another type.
class AliasType : public Type {
 public:
  /// Constructor
  /// @param name the alias name
  /// @param subtype the alias'd type
  AliasType(const std::string& name, Type* subtype);
  /// Move constructor
  AliasType(AliasType&&) = default;
  ~AliasType() override;

  /// @returns true if the type is an alias type
  bool IsAlias() const override;

  /// @returns the alias name
  const std::string& name() const { return name_; }
  /// @returns the alias type
  Type* type() const { return subtype_; }

  /// @returns the name for this type
  std::string type_name() const override;

 private:
  std::string name_;
  Type* subtype_ = nullptr;
};

}  // namespace type
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_ALIAS_TYPE_H_
