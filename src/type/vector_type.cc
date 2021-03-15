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

#include "src/type/vector_type.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Vector);

namespace tint {
namespace type {

Vector::Vector(Type* subtype, uint32_t size) : subtype_(subtype), size_(size) {
  assert(size_ > 1);
  assert(size_ < 5);
}

Vector::Vector(Vector&&) = default;

Vector::~Vector() = default;

std::string Vector::type_name() const {
  return "__vec_" + std::to_string(size_) + subtype_->type_name();
}

std::string Vector::FriendlyName(const SymbolTable& symbols) const {
  std::ostringstream out;
  out << "vec" << size_ << "<" << subtype_->FriendlyName(symbols) << ">";
  return out.str();
}

Vector* Vector::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto* ty = ctx->Clone(type());
  return ctx->dst->create<Vector>(ty, size_);
}

}  // namespace type
}  // namespace tint
