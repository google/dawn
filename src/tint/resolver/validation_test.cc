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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint::resolver {
namespace {

using ResolverValidationTest = ResolverTest;

class FakeStmt final : public Castable<FakeStmt, ast::Statement> {
 public:
  FakeStmt(ProgramID pid, Source src) : Base(pid, src) {}
  FakeStmt* Clone(CloneContext*) const override { return nullptr; }
};

class FakeExpr final : public Castable<FakeExpr, ast::Expression> {
 public:
  FakeExpr(ProgramID pid, Source src) : Base(pid, src) {}
  FakeExpr* Clone(CloneContext*) const override { return nullptr; }
};

TEST_F(ResolverValidationTest, WorkgroupMemoryUsedInVertexStage) {
  Global(Source{{1, 2}}, "wg", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  Global("dst", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

  Func(Source{{9, 10}}, "f0", ast::VariableList{}, ty.vec4<f32>(),
       {stmt, Return(Expr("dst"))},
       ast::AttributeList{Stage(ast::PipelineStage::kVertex)},
       ast::AttributeList{Builtin(ast::Builtin::kPosition)});

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
  // @stage(fragment)
  // fn f0() {
  //  f1();
  //}

  Global(Source{{1, 2}}, "wg", ty.vec4<f32>(), ast::StorageClass::kWorkgroup);
  Global("dst", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* stmt = Assign(Expr("dst"), Expr(Source{{3, 4}}, "wg"));

  Func(Source{{5, 6}}, "f2", {}, ty.void_(), {stmt});
  Func(Source{{7, 8}}, "f1", {}, ty.void_(), {CallStmt(Call("f2"))});
  Func(Source{{9, 10}}, "f0", {}, ty.void_(), {CallStmt(Call("f1"))},
       ast::AttributeList{Stage(ast::PipelineStage::kFragment)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(3:4 error: workgroup memory cannot be used by fragment pipeline stage
1:2 note: variable is declared here
5:6 note: called by function 'f2'
7:8 note: called by function 'f1'
9:10 note: called by entry point 'f0')");
}

TEST_F(ResolverValidationTest, UnhandledStmt) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.WrapInFunction(b.create<FakeStmt>());
        Program(std::move(b));
      },
      "internal compiler error: unhandled node type: tint::resolver::FakeStmt");
}

TEST_F(ResolverValidationTest, Stmt_If_NonBool) {
  // if (1.23f) {}

  WrapInFunction(If(Expr(Source{{12, 34}}, 1.23f), Block()));

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: if statement condition must be bool, got f32");
}

TEST_F(ResolverValidationTest, Stmt_ElseIf_NonBool) {
  // else if (1.23f) {}

  WrapInFunction(
      If(Expr(true), Block(), If(Expr(Source{{12, 34}}, 1.23f), Block())));

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "12:34 error: if statement condition must be bool, got f32");
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
  WrapInFunction(Expr(Source{{{3, 3}, {3, 8}}}, "func"));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for function call");
}

TEST_F(ResolverValidationTest, Expr_DontCall_Builtin) {
  WrapInFunction(Expr(Source{{{3, 3}, {3, 8}}}, "round"));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:8 error: missing '(' for builtin call");
}

TEST_F(ResolverValidationTest, Expr_DontCall_Type) {
  Alias("T", ty.u32());
  WrapInFunction(Expr(Source{{{3, 3}, {3, 8}}}, "T"));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "3:8 error: missing '(' for type constructor or cast");
}

TEST_F(ResolverValidationTest, AssignmentStmt_InvalidLHS_BuiltinFunctionName) {
  // normalize = 2;

  auto* lhs = Expr(Source{{12, 34}}, "normalize");
  auto* rhs = Expr(2);
  auto* assign = Assign(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: missing '(' for builtin call");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariable_Fail) {
  // b = 2;

  auto* lhs = Expr(Source{{12, 34}}, "b");
  auto* rhs = Expr(2);
  auto* assign = Assign(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: unknown identifier: 'b'");
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
  EXPECT_EQ(r()->error(), "12:34 error: unknown identifier: 'b'");
}

TEST_F(ResolverValidationTest, UsingUndefinedVariableGlobalVariable_Pass) {
  // var global_var: f32 = 2.1;
  // fn my_func() {
  //   global_var = 3.14;
  //   return;
  // }

  Global("global_var", ty.f32(), ast::StorageClass::kPrivate, Expr(2.1f));

  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Assign(Expr(Source{{12, 34}}, "global_var"), 3.14f),
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

  SetSource(Source{{12, 34}});
  auto* lhs = Expr(Source{{12, 34}}, "a");
  auto* rhs = Expr(3.14f);

  auto* outer_body = Block(If(cond, body), Assign(lhs, rhs));

  WrapInFunction(outer_body);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: unknown identifier: 'a'");
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

  auto* outer_body = Block(Decl(var), If(cond, body));

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
  EXPECT_EQ(r()->error(), "12:34 error: unknown identifier: 'a'");
}

TEST_F(ResolverValidationTest, StorageClass_FunctionVariableWorkgroupClass) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kWorkgroup);

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::AttributeList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

TEST_F(ResolverValidationTest, StorageClass_FunctionVariableI32) {
  auto* var = Var("s", ty.i32(), ast::StorageClass::kPrivate);

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::AttributeList{});

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(r()->error(),
            "error: function variable has a non-function storage class");
}

TEST_F(ResolverValidationTest, StorageClass_SamplerExplicitStorageClass) {
  auto* t = ty.sampler(ast::SamplerKind::kSampler);
  Global(Source{{12, 34}}, "var", t, ast::StorageClass::kHandle,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(0),
         });

  EXPECT_FALSE(r()->Resolve());

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: variables of type 'sampler' must not have a storage class)");
}

TEST_F(ResolverValidationTest, StorageClass_TextureExplicitStorageClass) {
  auto* t = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
  Global(Source{{12, 34}}, "var", t, ast::StorageClass::kHandle,
         ast::AttributeList{
             create<ast::BindingAttribute>(0),
             create<ast::GroupAttribute>(0),
         });

  EXPECT_FALSE(r()->Resolve()) << r()->error();

  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: variables of type 'texture_1d<f32>' must not have a storage class)");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadChar) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* ident = Expr(Source{{{3, 3}, {3, 7}}}, "xyqz");

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:5 error: invalid vector swizzle character");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_MixedChars) {
  Global("my_vec", ty.vec4<f32>(), ast::StorageClass::kPrivate);

  auto* ident = Expr(Source{{{3, 3}, {3, 7}}}, "rgyw");

  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "3:3 error: invalid mixing of vector swizzle characters rgba with xyzw");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadLength) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* ident = Expr(Source{{{3, 3}, {3, 8}}}, "zzzzz");
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle size");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_VectorSwizzle_BadIndex) {
  Global("my_vec", ty.vec2<f32>(), ast::StorageClass::kPrivate);

  auto* ident = Expr(Source{{3, 3}}, "z");
  auto* mem = MemberAccessor("my_vec", ident);
  WrapInFunction(mem);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "3:3 error: invalid vector swizzle member");
}

TEST_F(ResolverValidationTest, Expr_MemberAccessor_BadParent) {
  // var param: vec4<f32>
  // let ret: f32 = *(&param).x;
  auto* param = Var("param", ty.vec4<f32>());
  auto* x = Expr(Source{{{3, 3}, {3, 8}}}, "x");

  auto* addressOf_expr = AddressOf(Source{{12, 34}}, param);
  auto* accessor_expr = MemberAccessor(addressOf_expr, x);
  auto* star_p = Deref(accessor_expr);
  auto* ret = Var("r", ty.f32(), star_p);
  WrapInFunction(Decl(param), Decl(ret));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: invalid member accessor expression. Expected vector "
            "or struct, got 'ptr<function, vec4<f32>, read_write>'");
}

TEST_F(ResolverValidationTest, EXpr_MemberAccessor_FuncGoodParent) {
  // fn func(p: ptr<function, vec4<f32>>) -> f32 {
  //     let x: f32 = (*p).z;
  //     return x;
  // }
  auto* p =
      Param("p", ty.pointer(ty.vec4<f32>(), ast::StorageClass::kFunction));
  auto* star_p = Deref(p);
  auto* z = Expr(Source{{{3, 3}, {3, 8}}}, "z");
  auto* accessor_expr = MemberAccessor(star_p, z);
  auto* x = Var("x", ty.f32(), accessor_expr);
  Func("func", {p}, ty.f32(), {Decl(x), Return(x)});
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, EXpr_MemberAccessor_FuncBadParent) {
  // fn func(p: ptr<function, vec4<f32>>) -> f32 {
  //     let x: f32 = *p.z;
  //     return x;
  // }
  auto* p =
      Param("p", ty.pointer(ty.vec4<f32>(), ast::StorageClass::kFunction));
  auto* z = Expr(Source{{{3, 3}, {3, 8}}}, "z");
  auto* accessor_expr = MemberAccessor(p, z);
  auto* star_p = Deref(accessor_expr);
  auto* x = Var("x", ty.f32(), star_p);
  Func("func", {p}, ty.f32(), {Decl(x), Return(x)});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "error: invalid member accessor expression. "
      "Expected vector or struct, got 'ptr<function, vec4<f32>, read_write>'");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyBeforeDeclAndAfterDecl_UsageInContinuing) {
  // loop  {
  //     continue; // Bypasses z decl
  //     var z : i32; // unreachable
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{{12, 34}};
  auto* body =
      Block(Continue(),
            Decl(error_loc, Var("z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Assign(Expr("z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            R"(12:34 warning: code is unreachable
error: continue statement bypasses declaration of 'z'
note: identifier 'z' declared here
note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing_InBlocks) {
  // loop  {
  //     if (false) { break; }
  //     var z : i32;
  //     {{{continue;}}}
  //     continue; // Ok
  //
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto* body = Block(If(false, Block(Break())),  //
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),
                     Block(Block(Block(Continue()))));
  auto* continuing = Block(Assign(Expr("z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
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

  auto cont_loc = Source{{12, 34}};
  auto decl_loc = Source{{56, 78}};
  auto ref_loc = Source{{90, 12}};
  auto* body =
      Block(If(Expr(true), Block(Continue(cont_loc))),
            Decl(Var(decl_loc, "z", ty.i32(), ast::StorageClass::kNone)));
  auto* continuing = Block(Assign(Expr(ref_loc, "z"), 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
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

  auto cont_loc = Source{{12, 34}};
  auto decl_loc = Source{{56, 78}};
  auto ref_loc = Source{{90, 12}};
  auto* body =
      Block(If(Expr(true), Block(Continue(cont_loc))),
            Decl(Var(decl_loc, "z", ty.i32(), ast::StorageClass::kNone)));

  auto* continuing =
      Block(If(Expr(true), Block(Assign(Expr(ref_loc, "z"), 2))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInLoopBodySubscopeBeforeDecl_UsageOutsideBlock) {
  // loop  {
  //     if (true) {
  //         continue; // bypasses z decl (if we reach here)
  //     }
  //     var z : i32;
  //     continuing {
  //         // Must fail even if z is used in an expression that isn't
  //         // directly contained inside a block.
  //         if (z < 2) {
  //         }
  //     }
  // }

  auto cont_loc = Source{{12, 34}};
  auto decl_loc = Source{{56, 78}};
  auto ref_loc = Source{{90, 12}};
  auto* body =
      Block(If(Expr(true), Block(Continue(cont_loc))),
            Decl(Var(decl_loc, "z", ty.i32(), ast::StorageClass::kNone)));
  auto* compare = create<ast::BinaryExpression>(ast::BinaryOp::kLessThan,
                                                Expr(ref_loc, "z"), Expr(2));
  auto* continuing = Block(If(compare, Block()));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
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

  auto cont_loc = Source{{12, 34}};
  auto decl_loc = Source{{56, 78}};
  auto ref_loc = Source{{90, 12}};
  auto* body =
      Block(If(Expr(true), Block(Continue(cont_loc))),
            Decl(Var(decl_loc, "z", ty.i32(), ast::StorageClass::kNone)));

  auto* continuing = Block(Loop(Block(Assign(Expr(ref_loc, "z"), 2))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_FALSE(r()->Resolve()) << r()->error();
  EXPECT_EQ(r()->error(),
            R"(12:34 error: continue statement bypasses declaration of 'z'
56:78 note: identifier 'z' declared here
90:12 note: identifier 'z' referenced in continuing block here)");
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuing) {
  // loop  {
  //     loop {
  //         if (true) { continue; } // OK: not part of the outer loop
  //         break;
  //     }
  //     var z : i32;
  //     break;
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto* inner_loop = Loop(Block(    //
      If(true, Block(Continue())),  //
      Break()));
  auto* body = Block(inner_loop,                                          //
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),  //
                     Break());
  auto* continuing = Block(Assign("z", 2));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingSubscope) {
  // loop  {
  //     loop {
  //         if (true) { continue; } // OK: not part of the outer loop
  //         break;
  //     }
  //     var z : i32;
  //     break;
  //     continuing {
  //         if (true) {
  //             z = 2;
  //         }
  //     }
  // }

  auto* inner_loop = Loop(Block(If(true, Block(Continue())),  //
                                Break()));
  auto* body = Block(inner_loop,                                          //
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),  //
                     Break());
  auto* continuing = Block(If(Expr(true), Block(Assign("z", 2))));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest,
       Stmt_Loop_ContinueInNestedLoopBodyBeforeDecl_UsageInContinuingLoop) {
  // loop  {
  //     loop {
  //         if (true) { continue; } // OK: not part of the outer loop
  //         break;
  //     }
  //     var z : i32;
  //     break;
  //     continuing {
  //         loop {
  //             z = 2;
  //             break;
  //         }
  //     }
  // }

  auto* inner_loop = Loop(Block(If(true, Block(Continue())),  //
                                Break()));
  auto* body = Block(inner_loop,                                          //
                     Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),  //
                     Break());
  auto* continuing = Block(Loop(Block(Assign("z", 2),  //
                                      Break())));
  auto* loop_stmt = Loop(body, continuing);
  WrapInFunction(loop_stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInLoopBodyAfterDecl_UsageInContinuing) {
  // loop  {
  //     var z : i32;
  //     if (true) { continue; }
  //     break;
  //     continuing {
  //         z = 2;
  //     }
  // }

  auto error_loc = Source{{12, 34}};
  auto* body = Block(Decl(Var("z", ty.i32(), ast::StorageClass::kNone)),
                     If(true, Block(Continue())),  //
                     Break());
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
  //   if (false) { break; }
  //   continuing {
  //     loop {
  //       return;
  //     }
  //   }
  // }

  WrapInFunction(Loop(                   // outer loop
      Block(If(false, Block(Break()))),  //   outer loop block
      Block(Source{{56, 78}},            //   outer loop continuing block
            Loop(                        //     inner loop
                Block(                   //       inner loop block
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
          Discard(Source{{12, 34}}))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement)");
}

TEST_F(ResolverTest, Stmt_Loop_DiscardInContinuing_Indirect) {
  // loop {
  //   if (false) { break; }
  //   continuing {
  //     loop { discard; }
  //   }
  // }

  WrapInFunction(Loop(                   // outer loop
      Block(If(false, Block(Break()))),  //   outer loop block
      Block(Source{{56, 78}},            //   outer loop continuing block
            Loop(                        //     inner loop
                Block(                   //       inner loop block
                    Discard(Source{{12, 34}}))))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_Loop_DiscardInContinuing_Indirect_ViaCall) {
  // fn MayDiscard() { if (true) { discard; } }
  // fn F() { MayDiscard(); }
  // loop {
  //   continuing {
  //     loop { F(); }
  //   }
  // }

  Func("MayDiscard", {}, ty.void_(), {If(true, Block(Discard()))});
  Func("SomeFunc", {}, ty.void_(), {CallStmt(Call("MayDiscard"))});

  WrapInFunction(Loop(         // outer loop
      Block(),                 //   outer loop block
      Block(Source{{56, 78}},  //   outer loop continuing block
            Loop(              //     inner loop
                Block(         //       inner loop block
                    CallStmt(Call(Source{{12, 34}}, "SomeFunc")))))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: cannot call a function that may discard inside a continuing block
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
            Continue(Source{{12, 34}}))));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: continuing blocks must not contain a continue statement");
}

TEST_F(ResolverTest, Stmt_Loop_ContinueInContinuing_Indirect) {
  // loop {
  //   if (false) { break; }
  //   continuing {
  //     loop {
  //       if (false) { break; }
  //       continue;
  //     }
  //   }
  // }

  WrapInFunction(Loop(                        // outer loop
      Block(                                  //   outer loop block
          If(false, Block(Break()))),         //     if (false) { break; }
      Block(                                  //   outer loop continuing block
          Loop(                               //     inner loop
              Block(                          //       inner loop block
                  If(false, Block(Break())),  //          if (false) { break; }
                  Continue(Source{{12, 34}}))))));  //    continue

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_ForLoop_ReturnInContinuing_Direct) {
  // for(;; return) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr, Return(Source{{12, 34}}),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a return statement)");
}

TEST_F(ResolverTest, Stmt_ForLoop_ReturnInContinuing_Indirect) {
  // for(;; loop { return }) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr,
                     Loop(Source{{56, 78}},                  //
                          Block(Return(Source{{12, 34}}))),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a return statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_ForLoop_DiscardInContinuing_Direct) {
  // for(;; discard) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr, Discard(Source{{12, 34}}),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement)");
}

TEST_F(ResolverTest, Stmt_ForLoop_DiscardInContinuing_Indirect) {
  // for(;; loop { discard }) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr,
                     Loop(Source{{56, 78}},                   //
                          Block(Discard(Source{{12, 34}}))),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: continuing blocks must not contain a discard statement
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_ForLoop_DiscardInContinuing_Indirect_ViaCall) {
  // fn MayDiscard() { if (true) { discard; } }
  // fn F() { MayDiscard(); }
  // for(;; loop { F() }) {
  //   break;
  // }

  Func("MayDiscard", {}, ty.void_(), {If(true, Block(Discard()))});
  Func("F", {}, ty.void_(), {CallStmt(Call("MayDiscard"))});

  WrapInFunction(For(nullptr, nullptr,
                     Loop(Source{{56, 78}},                               //
                          Block(CallStmt(Call(Source{{12, 34}}, "F")))),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      R"(12:34 error: cannot call a function that may discard inside a continuing block
56:78 note: see continuing block here)");
}

TEST_F(ResolverTest, Stmt_ForLoop_ContinueInContinuing_Direct) {
  // for(;; continue) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr, Continue(Source{{12, 34}}),  //
                     Block(Break())));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: continuing blocks must not contain a continue statement");
}

TEST_F(ResolverTest, Stmt_ForLoop_ContinueInContinuing_Indirect) {
  // for(;; loop { if (false) { break; } continue }) {
  //   break;
  // }

  WrapInFunction(For(nullptr, nullptr,
                     Loop(                                    //
                         Block(If(false, Block(Break())),     //
                               Continue(Source{{12, 34}}))),  //
                     Block(Break())));

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
  WrapInFunction(Loop(Block(If(false, Block(Break())),  //
                            Continue(Source{{12, 34}}))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_ContinueNotInLoop) {
  WrapInFunction(Continue(Source{{12, 34}}));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: continue statement must be in a loop");
}

TEST_F(ResolverValidationTest, Stmt_BreakInLoop) {
  WrapInFunction(Loop(Block(Break(Source{{12, 34}}))));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInSwitch) {
  WrapInFunction(Loop(Block(Switch(Expr(1),               //
                                   Case(Expr(1),          //
                                        Block(Break())),  //
                                   DefaultCase()),        //
                            Break())));                   //
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueInContinuing) {
  auto* cont = Block(                           // continuing {
      If(true, Block(                           //   if(true) {
                   Break(Source{{12, 34}}))));  //     break;
                                                //   }
                                                // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseInContinuing) {
  auto* cont = Block(                     // continuing {
      If(true, Block(),                   //   if(true) {
         Block(                           //   } else {
             Break(Source{{12, 34}}))));  //     break;
                                          //   }
                                          // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverValidationTest, Stmt_BreakInContinuing) {
  auto* cont = Block(                   // continuing {
      Block(Break(Source{{12, 34}})));  //   break;
                                        // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "12:34 note: break statement is not directly in if statement block");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInIfInContinuing) {
  auto* cont = Block(                                      // continuing {
      If(true, Block(                                      //   if(true) {
                   If(Source{{56, 78}}, true,              //     if(true) {
                      Block(Break(Source{{12, 34}}))))));  //       break;
                                                           //     }
                                                           //   }
                                                           // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: if statement containing break statement is not directly in "
      "continuing block");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfTrueMultipleStmtsInContinuing) {
  auto* cont = Block(                             // continuing {
      If(true, Block(Source{{56, 78}},            //   if(true) {
                     Assign(Phony(), 1),          //     _ = 1;
                     Break(Source{{12, 34}}))));  //     break;
                                                  //   }
                                                  // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: if statement block contains multiple statements");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseMultipleStmtsInContinuing) {
  auto* cont = Block(                       // continuing {
      If(true, Block(),                     //   if(true) {
         Block(Source{{56, 78}},            //   } else {
               Assign(Phony(), 1),          //     _ = 1;
               Break(Source{{12, 34}}))));  //     break;
                                            //   }
                                            // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: if statement block contains multiple statements");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseIfInContinuing) {
  auto* cont = Block(                           // continuing {
      If(true, Block(),                         //   if(true) {
         If(Source{{56, 78}}, Expr(true),       //   } else if (true) {
            Block(Break(Source{{12, 34}})))));  //     break;
                                                //   }
                                                // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: else has condition");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfNonEmptyElseInContinuing) {
  auto* cont = Block(                     // continuing {
      If(true,                            //   if(true) {
         Block(Break(Source{{12, 34}})),  //     break;
         Block(Source{{56, 78}},          //   } else {
               Assign(Phony(), 1))));     //     _ = 1;
                                          //   }
                                          // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: non-empty false block");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfElseNonEmptyTrueInContinuing) {
  auto* cont = Block(                                  // continuing {
      If(true,                                         //   if(true) {
         Block(Source{{56, 78}}, Assign(Phony(), 1)),  //     _ = 1;
         Block(                                        //   } else {
             Break(Source{{12, 34}}))));               //     break;
                                                       //   }
                                                       // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: non-empty true block");
}

TEST_F(ResolverValidationTest, Stmt_BreakInIfInContinuingNotLast) {
  auto* cont = Block(                      // continuing {
      If(Source{{56, 78}}, true,           //   if(true) {
         Block(Break(Source{{12, 34}}))),  //     break;
                                           //   }
      Assign(Phony(), 1));                 //   _ = 1;
                                           // }
  WrapInFunction(Loop(Block(), cont));
  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: break statement in a continuing block must be the single "
      "statement of an if statement's true or false block, and that if "
      "statement must be the last statement of the continuing block\n"
      "56:78 note: if statement containing break statement is not the last "
      "statement of the continuing block");
}

TEST_F(ResolverValidationTest, Stmt_BreakNotInLoopOrSwitch) {
  WrapInFunction(Break(Source{{12, 34}}));
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

TEST_F(ResolverValidationTest, NonPOTStructMemberAlignAttribute) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberAlign(Source{{12, 34}}, 3)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: align value must be a positive, power-of-two integer");
}

TEST_F(ResolverValidationTest, ZeroStructMemberAlignAttribute) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberAlign(Source{{12, 34}}, 0)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: align value must be a positive, power-of-two integer");
}

TEST_F(ResolverValidationTest, ZeroStructMemberSizeAttribute) {
  Structure("S", {
                     Member("a", ty.f32(), {MemberSize(Source{{12, 34}}, 0)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: size must be at least as big as the type's size (4)");
}

TEST_F(ResolverValidationTest, OffsetAndSizeAttribute) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberSize(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset attributes cannot be used with align or size "
            "attributes");
}

TEST_F(ResolverValidationTest, OffsetAndAlignAttribute) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberAlign(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset attributes cannot be used with align or size "
            "attributes");
}

TEST_F(ResolverValidationTest, OffsetAndAlignAndSizeAttribute) {
  Structure("S", {
                     Member(Source{{12, 34}}, "a", ty.f32(),
                            {MemberOffset(0), MemberAlign(4), MemberSize(4)}),
                 });

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: offset attributes cannot be used with align or size "
            "attributes");
}

TEST_F(ResolverTest, Expr_Constructor_Cast_Pointer) {
  auto* vf = Var("vf", ty.f32());
  auto* c =
      Construct(Source{{12, 34}}, ty.pointer<i32>(ast::StorageClass::kFunction),
                ExprList(vf));
  auto* ip = Let("ip", ty.pointer<i32>(ast::StorageClass::kFunction), c);
  WrapInFunction(Decl(vf), Decl(ip));

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: type is not constructible");
}

}  // namespace
}  // namespace tint::resolver

TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeStmt);
TINT_INSTANTIATE_TYPEINFO(tint::resolver::FakeExpr);
