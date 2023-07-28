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

#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"

#include "src/tint/lang/wgsl/program/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::VariableDeclStatement);

namespace tint::ast {

VariableDeclStatement::VariableDeclStatement(GenerationID pid,
                                             NodeID nid,
                                             const Source& src,
                                             const Variable* var)
    : Base(pid, nid, src), variable(var) {
    TINT_ASSERT(variable);
    TINT_ASSERT_GENERATION_IDS_EQUAL_IF_VALID(variable, generation_id);
}

VariableDeclStatement::~VariableDeclStatement() = default;

const VariableDeclStatement* VariableDeclStatement::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* var = ctx->Clone(variable);
    return ctx->dst->create<VariableDeclStatement>(src, var);
}

}  // namespace tint::ast
