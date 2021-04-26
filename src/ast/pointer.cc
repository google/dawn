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

#include "src/ast/pointer.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Pointer);

namespace tint {
namespace ast {

Pointer::Pointer(ProgramID program_id,
                 const Source& source,
                 Type* const subtype,
                 ast::StorageClass storage_class)
    : Base(program_id, source),
      subtype_(subtype),
      storage_class_(storage_class) {}

std::string Pointer::type_name() const {
  std::ostringstream out;
  out << "__ptr_" << storage_class_ << subtype_->type_name();
  return out.str();
}

std::string Pointer::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "ptr<";
  if (storage_class_ != ast::StorageClass::kNone) {
    out << storage_class_ << ", ";
  }
  out << subtype_->FriendlyName(symbols) << ">";
  return out.str();
}

Pointer::Pointer(Pointer&&) = default;

Pointer::~Pointer() = default;

Pointer* Pointer::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto src = ctx->Clone(source());
  auto* ty = ctx->Clone(type());
  return ctx->dst->create<Pointer>(src, ty, storage_class_);
}

}  // namespace ast
}  // namespace tint
