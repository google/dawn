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

#include "src/resolver/resolver.h"

#include "gmock/gmock.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/semantic/call.h"
#include "src/semantic/function.h"
#include "src/semantic/member_accessor_expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"
#include "src/type/access_control_type.h"
#include "src/type/sampled_texture_type.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace {

using IntrinsicType = semantic::IntrinsicType;

class FakeStmt : public ast::Statement {
 public:
  explicit FakeStmt(Source source) : ast::Statement(source) {}
  FakeStmt* Clone(CloneContext*) const override { return nullptr; }
  bool IsValid() const override { return true; }
  void to_str(const semantic::Info&, std::ostream& out, size_t) const override {
    out << "Fake";
  }
};

class FakeExpr : public ast::Expression {
 public:
  explicit FakeExpr(Source source) : ast::Expression(source) {}
  FakeExpr* Clone(CloneContext*) const override { return nullptr; }
  bool IsValid() const override { return true; }
  void to_str(const semantic::Info&, std::ostream&, size_t) const override {}
};

class ResolverHelper : public ProgramBuilder {
 public:
  ResolverHelper() : td_(std::make_unique<Resolver>(this)) {}

  Resolver* r() const { return td_.get(); }

  ast::Statement* StmtOf(ast::Expression* expr) {
    auto* sem_stmt = Sem().Get(expr)->Stmt();
    return sem_stmt ? sem_stmt->Declaration() : nullptr;
  }

  bool CheckVarUsers(ast::Variable* var,
                     std::vector<ast::Expression*>&& expected_users) {
    auto& var_users = Sem().Get(var)->Users();
    if (var_users.size() != expected_users.size()) {
      return false;
    }
    for (size_t i = 0; i < var_users.size(); i++) {
      if (var_users[i]->Declaration() != expected_users[i]) {
        return false;
      }
    }
    return true;
  }

 private:
  std::unique_ptr<Resolver> td_;
};

class ResolverTest : public ResolverHelper, public testing::Test {};

template <typename T>
class ResolverTestWithParam : public ResolverHelper,
                              public testing::TestWithParam<T> {};

TEST_F(ResolverTest, Error_WithEmptySource) {
  auto* s = create<FakeStmt>();
  WrapInFunction(s);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: unknown statement type for type determination: Fake");
}

TEST_F(ResolverTest, Stmt_Error_Unknown) {
  auto* s = create<FakeStmt>(Source{Source::Location{2, 30}});
  WrapInFunction(s);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "2:30 error: unknown statement type for type determination: Fake");
}

TEST_F(ResolverTest, Stmt_Assign) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Case) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(ty.i32(), 3));
  auto* cse = create<ast::CaseStatement>(lit, body);
  WrapInFunction(cse);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Block) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* block = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  WrapInFunction(block);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Else) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{
      assign,
  });
  auto* cond = Expr(3);
  auto* stmt = create<ast::ElseStatement>(cond, body);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(StmtOf(cond), stmt);
}

TEST_F(ResolverTest, Stmt_If) {
  auto* else_lhs = Expr(2);
  auto* else_rhs = Expr(2.3f);

  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(else_lhs, else_rhs),
  });

  auto* else_cond = Expr(3);
  auto* else_stmt = create<ast::ElseStatement>(else_cond, else_body);

  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  auto* body = create<ast::BlockStatement>(ast::StatementList{assign});
  auto* cond = Expr(3);
  auto* stmt =
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{else_stmt});
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(else_lhs), nullptr);
  ASSERT_NE(TypeOf(else_rhs), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(else_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(else_rhs)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(StmtOf(cond), stmt);
  EXPECT_EQ(StmtOf(else_cond), else_stmt);
}

TEST_F(ResolverTest, Stmt_Loop) {
  auto* body_lhs = Expr(2);
  auto* body_rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(body_lhs, body_rhs),
  });
  auto* continuing_lhs = Expr(2);
  auto* continuing_rhs = Expr(2.3f);

  auto* continuing = create<ast::BlockStatement>(

      ast::StatementList{
          create<ast::AssignmentStatement>(continuing_lhs, continuing_rhs),
      });
  auto* stmt = create<ast::LoopStatement>(body, continuing);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(body_lhs), nullptr);
  ASSERT_NE(TypeOf(body_rhs), nullptr);
  ASSERT_NE(TypeOf(continuing_lhs), nullptr);
  ASSERT_NE(TypeOf(continuing_rhs), nullptr);
  EXPECT_TRUE(TypeOf(body_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(body_rhs)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(continuing_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(continuing_rhs)->Is<type::F32>());
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInContinuing) {
  // loop  {
  //     continue; // Bypasses z decl
  //     var z : i32;
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(create<ast::ContinueStatement>(),
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Assign(Expr(error_loc, "z"), Expr(2)));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDeclAndAfterDecl_UsageInContinuing) {
  // loop  {
  //     continue; // Bypasses z decl
  //     var z : i32;
  //     continue; // Ok
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(create<ast::ContinueStatement>(),
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),
                     create<ast::ContinueStatement>());
  auto* continuing = Block(Assign(Expr(error_loc, "z"), Expr(2)));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuing) {
  // loop  {
  //     if (true) {
  //         continue; // Still bypasses z decl (if we reach here)
  //     }
  //     var z : i32;
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(If(Expr(true), Block(create<ast::ContinueStatement>())),
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Assign(Expr(error_loc, "z"), Expr(2)));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(
    ResolverTest,
    Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingSubscope) {
  // loop  {
  //     if (true) {
  //         continue; // Still bypasses z decl (if we reach here)
  //     }
  //     var z : i32;
  //     continuing {
  //         if (true) {
  //             z = 2; // Must fail even if z is in a sub-scope
  //         }
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(If(Expr(true), Block(create<ast::ContinueStatement>())),
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));

  auto* continuing =
      Block(If(Expr(true), Block(Assign(Expr(error_loc, "z"), Expr(2)))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageInContinuingLoop) {
  // loop  {
  //     if (true) {
  //         continue; // Still bypasses z decl (if we reach here)
  //     }
  //     var z : i32;
  //     continuing {
  //         loop {
  //             z = 2; // Must fail even if z is in a sub-scope
  //         }
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(If(Expr(true), Block(create<ast::ContinueStatement>())),
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));

  auto* continuing = Block(Loop(Block(Assign(Expr(error_loc, "z"), Expr(2)))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuing) {
  // loop  {
  //     loop {
  //         continue; // OK: not part of the outer loop
  //     }
  //     var z : i32;
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto* inner_loop = Loop(Block(create<ast::ContinueStatement>()));
  auto* body =
      Block(inner_loop, Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Assign(Expr("z"), Expr(2)));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingSubscope) {
  // loop  {
  //     loop {
  //         continue; // OK: not part of the outer loop
  //     }
  //     var z : i32;
  //
  //     continuing {
  //         if (true) {
  //             z = 2;
  //         }
  //     }
  // }

  auto* inner_loop = Loop(Block(create<ast::ContinueStatement>()));
  auto* body =
      Block(inner_loop, Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(If(Expr(true), Block(Assign(Expr("z"), Expr(2)))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingLoop) {
  // loop  {
  //     loop {
  //         continue; // OK: not part of the outer loop
  //     }
  //     var z : i32;
  //
  //     continuing {
  //         loop {
  //             z = 2;
  //         }
  //     }
  // }

  auto* inner_loop = Loop(Block(create<ast::ContinueStatement>()));
  auto* body =
      Block(inner_loop, Decl(Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Loop(Block(Assign(Expr("z"), Expr(2)))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ContinueNotInLoop) {
  WrapInFunction(create<ast::ContinueStatement>());
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "error: continue statement must be in a loop");
}

TEST_F(ResolverTest, Stmt_Return) {
  auto* cond = Expr(2);

  auto* ret = create<ast::ReturnStatement>(cond);
  WrapInFunction(ret);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cond), nullptr);
  EXPECT_TRUE(TypeOf(cond)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_Return_WithoutValue) {
  auto* ret = create<ast::ReturnStatement>();
  WrapInFunction(ret);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Switch) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(ty.i32(), 3));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(lit, body));

  auto* stmt = create<ast::SwitchStatement>(Expr(2), cases);
  WrapInFunction(stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(ResolverTest, Stmt_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* expr = Call("my_func");

  auto* call = create<ast::CallStatement>(expr);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
  EXPECT_EQ(StmtOf(expr), call);
}

TEST_F(ResolverTest, Stmt_Call_undeclared) {
  // fn main() -> void {func(); return; }
  // fn func() -> void { return; }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("func");
  ast::VariableList params0;

  Func("main", params0, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{});

  Func("func", params0, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: unable to find called function: func");
}

TEST_F(ResolverTest, Stmt_Call_recursive) {
  // fn main() -> void {main(); }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("main");
  ast::VariableList params0;

  Func("main", params0, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: recursion is not permitted. 'main' attempted to call "
            "itself.");
}

TEST_F(ResolverTest, Stmt_VariableDecl) {
  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = create<ast::VariableDeclStatement>(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_Alias) {
  auto* my_int = ty.alias("MyInt", ty.i32());
  auto* var = Var("my_var", my_int, ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = create<ast::VariableDeclStatement>(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_MismatchedTypeScalarConstructor) {
  u32 unsigned_value = 2u;  // Type does not match variable type
  auto* var =
      Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(unsigned_value));

  auto* decl =
      create<ast::VariableDeclStatement>(Source{{{3, 3}, {3, 22}}}, var);
  WrapInFunction(decl);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: constructor expression type does not match variable type)");
}

TEST_F(ResolverTest, Stmt_VariableDecl_MismatchedTypeScalarConstructor_Alias) {
  auto* my_int = ty.alias("MyInt", ty.i32());
  u32 unsigned_value = 2u;  // Type does not match variable type
  auto* var =
      Var("my_var", my_int, ast::StorageClass::kNone, Expr(unsigned_value));

  auto* decl =
      create<ast::VariableDeclStatement>(Source{{{3, 3}, {3, 22}}}, var);
  WrapInFunction(decl);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:3 error: constructor expression type does not match variable type)");
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScope) {
  auto* init = Expr(2);
  Global("my_var", ty.i32(), ast::StorageClass::kNone, init);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
  EXPECT_EQ(StmtOf(init), nullptr);
}

TEST_F(ResolverTest, Stmt_VariableDecl_OuterScopeAfterInnerScope) {
  // fn func_i32() -> i32 {
  //   {
  //     var foo : i32 = 2;
  //     var bar : i32 = foo;
  //   }
  //   var foo : f32 = 2.0;
  //   var bar : f32 = foo;
  // }

  ast::VariableList params;

  // Declare i32 "foo" inside a block
  auto* foo_i32 = Var("foo", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* foo_i32_init = foo_i32->constructor();
  auto* foo_i32_decl = create<ast::VariableDeclStatement>(foo_i32);

  // Reference "foo" inside the block
  auto* bar_i32 = Var("bar", ty.i32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_i32_init = bar_i32->constructor();
  auto* bar_i32_decl = create<ast::VariableDeclStatement>(bar_i32);

  auto* inner = create<ast::BlockStatement>(
      ast::StatementList{foo_i32_decl, bar_i32_decl});

  // Declare f32 "foo" at function scope
  auto* foo_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* foo_f32_init = foo_f32->constructor();
  auto* foo_f32_decl = create<ast::VariableDeclStatement>(foo_f32);

  // Reference "foo" at function scope
  auto* bar_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_f32_init = bar_f32->constructor();
  auto* bar_f32_decl = create<ast::VariableDeclStatement>(bar_f32);

  Func("func", params, ty.f32(),
       ast::StatementList{inner, foo_f32_decl, bar_f32_decl},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve());
  ASSERT_NE(TypeOf(foo_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_i32_init)->Is<type::I32>());
  ASSERT_NE(TypeOf(foo_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_f32_init)->Is<type::F32>());
  ASSERT_NE(TypeOf(bar_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_i32_init)->UnwrapAll()->Is<type::I32>());
  ASSERT_NE(TypeOf(bar_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_f32_init)->UnwrapAll()->Is<type::F32>());
  EXPECT_EQ(StmtOf(foo_i32_init), foo_i32_decl);
  EXPECT_EQ(StmtOf(bar_i32_init), bar_i32_decl);
  EXPECT_EQ(StmtOf(foo_f32_init), foo_f32_decl);
  EXPECT_EQ(StmtOf(bar_f32_init), bar_f32_decl);
  EXPECT_TRUE(CheckVarUsers(foo_i32, {bar_i32->constructor()}));
  EXPECT_TRUE(CheckVarUsers(foo_f32, {bar_f32->constructor()}));
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScopeAfterFunctionScope) {
  // fn func_i32() -> i32 {
  //   var foo : i32 = 2;
  // }
  // var foo : f32 = 2.0;
  // fn func_f32() -> f32 {
  //   var bar : f32 = foo;
  // }

  ast::VariableList params;

  // Declare i32 "foo" inside a function
  auto* fn_i32 = Var("foo", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* fn_i32_init = fn_i32->constructor();
  auto* fn_i32_decl = create<ast::VariableDeclStatement>(fn_i32);
  Func("func_i32", params, ty.i32(), ast::StatementList{fn_i32_decl},
       ast::FunctionDecorationList{});

  // Declare f32 "foo" at module scope
  auto* mod_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* mod_init = mod_f32->constructor();
  AST().AddGlobalVariable(mod_f32);

  // Reference "foo" in another function
  auto* fn_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* fn_f32_init = fn_f32->constructor();
  auto* fn_f32_decl = create<ast::VariableDeclStatement>(fn_f32);
  Func("func_f32", params, ty.f32(), ast::StatementList{fn_f32_decl},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve());
  ASSERT_NE(TypeOf(mod_init), nullptr);
  EXPECT_TRUE(TypeOf(mod_init)->Is<type::F32>());
  ASSERT_NE(TypeOf(fn_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_i32_init)->Is<type::I32>());
  ASSERT_NE(TypeOf(fn_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_f32_init)->UnwrapAll()->Is<type::F32>());
  EXPECT_EQ(StmtOf(fn_i32_init), fn_i32_decl);
  EXPECT_EQ(StmtOf(mod_init), nullptr);
  EXPECT_EQ(StmtOf(fn_f32_init), fn_f32_decl);
  EXPECT_TRUE(CheckVarUsers(fn_i32, {}));
  EXPECT_TRUE(CheckVarUsers(mod_f32, {fn_f32->constructor()}));
}

TEST_F(ResolverTest, Expr_Error_Unknown) {
  FakeExpr e(Source{Source::Location{2, 30}});
  WrapInFunction(&e);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "2:30 error: unknown expression for type determination");
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array) {
  auto* idx = Expr(2);
  Global("my_var", ty.array<f32, 3>(), ast::StorageClass::kFunction);

  auto* acc = IndexAccessor("my_var", idx);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Alias_Array) {
  auto* aary = ty.alias("myarrty", ty.array<f32, 3>());

  Global("my_var", aary, ast::StorageClass::kFunction);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array_Constant) {
  GlobalConst("my_var", ty.array<f32, 3>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  EXPECT_TRUE(TypeOf(acc)->Is<type::F32>()) << TypeOf(acc)->type_name();
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::Vector>());
  EXPECT_EQ(ptr->type()->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor(IndexAccessor("my_var", 2), 1);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Vector) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Bitcast) {
  auto* bitcast = create<ast::BitcastExpression>(ty.f32(), Expr("name"));
  WrapInFunction(bitcast);

  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(bitcast), nullptr);
  EXPECT_TRUE(TypeOf(bitcast)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_InBinaryOp) {
  ast::VariableList params;
  Func("func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* expr = Add(Call("func"), Call("func"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_WithParams) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* param = Expr(2.4f);

  auto* call = Call("my_func", param);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(param), nullptr);
  EXPECT_TRUE(TypeOf(param)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Call_Intrinsic) {
  auto* call = Call("round", 2.4f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_DontCall_Function) {
  Func("func", {}, ty.void_(), {}, {});
  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("func"));
  WrapInFunction(ident);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for function call");
}

TEST_F(ResolverTest, Expr_DontCall_Intrinsic) {
  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("round"));
  WrapInFunction(ident);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for intrinsic call");
}

TEST_F(ResolverTest, Expr_Cast) {
  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  auto* cast = Construct(ty.f32(), "name");
  WrapInFunction(cast);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cast), nullptr);
  EXPECT_TRUE(TypeOf(cast)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Scalar) {
  auto* s = Expr(1.0f);
  WrapInFunction(s);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(s), nullptr);
  EXPECT_TRUE(TypeOf(s)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Type) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 3.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(tc)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalVariable) {
  auto* my_var = Global("my_var", ty.f32(), ast::StorageClass::kNone);

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(ident)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
}

TEST_F(ResolverTest, Expr_Identifier_GlobalConstant) {
  auto* my_var = GlobalConst("my_var", ty.f32());

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable_Const) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* var = Const("my_var", ty.f32());
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a, my_var_b}));
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  auto* var = Var("my_var", ty.f32(), ast::StorageClass::kNone);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_a)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_b)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a, my_var_b}));
}

TEST_F(ResolverTest, Expr_Identifier_Function_Ptr) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = create<ast::AssignmentStatement>(my_var_a, my_var_b);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("my_var", ty.pointer<f32>(ast::StorageClass::kFunction),
                   ast::StorageClass::kNone)),
           assign,
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_a)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  EXPECT_TRUE(TypeOf(my_var_b)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var_b)->As<type::Pointer>()->type()->Is<type::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
}

TEST_F(ResolverTest, Expr_Call_Function) {
  Func("my_func", ast::VariableList{}, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_Identifier_Unknown) {
  auto* a = Expr("a");
  WrapInFunction(a);

  EXPECT_FALSE(r()->Resolve());
}

TEST_F(ResolverTest, UsingUndefinedVariable_Fail) {
  // b = 2;

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("b");
  auto* rhs = Expr(2);
  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: b");
}

TEST_F(ResolverTest, UsingUndefinedVariableInBlockStatement_Fail) {
  // {
  //  b = 2;
  // }

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr("b");
  auto* rhs = Expr(2);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  WrapInFunction(body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: b");
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables) {
  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", ty.f32(), ast::StorageClass::kStorage);
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  auto* func = Func(
      "my_func", ast::VariableList{}, ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("out_var"), Expr("in_var")),
          create<ast::AssignmentStatement>(Expr("wg_var"), Expr("wg_var")),
          create<ast::AssignmentStatement>(Expr("sb_var"), Expr("sb_var")),
          create<ast::AssignmentStatement>(Expr("priv_var"), Expr("priv_var")),
      },
      ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  const auto& vars = func_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables_SubFunction) {
  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", ty.f32(), ast::StorageClass::kStorage);
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("out_var"), Expr("in_var")),
           create<ast::AssignmentStatement>(Expr("wg_var"), Expr("wg_var")),
           create<ast::AssignmentStatement>(Expr("sb_var"), Expr("sb_var")),
           create<ast::AssignmentStatement>(Expr("priv_var"), Expr("priv_var")),
       },
       ast::FunctionDecorationList{});

  auto* func2 = Func(
      "func", ast::VariableList{}, ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("out_var"), Call("my_func")),
      },
      ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func2_sem = Sem().Get(func2);
  ASSERT_NE(func2_sem, nullptr);

  const auto& vars = func2_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_NotRegisterFunctionVariable) {
  auto* var = Var("in_var", ty.f32(), ast::StorageClass::kFunction);

  auto* func =
      Func("my_func", ast::VariableList{}, ty.f32(),
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::AssignmentStatement>(Expr("var"), Expr(1.f)),
           },
           ast::FunctionDecorationList{});

  Global("var", ty.f32(), ast::StorageClass::kFunction);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->ReferencedModuleVariables().size(), 0u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("S", strct);
  Global("my_struct", st, ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct_Alias) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("alias", strct);
  auto* alias = ty.alias("alias", st);
  Global("my_struct", alias, ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_vec", "xzyw");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 4u);
  EXPECT_THAT(Sem().Get(mem)->Swizzle(), ElementsAre(0, 2, 1, 3));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* mem = MemberAccessor("my_vec", "b");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::F32>());
  EXPECT_THAT(Sem().Get(mem)->Swizzle(), ElementsAre(2));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 7}}},
      Symbols().Register("xyqz"));

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:5 error: invalid vector swizzle character");
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 7}}},
      Symbols().Register("rgyw"));

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "3:3 error: invalid mixing of vector swizzle characters rgba with xyzw");
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_BadLength) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("zzzzz"));
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle size");
}

TEST_F(ResolverTest, Expr_Accessor_MultiLevel) {
  // struct b {
  //   vec4<f32> foo
  // }
  // struct A {
  //   vec3<struct b> mem
  // }
  // var c : A
  // c.mem[0].foo.yx
  //   -> vec2<f32>
  //
  // MemberAccessor{
  //   MemberAccessor{
  //     ArrayAccessor{
  //       MemberAccessor{
  //         Identifier{c}
  //         Identifier{mem}
  //       }
  //       ScalarConstructor{0}
  //     }
  //     Identifier{foo}
  //   }
  //   Identifier{yx}
  // }
  //

  auto* strctB =
      create<ast::Struct>(ast::StructMemberList{Member("foo", ty.vec4<f32>())},
                          ast::StructDecorationList{});
  auto* stB = ty.struct_("B", strctB);

  type::Vector vecB(stB, 3);
  auto* strctA = create<ast::Struct>(
      ast::StructMemberList{Member("mem", &vecB)}, ast::StructDecorationList{});

  auto* stA = ty.struct_("A", strctA);
  Global("c", stA, ast::StorageClass::kNone);

  auto* mem = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0), "foo"),
      "yx");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 2u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_InBinaryOp) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.f32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("S", strct);
  Global("my_struct", st, ast::StorageClass::kNone);

  auto* expr = Add(MemberAccessor("my_struct", "first_member"),
                   MemberAccessor("my_struct", "second_member"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

using Expr_Binary_BitwiseTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_BitwiseTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_P(Expr_Binary_BitwiseTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<i32>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_BitwiseTest,
                         testing::Values(ast::BinaryOp::kAnd,
                                         ast::BinaryOp::kOr,
                                         ast::BinaryOp::kXor,
                                         ast::BinaryOp::kShiftLeft,
                                         ast::BinaryOp::kShiftRight,
                                         ast::BinaryOp::kAdd,
                                         ast::BinaryOp::kSubtract,
                                         ast::BinaryOp::kDivide,
                                         ast::BinaryOp::kModulo));

using Expr_Binary_LogicalTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_LogicalTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.bool_(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_LogicalTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<bool>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_LogicalTest,
                         testing::Values(ast::BinaryOp::kLogicalAnd,
                                         ast::BinaryOp::kLogicalOr));

using Expr_Binary_CompareTest = ResolverTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_CompareTest, Scalar) {
  auto op = GetParam();

  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_CompareTest, Vector) {
  auto op = GetParam();

  Global("val", ty.vec3<i32>(), ast::StorageClass::kNone);

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_CompareTest,
                         testing::Values(ast::BinaryOp::kEqual,
                                         ast::BinaryOp::kNotEqual,
                                         ast::BinaryOp::kLessThan,
                                         ast::BinaryOp::kGreaterThan,
                                         ast::BinaryOp::kLessThanEqual,
                                         ast::BinaryOp::kGreaterThanEqual));

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Scalar) {
  Global("val", ty.i32(), ast::StorageClass::kNone);

  auto* expr = Mul("val", "val");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Scalar) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Vector) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("scalar", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Vector) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Scalar) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("matrix", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Scalar_Matrix) {
  Global("scalar", ty.f32(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("scalar", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Vector) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("matrix", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Vector_Matrix) {
  Global("vector", ty.vec3<f32>(), ast::StorageClass::kNone);
  Global("matrix", ty.mat2x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("vector", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 2u);
}

TEST_F(ResolverTest, Expr_Binary_Multiply_Matrix_Matrix) {
  Global("mat3x4", ty.mat3x4<f32>(), ast::StorageClass::kNone);
  Global("mat4x3", ty.mat4x3<f32>(), ast::StorageClass::kNone);

  auto* expr = Mul("mat3x4", "mat4x3");
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 4u);
  EXPECT_EQ(mat->columns(), 4u);
}

using IntrinsicDerivativeTest = ResolverTestWithParam<std::string>;
TEST_P(IntrinsicDerivativeTest, Scalar) {
  auto name = GetParam();

  Global("ident", ty.f32(), ast::StorageClass::kNone);

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_P(IntrinsicDerivativeTest, Vector) {
  auto name = GetParam();
  Global("ident", ty.vec4<f32>(), ast::StorageClass::kNone);

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 4u);
}

TEST_P(IntrinsicDerivativeTest, MissingParam) {
  auto name = GetParam();

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> f32\n  " + name +
                              "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         IntrinsicDerivativeTest,
                         testing::Values("dpdx",
                                         "dpdxCoarse",
                                         "dpdxFine",
                                         "dpdy",
                                         "dpdyCoarse",
                                         "dpdyFine",
                                         "fwidth",
                                         "fwidthCoarse",
                                         "fwidthFine"));

using Intrinsic = ResolverTestWithParam<std::string>;
TEST_P(Intrinsic, Test) {
  auto name = GetParam();

  Global("my_var", ty.vec3<bool>(), ast::StorageClass::kNone);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Intrinsic,
                         testing::Values("any", "all"));

using Intrinsic_FloatMethod = ResolverTestWithParam<std::string>;
TEST_P(Intrinsic_FloatMethod, Vector) {
  auto name = GetParam();

  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatMethod, Scalar) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kNone);

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Intrinsic_FloatMethod, MissingParam) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kNone);

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> bool\n  " + name +
                              "(vecN<f32>) -> vecN<bool>\n");
}

TEST_P(Intrinsic_FloatMethod, TooManyParams) {
  auto name = GetParam();

  Global("my_var", ty.f32(), ast::StorageClass::kNone);

  auto* expr = Call(name, "my_var", 1.23f);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " + name +
                              "(ptr<f32>, f32)\n\n"
                              "2 candidate functions:\n  " +
                              name + "(f32) -> bool\n  " + name +
                              "(vecN<f32>) -> vecN<bool>\n");
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_FloatMethod,
    testing::Values("isInf", "isNan", "isFinite", "isNormal"));

enum class Texture { kF32, kI32, kU32 };
inline std::ostream& operator<<(std::ostream& out, Texture data) {
  if (data == Texture::kF32) {
    out << "f32";
  } else if (data == Texture::kI32) {
    out << "i32";
  } else {
    out << "u32";
  }
  return out;
}

struct TextureTestParams {
  type::TextureDimension dim;
  Texture type = Texture::kF32;
  type::ImageFormat format = type::ImageFormat::kR16Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
  out << data.dim << "_" << data.type;
  return out;
}

class Intrinsic_TextureOperation
    : public ResolverTestWithParam<TextureTestParams> {
 public:
  /// Gets an appropriate type for the coords parameter depending the the
  /// dimensionality of the texture being sampled.
  /// @param dim dimensionality of the texture being sampled
  /// @param scalar the scalar type
  /// @returns a pointer to a type appropriate for the coord param
  type::Type* GetCoordsType(type::TextureDimension dim, type::Type* scalar) {
    switch (dim) {
      case type::TextureDimension::k1d:
        return scalar;
      case type::TextureDimension::k2d:
      case type::TextureDimension::k2dArray:
        return create<type::Vector>(scalar, 2);
      case type::TextureDimension::k3d:
      case type::TextureDimension::kCube:
      case type::TextureDimension::kCubeArray:
        return create<type::Vector>(scalar, 3);
      default:
        [=]() { FAIL() << "Unsupported texture dimension: " << dim; }();
    }
    return nullptr;
  }

  void add_call_param(std::string name,
                      type::Type* type,
                      ast::ExpressionList* call_params) {
    Global(name, type, ast::StorageClass::kNone);
    call_params->push_back(Expr(name));
  }
  type::Type* subtype(Texture type) {
    if (type == Texture::kF32) {
      return create<type::F32>();
    }
    if (type == Texture::kI32) {
      return create<type::I32>();
    }
    return create<type::U32>();
  }
};

using Intrinsic_StorageTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_StorageTextureOperation, TextureLoadRo) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  auto* coords_type = GetCoordsType(dim, ty.i32());

  auto* subtype = type::StorageTexture::SubtypeFor(format, Types());
  auto* texture_type = create<type::StorageTexture>(dim, format, subtype);
  auto* ro_texture_type =
      create<type::AccessControl>(ast::AccessControl::kReadOnly, texture_type);

  ast::ExpressionList call_params;

  add_call_param("texture", ro_texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);

  if (type::IsTextureArray(dim)) {
    add_call_param("array_index", ty.i32(), &call_params);
  }

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
  } else {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::U32>());
  }
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_StorageTextureOperation,
    testing::Values(
        TextureTestParams{type::TextureDimension::k1d, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k1d, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k1d, Texture::kF32,
                          type::ImageFormat::kR8Unorm},
        TextureTestParams{type::TextureDimension::k2d, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k2d, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k2d, Texture::kF32,
                          type::ImageFormat::kR8Unorm},
        TextureTestParams{type::TextureDimension::k2dArray, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k2dArray, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k2dArray, Texture::kF32,
                          type::ImageFormat::kR8Unorm},
        TextureTestParams{type::TextureDimension::k3d, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k3d, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k3d, Texture::kF32,
                          type::ImageFormat::kR8Unorm}));

using Intrinsic_SampledTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_SampledTextureOperation, TextureLoadSampled) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  type::Type* s = subtype(type);
  auto* coords_type = GetCoordsType(dim, ty.i32());
  auto* texture_type = create<type::SampledTexture>(dim, s);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);
  if (dim == type::TextureDimension::k2dArray) {
    add_call_param("array_index", ty.i32(), &call_params);
  }
  add_call_param("level", ty.i32(), &call_params);

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
  } else {
    EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::U32>());
  }
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_SampledTextureOperation,
    testing::Values(TextureTestParams{type::TextureDimension::k1d},
                    TextureTestParams{type::TextureDimension::k2d},
                    TextureTestParams{type::TextureDimension::k2dArray},
                    TextureTestParams{type::TextureDimension::k3d}));

TEST_F(ResolverTest, Intrinsic_Dot_Vec2) {
  Global("my_var", ty.vec2<f32>(), ast::StorageClass::kNone);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Dot_Vec3) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Dot_Vec4) {
  Global("my_var", ty.vec4<f32>(), ast::StorageClass::kNone);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Dot_Error_Scalar) {
  auto* expr = Call("dot", 1.0f, 1.0f);
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to dot(f32, f32)

1 candidate function:
  dot(vecN<f32>, vecN<f32>) -> f32
)");
}

TEST_F(ResolverTest, Intrinsic_Dot_Error_VectorInt) {
  Global("my_var", ty.vec4<i32>(), ast::StorageClass::kNone);

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to dot(ptr<vec4<i32>>, ptr<vec4<i32>>)

1 candidate function:
  dot(vecN<f32>, vecN<f32>) -> f32
)");
}

TEST_F(ResolverTest, Intrinsic_Select) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kNone);

  Global("bool_var", ty.vec3<bool>(), ast::StorageClass::kNone);

  auto* expr = Call("select", "my_var", "my_var", "bool_var");
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Select_Error_NoParams) {
  auto* expr = Call("select");
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select()

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverTest, Intrinsic_Select_Error_SelectorInt) {
  auto* expr = Call("select", Expr(1), Expr(1), Expr(1));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(i32, i32, i32)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverTest, Intrinsic_Select_Error_Matrix) {
  auto* mat = mat2x2<float>(vec2<float>(1.0f, 1.0f), vec2<float>(1.0f, 1.0f));
  auto* expr = Call("select", mat, mat, Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(mat2x2<f32>, mat2x2<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverTest, Intrinsic_Select_Error_MismatchTypes) {
  auto* expr = Call("select", 1.0f, vec2<float>(2.0f, 3.0f), Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(f32, vec2<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

TEST_F(ResolverTest, Intrinsic_Select_Error_MismatchVectorSize) {
  auto* expr = Call("select", vec2<float>(1.0f, 2.0f),
                    vec3<float>(3.0f, 4.0f, 5.0f), Expr(true));
  WrapInFunction(expr);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to select(vec2<f32>, vec3<f32>, bool)

2 candidate functions:
  select(T, T, bool) -> T  where: T is scalar
  select(vecN<T>, vecN<T>, vecN<bool>) -> vecN<T>  where: T is scalar
)");
}

using UnaryOpExpressionTest = ResolverTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  Global("ident", ty.vec4<f32>(), ast::StorageClass::kNone);
  auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
  WrapInFunction(der);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(der), nullptr);
  ASSERT_TRUE(TypeOf(der)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(der)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(der)->As<type::Vector>()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(ResolverTest, StorageClass_SetsIfMissing) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kNone);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kFunction);
}

TEST_F(ResolverTest, StorageClass_DoesNotSetOnConst) {
  auto* var = Const("var", ty.i32());
  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kNone);
}

TEST_F(ResolverTest, StorageClass_NonFunctionClassError) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kWorkgroup);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

struct IntrinsicData {
  const char* name;
  IntrinsicType intrinsic;
};

inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}

using Intrinsic_DataPackingTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_DataPackingTest, InferType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f))
                     : Call(param.name, vec2<f32>(1.f, 2.f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(Intrinsic_DataPackingTest, Error_IncorrectParamType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<i32>(1, 2, 3, 4))
                     : Call(param.name, vec2<i32>(1, 2));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

TEST_P(Intrinsic_DataPackingTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

TEST_P(Intrinsic_DataPackingTest, Error_TooManyParams) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kPack4x8Snorm ||
               param.intrinsic == IntrinsicType::kPack4x8Unorm;

  auto* call = pack4 ? Call(param.name, vec4<f32>(1.f, 2.f, 3.f, 4.f), 1.0f)
                     : Call(param.name, vec2<f32>(1.f, 2.f), 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_THAT(r()->error(), HasSubstr("error: no matching call to " +
                                      std::string(param.name)));
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_DataPackingTest,
    testing::Values(
        IntrinsicData{"pack4x8snorm", IntrinsicType::kPack4x8Snorm},
        IntrinsicData{"pack4x8unorm", IntrinsicType::kPack4x8Unorm},
        IntrinsicData{"pack2x16snorm", IntrinsicType::kPack2x16Snorm},
        IntrinsicData{"pack2x16unorm", IntrinsicType::kPack2x16Unorm},
        IntrinsicData{"pack2x16float", IntrinsicType::kPack2x16Float}));

using Intrinsic_DataUnpackingTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_DataUnpackingTest, InferType) {
  auto param = GetParam();

  bool pack4 = param.intrinsic == IntrinsicType::kUnpack4x8Snorm ||
               param.intrinsic == IntrinsicType::kUnpack4x8Unorm;

  auto* call = Call(param.name, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  if (pack4) {
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 4u);
  } else {
    EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 2u);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_DataUnpackingTest,
    testing::Values(
        IntrinsicData{"unpack4x8snorm", IntrinsicType::kUnpack4x8Snorm},
        IntrinsicData{"unpack4x8unorm", IntrinsicType::kUnpack4x8Unorm},
        IntrinsicData{"unpack2x16snorm", IntrinsicType::kUnpack2x16Snorm},
        IntrinsicData{"unpack2x16unorm", IntrinsicType::kUnpack2x16Unorm},
        IntrinsicData{"unpack2x16float", IntrinsicType::kUnpack2x16Float}));

using Intrinsic_SingleParamTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_SingleParamTest, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(Intrinsic_SingleParamTest, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32) -> f32\n  " +
                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

TEST_P(Intrinsic_SingleParamTest, Error_TooManyParams) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 2, 3);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "(i32, i32, i32)\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32) -> f32\n  " +
                std::string(param.name) + "(vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_SingleParamTest,
    testing::Values(IntrinsicData{"acos", IntrinsicType::kAcos},
                    IntrinsicData{"asin", IntrinsicType::kAsin},
                    IntrinsicData{"atan", IntrinsicType::kAtan},
                    IntrinsicData{"ceil", IntrinsicType::kCeil},
                    IntrinsicData{"cos", IntrinsicType::kCos},
                    IntrinsicData{"cosh", IntrinsicType::kCosh},
                    IntrinsicData{"exp", IntrinsicType::kExp},
                    IntrinsicData{"exp2", IntrinsicType::kExp2},
                    IntrinsicData{"floor", IntrinsicType::kFloor},
                    IntrinsicData{"fract", IntrinsicType::kFract},
                    IntrinsicData{"inverseSqrt", IntrinsicType::kInverseSqrt},
                    IntrinsicData{"log", IntrinsicType::kLog},
                    IntrinsicData{"log2", IntrinsicType::kLog2},
                    IntrinsicData{"round", IntrinsicType::kRound},
                    IntrinsicData{"sign", IntrinsicType::kSign},
                    IntrinsicData{"sin", IntrinsicType::kSin},
                    IntrinsicData{"sinh", IntrinsicType::kSinh},
                    IntrinsicData{"sqrt", IntrinsicType::kSqrt},
                    IntrinsicData{"tan", IntrinsicType::kTan},
                    IntrinsicData{"tanh", IntrinsicType::kTanh},
                    IntrinsicData{"trunc", IntrinsicType::kTrunc}));

using IntrinsicDataTest = ResolverTest;

TEST_F(IntrinsicDataTest, ArrayLength_Vector) {
  Global("arr", ty.array<int>(), ast::StorageClass::kNone);
  auto* call = Call("arrayLength", "arr");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_F(IntrinsicDataTest, ArrayLength_Error_ArraySized) {
  Global("arr", ty.array<int, 4>(), ast::StorageClass::kNone);
  auto* call = Call("arrayLength", "arr");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to arrayLength(ptr<array<i32, 4>>)\n\n"
            "1 candidate function:\n"
            "  arrayLength(array<T>) -> u32\n");
}

TEST_F(IntrinsicDataTest, Normalize_Vector) {
  auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_F(IntrinsicDataTest, Normalize_Error_NoParams) {
  auto* call = Call("normalize");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to normalize()\n\n"
            "1 candidate function:\n"
            "  normalize(vecN<f32>) -> vecN<f32>\n");
}

TEST_F(IntrinsicDataTest, FrexpScalar) {
  Global("exp", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1.0f, "exp");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(IntrinsicDataTest, FrexpVector) {
  Global("exp", ty.vec3<i32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", vec3<f32>(1.0f, 2.0f, 3.0f), "exp");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(IntrinsicDataTest, Frexp_Error_FirstParamInt) {
  Global("exp", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1, "exp");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(i32, ptr<workgroup, i32>)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(IntrinsicDataTest, Frexp_Error_SecondParamFloatPtr) {
  Global("exp", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", 1.0f, "exp");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(f32, ptr<workgroup, f32>)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(IntrinsicDataTest, Frexp_Error_SecondParamNotAPointer) {
  auto* call = Call("frexp", 1.0f, 1);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(f32, i32)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(IntrinsicDataTest, Frexp_Error_VectorSizesDontMatch) {
  Global("exp", ty.vec4<i32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("frexp", vec2<f32>(1.0f, 2.0f), "exp");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to frexp(vec2<f32>, ptr<workgroup, "
            "vec4<i32>>)\n\n"
            "2 candidate functions:\n"
            "  frexp(f32, ptr<T>) -> f32  where: T is i32 or u32\n"
            "  frexp(vecN<f32>, ptr<vecN<T>>) -> vecN<f32>  "
            "where: T is i32 or u32\n");
}

TEST_F(IntrinsicDataTest, ModfScalar) {
  Global("whole", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1.0f, "whole");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(IntrinsicDataTest, ModfVector) {
  Global("whole", ty.vec3<f32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", vec3<f32>(1.0f, 2.0f, 3.0f), "whole");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(call)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(IntrinsicDataTest, Modf_Error_FirstParamInt) {
  Global("whole", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1, "whole");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(i32, ptr<workgroup, f32>)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(IntrinsicDataTest, Modf_Error_SecondParamIntPtr) {
  Global("whole", ty.i32(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", 1.0f, "whole");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(f32, ptr<workgroup, i32>)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(IntrinsicDataTest, Modf_Error_SecondParamNotAPointer) {
  auto* call = Call("modf", 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(f32, f32)\n\n"
            "2 candidate functions:\n"
            "  modf(f32, ptr<f32>) -> f32\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n");
}

TEST_F(IntrinsicDataTest, Modf_Error_VectorSizesDontMatch) {
  Global("whole", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  auto* call = Call("modf", vec2<f32>(1.0f, 2.0f), "whole");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to modf(vec2<f32>, ptr<workgroup, "
            "vec4<f32>>)\n\n"
            "2 candidate functions:\n"
            "  modf(vecN<f32>, ptr<vecN<f32>>) -> vecN<f32>\n"
            "  modf(f32, ptr<f32>) -> f32\n");
}

using Intrinsic_SingleParam_FloatOrInt_Test =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, -1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  ast::ExpressionList vals;
  vals.push_back(Expr(1));
  vals.push_back(Expr(1));
  vals.push_back(Expr(3));

  ast::ExpressionList params;
  params.push_back(vec3<i32>(vals));

  auto* call = Call(param.name, params);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  ast::ExpressionList params;
  params.push_back(Expr(1u));

  auto* call = Call(param.name, params);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_SingleParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Intrinsic_SingleParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{"abs",
                                                       IntrinsicType::kAbs}));

TEST_F(ResolverTest, Intrinsic_Length_Scalar) {
  auto* call = Call("length", 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverTest, Intrinsic_Length_FloatVector) {
  ast::ExpressionList params;
  params.push_back(vec3<f32>(1.0f, 1.0f, 3.0f));

  auto* call = Call("length", params);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

using Intrinsic_TwoParamTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_TwoParamTest, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(Intrinsic_TwoParamTest, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_TwoParamTest, Error_NoTooManyParams) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 2, 3);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "(i32, i32, i32)\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

TEST_P(Intrinsic_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_TwoParamTest,
    testing::Values(IntrinsicData{"atan2", IntrinsicType::kAtan2},
                    IntrinsicData{"pow", IntrinsicType::kPow},
                    IntrinsicData{"step", IntrinsicType::kStep},
                    IntrinsicData{"reflect", IntrinsicType::kReflect}));

TEST_F(ResolverTest, Intrinsic_Distance_Scalar) {
  auto* call = Call("distance", 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(ResolverTest, Intrinsic_Distance_Vector) {
  auto* call = Call("distance", vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Cross) {
  auto* call =
      Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Intrinsic_Cross_Error_NoArgs) {
  auto* call = Call("cross");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to cross()

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverTest, Intrinsic_Cross_Error_Scalar) {
  auto* call = Call("cross", 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to cross(f32, f32)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverTest, Intrinsic_Cross_Error_Vec3Int) {
  auto* call = Call("cross", vec3<i32>(1, 2, 3), vec3<i32>(1, 2, 3));
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec3<i32>, vec3<i32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverTest, Intrinsic_Cross_Error_Vec4) {
  auto* call = Call("cross", vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f),
                    vec4<f32>(1.0f, 2.0f, 3.0f, 4.0f));

  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec4<f32>, vec4<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}

TEST_F(ResolverTest, Intrinsic_Cross_Error_TooManyParams) {
  auto* call = Call("cross", vec3<f32>(1.0f, 2.0f, 3.0f),
                    vec3<f32>(1.0f, 2.0f, 3.0f), vec3<f32>(1.0f, 2.0f, 3.0f));

  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            R"(error: no matching call to cross(vec3<f32>, vec3<f32>, vec3<f32>)

1 candidate function:
  cross(vec3<f32>, vec3<f32>) -> vec3<f32>
)");
}
TEST_F(ResolverTest, Intrinsic_Normalize) {
  auto* call = Call("normalize", vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Intrinsic_Normalize_NoArgs) {
  auto* call = Call("normalize");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), R"(error: no matching call to normalize()

1 candidate function:
  normalize(vecN<f32>) -> vecN<f32>
)");
}

using Intrinsic_ThreeParamTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_ThreeParamTest, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(Intrinsic_ThreeParamTest, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}
TEST_P(Intrinsic_ThreeParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) + "(f32, f32, f32) -> f32\n  " +
                std::string(param.name) +
                "(vecN<f32>, vecN<f32>, vecN<f32>) -> vecN<f32>\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_ThreeParamTest,
    testing::Values(IntrinsicData{"mix", IntrinsicType::kMix},
                    IntrinsicData{"smoothStep", IntrinsicType::kSmoothStep},
                    IntrinsicData{"fma", IntrinsicType::kFma},
                    IntrinsicData{"faceForward", IntrinsicType::kFaceForward}));

using Intrinsic_ThreeParam_FloatOrInt_Test =
    ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3),
                    vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1u, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u),
                    vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_ThreeParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T, T, T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>, vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 "
                "or u32\n");
}

INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Intrinsic_ThreeParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{"clamp",
                                                       IntrinsicType::kClamp}));

using Intrinsic_Int_SingleParamTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_Int_SingleParamTest, Scalar) {
  auto param = GetParam();

  auto* call = Call(param.name, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_integer_scalar());
}

TEST_P(Intrinsic_Int_SingleParamTest, Vector) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_Int_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(), "error: no matching call to " +
                              std::string(param.name) +
                              "()\n\n"
                              "2 candidate functions:\n  " +
                              std::string(param.name) +
                              "(T) -> T  where: T is i32 or u32\n  " +
                              std::string(param.name) +
                              "(vecN<T>) -> vecN<T>  where: T is i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_Int_SingleParamTest,
    testing::Values(IntrinsicData{"countOneBits", IntrinsicType::kCountOneBits},
                    IntrinsicData{"reverseBits", IntrinsicType::kReverseBits}));

using Intrinsic_FloatOrInt_TwoParamTest = ResolverTestWithParam<IntrinsicData>;
TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Scalar_Signed) {
  auto param = GetParam();

  auto* call = Call(param.name, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Scalar_Unsigned) {
  auto param = GetParam();

  auto* call = Call(param.name, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Scalar_Float) {
  auto param = GetParam();

  auto* call = Call(param.name, 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Vector_Signed) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Vector_Unsigned) {
  auto param = GetParam();

  auto* call = Call(param.name, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Vector_Float) {
  auto param = GetParam();

  auto* call =
      Call(param.name, vec3<f32>(1.f, 1.f, 3.f), vec3<f32>(1.f, 1.f, 3.f));
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatOrInt_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to " + std::string(param.name) +
                "()\n\n"
                "2 candidate functions:\n  " +
                std::string(param.name) +
                "(T, T) -> T  where: T is f32, i32 or u32\n  " +
                std::string(param.name) +
                "(vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32\n");
}

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Intrinsic_FloatOrInt_TwoParamTest,
    testing::Values(IntrinsicData{"min", IntrinsicType::kMin},
                    IntrinsicData{"max", IntrinsicType::kMax}));

TEST_F(ResolverTest, Intrinsic_Determinant_2x2) {
  Global("var", ty.mat2x2<f32>(), ast::StorageClass::kFunction);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Determinant_3x3) {
  Global("var", ty.mat3x3<f32>(), ast::StorageClass::kFunction);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Determinant_4x4) {
  Global("var", ty.mat4x4<f32>(), ast::StorageClass::kFunction);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(ResolverTest, Intrinsic_Determinant_NotSquare) {
  Global("var", ty.mat2x3<f32>(), ast::StorageClass::kFunction);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      "error: no matching call to determinant(ptr<function, mat2x3<f32>>)\n\n"
      "1 candidate function:\n"
      "  determinant(matNxN<f32>) -> f32\n");
}

TEST_F(ResolverTest, Intrinsic_Determinant_NotMatrix) {
  Global("var", ty.f32(), ast::StorageClass::kFunction);

  auto* call = Call("determinant", "var");
  WrapInFunction(call);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: no matching call to determinant(ptr<function, f32>)\n\n"
            "1 candidate function:\n"
            "  determinant(matNxN<f32>) -> f32\n");
}

TEST_F(ResolverTest, Function_EntryPoints_StageDecoration) {
  // fn b() {}
  // fn c() { b(); }
  // fn a() { c(); }
  // fn ep_1() { a(); b(); }
  // fn ep_2() { c();}
  //
  // c -> {ep_1, ep_2}
  // a -> {ep_1}
  // b -> {ep_1, ep_2}
  // ep_1 -> {}
  // ep_2 -> {}

  ast::VariableList params;
  auto* func_b = Func("b", params, ty.f32(), ast::StatementList{},
                      ast::FunctionDecorationList{});
  auto* func_c =
      Func("c", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("second"), Call("b")),
           },
           ast::FunctionDecorationList{});

  auto* func_a =
      Func("a", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("first"), Call("c")),
           },
           ast::FunctionDecorationList{});

  auto* ep_1 =
      Func("ep_1", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("call_a"), Call("a")),
               create<ast::AssignmentStatement>(Expr("call_b"), Call("b")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  auto* ep_2 =
      Func("ep_2", params, ty.f32(),
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("call_c"), Call("c")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  Global("first", ty.f32(), ast::StorageClass::kPrivate);
  Global("second", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_a", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_b", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_c", ty.f32(), ast::StorageClass::kPrivate);

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  auto* func_b_sem = Sem().Get(func_b);
  auto* func_a_sem = Sem().Get(func_a);
  auto* func_c_sem = Sem().Get(func_c);
  auto* ep_1_sem = Sem().Get(ep_1);
  auto* ep_2_sem = Sem().Get(ep_2);
  ASSERT_NE(func_b_sem, nullptr);
  ASSERT_NE(func_a_sem, nullptr);
  ASSERT_NE(func_c_sem, nullptr);
  ASSERT_NE(ep_1_sem, nullptr);
  ASSERT_NE(ep_2_sem, nullptr);

  const auto& b_eps = func_b_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, b_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), b_eps[0]);
  EXPECT_EQ(Symbols().Register("ep_2"), b_eps[1]);

  const auto& a_eps = func_a_sem->AncestorEntryPoints();
  ASSERT_EQ(1u, a_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), a_eps[0]);

  const auto& c_eps = func_c_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, c_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), c_eps[0]);
  EXPECT_EQ(Symbols().Register("ep_2"), c_eps[1]);

  EXPECT_TRUE(ep_1_sem->AncestorEntryPoints().empty());
  EXPECT_TRUE(ep_2_sem->AncestorEntryPoints().empty());
}

// Check for linear-time traversal of functions reachable from entry points.
// See: crbug.com/tint/245
TEST_F(ResolverTest, Function_EntryPoints_LinearTime) {
  // fn lNa() { }
  // fn lNb() { }
  // ...
  // fn l2a() { l3a(); l3b(); }
  // fn l2b() { l3a(); l3b(); }
  // fn l1a() { l2a(); l2b(); }
  // fn l1b() { l2a(); l2b(); }
  // fn main() { l1a(); l1b(); }

  static constexpr int levels = 64;

  auto fn_a = [](int level) { return "l" + std::to_string(level + 1) + "a"; };
  auto fn_b = [](int level) { return "l" + std::to_string(level + 1) + "b"; };

  Func(fn_a(levels), {}, ty.void_(), {}, {});
  Func(fn_b(levels), {}, ty.void_(), {}, {});

  for (int i = levels - 1; i >= 0; i--) {
    Func(fn_a(i), {}, ty.void_(),
         {
             create<ast::CallStatement>(Call(fn_a(i + 1))),
             create<ast::CallStatement>(Call(fn_b(i + 1))),
         },
         {});
    Func(fn_b(i), {}, ty.void_(),
         {
             create<ast::CallStatement>(Call(fn_a(i + 1))),
             create<ast::CallStatement>(Call(fn_b(i + 1))),
         },
         {});
  }

  Func("main", {}, ty.void_(),
       {
           create<ast::CallStatement>(Call(fn_a(0))),
           create<ast::CallStatement>(Call(fn_b(0))),
       },
       {
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

using ResolverTextureIntrinsicTest =
    ResolverTestWithParam<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    ResolverTextureIntrinsicTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

std::string to_str(const std::string& function,
                   const semantic::ParameterList& params) {
  std::stringstream out;
  out << function << "(";
  bool first = true;
  for (auto& param : params) {
    if (!first) {
      out << ", ";
    }
    out << semantic::str(param.usage);
    first = false;
  }
  out << ")";
  return out.str();
}

const char* expected_texture_overload(
    ast::intrinsic::test::ValidTextureOverload overload) {
  using ValidTextureOverload = ast::intrinsic::test::ValidTextureOverload;
  switch (overload) {
    case ValidTextureOverload::kDimensions1d:
    case ValidTextureOverload::kDimensions2d:
    case ValidTextureOverload::kDimensions2dArray:
    case ValidTextureOverload::kDimensions3d:
    case ValidTextureOverload::kDimensionsCube:
    case ValidTextureOverload::kDimensionsCubeArray:
    case ValidTextureOverload::kDimensionsMultisampled2d:
    case ValidTextureOverload::kDimensionsMultisampled2dArray:
    case ValidTextureOverload::kDimensionsDepth2d:
    case ValidTextureOverload::kDimensionsDepth2dArray:
    case ValidTextureOverload::kDimensionsDepthCube:
    case ValidTextureOverload::kDimensionsDepthCubeArray:
    case ValidTextureOverload::kDimensionsStorageRO1d:
    case ValidTextureOverload::kDimensionsStorageRO2d:
    case ValidTextureOverload::kDimensionsStorageRO2dArray:
    case ValidTextureOverload::kDimensionsStorageRO3d:
    case ValidTextureOverload::kDimensionsStorageWO1d:
    case ValidTextureOverload::kDimensionsStorageWO2d:
    case ValidTextureOverload::kDimensionsStorageWO2dArray:
    case ValidTextureOverload::kDimensionsStorageWO3d:
      return R"(textureDimensions(texture))";
    case ValidTextureOverload::kNumLayers2dArray:
    case ValidTextureOverload::kNumLayersCubeArray:
    case ValidTextureOverload::kNumLayersMultisampled2dArray:
    case ValidTextureOverload::kNumLayersDepth2dArray:
    case ValidTextureOverload::kNumLayersDepthCubeArray:
    case ValidTextureOverload::kNumLayersStorageWO2dArray:
      return R"(textureNumLayers(texture))";
    case ValidTextureOverload::kNumLevels2d:
    case ValidTextureOverload::kNumLevels2dArray:
    case ValidTextureOverload::kNumLevels3d:
    case ValidTextureOverload::kNumLevelsCube:
    case ValidTextureOverload::kNumLevelsCubeArray:
    case ValidTextureOverload::kNumLevelsDepth2d:
    case ValidTextureOverload::kNumLevelsDepth2dArray:
    case ValidTextureOverload::kNumLevelsDepthCube:
    case ValidTextureOverload::kNumLevelsDepthCubeArray:
      return R"(textureNumLevels(texture))";
    case ValidTextureOverload::kNumSamplesMultisampled2d:
    case ValidTextureOverload::kNumSamplesMultisampled2dArray:
      return R"(textureNumSamples(texture))";
    case ValidTextureOverload::kDimensions2dLevel:
    case ValidTextureOverload::kDimensions2dArrayLevel:
    case ValidTextureOverload::kDimensions3dLevel:
    case ValidTextureOverload::kDimensionsCubeLevel:
    case ValidTextureOverload::kDimensionsCubeArrayLevel:
    case ValidTextureOverload::kDimensionsDepth2dLevel:
    case ValidTextureOverload::kDimensionsDepth2dArrayLevel:
    case ValidTextureOverload::kDimensionsDepthCubeLevel:
    case ValidTextureOverload::kDimensionsDepthCubeArrayLevel:
      return R"(textureDimensions(texture, level))";
    case ValidTextureOverload::kSample1dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample2dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample2dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSample2dArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSample2dArrayOffsetF32:
      return R"(textureSample(texture, sampler, coords, array_index, offset))";
    case ValidTextureOverload::kSample3dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSample3dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSampleCubeF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleCubeArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleDepth2dF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleDepth2dOffsetF32:
      return R"(textureSample(texture, sampler, coords, offset))";
    case ValidTextureOverload::kSampleDepth2dArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleDepth2dArrayOffsetF32:
      return R"(textureSample(texture, sampler, coords, array_index, offset))";
    case ValidTextureOverload::kSampleDepthCubeF32:
      return R"(textureSample(texture, sampler, coords))";
    case ValidTextureOverload::kSampleDepthCubeArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
    case ValidTextureOverload::kSampleBias2dF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBias2dOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
    case ValidTextureOverload::kSampleBias2dArrayF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
    case ValidTextureOverload::kSampleBias2dArrayOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias, offset))";
    case ValidTextureOverload::kSampleBias3dF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBias3dOffsetF32:
      return R"(textureSampleBias(texture, sampler, coords, bias, offset))";
    case ValidTextureOverload::kSampleBiasCubeF32:
      return R"(textureSampleBias(texture, sampler, coords, bias))";
    case ValidTextureOverload::kSampleBiasCubeArrayF32:
      return R"(textureSampleBias(texture, sampler, coords, array_index, bias))";
    case ValidTextureOverload::kSampleLevel2dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevel2dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevel2dArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevel2dArrayOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
    case ValidTextureOverload::kSampleLevel3dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevel3dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevelCubeF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelCubeArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevelDepth2dF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelDepth2dOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, level, offset))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleLevelDepth2dArrayOffsetF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level, offset))";
    case ValidTextureOverload::kSampleLevelDepthCubeF32:
      return R"(textureSampleLevel(texture, sampler, coords, level))";
    case ValidTextureOverload::kSampleLevelDepthCubeArrayF32:
      return R"(textureSampleLevel(texture, sampler, coords, array_index, level))";
    case ValidTextureOverload::kSampleGrad2dF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad2dOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGrad2dArrayF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad2dArrayOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGrad3dF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGrad3dOffsetF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy, offset))";
    case ValidTextureOverload::kSampleGradCubeF32:
      return R"(textureSampleGrad(texture, sampler, coords, ddx, ddy))";
    case ValidTextureOverload::kSampleGradCubeArrayF32:
      return R"(textureSampleGrad(texture, sampler, coords, array_index, ddx, ddy))";
    case ValidTextureOverload::kSampleCompareDepth2dF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepth2dOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref, offset))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepth2dArrayOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref, offset))";
    case ValidTextureOverload::kSampleCompareDepthCubeF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleCompareDepthCubeArrayF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
    case ValidTextureOverload::kLoad1dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad1dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad1dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelU32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelI32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad3dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad3dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad3dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoadMultisampled2dF32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dU32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dI32:
      return R"(textureLoad(texture, coords, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dArrayF32:
      return R"(textureLoad(texture, coords, array_index, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dArrayU32:
      return R"(textureLoad(texture, coords, array_index, sample_index))";
    case ValidTextureOverload::kLoadMultisampled2dArrayI32:
      return R"(textureLoad(texture, coords, array_index, sample_index))";
    case ValidTextureOverload::kLoadDepth2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoadStorageRO1dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadStorageRO2dRgba8unorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8snorm:
    case ValidTextureOverload::kLoadStorageRO2dRgba8uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba8sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba16float:
    case ValidTextureOverload::kLoadStorageRO2dR32uint:
    case ValidTextureOverload::kLoadStorageRO2dR32sint:
    case ValidTextureOverload::kLoadStorageRO2dR32float:
    case ValidTextureOverload::kLoadStorageRO2dRg32uint:
    case ValidTextureOverload::kLoadStorageRO2dRg32sint:
    case ValidTextureOverload::kLoadStorageRO2dRg32float:
    case ValidTextureOverload::kLoadStorageRO2dRgba32uint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32sint:
    case ValidTextureOverload::kLoadStorageRO2dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadStorageRO2dArrayRgba32float:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoadStorageRO3dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kStoreWO1dRgba32float:
      return R"(textureStore(texture, coords, value))";
    case ValidTextureOverload::kStoreWO2dRgba32float:
      return R"(textureStore(texture, coords, value))";
    case ValidTextureOverload::kStoreWO2dArrayRgba32float:
      return R"(textureStore(texture, coords, array_index, value))";
    case ValidTextureOverload::kStoreWO3dRgba32float:
      return R"(textureStore(texture, coords, value))";
  }
  return "<unmatched texture overload>";
}

TEST_P(ResolverTextureIntrinsicTest, Call) {
  auto param = GetParam();

  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto* call = Call(param.function, param.args(this));
  WrapInFunction(call);

  ASSERT_TRUE(r()->Resolve()) << r()->error();

  if (std::string(param.function) == "textureDimensions") {
    switch (param.texture_dimension) {
      default:
        FAIL() << "invalid texture dimensions: " << param.texture_dimension;
      case type::TextureDimension::k1d:
        EXPECT_EQ(TypeOf(call)->type_name(), ty.i32()->type_name());
        break;
      case type::TextureDimension::k2d:
      case type::TextureDimension::k2dArray:
        EXPECT_EQ(TypeOf(call)->type_name(), ty.vec2<i32>()->type_name());
        break;
      case type::TextureDimension::k3d:
      case type::TextureDimension::kCube:
      case type::TextureDimension::kCubeArray:
        EXPECT_EQ(TypeOf(call)->type_name(), ty.vec3<i32>()->type_name());
        break;
    }
  } else if (std::string(param.function) == "textureNumLayers") {
    EXPECT_EQ(TypeOf(call), ty.i32());
  } else if (std::string(param.function) == "textureNumLevels") {
    EXPECT_EQ(TypeOf(call), ty.i32());
  } else if (std::string(param.function) == "textureNumSamples") {
    EXPECT_EQ(TypeOf(call), ty.i32());
  } else if (std::string(param.function) == "textureStore") {
    EXPECT_EQ(TypeOf(call), ty.void_());
  } else {
    switch (param.texture_kind) {
      case ast::intrinsic::test::TextureKind::kRegular:
      case ast::intrinsic::test::TextureKind::kMultisampled:
      case ast::intrinsic::test::TextureKind::kStorage: {
        auto* datatype = param.resultVectorComponentType(this);
        ASSERT_TRUE(TypeOf(call)->Is<type::Vector>());
        EXPECT_EQ(TypeOf(call)->As<type::Vector>()->type(), datatype);
        break;
      }
      case ast::intrinsic::test::TextureKind::kDepth: {
        EXPECT_EQ(TypeOf(call), ty.f32());
        break;
      }
    }
  }

  auto* call_sem = Sem().Get(call);
  ASSERT_NE(call_sem, nullptr);
  auto* target = call_sem->Target();
  ASSERT_NE(target, nullptr);

  auto got = ::tint::to_str(param.function, target->Parameters());
  auto* expected = expected_texture_overload(param.overload);
  EXPECT_EQ(got, expected);
}

}  // namespace
}  // namespace tint
