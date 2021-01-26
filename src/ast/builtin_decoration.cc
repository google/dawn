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

#include "src/ast/builtin_decoration.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::BuiltinDecoration);

namespace tint {
namespace ast {

BuiltinDecoration::BuiltinDecoration(const Source& source, Builtin builtin)
    : Base(source), builtin_(builtin) {}

BuiltinDecoration::~BuiltinDecoration() = default;

void BuiltinDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "BuiltinDecoration{" << builtin_ << "}" << std::endl;
}

BuiltinDecoration* BuiltinDecoration::Clone(CloneContext* ctx) const {
  return ctx->dst->create<BuiltinDecoration>(ctx->Clone(source()), builtin_);
}

}  // namespace ast
}  // namespace tint
