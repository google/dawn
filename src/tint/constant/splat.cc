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

#include "src/tint/constant/splat.h"

#include "src/tint/constant/manager.h"

TINT_INSTANTIATE_TYPEINFO(tint::constant::Splat);

namespace tint::constant {

Splat::Splat(const type::Type* t, const constant::Value* e, size_t n) : type(t), el(e), count(n) {}

Splat::~Splat() = default;

const Splat* Splat::Clone(CloneContext& ctx) const {
    auto* ty = type->Clone(ctx.type_ctx);
    auto* element = el->Clone(ctx);
    return ctx.dst.Splat(ty, element, count);
}

}  // namespace tint::constant
