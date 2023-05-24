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

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Not) {
    Func("my_func", utils::Empty, ty.bool_(), utils::Vector{Return(false)});
    auto* expr = Not(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func my_func():bool -> %fn1 {
  %fn1 = block {
    br %fn2 false  # return
  }
  %fn2 = func_terminator
}
%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:bool = call my_func
    %tint_symbol:bool = eq %3, false
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Complement) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(1_u)});
    auto* expr = Complement(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func my_func():u32 -> %fn1 {
  %fn1 = block {
    br %fn2 1u  # return
  }
  %fn2 = func_terminator
}
%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:u32 = call my_func
    %tint_symbol:u32 = complement %3
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Negation) {
    Func("my_func", utils::Empty, ty.i32(), utils::Vector{Return(1_i)});
    auto* expr = Negation(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%1 = func my_func():i32 -> %fn1 {
  %fn1 = block {
    br %fn2 1i  # return
  }
  %fn2 = func_terminator
}
%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    %3:i32 = call my_func
    %tint_symbol:i32 = negation %3
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_AddressOf) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.i32());

    auto* expr = Decl(Let("v2", AddressOf("v1")));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%fn1 = block {
  %v2:ptr<private, i32, read_write> = var
  br %fn2  # root_end
}

%fn2 = root_terminator

%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Indirection) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.i32());
    utils::Vector stmts = {
        Decl(Let("v3", AddressOf("v1"))),
        Assign(Deref("v3"), 42_i),
    };
    WrapInFunction(stmts);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%fn1 = block {
  %v3:ptr<private, i32, read_write> = var
  br %fn2  # root_end
}

%fn2 = root_terminator

%2 = func test_function():void [@compute @workgroup_size(1, 1, 1)] -> %fn3 {
  %fn3 = block {
    store %v3, 42i
    br %fn4  # return
  }
  %fn4 = func_terminator
}
)");
}

}  // namespace
}  // namespace tint::ir
