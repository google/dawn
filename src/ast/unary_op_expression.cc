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

#include "src/ast/unary_op_expression.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::UnaryOpExpression);

namespace tint {
namespace ast {

UnaryOpExpression::UnaryOpExpression(const Source& source,
                                     UnaryOp op,
                                     Expression* expr)
    : Base(source), op_(op), expr_(expr) {}

UnaryOpExpression::UnaryOpExpression(UnaryOpExpression&&) = default;

UnaryOpExpression::~UnaryOpExpression() = default;

UnaryOpExpression* UnaryOpExpression::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* e = ctx->Clone(expr());
  return ctx->dst->create<UnaryOpExpression>(src, op_, e);
}

bool UnaryOpExpression::IsValid() const {
  return expr_ != nullptr && expr_->IsValid();
}

void UnaryOpExpression::to_str(const semantic::Info& sem,
                               std::ostream& out,
                               size_t indent) const {
  make_indent(out, indent);
  out << "UnaryOp[" << result_type_str(sem) << "]{" << std::endl;
  make_indent(out, indent + 2);
  out << op_ << std::endl;
  expr_->to_str(sem, out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
