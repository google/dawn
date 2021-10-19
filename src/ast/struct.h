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

#ifndef SRC_AST_STRUCT_H_
#define SRC_AST_STRUCT_H_

#include <string>
#include <utility>

#include "src/ast/decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/type_decl.h"

namespace tint {
namespace ast {

/// A struct statement.
class Struct : public Castable<Struct, TypeDecl> {
 public:
  /// Create a new struct statement
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node for the import statement
  /// @param name The name of the structure
  /// @param members The struct members
  /// @param decorations The struct decorations
  Struct(ProgramID pid,
         const Source& src,
         Symbol name,
         StructMemberList members,
         DecorationList decorations);
  /// Move constructor
  Struct(Struct&&);

  ~Struct() override;

  /// @returns true if the struct is block decorated
  bool IsBlockDecorated() const;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  const Struct* Clone(CloneContext* ctx) const override;

  /// The members
  const StructMemberList members;

  /// The struct decorations
  const DecorationList decorations;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_H_
