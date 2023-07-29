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

#include "src/tint/lang/wgsl/program/clone_context.h"

#include <string>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/utils/containers/map.h"

namespace tint::program {

CloneContext::CloneContext(ProgramBuilder* to, Program const* from, bool auto_clone_symbols)
    : dst(to), src(from), ctx_(to, from->ID()) {
    if (auto_clone_symbols) {
        // Almost all transforms will want to clone all symbols before doing any
        // work, to avoid any newly created symbols clashing with existing symbols
        // in the source program and causing them to be renamed.
        from->Symbols().Foreach([&](Symbol s) { Clone(s); });
    }
}

CloneContext::~CloneContext() = default;

void CloneContext::Clone() {
    dst->AST().Copy(ctx_, &src->AST());
}

}  // namespace tint::program
