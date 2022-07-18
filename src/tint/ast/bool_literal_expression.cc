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

#include "src/tint/ast/bool_literal_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::BoolLiteralExpression);

namespace tint::ast {

BoolLiteralExpression::BoolLiteralExpression(ProgramID pid, NodeID nid, const Source& src, bool val)
    : Base(pid, nid, src), value(val) {}

BoolLiteralExpression::~BoolLiteralExpression() = default;

const BoolLiteralExpression* BoolLiteralExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    return ctx->dst->create<BoolLiteralExpression>(src, value);
}

}  // namespace tint::ast
