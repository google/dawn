// Copyright 2021 The Tint Authors.
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

#include "src/ast/struct_member_size_decoration.h"

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::StructMemberSizeDecoration);

namespace tint {
namespace ast {

StructMemberSizeDecoration::StructMemberSizeDecoration(ProgramID program_id,
                                                       const Source& source,
                                                       uint32_t size)
    : Base(program_id, source), size_(size) {}

StructMemberSizeDecoration::~StructMemberSizeDecoration() = default;

void StructMemberSizeDecoration::to_str(const sem::Info&,
                                        std::ostream& out,
                                        size_t indent) const {
  make_indent(out, indent);
  out << "size " << std::to_string(size_);
}

StructMemberSizeDecoration* StructMemberSizeDecoration::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  return ctx->dst->create<StructMemberSizeDecoration>(src, size_);
}

}  // namespace ast
}  // namespace tint
