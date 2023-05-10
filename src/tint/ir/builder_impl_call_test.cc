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

    auto& b = CreateBuilder();
    InjectFlowBlock();
    auto r = b.EmitExpression(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());
    ASSERT_TRUE(r);

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:f32 = call my_func
%2:f32 = bitcast %1:f32
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_Discard) {
    auto* expr = Discard();
    Func("test_function", {}, ty.void_(), expr,
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    auto& b = CreateBuilder();
    InjectFlowBlock();
    b.EmitStatement(expr);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(discard
)");
}

TEST_F(IR_BuilderImplTest, EmitStatement_UserFunction) {
    Func("my_func", utils::Vector{Param("p", ty.f32())}, ty.void_(), utils::Empty);

    auto* stmt = CallStmt(Call("my_func", Mul(2_a, 3_a)));
    WrapInFunction(stmt);

    auto& b = CreateBuilder();

    InjectFlowBlock();
    b.EmitStatement(stmt);
    ASSERT_THAT(b.Diagnostics(), testing::IsEmpty());

    Disassembler d(b.builder.ir);
    d.EmitBlockInstructions(b.current_flow_block->As<ir::Block>());
    EXPECT_EQ(d.AsString(), R"(%1:void = call my_func, 6.0f
)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Convert) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_i));
    auto* expr = Call(ty.f32(), i);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();
    ASSERT_TRUE(r);

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%i:ref<private, i32, read_write> = var private, read_write, 1i



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %tint_symbol:f32 = convert i32, %i:ref<private, i32, read_write>
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_ConstructEmpty) {
    auto* expr = vec3(ty.f32());
    GlobalVar("i", builtin::AddressSpace::kPrivate, expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();
    ASSERT_TRUE(r);

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%i:ref<private, vec3<f32>, read_write> = var private, read_write, vec3<f32> 0.0f



)");
}

TEST_F(IR_BuilderImplTest, EmitExpression_Construct) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = vec3(ty.f32(), 2_f, 3_f, i);
    WrapInFunction(expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();
    ASSERT_TRUE(r);

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%i:ref<private, f32, read_write> = var private, read_write, 1.0f



%fn2 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn3 = block
  %tint_symbol:vec3<f32> = construct 2.0f, 3.0f, %i:ref<private, f32, read_write>
  ret
func_end

)");
}

}  // namespace
}  // namespace tint::ir
