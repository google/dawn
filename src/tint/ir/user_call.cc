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

#include "src/tint/ir/user_call.h"

#include <utility>

#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::UserCall);

namespace tint::ir {

UserCall::UserCall(const type::Type* ty, Symbol n, utils::VectorRef<Value*> arguments)
    : Base(ty, std::move(arguments)), name(n) {}

UserCall::~UserCall() = default;

}  // namespace tint::ir
