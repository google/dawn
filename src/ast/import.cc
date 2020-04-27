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

#include "src/ast/import.h"

#include <cctype>

namespace tint {
namespace ast {

Import::Import() = default;

Import::Import(const std::string& path, const std::string& name)
    : Node(), path_(path), name_(name) {}

Import::Import(const Source& source,
               const std::string& path,
               const std::string& name)
    : Node(source), path_(path), name_(name) {}

Import::Import(Import&&) = default;

Import::~Import() = default;

bool Import::IsValid() const {
  if (path_.length() == 0) {
    return false;
  }

  auto len = name_.length();
  if (len == 0) {
    return false;
  }

  // Verify the import name ends in a character, number or _
  if (len > 2 && !std::isalnum(name_[len - 1]) && name_[len] != '_') {
    return false;
  }

  return true;
}

void Import::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << R"(Import{")" + path_ + R"(" as )" + name_ + "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
