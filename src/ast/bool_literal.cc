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

#include "src/ast/bool_literal.h"

namespace tint {
namespace ast {

BoolLiteral::BoolLiteral(ast::type::Type* type, bool value)
    : Literal(type), value_(value) {}

BoolLiteral::~BoolLiteral() = default;

bool BoolLiteral::IsBool() const {
  return true;
}

std::string BoolLiteral::to_str() const {
  return value_ ? "true" : "false";
}

std::string BoolLiteral::name() const {
  return value_ ? "__bool_true" : "__bool_false";
}

}  // namespace ast
}  // namespace tint
