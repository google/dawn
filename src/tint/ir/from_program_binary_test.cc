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

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = add %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Increment) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = Increment("v1");
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = add %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundAdd) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kAdd);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = add %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Subtract) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Sub(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = sub %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Decrement) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.i32());
    auto* expr = Decrement("v1");
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, i32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:i32 = load %v1
    %3:i32 = sub %2, 1i
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundSubtract) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kSubtract);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = sub %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Multiply) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mul(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = mul %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundMultiply) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kMultiply);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = mul %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Div) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Div(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = div %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundDiv) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kDivide);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = div %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Modulo) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Mod(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = mod %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundModulo) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kModulo);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = mod %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_And) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = And(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = and %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundAnd) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, ast::BinaryOp::kAnd);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, bool, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:bool = load %v1
    %3:bool = and %2, false
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Or) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Or(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = or %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundOr) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.bool_());
    auto* expr = CompoundAssign("v1", false, ast::BinaryOp::kOr);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, bool, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:bool = load %v1
    %3:bool = or %2, false
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Xor) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Xor(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = xor %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundXor) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kXor);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = xor %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalAnd) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = If(LogicalAnd(Call("my_func"), false), Block());
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():bool {
  %fn2 = block {
  } -> %func_end true # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:bool = call my_func
  } -> %fn5 # branch

  %fn5 = if %1 [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    %fn6 = block {
    } -> %fn8 false # branch

    # false branch
    %fn7 = block {
    } -> %fn8 %1 # branch

  # if merge
  %fn8 = block (%2:bool) {
  } -> %fn9 # branch

  %fn9 = if %2:bool [t: %fn10, f: %fn11, m: %fn12]
    # true branch
    %fn10 = block {
    } -> %fn12 # branch

    # false branch
    %fn11 = block {
    } -> %fn12 # branch

  # if merge
  %fn12 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LogicalOr) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = If(LogicalOr(Call("my_func"), true), Block());
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():bool {
  %fn2 = block {
  } -> %func_end true # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:bool = call my_func
  } -> %fn5 # branch

  %fn5 = if %1 [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    %fn6 = block {
    } -> %fn8 %1 # branch

    # false branch
    %fn7 = block {
    } -> %fn8 true # branch

  # if merge
  %fn8 = block (%2:bool) {
  } -> %fn9 # branch

  %fn9 = if %2:bool [t: %fn10, f: %fn11, m: %fn12]
    # true branch
    %fn10 = block {
    } -> %fn12 # branch

    # false branch
    %fn11 = block {
    } -> %fn12 # branch

  # if merge
  %fn12 = block {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Equal) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Equal(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = eq %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_NotEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = NotEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = neq %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = lt %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThan) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThan(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = gt %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_LessThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = LessThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = lte %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_GreaterThanEqual) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = GreaterThanEqual(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:bool = gte %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftLeft) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shl(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = shiftl %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundShiftLeft) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kShiftLeft);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = shiftl %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_ShiftRight) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(0_u)});
    auto* expr = Shr(Call("my_func"), 4_u);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():u32 {
  %fn2 = block {
  } -> %func_end 0u # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:u32 = call my_func
    %tint_symbol:u32 = shiftr %1, 4u
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_CompoundShiftRight) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.u32());
    auto* expr = CompoundAssign("v1", 1_u, ast::BinaryOp::kShiftRight);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = block {
  %v1:ptr<private, u32, read_write> = var
}


%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn3 = block {
    %2:u32 = load %v1
    %3:u32 = shiftr %2, 1u
    store %v1, %3
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = LogicalAnd(LessThan(Call("my_func"), 2_f),
                            GreaterThan(2.5_f, Div(Call("my_func"), Mul(2.3_f, Call("my_func")))));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func():f32 {
  %fn2 = block {
  } -> %func_end 0.0f # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %1:f32 = call my_func
    %2:bool = lt %1, 2.0f
  } -> %fn5 # branch

  %fn5 = if %2 [t: %fn6, f: %fn7, m: %fn8]
    # true branch
    %fn6 = block {
      %3:f32 = call my_func
      %4:f32 = call my_func
      %5:f32 = mul 2.29999995231628417969f, %4
      %6:f32 = div %3, %5
      %7:bool = gt 2.5f, %6
    } -> %fn8 %7 # branch

    # false branch
    %fn7 = block {
    } -> %fn8 %2 # branch

  # if merge
  %fn8 = block (%tint_symbol:bool) {
  } -> %func_end # return
} %func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Binary_Compound_WithConstEval) {
    Func("my_func", utils::Vector{Param("p", ty.bool_())}, ty.bool_(), utils::Vector{Return(true)});
    auto* expr = Call("my_func", LogicalAnd(LessThan(2.4_f, 2_f),
                                            GreaterThan(2.5_f, Div(10_f, Mul(2.3_f, 9.4_f)))));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%fn1 = func my_func(%p:bool):bool {
  %fn2 = block {
  } -> %func_end true # return
} %func_end

%fn3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] {
  %fn4 = block {
    %tint_symbol:bool = call my_func, false
  } -> %func_end # return
} %func_end

)");
}

}  // namespace
}  // namespace tint::ir
