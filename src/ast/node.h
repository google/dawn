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

#include <ostream>
#include <string>

#include "src/source.h"

namespace tint {
namespace ast {

/// AST base class node
class Node {
 public:
  virtual ~Node();

  /// @returns the node source data
  const Source& source() const { return source_; }
  /// Sets the source data
  /// @param source the source data
  void set_source(const Source& source) { source_ = source; }

  /// @returns the line the node was declared on
  size_t line() const { return source_.line; }
  /// @returns the column the node was declared on
  size_t column() const { return source_.column; }

  /// @returns true if the node is valid
  virtual bool IsValid() const = 0;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  virtual void to_str(std::ostream& out, size_t indent) const = 0;

  /// Convenience wrapper around the |to_str| method.
  /// @returns the node as a string
  std::string str() const;

 protected:
  /// Create a new node
  Node();
  /// Create a new node
  /// @param source The input source for the node
  explicit Node(const Source& source);
  /// Move constructor
  Node(Node&&) = default;

  /// Writes indent into stream
  /// @param out the stream to write to
  /// @param indent the number of spaces to write
  void make_indent(std::ostream& out, size_t indent) const;

 private:
  Node(const Node&) = delete;

  Source source_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_NODE_H_
