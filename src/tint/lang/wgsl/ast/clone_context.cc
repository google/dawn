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

#include "src/tint/lang/wgsl/ast/clone_context.h"

#include <string>

#include "src/tint/lang/wgsl/ast/builder.h"
#include "src/tint/utils/containers/map.h"

namespace tint::ast {

CloneContext::CloneContext(ast::Builder* to, GenerationID from) : dst(to), src_id(from) {}

CloneContext::~CloneContext() = default;

Symbol CloneContext::Clone(Symbol s) {
    return cloned_symbols_.GetOrCreate(s, [&]() -> Symbol {
        if (symbol_transform_) {
            return symbol_transform_(s);
        }
        return dst->Symbols().New(s.Name());
    });
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

const ast::Node* CloneContext::CloneNode(const ast::Node* node) {
    // If the input is nullptr, there's nothing to clone - just return nullptr.
    if (node == nullptr) {
        return nullptr;
    }

    // Was Replace() called for this node?
    if (auto fn = replacements_.Find(node)) {
        return (*fn)();
    }

    // Attempt to clone using the registered replacer functions.
    auto& typeinfo = node->TypeInfo();
    for (auto& transform : transforms_) {
        if (typeinfo.Is(transform.typeinfo)) {
            if (auto* transformed = transform.function(node)) {
                return transformed;
            }
            break;
        }
    }

    // No transform for this type, or the transform returned nullptr.
    // Clone with T::Clone().
    return node->Clone(*this);
}

void CloneContext::CheckedCastFailure(const ast::Node* got, const TypeInfo& expected) {
    TINT_ICE() << "Cloned object was not of the expected type\n"
               << "got:      " << got->TypeInfo().name << "\n"
               << "expected: " << expected.name;
}

diag::List& CloneContext::Diagnostics() const {
    return dst->Diagnostics();
}

CloneContext::CloneableTransform::CloneableTransform() = default;
CloneContext::CloneableTransform::CloneableTransform(const CloneableTransform&) = default;
CloneContext::CloneableTransform::~CloneableTransform() = default;

}  // namespace tint::ast
