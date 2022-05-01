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

#include "src/tint/ast/uint_literal_expression.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::UintLiteralExpression);

namespace tint::ast {

UintLiteralExpression::UintLiteralExpression(ProgramID pid, const Source& src, uint32_t val)
    : Base(pid, src), value(val) {}

UintLiteralExpression::~UintLiteralExpression() = default;

uint32_t UintLiteralExpression::ValueAsU32() const {
    return value;
}

const UintLiteralExpression* UintLiteralExpression::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    return ctx->dst->create<UintLiteralExpression>(src, value);
}

}  // namespace tint::ast
