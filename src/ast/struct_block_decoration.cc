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

#include "src/ast/struct_block_decoration.h"

#include "src/ast/module.h"
#include "src/clone_context.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::StructBlockDecoration);

namespace tint {
namespace ast {

StructBlockDecoration::StructBlockDecoration(const Source& source)
    : Base(source) {}

StructBlockDecoration::~StructBlockDecoration() = default;

void StructBlockDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "block";
}

StructBlockDecoration* StructBlockDecoration::Clone(CloneContext* ctx) const {
  return ctx->dst->create<StructBlockDecoration>(ctx->Clone(source()));
}

}  // namespace ast
}  // namespace tint
