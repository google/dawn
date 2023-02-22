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

#include "src/tint/ast/struct_member_align_attribute.h"

#include <string>

#include "src/tint/clone_context.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::StructMemberAlignAttribute);

namespace tint::ast {

StructMemberAlignAttribute::StructMemberAlignAttribute(ProgramID pid,
                                                       NodeID nid,
                                                       const Source& src,
                                                       const Expression* a)
    : Base(pid, nid, src), expr(a) {}

StructMemberAlignAttribute::~StructMemberAlignAttribute() = default;

std::string StructMemberAlignAttribute::Name() const {
    return "align";
}

const StructMemberAlignAttribute* StructMemberAlignAttribute::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* expr_ = ctx->Clone(expr);
    return ctx->dst->create<StructMemberAlignAttribute>(src, expr_);
}

}  // namespace tint::ast
