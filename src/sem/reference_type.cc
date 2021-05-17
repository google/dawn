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

#include "src/sem/reference_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Reference);

namespace tint {
namespace sem {

Reference::Reference(const Type* subtype, ast::StorageClass storage_class)
    : subtype_(subtype), storage_class_(storage_class) {
  TINT_ASSERT(!subtype->Is<Reference>());
}

std::string Reference::type_name() const {
  std::ostringstream out;
  out << "__ref_" << storage_class_ << subtype_->type_name();
  return out.str();
}

std::string Reference::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "ref<";
  if (storage_class_ != ast::StorageClass::kNone) {
    out << storage_class_ << ", ";
  }
  out << subtype_->FriendlyName(symbols) << ">";
  return out.str();
}

Reference::Reference(Reference&&) = default;

Reference::~Reference() = default;

}  // namespace sem
}  // namespace tint
