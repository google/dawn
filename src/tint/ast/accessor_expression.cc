// Copyright 2023 The Tint Authors.
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

#include "src/tint/ast/accessor_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::AccessorExpression);

namespace tint::ast {

AccessorExpression::AccessorExpression(ProgramID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Expression* obj)
    : Base(pid, nid, src), object(obj) {
    TINT_ASSERT(AST, object);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, object, program_id);
}

AccessorExpression::~AccessorExpression() = default;

}  // namespace tint::ast
