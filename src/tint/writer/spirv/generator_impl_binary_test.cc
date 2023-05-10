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

#include "src/tint/writer/spirv/test_helper_ir.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Binary_Add_I32) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("foo");
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    func->start_target->instructions.Push(CreateBinary(
        ir::Binary::Kind::kAdd, ir.types.Get<type::I32>(), Constant(1_i), Constant(2_i)));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_U32) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("foo");
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    func->start_target->instructions.Push(CreateBinary(
        ir::Binary::Kind::kAdd, ir.types.Get<type::U32>(), Constant(1_u), Constant(2_u)));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_F32) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("foo");
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    func->start_target->instructions.Push(CreateBinary(
        ir::Binary::Kind::kAdd, ir.types.Get<type::F32>(), Constant(1_f), Constant(2_f)));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_Chain) {
    auto* func = CreateFunction();
    func->name = ir.symbols.Register("foo");
    func->return_type = ir.types.Get<type::Void>();
    func->start_target->branch.target = func->end_target;

    auto* a = CreateBinary(ir::Binary::Kind::kAdd, ir.types.Get<type::I32>(), Constant(1_i),
                           Constant(2_i));
    func->start_target->instructions.Push(a);
    func->start_target->instructions.Push(
        CreateBinary(ir::Binary::Kind::kAdd, ir.types.Get<type::I32>(), a, a));

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
%9 = OpIAdd %6 %5 %5
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
