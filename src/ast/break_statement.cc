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

#include "src/ast/break_statement.h"

#include "src/clone_context.h"
#include "src/program.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::BreakStatement);

namespace tint {
namespace ast {

BreakStatement::BreakStatement(const Source& source) : Base(source) {}

BreakStatement::BreakStatement(BreakStatement&&) = default;

BreakStatement::~BreakStatement() = default;

BreakStatement* BreakStatement::Clone(CloneContext* ctx) const {
  return ctx->dst->create<BreakStatement>(ctx->Clone(source()));
}

bool BreakStatement::IsValid() const {
  return true;
}

void BreakStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Break{}" << std::endl;
}

}  // namespace ast
}  // namespace tint
