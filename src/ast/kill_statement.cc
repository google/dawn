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

#include "src/ast/kill_statement.h"

namespace tint {
namespace ast {

KillStatement::KillStatement() : Statement() {}

KillStatement::KillStatement(const Source& source) : Statement(source) {}

KillStatement::~KillStatement() = default;

bool KillStatement::IsKill() const {
  return true;
}

bool KillStatement::IsValid() const {
  return true;
}

void KillStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Kill{}" << std::endl;
}

}  // namespace ast
}  // namespace tint
