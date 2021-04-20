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

#include "src/ast/i32.h"

#include "src/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::I32);

namespace tint {
namespace ast {

I32::I32(ProgramID program_id, const Source& source)
    : Base(program_id, source) {}

I32::I32(I32&&) = default;

I32::~I32() = default;

std::string I32::type_name() const {
  return "__i32";
}

std::string I32::FriendlyName(const SymbolTable&) const {
  return "i32";
}

I32* I32::Clone(CloneContext* ctx) const {
  auto src = ctx->Clone(source());
  return ctx->dst->create<I32>(src);
}

}  // namespace ast
}  // namespace tint
