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

  /// @returns the name for this type. The type name is unique over all types.
  virtual std::string type_name() const = 0;

  /// @param symbols the program's symbol table
  /// @returns the name for this type that closely resembles how it would be
  /// declared in WGSL.
  virtual std::string FriendlyName(const SymbolTable& symbols) const = 0;

  /// @returns the type with all aliasing, access control and pointers removed
  Type* UnwrapAll();

  /// @returns the type with all aliasing, access control and pointers removed
  const Type* UnwrapAll() const { return const_cast<Type*>(this)->UnwrapAll(); }

  /// @returns true if this type is a scalar
  bool is_scalar() const;
  /// @returns true if this type is a float scalar
  bool is_float_scalar() const;
  /// @returns true if this type is a float matrix
  bool is_float_matrix() const;
  /// @returns true if this type is a float vector
  bool is_float_vector() const;
  /// @returns true if this type is a float scalar or vector
  bool is_float_scalar_or_vector() const;
  /// @returns true if this type is a float scalar or vector or matrix
  bool is_float_scalar_or_vector_or_matrix() const;
  /// @returns true if this type is an integer scalar
  bool is_integer_scalar() const;
  /// @returns true if this type is a signed integer vector
  bool is_signed_integer_vector() const;
  /// @returns true if this type is an unsigned vector
  bool is_unsigned_integer_vector() const;
  /// @returns true if this type is an unsigned scalar or vector
  bool is_unsigned_scalar_or_vector() const;
  /// @returns true if this type is a signed scalar or vector
  bool is_signed_scalar_or_vector() const;
  /// @returns true if this type is an integer scalar or vector
  bool is_integer_scalar_or_vector() const;
  /// @returns true if this type is a boolean vector
  bool is_bool_vector() const;
  /// @returns true if this type is boolean scalar or vector
  bool is_bool_scalar_or_vector() const;
  /// @returns true if this type is a handle type
  bool is_handle() const;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

 protected:
  /// Constructor
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this node
  Type(ProgramID program_id, const Source& source);
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TYPE_H_
