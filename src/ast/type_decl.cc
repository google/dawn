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

#include "src/ast/type_decl.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::TypeDecl);

namespace tint {
namespace ast {

TypeDecl::TypeDecl(ProgramID program_id, const Source& source, Symbol name)
    : Base(program_id, source), name_(name) {
  TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, name, program_id);
}

TypeDecl::TypeDecl(TypeDecl&&) = default;

TypeDecl::~TypeDecl() = default;

void TypeDecl::to_str(const sem::Info&,
                      std::ostream& out,
                      size_t indent) const {
  make_indent(out, indent);
  out << type_name();
}

}  // namespace ast
}  // namespace tint
