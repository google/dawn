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

#include "src/tint/ast/vector.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Vector);

namespace tint::ast {

Vector::Vector(ProgramID pid, Source const& src, const Type* subtype, uint32_t w)
    : Base(pid, src), type(subtype), width(w) {
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, subtype, program_id);
    TINT_ASSERT(AST, width > 1);
    TINT_ASSERT(AST, width < 5);
}

Vector::Vector(Vector&&) = default;

Vector::~Vector() = default;

std::string Vector::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    out << "vec" << width;
    if (type) {
        out << "<" << type->FriendlyName(symbols) << ">";
    }
    return out.str();
}

const Vector* Vector::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* ty = ctx->Clone(type);
    return ctx->dst->create<Vector>(src, ty, width);
}

}  // namespace tint::ast
