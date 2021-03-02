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

#include "src/ast/assignment_statement.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::AssignmentStatement);

namespace tint {
namespace ast {

AssignmentStatement::AssignmentStatement(const Source& source,
                                         Expression* lhs,
                                         Expression* rhs)
    : Base(source), lhs_(lhs), rhs_(rhs) {}

AssignmentStatement::AssignmentStatement(AssignmentStatement&&) = default;

AssignmentStatement::~AssignmentStatement() = default;

AssignmentStatement* AssignmentStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* l = ctx->Clone(lhs_);
  auto* r = ctx->Clone(rhs_);
  return ctx->dst->create<AssignmentStatement>(src, l, r);
}

bool AssignmentStatement::IsValid() const {
  if (lhs_ == nullptr || !lhs_->IsValid())
    return false;
  if (rhs_ == nullptr || !rhs_->IsValid())
    return false;

  return true;
}

void AssignmentStatement::to_str(const semantic::Info& sem,
                                 std::ostream& out,
                                 size_t indent) const {
  make_indent(out, indent);
  out << "Assignment{" << std::endl;
  lhs_->to_str(sem, out, indent + 2);
  rhs_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
