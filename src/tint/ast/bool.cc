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

#include "src/tint/ast/bool.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Bool);

namespace tint::ast {

Bool::Bool(ProgramID pid, const Source& src) : Base(pid, src) {}

Bool::Bool(Bool&&) = default;

Bool::~Bool() = default;

std::string Bool::FriendlyName(const SymbolTable&) const {
    return "bool";
}

const Bool* Bool::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    return ctx->dst->create<Bool>(src);
}

}  // namespace tint::ast
