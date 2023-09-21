// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/core/ir/block_param.h"

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::BlockParam);

namespace tint::core::ir {

BlockParam::BlockParam(const core::type::Type* ty) : type_(ty) {
    TINT_ASSERT(type_ != nullptr);
}

BlockParam::~BlockParam() = default;

BlockParam* BlockParam::Clone(CloneContext& ctx) {
    auto* new_bp = ctx.ir.values.Create<BlockParam>(type_);

    auto name = ctx.ir.NameOf(this);
    if (name.IsValid()) {
        ctx.ir.SetName(new_bp, ctx.ir.NameOf(this).Name());
    }
    return new_bp;
}

}  // namespace tint::core::ir
