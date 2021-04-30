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

#ifndef SRC_AST_ALIAS_H_
#define SRC_AST_ALIAS_H_

#include <string>

#include "src/ast/named_type.h"

namespace tint {
namespace ast {

/// A type alias type. Holds a name and pointer to another type.
class Alias : public Castable<Alias, NamedType> {
 public:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  /// @param name the symbol for the alias
  /// @param subtype the alias'd type
  Alias(ProgramID program_id,
        const Source& source,
        const Symbol& name,
        Type* subtype);
  /// Move constructor
  Alias(Alias&&);
  /// Destructor
  ~Alias() override;

  /// [DEPRECATED] use name()
  /// @returns the alias symbol
  Symbol symbol() const { return name(); }
  /// @returns the alias type
  Type* type() const { return subtype_; }

  /// @returns the type_name for this type
  std::string type_name() const override;

  /// Clones this type and all transitive types using the `CloneContext` `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned type
  Alias* Clone(CloneContext* ctx) const override;

 private:
  Type* const subtype_;
  std::string const type_name_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_ALIAS_H_
