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

#include "src/ast/sint_literal.h"

namespace tint {
namespace ast {

SintLiteral::SintLiteral(ast::type::Type* type, int32_t value)
    : IntLiteral(type), value_(value) {}

SintLiteral::~SintLiteral() = default;

bool SintLiteral::IsSint() const {
  return true;
}

std::string SintLiteral::to_str() const {
  return std::to_string(value_);
}

std::string SintLiteral::name() const {
  return "__sint" + type()->type_name() + "_" + std::to_string(value_);
}

}  // namespace ast
}  // namespace tint
