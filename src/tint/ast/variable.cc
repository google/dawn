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

TINT_INSTANTIATE_TYPEINFO(tint::ast::Variable);

namespace tint::ast {

Variable::Variable(ProgramID pid,
                   NodeID nid,
                   const Source& src,
                   const Symbol& sym,
                   const ast::Type* ty,
                   const Expression* ctor,
                   utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src), symbol(sym), type(ty), constructor(ctor), attributes(std::move(attrs)) {
    TINT_ASSERT(AST, symbol.IsValid());
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, symbol, program_id);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, constructor, program_id);
}

Variable::Variable(Variable&&) = default;

Variable::~Variable() = default;

}  // namespace tint::ast
