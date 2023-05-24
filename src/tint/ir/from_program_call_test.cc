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

TEST_F(IR_BuilderImplTest, EmitExpression_Bitcast) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = Bitcast<f32>(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func my_func():f32 -> %fn1 {
  %fn1 = block {
    br %fn2 0.0f  # return
  }
  %fn2 = func_terminator
}
%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:f32 = call my_func
    %tint_symbol:f32 = bitcast %3
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_Discard) {
    auto* expr = Discard();
    Func("test_function", {}, ty.void_(), expr,
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func test_function():void [@fragment] -> %fn1 {
  %fn1 = block {
    discard
    br %fn2  # return
  }
  %fn2 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_UserFunction) {
    Func("my_func", utils::Vector{Param("p", ty.f32())}, ty.void_(), utils::Empty);

    auto* stmt = CallStmt(Call("my_func", Mul(2_a, 3_a)));
    WrapInFunction(stmt);
    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func my_func(%p:f32):void -> %fn1 {
  %fn1 = block {
    br %fn2  # return
  }
  %fn2 = func_terminator
}
%3 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %4:void = call my_func, 6.0f
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Convert) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_i));
    auto* expr = Call(ty.f32(), i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%fn1 = block {
  %i:ptr<private, i32, read_write> = var, 1i
  br %fn2  # root_end
}

%fn2 = root_terminator

%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:i32 = load %i
    %tint_symbol:f32 = convert i32, %3
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_ConstructEmpty) {
    auto* expr = vec3(ty.f32());
    GlobalVar("i", builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%fn1 = block {
  %i:ptr<private, vec3<f32>, read_write> = var, vec3<f32> 0.0f
  br %fn2  # root_end
}

%fn2 = root_terminator

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Construct) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = vec3(ty.f32(), 2_f, 3_f, i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%fn1 = block {
  %i:ptr<private, f32, read_write> = var, 1.0f
  br %fn2  # root_end
}

%fn2 = root_terminator

%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:f32 = load %i
    %tint_symbol:vec3<f32> = construct 2.0f, 3.0f, %3
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

}  // namespace
}  // namespace tint::ir
