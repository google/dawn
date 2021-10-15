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

#ifndef SRC_AST_TYPE_H_
#define SRC_AST_TYPE_H_

#include <string>

#include "src/ast/node.h"
#include "src/clone_context.h"

namespace tint {

// Forward declarations
class ProgramBuilder;
class SymbolTable;

namespace ast {

/// Base class for a type in the system
class Type : public Castable<Type, Node> {
 public:
  /// Move constructor
  Type(Type&&);
  ~Type() override;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  virtual std::string FriendlyName(const SymbolTable& symbols) const = 0;

 protected:
  /// Constructor
  /// @param pid the identifier of the program that owns this node
  /// @param src the source of this node
  Type(ProgramID pid, const Source& src);
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_H_
