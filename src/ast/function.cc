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

#include "src/ast/function.h"

#include <sstream>

namespace tint {
namespace ast {

Function::Function() = default;

Function::Function(const std::string& name,
                   VariableList params,
                   type::Type* return_type)
    : Node(),
      name_(name),
      params_(std::move(params)),
      return_type_(return_type) {}

Function::Function(const Source& source,
                   const std::string& name,
                   VariableList params,
                   type::Type* return_type)
    : Node(source),
      name_(name),
      params_(std::move(params)),
      return_type_(return_type) {}

Function::Function(Function&&) = default;

Function::~Function() = default;

bool Function::IsValid() const {
  for (const auto& param : params_) {
    if (param == nullptr || !param->IsValid())
      return false;
  }
  for (const auto& stmt : body_) {
    if (stmt == nullptr || !stmt->IsValid())
      return false;
  }

  if (name_.length() == 0) {
    return false;
  }
  if (return_type_ == nullptr) {
    return false;
  }
  return true;
}

void Function::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Function " << name_ << " -> " << return_type_->type_name()
      << std::endl;

  make_indent(out, indent);
  out << "(";

  if (params_.size() > 0) {
    out << std::endl;

    for (const auto& param : params_)
      param->to_str(out, indent + 2);

    make_indent(out, indent);
  }
  out << ")" << std::endl;

  make_indent(out, indent);
  out << "{" << std::endl;

  for (const auto& stmt : body_)
    stmt->to_str(out, indent + 2);

  make_indent(out, indent);
  out << "}" << std::endl;
}

std::string Function::type_name() const {
  std::ostringstream out;

  out << "__func" + return_type_->type_name();
  for (const auto& param : params_) {
    out << param->type()->type_name();
  }

  return out.str();
}

}  // namespace ast
}  // namespace tint
