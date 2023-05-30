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

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_NoInit) {
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %a:ptr<private, u32, read_write> = var
}

)");
}

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_Init) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate, expr);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %a:ptr<private, u32, read_write> = var, 2u
}

)");
}

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_GroupBinding) {
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kStorage,
              utils::Vector{Group(2_u), Binding(3_u)});

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()), R"(# Root block
%b1 = block {
  %a:ptr<storage, u32, read> = var @binding_point(2, 3)
}

)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_NoInit) {
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var
    ret
  }
}
)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_Init_Constant) {
    auto* expr = Expr(2_u);
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction, expr);
    WrapInFunction(a);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var, 2u
    ret
  }
}
)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_Init_NonConstant) {
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction);
    auto* b = Var("b", ty.u32(), builtin::AddressSpace::kFunction, Add("a", 2_u));
    WrapInFunction(a, b);

    auto m = Build();
    ASSERT_TRUE(m) << (!m ? m.Failure() : "");

    EXPECT_EQ(Disassemble(m.Get()),
              R"(%test_function = @compute @workgroup_size(1, 1, 1) func():void -> %b1 {
  %b1 = block {
    %a:ptr<function, u32, read_write> = var
    %3:u32 = load %a
    %4:u32 = add %3, 2u
    %b:ptr<function, u32, read_write> = var, %4
    ret
  }
}
)");
}

}  // namespace
}  // namespace tint::ir
