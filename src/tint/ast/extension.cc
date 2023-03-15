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

#include "src/tint/ast/extension.h"

#include "src/tint/program_builder.h"

//! @cond Doxygen_Suppress
// Doxygen gets confused with tint::ast::Extension and tint::builtin::Extension

TINT_INSTANTIATE_TYPEINFO(tint::ast::Extension);

namespace tint::ast {

Extension::Extension(ProgramID pid, NodeID nid, const Source& src, builtin::Extension ext)
    : Base(pid, nid, src), name(ext) {}

Extension::~Extension() = default;

const Extension* Extension::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    return ctx->dst->create<Extension>(src, name);
}

}  // namespace tint::ast

//! @endcond
