// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"

namespace tint::resolver {
namespace {

class ResolverControlBlockValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverControlBlockValidationTest, SwitchSelectorExpressionNoneIntegerType_Fail) {
    // var a : f32 = 3.14;
    // switch (a) {
    //   default: {}
    // }
    auto* var = Var("a", ty.f32(), Expr(3.14f));

    auto* block = Block(Decl(var), Switch(Expr(Source{{12, 34}}, "a"),  //
                                          DefaultCase()));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: switch statement selector expression must be of a "
              "scalar integer type");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithoutDefault_Fail) {
    // var a : i32 = 2;
    // switch (a) {
    //   case 1: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2));

    auto* block = Block(Decl(var),                     //
                        Switch(Source{{12, 34}}, "a",  //
                               Case(Expr(1))));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: switch statement must have a default clause");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithTwoDefault_Fail) {
    // var a : i32 = 2;
    // switch (a) {
    //   default: {}
    //   case 1: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2));

    auto* block = Block(Decl(var),             //
                        Switch("a",            //
                               DefaultCase(),  //
                               Case(Expr(1)),  //
                               DefaultCase(Source{{12, 34}})));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: switch statement must have exactly one default clause");
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_Loop_continue) {
    // loop {
    //   if (false) { break; }
    //   var z: i32;
    //   continue;
    //   z = 1;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* cont = Continue();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(Loop(Block(If(false, Block(Break())), decl_z, cont, assign_z)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(cont)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_Loop_continue_InBlocks) {
    // loop {
    //   if (false) { break; }
    //   var z: i32;
    //   {{{continue;}}}
    //   z = 1;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* cont = Continue();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(
        Loop(Block(If(false, Block(Break())), decl_z, Block(Block(Block(cont))), assign_z)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(cont)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_ForLoop_continue) {
    // for (;false;) {
    //   var z: i32;
    //   continue;
    //   z = 1;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* cont = Continue();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(For(nullptr, false, nullptr,  //
                       Block(decl_z, cont, assign_z)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(cont)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_ForLoop_continue_InBlocks) {
    // for (;false;) {
    //   var z: i32;
    //   {{{continue;}}}
    //   z = 1;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* cont = Continue();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(
        For(nullptr, false, nullptr, Block(decl_z, Block(Block(Block(cont))), assign_z)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(cont)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_break) {
    // switch (1) {
    //   case 1: {
    //     var z: i32;
    //     break;
    //     z = 1;
    //   default: {}
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* brk = Break();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(                                                //
        Block(Switch(1,                                            //
                     Case(Expr(1), Block(decl_z, brk, assign_z)),  //
                     DefaultCase())));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(brk)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_break_InBlocks) {
    // loop {
    //   switch (1) {
    //     case 1: { {{{break;}}} var a : u32 = 2;}
    //     default: {}
    //   }
    //   break;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* brk = Break();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1);
    WrapInFunction(
        Loop(Block(Switch(1,  //
                          Case(Expr(1), Block(decl_z, Block(Block(Block(brk))), assign_z)),
                          DefaultCase()),  //
                   Break())));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(brk)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, SwitchConditionTypeMustMatchSelectorType2_Fail) {
    // var a : u32 = 2;
    // switch (a) {
    //   case 1: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2));

    auto* block = Block(Decl(var), Switch("a",                                 //
                                          Case(Source{{12, 34}}, {Expr(1u)}),  //
                                          DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: the case selector values must have the same type as "
              "the selector expression.");
}

TEST_F(ResolverControlBlockValidationTest, SwitchConditionTypeMustMatchSelectorType_Fail) {
    // var a : u32 = 2;
    // switch (a) {
    //   case -1: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.u32(), Expr(2u));

    auto* block = Block(Decl(var),                                  //
                        Switch("a",                                 //
                               Case(Source{{12, 34}}, {Expr(-1)}),  //
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: the case selector values must have the same type as "
              "the selector expression.");
}

TEST_F(ResolverControlBlockValidationTest, NonUniqueCaseSelectorValueUint_Fail) {
    // var a : u32 = 3;
    // switch (a) {
    //   case 0u: {}
    //   case 2u, 3u, 2u: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.u32(), Expr(3u));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(Expr(0u)),
                               Case({
                                   Expr(Source{{12, 34}}, 2u),
                                   Expr(3u),
                                   Expr(Source{{56, 78}}, 2u),
                               }),
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: duplicate switch case '2'\n"
              "12:34 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, NonUniqueCaseSelectorValueSint_Fail) {
    // var a : i32 = 2;
    // switch (a) {
    //   case -10: {}
    //   case 0,1,2,-10: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(Expr(Source{{12, 34}}, -10)),
                               Case({
                                   Expr(0),
                                   Expr(1),
                                   Expr(2),
                                   Expr(Source{{56, 78}}, -10),
                               }),
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: duplicate switch case '-10'\n"
              "12:34 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, LastClauseLastStatementIsFallthrough_Fail) {
    // var a : i32 = 2;
    // switch (a) {
    //   default: { fallthrough; }
    // }
    auto* var = Var("a", ty.i32(), Expr(2));
    auto* fallthrough = create<ast::FallthroughStatement>(Source{{12, 34}});
    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               DefaultCase(Block(fallthrough))));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: a fallthrough statement must not be used in the last "
              "switch case");
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Pass) {
    // var a : i32 = 2;
    // switch (a) {
    //   default: {}
    //   case 5: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2));

    auto* block = Block(Decl(var),                             //
                        Switch("a",                            //
                               DefaultCase(Source{{12, 34}}),  //
                               Case(Expr(5))));
    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCaseAlias_Pass) {
    // type MyInt = u32;
    // var v: MyInt;
    // switch(v){
    //   default: {}
    // }

    auto* my_int = Alias("MyInt", ty.u32());
    auto* var = Var("a", ty.Of(my_int), Expr(2u));
    auto* block = Block(Decl(var),  //
                        Switch("a", DefaultCase(Source{{12, 34}})));

    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

}  // namespace
}  // namespace tint::resolver
