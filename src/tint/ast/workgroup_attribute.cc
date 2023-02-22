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

#include "src/tint/ast/workgroup_attribute.h"

#include <string>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::WorkgroupAttribute);

namespace tint::ast {

WorkgroupAttribute::WorkgroupAttribute(ProgramID pid,
                                       NodeID nid,
                                       const Source& src,
                                       const Expression* x_,
                                       const Expression* y_,
                                       const Expression* z_)
    : Base(pid, nid, src), x(x_), y(y_), z(z_) {}

WorkgroupAttribute::~WorkgroupAttribute() = default;

std::string WorkgroupAttribute::Name() const {
    return "workgroup_size";
}

const WorkgroupAttribute* WorkgroupAttribute::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* x_ = ctx->Clone(x);
    auto* y_ = ctx->Clone(y);
    auto* z_ = ctx->Clone(z);
    return ctx->dst->create<WorkgroupAttribute>(src, x_, y_, z_);
}

}  // namespace tint::ast
