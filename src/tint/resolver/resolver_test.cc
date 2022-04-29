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

#include "src/tint/resolver/resolver.h"

#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/builtin_texture_helper_test.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/float_literal_expression.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint::resolver {
namespace {

// Helpers and typedefs
template <typename T>
using DataType = builder::DataType<T>;
template <int N, typename T>
using vec = builder::vec<N, T>;
template <typename T>
using vec2 = builder::vec2<T>;
template <typename T>
using vec3 = builder::vec3<T>;
template <typename T>
using vec4 = builder::vec4<T>;
template <int N, int M, typename T>
using mat = builder::mat<N, M, T>;
template <typename T>
using mat2x2 = builder::mat2x2<T>;
template <typename T>
using mat2x3 = builder::mat2x3<T>;
template <typename T>
using mat3x2 = builder::mat3x2<T>;
template <typename T>
using mat3x3 = builder::mat3x3<T>;
template <typename T>
using mat4x4 = builder::mat4x4<T>;
template <typename T, int ID = 0>
using alias = builder::alias<T, ID>;
template <typename T>
using alias1 = builder::alias1<T>;
template <typename T>
using alias2 = builder::alias2<T>;
template <typename T>
using alias3 = builder::alias3<T>;
using f32 = builder::f32;
using i32 = builder::i32;
using u32 = builder::u32;
using Op = ast::BinaryOp;

TEST_F(ResolverTest, Stmt_Assign) {
  auto* v = Var("v", ty.f32());
  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);

  auto* assign = Assign(lhs, rhs);
  WrapInFunction(v, assign);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
}

TEST_F(ResolverTest, Stmt_Case) {
  auto* v = Var("v", ty.f32());
  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);

  auto* assign = Assign(lhs, rhs);
  auto* block = Block(assign);
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteralExpression>(3));
  auto* cse = create<ast::CaseStatement>(lit, block);
  auto* cond_var = Var("c", ty.i32());
  auto* sw = Switch(cond_var, cse, DefaultCase());
  WrapInFunction(v, cond_var, sw);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(BlockOf(assign), block);
}

TEST_F(ResolverTest, Stmt_Block) {
  auto* v = Var("v", ty.f32());
  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);

  auto* assign = Assign(lhs, rhs);
  auto* block = Block(assign);
  WrapInFunction(v, block);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(BlockOf(lhs), block);
  EXPECT_EQ(BlockOf(rhs), block);
  EXPECT_EQ(BlockOf(assign), block);
}

TEST_F(ResolverTest, Stmt_If) {
  auto* v = Var("v", ty.f32());
  auto* else_lhs = Expr("v");
  auto* else_rhs = Expr(2.3f);

  auto* else_body = Block(Assign(else_lhs, else_rhs));

  auto* else_cond = Expr(true);
  auto* else_stmt = If(else_cond, else_body);

  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);

  auto* assign = Assign(lhs, rhs);
  auto* body = Block(assign);
  auto* cond = Expr(true);
  auto* stmt = If(cond, body, else_stmt);
  WrapInFunction(v, stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition), nullptr);
  ASSERT_NE(TypeOf(else_lhs), nullptr);
  ASSERT_NE(TypeOf(else_rhs), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition)->Is<sem::Bool>());
  EXPECT_TRUE(TypeOf(else_lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(else_rhs)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(lhs), assign);
  EXPECT_EQ(StmtOf(rhs), assign);
  EXPECT_EQ(StmtOf(cond), stmt);
  EXPECT_EQ(StmtOf(else_cond), else_stmt);
  EXPECT_EQ(BlockOf(lhs), body);
  EXPECT_EQ(BlockOf(rhs), body);
  EXPECT_EQ(BlockOf(else_lhs), else_body);
  EXPECT_EQ(BlockOf(else_rhs), else_body);
}

TEST_F(ResolverTest, Stmt_Loop) {
  auto* v = Var("v", ty.f32());
  auto* body_lhs = Expr("v");
  auto* body_rhs = Expr(2.3f);

  auto* body = Block(Assign(body_lhs, body_rhs), Break());
  auto* continuing_lhs = Expr("v");
  auto* continuing_rhs = Expr(2.3f);

  auto* continuing = Block(Assign(continuing_lhs, continuing_rhs));
  auto* stmt = Loop(body, continuing);
  WrapInFunction(v, stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(body_lhs), nullptr);
  ASSERT_NE(TypeOf(body_rhs), nullptr);
  ASSERT_NE(TypeOf(continuing_lhs), nullptr);
  ASSERT_NE(TypeOf(continuing_rhs), nullptr);
  EXPECT_TRUE(TypeOf(body_lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(body_rhs)->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(continuing_lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(continuing_rhs)->Is<sem::F32>());
  EXPECT_EQ(BlockOf(body_lhs), body);
  EXPECT_EQ(BlockOf(body_rhs), body);
  EXPECT_EQ(BlockOf(continuing_lhs), continuing);
  EXPECT_EQ(BlockOf(continuing_rhs), continuing);
}

TEST_F(ResolverTest, Stmt_Return) {
  auto* cond = Expr(2);

  auto* ret = Return(cond);
  Func("test", {}, ty.i32(), {ret}, {});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cond), nullptr);
  EXPECT_TRUE(TypeOf(cond)->Is<sem::I32>());
}

TEST_F(ResolverTest, Stmt_Return_WithoutValue) {
  auto* ret = Return();
  WrapInFunction(ret);

  EXPECT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, Stmt_Switch) {
  auto* v = Var("v", ty.f32());
  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);
  auto* case_block = Block(Assign(lhs, rhs));
  auto* stmt = Switch(Expr(2), Case(Expr(3), case_block), DefaultCase());
  WrapInFunction(v, stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(stmt->condition)->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(BlockOf(lhs), case_block);
  EXPECT_EQ(BlockOf(rhs), case_block);
}

TEST_F(ResolverTest, Stmt_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.void_(), {Return()}, ast::AttributeList{});

  auto* expr = Call("my_func");

  auto* call = CallStmt(expr);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::Void>());
  EXPECT_EQ(StmtOf(expr), call);
}

TEST_F(ResolverTest, Stmt_VariableDecl) {
  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor;

  auto* decl = Decl(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<sem::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_Alias) {
  auto* my_int = Alias("MyInt", ty.i32());
  auto* var = Var("my_var", ty.Of(my_int), ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor;

  auto* decl = Decl(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<sem::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScope) {
  auto* init = Expr(2);
  Global("my_var", ty.i32(), ast::StorageClass::kPrivate, init);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<sem::I32>());
  EXPECT_EQ(StmtOf(init), nullptr);
}

TEST_F(ResolverTest, Stmt_VariableDecl_OuterScopeAfterInnerScope) {
  // fn func_i32() {
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
  auto* foo_i32_init = foo_i32->constructor;
  auto* foo_i32_decl = Decl(foo_i32);

  // Reference "foo" inside the block
  auto* bar_i32 = Var("bar", ty.i32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_i32_init = bar_i32->constructor;
  auto* bar_i32_decl = Decl(bar_i32);

  auto* inner = Block(foo_i32_decl, bar_i32_decl);

  // Declare f32 "foo" at function scope
  auto* foo_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* foo_f32_init = foo_f32->constructor;
  auto* foo_f32_decl = Decl(foo_f32);

  // Reference "foo" at function scope
  auto* bar_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_f32_init = bar_f32->constructor;
  auto* bar_f32_decl = Decl(bar_f32);

  Func("func", params, ty.void_(), {inner, foo_f32_decl, bar_f32_decl},
       ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(foo_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_i32_init)->Is<sem::I32>());
  ASSERT_NE(TypeOf(foo_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(foo_f32_init)->Is<sem::F32>());
  ASSERT_NE(TypeOf(bar_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_i32_init)->UnwrapRef()->Is<sem::I32>());
  ASSERT_NE(TypeOf(bar_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(bar_f32_init)->UnwrapRef()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(foo_i32_init), foo_i32_decl);
  EXPECT_EQ(StmtOf(bar_i32_init), bar_i32_decl);
  EXPECT_EQ(StmtOf(foo_f32_init), foo_f32_decl);
  EXPECT_EQ(StmtOf(bar_f32_init), bar_f32_decl);
  EXPECT_TRUE(CheckVarUsers(foo_i32, {bar_i32->constructor}));
  EXPECT_TRUE(CheckVarUsers(foo_f32, {bar_f32->constructor}));
  ASSERT_NE(VarOf(bar_i32->constructor), nullptr);
  EXPECT_EQ(VarOf(bar_i32->constructor)->Declaration(), foo_i32);
  ASSERT_NE(VarOf(bar_f32->constructor), nullptr);
  EXPECT_EQ(VarOf(bar_f32->constructor)->Declaration(), foo_f32);
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScopeAfterFunctionScope) {
  // fn func_i32() {
  //   var foo : i32 = 2;
  // }
  // var foo : f32 = 2.0;
  // fn func_f32() {
  //   var bar : f32 = foo;
  // }

  ast::VariableList params;

  // Declare i32 "foo" inside a function
  auto* fn_i32 = Var("foo", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* fn_i32_init = fn_i32->constructor;
  auto* fn_i32_decl = Decl(fn_i32);
  Func("func_i32", params, ty.void_(), {fn_i32_decl}, ast::AttributeList{});

  // Declare f32 "foo" at module scope
  auto* mod_f32 = Var("foo", ty.f32(), ast::StorageClass::kPrivate, Expr(2.f));
  auto* mod_init = mod_f32->constructor;
  AST().AddGlobalVariable(mod_f32);

  // Reference "foo" in another function
  auto* fn_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* fn_f32_init = fn_f32->constructor;
  auto* fn_f32_decl = Decl(fn_f32);
  Func("func_f32", params, ty.void_(), {fn_f32_decl}, ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(mod_init), nullptr);
  EXPECT_TRUE(TypeOf(mod_init)->Is<sem::F32>());
  ASSERT_NE(TypeOf(fn_i32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_i32_init)->Is<sem::I32>());
  ASSERT_NE(TypeOf(fn_f32_init), nullptr);
  EXPECT_TRUE(TypeOf(fn_f32_init)->UnwrapRef()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(fn_i32_init), fn_i32_decl);
  EXPECT_EQ(StmtOf(mod_init), nullptr);
  EXPECT_EQ(StmtOf(fn_f32_init), fn_f32_decl);
  EXPECT_TRUE(CheckVarUsers(fn_i32, {}));
  EXPECT_TRUE(CheckVarUsers(mod_f32, {fn_f32->constructor}));
  ASSERT_NE(VarOf(fn_f32->constructor), nullptr);
  EXPECT_EQ(VarOf(fn_f32->constructor)->Declaration(), mod_f32);
}

TEST_F(ResolverTest, ArraySize_UnsignedLiteral) {
  // var<private> a : array<f32, 10u>;
  auto* a =
      Global("a", ty.array(ty.f32(), Expr(10u)), ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(a), nullptr);
  auto* ref = TypeOf(a)->As<sem::Reference>();
  ASSERT_NE(ref, nullptr);
  auto* ary = ref->StoreType()->As<sem::Array>();
  EXPECT_EQ(ary->Count(), 10u);
}

TEST_F(ResolverTest, ArraySize_SignedLiteral) {
  // var<private> a : array<f32, 10>;
  auto* a =
      Global("a", ty.array(ty.f32(), Expr(10)), ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(a), nullptr);
  auto* ref = TypeOf(a)->As<sem::Reference>();
  ASSERT_NE(ref, nullptr);
  auto* ary = ref->StoreType()->As<sem::Array>();
  EXPECT_EQ(ary->Count(), 10u);
}

TEST_F(ResolverTest, ArraySize_UnsignedConstant) {
  // let size = 0u;
  // var<private> a : array<f32, 10u>;
  GlobalConst("size", nullptr, Expr(10u));
  auto* a = Global("a", ty.array(ty.f32(), Expr("size")),
                   ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(a), nullptr);
  auto* ref = TypeOf(a)->As<sem::Reference>();
  ASSERT_NE(ref, nullptr);
  auto* ary = ref->StoreType()->As<sem::Array>();
  EXPECT_EQ(ary->Count(), 10u);
}

TEST_F(ResolverTest, ArraySize_SignedConstant) {
  // let size = 0;
  // var<private> a : array<f32, 10>;
  GlobalConst("size", nullptr, Expr(10));
  auto* a = Global("a", ty.array(ty.f32(), Expr("size")),
                   ast::StorageClass::kPrivate);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(a), nullptr);
  auto* ref = TypeOf(a)->As<sem::Reference>();
  ASSERT_NE(ref, nullptr);
  auto* ary = ref->StoreType()->As<sem::Array>();
  EXPECT_EQ(ary->Count(), 10u);
}

TEST_F(ResolverTest, Expr_Bitcast) {
  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  auto* bitcast = create<ast::BitcastExpression>(ty.f32(), Expr("name"));
  WrapInFunction(bitcast);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(bitcast), nullptr);
  EXPECT_TRUE(TypeOf(bitcast)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), {Return(0.0f)}, ast::AttributeList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call_InBinaryOp) {
  ast::VariableList params;
  Func("func", params, ty.f32(), {Return(0.0f)}, ast::AttributeList{});

  auto* expr = Add(Call("func"), Call("func"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call_WithParams) {
  Func("my_func", {Param(Sym(), ty.f32())}, ty.f32(),
       {
           Return(1.2f),
       });

  auto* param = Expr(2.4f);

  auto* call = Call("my_func", param);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(param), nullptr);
  EXPECT_TRUE(TypeOf(param)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call_Builtin) {
  auto* call = Call("round", 2.4f);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Cast) {
  Global("name", ty.f32(), ast::StorageClass::kPrivate);

  auto* cast = Construct(ty.f32(), "name");
  WrapInFunction(cast);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(cast), nullptr);
  EXPECT_TRUE(TypeOf(cast)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Scalar) {
  auto* s = Expr(1.0f);
  WrapInFunction(s);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(s), nullptr);
  EXPECT_TRUE(TypeOf(s)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Constructor_Type_Vec2) {
  auto* tc = vec2<f32>(1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 2u);
}

TEST_F(ResolverTest, Expr_Constructor_Type_Vec3) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 3u);
}

TEST_F(ResolverTest, Expr_Constructor_Type_Vec4) {
  auto* tc = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->Width(), 4u);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalVariable) {
  auto* my_var = Global("my_var", ty.f32(), ast::StorageClass::kPrivate);

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  ASSERT_TRUE(TypeOf(ident)->Is<sem::Reference>());
  EXPECT_TRUE(TypeOf(ident)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
  ASSERT_NE(VarOf(ident), nullptr);
  EXPECT_EQ(VarOf(ident)->Declaration(), my_var);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalConstant) {
  auto* my_var = GlobalConst("my_var", ty.f32(), Construct(ty.f32()));

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<sem::F32>());
  EXPECT_TRUE(CheckVarUsers(my_var, {ident}));
  ASSERT_NE(VarOf(ident), nullptr);
  EXPECT_EQ(VarOf(ident)->Declaration(), my_var);
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable_Const) {
  auto* my_var_a = Expr("my_var");
  auto* var = Let("my_var", ty.f32(), Construct(ty.f32()));
  auto* decl = Decl(Var("b", ty.f32(), ast::StorageClass::kNone, my_var_a));

  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           decl,
       },
       ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(my_var_a), decl);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a}));
  ASSERT_NE(VarOf(my_var_a), nullptr);
  EXPECT_EQ(VarOf(my_var_a)->Declaration(), var);
}

TEST_F(ResolverTest, IndexAccessor_Dynamic_Ref_F32) {
  // var a : array<bool, 10> = 0;
  // var idx : f32 = f32();
  // var f : f32 = a[idx];
  auto* a = Var("a", ty.array<bool, 10>(), array<bool, 10>());
  auto* idx = Var("idx", ty.f32(), Construct(ty.f32()));
  auto* f = Var("f", ty.f32(), IndexAccessor("a", Expr(Source{{12, 34}}, idx)));
  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Decl(a),
           Decl(idx),
           Decl(f),
       },
       ast::AttributeList{});

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: index must be of type 'i32' or 'u32', found: 'f32'");
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = Assign(my_var_a, my_var_b);

  auto* var = Var("my_var", ty.f32());

  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           assign,
       },
       ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  ASSERT_TRUE(TypeOf(my_var_a)->Is<sem::Reference>());
  EXPECT_TRUE(TypeOf(my_var_a)->UnwrapRef()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(my_var_a), assign);
  ASSERT_NE(TypeOf(my_var_b), nullptr);
  ASSERT_TRUE(TypeOf(my_var_b)->Is<sem::Reference>());
  EXPECT_TRUE(TypeOf(my_var_b)->UnwrapRef()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(my_var_b), assign);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a, my_var_b}));
  ASSERT_NE(VarOf(my_var_a), nullptr);
  EXPECT_EQ(VarOf(my_var_a)->Declaration(), var);
  ASSERT_NE(VarOf(my_var_b), nullptr);
  EXPECT_EQ(VarOf(my_var_b)->Declaration(), var);
}

TEST_F(ResolverTest, Expr_Identifier_Function_Ptr) {
  auto* v = Expr("v");
  auto* p = Expr("p");
  auto* v_decl = Decl(Var("v", ty.f32()));
  auto* p_decl = Decl(
      Let("p", ty.pointer<f32>(ast::StorageClass::kFunction), AddressOf(v)));
  auto* assign = Assign(Deref(p), 1.23f);
  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           v_decl,
           p_decl,
           assign,
       },
       ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(v), nullptr);
  ASSERT_TRUE(TypeOf(v)->Is<sem::Reference>());
  EXPECT_TRUE(TypeOf(v)->UnwrapRef()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(v), p_decl);
  ASSERT_NE(TypeOf(p), nullptr);
  ASSERT_TRUE(TypeOf(p)->Is<sem::Pointer>());
  EXPECT_TRUE(TypeOf(p)->UnwrapPtr()->Is<sem::F32>());
  EXPECT_EQ(StmtOf(p), assign);
}

TEST_F(ResolverTest, Expr_Call_Function) {
  Func("my_func", ast::VariableList{}, ty.f32(), {Return(0.0f)},
       ast::AttributeList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Identifier_Unknown) {
  auto* a = Expr("a");
  WrapInFunction(a);

  EXPECT_FALSE(r()->Resolve());
}

TEST_F(ResolverTest, Function_Parameters) {
  auto* param_a = Param("a", ty.f32());
  auto* param_b = Param("b", ty.i32());
  auto* param_c = Param("c", ty.u32());

  auto* func = Func("my_func",
                    ast::VariableList{
                        param_a,
                        param_b,
                        param_c,
                    },
                    ty.void_(), {});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);
  EXPECT_EQ(func_sem->Parameters().size(), 3u);
  EXPECT_TRUE(func_sem->Parameters()[0]->Type()->Is<sem::F32>());
  EXPECT_TRUE(func_sem->Parameters()[1]->Type()->Is<sem::I32>());
  EXPECT_TRUE(func_sem->Parameters()[2]->Type()->Is<sem::U32>());
  EXPECT_EQ(func_sem->Parameters()[0]->Declaration(), param_a);
  EXPECT_EQ(func_sem->Parameters()[1]->Declaration(), param_b);
  EXPECT_EQ(func_sem->Parameters()[2]->Declaration(), param_c);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables) {
  auto* s = Structure("S", {Member("m", ty.u32())});

  auto* sb_var = Global("sb_var", ty.Of(s), ast::StorageClass::kStorage,
                        ast::Access::kReadWrite,
                        ast::AttributeList{
                            create<ast::BindingAttribute>(0),
                            create<ast::GroupAttribute>(0),
                        });
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    {
                        Assign("wg_var", "wg_var"),
                        Assign("sb_var", "sb_var"),
                        Assign("priv_var", "priv_var"),
                    });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);
  EXPECT_EQ(func_sem->Parameters().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());

  const auto& vars = func_sem->TransitivelyReferencedGlobals();
  ASSERT_EQ(vars.size(), 3u);
  EXPECT_EQ(vars[0]->Declaration(), wg_var);
  EXPECT_EQ(vars[1]->Declaration(), sb_var);
  EXPECT_EQ(vars[2]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables_SubFunction) {
  auto* s = Structure("S", {Member("m", ty.u32())});

  auto* sb_var = Global("sb_var", ty.Of(s), ast::StorageClass::kStorage,
                        ast::Access::kReadWrite,
                        ast::AttributeList{
                            create<ast::BindingAttribute>(0),
                            create<ast::GroupAttribute>(0),
                        });
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  Func("my_func", ast::VariableList{}, ty.f32(),
       {Assign("wg_var", "wg_var"), Assign("sb_var", "sb_var"),
        Assign("priv_var", "priv_var"), Return(0.0f)},
       ast::AttributeList{});

  auto* func2 = Func("func", ast::VariableList{}, ty.void_(),
                     {
                         WrapInStatement(Call("my_func")),
                     },
                     ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func2_sem = Sem().Get(func2);
  ASSERT_NE(func2_sem, nullptr);
  EXPECT_EQ(func2_sem->Parameters().size(), 0u);

  const auto& vars = func2_sem->TransitivelyReferencedGlobals();
  ASSERT_EQ(vars.size(), 3u);
  EXPECT_EQ(vars[0]->Declaration(), wg_var);
  EXPECT_EQ(vars[1]->Declaration(), sb_var);
  EXPECT_EQ(vars[2]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_NotRegisterFunctionVariable) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    {
                        Decl(Var("var", ty.f32())),
                        Assign("var", 1.f),
                    });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());
}

TEST_F(ResolverTest, Function_NotRegisterFunctionConstant) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    {
                        Decl(Let("var", ty.f32(), Construct(ty.f32()))),
                    });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());
}

TEST_F(ResolverTest, Function_NotRegisterFunctionParams) {
  auto* func = Func("my_func", {Let("var", ty.f32(), Construct(ty.f32()))},
                    ty.void_(), {});
  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->TransitivelyReferencedGlobals().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());
}

TEST_F(ResolverTest, Function_CallSites) {
  auto* foo = Func("foo", ast::VariableList{}, ty.void_(), {});

  auto* call_1 = Call("foo");
  auto* call_2 = Call("foo");
  auto* bar = Func("bar", ast::VariableList{}, ty.void_(),
                   {
                       CallStmt(call_1),
                       CallStmt(call_2),
                   });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* foo_sem = Sem().Get(foo);
  ASSERT_NE(foo_sem, nullptr);
  ASSERT_EQ(foo_sem->CallSites().size(), 2u);
  EXPECT_EQ(foo_sem->CallSites()[0]->Declaration(), call_1);
  EXPECT_EQ(foo_sem->CallSites()[1]->Declaration(), call_2);

  auto* bar_sem = Sem().Get(bar);
  ASSERT_NE(bar_sem, nullptr);
  EXPECT_EQ(bar_sem->CallSites().size(), 0u);
}

TEST_F(ResolverTest, Function_WorkgroupSize_NotSet) {
  // @stage(compute) @workgroup_size(1)
  // fn main() {}
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {}, {});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 1u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 1u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 1u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Literals) {
  // @stage(compute) @workgroup_size(8, 2, 3)
  // fn main() {}
  auto* func =
      Func("main", ast::VariableList{}, ty.void_(), {},
           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(8, 2, 3)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 8u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 2u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 3u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Consts) {
  // let width = 16;
  // let height = 8;
  // let depth = 2;
  // @stage(compute) @workgroup_size(width, height, depth)
  // fn main() {}
  GlobalConst("width", ty.i32(), Expr(16));
  GlobalConst("height", ty.i32(), Expr(8));
  GlobalConst("depth", ty.i32(), Expr(2));
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize("width", "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 16u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 8u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 2u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Consts_NestedInitializer) {
  // let width = i32(i32(i32(8)));
  // let height = i32(i32(i32(4)));
  // @stage(compute) @workgroup_size(width, height)
  // fn main() {}
  GlobalConst("width", ty.i32(),
              Construct(ty.i32(), Construct(ty.i32(), Construct(ty.i32(), 8))));
  GlobalConst("height", ty.i32(),
              Construct(ty.i32(), Construct(ty.i32(), Construct(ty.i32(), 4))));
  auto* func = Func(
      "main", ast::VariableList{}, ty.void_(), {},
      {Stage(ast::PipelineStage::kCompute), WorkgroupSize("width", "height")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 8u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 4u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 1u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts) {
  // @id(0) override width = 16;
  // @id(1) override height = 8;
  // @id(2) override depth = 2;
  // @stage(compute) @workgroup_size(width, height, depth)
  // fn main() {}
  auto* width = Override("width", ty.i32(), Expr(16), {Id(0)});
  auto* height = Override("height", ty.i32(), Expr(8), {Id(1)});
  auto* depth = Override("depth", ty.i32(), Expr(2), {Id(2)});
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize("width", "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 16u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 8u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 2u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, width);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, height);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, depth);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts_NoInit) {
  // @id(0) override width : i32;
  // @id(1) override height : i32;
  // @id(2) override depth : i32;
  // @stage(compute) @workgroup_size(width, height, depth)
  // fn main() {}
  auto* width = Override("width", ty.i32(), nullptr, {Id(0)});
  auto* height = Override("height", ty.i32(), nullptr, {Id(1)});
  auto* depth = Override("depth", ty.i32(), nullptr, {Id(2)});
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize("width", "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 0u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 0u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 0u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, width);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, height);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, depth);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Mixed) {
  // @id(1) override height = 2;
  // let depth = 3;
  // @stage(compute) @workgroup_size(8, height, depth)
  // fn main() {}
  auto* height = Override("height", ty.i32(), Expr(2), {Id(0)});
  GlobalConst("depth", ty.i32(), Expr(3));
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize(8, "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->WorkgroupSize()[0].value, 8u);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].value, 2u);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].value, 3u);
  EXPECT_EQ(func_sem->WorkgroupSize()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->WorkgroupSize()[1].overridable_const, height);
  EXPECT_EQ(func_sem->WorkgroupSize()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct) {
  auto* st = Structure("S", {Member("first_member", ty.i32()),
                             Member("second_member", ty.f32())});
  Global("my_struct", ty.Of(st), ast::StorageClass::kPrivate);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Reference>());

  auto* ref = TypeOf(mem)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
  auto* sma = Sem().Get(mem)->As<sem::StructMemberAccess>();
  ASSERT_NE(sma, nullptr);
  EXPECT_TRUE(sma->Member()->Type()->Is<sem::F32>());
  EXPECT_EQ(sma->Member()->Index(), 1u);
  EXPECT_EQ(sma->Member()->Declaration()->symbol,
            Symbols().Get("second_member"));
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct_Alias) {
  auto* st = Structure("S", {Member("first_member", ty.i32()),
                             Member("second_member", ty.f32())});
  auto* alias = Alias("alias", ty.Of(st));
  Global("my_struct", ty.Of(alias), ast::StorageClass::kPrivate);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Reference>());

  auto* ref = TypeOf(mem)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
  auto* sma = Sem().Get(mem)->As<sem::StructMemberAccess>();
  ASSERT_NE(sma, nullptr);
  EXPECT_TRUE(sma->Member()->Type()->Is<sem::F32>());
  EXPECT_EQ(sma->Member()->Index(), 1u);
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle) {
  Global("my_vec", ty.vec4<f32>(), ast::StorageClass::kPrivate);

  auto* mem = MemberAccessor("my_vec", "xzyw");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(mem)->As<sem::Vector>()->Width(), 4u);
  ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
  EXPECT_THAT(Sem().Get(mem)->As<sem::Swizzle>()->Indices(),
              ElementsAre(0, 2, 1, 3));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* mem = MemberAccessor("my_vec", "b");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Reference>());

  auto* ref = TypeOf(mem)->As<sem::Reference>();
  ASSERT_TRUE(ref->StoreType()->Is<sem::F32>());
  ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
  EXPECT_THAT(Sem().Get(mem)->As<sem::Swizzle>()->Indices(), ElementsAre(2));
}

TEST_F(ResolverTest, Expr_Accessor_MultiLevel) {
  // struct b {
  //   vec4<f32> foo
  // }
  // struct A {
  //   array<b, 3> mem
  // }
  // var c : A
  // c.mem[0].foo.yx
  //   -> vec2<f32>
  //
  // fn f() {
  //   c.mem[0].foo
  // }
  //

  auto* stB = Structure("B", {Member("foo", ty.vec4<f32>())});
  auto* stA = Structure("A", {Member("mem", ty.array(ty.Of(stB), 3))});
  Global("c", ty.Of(stA), ast::StorageClass::kPrivate);

  auto* mem = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0), "foo"),
      "yx");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(mem)->As<sem::Vector>()->Width(), 2u);
  ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_InBinaryOp) {
  auto* st = Structure("S", {Member("first_member", ty.f32()),
                             Member("second_member", ty.f32())});
  Global("my_struct", ty.Of(st), ast::StorageClass::kPrivate);

  auto* expr = Add(MemberAccessor("my_struct", "first_member"),
                   MemberAccessor("my_struct", "second_member"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

namespace ExprBinaryTest {

template <typename T, int ID>
struct Aliased {
  using type = alias<T, ID>;
};

template <int N, typename T, int ID>
struct Aliased<vec<N, T>, ID> {
  using type = vec<N, alias<T, ID>>;
};

template <int N, int M, typename T, int ID>
struct Aliased<mat<N, M, T>, ID> {
  using type = mat<N, M, alias<T, ID>>;
};

struct Params {
  ast::BinaryOp op;
  builder::ast_type_func_ptr create_lhs_type;
  builder::ast_type_func_ptr create_rhs_type;
  builder::ast_type_func_ptr create_lhs_alias_type;
  builder::ast_type_func_ptr create_rhs_alias_type;
  builder::sem_type_func_ptr create_result_type;
};

template <typename LHS, typename RHS, typename RES>
constexpr Params ParamsFor(ast::BinaryOp op) {
  return Params{op,
                DataType<LHS>::AST,
                DataType<RHS>::AST,
                DataType<typename Aliased<LHS, 0>::type>::AST,
                DataType<typename Aliased<RHS, 1>::type>::AST,
                DataType<RES>::Sem};
}

static constexpr ast::BinaryOp all_ops[] = {
    ast::BinaryOp::kAnd,
    ast::BinaryOp::kOr,
    ast::BinaryOp::kXor,
    ast::BinaryOp::kLogicalAnd,
    ast::BinaryOp::kLogicalOr,
    ast::BinaryOp::kEqual,
    ast::BinaryOp::kNotEqual,
    ast::BinaryOp::kLessThan,
    ast::BinaryOp::kGreaterThan,
    ast::BinaryOp::kLessThanEqual,
    ast::BinaryOp::kGreaterThanEqual,
    ast::BinaryOp::kShiftLeft,
    ast::BinaryOp::kShiftRight,
    ast::BinaryOp::kAdd,
    ast::BinaryOp::kSubtract,
    ast::BinaryOp::kMultiply,
    ast::BinaryOp::kDivide,
    ast::BinaryOp::kModulo,
};

static constexpr builder::ast_type_func_ptr all_create_type_funcs[] = {
    DataType<bool>::AST,         //
    DataType<u32>::AST,          //
    DataType<i32>::AST,          //
    DataType<f32>::AST,          //
    DataType<vec3<bool>>::AST,   //
    DataType<vec3<i32>>::AST,    //
    DataType<vec3<u32>>::AST,    //
    DataType<vec3<f32>>::AST,    //
    DataType<mat3x3<f32>>::AST,  //
    DataType<mat2x3<f32>>::AST,  //
    DataType<mat3x2<f32>>::AST   //
};

// A list of all valid test cases for 'lhs op rhs', except that for vecN and
// matNxN, we only test N=3.
static constexpr Params all_valid_cases[] = {
    // Logical expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#logical-expr

    // Binary logical expressions
    ParamsFor<bool, bool, bool>(Op::kLogicalAnd),
    ParamsFor<bool, bool, bool>(Op::kLogicalOr),

    ParamsFor<bool, bool, bool>(Op::kAnd),
    ParamsFor<bool, bool, bool>(Op::kOr),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kAnd),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kOr),

    // Arithmetic expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#arithmetic-expr

    // Binary arithmetic expressions over scalars
    ParamsFor<i32, i32, i32>(Op::kAdd),
    ParamsFor<i32, i32, i32>(Op::kSubtract),
    ParamsFor<i32, i32, i32>(Op::kMultiply),
    ParamsFor<i32, i32, i32>(Op::kDivide),
    ParamsFor<i32, i32, i32>(Op::kModulo),

    ParamsFor<u32, u32, u32>(Op::kAdd),
    ParamsFor<u32, u32, u32>(Op::kSubtract),
    ParamsFor<u32, u32, u32>(Op::kMultiply),
    ParamsFor<u32, u32, u32>(Op::kDivide),
    ParamsFor<u32, u32, u32>(Op::kModulo),

    ParamsFor<f32, f32, f32>(Op::kAdd),
    ParamsFor<f32, f32, f32>(Op::kSubtract),
    ParamsFor<f32, f32, f32>(Op::kMultiply),
    ParamsFor<f32, f32, f32>(Op::kDivide),
    ParamsFor<f32, f32, f32>(Op::kModulo),

    // Binary arithmetic expressions over vectors
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kAdd),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kSubtract),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kMultiply),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kDivide),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kModulo),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kAdd),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kSubtract),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kMultiply),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kDivide),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kModulo),

    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kAdd),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kSubtract),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kDivide),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<f32>>(Op::kModulo),

    // Binary arithmetic expressions with mixed scalar and vector operands
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kAdd),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kSubtract),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kMultiply),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kDivide),
    ParamsFor<vec3<i32>, i32, vec3<i32>>(Op::kModulo),

    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kAdd),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kSubtract),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kMultiply),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kDivide),
    ParamsFor<i32, vec3<i32>, vec3<i32>>(Op::kModulo),

    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kAdd),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kSubtract),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kMultiply),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kDivide),
    ParamsFor<vec3<u32>, u32, vec3<u32>>(Op::kModulo),

    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kAdd),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kSubtract),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kMultiply),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kDivide),
    ParamsFor<u32, vec3<u32>, vec3<u32>>(Op::kModulo),

    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kAdd),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kSubtract),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kDivide),
    ParamsFor<vec3<f32>, f32, vec3<f32>>(Op::kModulo),

    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kAdd),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kSubtract),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kDivide),
    ParamsFor<f32, vec3<f32>, vec3<f32>>(Op::kModulo),

    // Matrix arithmetic
    ParamsFor<mat2x3<f32>, f32, mat2x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, f32, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, f32, mat3x3<f32>>(Op::kMultiply),

    ParamsFor<f32, mat2x3<f32>, mat2x3<f32>>(Op::kMultiply),
    ParamsFor<f32, mat3x2<f32>, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<f32, mat3x3<f32>, mat3x3<f32>>(Op::kMultiply),

    ParamsFor<vec3<f32>, mat2x3<f32>, vec2<f32>>(Op::kMultiply),
    ParamsFor<vec2<f32>, mat3x2<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<vec3<f32>, mat3x3<f32>, vec3<f32>>(Op::kMultiply),

    ParamsFor<mat3x2<f32>, vec3<f32>, vec2<f32>>(Op::kMultiply),
    ParamsFor<mat2x3<f32>, vec2<f32>, vec3<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, vec3<f32>, vec3<f32>>(Op::kMultiply),

    ParamsFor<mat2x3<f32>, mat3x2<f32>, mat3x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, mat2x3<f32>, mat2x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x2<f32>, mat3x3<f32>, mat3x2<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kMultiply),
    ParamsFor<mat3x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kMultiply),

    ParamsFor<mat2x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kAdd),
    ParamsFor<mat3x2<f32>, mat3x2<f32>, mat3x2<f32>>(Op::kAdd),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kAdd),

    ParamsFor<mat2x3<f32>, mat2x3<f32>, mat2x3<f32>>(Op::kSubtract),
    ParamsFor<mat3x2<f32>, mat3x2<f32>, mat3x2<f32>>(Op::kSubtract),
    ParamsFor<mat3x3<f32>, mat3x3<f32>, mat3x3<f32>>(Op::kSubtract),

    // Comparison expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#comparison-expr

    // Comparisons over scalars
    ParamsFor<bool, bool, bool>(Op::kEqual),
    ParamsFor<bool, bool, bool>(Op::kNotEqual),

    ParamsFor<i32, i32, bool>(Op::kEqual),
    ParamsFor<i32, i32, bool>(Op::kNotEqual),
    ParamsFor<i32, i32, bool>(Op::kLessThan),
    ParamsFor<i32, i32, bool>(Op::kLessThanEqual),
    ParamsFor<i32, i32, bool>(Op::kGreaterThan),
    ParamsFor<i32, i32, bool>(Op::kGreaterThanEqual),

    ParamsFor<u32, u32, bool>(Op::kEqual),
    ParamsFor<u32, u32, bool>(Op::kNotEqual),
    ParamsFor<u32, u32, bool>(Op::kLessThan),
    ParamsFor<u32, u32, bool>(Op::kLessThanEqual),
    ParamsFor<u32, u32, bool>(Op::kGreaterThan),
    ParamsFor<u32, u32, bool>(Op::kGreaterThanEqual),

    ParamsFor<f32, f32, bool>(Op::kEqual),
    ParamsFor<f32, f32, bool>(Op::kNotEqual),
    ParamsFor<f32, f32, bool>(Op::kLessThan),
    ParamsFor<f32, f32, bool>(Op::kLessThanEqual),
    ParamsFor<f32, f32, bool>(Op::kGreaterThan),
    ParamsFor<f32, f32, bool>(Op::kGreaterThanEqual),

    // Comparisons over vectors
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<bool>, vec3<bool>, vec3<bool>>(Op::kNotEqual),

    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<bool>>(Op::kGreaterThanEqual),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<bool>>(Op::kGreaterThanEqual),

    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kNotEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kLessThan),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kLessThanEqual),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kGreaterThan),
    ParamsFor<vec3<f32>, vec3<f32>, vec3<bool>>(Op::kGreaterThanEqual),

    // Binary bitwise operations
    ParamsFor<i32, i32, i32>(Op::kOr),
    ParamsFor<i32, i32, i32>(Op::kAnd),
    ParamsFor<i32, i32, i32>(Op::kXor),

    ParamsFor<u32, u32, u32>(Op::kOr),
    ParamsFor<u32, u32, u32>(Op::kAnd),
    ParamsFor<u32, u32, u32>(Op::kXor),

    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kOr),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kAnd),
    ParamsFor<vec3<i32>, vec3<i32>, vec3<i32>>(Op::kXor),

    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kOr),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kAnd),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kXor),

    // Bit shift expressions
    ParamsFor<i32, u32, i32>(Op::kShiftLeft),
    ParamsFor<vec3<i32>, vec3<u32>, vec3<i32>>(Op::kShiftLeft),

    ParamsFor<u32, u32, u32>(Op::kShiftLeft),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kShiftLeft),

    ParamsFor<i32, u32, i32>(Op::kShiftRight),
    ParamsFor<vec3<i32>, vec3<u32>, vec3<i32>>(Op::kShiftRight),

    ParamsFor<u32, u32, u32>(Op::kShiftRight),
    ParamsFor<vec3<u32>, vec3<u32>, vec3<u32>>(Op::kShiftRight),
};

using Expr_Binary_Test_Valid = ResolverTestWithParam<Params>;
TEST_P(Expr_Binary_Test_Valid, All) {
  auto& params = GetParam();

  auto* lhs_type = params.create_lhs_type(*this);
  auto* rhs_type = params.create_rhs_type(*this);
  auto* result_type = params.create_result_type(*this);

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << params.op << " "
     << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kPrivate);
  Global("rhs", rhs_type, ast::StorageClass::kPrivate);

  auto* expr =
      create<ast::BinaryExpression>(params.op, Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr) == result_type);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Valid,
                         testing::ValuesIn(all_valid_cases));

enum class BinaryExprSide { Left, Right, Both };
using Expr_Binary_Test_WithAlias_Valid =
    ResolverTestWithParam<std::tuple<Params, BinaryExprSide>>;
TEST_P(Expr_Binary_Test_WithAlias_Valid, All) {
  const Params& params = std::get<0>(GetParam());
  BinaryExprSide side = std::get<1>(GetParam());

  auto* create_lhs_type =
      (side == BinaryExprSide::Left || side == BinaryExprSide::Both)
          ? params.create_lhs_alias_type
          : params.create_lhs_type;
  auto* create_rhs_type =
      (side == BinaryExprSide::Right || side == BinaryExprSide::Both)
          ? params.create_rhs_alias_type
          : params.create_rhs_type;

  auto* lhs_type = create_lhs_type(*this);
  auto* rhs_type = create_rhs_type(*this);

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << params.op << " "
     << FriendlyName(rhs_type);

  ss << ", After aliasing: " << FriendlyName(lhs_type) << " " << params.op
     << " " << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kPrivate);
  Global("rhs", rhs_type, ast::StorageClass::kPrivate);

  auto* expr =
      create<ast::BinaryExpression>(params.op, Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  // TODO(amaiorano): Bring this back once we have a way to get the canonical
  // type
  // auto* *result_type = params.create_result_type(*this);
  // ASSERT_TRUE(TypeOf(expr) == result_type);
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Expr_Binary_Test_WithAlias_Valid,
    testing::Combine(testing::ValuesIn(all_valid_cases),
                     testing::Values(BinaryExprSide::Left,
                                     BinaryExprSide::Right,
                                     BinaryExprSide::Both)));

// This test works by taking the cartesian product of all possible
// (type * type * op), and processing only the triplets that are not found in
// the `all_valid_cases` table.
using Expr_Binary_Test_Invalid =
    ResolverTestWithParam<std::tuple<builder::ast_type_func_ptr,
                                     builder::ast_type_func_ptr,
                                     ast::BinaryOp>>;
TEST_P(Expr_Binary_Test_Invalid, All) {
  const builder::ast_type_func_ptr& lhs_create_type_func =
      std::get<0>(GetParam());
  const builder::ast_type_func_ptr& rhs_create_type_func =
      std::get<1>(GetParam());
  const ast::BinaryOp op = std::get<2>(GetParam());

  // Skip if valid case
  // TODO(amaiorano): replace linear lookup with O(1) if too slow
  for (auto& c : all_valid_cases) {
    if (c.create_lhs_type == lhs_create_type_func &&
        c.create_rhs_type == rhs_create_type_func && c.op == op) {
      return;
    }
  }

  auto* lhs_type = lhs_create_type_func(*this);
  auto* rhs_type = rhs_create_type_func(*this);

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << op << " " << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kPrivate);
  Global("rhs", rhs_type, ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(Source{{12, 34}}, op, Expr("lhs"),
                                             Expr("rhs"));
  WrapInFunction(expr);

  ASSERT_FALSE(r()->Resolve());
  ASSERT_EQ(r()->error(),
            "12:34 error: Binary expression operand types are invalid for "
            "this operation: " +
                FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op) +
                " " + FriendlyName(rhs_type));
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Expr_Binary_Test_Invalid,
    testing::Combine(testing::ValuesIn(all_create_type_funcs),
                     testing::ValuesIn(all_create_type_funcs),
                     testing::ValuesIn(all_ops)));

using Expr_Binary_Test_Invalid_VectorMatrixMultiply =
    ResolverTestWithParam<std::tuple<bool, uint32_t, uint32_t, uint32_t>>;
TEST_P(Expr_Binary_Test_Invalid_VectorMatrixMultiply, All) {
  bool vec_by_mat = std::get<0>(GetParam());
  uint32_t vec_size = std::get<1>(GetParam());
  uint32_t mat_rows = std::get<2>(GetParam());
  uint32_t mat_cols = std::get<3>(GetParam());

  const ast::Type* lhs_type = nullptr;
  const ast::Type* rhs_type = nullptr;
  const sem::Type* result_type = nullptr;
  bool is_valid_expr;

  if (vec_by_mat) {
    lhs_type = ty.vec<f32>(vec_size);
    rhs_type = ty.mat<f32>(mat_cols, mat_rows);
    result_type = create<sem::Vector>(create<sem::F32>(), mat_cols);
    is_valid_expr = vec_size == mat_rows;
  } else {
    lhs_type = ty.mat<f32>(mat_cols, mat_rows);
    rhs_type = ty.vec<f32>(vec_size);
    result_type = create<sem::Vector>(create<sem::F32>(), mat_rows);
    is_valid_expr = vec_size == mat_cols;
  }

  Global("lhs", lhs_type, ast::StorageClass::kPrivate);
  Global("rhs", rhs_type, ast::StorageClass::kPrivate);

  auto* expr = Mul(Source{{12, 34}}, Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  if (is_valid_expr) {
    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(expr) == result_type);
  } else {
    ASSERT_FALSE(r()->Resolve());
    ASSERT_EQ(r()->error(),
              "12:34 error: Binary expression operand types are invalid for "
              "this operation: " +
                  FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op) +
                  " " + FriendlyName(rhs_type));
  }
}
auto all_dimension_values = testing::Values(2u, 3u, 4u);
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Invalid_VectorMatrixMultiply,
                         testing::Combine(testing::Values(true, false),
                                          all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values));

using Expr_Binary_Test_Invalid_MatrixMatrixMultiply =
    ResolverTestWithParam<std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>;
TEST_P(Expr_Binary_Test_Invalid_MatrixMatrixMultiply, All) {
  uint32_t lhs_mat_rows = std::get<0>(GetParam());
  uint32_t lhs_mat_cols = std::get<1>(GetParam());
  uint32_t rhs_mat_rows = std::get<2>(GetParam());
  uint32_t rhs_mat_cols = std::get<3>(GetParam());

  auto* lhs_type = ty.mat<f32>(lhs_mat_cols, lhs_mat_rows);
  auto* rhs_type = ty.mat<f32>(rhs_mat_cols, rhs_mat_rows);

  auto* f32 = create<sem::F32>();
  auto* col = create<sem::Vector>(f32, lhs_mat_rows);
  auto* result_type = create<sem::Matrix>(col, rhs_mat_cols);

  Global("lhs", lhs_type, ast::StorageClass::kPrivate);
  Global("rhs", rhs_type, ast::StorageClass::kPrivate);

  auto* expr = Mul(Source{{12, 34}}, Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  bool is_valid_expr = lhs_mat_cols == rhs_mat_rows;
  if (is_valid_expr) {
    ASSERT_TRUE(r()->Resolve()) << r()->error();
    ASSERT_TRUE(TypeOf(expr) == result_type);
  } else {
    ASSERT_FALSE(r()->Resolve());
    ASSERT_EQ(r()->error(),
              "12:34 error: Binary expression operand types are invalid for "
              "this operation: " +
                  FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op) +
                  " " + FriendlyName(rhs_type));
  }
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         Expr_Binary_Test_Invalid_MatrixMatrixMultiply,
                         testing::Combine(all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values,
                                          all_dimension_values));

}  // namespace ExprBinaryTest

using UnaryOpExpressionTest = ResolverTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  if (op == ast::UnaryOp::kNot) {
    Global("ident", ty.vec4<bool>(), ast::StorageClass::kPrivate);
  } else if (op == ast::UnaryOp::kNegation || op == ast::UnaryOp::kComplement) {
    Global("ident", ty.vec4<i32>(), ast::StorageClass::kPrivate);
  } else {
    Global("ident", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  }
  auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
  WrapInFunction(der);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(der), nullptr);
  ASSERT_TRUE(TypeOf(der)->Is<sem::Vector>());
  if (op == ast::UnaryOp::kNot) {
    EXPECT_TRUE(TypeOf(der)->As<sem::Vector>()->type()->Is<sem::Bool>());
  } else if (op == ast::UnaryOp::kNegation || op == ast::UnaryOp::kComplement) {
    EXPECT_TRUE(TypeOf(der)->As<sem::Vector>()->type()->Is<sem::I32>());
  } else {
    EXPECT_TRUE(TypeOf(der)->As<sem::Vector>()->type()->Is<sem::F32>());
  }
  EXPECT_EQ(TypeOf(der)->As<sem::Vector>()->Width(), 4u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kComplement,
                                         ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(ResolverTest, StorageClass_SetsIfMissing) {
  auto* var = Var("var", ty.i32());

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kFunction);
}

TEST_F(ResolverTest, StorageClass_SetForSampler) {
  auto* t = ty.sampler(ast::SamplerKind::kSampler);
  auto* var = Global("var", t,
                     ast::AttributeList{
                         create<ast::BindingAttribute>(0),
                         create<ast::GroupAttribute>(0),
                     });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kHandle);
}

TEST_F(ResolverTest, StorageClass_SetForTexture) {
  auto* t = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
  auto* var = Global("var", t,
                     ast::AttributeList{
                         create<ast::BindingAttribute>(0),
                         create<ast::GroupAttribute>(0),
                     });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kHandle);
}

TEST_F(ResolverTest, StorageClass_DoesNotSetOnConst) {
  auto* var = Let("var", ty.i32(), Construct(ty.i32()));
  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::AttributeList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kNone);
}

TEST_F(ResolverTest, Access_SetForStorageBuffer) {
  // struct S { x : i32 };
  // var<storage> g : S;
  auto* s = Structure("S", {Member(Source{{12, 34}}, "x", ty.i32())});
  auto* var =
      Global(Source{{56, 78}}, "g", ty.Of(s), ast::StorageClass::kStorage,
             ast::AttributeList{
                 create<ast::BindingAttribute>(0),
                 create<ast::GroupAttribute>(0),
             });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->Access(), ast::Access::kRead);
}

TEST_F(ResolverTest, BindingPoint_SetForResources) {
  // @group(1) @binding(2) var s1 : sampler;
  // @group(3) @binding(4) var s2 : sampler;
  auto* s1 = Global(Sym(), ty.sampler(ast::SamplerKind::kSampler),
                    ast::AttributeList{create<ast::GroupAttribute>(1),
                                       create<ast::BindingAttribute>(2)});
  auto* s2 = Global(Sym(), ty.sampler(ast::SamplerKind::kSampler),
                    ast::AttributeList{create<ast::GroupAttribute>(3),
                                       create<ast::BindingAttribute>(4)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get<sem::GlobalVariable>(s1)->BindingPoint(),
            (sem::BindingPoint{1u, 2u}));
  EXPECT_EQ(Sem().Get<sem::GlobalVariable>(s2)->BindingPoint(),
            (sem::BindingPoint{3u, 4u}));
}

TEST_F(ResolverTest, Function_EntryPoints_StageAttribute) {
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

  Global("first", ty.f32(), ast::StorageClass::kPrivate);
  Global("second", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_a", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_b", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_c", ty.f32(), ast::StorageClass::kPrivate);

  ast::VariableList params;
  auto* func_b =
      Func("b", params, ty.f32(), {Return(0.0f)}, ast::AttributeList{});
  auto* func_c =
      Func("c", params, ty.f32(), {Assign("second", Call("b")), Return(0.0f)},
           ast::AttributeList{});

  auto* func_a =
      Func("a", params, ty.f32(), {Assign("first", Call("c")), Return(0.0f)},
           ast::AttributeList{});

  auto* ep_1 = Func("ep_1", params, ty.void_(),
                    {
                        Assign("call_a", Call("a")),
                        Assign("call_b", Call("b")),
                    },
                    ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                                       WorkgroupSize(1)});

  auto* ep_2 = Func("ep_2", params, ty.void_(),
                    {
                        Assign("call_c", Call("c")),
                    },
                    ast::AttributeList{Stage(ast::PipelineStage::kCompute),
                                       WorkgroupSize(1)});

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

  EXPECT_EQ(func_b_sem->Parameters().size(), 0u);
  EXPECT_EQ(func_a_sem->Parameters().size(), 0u);
  EXPECT_EQ(func_c_sem->Parameters().size(), 0u);

  const auto& b_eps = func_b_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, b_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), b_eps[0]->Declaration()->symbol);
  EXPECT_EQ(Symbols().Register("ep_2"), b_eps[1]->Declaration()->symbol);

  const auto& a_eps = func_a_sem->AncestorEntryPoints();
  ASSERT_EQ(1u, a_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), a_eps[0]->Declaration()->symbol);

  const auto& c_eps = func_c_sem->AncestorEntryPoints();
  ASSERT_EQ(2u, c_eps.size());
  EXPECT_EQ(Symbols().Register("ep_1"), c_eps[0]->Declaration()->symbol);
  EXPECT_EQ(Symbols().Register("ep_2"), c_eps[1]->Declaration()->symbol);

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
             CallStmt(Call(fn_a(i + 1))),
             CallStmt(Call(fn_b(i + 1))),
         },
         {});
    Func(fn_b(i), {}, ty.void_(),
         {
             CallStmt(Call(fn_a(i + 1))),
             CallStmt(Call(fn_b(i + 1))),
         },
         {});
  }

  Func("main", {}, ty.void_(),
       {
           CallStmt(Call(fn_a(0))),
           CallStmt(Call(fn_b(0))),
       },
       {Stage(ast::PipelineStage::kCompute), WorkgroupSize(1)});

  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

// Test for crbug.com/tint/728
TEST_F(ResolverTest, ASTNodesAreReached) {
  Structure("A", {Member("x", ty.array<f32, 4>(4))});
  Structure("B", {Member("x", ty.array<f32, 4>(4))});
  ASSERT_TRUE(r()->Resolve()) << r()->error();
}

TEST_F(ResolverTest, ASTNodeNotReached) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        b.Expr("expr");
        Resolver(&b).Resolve();
      },
      "internal compiler error: AST node 'tint::ast::IdentifierExpression' was "
      "not reached by the resolver");
}

TEST_F(ResolverTest, ASTNodeReachedTwice) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder b;
        auto* expr = b.Expr(1);
        b.Global("a", b.ty.i32(), ast::StorageClass::kPrivate, expr);
        b.Global("b", b.ty.i32(), ast::StorageClass::kPrivate, expr);
        Resolver(&b).Resolve();
      },
      "internal compiler error: AST node 'tint::ast::SintLiteralExpression' "
      "was encountered twice in the same AST of a Program");
}

TEST_F(ResolverTest, UnaryOp_Not) {
  Global("ident", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* der = create<ast::UnaryOpExpression>(ast::UnaryOp::kNot,
                                             Expr(Source{{12, 34}}, "ident"));
  WrapInFunction(der);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "12:34 error: cannot logical negate expression of type 'vec4<f32>");
}

TEST_F(ResolverTest, UnaryOp_Complement) {
  Global("ident", ty.vec4<f32>(), ast::StorageClass::kPrivate);
  auto* der = create<ast::UnaryOpExpression>(ast::UnaryOp::kComplement,
                                             Expr(Source{{12, 34}}, "ident"));
  WrapInFunction(der);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(
      r()->error(),
      "12:34 error: cannot bitwise complement expression of type 'vec4<f32>");
}

TEST_F(ResolverTest, UnaryOp_Negation) {
  Global("ident", ty.u32(), ast::StorageClass::kPrivate);
  auto* der = create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation,
                                             Expr(Source{{12, 34}}, "ident"));
  WrapInFunction(der);

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(), "12:34 error: cannot negate expression of type 'u32");
}

TEST_F(ResolverTest, TextureSampler_TextureSample) {
  Global("t", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 1));
  Global("s", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(1, 2));

  auto* call = CallStmt(Call("textureSample", "t", "s", vec2<f32>(1.0f, 2.0f)));
  const ast::Function* f = Func("test_function", {}, ty.void_(), {call},
                                {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  const sem::Function* sf = Sem().Get(f);
  auto pairs = sf->TextureSamplerPairs();
  ASSERT_EQ(pairs.size(), 1u);
  EXPECT_TRUE(pairs[0].first != nullptr);
  EXPECT_TRUE(pairs[0].second != nullptr);
}

TEST_F(ResolverTest, TextureSampler_TextureSampleInFunction) {
  Global("t", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 1));
  Global("s", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(1, 2));

  auto* inner_call =
      CallStmt(Call("textureSample", "t", "s", vec2<f32>(1.0f, 2.0f)));
  const ast::Function* inner_func =
      Func("inner_func", {}, ty.void_(), {inner_call});
  auto* outer_call = CallStmt(Call("inner_func"));
  const ast::Function* outer_func =
      Func("outer_func", {}, ty.void_(), {outer_call},
           {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto inner_pairs = Sem().Get(inner_func)->TextureSamplerPairs();
  ASSERT_EQ(inner_pairs.size(), 1u);
  EXPECT_TRUE(inner_pairs[0].first != nullptr);
  EXPECT_TRUE(inner_pairs[0].second != nullptr);

  auto outer_pairs = Sem().Get(outer_func)->TextureSamplerPairs();
  ASSERT_EQ(outer_pairs.size(), 1u);
  EXPECT_TRUE(outer_pairs[0].first != nullptr);
  EXPECT_TRUE(outer_pairs[0].second != nullptr);
}

TEST_F(ResolverTest, TextureSampler_TextureSampleFunctionDiamondSameVariables) {
  Global("t", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 1));
  Global("s", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(1, 2));

  auto* inner_call_1 =
      CallStmt(Call("textureSample", "t", "s", vec2<f32>(1.0f, 2.0f)));
  const ast::Function* inner_func_1 =
      Func("inner_func_1", {}, ty.void_(), {inner_call_1});
  auto* inner_call_2 =
      CallStmt(Call("textureSample", "t", "s", vec2<f32>(3.0f, 4.0f)));
  const ast::Function* inner_func_2 =
      Func("inner_func_2", {}, ty.void_(), {inner_call_2});
  auto* outer_call_1 = CallStmt(Call("inner_func_1"));
  auto* outer_call_2 = CallStmt(Call("inner_func_2"));
  const ast::Function* outer_func =
      Func("outer_func", {}, ty.void_(), {outer_call_1, outer_call_2},
           {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto inner_pairs_1 = Sem().Get(inner_func_1)->TextureSamplerPairs();
  ASSERT_EQ(inner_pairs_1.size(), 1u);
  EXPECT_TRUE(inner_pairs_1[0].first != nullptr);
  EXPECT_TRUE(inner_pairs_1[0].second != nullptr);

  auto inner_pairs_2 = Sem().Get(inner_func_2)->TextureSamplerPairs();
  ASSERT_EQ(inner_pairs_1.size(), 1u);
  EXPECT_TRUE(inner_pairs_2[0].first != nullptr);
  EXPECT_TRUE(inner_pairs_2[0].second != nullptr);

  auto outer_pairs = Sem().Get(outer_func)->TextureSamplerPairs();
  ASSERT_EQ(outer_pairs.size(), 1u);
  EXPECT_TRUE(outer_pairs[0].first != nullptr);
  EXPECT_TRUE(outer_pairs[0].second != nullptr);
}

TEST_F(ResolverTest,
       TextureSampler_TextureSampleFunctionDiamondDifferentVariables) {
  Global("t1", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 1));
  Global("t2", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 2));
  Global("s", ty.sampler(ast::SamplerKind::kSampler), GroupAndBinding(1, 3));

  auto* inner_call_1 =
      CallStmt(Call("textureSample", "t1", "s", vec2<f32>(1.0f, 2.0f)));
  const ast::Function* inner_func_1 =
      Func("inner_func_1", {}, ty.void_(), {inner_call_1});
  auto* inner_call_2 =
      CallStmt(Call("textureSample", "t2", "s", vec2<f32>(3.0f, 4.0f)));
  const ast::Function* inner_func_2 =
      Func("inner_func_2", {}, ty.void_(), {inner_call_2});
  auto* outer_call_1 = CallStmt(Call("inner_func_1"));
  auto* outer_call_2 = CallStmt(Call("inner_func_2"));
  const ast::Function* outer_func =
      Func("outer_func", {}, ty.void_(), {outer_call_1, outer_call_2},
           {Stage(ast::PipelineStage::kFragment)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto inner_pairs_1 = Sem().Get(inner_func_1)->TextureSamplerPairs();
  ASSERT_EQ(inner_pairs_1.size(), 1u);
  EXPECT_TRUE(inner_pairs_1[0].first != nullptr);
  EXPECT_TRUE(inner_pairs_1[0].second != nullptr);

  auto inner_pairs_2 = Sem().Get(inner_func_2)->TextureSamplerPairs();
  ASSERT_EQ(inner_pairs_2.size(), 1u);
  EXPECT_TRUE(inner_pairs_2[0].first != nullptr);
  EXPECT_TRUE(inner_pairs_2[0].second != nullptr);

  auto outer_pairs = Sem().Get(outer_func)->TextureSamplerPairs();
  ASSERT_EQ(outer_pairs.size(), 2u);
  EXPECT_TRUE(outer_pairs[0].first == inner_pairs_1[0].first);
  EXPECT_TRUE(outer_pairs[0].second == inner_pairs_1[0].second);
  EXPECT_TRUE(outer_pairs[1].first == inner_pairs_2[0].first);
  EXPECT_TRUE(outer_pairs[1].second == inner_pairs_2[0].second);
}

TEST_F(ResolverTest, TextureSampler_TextureDimensions) {
  Global("t", ty.sampled_texture(ast::TextureDimension::k2d, ty.f32()),
         GroupAndBinding(1, 2));

  auto* call = Call("textureDimensions", "t");
  const ast::Function* f = WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  const sem::Function* sf = Sem().Get(f);
  auto pairs = sf->TextureSamplerPairs();
  ASSERT_EQ(pairs.size(), 1u);
  EXPECT_TRUE(pairs[0].first != nullptr);
  EXPECT_TRUE(pairs[0].second == nullptr);
}

TEST_F(ResolverTest, ModuleDependencyOrderedDeclarations) {
  auto* f0 = Func("f0", {}, ty.void_(), {});
  auto* v0 = Global("v0", ty.i32(), ast::StorageClass::kPrivate);
  auto* a0 = Alias("a0", ty.i32());
  auto* s0 = Structure("s0", {Member("m", ty.i32())});
  auto* f1 = Func("f1", {}, ty.void_(), {});
  auto* v1 = Global("v1", ty.i32(), ast::StorageClass::kPrivate);
  auto* a1 = Alias("a1", ty.i32());
  auto* s1 = Structure("s1", {Member("m", ty.i32())});
  auto* f2 = Func("f2", {}, ty.void_(), {});
  auto* v2 = Global("v2", ty.i32(), ast::StorageClass::kPrivate);
  auto* a2 = Alias("a2", ty.i32());
  auto* s2 = Structure("s2", {Member("m", ty.i32())});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(Sem().Module(), nullptr);
  EXPECT_THAT(Sem().Module()->DependencyOrderedDeclarations(),
              ElementsAre(f0, v0, a0, s0, f1, v1, a1, s1, f2, v2, a2, s2));
}

}  // namespace
}  // namespace tint::resolver
