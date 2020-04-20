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

#include "src/ast/identifier_expression.h"

namespace tint {
namespace ast {

IdentifierExpression::IdentifierExpression(const std::string& name)
    : Expression(), segments_({name}) {}

IdentifierExpression::IdentifierExpression(const Source& source,
                                           const std::string& name)
    : Expression(source), segments_({name}) {}

IdentifierExpression::IdentifierExpression(std::vector<std::string> segments)
    : Expression(), segments_(std::move(segments)) {}

IdentifierExpression::IdentifierExpression(const Source& source,
                                           std::vector<std::string> segments)
    : Expression(source), segments_(std::move(segments)) {}

IdentifierExpression::IdentifierExpression(IdentifierExpression&&) = default;

IdentifierExpression::~IdentifierExpression() = default;

std::string IdentifierExpression::path() const {
  if (segments_.size() < 2) {
    return "";
  }

  std::string path = "";
  // We skip the last segment as that's the name, not part of the path
  for (size_t i = 0; i < segments_.size() - 1; ++i) {
    if (i > 0) {
      path += "::";
    }
    path += segments_[i];
  }
  return path;
}

bool IdentifierExpression::IsIdentifier() const {
  return true;
}

bool IdentifierExpression::IsValid() const {
  if (segments_.size() == 0)
    return false;

  for (const auto& segment : segments_) {
    if (segment.size() == 0)
      return false;
  }
  return true;
}

void IdentifierExpression::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Identifier{";
  if (has_path()) {
    out << path() << "::";
  }
  out << name();
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
