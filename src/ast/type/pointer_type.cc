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

#include "src/ast/type/pointer_type.h"

#include "src/ast/clone_context.h"
#include "src/ast/module.h"

namespace tint {
namespace ast {
namespace type {

Pointer::Pointer(Type* subtype, StorageClass storage_class)
    : subtype_(subtype), storage_class_(storage_class) {}

std::string Pointer::type_name() const {
  std::ostringstream out;
  out << "__ptr_" << storage_class_ << subtype_->type_name();
  return out.str();
}

Pointer::Pointer(Pointer&&) = default;

Pointer::~Pointer() = default;

Pointer* Pointer::Clone(CloneContext* ctx) const {
  return ctx->mod->create<Pointer>(ctx->Clone(subtype_), storage_class_);
}

}  // namespace type
}  // namespace ast
}  // namespace tint
