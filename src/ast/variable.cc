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

#include "src/ast/variable.h"

#include <assert.h>

#include "src/ast/decorated_variable.h"

namespace tint {
namespace ast {

Variable::Variable() = default;

Variable::Variable(const std::string& name, StorageClass sc, type::Type* type)
    : Node(), name_(name), storage_class_(sc), type_(type) {}

Variable::Variable(const Source& source,
                   const std::string& name,
                   StorageClass sc,
                   type::Type* type)
    : Node(source), name_(name), storage_class_(sc), type_(type) {}

Variable::Variable(Variable&&) = default;

Variable::~Variable() = default;

DecoratedVariable* Variable::AsDecorated() {
  assert(IsDecorated());
  return static_cast<DecoratedVariable*>(this);
}

bool Variable::IsDecorated() const {
  return false;
}

bool Variable::IsValid() const {
  if (name_.length() == 0) {
    return false;
  }
  if (type_ == nullptr) {
    return false;
  }
  if (constructor_ && !constructor_->IsValid()) {
    return false;
  }
  return true;
}

void Variable::info_to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << name_ << std::endl;
  make_indent(out, indent);
  out << storage_class_ << std::endl;
  make_indent(out, indent);
  out << type_->type_name() << std::endl;
}

void Variable::constructor_to_str(std::ostream& out, size_t indent) const {
  if (constructor_ == nullptr)
    return;

  make_indent(out, indent);
  out << "{" << std::endl;

  constructor_->to_str(out, indent + 2);

  make_indent(out, indent);
  out << "}" << std::endl;
}

void Variable::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "Variable{" << std::endl;
  info_to_str(out, indent + 2);
  constructor_to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
