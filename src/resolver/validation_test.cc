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
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
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
#include "src/resolver/resolver_test_helper.h"
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
namespace resolver {
namespace {

using ResolverValidationTest = ResolverTest;

class FakeStmt : public ast::Statement {
 public:
  explicit FakeStmt(Source source) : ast::Statement(source) {}
  FakeStmt* Clone(CloneContext*) const override { return nullptr; }
  void to_str(const semantic::Info&, std::ostream& out, size_t) const override {
    out << "Fake";
  }
};

class FakeExpr : public ast::Expression {
 public:
  explicit FakeExpr(Source source) : ast::Expression(source) {}
  FakeExpr* Clone(CloneContext*) const override { return nullptr; }
  void to_str(const semantic::Info&, std::ostream&, size_t) const override {}
};

TEST_F(ResolverValidationTest, Error_WithEmptySource) {
  auto* s = create<FakeStmt>();
  WrapInFunction(s);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: unknown statement type for type determination: Fake");
}

TEST_F(ResolverValidationTest, Stmt_Error_Unknown) {
  auto* s = create<FakeStmt>(Source{Source::Location{2, 30}});
  WrapInFunction(s);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "2:30 error: unknown statement type for type determination: Fake");
}

TEST_F(ResolverValidationTest, Stmt_Call_undeclared) {
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
       ast::DecorationList{});

  Func("func", params0, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: unable to find called function: func");
}

TEST_F(ResolverValidationTest, Stmt_Call_recursive) {
  // fn main() -> void {main(); }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("main");
  ast::VariableList params0;

  Func("main", params0, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: recursion is not permitted. 'main' attempted to call "
            "itself.");
}

TEST_F(ResolverValidationTest, Stmt_If_NonBool) {
  // if (1.23f) {}

  WrapInFunction(If(create<ast::ScalarConstructorExpression>(Source{{12, 34}},
                                                             Literal(1.23f)),
                    Block()));

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: if statement condition must be bool, got f32");
}

TEST_F(ResolverValidationTest,
       Stmt_VariableDecl_MismatchedTypeScalarConstructor) {
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

TEST_F(ResolverValidationTest,
       Stmt_VariableDecl_MismatchedTypeScalarConstructor_Alias) {
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

TEST_F(ResolverValidationTest, Expr_Error_Unknown) {
  FakeExpr e(Source{Source::Location{2, 30}});
  WrapInFunction(&e);

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "2:30 error: unknown expression for type determination");
}

TEST_F(ResolverValidationTest, Expr_DontCall_Function) {
  Func("func", {}, ty.void_(), {}, {});
  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("func"));
  WrapInFunction(ident);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for function call");
}

TEST_F(ResolverValidationTest, Expr_DontCall_Intrinsic) {
  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("round"));
  WrapInFunction(ident);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for intrinsic call");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariable_Fail) {
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

TEST_F(ResolverValidationTest, UsingUndefinedVariableInBlockStatement_Fail) {
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

TEST_F(ResolverValidationTest, StorageClass_NonFunctionClassError) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kWorkgroup);

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 7}}},
      Symbols().Register("xyqz"));

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:5 error: invalid vector swizzle character");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
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

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadLength) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kNone);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("zzzzz"));
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle size");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDecl_UsageInContinuing) {
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverValidationTest,
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

TEST_F(ResolverTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing) {
  // loop  {
  //     var z : i32;
  //     continue;
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{Source::Location{12, 34}};
  auto* body = Block(Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),
                     create<ast::ContinueStatement>());
  auto* continuing = Block(Assign(Expr(error_loc, "z"), Expr(2)));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve());
}

TEST_F(ResolverValidationTest, Stmt_ContinueInLoop) {
  WrapInFunction(Loop(Block(create<ast::ContinueStatement>(Source{{12, 34}}))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_ContinueNotInLoop) {
  WrapInFunction(create<ast::ContinueStatement>(Source{{12, 34}}));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: continue statement must be in a loop");
}

TEST_F(ResolverValidationTest, Stmt_BreakInLoop) {
  WrapInFunction(Loop(Block(create<ast::BreakStatement>(Source{{12, 34}}))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitch) {
  WrapInFunction(Loop(Block(create<ast::SwitchStatement>(
      Expr(1), ast::CaseStatementList{
                   create<ast::CaseStatement>(
                       ast::CaseSelectorList{Literal(1)},
                       Block(create<ast::BreakStatement>(Source{{12, 34}}))),
               }))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakNotInLoopOrSwitch) {
  WrapInFunction(create<ast::BreakStatement>(Source{{12, 34}}));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: break statement must be in a loop or switch case");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
