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

#include "src/tint/ir/return.h"

#include <utility>

#include "src/tint/ir/function.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Return);

namespace tint::ir {

Return::Return(Function* func) : func_(func) {
    TINT_ASSERT_OR_RETURN(IR, func_);

    func_->AddUsage({this, 0u});
}

Return::Return(Function* func, Value* arg) : func_(func) {
    TINT_ASSERT_OR_RETURN(IR, func_);
    TINT_ASSERT_OR_RETURN(IR, arg);

    func_->AddUsage({this, 0u});

    AddOperand(arg);
}

Return::~Return() = default;

}  // namespace tint::ir
