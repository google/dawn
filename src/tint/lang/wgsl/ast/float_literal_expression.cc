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

#include "src/tint/lang/wgsl/ast/float_literal_expression.h"

#include <limits>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/lang/wgsl/ast/clone_context.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::FloatLiteralExpression);

namespace tint::ast {

FloatLiteralExpression::FloatLiteralExpression(GenerationID pid,
                                               NodeID nid,
                                               const Source& src,
                                               double val,
                                               Suffix suf)
    : Base(pid, nid, src), value(val), suffix(suf) {}

FloatLiteralExpression::~FloatLiteralExpression() = default;

const FloatLiteralExpression* FloatLiteralExpression::Clone(CloneContext& ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx.Clone(source);
    return ctx.dst->create<FloatLiteralExpression>(src, value, suffix);
}

std::string_view ToString(FloatLiteralExpression::Suffix suffix) {
    switch (suffix) {
        default:
            return "";
        case FloatLiteralExpression::Suffix::kF:
            return "f";
        case FloatLiteralExpression::Suffix::kH:
            return "h";
    }
}

}  // namespace tint::ast
