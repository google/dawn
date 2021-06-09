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

#ifndef SRC_AST_TYPE_DECL_H_
#define SRC_AST_TYPE_DECL_H_

#include <string>

#include "src/ast/type.h"

namespace tint {
namespace ast {

/// The base class for type declarations.
class TypeDecl : public Castable<TypeDecl, Node> {
 public:
  /// Create a new struct statement
  /// @param program_id the identifier of the program that owns this node
  /// @param source The input source for the import statement
  /// @param name The name of the structure
  TypeDecl(ProgramID program_id, const Source& source, Symbol name);
  /// Move constructor
  TypeDecl(TypeDecl&&);

  ~TypeDecl() override;

  /// @returns the name of the type declaration
  Symbol name() const { return name_; }

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// @returns the name for this type. The type name is unique over all types.
  virtual std::string type_name() const = 0;

 private:
  TypeDecl(const TypeDecl&) = delete;
  TypeDecl& operator=(const TypeDecl&) = delete;

  Symbol const name_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_DECL_H_
