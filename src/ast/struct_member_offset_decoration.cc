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

#include "src/ast/struct_member_offset_decoration.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::StructMemberOffsetDecoration);

namespace tint {
namespace ast {

StructMemberOffsetDecoration::StructMemberOffsetDecoration(const Source& source,
                                                           uint32_t offset)
    : Base(source), offset_(offset) {}

StructMemberOffsetDecoration::~StructMemberOffsetDecoration() = default;

void StructMemberOffsetDecoration::to_str(const semantic::Info&,
                                          std::ostream& out,
                                          size_t indent) const {
  make_indent(out, indent);
  out << "offset " << std::to_string(offset_);
}

StructMemberOffsetDecoration* StructMemberOffsetDecoration::Clone(
    CloneContext* ctx) const {
  return ctx->dst->create<StructMemberOffsetDecoration>(ctx->Clone(source()),
                                                        offset_);
}

}  // namespace ast
}  // namespace tint
