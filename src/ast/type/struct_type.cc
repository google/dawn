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

#include "src/ast/type/struct_type.h"

#include <cmath>
#include <utility>

#include "src/ast/clone_context.h"
#include "src/ast/module.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/vector_type.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::type::Struct);

namespace tint {
namespace ast {
namespace type {

Struct::Struct(const Symbol& sym, ast::Struct* impl)
    : symbol_(sym), struct_(impl) {}

Struct::Struct(Struct&&) = default;

Struct::~Struct() = default;

std::string Struct::type_name() const {
  return "__struct_" + symbol_.to_str();
}

uint64_t Struct::MinBufferBindingSize(MemoryLayout mem_layout) const {
  if (!struct_->members().size()) {
    return 0;
  }

  auto* last_member = struct_->members().back();

  // If there is no offset, then this is not a host-shareable struct, returning
  // 0 indicates this to the caller.
  if (!last_member->has_offset_decoration()) {
    return 0;
  }

  uint64_t size = last_member->type()->MinBufferBindingSize(mem_layout);
  if (!size) {
    return 0;
  }

  float unaligned = static_cast<float>(last_member->offset() + size);
  float alignment = static_cast<float>(BaseAlignment(mem_layout));

  return static_cast<uint64_t>(alignment * std::ceil(unaligned / alignment));
}

uint64_t Struct::BaseAlignment(MemoryLayout mem_layout) const {
  uint64_t max = 0;
  for (auto* member : struct_->members()) {
    if (member->type()->BaseAlignment(mem_layout) > max) {
      max = member->type()->BaseAlignment(mem_layout);
    }
  }

  if (mem_layout == MemoryLayout::kUniformBuffer) {
    // Round up to a vec4.
    return static_cast<uint64_t>(16 *
                                 std::ceil(static_cast<float>(max) / 16.0f));
  } else if (mem_layout == MemoryLayout::kStorageBuffer) {
    return max;
  }
  return 0;
}

Struct* Struct::Clone(CloneContext* ctx) const {
  return ctx->mod->create<Struct>(ctx->Clone(symbol()), ctx->Clone(struct_));
}

}  // namespace type
}  // namespace ast
}  // namespace tint
