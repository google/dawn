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

#include "src/tint/resolver/resolver.h"

#include "gtest/gtest.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/expression.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/if_statement.h"

namespace tint::resolver {
namespace {

class ResolverBehaviorTest : public ResolverTest {
  protected:
    void SetUp() override {
        // Create a function called 'DiscardOrNext' which returns an i32, and has
        // the behavior of {Discard, Return}, which when called, will have the
        // behavior {Discard, Next}.
        Func("DiscardOrNext", {}, ty.i32(),
             {
                 If(true, Block(Discard())),
                 Return(1),
             });
    }
};

TEST_F(ResolverBehaviorTest, ExprBinaryOp_LHS) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Add(Call("DiscardOrNext"), 1)));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprBinaryOp_RHS) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Add(1, Call("DiscardOrNext"))));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprBitcastOp) {
    auto* stmt = Decl(Var("lhs", ty.u32(), Bitcast<u32>(Call("DiscardOrNext"))));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprIndex_Arr) {
    Func("ArrayDiscardOrNext", {}, ty.array<i32, 4>(),
         {
             If(true, Block(Discard())),
             Return(Construct(ty.array<i32, 4>())),
         });

    auto* stmt = Decl(Var("lhs", ty.i32(), IndexAccessor(Call("ArrayDiscardOrNext"), 1)));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprIndex_Idx) {
    auto* stmt = Decl(Var("lhs", ty.i32(), IndexAccessor("arr", Call("DiscardOrNext"))));
    WrapInFunction(Decl(Var("arr", ty.array<i32, 4>())),  //
                   stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, ExprUnaryOp) {
    auto* stmt =
        Decl(Var("lhs", ty.i32(),
                 create<ast::UnaryOpExpression>(ast::UnaryOp::kComplement, Call("DiscardOrNext"))));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtAssign) {
    auto* stmt = Assign("lhs", "rhs");
    WrapInFunction(Decl(Var("lhs", ty.i32())),  //
                   Decl(Var("rhs", ty.i32())),  //
                   stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtAssign_LHSDiscardOrNext) {
    auto* stmt = Assign(IndexAccessor("lhs", Call("DiscardOrNext")), 1);
    WrapInFunction(Decl(Var("lhs", ty.array<i32, 4>())),  //
                   stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtAssign_RHSDiscardOrNext) {
    auto* stmt = Assign("lhs", Call("DiscardOrNext"));
    WrapInFunction(Decl(Var("lhs", ty.i32())),  //
                   stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtBlockEmpty) {
    auto* stmt = Block();
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtBlockSingleStmt) {
    auto* stmt = Block(Discard());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtCallReturn) {
    Func("f", {}, ty.void_(), {Return()});
    auto* stmt = CallStmt(Call("f"));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtCallFuncDiscard) {
    Func("f", {}, ty.void_(), {Discard()});
    auto* stmt = CallStmt(Call("f"));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtCallFuncMayDiscard) {
    auto* stmt =
        For(Decl(Var("v", ty.i32(), Call("DiscardOrNext"))), nullptr, nullptr, Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtBreak) {
    auto* stmt = Break();
    WrapInFunction(Loop(Block(stmt)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kBreak);
}

TEST_F(ResolverBehaviorTest, StmtContinue) {
    auto* stmt = Continue();
    WrapInFunction(Loop(Block(If(true, Block(Break())),  //
                              stmt)));

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kContinue);
}

TEST_F(ResolverBehaviorTest, StmtDiscard) {
    auto* stmt = Discard();
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_NoExit) {
    auto* stmt = For(Source{{12, 34}}, nullptr, nullptr, nullptr, Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: for-loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtForLoopBreak) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtForLoopContinue_NoExit) {
    auto* stmt = For(Source{{12, 34}}, nullptr, nullptr, nullptr, Block(Continue()));
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: for-loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtForLoopDiscard) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Discard()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtForLoopReturn) {
    auto* stmt = For(nullptr, nullptr, nullptr, Block(Return()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtForLoopBreak_InitCallFuncMayDiscard) {
    auto* stmt =
        For(Decl(Var("v", ty.i32(), Call("DiscardOrNext"))), nullptr, nullptr, Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_InitCallFuncMayDiscard) {
    auto* stmt = For(Decl(Var("v", ty.i32(), Call("DiscardOrNext"))), nullptr, nullptr, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_CondTrue) {
    auto* stmt = For(nullptr, true, nullptr, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtForLoopEmpty_CondCallFuncMayDiscard) {
    auto* stmt = For(nullptr, Equal(Call("DiscardOrNext"), 1), nullptr, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock) {
    auto* stmt = If(true, Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenDiscard) {
    auto* stmt = If(true, Block(Discard()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock_ElseDiscard) {
    auto* stmt = If(true, Block(), Block(Discard()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenDiscard_ElseDiscard) {
    auto* stmt = If(true, Block(Discard()), Block(Discard()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtIfCallFuncMayDiscard_ThenEmptyBlock) {
    auto* stmt = If(Equal(Call("DiscardOrNext"), 1), Block());
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtIfTrue_ThenEmptyBlock_ElseCallFuncMayDiscard) {
    auto* stmt = If(true, Block(),  //
                    If(Equal(Call("DiscardOrNext"), 1), Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtLetDecl) {
    auto* stmt = Decl(Let("v", ty.i32(), Expr(1)));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtLetDecl_RHSDiscardOrNext) {
    auto* stmt = Decl(Let("lhs", ty.i32(), Call("DiscardOrNext")));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopBreak) {
    auto* stmt = Loop(Block(Break()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtLoopContinue_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block(Continue()));
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopDiscard) {
    auto* stmt = Loop(Block(Discard()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtLoopReturn) {
    auto* stmt = Loop(Block(Return()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_ContEmpty_NoExit) {
    auto* stmt = Loop(Source{{12, 34}}, Block(), Block());
    WrapInFunction(stmt);

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), "12:34 error: loop does not exit");
}

TEST_F(ResolverBehaviorTest, StmtLoopEmpty_ContIfTrueBreak) {
    auto* stmt = Loop(Block(), Block(If(true, Block(Break()))));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtReturn) {
    auto* stmt = Return();
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtReturn_DiscardOrNext) {
    auto* stmt = Return(Call("DiscardOrNext"));
    Func("F", {}, ty.i32(), {stmt});

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kReturn, sem::Behavior::kDiscard));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondTrue_DefaultEmpty) {
    auto* stmt = Switch(1, DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultEmpty) {
    auto* stmt = Switch(1, DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultDiscard) {
    auto* stmt = Switch(1, DefaultCase(Block(Discard())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_DefaultReturn) {
    auto* stmt = Switch(1, DefaultCase(Block(Return())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kReturn);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultEmpty) {
    auto* stmt = Switch(1, Case(Expr(0), Block()), DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultDiscard) {
    auto* stmt = Switch(1, Case(Expr(0), Block()), DefaultCase(Block(Discard())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext, sem::Behavior::kDiscard));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Empty_DefaultReturn) {
    auto* stmt = Switch(1, Case(Expr(0), Block()), DefaultCase(Block(Return())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kNext, sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultEmpty) {
    auto* stmt = Switch(1, Case(Expr(0), Block(Discard())), DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultDiscard) {
    auto* stmt = Switch(1, Case(Expr(0), Block(Discard())), DefaultCase(Block(Discard())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kDiscard);
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_DefaultReturn) {
    auto* stmt = Switch(1, Case(Expr(0), Block(Discard())), DefaultCase(Block(Return())));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondLiteral_Case0Discard_Case1Return_DefaultEmpty) {
    auto* stmt = Switch(1,                                //
                        Case(Expr(0), Block(Discard())),  //
                        Case(Expr(1), Block(Return())),   //
                        DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext,
                                               sem::Behavior::kReturn));
}

TEST_F(ResolverBehaviorTest, StmtSwitch_CondCallFuncMayDiscard_DefaultEmpty) {
    auto* stmt = Switch(Call("DiscardOrNext"), DefaultCase(Block()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

TEST_F(ResolverBehaviorTest, StmtVarDecl) {
    auto* stmt = Decl(Var("v", ty.i32()));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behavior::kNext);
}

TEST_F(ResolverBehaviorTest, StmtVarDecl_RHSDiscardOrNext) {
    auto* stmt = Decl(Var("lhs", ty.i32(), Call("DiscardOrNext")));
    WrapInFunction(stmt);

    ASSERT_TRUE(r()->Resolve()) << r()->error();

    auto* sem = Sem().Get(stmt);
    EXPECT_EQ(sem->Behaviors(), sem::Behaviors(sem::Behavior::kDiscard, sem::Behavior::kNext));
}

}  // namespace
}  // namespace tint::resolver
