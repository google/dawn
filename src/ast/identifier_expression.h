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

#ifndef SRC_AST_IDENTIFIER_EXPRESSION_H_
#define SRC_AST_IDENTIFIER_EXPRESSION_H_

#include <string>
#include <utility>
#include <vector>

#include "src/ast/expression.h"

namespace tint {
namespace ast {

/// An identifier expression
class IdentifierExpression : public Expression {
 public:
  /// Constructor
  /// @param name the name
  explicit IdentifierExpression(const std::string& name);
  /// Constructor
  /// @param source the source
  /// @param name the name
  IdentifierExpression(const Source& source, const std::string& name);
  /// Constructor
  /// @param segments the name segments
  explicit IdentifierExpression(std::vector<std::string> segments);
  /// Constructor
  /// @param source the identifier expression source
  /// @param segments the name segments
  IdentifierExpression(const Source& source, std::vector<std::string> segments);
  /// Move constructor
  IdentifierExpression(IdentifierExpression&&);
  ~IdentifierExpression() override;

  /// Sets the name segments
  /// @param segments the name segments
  void set_segments(std::vector<std::string> segments) {
    segments_ = std::move(segments);
  }
  /// @returns the name
  std::vector<std::string> segments() const { return segments_; }

  /// @returns true if this identifier has a path component
  bool has_path() const { return segments_.size() > 1; }
  /// @returns the path part of the identifier or blank if no path
  std::string path() const;
  /// @returns the name part of the identifier
  std::string name() const { return segments_.back(); }

  /// @returns true if this is an identifier expression
  bool IsIdentifier() const override;

  /// @returns true if the node is valid
  bool IsValid() const override;

  /// Writes a representation of the node to the output stream
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(std::ostream& out, size_t indent) const override;

 private:
  IdentifierExpression(const IdentifierExpression&) = delete;

  std::vector<std::string> segments_;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_IDENTIFIER_EXPRESSION_H_
