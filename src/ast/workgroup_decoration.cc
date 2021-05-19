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

#include "src/ast/workgroup_decoration.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::WorkgroupDecoration);

namespace tint {
namespace ast {

WorkgroupDecoration::WorkgroupDecoration(ProgramID program_id,
                                         const Source& source,
                                         ast::Expression* x,
                                         ast::Expression* y,
                                         ast::Expression* z)
    : Base(program_id, source), x_(x), y_(y), z_(z) {}

WorkgroupDecoration::~WorkgroupDecoration() = default;

void WorkgroupDecoration::to_str(const sem::Info& sem,
                                 std::ostream& out,
                                 size_t indent) const {
  make_indent(out, indent);
  out << "WorkgroupDecoration{" << std::endl;
  x_->to_str(sem, out, indent + 2);
  if (y_) {
    y_->to_str(sem, out, indent + 2);
    if (z_) {
      z_->to_str(sem, out, indent + 2);
    }
  }
  make_indent(out, indent);
  out << "}" << std::endl;
}

WorkgroupDecoration* WorkgroupDecoration::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* x = ctx->Clone(x_);
  auto* y = ctx->Clone(y_);
  auto* z = ctx->Clone(z_);
  return ctx->dst->create<WorkgroupDecoration>(src, x, y, z);
}

}  // namespace ast
}  // namespace tint
