// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::hlsl::writer {
namespace {

using HlslASTPrinterTest_Switch = TestHelper;

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* s = Switch(                             //
        Expr("cond"),                             //
        Case(CaseSelector(5_i), Block(Break())),  //
        DefaultCase());
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_MixedDefault) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Expr("cond"),  //
        Case(Vector{CaseSelector(5_i), DefaultCaseSelector()}, Block(Break())));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5:
    default: {
      break;
    }
  }
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_OnlyDefaultCase_NoSideEffectsCondition) {
    // var<private> cond : i32;
    // var<private> a : i32;
    // fn test() {
    //   switch(cond) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);
    GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Expr("cond"),  //
        DefaultCase(Block(Assign(Expr("a"), Expr(42_i)))));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  do {
    a = 42;
  } while (false);
)");
}

TEST_F(HlslASTPrinterTest_Switch, Emit_Switch_OnlyDefaultCase_SideEffectsCondition) {
    // var<private> global : i32;
    // fn bar() -> i32 {
    //   global = 84;
    //   return global;
    // }
    //
    // var<private> a : i32;
    // fn test() {
    //   switch(bar()) {
    //     default: {
    //       a = 42;
    //     }
    //   }
    // }
    GlobalVar("global", ty.i32(), builtin::AddressSpace::kPrivate);
    Func("bar", {}, ty.i32(),
         Vector{                               //
                Assign("global", Expr(84_i)),  //
                Return("global")});

    GlobalVar("a", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* s = Switch(  //
        Call("bar"),   //
        DefaultCase(Block(Assign(Expr("a"), Expr(42_i)))));
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  bar();
  do {
    a = 42;
  } while (false);
)");
}

}  // namespace
}  // namespace tint::hlsl::writer
