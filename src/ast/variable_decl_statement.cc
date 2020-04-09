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

#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace ast {

VariableDeclStatement::VariableDeclStatement() : Statement() {}

VariableDeclStatement::VariableDeclStatement(std::unique_ptr<Variable> variable)
    : Statement(), variable_(std::move(variable)) {}

VariableDeclStatement::VariableDeclStatement(const Source& source,
                                             std::unique_ptr<Variable> variable)
    : Statement(source), variable_(std::move(variable)) {}

VariableDeclStatement::VariableDeclStatement(VariableDeclStatement&&) = default;

VariableDeclStatement::~VariableDeclStatement() = default;

bool VariableDeclStatement::IsVariableDecl() const {
  return true;
}

bool VariableDeclStatement::IsValid() const {
  return variable_ != nullptr && variable_->IsValid();
}

void VariableDeclStatement::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "VariableDeclStatement{" << std::endl;
  variable_->to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
