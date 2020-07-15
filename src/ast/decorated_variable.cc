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

#include "src/ast/decorated_variable.h"

namespace tint {
namespace ast {

DecoratedVariable::DecoratedVariable() = default;

DecoratedVariable::DecoratedVariable(std::unique_ptr<Variable> var)
    : Variable(var->source(), var->name(), var->storage_class(), var->type()) {}

DecoratedVariable::DecoratedVariable(DecoratedVariable&&) = default;

DecoratedVariable::~DecoratedVariable() = default;

bool DecoratedVariable::HasLocationDecoration() const {
  for (const auto& deco : decorations_) {
    if (deco->IsLocation()) {
      return true;
    }
  }
  return false;
}

bool DecoratedVariable::HasBuiltinDecoration() const {
  for (const auto& deco : decorations_) {
    if (deco->IsBuiltin()) {
      return true;
    }
  }
  return false;
}

bool DecoratedVariable::IsDecorated() const {
  return true;
}

bool DecoratedVariable::IsValid() const {
  return Variable::IsValid();
}

void DecoratedVariable::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "DecoratedVariable{" << std::endl;

  make_indent(out, indent + 2);
  out << "Decorations{" << std::endl;
  for (const auto& deco : decorations_) {
    make_indent(out, indent + 4);
    deco->to_str(out);
  }

  make_indent(out, indent + 2);
  out << "}" << std::endl;

  info_to_str(out, indent + 2);
  constructor_to_str(out, indent + 2);
  make_indent(out, indent);
  out << "}" << std::endl;
}

}  // namespace ast
}  // namespace tint
