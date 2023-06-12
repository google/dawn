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

#include "src/tint/ir/load.h"

#include "src/tint/debug.h"
#include "src/tint/type/pointer.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Load);

namespace tint::ir {

Load::Load(Value* from) {
    TINT_ASSERT_OR_RETURN(IR, from);
    TINT_ASSERT_OR_RETURN(IR, tint::Is<type::Pointer>(from->Type()));

    result_type_ = from->Type()->UnwrapPtr();
    AddOperand(from);
}

Load::~Load() = default;

}  // namespace tint::ir
