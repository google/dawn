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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/block_statement.h"

namespace tint {
namespace resolver {
namespace {

using ResolverBlockTest = ResolverTest;

TEST_F(ResolverBlockTest, FunctionBlock) {
  // fn F() {
  //   var x : 32;
  // }
  auto* stmt = Decl(Var("x", ty.i32()));
  auto* f = Func("F", {}, ty.void_(), {stmt});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* s = Sem().Get(stmt);
  ASSERT_NE(s, nullptr);
  ASSERT_NE(s->Block(), nullptr);
  ASSERT_TRUE(s->Block()->Is<sem::FunctionBlockStatement>());
  EXPECT_EQ(s->Block(), s->Block()->FindFirstParent<sem::BlockStatement>());
  EXPECT_EQ(s->Block(),
            s->Block()->FindFirstParent<sem::FunctionBlockStatement>());
  EXPECT_EQ(s->Block()->As<sem::FunctionBlockStatement>()->Function(), f);
  EXPECT_EQ(s->Block()->Parent(), nullptr);
}

TEST_F(ResolverBlockTest, Block) {
  // fn F() {
  //   {
  //     var x : 32;
  //   }
  // }
  auto* stmt = Decl(Var("x", ty.i32()));
  auto* block = Block(stmt);
  auto* f = Func("F", {}, ty.void_(), {block});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* s = Sem().Get(stmt);
  ASSERT_NE(s, nullptr);
  ASSERT_NE(s->Block(), nullptr);
  EXPECT_EQ(s->Block(), s->Block()->FindFirstParent<sem::BlockStatement>());
  EXPECT_EQ(s->Block()->Parent(),
            s->Block()->FindFirstParent<sem::FunctionBlockStatement>());
  ASSERT_TRUE(s->Block()->Parent()->Is<sem::FunctionBlockStatement>());
  EXPECT_EQ(s->Block()->Parent()->As<sem::FunctionBlockStatement>()->Function(),
            f);
  EXPECT_EQ(s->Block()->Parent()->Parent(), nullptr);
}

TEST_F(ResolverBlockTest, LoopBlock) {
  // fn F() {
  //   loop {
  //     var x : 32;
  //   }
  // }
  auto* stmt = Decl(Var("x", ty.i32()));
  auto* loop = Loop(Block(stmt));
  auto* f = Func("F", {}, ty.void_(), {loop});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* s = Sem().Get(stmt);
  ASSERT_NE(s, nullptr);
  ASSERT_NE(s->Block(), nullptr);
  EXPECT_EQ(s->Block(), s->Block()->FindFirstParent<sem::LoopBlockStatement>());
  ASSERT_TRUE(Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
  EXPECT_EQ(s->Block()->Parent()->Parent(),
            s->Block()->FindFirstParent<sem::FunctionBlockStatement>());
  EXPECT_EQ(s->Block()
                ->Parent()
                ->Parent()
                ->As<sem::FunctionBlockStatement>()
                ->Function(),
            f);
  EXPECT_EQ(s->Block()->Parent()->Parent()->Parent(), nullptr);
}

TEST_F(ResolverBlockTest, ForLoopBlock) {
  // fn F() {
  //   for (var i : u32; true; i = i + 1u) {
  //     return;
  //   }
  // }
  auto* init = Decl(Var("i", ty.u32()));
  auto* cond = Expr(true);
  auto* cont = Assign("i", Add("i", 1u));
  auto* stmt = Return();
  auto* body = Block(stmt);
  auto* for_ = For(init, cond, cont, body);
  auto* f = Func("F", {}, ty.void_(), {for_});

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  {
    auto* s = Sem().Get(init);
    ASSERT_NE(s, nullptr);
    ASSERT_NE(s->Block(), nullptr);
    EXPECT_EQ(s->Block(),
              s->Block()->FindFirstParent<sem::LoopBlockStatement>());
    ASSERT_TRUE(
        Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
  }
  {  // Condition expression's statement is the for-loop itself
    auto* s = Sem().Get(cond);
    ASSERT_NE(s, nullptr);
    ASSERT_NE(s->Stmt()->Block(), nullptr);
    EXPECT_EQ(
        s->Stmt()->Block(),
        s->Stmt()->Block()->FindFirstParent<sem::FunctionBlockStatement>());
    ASSERT_TRUE(Is<sem::FunctionBlockStatement>(s->Stmt()->Block()));
  }
  {
    auto* s = Sem().Get(cont);
    ASSERT_NE(s, nullptr);
    ASSERT_NE(s->Block(), nullptr);
    EXPECT_EQ(s->Block(),
              s->Block()->FindFirstParent<sem::LoopBlockStatement>());
    ASSERT_TRUE(
        Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
  }
  {
    auto* s = Sem().Get(stmt);
    ASSERT_NE(s, nullptr);
    ASSERT_NE(s->Block(), nullptr);
    EXPECT_EQ(s->Block(),
              s->Block()->FindFirstParent<sem::LoopBlockStatement>());
    ASSERT_TRUE(
        Is<sem::FunctionBlockStatement>(s->Block()->Parent()->Parent()));
    EXPECT_EQ(s->Block()->Parent()->Parent(),
              s->Block()->FindFirstParent<sem::FunctionBlockStatement>());
    EXPECT_EQ(s->Block()
                  ->Parent()
                  ->Parent()
                  ->As<sem::FunctionBlockStatement>()
                  ->Function(),
              f);
    EXPECT_EQ(s->Block()->Parent()->Parent()->Parent(), nullptr);
  }
}
// TODO(bclayton): Add tests for other block types
//                 (LoopContinuingBlockStatement, SwitchCaseBlockStatement)

}  // namespace
}  // namespace resolver
}  // namespace tint
