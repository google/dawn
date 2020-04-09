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

#include "src/ast/float_literal.h"

#include <limits>
#include <sstream>

namespace tint {
namespace ast {

FloatLiteral::FloatLiteral(ast::type::Type* type, float value)
    : Literal(type), value_(value) {}

FloatLiteral::~FloatLiteral() = default;

bool FloatLiteral::IsFloat() const {
  return true;
}

std::string FloatLiteral::to_str() const {
  return std::to_string(value_);
}

std::string FloatLiteral::name() const {
  std::ostringstream out;
  out.flags(out.flags() | std::ios_base::showpoint);
  out.precision(std::numeric_limits<float>::max_digits10);
  out << "__float" << value_;
  return out.str();
}

}  // namespace ast
}  // namespace tint
