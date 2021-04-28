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

#include "src/ast/literal.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Literal);

namespace tint {
namespace ast {

Literal::Literal(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

Literal::~Literal() = default;

void Literal::to_str(const sem::Info& sem,
                     std::ostream& out,
                     size_t indent) const {
  make_indent(out, indent);
  out << to_str(sem);
}

}  // namespace ast
}  // namespace tint
