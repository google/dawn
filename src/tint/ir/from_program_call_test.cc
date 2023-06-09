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

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/ir/program_test_helper.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_FromProgramCallTest = ProgramTestHelper;

TEST_F(IR_FromProgramCallTest, EmitExpression_Bitcast) {
    Func("my_func", utils::Empty, ty.f32(), utils::Vector{Return(0_f)});

    auto* expr = Bitcast<f32>(Call("my_func"));
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func():f32 -> %b1 {
  %b1 = block {
    ret 0.0f
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:f32 = call %my_func
    %tint_symbol:f32 = bitcast %3
    ret
  }
}
)");
}

TEST_F(IR_FromProgramCallTest, EmitStatement_Discard) {
    auto* expr = Discard();
    Func("test_function", {}, ty.void_(), expr,
         utils::Vector{
             create<ast::StageAttribute>(ast::PipelineStage::kFragment),
         });

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%test_function = @fragment func():void -> %b1 {
  %b1 = block {
    discard
    ret
  }
}
)");
}

TEST_F(IR_FromProgramCallTest, EmitStatement_UserFunction) {
    Func("my_func", utils::Vector{Param("p", ty.f32())}, ty.void_(), utils::Empty);

    auto* stmt = CallStmt(Call("my_func", Mul(2_a, 3_a)));
    WrapInFunction(stmt);
    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(%my_func = func(%p:f32):void -> %b1 {
  %b1 = block {
    ret
  }
}
%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %4:void = call %my_func, 6.0f
    ret
  }
}
)");
}

TEST_F(IR_FromProgramCallTest, EmitExpression_Convert) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_i));
    auto* expr = Call(ty.f32(), i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %i:ptr<private, i32, read_write> = var, 1i
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:i32 = load %i
    %tint_symbol:f32 = convert %3
    ret
  }
}
)");
}

TEST_F(IR_FromProgramCallTest, EmitExpression_ConstructEmpty) {
    auto* expr = vec3(ty.f32());
    GlobalVar("i", builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %i:ptr<private, vec3<f32>, read_write> = var, vec3<f32>(0.0f)
}

)");
}

TEST_F(IR_FromProgramCallTest, EmitExpression_Construct) {
    auto i = GlobalVar("i", builtin::AddressSpace::kPrivate, Expr(1_f));
    auto* expr = vec3(ty.f32(), 2_f, 3_f, i);
    WrapInFunction(expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %i:ptr<private, f32, read_write> = var, 1.0f
}

%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b2 {
  %b2 = block {
    %3:f32 = load %i
    %tint_symbol:vec3<f32> = construct 2.0f, 3.0f, %3
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
