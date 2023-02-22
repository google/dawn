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

#include "src/tint/ast/diagnostic_attribute.h"

#include <string>
#include <utility>

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DiagnosticAttribute);

namespace tint::ast {

DiagnosticAttribute::DiagnosticAttribute(ProgramID pid,
                                         NodeID nid,
                                         const Source& src,
                                         DiagnosticControl&& dc)
    : Base(pid, nid, src), control(std::move(dc)) {}

DiagnosticAttribute::~DiagnosticAttribute() = default;

std::string DiagnosticAttribute::Name() const {
    return "diagnostic";
}

const DiagnosticAttribute* DiagnosticAttribute::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto rule = ctx->Clone(control.rule_name);
    DiagnosticControl dc(control.severity, rule);
    return ctx->dst->create<DiagnosticAttribute>(src, std::move(dc));
}

}  // namespace tint::ast
