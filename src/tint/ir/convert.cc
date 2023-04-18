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

Convert::Convert(Value* result, const type::Type* from, utils::VectorRef<Value*> args)
    : Base(result, args), from_(from) {}

Convert::~Convert() = default;

utils::StringStream& Convert::ToString(utils::StringStream& out) const {
    Result()->ToString(out);
    out << " = convert(" << Result()->Type()->FriendlyName() << ", " << from_->FriendlyName()
        << ", ";
    EmitArgs(out);
    out << ")";
    return out;
}

}  // namespace tint::ir
