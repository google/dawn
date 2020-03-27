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

#include "src/ast/uint_literal.h"

namespace tint {
namespace ast {

UintLiteral::UintLiteral(ast::type::Type* type, uint32_t value)
    : Literal(type), value_(value) {}

UintLiteral::~UintLiteral() = default;

std::string UintLiteral::to_str() const {
  return std::to_string(value_);
}

std::string UintLiteral::name() const {
  return "__uint" + std::to_string(value_);
}

}  // namespace ast
}  // namespace tint
