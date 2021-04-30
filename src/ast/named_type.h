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

#ifndef SRC_AST_NAMED_TYPE_H_
#define SRC_AST_NAMED_TYPE_H_

#include <string>

#include "src/ast/type.h"

namespace tint {
namespace ast {

/// The base class for user declared, named types.
class NamedType : public Castable<NamedType, Type> {
 public:
  /// Create a new struct statement
  /// @param program_id the identifier of the program that owns this node
  /// @param source The input source for the import statement
  /// @param name The name of the structure
  NamedType(ProgramID program_id, const Source& source, Symbol name);
  /// Move constructor
  NamedType(NamedType&&);

  ~NamedType() override;

  /// @returns the name of the structure
  Symbol name() const { return name_; }

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  std::string FriendlyName(const SymbolTable& symbols) const override;

 private:
  NamedType(const NamedType&) = delete;
  NamedType& operator=(const NamedType&) = delete;

  Symbol const name_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_NAMED_TYPE_H_
