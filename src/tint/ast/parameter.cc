// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/parameter.h"

#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Parameter);

namespace tint::ast {

Parameter::Parameter(ProgramID pid,
                     NodeID nid,
                     const Source& src,
                     const Identifier* n,
                     Type ty,
                     utils::VectorRef<const Attribute*> attrs)
    : Base(pid, nid, src, n, ty, nullptr, std::move(attrs)) {}

Parameter::~Parameter() = default;

const char* Parameter::Kind() const {
    return "parameter";
}

const Parameter* Parameter::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto* n = ctx->Clone(name);
    auto ty = ctx->Clone(type);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Parameter>(src, n, ty, std::move(attrs));
}

}  // namespace tint::ast
