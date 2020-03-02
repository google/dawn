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

#include "src/ast/node.h"

#include <sstream>

namespace tint {
namespace ast {

Node::Node() = default;

Node::Node(const Source& source) : source_(source) {}

Node::~Node() = default;

void Node::make_indent(std::ostream& out, size_t indent) const {
  for (size_t i = 0; i < indent; ++i)
    out << " ";
}

std::string Node::str() const {
  std::ostringstream out;
  to_str(out, 0);
  return out.str();
}

}  // namespace ast
}  // namespace tint
