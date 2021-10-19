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

#include <string>

#include "src/clone_context.h"
#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::StructMemberSizeDecoration);

namespace tint {
namespace ast {

StructMemberSizeDecoration::StructMemberSizeDecoration(ProgramID pid,
                                                       const Source& src,
                                                       uint32_t sz)
    : Base(pid, src), size(sz) {}

StructMemberSizeDecoration::~StructMemberSizeDecoration() = default;

std::string StructMemberSizeDecoration::Name() const {
  return "size";
}

const StructMemberSizeDecoration* StructMemberSizeDecoration::Clone(
    CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source);
  return ctx->dst->create<StructMemberSizeDecoration>(src, size);
}

}  // namespace ast
}  // namespace tint
