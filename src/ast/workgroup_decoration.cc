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

namespace tint {
namespace ast {

WorkgroupDecoration::WorkgroupDecoration(uint32_t x, const Source& source)
    : Base(source), x_(x) {}

WorkgroupDecoration::WorkgroupDecoration(uint32_t x,
                                         uint32_t y,
                                         const Source& source)
    : Base(source), x_(x), y_(y) {}

WorkgroupDecoration::WorkgroupDecoration(uint32_t x,
                                         uint32_t y,
                                         uint32_t z,
                                         const Source& source)
    : Base(source), x_(x), y_(y), z_(z) {}

WorkgroupDecoration::~WorkgroupDecoration() = default;

void WorkgroupDecoration::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "WorkgroupDecoration{" << x_ << " " << y_ << " " << z_ << "}"
      << std::endl;
}

}  // namespace ast
}  // namespace tint
