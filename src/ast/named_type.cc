// Copyright 2021 The Tint Authors.
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

#include "src/ast/named_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::NamedType);

namespace tint {
namespace ast {

NamedType::NamedType(ProgramID program_id, const Source& source, Symbol name)
    : Base(program_id, source), name_(name) {
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(name, program_id);
}

NamedType::NamedType(NamedType&&) = default;

NamedType::~NamedType() = default;

std::string NamedType::FriendlyName(const SymbolTable& symbols) const {
  return symbols.NameFor(name());
}

}  // namespace ast
}  // namespace tint
