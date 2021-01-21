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

#include "src/type/access_control_type.h"

#include <assert.h>

#include "src/ast/module.h"
#include "src/clone_context.h"

TINT_INSTANTIATE_CLASS_ID(tint::type::AccessControl);

namespace tint {
namespace type {

AccessControl::AccessControl(ast::AccessControl access, Type* subtype)
    : access_(access), subtype_(subtype) {
  assert(subtype_);
  assert(!subtype_->Is<AccessControl>());
}

AccessControl::AccessControl(AccessControl&&) = default;

AccessControl::~AccessControl() = default;

std::string AccessControl::type_name() const {
  std::string name = "__access_control_";
  switch (access_) {
    case ast::AccessControl::kReadOnly:
      name += "read_only";
      break;
    case ast::AccessControl::kWriteOnly:
      name += "write_only";
      break;
    case ast::AccessControl::kReadWrite:
      name += "read_write";
      break;
  }
  return name + subtype_->type_name();
}

uint64_t AccessControl::MinBufferBindingSize(MemoryLayout mem_layout) const {
  return subtype_->MinBufferBindingSize(mem_layout);
}

uint64_t AccessControl::BaseAlignment(MemoryLayout mem_layout) const {
  return subtype_->BaseAlignment(mem_layout);
}

AccessControl* AccessControl::Clone(CloneContext* ctx) const {
  return ctx->mod->create<AccessControl>(access_, ctx->Clone(subtype_));
}

}  // namespace type
}  // namespace tint
