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

#include "src/sem/atomic_type.h"

#include "src/program_builder.h"
#include "src/sem/reference_type.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::Atomic);

namespace tint {
namespace sem {

Atomic::Atomic(const sem::Type* subtype) : subtype_(subtype) {
  TINT_ASSERT(AST, !subtype->Is<Reference>());
}

std::string Atomic::type_name() const {
  std::ostringstream out;
  out << "__atomic" << subtype_->type_name();
  return out.str();
}

std::string Atomic::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "atomic<" << subtype_->FriendlyName(symbols) << ">";
  return out.str();
}

uint32_t Atomic::Size() const {
  return subtype_->Size();
}

uint32_t Atomic::Align() const {
  return subtype_->Align();
}

bool Atomic::IsConstructible() const {
  return false;
}

Atomic::Atomic(Atomic&&) = default;

Atomic::~Atomic() = default;

}  // namespace sem
}  // namespace tint
