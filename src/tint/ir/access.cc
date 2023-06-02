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

#include "src/tint/ir/access.h"

#include <utility>

#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Access);

namespace tint::ir {

//! @cond Doxygen_Suppress
Access::Access(const type::Type* ty, Value* object, utils::VectorRef<Value*> indices)
    : result_type_(ty), object_(object), indices_(std::move(indices)) {
    object_->AddUsage(this);
}

Access::~Access() = default;
//! @endcond

}  // namespace tint::ir
