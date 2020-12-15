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

#include "src/ast/type/alias_type.h"

#include <assert.h>

#include "src/ast/clone_context.h"
#include "src/ast/module.h"

TINT_INSTANTIATE_CLASS_ID(tint::ast::type::Alias);

namespace tint {
namespace ast {
namespace type {

Alias::Alias(const Symbol& sym, const std::string& name, Type* subtype)
    : symbol_(sym), name_(name), subtype_(subtype) {
  assert(subtype_);
}

Alias::Alias(Alias&&) = default;

Alias::~Alias() = default;

std::string Alias::type_name() const {
  return "__alias_" + symbol_.to_str() + subtype_->type_name();
}

uint64_t Alias::MinBufferBindingSize(MemoryLayout mem_layout) const {
  return subtype_->MinBufferBindingSize(mem_layout);
}

uint64_t Alias::BaseAlignment(MemoryLayout mem_layout) const {
  return subtype_->BaseAlignment(mem_layout);
}

Alias* Alias::Clone(CloneContext* ctx) const {
  return ctx->mod->create<Alias>(ctx->Clone(symbol()), name_,
                                 ctx->Clone(subtype_));
}

}  // namespace type
}  // namespace ast
}  // namespace tint
