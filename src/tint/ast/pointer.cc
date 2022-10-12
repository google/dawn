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

#include "src/tint/ast/pointer.h"

#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Pointer);

namespace tint::ast {

Pointer::Pointer(ProgramID pid,
                 NodeID nid,
                 const Source& src,
                 const Type* const subtype,
                 ast::AddressSpace addr_space,
                 ast::Access ac)
    : Base(pid, nid, src), type(subtype), address_space(addr_space), access(ac) {}

std::string Pointer::FriendlyName(const SymbolTable& symbols) const {
    std::ostringstream out;
    out << "ptr<";
    if (address_space != ast::AddressSpace::kNone) {
        out << address_space << ", ";
    }
    out << type->FriendlyName(symbols);
    if (access != ast::Access::kUndefined) {
        out << ", " << access;
    }
    out << ">";
    return out.str();
}

Pointer::Pointer(Pointer&&) = default;

Pointer::~Pointer() = default;

const Pointer* Pointer::Clone(CloneContext* ctx) const {
    // Clone arguments outside of create() call to have deterministic ordering
    auto src = ctx->Clone(source);
    auto* ty = ctx->Clone(type);
    return ctx->dst->create<Pointer>(src, ty, address_space, access);
}

}  // namespace tint::ast
