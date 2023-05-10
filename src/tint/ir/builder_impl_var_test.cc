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

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%a:ref<private, u32, read_write> = var private, read_write



)");
}

TEST_F(IR_BuilderImplTest, Emit_GlobalVar_Init) {
    auto* expr = Expr(2_u);
    GlobalVar("a", ty.u32(), builtin::AddressSpace::kPrivate, expr);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn1 = block
%a:ref<private, u32, read_write> = var private, read_write, 2u



)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_NoInit) {
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction);
    WrapInFunction(a);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn2 = block
  %a:ref<function, u32, read_write> = var function, read_write
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, Emit_Var_Init) {
    auto* expr = Expr(2_u);
    auto* a = Var("a", ty.u32(), builtin::AddressSpace::kFunction, expr);
    WrapInFunction(a);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m),
              R"(%fn1 = func test_function():void [@compute @workgroup_size(1, 1, 1)]
  %fn2 = block
  %a:ref<function, u32, read_write> = var function, read_write, 2u
  ret
func_end

)");
}
}  // namespace
}  // namespace tint::ir
