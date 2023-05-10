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

#include "src/tint/ir/test_helper.h"

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Add) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Add(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = add %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundAdd) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kAdd);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = add %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Subtract) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Sub(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = sub %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundSubtract) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kSubtract);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = sub %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Multiply) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mul(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = mul %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundMultiply) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kMultiply);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = mul %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Div) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Div(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = div %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundDiv) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kDivide);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = div %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Modulo) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mod(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = mod %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundModulo) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kModulo);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = mod %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_And) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = And(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = and %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundAnd) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, ast::BinaryOp::kAnd);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, bool, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, bool, read_write> = and %v1:ref<private, bool, read_write>, false
  store %v1:ref<private, bool, read_write>, %2:ref<private, bool, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Or) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Or(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = or %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundOr) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, ast::BinaryOp::kOr);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, bool, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, bool, read_write> = or %v1:ref<private, bool, read_write>, false
  store %v1:ref<private, bool, read_write>, %2:ref<private, bool, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Xor) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Xor(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = xor %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundXor) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kXor);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = xor %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalAnd) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = LogicalAnd(Call("my_func"), false);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = func my_func():bool
  %fn2 = block
  ret true
func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn4 = block
  %1:bool = call my_func
  %tint_symbol:bool = var function, read_write
  store %tint_symbol:bool, %1:bool
  branch %fn5

  %fn5 = if %1:bool [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    %fn6 = block
    store %tint_symbol:bool, false
    branch %fn8

  # if merge
  %fn8 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalOr) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = LogicalOr(Call("my_func"), true);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = func my_func():bool
  %fn2 = block
  ret true
func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn4 = block
  %1:bool = call my_func
  %tint_symbol:bool = var function, read_write
  store %tint_symbol:bool, %1:bool
  branch %fn5

  %fn5 = if %1:bool [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    # false branch
    %fn7 = block
    store %tint_symbol:bool, true
    branch %fn8

  # if merge
  %fn8 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Equal) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Equal(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = eq %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_NotEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = NotEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = neq %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = lt %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = gt %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = lte %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:bool = gte %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftLeft) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shl(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = shiftl %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundShiftLeft) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kShiftLeft);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = shiftl %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftRight) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shr(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = shiftr %1:u32, 4u
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundShiftRight) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kShiftRight);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, u32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %2:ref<private, u32, read_write> = shiftr %v1:ref<private, u32, read_write>, 1u
  store %v1:ref<private, u32, read_write>, %2:ref<private, u32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = LogicalAnd(LessThan(Call("my_func"), 2_f),
                            GreaterThan(2.5_f, Div(Call("my_func"), Mul(2.3_f, Call("my_func")))));
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = func my_func():f32
  %fn2 = block
  ret 0.0f
func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn4 = block
  %1:f32 = call my_func
  %2:bool = lt %1:f32, 2.0f
  %tint_symbol:bool = var function, read_write
  store %tint_symbol:bool, %2:bool
  branch %fn5

  %fn5 = if %2:bool [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    %fn6 = block
    %4:f32 = call my_func
    %5:f32 = call my_func
    %6:f32 = mul 2.29999995231628417969f, %5:f32
    %7:f32 = div %4:f32, %6:f32
    %8:bool = gt 2.5f, %7:f32
    store %tint_symbol:bool, %8:bool
    branch %fn8

  # if merge
  %fn8 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound_WithConstEval) {
    Func("my_func", utils::Vector{Param("p", ty.bool_())}, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = Call("my_func", LogicalAnd(LessThan(2.4_f, 2_f),
                                            GreaterThan(2.5_f, Div(10_f, Mul(2.3_f, 9.4_f)))));
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = func my_func():bool
  %fn2 = block
  ret true
func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn4 = block
  %tint_symbol:bool = call my_func, false
  ret
func_end

)");
}

}  // namespace
}  // namespace tint::ir
