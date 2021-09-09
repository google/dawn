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
#include "gtest/gtest-spi.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
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

class FakeExpr : public Castable<FakeExpr, ast::Expression> {
 public:
  FakeExpr(ProgramID program_id, Source source) : Base(program_id, source) {}
  FakeExpr* Clone(CloneContext*) const override { return nullptr; }
  void to_str(const sem::Info&, std::ostream&, size_t) const override {}
};

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInVertexStage) {
  Global(Source{{1, 2}}, "wg", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  Global("dst", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

  Func(Source{{9, 10}}, "f0", ast::VariableList{}, ty.vec4<f32>(),
       {stmt, Return(Expr("dst"))},
       ast::DecorationList{Stage(ast::PipelineStage::kVertex)},
       ast::DecorationList{Builtin(ast::Builtin::kPosition)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "3:4 error: workgroup memory cannot be used by vertex pipeline "
            "stage\n1:2 note: variable is declared here");
}

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInFragmentStage) {
  // var<workgroup> wg : vec4<f32>;
  // var<workgroup> dst : vec4<f32>;
  // fn f2(){ dst = wg; }
  // fn f1() { f2(); }
  // [[stage(fragment)]]
  // fn f0() {
  //  f1();
  //}

  Global(Source{{1, 2}}, "wg", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  Global("dst", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

  Func(Source{{5, 6}}, "f2", {}, ty.void_(), {stmt});
  Func(Source{{7, 8}}, "f1", {}, ty.void_(),
       {create<ast::CallStatement>(Call("f2"))});
  Func(Source{{9, 10}}, "f0", {}, ty.void_(),
       {create<ast::CallStatement>(Call("f1"))},
       ast::DecorationList{Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:4 error: workgroup memory cannot be used by fragment pipeline stage
1:2 note: variable is declared here
5:6 note: called by function 'f2'
7:8 note: called by function 'f1'
9:10 note: called by entry point 'f0')");
}

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

TEST_F(ResolverValidationTest, Expr_ErrUnknownExprType) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.WrapInFunction(b.create<FakeExpr>());
        Resolver(&b).Resolve();
      },
      "internal compiler error: unhandled expression type: "
      "tint::resolver::FakeExpr");
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

TEST_F(ResolverValidationTest,
       AssignmentStmt_InvalidLHS_IntrinsicFunctionName) {
  // normalize = 2;

  auto* lhs = Expr(Source{{12, 34}}, "normalize");
  auto* rhs = Expr(2);
  auto* assign = Assign(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: missing '(' for intrinsic call");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariable_Fail) {
  // b = 2;

  auto* lhs = Expr(Source{{12, 34}}, "b");
  auto* rhs = Expr(2);
  auto* assign = Assign(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: identifier must be declared before use: b");
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
            "12:34 error: identifier must be declared before use: b");
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
            "12:34 error: identifier must be declared before use: global_var");
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
            "12:34 error: identifier must be declared before use: a");
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
            "12:34 error: identifier must be declared before use: a");
}

TEST_F(ResolverValidationTest, StorageClass_FunctionVariableWorkgroupClass) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kWorkgroup);

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), ast::StatementList{stmt},
       ast::DecorationList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

TEST_F(ResolverValidationTest, StorageClass_FunctionVariableI32) {
  auto* var = Var("s", ty.i32(), ast::StorageClass::kPrivate);

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
         ast::DecorationList{
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
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  EXPECT_FALSE(r()->Resolve()) << r()->error();

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: variables of type 'texture_1d<f32>' must not have a storage class)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 7}}},
      Symbols().Register("xyqz"));

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:5 error: invalid vector swizzle character");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
  Global("my_vec", ty.vec4<f32>(), ast::StorageClass::kPrivate);

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
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* ident = create<ast::IdentifierExpression>(
      Source{{Source::Location{3, 3}, Source::Location{3, 8}}},
      Symbols().Register("zzzzz"));
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle size");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadIndex) {
  Global("my_vec", ty.vec2<f32>(), ast::StorageClass::kPrivate);

  auto* ident = create<ast::IdentifierExpression>(Source{{3, 3}},
                                                  Symbols().Register("z"));
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle member");
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
  auto* body =
      Block(create<ast::ContinueStatement>(),
            Decl(error_loc, Var("z", ty.i32(), ast::StorageClass::kNone)),
            create<ast::ContinueStatement>());
  auto* continuing = Block(Assign(Expr("z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(), "12:34 error: code is unreachable");
}

TEST_F(
    ResolverValidationTest,
    Stmt_Loop_ContinueInLoopBodyBeforeDeclAndAfterDecl_UsageInContinuing_InBlocks) {  // NOLINT - line length
  // loop  {
  //     var z : i32;
  //     {{{continue;}}} // Bypasses z decl
  //     z = 1;
  //     continue; // Ok
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto* body =
      Block(Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),
            Block(Block(Block(create<ast::ContinueStatement>()))),
            Assign(Source{{12, 34}}, "z", 2), create<ast::ContinueStatement>());
  auto* continuing = Block(Assign(Expr("z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(), "12:34 error: code is unreachable");
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

TEST_F(ResolverTest, Stmt_Loop_ReturnInContinuing_Direct) {
  // loop  {
  //   continuing {
  //     return;
  //   }
  // }

  WrapInFunction(Loop(  // loop
      Block(),          //   loop block
      Block(            //   loop continuing block
          Return(Source{{12, 34}}))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a return statement)");
}

TEST_F(ResolverTest, Stmt_Loop_ReturnInContinuing_Indirect) {
  // loop {
  //   continuing {
  //     loop {
  //       return;
  //     }
  //   }
  // }

  WrapInFunction(Loop(         // outer loop
      Block(),                 //   outer loop block
      Block(Source{{56, 78}},  //   outer loop continuing block
            Loop(              //     inner loop
                Block(         //       inner loop block
                    Return(Source{{12, 34}}))))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a return statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_Loop_DiscardInContinuing_Direct) {
  // loop  {
  //   continuing {
  //     discard;
  //   }
  // }

  WrapInFunction(Loop(  // loop
      Block(),          //   loop block
      Block(            //   loop continuing block
          create<ast::DiscardStatement>(Source{{12, 34}}))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement)");
}

TEST_F(ResolverTest, Stmt_Loop_DiscardInContinuing_Indirect) {
  // loop {
  //   continuing {
  //     loop { discard; }
  //   }
  // }

  WrapInFunction(Loop(         // outer loop
      Block(),                 //   outer loop block
      Block(Source{{56, 78}},  //   outer loop continuing block
            Loop(              //     inner loop
                Block(         //       inner loop block
                    create<ast::DiscardStatement>(Source{{12, 34}}))))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInContinuing_Direct) {
  // loop  {
  //     continuing {
  //         continue;
  //     }
  // }

  WrapInFunction(Loop(         // loop
      Block(),                 //   loop block
      Block(Source{{56, 78}},  //   loop continuing block
            create<ast::ContinueStatement>(Source{{12, 34}}))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: continuing blocks must not contain a continue statement");
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInContinuing_Indirect) {
  // loop {
  //   continuing {
  //     loop {
  //       continue;
  //     }
  //   }
  // }

  WrapInFunction(Loop(  // outer loop
      Block(),          //   outer loop block
      Block(            //   outer loop continuing block
          Loop(         //     inner loop
              Block(    //       inner loop block
                  create<ast::ContinueStatement>(Source{{12, 34}}))))));

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_CondIsBoolRef) {
  // var cond : bool = true;
  // for (; cond; ) {
  // }

  auto* cond = Var("cond", ty.bool_(), Expr(true));
  WrapInFunction(Decl(cond), For(nullptr, "cond", nullptr, Block()));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_CondIsNotBool) {
  // for (; 1.0f; ) {
  // }

  WrapInFunction(For(nullptr, Expr(Source{{12, 34}}, 1.0f), nullptr, Block()));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: for-loop condition must be bool, got f32");
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

TEST_F(ResolverValidationTest, StructMemberDuplicateName) {
  Structure("S", {Member(Source{{12, 34}}, "a", ty.i32()),
                  Member(Source{{56, 78}}, "a", ty.i32())});
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: redefinition of 'a'\n12:34 note: previous definition "
            "is here");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNameDifferentTypes) {
  Structure("S", {Member(Source{{12, 34}}, "a", ty.bool_()),
                  Member(Source{{12, 34}}, "a", ty.vec3<f32>())});
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: redefinition of 'a'\n12:34 note: previous definition "
            "is here");
}
TEST_F(ResolverValidationTest, StructMemberDuplicateNamePass) {
  Structure("S", {Member("a", ty.i32()), Member("b", ty.f32())});
  Structure("S1", {Member("a", ty.i32()), Member("b", ty.f32())});
  EXPECT_TRUE(r()->Resolve());
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

TEST_F(ResolverTest, Expr_Constructor_Cast_Pointer) {
  auto* vf = Var("vf", ty.f32());
  auto* c = create<ast::TypeConstructorExpression>(
      Source{{12, 34}}, ty.pointer<i32>(ast::StorageClass::kFunction),
      ExprList(vf));
  auto* ip = Const("ip", ty.pointer<i32>(ast::StorageClass::kFunction), c);
  WrapInFunction(Decl(vf), Decl(ip));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

}  // namespace
}  // namespace resolver
}  // namespace tint

TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeExpr);
