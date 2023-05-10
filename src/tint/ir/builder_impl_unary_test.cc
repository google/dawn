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

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:bool = call my_func
%2:bool = eq %1:bool, false
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Complement) {
    Func("my_func", utils::Empty, ty.u32(), utils::Vector{Return(1_u)});
    auto* expr = Complement(Call("my_func"));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:u32 = call my_func
%2:u32 = complement %1:u32
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Negation) {
    Func("my_func", utils::Empty, ty.i32(), utils::Vector{Return(1_i)});
    auto* expr = Negation(Call("my_func"));
    WrapInFunction(expr);

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:i32 = call my_func
%2:i32 = negation %1:i32
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_AddressOf) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.i32());

    auto* expr = Decl(Let("v2", AddressOf("v1")));
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, i32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %v2:ptr<private, i32, read_write> = addr_of %v1:ref<private, i32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Unary_Indirection) {
    GlobalVar("v1", builtin::AddressSpace::kPrivate, ty.i32());
    utils::Vector stmts = {
        Decl(Let("v3", AddressOf("v1"))),
        Decl(Let("v2", Deref("v3"))),
    };
    WrapInFunction(stmts);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%v1:ref<private, i32, read_write> = var private, read_write



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %v3:ptr<private, i32, read_write> = addr_of %v1:ref<private, i32, read_write>
  %v2:i32 = indirection %v3:ptr<private, i32, read_write>
  ret
func_end

)");
}

}  // namespace
}  // namespace tint::ir
