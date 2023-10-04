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

#include "src/tint/lang/spirv/ir/literal_operand.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::ir::LiteralOperand);

namespace tint::spirv::ir {

LiteralOperand::LiteralOperand(const core::constant::Value* value) : Base(value) {}

LiteralOperand::~LiteralOperand() = default;

}  // namespace tint::spirv::ir
