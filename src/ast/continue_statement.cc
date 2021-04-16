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

#include "src/ast/continue_statement.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::ContinueStatement);

namespace tint {
namespace ast {

ContinueStatement::ContinueStatement(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

ContinueStatement::ContinueStatement(ContinueStatement&&) = default;

ContinueStatement::~ContinueStatement() = default;

ContinueStatement* ContinueStatement::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  return ctx->dst->create<ContinueStatement>(src);
}

void ContinueStatement::to_str(const sem::Info&,
                               std::ostream& out,
                               size_t indent) const {
  make_indent(out, indent);
  out << "Continue{}" << std::endl;
}

}  // namespace ast
}  // namespace tint
