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

#include "src/clone_context.h"
#include "src/program.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::WorkgroupDecoration);

namespace tint {
namespace ast {

WorkgroupDecoration::WorkgroupDecoration(const Source& source, uint32_t x)
    : WorkgroupDecoration(source, x, 1, 1) {}

WorkgroupDecoration::WorkgroupDecoration(const Source& source,
                                         uint32_t x,
                                         uint32_t y)
    : WorkgroupDecoration(source, x, y, 1) {}

WorkgroupDecoration::WorkgroupDecoration(const Source& source,
                                         uint32_t x,
                                         uint32_t y,
                                         uint32_t z)
    : Base(source), x_(x), y_(y), z_(z) {}

WorkgroupDecoration::~WorkgroupDecoration() = default;

void WorkgroupDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "WorkgroupDecoration{" << x_ << " " << y_ << " " << z_ << "}"
      << std::endl;
}

WorkgroupDecoration* WorkgroupDecoration::Clone(CloneContext* ctx) const {
  return ctx->dst->create<WorkgroupDecoration>(ctx->Clone(source()), x_, y_,
                                               z_);
}

}  // namespace ast
}  // namespace tint
