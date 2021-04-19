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

#ifndef SRC_AST_NODE_H_
#define SRC_AST_NODE_H_

#include <string>

#include "src/clone_context.h"
#include "src/program_id.h"

namespace tint {

// Forward declarations
class CloneContext;
namespace sem {
class Type;
}
namespace sem {
class Info;
}

namespace ast {

/// AST base class node
class Node : public Castable<Node, Cloneable> {
 public:
  ~Node() override;

  /// @returns the identifier of the program that owns this node
  ProgramID program_id() const { return program_id_; }

  /// @returns the node source data
  const Source& source() const { return source_; }

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  virtual void to_str(const sem::Info& sem,
                      std::ostream& out,
                      size_t indent) const = 0;

  /// Convenience wrapper around the to_str() method.
  /// @param sem the semantic info for the program
  /// @returns the node as a string
  std::string str(const sem::Info& sem) const;

 protected:
  /// Create a new node
  /// @param program_id the identifier of the program that owns this node
  /// @param source the input source for the node
  Node(ProgramID program_id, const Source& source);
  /// Move constructor
  Node(Node&&);

  /// Writes indent into stream
  /// @param out the stream to write to
  /// @param indent the number of spaces to write
  void make_indent(std::ostream& out, size_t indent) const;

 private:
  Node(const Node&) = delete;

  ProgramID const program_id_;
  Source const source_;
};

}  // namespace ast

/// @param node a pointer to an AST node
/// @returns the ProgramID of the given AST node.
inline ProgramID ProgramIDOf(const ast::Node* node) {
  return node ? node->program_id() : ProgramID();
}

}  // namespace tint

#endif  // SRC_AST_NODE_H_
