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

#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/override_decoration.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/resolver/resolver_test_helper.h"
#include "src/sem/call.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/variable.h"

using ::testing::ElementsAre;
using ::testing::HasSubstr;

namespace tint {
namespace resolver {
namespace {

// Helpers and typedefs
using i32 = ProgramBuilder::i32;
using u32 = ProgramBuilder::u32;
using f32 = ProgramBuilder::f32;
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
  lit.push_back(create<ast::SintLiteral>(3));
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
  auto* else_stmt = create<ast::ElseStatement>(else_cond, else_body);

  auto* lhs = Expr("v");
  auto* rhs = Expr(2.3f);

  auto* assign = Assign(lhs, rhs);
  auto* body = Block(assign);
  auto* cond = Expr(true);
  auto* stmt =
      create<ast::IfStatement>(cond, body, ast::ElseStatementList{else_stmt});
  WrapInFunction(v, stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(else_lhs), nullptr);
  ASSERT_NE(TypeOf(else_rhs), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<sem::Bool>());
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

  auto* body = Block(Assign(body_lhs, body_rhs));
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
  auto* stmt = Switch(Expr(2), Case(Literal(3), case_block), DefaultCase());
  WrapInFunction(v, stmt);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(stmt->condition())->Is<sem::I32>());
  EXPECT_TRUE(TypeOf(lhs)->UnwrapRef()->Is<sem::F32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<sem::F32>());
  EXPECT_EQ(BlockOf(lhs), case_block);
  EXPECT_EQ(BlockOf(rhs), case_block);
}

TEST_F(ResolverTest, Stmt_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), {Return(0.0f)}, ast::DecorationList{});

  auto* expr = Call("my_func");

  auto* call = create<ast::CallStatement>(expr);
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(expr), call);
}

TEST_F(ResolverTest, Stmt_VariableDecl) {
  auto* var = Var("my_var", ty.i32(), ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = Decl(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<sem::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_Alias) {
  auto* my_int = ty.alias("MyInt", ty.i32());
  AST().AddConstructedType(my_int);
  auto* var = Var("my_var", my_int, ast::StorageClass::kNone, Expr(2));
  auto* init = var->constructor();

  auto* decl = Decl(var);
  WrapInFunction(decl);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<sem::I32>());
}

TEST_F(ResolverTest, Stmt_VariableDecl_AliasRedeclared) {
  auto* my_int1 = ty.alias(Source{{12, 34}}, "MyInt", ty.i32());
  auto* my_int2 = ty.alias(Source{{56, 78}}, "MyInt", ty.i32());
  AST().AddConstructedType(my_int1);
  AST().AddConstructedType(my_int2);
  WrapInFunction();

  EXPECT_FALSE(r()->Resolve());
  EXPECT_EQ(r()->error(),
            "56:78 error: type with the name 'MyInt' was already declared\n"
            "12:34 note: first declared here");
}

TEST_F(ResolverTest, Stmt_VariableDecl_ModuleScope) {
  auto* init = Expr(2);
  Global("my_var", ty.i32(), ast::StorageClass::kInput, init);

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
  auto* foo_i32_init = foo_i32->constructor();
  auto* foo_i32_decl = Decl(foo_i32);

  // Reference "foo" inside the block
  auto* bar_i32 = Var("bar", ty.i32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_i32_init = bar_i32->constructor();
  auto* bar_i32_decl = Decl(bar_i32);

  auto* inner = Block(foo_i32_decl, bar_i32_decl);

  // Declare f32 "foo" at function scope
  auto* foo_f32 = Var("foo", ty.f32(), ast::StorageClass::kNone, Expr(2.f));
  auto* foo_f32_init = foo_f32->constructor();
  auto* foo_f32_decl = Decl(foo_f32);

  // Reference "foo" at function scope
  auto* bar_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* bar_f32_init = bar_f32->constructor();
  auto* bar_f32_decl = Decl(bar_f32);

  Func("func", params, ty.void_(), {inner, foo_f32_decl, bar_f32_decl},
       ast::DecorationList{});

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
  EXPECT_TRUE(CheckVarUsers(foo_i32, {bar_i32->constructor()}));
  EXPECT_TRUE(CheckVarUsers(foo_f32, {bar_f32->constructor()}));
  ASSERT_NE(VarOf(bar_i32->constructor()), nullptr);
  EXPECT_EQ(VarOf(bar_i32->constructor())->Declaration(), foo_i32);
  ASSERT_NE(VarOf(bar_f32->constructor()), nullptr);
  EXPECT_EQ(VarOf(bar_f32->constructor())->Declaration(), foo_f32);
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
  auto* fn_i32_init = fn_i32->constructor();
  auto* fn_i32_decl = Decl(fn_i32);
  Func("func_i32", params, ty.void_(), {fn_i32_decl}, ast::DecorationList{});

  // Declare f32 "foo" at module scope
  auto* mod_f32 = Var("foo", ty.f32(), ast::StorageClass::kInput, Expr(2.f));
  auto* mod_init = mod_f32->constructor();
  AST().AddGlobalVariable(mod_f32);

  // Reference "foo" in another function
  auto* fn_f32 = Var("bar", ty.f32(), ast::StorageClass::kNone, Expr("foo"));
  auto* fn_f32_init = fn_f32->constructor();
  auto* fn_f32_decl = Decl(fn_f32);
  Func("func_f32", params, ty.void_(), {fn_f32_decl}, ast::DecorationList{});

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
  EXPECT_TRUE(CheckVarUsers(mod_f32, {fn_f32->constructor()}));
  ASSERT_NE(VarOf(fn_f32->constructor()), nullptr);
  EXPECT_EQ(VarOf(fn_f32->constructor())->Declaration(), mod_f32);
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array) {
  auto* idx = Expr(2);
  Global("my_var", ty.array<f32, 3>(), ast::StorageClass::kPrivate);

  auto* acc = IndexAccessor("my_var", idx);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

  auto* ref = TypeOf(acc)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Alias_Array) {
  auto* aary = ty.alias("myarrty", ty.array<f32, 3>());
  AST().AddConstructedType(aary);

  Global("my_var", aary, ast::StorageClass::kPrivate);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

  auto* ref = TypeOf(acc)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Array_Constant) {
  GlobalConst("my_var", ty.array<f32, 3>(), array<f32, 3>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  EXPECT_TRUE(TypeOf(acc)->Is<sem::F32>()) << TypeOf(acc)->type_name();
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kInput);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

  auto* ref = TypeOf(acc)->As<sem::Reference>();
  ASSERT_TRUE(ref->StoreType()->Is<sem::Vector>());
  EXPECT_EQ(ref->StoreType()->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  Global("my_var", ty.mat2x3<f32>(), ast::StorageClass::kInput);

  auto* acc = IndexAccessor(IndexAccessor("my_var", 2), 1);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

  auto* ref = TypeOf(acc)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_ArrayAccessor_Vector) {
  Global("my_var", ty.vec3<f32>(), ast::StorageClass::kInput);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<sem::Reference>());

  auto* ref = TypeOf(acc)->As<sem::Reference>();
  EXPECT_TRUE(ref->StoreType()->Is<sem::F32>());
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
  Func("my_func", params, ty.f32(), {Return(0.0f)}, ast::DecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call_InBinaryOp) {
  ast::VariableList params;
  Func("func", params, ty.f32(), {Return(0.0f)}, ast::DecorationList{});

  auto* expr = Add(Call("func"), Call("func"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

TEST_F(ResolverTest, Expr_Call_WithParams) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(),
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

TEST_F(ResolverTest, Expr_Call_Intrinsic) {
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
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 2u);
}

TEST_F(ResolverTest, Expr_Constructor_Type_Vec3) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 3u);
}

TEST_F(ResolverTest, Expr_Constructor_Type_Vec4) {
  auto* tc = vec4<f32>(1.0f, 1.0f, 1.0f, 1.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(tc)->As<sem::Vector>()->size(), 4u);
}

TEST_F(ResolverTest, Expr_Identifier_GlobalVariable) {
  auto* my_var = Global("my_var", ty.f32(), ast::StorageClass::kInput);

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
  auto* var = Const("my_var", ty.f32(), Construct(ty.f32()));
  auto* decl = Decl(Var("b", ty.f32(), ast::StorageClass::kNone, my_var_a));

  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           decl,
       },
       ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(my_var_a), nullptr);
  EXPECT_TRUE(TypeOf(my_var_a)->Is<sem::F32>());
  EXPECT_EQ(StmtOf(my_var_a), decl);
  EXPECT_TRUE(CheckVarUsers(var, {my_var_a}));
  ASSERT_NE(VarOf(my_var_a), nullptr);
  EXPECT_EQ(VarOf(my_var_a)->Declaration(), var);
}

TEST_F(ResolverTest, Expr_Identifier_FunctionVariable) {
  auto* my_var_a = Expr("my_var");
  auto* my_var_b = Expr("my_var");
  auto* assign = Assign(my_var_a, my_var_b);

  auto* var = Var("my_var", ty.f32(), ast::StorageClass::kNone, nullptr,
                  {
                      create<ast::BindingDecoration>(0),
                      create<ast::GroupDecoration>(0),
                  });

  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           Decl(var),
           assign,
       },
       ast::DecorationList{});

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
      Const("p", ty.pointer<f32>(ast::StorageClass::kFunction), AddressOf(v)));
  auto* assign = Assign(Deref(p), 1.23f);
  Func("my_func", ast::VariableList{}, ty.void_(),
       {
           v_decl,
           p_decl,
           assign,
       },
       ast::DecorationList{});

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
       ast::DecorationList{});

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
  auto* s = Structure("S", {Member("m", ty.u32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);

  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", a, ast::StorageClass::kStorage, nullptr,
                        {
                            create<ast::BindingDecoration>(0),
                            create<ast::GroupDecoration>(0),
                        });
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  auto* func = Func("my_func", ast::VariableList{}, ty.void_(),
                    {
                        Assign("out_var", "in_var"),
                        Assign("wg_var", "wg_var"),
                        Assign("sb_var", "sb_var"),
                        Assign("priv_var", "priv_var"),
                    });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);
  EXPECT_EQ(func_sem->Parameters().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());

  const auto& vars = func_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
}

TEST_F(ResolverTest, Function_RegisterInputOutputVariables_SubFunction) {
  auto* s = Structure("S", {Member("m", ty.u32())},
                      {create<ast::StructBlockDecoration>()});
  auto* a = ty.access(ast::AccessControl::kReadOnly, s);

  auto* in_var = Global("in_var", ty.f32(), ast::StorageClass::kInput);
  auto* out_var = Global("out_var", ty.f32(), ast::StorageClass::kOutput);
  auto* sb_var = Global("sb_var", a, ast::StorageClass::kStorage, nullptr,
                        {
                            create<ast::BindingDecoration>(0),
                            create<ast::GroupDecoration>(0),
                        });
  auto* wg_var = Global("wg_var", ty.f32(), ast::StorageClass::kWorkgroup);
  auto* priv_var = Global("priv_var", ty.f32(), ast::StorageClass::kPrivate);

  Func("my_func", ast::VariableList{}, ty.f32(),
       {Assign("out_var", "in_var"), Assign("wg_var", "wg_var"),
        Assign("sb_var", "sb_var"), Assign("priv_var", "priv_var"),
        Return(0.0f)},
       ast::DecorationList{});

  auto* func2 = Func("func", ast::VariableList{}, ty.void_(),
                     {
                         Assign("out_var", Call("my_func")),
                     },
                     ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func2_sem = Sem().Get(func2);
  ASSERT_NE(func2_sem, nullptr);
  EXPECT_EQ(func2_sem->Parameters().size(), 0u);

  const auto& vars = func2_sem->ReferencedModuleVariables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0]->Declaration(), out_var);
  EXPECT_EQ(vars[1]->Declaration(), in_var);
  EXPECT_EQ(vars[2]->Declaration(), wg_var);
  EXPECT_EQ(vars[3]->Declaration(), sb_var);
  EXPECT_EQ(vars[4]->Declaration(), priv_var);
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

  EXPECT_EQ(func_sem->ReferencedModuleVariables().size(), 0u);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::Void>());
}

TEST_F(ResolverTest, Function_ReturnStatements) {
  auto* var = Var("foo", ty.f32());

  auto* ret_1 = Return(1.f);
  auto* ret_foo = Return("foo");
  auto* func = Func("my_func", ast::VariableList{}, ty.f32(),
                    {
                        Decl(var),
                        If(true, Block(ret_1)),
                        ret_foo,
                    });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);
  EXPECT_EQ(func_sem->Parameters().size(), 0u);

  EXPECT_EQ(func_sem->ReturnStatements().size(), 2u);
  EXPECT_EQ(func_sem->ReturnStatements()[0], ret_1);
  EXPECT_EQ(func_sem->ReturnStatements()[1], ret_foo);
  EXPECT_TRUE(func_sem->ReturnType()->Is<sem::F32>());
}

TEST_F(ResolverTest, Function_WorkgroupSize_NotSet) {
  // [[stage(compute)]]
  // fn main() {}
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {}, {});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 1u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 1u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 1u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Literals) {
  // [[stage(compute), workgroup_size(8, 2, 3)]]
  // fn main() {}
  auto* func =
      Func("main", ast::VariableList{}, ty.void_(), {},
           {Stage(ast::PipelineStage::kCompute), WorkgroupSize(8, 2, 3)});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 8u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 2u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 3u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Consts) {
  // let width = 16;
  // let height = 8;
  // let depth = 2;
  // [[stage(compute), workgroup_size(width, height, depth)]]
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

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 16u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 8u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 2u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Consts_NestedInitializer) {
  // let width = i32(i32(i32(8)));
  // let height = i32(i32(i32(4)));
  // [[stage(compute), workgroup_size(width, height)]]
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

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 8u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 4u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 1u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts) {
  // [[override(0)]] let width = 16;
  // [[override(1)]] let height = 8;
  // [[override(2)]] let depth = 2;
  // [[stage(compute), workgroup_size(width, height, depth)]]
  // fn main() {}
  auto* width = GlobalConst("width", ty.i32(), Expr(16), {Override(0)});
  auto* height = GlobalConst("height", ty.i32(), Expr(8), {Override(1)});
  auto* depth = GlobalConst("depth", ty.i32(), Expr(2), {Override(2)});
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize("width", "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 16u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 8u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 2u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, width);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, height);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, depth);
}

TEST_F(ResolverTest, Function_WorkgroupSize_OverridableConsts_NoInit) {
  // [[override(0)]] let width : i32;
  // [[override(1)]] let height : i32;
  // [[override(2)]] let depth : i32;
  // [[stage(compute), workgroup_size(width, height, depth)]]
  // fn main() {}
  auto* width = GlobalConst("width", ty.i32(), nullptr, {Override(0)});
  auto* height = GlobalConst("height", ty.i32(), nullptr, {Override(1)});
  auto* depth = GlobalConst("depth", ty.i32(), nullptr, {Override(2)});
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize("width", "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 0u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 0u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 0u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, width);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, height);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, depth);
}

TEST_F(ResolverTest, Function_WorkgroupSize_Mixed) {
  // [[override(1)]] let height = 2;
  // let depth = 3;
  // [[stage(compute), workgroup_size(8, height, depth)]]
  // fn main() {}
  auto* height = GlobalConst("height", ty.i32(), Expr(2), {Override(0)});
  GlobalConst("depth", ty.i32(), Expr(3));
  auto* func = Func("main", ast::VariableList{}, ty.void_(), {},
                    {Stage(ast::PipelineStage::kCompute),
                     WorkgroupSize(8, "height", "depth")});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->workgroup_size()[0].value, 8u);
  EXPECT_EQ(func_sem->workgroup_size()[1].value, 2u);
  EXPECT_EQ(func_sem->workgroup_size()[2].value, 3u);
  EXPECT_EQ(func_sem->workgroup_size()[0].overridable_const, nullptr);
  EXPECT_EQ(func_sem->workgroup_size()[1].overridable_const, height);
  EXPECT_EQ(func_sem->workgroup_size()[2].overridable_const, nullptr);
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct) {
  auto* st = Structure("S", {Member("first_member", ty.i32()),
                             Member("second_member", ty.f32())});
  Global("my_struct", st, ast::StorageClass::kInput);

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
  EXPECT_EQ(sma->Member()->Declaration()->symbol(),
            Symbols().Get("second_member"));
}

TEST_F(ResolverTest, Expr_MemberAccessor_Struct_Alias) {
  auto* st = Structure("S", {Member("first_member", ty.i32()),
                             Member("second_member", ty.f32())});
  auto* alias = ty.alias("alias", st);
  AST().AddConstructedType(alias);
  Global("my_struct", alias, ast::StorageClass::kInput);

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
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kInput);

  auto* mem = MemberAccessor("my_vec", "xzyw");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(mem)->As<sem::Vector>()->size(), 4u);
  ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
  EXPECT_THAT(Sem().Get(mem)->As<sem::Swizzle>()->Indices(),
              ElementsAre(0, 2, 1, 3));
}

TEST_F(ResolverTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  Global("my_vec", ty.vec3<f32>(), ast::StorageClass::kInput);

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

  auto* stB = Structure("B", {Member("foo", ty.vec4<f32>())});
  auto* stA = Structure("A", {Member("mem", ty.vec(stB, 3))});
  Global("c", stA, ast::StorageClass::kInput);

  auto* mem = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0), "foo"),
      "yx");
  WrapInFunction(mem);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(mem)->As<sem::Vector>()->size(), 2u);
  ASSERT_TRUE(Sem().Get(mem)->Is<sem::Swizzle>());
}

TEST_F(ResolverTest, Expr_MemberAccessor_InBinaryOp) {
  auto* st = Structure("S", {Member("first_member", ty.f32()),
                             Member("second_member", ty.f32())});
  Global("my_struct", st, ast::StorageClass::kInput);

  auto* expr = Add(MemberAccessor("my_struct", "first_member"),
                   MemberAccessor("my_struct", "second_member"));
  WrapInFunction(expr);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<sem::F32>());
}

namespace ExprBinaryTest {

struct Params {
  ast::BinaryOp op;
  create_ast_type_func_ptr create_lhs_type;
  create_ast_type_func_ptr create_rhs_type;
  create_sem_type_func_ptr create_result_type;
};

static constexpr create_ast_type_func_ptr all_create_type_funcs[] = {
    ast_bool,        ast_u32,         ast_i32,        ast_f32,
    ast_vec3<bool>,  ast_vec3<i32>,   ast_vec3<u32>,  ast_vec3<f32>,
    ast_mat3x3<i32>, ast_mat3x3<u32>, ast_mat3x3<f32>};

// A list of all valid test cases for 'lhs op rhs', except that for vecN and
// matNxN, we only test N=3.
static constexpr Params all_valid_cases[] = {
    // Logical expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#logical-expr

    // Binary logical expressions
    Params{Op::kLogicalAnd, ast_bool, ast_bool, sem_bool},
    Params{Op::kLogicalOr, ast_bool, ast_bool, sem_bool},

    Params{Op::kAnd, ast_bool, ast_bool, sem_bool},
    Params{Op::kOr, ast_bool, ast_bool, sem_bool},
    Params{Op::kAnd, ast_vec3<bool>, ast_vec3<bool>, sem_vec3<sem_bool>},
    Params{Op::kOr, ast_vec3<bool>, ast_vec3<bool>, sem_vec3<sem_bool>},

    // Arithmetic expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#arithmetic-expr

    // Binary arithmetic expressions over scalars
    Params{Op::kAdd, ast_i32, ast_i32, sem_i32},
    Params{Op::kSubtract, ast_i32, ast_i32, sem_i32},
    Params{Op::kMultiply, ast_i32, ast_i32, sem_i32},
    Params{Op::kDivide, ast_i32, ast_i32, sem_i32},
    Params{Op::kModulo, ast_i32, ast_i32, sem_i32},

    Params{Op::kAdd, ast_u32, ast_u32, sem_u32},
    Params{Op::kSubtract, ast_u32, ast_u32, sem_u32},
    Params{Op::kMultiply, ast_u32, ast_u32, sem_u32},
    Params{Op::kDivide, ast_u32, ast_u32, sem_u32},
    Params{Op::kModulo, ast_u32, ast_u32, sem_u32},

    Params{Op::kAdd, ast_f32, ast_f32, sem_f32},
    Params{Op::kSubtract, ast_f32, ast_f32, sem_f32},
    Params{Op::kMultiply, ast_f32, ast_f32, sem_f32},
    Params{Op::kDivide, ast_f32, ast_f32, sem_f32},
    Params{Op::kModulo, ast_f32, ast_f32, sem_f32},

    // Binary arithmetic expressions over vectors
    Params{Op::kAdd, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{Op::kSubtract, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{Op::kMultiply, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{Op::kDivide, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_i32>},
    Params{Op::kModulo, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_i32>},

    Params{Op::kAdd, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{Op::kSubtract, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{Op::kMultiply, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{Op::kDivide, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},
    Params{Op::kModulo, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},

    Params{Op::kAdd, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{Op::kSubtract, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{Op::kMultiply, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{Op::kDivide, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{Op::kModulo, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},

    // Binary arithmetic expressions with mixed scalar, vector, and matrix
    // operands
    Params{Op::kMultiply, ast_vec3<f32>, ast_f32, sem_vec3<sem_f32>},
    Params{Op::kMultiply, ast_f32, ast_vec3<f32>, sem_vec3<sem_f32>},

    Params{Op::kMultiply, ast_mat3x3<f32>, ast_f32, sem_mat3x3<sem_f32>},
    Params{Op::kMultiply, ast_f32, ast_mat3x3<f32>, sem_mat3x3<sem_f32>},

    Params{Op::kMultiply, ast_vec3<f32>, ast_mat3x3<f32>, sem_vec3<sem_f32>},
    Params{Op::kMultiply, ast_mat3x3<f32>, ast_vec3<f32>, sem_vec3<sem_f32>},
    Params{Op::kMultiply, ast_mat3x3<f32>, ast_mat3x3<f32>,
           sem_mat3x3<sem_f32>},

    // Comparison expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#comparison-expr

    // Comparisons over scalars
    Params{Op::kEqual, ast_bool, ast_bool, sem_bool},
    Params{Op::kNotEqual, ast_bool, ast_bool, sem_bool},

    Params{Op::kEqual, ast_i32, ast_i32, sem_bool},
    Params{Op::kNotEqual, ast_i32, ast_i32, sem_bool},
    Params{Op::kLessThan, ast_i32, ast_i32, sem_bool},
    Params{Op::kLessThanEqual, ast_i32, ast_i32, sem_bool},
    Params{Op::kGreaterThan, ast_i32, ast_i32, sem_bool},
    Params{Op::kGreaterThanEqual, ast_i32, ast_i32, sem_bool},

    Params{Op::kEqual, ast_u32, ast_u32, sem_bool},
    Params{Op::kNotEqual, ast_u32, ast_u32, sem_bool},
    Params{Op::kLessThan, ast_u32, ast_u32, sem_bool},
    Params{Op::kLessThanEqual, ast_u32, ast_u32, sem_bool},
    Params{Op::kGreaterThan, ast_u32, ast_u32, sem_bool},
    Params{Op::kGreaterThanEqual, ast_u32, ast_u32, sem_bool},

    Params{Op::kEqual, ast_f32, ast_f32, sem_bool},
    Params{Op::kNotEqual, ast_f32, ast_f32, sem_bool},
    Params{Op::kLessThan, ast_f32, ast_f32, sem_bool},
    Params{Op::kLessThanEqual, ast_f32, ast_f32, sem_bool},
    Params{Op::kGreaterThan, ast_f32, ast_f32, sem_bool},
    Params{Op::kGreaterThanEqual, ast_f32, ast_f32, sem_bool},

    // Comparisons over vectors
    Params{Op::kEqual, ast_vec3<bool>, ast_vec3<bool>, sem_vec3<sem_bool>},
    Params{Op::kNotEqual, ast_vec3<bool>, ast_vec3<bool>, sem_vec3<sem_bool>},

    Params{Op::kEqual, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_bool>},
    Params{Op::kNotEqual, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_bool>},
    Params{Op::kLessThan, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_bool>},
    Params{Op::kLessThanEqual, ast_vec3<i32>, ast_vec3<i32>,
           sem_vec3<sem_bool>},
    Params{Op::kGreaterThan, ast_vec3<i32>, ast_vec3<i32>, sem_vec3<sem_bool>},
    Params{Op::kGreaterThanEqual, ast_vec3<i32>, ast_vec3<i32>,
           sem_vec3<sem_bool>},

    Params{Op::kEqual, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_bool>},
    Params{Op::kNotEqual, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_bool>},
    Params{Op::kLessThan, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_bool>},
    Params{Op::kLessThanEqual, ast_vec3<u32>, ast_vec3<u32>,
           sem_vec3<sem_bool>},
    Params{Op::kGreaterThan, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_bool>},
    Params{Op::kGreaterThanEqual, ast_vec3<u32>, ast_vec3<u32>,
           sem_vec3<sem_bool>},

    Params{Op::kEqual, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_bool>},
    Params{Op::kNotEqual, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_bool>},
    Params{Op::kLessThan, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_bool>},
    Params{Op::kLessThanEqual, ast_vec3<f32>, ast_vec3<f32>,
           sem_vec3<sem_bool>},
    Params{Op::kGreaterThan, ast_vec3<f32>, ast_vec3<f32>, sem_vec3<sem_bool>},
    Params{Op::kGreaterThanEqual, ast_vec3<f32>, ast_vec3<f32>,
           sem_vec3<sem_bool>},

    // Bit expressions
    // https://gpuweb.github.io/gpuweb/wgsl.html#bit-expr

    // Binary bitwise operations
    Params{Op::kOr, ast_i32, ast_i32, sem_i32},
    Params{Op::kAnd, ast_i32, ast_i32, sem_i32},
    Params{Op::kXor, ast_i32, ast_i32, sem_i32},

    Params{Op::kOr, ast_u32, ast_u32, sem_u32},
    Params{Op::kAnd, ast_u32, ast_u32, sem_u32},
    Params{Op::kXor, ast_u32, ast_u32, sem_u32},

    // Bit shift expressions
    Params{Op::kShiftLeft, ast_i32, ast_u32, sem_i32},
    Params{Op::kShiftLeft, ast_vec3<i32>, ast_vec3<u32>, sem_vec3<sem_i32>},

    Params{Op::kShiftLeft, ast_u32, ast_u32, sem_u32},
    Params{Op::kShiftLeft, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>},

    Params{Op::kShiftRight, ast_i32, ast_u32, sem_i32},
    Params{Op::kShiftRight, ast_vec3<i32>, ast_vec3<u32>, sem_vec3<sem_i32>},

    Params{Op::kShiftRight, ast_u32, ast_u32, sem_u32},
    Params{Op::kShiftRight, ast_vec3<u32>, ast_vec3<u32>, sem_vec3<sem_u32>}};

using Expr_Binary_Test_Valid = ResolverTestWithParam<Params>;
TEST_P(Expr_Binary_Test_Valid, All) {
  auto& params = GetParam();

  auto* lhs_type = params.create_lhs_type(ty);
  auto* rhs_type = params.create_rhs_type(ty);
  auto* result_type = params.create_result_type(ty);

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << params.op << " "
     << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kInput);
  Global("rhs", rhs_type, ast::StorageClass::kInput);

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

  auto* lhs_type = params.create_lhs_type(ty);
  auto* rhs_type = params.create_rhs_type(ty);

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << params.op << " "
     << FriendlyName(rhs_type);

  // For vectors and matrices, wrap the sub type in an alias
  auto make_alias = [this](ast::Type* type) -> ast::Type* {
    if (auto* v = type->As<ast::Vector>()) {
      auto* alias = ty.alias(Symbols().New(), v->type());
      AST().AddConstructedType(alias);
      return ty.vec(alias, v->size());
    }
    if (auto* m = type->As<ast::Matrix>()) {
      auto* alias = ty.alias(Symbols().New(), m->type());
      AST().AddConstructedType(alias);
      return ty.mat(alias, m->columns(), m->rows());
    }
    auto* alias = ty.alias(Symbols().New(), type);
    AST().AddConstructedType(alias);
    return ty.type_name(alias->name());
  };

  // Wrap in alias
  if (side == BinaryExprSide::Left || side == BinaryExprSide::Both) {
    lhs_type = make_alias(lhs_type);
  }
  if (side == BinaryExprSide::Right || side == BinaryExprSide::Both) {
    rhs_type = make_alias(rhs_type);
  }

  ss << ", After aliasing: " << FriendlyName(lhs_type) << " " << params.op
     << " " << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kInput);
  Global("rhs", rhs_type, ast::StorageClass::kInput);

  auto* expr =
      create<ast::BinaryExpression>(params.op, Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  ASSERT_TRUE(r()->Resolve()) << r()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  // TODO(amaiorano): Bring this back once we have a way to get the canonical
  // type
  // auto* *result_type = params.create_result_type(ty);
  // ASSERT_TRUE(TypeOf(expr) == result_type);
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Expr_Binary_Test_WithAlias_Valid,
    testing::Combine(testing::ValuesIn(all_valid_cases),
                     testing::Values(BinaryExprSide::Left,
                                     BinaryExprSide::Right,
                                     BinaryExprSide::Both)));

using Expr_Binary_Test_Invalid =
    ResolverTestWithParam<std::tuple<Params, create_ast_type_func_ptr>>;
TEST_P(Expr_Binary_Test_Invalid, All) {
  const Params& params = std::get<0>(GetParam());
  auto& create_type_func = std::get<1>(GetParam());

  // Currently, for most operations, for a given lhs type, there is exactly one
  // rhs type allowed.  The only exception is for multiplication, which allows
  // any permutation of f32, vecN<f32>, and matNxN<f32>. We are fed valid inputs
  // only via `params`, and all possible types via `create_type_func`, so we
  // test invalid combinations by testing every other rhs type, modulo
  // exceptions.

  // Skip valid rhs type
  if (params.create_rhs_type == create_type_func) {
    return;
  }

  auto* lhs_type = params.create_lhs_type(ty);
  auto* rhs_type = create_type_func(ty);

  // Skip exceptions: multiplication of f32, vecN<f32>, and matNxN<f32>
  if (params.op == Op::kMultiply &&
      lhs_type->is_float_scalar_or_vector_or_matrix() &&
      rhs_type->is_float_scalar_or_vector_or_matrix()) {
    return;
  }

  std::stringstream ss;
  ss << FriendlyName(lhs_type) << " " << params.op << " "
     << FriendlyName(rhs_type);
  SCOPED_TRACE(ss.str());

  Global("lhs", lhs_type, ast::StorageClass::kInput);
  Global("rhs", rhs_type, ast::StorageClass::kInput);

  auto* expr = create<ast::BinaryExpression>(Source{{12, 34}}, params.op,
                                             Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  ASSERT_FALSE(r()->Resolve());
  ASSERT_EQ(r()->error(),
            "12:34 error: Binary expression operand types are invalid for "
            "this operation: " +
                FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op()) +
                " " + FriendlyName(rhs_type));
}
INSTANTIATE_TEST_SUITE_P(
    ResolverTest,
    Expr_Binary_Test_Invalid,
    testing::Combine(testing::ValuesIn(all_valid_cases),
                     testing::ValuesIn(all_create_type_funcs)));

using Expr_Binary_Test_Invalid_VectorMatrixMultiply =
    ResolverTestWithParam<std::tuple<bool, uint32_t, uint32_t, uint32_t>>;
TEST_P(Expr_Binary_Test_Invalid_VectorMatrixMultiply, All) {
  bool vec_by_mat = std::get<0>(GetParam());
  uint32_t vec_size = std::get<1>(GetParam());
  uint32_t mat_rows = std::get<2>(GetParam());
  uint32_t mat_cols = std::get<3>(GetParam());

  ast::Type* lhs_type;
  ast::Type* rhs_type;
  sem::Type* result_type;
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

  Global("lhs", lhs_type, ast::StorageClass::kInput);
  Global("rhs", rhs_type, ast::StorageClass::kInput);

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
                  FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op()) +
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

  Global("lhs", lhs_type, ast::StorageClass::kInput);
  Global("rhs", rhs_type, ast::StorageClass::kInput);

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
                  FriendlyName(lhs_type) + " " + ast::FriendlyName(expr->op()) +
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

  Global("ident", ty.vec4<f32>(), ast::StorageClass::kInput);
  auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
  WrapInFunction(der);

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  ASSERT_NE(TypeOf(der), nullptr);
  ASSERT_TRUE(TypeOf(der)->Is<sem::Vector>());
  EXPECT_TRUE(TypeOf(der)->As<sem::Vector>()->type()->Is<sem::F32>());
  EXPECT_EQ(TypeOf(der)->As<sem::Vector>()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(ResolverTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(ResolverTest, StorageClass_SetsIfMissing) {
  auto* var = Var("var", ty.i32(), ast::StorageClass::kNone, nullptr,
                  {
                      create<ast::BindingDecoration>(0),
                      create<ast::GroupDecoration>(0),
                  });

  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kFunction);
}

TEST_F(ResolverTest, StorageClass_SetForSampler) {
  auto* t = ty.sampler(ast::SamplerKind::kSampler);
  auto* var = Global("var", t, ast::StorageClass::kNone, nullptr,
                     {
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(),
            ast::StorageClass::kUniformConstant);
}

TEST_F(ResolverTest, StorageClass_SetForTexture) {
  auto* t = ty.sampled_texture(ast::TextureDimension::k1d, ty.f32());
  auto* ac = ty.access(ast::AccessControl::kReadOnly, t);
  auto* var = Global("var", ac, ast::StorageClass::kNone, nullptr,
                     {
                         create<ast::BindingDecoration>(0),
                         create<ast::GroupDecoration>(0),
                     });

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(),
            ast::StorageClass::kUniformConstant);
}

TEST_F(ResolverTest, StorageClass_DoesNotSetOnConst) {
  auto* var = Const("var", ty.i32(), Construct(ty.i32()));
  auto* stmt = Decl(var);
  Func("func", ast::VariableList{}, ty.void_(), {stmt}, ast::DecorationList{});

  EXPECT_TRUE(r()->Resolve()) << r()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kNone);
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

  Global("first", ty.f32(), ast::StorageClass::kPrivate);
  Global("second", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_a", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_b", ty.f32(), ast::StorageClass::kPrivate);
  Global("call_c", ty.f32(), ast::StorageClass::kPrivate);

  ast::VariableList params;
  auto* func_b =
      Func("b", params, ty.f32(), {Return(0.0f)}, ast::DecorationList{});
  auto* func_c =
      Func("c", params, ty.f32(), {Assign("second", Call("b")), Return(0.0f)},
           ast::DecorationList{});

  auto* func_a =
      Func("a", params, ty.f32(), {Assign("first", Call("c")), Return(0.0f)},
           ast::DecorationList{});

  auto* ep_1 = Func("ep_1", params, ty.void_(),
                    {
                        Assign("call_a", Call("a")),
                        Assign("call_b", Call("b")),
                    },
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kCompute),
                    });

  auto* ep_2 = Func("ep_2", params, ty.void_(),
                    {
                        Assign("call_c", Call("c")),
                    },
                    ast::DecorationList{
                        Stage(ast::PipelineStage::kCompute),
                    });

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
           Stage(ast::PipelineStage::kCompute),
       });

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
        ProgramBuilder builder;
        builder.Expr("1");
        Resolver(&builder).Resolve();
      },
      "internal compiler error: AST node 'tint::ast::IdentifierExpression' was "
      "not reached by the resolver");
}

TEST_F(ResolverTest, ASTNodeReachedTwice) {
  EXPECT_FATAL_FAILURE(
      {
        ProgramBuilder builder;
        auto* expr = builder.Expr("1");
        auto* usesExprTwice = builder.Add(expr, expr);
        builder.Global("g", builder.ty.i32(), ast::StorageClass::kPrivate,
                       usesExprTwice);
        Resolver(&builder).Resolve();
      },
      "internal compiler error: AST node 'tint::ast::IdentifierExpression' was "
      "encountered twice in the same AST of a Program");
}

}  // namespace
}  // namespace resolver
}  // namespace tint
