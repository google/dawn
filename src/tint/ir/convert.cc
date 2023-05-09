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

#include "src/tint/ir/convert.h"
#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Convert);

namespace tint::ir {

Convert::Convert(const type::Type* to_type,
                 const type::Type* from_type,
                 utils::VectorRef<Value*> arguments)
    : Base(to_type, arguments), from_type_(from_type) {}

Convert::~Convert() = default;

}  // namespace tint::ir
