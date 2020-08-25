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

#ifndef SRC_AST_STRUCT_MEMBER_H_
#define SRC_AST_STRUCT_MEMBER_H_

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "src/ast/node.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type/type.h"

namespace tint {
namespace ast {

/// A struct member statement.
class StructMember : public Node {
 public:
  /// Create a new empty struct member statement
  StructMember();
  /// Create a new struct member statement
  /// @param name The struct member name
  /// @param type The struct member type
  /// @param decorations The struct member decorations
  StructMember(const std::string& name,
               type::Type* type,
               StructMemberDecorationList decorations);
  /// Create a new struct member statement
  /// @param source The input source for the struct member statement
  /// @param name The struct member name
  /// @param type The struct member type
  /// @param decorations The struct member decorations
  StructMember(const Source& source,
               const std::string& name,
               type::Type* type,
               StructMemberDecorationList decorations);
  /// Move constructor
  StructMember(StructMember&&);

  ~StructMember() override;

  /// Sets the name
  /// @param name the name to set
  void set_name(const std::string& name) { name_ = name; }
  /// @returns the name
  const std::string& name() const { return name_; }
  /// Sets the type
  /// @param type the type to set
  void set_type(type::Type* type) { type_ = type; }
  /// @returns the type
  type::Type* type() const { return type_; }
  /// Sets the decorations
  /// @param decorations the decorations
  void set_decorations(StructMemberDecorationList decorations) {
    decorations_ = std::move(decorations);
  }
  /// @returns the decorations
  const StructMemberDecorationList& decorations() const { return decorations_; }

  /// @returns true if the struct member has an offset decoration
  bool has_offset_decoration() const;
  /// @returns the offset decoration value.
  uint32_t offset() const;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  StructMember(const StructMember&) = delete;

  std::string name_;
  type::Type* type_ = nullptr;
  StructMemberDecorationList decorations_;
};

/// A list of unique struct members
using StructMemberList = std::vector<std::unique_ptr<StructMember>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STRUCT_MEMBER_H_
