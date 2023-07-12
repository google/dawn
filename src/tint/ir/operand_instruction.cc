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

#include "src/tint/ir/operand_instruction.h"

using Op10 = tint::ir::OperandInstruction<1, 0>;
using Op11 = tint::ir::OperandInstruction<1, 1>;
using Op20 = tint::ir::OperandInstruction<2, 0>;
using Op30 = tint::ir::OperandInstruction<3, 0>;
using Op21 = tint::ir::OperandInstruction<2, 1>;
using Op31 = tint::ir::OperandInstruction<3, 1>;
using Op41 = tint::ir::OperandInstruction<4, 1>;

TINT_INSTANTIATE_TYPEINFO(Op10);
TINT_INSTANTIATE_TYPEINFO(Op11);
TINT_INSTANTIATE_TYPEINFO(Op20);
TINT_INSTANTIATE_TYPEINFO(Op30);
TINT_INSTANTIATE_TYPEINFO(Op21);
TINT_INSTANTIATE_TYPEINFO(Op31);
TINT_INSTANTIATE_TYPEINFO(Op41);

namespace tint::ir {}  // namespace tint::ir
