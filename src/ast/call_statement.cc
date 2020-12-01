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

#include "src/ast/call_expression.h"
#include "src/ast/clone_context.h"
#include "src/ast/module.h"

namespace tint {
namespace ast {

CallStatement::CallStatement() : Base() {}

CallStatement::CallStatement(CallExpression* call) : Base(), call_(call) {}

CallStatement::CallStatement(CallStatement&&) = default;

CallStatement::~CallStatement() = default;

CallStatement* CallStatement::Clone(CloneContext* ctx) const {
  return ctx->mod->create<CallStatement>(ctx->Clone(call_));
}

bool CallStatement::IsValid() const {
  return call_ != nullptr && call_->IsValid();
}

void CallStatement::to_str(std::ostream& out, size_t indent) const {
  call_->to_str(out, indent);
}

}  // namespace ast
}  // namespace tint
