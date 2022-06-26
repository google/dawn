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

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Parameter);

namespace tint::ast {

Parameter::Parameter(ProgramID pid,
                     const Source& src,
                     const Symbol& sym,
                     const ast::Type* ty,
                     AttributeList attrs)
    : Base(pid, src, sym, ty, nullptr, attrs) {}

Parameter::Parameter(Parameter&&) = default;

Parameter::~Parameter() = default;

const char* Parameter::Kind() const {
    return "parameter";
}

const Parameter* Parameter::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    auto* ty = ctx->Clone(type);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Parameter>(src, sym, ty, attrs);
}

}  // namespace tint::ast
