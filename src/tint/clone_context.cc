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

#include "src/tint/clone_context.h"

#include <string>

#include "src/tint/program_builder.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::Cloneable);

namespace tint {

Cloneable::Cloneable() = default;
Cloneable::Cloneable(Cloneable&&) = default;
Cloneable::~Cloneable() = default;

CloneContext::CloneContext(ProgramBuilder* to, Program const* from, bool auto_clone_symbols)
    : dst(to), src(from) {
    if (auto_clone_symbols) {
        // Almost all transforms will want to clone all symbols before doing any
        // work, to avoid any newly created symbols clashing with existing symbols
        // in the source program and causing them to be renamed.
        from->Symbols().Foreach([&](Symbol s) { Clone(s); });
    }
}

CloneContext::CloneContext(ProgramBuilder* builder) : CloneContext(builder, nullptr, false) {}

CloneContext::~CloneContext() = default;

Symbol CloneContext::Clone(Symbol s) {
    if (!src) {
        return s;  // In-place clone
    }
    return cloned_symbols_.GetOrCreate(s, [&]() -> Symbol {
        if (symbol_transform_) {
            return symbol_transform_(s);
        }
        return dst->Symbols().New(s.Name());
    });
}

void CloneContext::Clone() {
    dst->AST().Copy(this, &src->AST());
}

ast::FunctionList CloneContext::Clone(const ast::FunctionList& v) {
    ast::FunctionList out;
    out.Reserve(v.Length());
    for (const ast::Function* el : v) {
        out.Add(Clone(el));
    }
    return out;
}

ast::Type CloneContext::Clone(const ast::Type& ty) {
    return {Clone(ty.expr)};
}

const tint::Cloneable* CloneContext::CloneCloneable(const Cloneable* object) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (object == nullptr) {
        return nullptr;
    }

    // Was Replace() called for this object?
    if (auto fn = replacements_.Find(object)) {
        return (*fn)();
    }

    // Attempt to clone using the registered replacer functions.
    auto& typeinfo = object->TypeInfo();
    for (auto& transform : transforms_) {
        if (typeinfo.Is(transform.typeinfo)) {
            if (auto* transformed = transform.function(object)) {
                return transformed;
            }
            break;
        }
    }

    // No transform for this type, or the transform returned nullptr.
    // Clone with T::Clone().
    return object->Clone(this);
}

void CloneContext::CheckedCastFailure(const Cloneable* got, const utils::TypeInfo& expected) {
    TINT_ICE(Clone, Diagnostics()) << "Cloned object was not of the expected type\n"
                                   << "got:      " << got->TypeInfo().name << "\n"
                                   << "expected: " << expected.name;
}

diag::List& CloneContext::Diagnostics() const {
    return dst->Diagnostics();
}

CloneContext::CloneableTransform::CloneableTransform() = default;
CloneContext::CloneableTransform::CloneableTransform(const CloneableTransform&) = default;
CloneContext::CloneableTransform::~CloneableTransform() = default;

}  // namespace tint
