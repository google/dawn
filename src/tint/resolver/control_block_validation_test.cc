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
#include "src/tint/ast/switch_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::resolver {
namespace {

class ResolverControlBlockValidationTest : public TestHelper, public testing::Test {};

TEST_F(ResolverControlBlockValidationTest, SwitchSelectorExpression_F32) {
    // var a : f32 = 3.14;
    // switch (a) {
    //   default: {}
    // }
    auto* var = Var("a", ty.f32(), Expr(3.14_f));

    auto* block = Block(Decl(var), Switch(Expr(Source{{12, 34}}, "a"),  //
                                          DefaultCase()));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: switch statement selector expression must be of a "
              "scalar integer type");
}

TEST_F(ResolverControlBlockValidationTest, SwitchSelectorExpression_bool) {
    // var a : bool = true;
    // switch (a) {
    //   default: {}
    // }
    auto* var = Var("a", ty.bool_(), Expr(false));

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
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),                     //
                        Switch(Source{{12, 34}}, "a",  //
                               Case(CaseSelector(1_i))));

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
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),                           //
                        Switch("a",                          //
                               DefaultCase(Source{{9, 2}}),  //
                               Case(CaseSelector(1_i)),      //
                               DefaultCase(Source{{12, 34}})));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: switch statement must have exactly one default clause
9:2 note: previous default case)");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithTwoDefault_OneInCase_Fail) {
    // var a : i32 = 2;
    // switch (a) {
    //   case 1, default: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(
        Decl(var),                                                                           //
        Switch("a",                                                                          //
               Case(utils::Vector{CaseSelector(1_i), DefaultCaseSelector(Source{{9, 2}})}),  //
               DefaultCase(Source{{12, 34}})));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: switch statement must have exactly one default clause
9:2 note: previous default case)");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithTwoDefault_SameCase) {
    // var a : i32 = 2;
    // switch (a) {
    //   case default, 1, default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block =
        Block(Decl(var),   //
              Switch("a",  //
                     Case(utils::Vector{DefaultCaseSelector(Source{{9, 2}}), CaseSelector(1_i),
                                        DefaultCaseSelector(Source{{12, 34}})})));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: switch statement must have exactly one default clause
9:2 note: previous default case)");
}

TEST_F(ResolverControlBlockValidationTest, SwitchWithTwoDefault_DifferentMultiCase) {
    // var a : i32 = 2;
    // switch (a) {
    //   case 1, default: {}
    //   case default, 2: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(
        Decl(var),   //
        Switch("a",  //
               Case(utils::Vector{CaseSelector(1_i), DefaultCaseSelector(Source{{9, 2}})}),
               Case(utils::Vector{DefaultCaseSelector(Source{{12, 34}}), CaseSelector(2_i)})));

    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(12:34 error: switch statement must have exactly one default clause
9:2 note: previous default case)");
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
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
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
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
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
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
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
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
    WrapInFunction(
        For(nullptr, false, nullptr, Block(decl_z, Block(Block(Block(cont))), assign_z)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(cont)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_break) {
    // switch (1i) {
    //   case 1i: {
    //     var z: i32;
    //     break;
    //     z = 1i;
    //   default: {}
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* brk = Break();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
    WrapInFunction(                                                          //
        Block(Switch(1_i,                                                    //
                     Case(CaseSelector(1_i), Block(decl_z, brk, assign_z)),  //
                     DefaultCase())));

    ASSERT_TRUE(r()->Resolve()) << r()->error();
    EXPECT_EQ(r()->error(), "12:34 warning: code is unreachable");
    EXPECT_TRUE(Sem().Get(decl_z)->IsReachable());
    EXPECT_TRUE(Sem().Get(brk)->IsReachable());
    EXPECT_FALSE(Sem().Get(assign_z)->IsReachable());
}

TEST_F(ResolverControlBlockValidationTest, UnreachableCode_break_InBlocks) {
    // loop {
    //   switch (1i) {
    //     case 1i: { {{{break;}}} var a : u32 = 2;}
    //     default: {}
    //   }
    //   break;
    // }
    auto* decl_z = Decl(Var("z", ty.i32()));
    auto* brk = Break();
    auto* assign_z = Assign(Source{{12, 34}}, "z", 1_i);
    WrapInFunction(Loop(
        Block(Switch(1_i,  //
                     Case(CaseSelector(1_i), Block(decl_z, Block(Block(Block(brk))), assign_z)),
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
    //   case 1i: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var), Switch("a",                                        //
                                          Case(CaseSelector(Source{{12, 34}}, 1_u)),  //
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
    //   case -1i: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.u32(), Expr(2_u));

    auto* block = Block(Decl(var),                                          //
                        Switch("a",                                         //
                               Case(CaseSelector(Source{{12, 34}}, -1_i)),  //
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
    auto* var = Var("a", ty.u32(), Expr(3_u));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(CaseSelector(0_u)),
                               Case(utils::Vector{
                                   CaseSelector(Source{{12, 34}}, 2_u),
                                   CaseSelector(3_u),
                                   CaseSelector(Source{{56, 78}}, 2_u),
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
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(CaseSelector(Source{{12, 34}}, -10_i)),
                               Case(utils::Vector{
                                   CaseSelector(0_i),
                                   CaseSelector(1_i),
                                   CaseSelector(2_i),
                                   CaseSelector(Source{{56, 78}}, -10_i),
                               }),
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: duplicate switch case '-10'\n"
              "12:34 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Pass) {
    // var a : i32 = 2;
    // switch (a) {
    //   default: {}
    //   case 5: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),                             //
                        Switch("a",                            //
                               DefaultCase(Source{{12, 34}}),  //
                               Case(CaseSelector(5_i))));
    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Expression_Pass) {
    // var a : i32 = 2;
    // switch (a) {
    //   default: {}
    //   case 5 + 6: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),                             //
                        Switch("a",                            //
                               DefaultCase(Source{{12, 34}}),  //
                               Case(CaseSelector(Add(5_i, 6_i)))));
    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Expression_MixI32_Abstract) {
    // var a = 2;
    // switch (a) {
    //   default: {}
    //   case 5i + 6i: {}
    // }
    auto* var = Var("a", Expr(2_a));

    auto* block = Block(Decl(var),                             //
                        Switch("a",                            //
                               DefaultCase(Source{{12, 34}}),  //
                               Case(CaseSelector(Add(5_i, 6_i)))));
    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Expression_MixU32_Abstract) {
    // var a = 2u;
    // switch (a) {
    //   default: {}
    //   case 5 + 6: {}
    // }
    auto* var = Var("a", Expr(2_u));

    auto* block = Block(Decl(var),                             //
                        Switch("a",                            //
                               DefaultCase(Source{{12, 34}}),  //
                               Case(CaseSelector(Add(5_a, 6_a)))));
    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, SwitchCase_Expression_Multiple) {
    // var a = 2u;
    // switch (a) {
    //   default: {}
    //   case 5 + 6, 7+9, 2*4: {}
    // }
    auto* var = Var("a", Expr(2_u));

    auto* block =
        Block(Decl(var),                             //
              Switch("a",                            //
                     DefaultCase(Source{{12, 34}}),  //
                     Case(utils::Vector{CaseSelector(Add(5_u, 6_u)), CaseSelector(Add(7_u, 9_u)),
                                        CaseSelector(Mul(2_u, 4_u))})));
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
    auto* var = Var("a", ty.Of(my_int), Expr(2_u));
    auto* block = Block(Decl(var),  //
                        Switch("a", DefaultCase(Source{{12, 34}})));

    WrapInFunction(block);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, NonUniqueCaseSelector_Expression_Fail) {
    // var a : i32 = 2i;
    // switch (a) {
    //   case 10i: {}
    //   case 5i+5i: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(CaseSelector(Source{{12, 34}}, 10_i)),
                               Case(CaseSelector(Source{{56, 78}}, Add(5_i, 5_i))), DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "56:78 error: duplicate switch case '10'\n"
              "12:34 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, NonUniqueCaseSelectorSameCase_BothExpression_Fail) {
    // var a : i32 = 2i;
    // switch (a) {
    //   case 5i+5i, 6i+4i: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(utils::Vector{CaseSelector(Source{{56, 78}}, Add(5_i, 5_i)),
                                                  CaseSelector(Source{{12, 34}}, Add(6_i, 4_i))}),
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: duplicate switch case '10'\n"
              "56:78 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, NonUniqueCaseSelectorSame_Case_Expression_Fail) {
    // var a : i32 = 2i;
    // switch (a) {
    //   case 5u+5u, 10i: {}
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(utils::Vector{CaseSelector(Source{{56, 78}}, Add(5_i, 5_i)),
                                                  CaseSelector(Source{{12, 34}}, 10_i)}),
                               DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              "12:34 error: duplicate switch case '10'\n"
              "56:78 note: previous case declared here");
}

TEST_F(ResolverControlBlockValidationTest, Switch_OverrideCondition_Fail) {
    // override a : i32 = 2;
    // switch (a) {
    //   default: {}
    // }
    auto* var = Var("a", ty.i32(), Expr(2_i));
    Override("b", ty.i32(), Expr(2_i));

    auto* block = Block(Decl(var),   //
                        Switch("a",  //
                               Case(CaseSelector(Source{{12, 34}}, "b")), DefaultCase()));
    WrapInFunction(block);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: case selector must be a constant expression");
}

constexpr size_t kMaxSwitchCaseSelectors = 16383;

TEST_F(ResolverControlBlockValidationTest, Switch_MaxSelectors_Valid) {
    utils::Vector<const ast::CaseStatement*, 0> cases;
    for (size_t i = 0; i < kMaxSwitchCaseSelectors - 1; ++i) {
        cases.Push(Case(CaseSelector(Expr(i32(i)))));
    }
    cases.Push(DefaultCase());

    auto* var = Var("a", ty.i32());
    auto* s = Switch("a", std::move(cases));
    WrapInFunction(var, s);

    EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverControlBlockValidationTest, Switch_MaxSelectors_Invalid) {
    utils::Vector<const ast::CaseStatement*, 0> cases;
    for (size_t i = 0; i < kMaxSwitchCaseSelectors; ++i) {
        cases.Push(Case(CaseSelector(Expr(i32(i)))));
    }
    cases.Push(DefaultCase());

    auto* var = Var("a", ty.i32());
    auto* s = Switch(Source{{12, 34}}, "a", std::move(cases));
    WrapInFunction(var, s);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: switch statement has 16384 case selectors, max is 16383");
}

}  // namespace
}  // namespace tint::resolver
