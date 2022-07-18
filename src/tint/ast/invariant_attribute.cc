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

#include "src/tint/ast/invariant_attribute.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::InvariantAttribute);

namespace tint::ast {

InvariantAttribute::InvariantAttribute(ProgramID pid, NodeID nid, const Source& src)
    : Base(pid, nid, src) {}

InvariantAttribute::~InvariantAttribute() = default;

std::string InvariantAttribute::Name() const {
    return "invariant";
}

const InvariantAttribute* InvariantAttribute::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    return ctx->dst->create<InvariantAttribute>(src);
}

}  // namespace tint::ast
