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

#include "src/ast/override_decoration.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::OverrideDecoration);

namespace tint {
namespace ast {

OverrideDecoration::OverrideDecoration(ProgramID program_id,
                                       const Source& source)
    : Base(program_id, source), has_value_(false), value_(0) {}

OverrideDecoration::OverrideDecoration(ProgramID program_id,
                                       const Source& source,
                                       uint32_t val)
    : Base(program_id, source), has_value_(true), value_(val) {}

OverrideDecoration::~OverrideDecoration() = default;

void OverrideDecoration::to_str(const sem::Info&,
                                std::ostream& out,
                                size_t indent) const {
  make_indent(out, indent);
  out << "OverrideDecoration";
  if (has_value_) {
    out << "{" << value_ << "}";
  }
  out << std::endl;
}

OverrideDecoration* OverrideDecoration::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  if (has_value_) {
    return ctx->dst->create<OverrideDecoration>(src, value_);
  } else {
    return ctx->dst->create<OverrideDecoration>(src);
  }
}

}  // namespace ast
}  // namespace tint
