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

#include "src/ast/call_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::CallStatement);

namespace tint {
namespace ast {

CallStatement::CallStatement(ProgramID program_id,
                             const Source& source,
                             CallExpression* call)
    : Base(program_id, source), call_(call) {
  TINT_ASSERT(call_);
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(call_, program_id);
}

CallStatement::CallStatement(CallStatement&&) = default;

CallStatement::~CallStatement() = default;

CallStatement* CallStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* call = ctx->Clone(call_);
  return ctx->dst->create<CallStatement>(src, call);
}

void CallStatement::to_str(const sem::Info& sem,
                           std::ostream& out,
                           size_t indent) const {
  call_->to_str(sem, out, indent);
}

}  // namespace ast
}  // namespace tint
