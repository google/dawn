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
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace resolver {
namespace {

using ResolverValidationTest = ResolverTest;

class FakeStmt : public ast::Statement {
 public:
  FakeStmt(ProgramID program_id, Source source)
      : ast::Statement(program_id, source) {}
  FakeStmt* Clone(CloneContext*) const override { return nullptr; }
  void to_str(const sem::Info&, std::ostream& out, size_t) const override {
    out << "Fake";
  }
};

class FakeExpr : public ast::Expression {
 public:
  FakeExpr(ProgramID program_id, Source source)
      : ast::Expression(program_id, source) {}
  FakeExpr* Clone(CloneContext*) const override { return nullptr; }
  void to_str(const sem::Info&, std::ostream&, size_t) const override {}
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
  // fn main() {func(); return; }
  // fn func() { return; }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("func");
  ast::VariableList params0;

  Func("main", params0, ty.f32(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
           Return(),
       },
       ast::DecorationList{});

  Func("func", params0, ty.f32(),
       ast::StatementList{
           Return(),
       },
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: unable to find called function: func");
}

TEST_F(ResolverValidationTest, Stmt_Call_recursive) {
  // fn main() {main(); }

  SetSource(Source::Location{12, 34});
  auto* call_expr = Call("main");
  ast::VariableList params0;

  Func("main", params0, ty.void_(),
       ast::StatementList{
           create<ast::CallStatement>(call_expr),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kVertex),
       });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error v-0004: recursion is not permitted. 'main' attempted "
            "to call "
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

TEST_F(ResolverValidationTest, Stmt_Else_NonBool) {
  // else (1.23f) {}

  WrapInFunction(If(Expr(true), Block(),
                    Else(create<ast::ScalarConstructorExpression>(
                             Source{{12, 34}}, Literal(1.23f)),
                         Block())));

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: else statement condition must be bool, got f32");
}

TEST_F(ResolverValidationTest, Expr_Error_Unknown) {
  auto* e = create<FakeExpr>(Source{Source::Location{2, 30}});
  WrapInFunction(e);

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

  auto* lhs = Expr(Source{{12, 34}}, "b");
  auto* rhs = Expr(2);
  auto* assign = Assign(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: b");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInBlockStatement_Fail) {
  // {
  //  b = 2;
  // }

  auto* lhs = Expr(Source{{12, 34}}, "b");
  auto* rhs = Expr(2);

  auto* body = Block(Assign(lhs, rhs));
  WrapInFunction(body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: b");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableGlobalVariableAfter_Fail) {
  // fn my_func() {
  //   global_var = 3.14f;
  // }
  // var global_var: f32 = 2.1;

  auto* lhs = Expr(Source{{12, 34}}, "global_var");
  auto* rhs = Expr(3.14f);

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign(lhs, rhs),
       },
       ast::DecorationList{Stage(ast::PipelineStage::kVertex)});

  Global("global_var", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: "
            "global_var");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableGlobalVariable_Pass) {
  // var global_var: f32 = 2.1;
  // fn my_func() {
  //   global_var = 3.14;
  //   return;
  // }

  Global("global_var", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign(Expr(Source{Source::Location{12, 34}}, "global_var"), 3.14f),
           Return(),
       });

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableInnerScope_Fail) {
  // {
  //   if (true) { var a : f32 = 2.0; }
  //   a = 3.14;
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* cond = Expr(true);
  auto* body = Block(Decl(var));

  SetSource(Source{Source::Location{12, 34}});
  auto* lhs = Expr(Source{{12, 34}}, "a");
  auto* rhs = Expr(3.14f);

  auto* outer_body =
      Block(create<ast::IfStatement>(cond, body, ast::ElseStatementList{}),
            Assign(lhs, rhs));

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: a");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableOuterScope_Pass) {
  // {
  //   var a : f32 = 2.0;
  //   if (true) { a = 3.14; }
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));

  auto* lhs = Expr(Source{{12, 34}}, "a");
  auto* rhs = Expr(3.14f);

  auto* cond = Expr(true);
  auto* body = Block(Assign(lhs, rhs));

  auto* outer_body =
      Block(Decl(var),
            create<ast::IfStatement>(cond, body, ast::ElseStatementList{}));

  WrapInFunction(outer_body);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableDifferentScope_Fail) {
  // {
  //  { var a : f32 = 2.0; }
  //  { a = 3.14; }
  // }
  auto* var = Var("a", ty.f32(), ast::StorageClass::kNone, Expr(2.0f));
  auto* first_body = Block(Decl(var));

  auto* lhs = Expr(Source{{12, 34}}, "a");
  auto* rhs = Expr(3.14f);
  auto* second_body = Block(Assign(lhs, rhs));

  auto* outer_body = Block(first_body, second_body);

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: v-0006: identifier must be declared before use: a");
}

TEST_F(ResolverValidationTest, StorageClass_NonFunctionClassError) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kWorkgroup);

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), ast::StatementList{stmt},
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

TEST_F(ResolverValidationTest, StorageClass_SamplerExplicitStorageClass) {
  auto* t = ty.sampler(ast::SamplerKind::kSampler);
  Global(Source{{12, 34}}, "var", t, ast::StorageClass::kUniformConstant,
         nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: variables of type 'sampler' must not have a storage class)");
}

TEST_F(ResolverValidationTest, StorageClass_TextureExplicitStorageClass) {
  auto* t = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
  Global(Source{{12, 34}}, "var", t, ast::StorageClass::kUniformConstant,
         nullptr,
         {
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_FALSE(r()->Resolve()) << r()->error();

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: variables of type 'texture_1d<f32>' must not have a storage class)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kInput);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 7}}},
      Symbols().Register("xyqz"));

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:5 error: invalid vector swizzle character");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kInput);

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
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kInput);

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
  auto* continuing = Block(Assign(Expr(error_loc, "z"), 2));
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
  auto* continuing = Block(Assign(Expr(error_loc, "z"), 2));
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
  auto* continuing = Block(Assign(Expr(error_loc, "z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            "12:34 error: continue statement bypasses declaration of 'z' in "
            "continuing block");
}

TEST_F(
    ResolverValidationTest,
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
      Block(If(Expr(true), Block(Assign(Expr(error_loc, "z"), 2))));
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

  auto* continuing = Block(Loop(Block(Assign(Expr(error_loc, "z"), 2))));
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
  auto* continuing = Block(Assign("z", 2));
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
  auto* continuing = Block(If(Expr(true), Block(Assign("z", 2))));
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
  auto* continuing = Block(Loop(Block(Assign("z", 2))));
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
  auto* continuing = Block(Assign(Expr(error_loc, "z"), 2));
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
  WrapInFunction(Loop(Block(Switch(
      Expr(1),
      Case(Literal(1), Block(create<ast::BreakStatement>(Source{{12, 34}}))),
      DefaultCase()))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakNotInLoopOrSwitch) {
  WrapInFunction(create<ast::BreakStatement>(Source{{12, 34}}));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: break statement must be in a loop or switch case");
}

TEST_F(ResolverValidationTest, NonPOTStructMemberAlignDecoration) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberAlign(Source{{12, 34}}, 3)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: align value must be a positive, power-of-two integer");
}

TEST_F(ResolverValidationTest, ZeroStructMemberAlignDecoration) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberAlign(Source{{12, 34}}, 0)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: align value must be a positive, power-of-two integer");
}

TEST_F(ResolverValidationTest, ZeroStructMemberSizeDecoration) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberSize(Source{{12, 34}}, 0)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: size must be at least as big as the type's size (4)");
}

TEST_F(ResolverValidationTest, OffsetAndSizeDecoration) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberSize(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset decorations cannot be used with align or size "
            "decorations");
}

TEST_F(ResolverValidationTest, OffsetAndAlignDecoration) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberAlign(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset decorations cannot be used with align or size "
            "decorations");
}

TEST_F(ResolverValidationTest, OffsetAndAlignAndSizeDecoration) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberAlign(4), MemberSize(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset decorations cannot be used with align or size "
            "decorations");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<u32>(1u, create<ast::ScalarConstructorExpression>(
                               Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<i32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec2<bool>(true, create<ast::ScalarConstructorExpression>(
                                  Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_Vec3ArgumentCardinalityTooLarge) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_Vec4ArgumentCardinalityTooLarge) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec4<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_TooFewArgumentsScalar) {
  auto* tc = vec2<f32>(create<ast::ScalarConstructorExpression>(
      Source{{12, 34}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 1 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsScalar) {
  auto* tc = vec2<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsVector) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_TooManyArgumentsVectorAndScalar) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_InvalidConversionFromVec2Bool) {
  SetSource(Source::Location({12, 34}));

  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec2<bool>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'bool'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Error_InvalidArgumentType) {
  auto* tc = vec2<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2_Success_ZeroValue) {
  auto* tc = vec2<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2F32_Success_Scalar) {
  auto* tc = vec2<f32>(1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2U32_Success_Scalar) {
  auto* tc = vec2<u32>(1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2I32_Success_Scalar) {
  auto* tc = vec2<i32>(1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2Bool_Success_Scalar) {
  auto* tc = vec2<bool>(true, false);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec2_Success_Identity) {
  auto* tc = vec2<f32>(vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec2_Success_Vec2TypeConversion) {
  auto* tc = vec2<f32>(vec2<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<f32>(
      1.0f, 1.0f,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<u32>(
      1u,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1u);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<i32>(
      1,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec3<bool>(
      true,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      false);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_Vec4ArgumentCardinalityTooLarge) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec4<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooFewArgumentsScalar) {
  auto* tc = vec3<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsScalar) {
  auto* tc = vec3<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooFewArgumentsVec2) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 2 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec2) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec2AndScalar) {
  auto* tc = vec3<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec2<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_TooManyArgumentsVec3) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec3<f32>' with 4 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_InvalidConversionFromVec3Bool) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec3<bool>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'bool'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Error_InvalidArgumentType) {
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3_Success_ZeroValue) {
  auto* tc = vec3<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3F32_Success_Scalar) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3U32_Success_Scalar) {
  auto* tc = vec3<u32>(1u, 1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3I32_Success_Scalar) {
  auto* tc = vec3<i32>(1, 1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3Bool_Success_Scalar) {
  auto* tc = vec3<bool>(true, false, true);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3_Success_Vec2AndScalar) {
  auto* tc = vec3<f32>(vec2<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3_Success_ScalarAndVec2) {
  auto* tc = vec3<f32>(1.0f, vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec3_Success_Identity) {
  auto* tc = vec3<f32>(vec3<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec3_Success_Vec3TypeConversion) {
  auto* tc = vec3<f32>(vec3<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4F32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<f32>(
      1.0f, 1.0f,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4U32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<u32>(
      1u, 1u,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      1u);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4I32_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<i32>(
      1, 1,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1u)),
      1);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'i32', found 'u32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4Bool_Error_ScalarArgumentTypeMismatch) {
  auto* tc = vec4<bool>(
      true, false,
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1)),
      true);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'bool', found 'i32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsScalar) {
  auto* tc = vec4<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsScalar) {
  auto* tc = vec4<f32>(
      create<ast::ScalarConstructorExpression>(Source{{12, 34}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 58}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsVec2AndScalar) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 40}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndScalars) {
  auto* tc = vec4<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec2<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 52}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Scalar) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()),
                       create<ast::ScalarConstructorExpression>(
                           Source{{12, 46}}, Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2Vec2Vec2) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooFewArgumentsVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 3 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndScalars) {
  auto* tc = vec4<f32>(
      create<ast::TypeConstructorExpression>(Source{{12, 34}}, ty.vec3<f32>(),
                                             ExprList()),
      create<ast::ScalarConstructorExpression>(Source{{12, 40}}, Literal(1.0f)),
      create<ast::ScalarConstructorExpression>(Source{{12, 46}},
                                               Literal(1.0f)));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec2) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec2AndVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec2<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 5 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_TooManyArgumentsVec3AndVec3) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, ty.vec3<f32>(), ExprList()),
                       create<ast::TypeConstructorExpression>(
                           Source{{12, 40}}, ty.vec3<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec4<f32>' with 6 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_InvalidConversionFromVec4Bool) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.vec4<bool>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'bool'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Error_InvalidArgumentType) {
  auto* tc = vec4<f32>(create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.mat2x2<f32>(), ExprList()));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected vector or scalar type in vector "
            "constructor; found: mat2x2<f32>");
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_ZeroValue) {
  auto* tc = vec4<f32>();
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4F32_Success_Scalar) {
  auto* tc = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4U32_Success_Scalar) {
  auto* tc = vec4<u32>(1u, 1u, 1u, 1u);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::U32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4I32_Success_Scalar) {
  auto* tc = vec4<i32>(1, 1, 1, 1);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::I32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4Bool_Success_Scalar) {
  auto* tc = vec4<bool>(true, false, true, false);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::Bool>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_Vec2ScalarScalar) {
  auto* tc = vec4<f32>(vec2<f32>(), 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_ScalarVec2Scalar) {
  auto* tc = vec4<f32>(1.0f, vec2<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_ScalarScalarVec2) {
  auto* tc = vec4<f32>(1.0f, 1.0f, vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_Vec2AndVec2) {
  auto* tc = vec4<f32>(vec2<f32>(), vec2<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_Vec3AndScalar) {
  auto* tc = vec4<f32>(vec3<f32>(), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_ScalarAndVec3) {
  auto* tc = vec4<f32>(1.0f, vec3<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vec4_Success_Identity) {
  auto* tc = vec4<f32>(vec4<f32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vec4_Success_Vec4TypeConversion) {
  auto* tc = vec4<f32>(vec4<i32>());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_NestedVectorConstructors_InnerError) {
  auto* tc = vec4<f32>(
      vec3<f32>(1.0f, vec2<f32>(create<ast::ScalarConstructorExpression>(
                          Source{{12, 34}}, Literal(1.0f)))),
      1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: attempted to construct 'vec2<f32>' with 1 component(s)");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_NestedVectorConstructors_Success) {
  auto* tc = vec4<f32>(vec3<f32>(vec2<f32>(1.0f, 1.0f), 1.0f), 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vector_Alias_Argument_Error) {
  auto* alias = ty.alias("UnsignedInt", ty.u32());
  AST().AddConstructedType(alias);
  Global("uint_var", alias, ast::StorageClass::kInput);

  auto* tc = vec2<f32>(Expr(Source{{12, 34}}, "uint_var"));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'UnsignedInt'");
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vector_Alias_Argument_Success) {
  auto* f32_alias = ty.alias("Float32", ty.f32());
  auto* vec2_alias = ty.alias("VectorFloat2", ty.vec2<f32>());
  AST().AddConstructedType(f32_alias);
  AST().AddConstructedType(vec2_alias);
  Global("my_f32", f32_alias, ast::StorageClass::kInput);
  Global("my_vec2", vec2_alias, ast::StorageClass::kInput);

  auto* tc = vec3<f32>("my_vec2", "my_f32");
  WrapInFunction(tc);
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Expr_Constructor_Vector_ElementTypeAlias_Error) {
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  // vec2<Float32>(1.0f, 1u)
  auto* vec_type = ty.vec(f32_alias, 2);
  auto* tc = create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, vec_type,
      ExprList(1.0f, create<ast::ScalarConstructorExpression>(Source{{12, 40}},
                                                              Literal(1u))));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:40 error: type in vector constructor does not match vector "
            "type: expected 'f32', found 'u32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vector_ElementTypeAlias_Success) {
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  // vec2<Float32>(1.0f, 1.0f)
  auto* vec_type = ty.vec(f32_alias, 2);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 34}}, vec_type,
                                                    ExprList(1.0f, 1.0f));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vector_ArgumentElementTypeAlias_Error) {
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  // vec3<u32>(vec<Float32>(), 1.0f)
  auto* vec_type = ty.vec(f32_alias, 2);
  auto* tc = vec3<u32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, vec_type, ExprList()),
                       1.0f);
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: type in vector constructor does not match vector "
            "type: expected 'u32', found 'f32'");
}

TEST_F(ResolverValidationTest,
       Expr_Constructor_Vector_ArgumentElementTypeAlias_Success) {
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  // vec3<f32>(vec<Float32>(), 1.0f)
  auto* vec_type = ty.vec(f32_alias, 2);
  auto* tc = vec3<f32>(create<ast::TypeConstructorExpression>(
                           Source{{12, 34}}, vec_type, ExprList()),
                       1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

struct MatrixDimensions {
  uint32_t rows;
  uint32_t columns;
};

std::string MatrixStr(const MatrixDimensions& dimensions,
                      std::string subtype = "f32") {
  return "mat" + std::to_string(dimensions.columns) + "x" +
         std::to_string(dimensions.rows) + "<" + subtype + ">";
}

std::string VecStr(uint32_t dimensions, std::string subtype = "f32") {
  return "vec" + std::to_string(dimensions) + "<" + subtype + ">";
}

using MatrixConstructorTest = ResolverTestWithParam<MatrixDimensions>;

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_TooFewArguments) {
  // matNxM<f32>(vecM<f32>(), ...); with N - 1 arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected " + std::to_string(param.columns) + " '" +
                VecStr(param.rows) + "' arguments in '" + MatrixStr(param) +
                "' constructor, found " + std::to_string(param.columns - 1));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_TooManyArguments) {
  // matNxM<f32>(vecM<f32>(), ...); with N + 1 arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns + 1; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected " + std::to_string(param.columns) + " '" +
                VecStr(param.rows) + "' arguments in '" + MatrixStr(param) +
                "' constructor, found " + std::to_string(param.columns + 1));
}

TEST_P(MatrixConstructorTest, Expr_Constructor_Error_InvalidArgumentType) {
  // matNxM<f32>(1.0, 1.0, ...); N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    args.push_back(create<ast::ScalarConstructorExpression>(Source{{12, i}},
                                                            Literal(1.0f)));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found 'f32'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_TooFewRowsInVectorArgument) {
  // matNxM<f32>(vecM<f32>(),...,vecM-1<f32>());

  const auto param = GetParam();

  // Skip the test if parameters would have resuled in an invalid vec1 type.
  if (param.rows == 2) {
    return;
  }

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* valid_vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, valid_vec_type, ExprList()));
  }
  const size_t kInvalidLoc = 2 * (param.columns - 1);
  auto* invalid_vec_type = ty.vec<f32>(param.rows - 1);
  args.push_back(create<ast::TypeConstructorExpression>(
      Source{{12, kInvalidLoc}}, invalid_vec_type, ExprList()));

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:" + std::to_string(kInvalidLoc) +
                              " error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows - 1) + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_TooManyRowsInVectorArgument) {
  // matNxM<f32>(vecM<f32>(),...,vecM+1<f32>());

  const auto param = GetParam();

  // Skip the test if parameters would have resuled in an invalid vec5 type.
  if (param.rows == 4) {
    return;
  }

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns - 1; i++) {
    auto* valid_vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, valid_vec_type, ExprList()));
  }
  const size_t kInvalidLoc = 2 * (param.columns - 1);
  auto* invalid_vec_type = ty.vec<f32>(param.rows + 1);
  args.push_back(create<ast::TypeConstructorExpression>(
      Source{{12, kInvalidLoc}}, invalid_vec_type, ExprList()));

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:" + std::to_string(kInvalidLoc) +
                              " error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows + 1) + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_Error_ArgumentVectorElementTypeMismatch) {
  // matNxM<f32>(vecM<u32>(), ...); with N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<u32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows, "u32") + "'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ZeroValue_Success) {
  // matNxM<f32>();

  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{{12, 40}},
                                                    matrix_type, ExprList());
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_WithArguments_Success) {
  // matNxM<f32>(vecM<f32>(), ...); with N arguments

  const auto param = GetParam();

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Error) {
  // matNxM<Float32>(vecM<u32>(), ...); with N arguments

  const auto param = GetParam();
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(ty.u32(), param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat(f32_alias, param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:1 error: expected argument type '" + VecStr(param.rows) +
                "' in '" + MatrixStr(param, "Float32") +
                "' constructor, found '" + VecStr(param.rows, "u32") + "'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ElementTypeAlias_Success) {
  // matNxM<Float32>(vecM<f32>(), ...); with N arguments

  const auto param = GetParam();
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec<f32>(param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat(f32_alias, param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Expr_MatrixConstructor_ArgumentTypeAlias_Error) {
  auto* alias = ty.alias("VectorUnsigned2", ty.vec2<u32>());
  AST().AddConstructedType(alias);
  auto* tc = mat2x2<f32>(
      create<ast::TypeConstructorExpression>(
          Source{{12, 34}}, ty.MaybeCreateTypename(alias), ExprList()),
      vec2<f32>());
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: expected argument type 'vec2<f32>' in 'mat2x2<f32>' "
            "constructor, found 'VectorUnsigned2'");
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentTypeAlias_Success) {
  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* vec_type = ty.vec<f32>(param.rows);
  auto* vec_alias = ty.alias("VectorFloat2", vec_type);
  AST().AddConstructedType(vec_alias);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, ty.MaybeCreateTypename(vec_alias), ExprList()));
  }

  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_P(MatrixConstructorTest, Expr_Constructor_ArgumentElementTypeAlias_Error) {
  const auto param = GetParam();
  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* f32_alias = ty.alias("UnsignedInt", ty.u32());
  AST().AddConstructedType(f32_alias);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(f32_alias, param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:1 error: expected argument type '" +
                              VecStr(param.rows) + "' in '" + MatrixStr(param) +
                              "' constructor, found '" +
                              VecStr(param.rows, "UnsignedInt") + "'");
}

TEST_P(MatrixConstructorTest,
       Expr_Constructor_ArgumentElementTypeAlias_Success) {
  const auto param = GetParam();
  auto* f32_alias = ty.alias("Float32", ty.f32());
  AST().AddConstructedType(f32_alias);

  ast::ExpressionList args;
  for (uint32_t i = 1; i <= param.columns; i++) {
    auto* vec_type = ty.vec(f32_alias, param.rows);
    args.push_back(create<ast::TypeConstructorExpression>(
        Source{{12, i}}, vec_type, ExprList()));
  }

  auto* matrix_type = ty.mat<f32>(param.columns, param.rows);
  auto* tc = create<ast::TypeConstructorExpression>(Source{}, matrix_type,
                                                    std::move(args));
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

INSTANTIATE_TEST_SUITE_P(ResolverValidationTest,
                         MatrixConstructorTest,
                         testing::Values(MatrixDimensions{2, 2},
                                         MatrixDimensions{3, 2},
                                         MatrixDimensions{4, 2},
                                         MatrixDimensions{2, 3},
                                         MatrixDimensions{3, 3},
                                         MatrixDimensions{4, 3},
                                         MatrixDimensions{2, 4},
                                         MatrixDimensions{3, 4},
                                         MatrixDimensions{4, 4}));

}  // namespace
}  // namespace resolver
}  // namespace tint
