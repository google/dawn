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

#include "src/tint/ast/variable.h"
#include "src/tint/ast/binding_attribute.h"
#include "src/tint/ast/group_attribute.h"
#include "src/tint/ast/templated_identifier.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Variable);

namespace tint::ast {

Variable::Variable(ProgramID pid,
                   NodeID nid,
                   const Source& src,
                   const Identifier* n,
                   Type ty,
                   const Expression* init,
                   utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src), name(n), type(ty), initializer(init), attributes(std::move(attrs)) {
    TINT_ASSERT(AST, name);
    if (name) {
        TINT_ASSERT(AST, !name->Is<TemplatedIdentifier>());
    }
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, initializer, program_id);
}

Variable::~Variable() = default;

}  // namespace tint::ast
