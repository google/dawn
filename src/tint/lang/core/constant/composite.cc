// Copyright 2022 The Tint Authors.
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

#include "src/tint/lang/core/constant/composite.h"

#include <utility>

#include "src/tint/lang/core/constant/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::constant::Composite);

namespace tint::constant {

Composite::Composite(const type::Type* t,
                     VectorRef<const constant::Value*> els,
                     bool all_0,
                     bool any_0)
    : type(t), elements(std::move(els)), all_zero(all_0), any_zero(any_0), hash(CalcHash()) {
    TINT_ASSERT(!elements.IsEmpty());
}

Composite::~Composite() = default;

const Composite* Composite::Clone(CloneContext& ctx) const {
    auto* ty = type->Clone(ctx.type_ctx);
    Vector<const constant::Value*, 4> els;
    for (const auto* el : elements) {
        els.Push(el->Clone(ctx));
    }
    return ctx.dst.Get<Composite>(ty, std::move(els), all_zero, any_zero);
}

}  // namespace tint::constant
