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

#include "src/clone_context.h"

#include <string>

#include "src/program_builder.h"
#include "src/utils/get_or_create.h"

TINT_INSTANTIATE_TYPEINFO(tint::Cloneable);

namespace tint {

CloneContext::ListTransforms::ListTransforms() = default;
CloneContext::ListTransforms::~ListTransforms() = default;

CloneContext::CloneContext(ProgramBuilder* to,
                           Program const* from,
                           bool auto_clone_symbols)
    : dst(to), src(from) {
  if (auto_clone_symbols) {
    // Almost all transforms will want to clone all symbols before doing any
    // work, to avoid any newly created symbols clashing with existing symbols
    // in the source program and causing them to be renamed.
    from->Symbols().Foreach([&](Symbol s, const std::string&) { Clone(s); });
  }
}

CloneContext::CloneContext(ProgramBuilder* builder)
    : CloneContext(builder, nullptr, false) {}

CloneContext::~CloneContext() = default;

Symbol CloneContext::Clone(Symbol s) {
  if (!src) {
    return s;  // In-place clone
  }
  return utils::GetOrCreate(cloned_symbols_, s, [&]() -> Symbol {
    if (symbol_transform_) {
      return symbol_transform_(s);
    }
    return dst->Symbols().New(src->Symbols().NameFor(s));
  });
}

void CloneContext::Clone() {
  dst->AST().Copy(this, &src->AST());
}

ast::FunctionList CloneContext::Clone(const ast::FunctionList& v) {
  ast::FunctionList out;
  out.reserve(v.size());
  for (ast::Function* el : v) {
    out.Add(Clone(el));
  }
  return out;
}

diag::List& CloneContext::Diagnostics() const {
  return dst->Diagnostics();
}

CloneContext::CloneableTransform::CloneableTransform() = default;
CloneContext::CloneableTransform::CloneableTransform(
    const CloneableTransform&) = default;
CloneContext::CloneableTransform::~CloneableTransform() = default;

}  // namespace tint
