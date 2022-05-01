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

#include "src/tint/ast/variable.h"

#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Variable);

namespace tint::ast {

Variable::Variable(ProgramID pid,
                   const Source& src,
                   const Symbol& sym,
                   StorageClass dsc,
                   Access da,
                   const ast::Type* ty,
                   bool constant,
                   bool overridable,
                   const Expression* ctor,
                   AttributeList attrs)
    : Base(pid, src),
      symbol(sym),
      type(ty),
      is_const(constant),
      is_overridable(overridable),
      constructor(ctor),
      attributes(std::move(attrs)),
      declared_storage_class(dsc),
      declared_access(da) {
    TINT_ASSERT(AST, symbol.IsValid());
    TINT_ASSERT(AST, is_overridable ? is_const : true);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, symbol, program_id);
    TINT_ASSERT_PROGRAM_IDS_EQUAL_IF_VALID(AST, constructor, program_id);
}

Variable::Variable(Variable&&) = default;

Variable::~Variable() = default;

VariableBindingPoint Variable::BindingPoint() const {
    const GroupAttribute* group = nullptr;
    const BindingAttribute* binding = nullptr;
    for (auto* attr : attributes) {
        if (auto* g = attr->As<GroupAttribute>()) {
            group = g;
        } else if (auto* b = attr->As<BindingAttribute>()) {
            binding = b;
        }
    }
    return VariableBindingPoint{group, binding};
}

const Variable* Variable::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    auto sym = ctx->Clone(symbol);
    auto* ty = ctx->Clone(type);
    auto* ctor = ctx->Clone(constructor);
    auto attrs = ctx->Clone(attributes);
    return ctx->dst->create<Variable>(src, sym, declared_storage_class, declared_access, ty,
                                      is_const, is_overridable, ctor, attrs);
}

}  // namespace tint::ast
