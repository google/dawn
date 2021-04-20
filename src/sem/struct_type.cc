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

#include "src/sem/struct_type.h"

#include <cmath>

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::StructType);

namespace tint {
namespace sem {

StructType::StructType(ast::Struct* impl) : struct_(impl) {}

StructType::StructType(StructType&&) = default;

StructType::~StructType() = default;

std::string StructType::type_name() const {
  return impl()->type_name();
}

std::string StructType::FriendlyName(const SymbolTable& symbols) const {
  return impl()->FriendlyName(symbols);
}

StructType* StructType::Clone(CloneContext* ctx) const {
  // Clone arguments outside of create() call to have deterministic ordering
  auto* str = ctx->Clone(impl());
  return ctx->dst->create<StructType>(str);
}

}  // namespace sem
}  // namespace tint
