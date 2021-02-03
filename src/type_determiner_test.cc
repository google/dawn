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

#include "src/type_determiner.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic_texture_helper_test.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/semantic/call.h"
#include "src/semantic/expression.h"
#include "src/semantic/function.h"
#include "src/semantic/variable.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/bool_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/pointer_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/sampler_type.h"
#include "src/type/storage_texture_type.h"
#include "src/type/struct_type.h"
#include "src/type/texture_type.h"
#include "src/type/u32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace {

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

class TypeDeterminerHelper : public ProgramBuilder {
 public:
  TypeDeterminerHelper() : td_(std::make_unique<TypeDeterminer>(this)) {}

  TypeDeterminer* td() const { return td_.get(); }

 private:
  std::unique_ptr<TypeDeterminer> td_;
};

class TypeDeterminerTest : public TypeDeterminerHelper, public testing::Test {};

template <typename T>
class TypeDeterminerTestWithParam : public TypeDeterminerHelper,
                                    public testing::TestWithParam<T> {};

TEST_F(TypeDeterminerTest, Error_WithEmptySource) {
  auto* s = create<FakeStmt>();
  WrapInFunction(s);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Error_Unknown) {
  auto* s = create<FakeStmt>(Source{Source::Location{2, 30}});
  WrapInFunction(s);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "2:30: unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Assign) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* assign = create<ast::AssignmentStatement>(lhs, rhs);
  WrapInFunction(assign);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Case) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(ty.i32(), 3));
  auto* cse = create<ast::CaseStatement>(lit, body);
  WrapInFunction(cse);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Block) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* block = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  WrapInFunction(block);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Else) {
  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  auto* stmt = create<ast::ElseStatement>(Expr(3), body);
  WrapInFunction(stmt);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);
  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_If) {
  auto* else_lhs = Expr(2);
  auto* else_rhs = Expr(2.3f);

  auto* else_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(else_lhs, else_rhs),
  });

  auto* else_stmt = create<ast::ElseStatement>(Expr(3), else_body);

  auto* lhs = Expr(2);
  auto* rhs = Expr(2.3f);

  auto* body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::AssignmentStatement>(lhs, rhs),
  });
  auto* stmt = create<ast::IfStatement>(Expr(3), body,
                                        ast::ElseStatementList{else_stmt});
  WrapInFunction(stmt);

  EXPECT_TRUE(td()->Determine()) << td()->error();

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
}

TEST_F(TypeDeterminerTest, Stmt_Loop) {
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

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(body_lhs), nullptr);
  ASSERT_NE(TypeOf(body_rhs), nullptr);
  ASSERT_NE(TypeOf(continuing_lhs), nullptr);
  ASSERT_NE(TypeOf(continuing_rhs), nullptr);
  EXPECT_TRUE(TypeOf(body_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(body_rhs)->Is<type::F32>());
  EXPECT_TRUE(TypeOf(continuing_lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(continuing_rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Return) {
  auto* cond = Expr(2);

  auto* ret = create<ast::ReturnStatement>(cond);
  WrapInFunction(ret);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(cond), nullptr);
  EXPECT_TRUE(TypeOf(cond)->Is<type::I32>());
}

TEST_F(TypeDeterminerTest, Stmt_Return_WithoutValue) {
  auto* ret = create<ast::ReturnStatement>();
  WrapInFunction(ret);

  EXPECT_TRUE(td()->Determine()) << td()->error();
}

TEST_F(TypeDeterminerTest, Stmt_Switch) {
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

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(stmt->condition()), nullptr);
  ASSERT_NE(TypeOf(lhs), nullptr);
  ASSERT_NE(TypeOf(rhs), nullptr);

  EXPECT_TRUE(TypeOf(stmt->condition())->Is<type::I32>());
  EXPECT_TRUE(TypeOf(lhs)->Is<type::I32>());
  EXPECT_TRUE(TypeOf(rhs)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* expr = Call("my_func");

  auto* call = create<ast::CallStatement>(expr);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Call_undeclared) {
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

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: func");
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl) {
  auto* var = Var("my_var", ast::StorageClass::kNone, ty.i32(), Expr(2),
                  ast::VariableDecorationList{});
  auto* init = var->constructor();

  auto* decl = create<ast::VariableDeclStatement>(var);
  WrapInFunction(decl);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl_ModuleScope) {
  auto* var = Global("my_var", ast::StorageClass::kNone, ty.i32(), Expr(2),
                     ast::VariableDecorationList{});
  auto* init = var->constructor();

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(init), nullptr);
  EXPECT_TRUE(TypeOf(init)->Is<type::I32>());
}

TEST_F(TypeDeterminerTest, Expr_Error_Unknown) {
  FakeExpr e(Source{Source::Location{2, 30}});
  WrapInFunction(&e);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(), "2:30: unknown expression for type determination");
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array) {
  auto* idx = Expr(2);
  Global("my_var", ast::StorageClass::kFunction, ty.array<f32, 3>());

  auto* acc = IndexAccessor("my_var", idx);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Alias_Array) {
  auto* aary = ty.alias("myarrty", ty.array<f32, 3>());

  Global("my_var", ast::StorageClass::kFunction, aary);

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array_Constant) {
  GlobalConst("my_var", ast::StorageClass::kFunction, ty.array<f32, 3>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  EXPECT_TRUE(TypeOf(acc)->Is<type::F32>()) << TypeOf(acc)->type_name();
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix) {
  Global("my_var", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::Vector>());
  EXPECT_EQ(ptr->type()->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  Global("my_var", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* acc = IndexAccessor(IndexAccessor("my_var", 2), 1);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Vector) {
  Global("my_var", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* acc = IndexAccessor("my_var", 2);
  WrapInFunction(acc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(acc), nullptr);
  ASSERT_TRUE(TypeOf(acc)->Is<type::Pointer>());

  auto* ptr = TypeOf(acc)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Bitcast) {
  auto* bitcast = create<ast::BitcastExpression>(ty.f32(), Expr("name"));
  WrapInFunction(bitcast);

  Global("name", ast::StorageClass::kPrivate, ty.f32());

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(bitcast), nullptr);
  EXPECT_TRUE(TypeOf(bitcast)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call_WithParams) {
  ast::VariableList params;
  Func("my_func", params, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* param = Expr(2.4f);

  auto* call = Call("my_func", param);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(param), nullptr);
  EXPECT_TRUE(TypeOf(param)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call_Intrinsic) {
  auto* call = Call("round", 2.4f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Cast) {
  Global("name", ast::StorageClass::kPrivate, ty.f32());

  auto* cast = Construct(ty.f32(), "name");
  WrapInFunction(cast);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(cast), nullptr);
  EXPECT_TRUE(TypeOf(cast)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Scalar) {
  auto* s = Expr(1.0f);
  WrapInFunction(s);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(s), nullptr);
  EXPECT_TRUE(TypeOf(s)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Type) {
  auto* tc = vec3<f32>(1.0f, 1.0f, 3.0f);
  WrapInFunction(tc);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(tc), nullptr);
  ASSERT_TRUE(TypeOf(tc)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(tc)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(tc)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalVariable) {
  Global("my_var", ast::StorageClass::kNone, ty.f32());

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(ident)->As<type::Pointer>()->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalConstant) {
  GlobalConst("my_var", ast::StorageClass::kNone, ty.f32());

  auto* ident = Expr("my_var");
  WrapInFunction(ident);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(ident), nullptr);
  EXPECT_TRUE(TypeOf(ident)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable_Const) {
  auto* my_var = Expr("my_var");

  auto* var = Const("my_var", ast::StorageClass::kNone, ty.f32());

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::AssignmentStatement>(my_var, Expr("my_var")),
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(my_var), nullptr);
  EXPECT_TRUE(TypeOf(my_var)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable) {
  auto* my_var = Expr("my_var");

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("my_var", ast::StorageClass::kNone, ty.f32())),
           create<ast::AssignmentStatement>(my_var, Expr("my_var")),
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(my_var), nullptr);
  EXPECT_TRUE(TypeOf(my_var)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var)->As<type::Pointer>()->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Function_Ptr) {
  type::Pointer ptr(ty.f32(), ast::StorageClass::kFunction);

  auto* my_var = Expr("my_var");

  Func("my_func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(
               Var("my_var", ast::StorageClass::kNone, &ptr)),
           create<ast::AssignmentStatement>(my_var, Expr("my_var")),
       },
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(my_var), nullptr);
  EXPECT_TRUE(TypeOf(my_var)->Is<type::Pointer>());
  EXPECT_TRUE(TypeOf(my_var)->As<type::Pointer>()->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call_Function) {
  Func("my_func", ast::VariableList{}, ty.f32(), ast::StatementList{},
       ast::FunctionDecorationList{});

  auto* call = Call("my_func");
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Unknown) {
  auto* a = Expr("a");
  WrapInFunction(a);

  EXPECT_FALSE(td()->Determine());
}

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables) {
  auto* in_var = Global("in_var", ast::StorageClass::kInput, ty.f32());
  auto* out_var = Global("out_var", ast::StorageClass::kOutput, ty.f32());
  auto* sb_var = Global("sb_var", ast::StorageClass::kStorage, ty.f32());
  auto* wg_var = Global("wg_var", ast::StorageClass::kWorkgroup, ty.f32());
  auto* priv_var = Global("priv_var", ast::StorageClass::kPrivate, ty.f32());

  auto* func = Func(
      "my_func", ast::VariableList{}, ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("out_var"), Expr("in_var")),
          create<ast::AssignmentStatement>(Expr("wg_var"), Expr("wg_var")),
          create<ast::AssignmentStatement>(Expr("sb_var"), Expr("sb_var")),
          create<ast::AssignmentStatement>(Expr("priv_var"), Expr("priv_var")),
      },
      ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

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

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables_SubFunction) {
  auto* in_var = Global("in_var", ast::StorageClass::kInput, ty.f32());
  auto* out_var = Global("out_var", ast::StorageClass::kOutput, ty.f32());
  auto* sb_var = Global("sb_var", ast::StorageClass::kStorage, ty.f32());
  auto* wg_var = Global("wg_var", ast::StorageClass::kWorkgroup, ty.f32());
  auto* priv_var = Global("priv_var", ast::StorageClass::kPrivate, ty.f32());

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

  EXPECT_TRUE(td()->Determine()) << td()->error();

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

TEST_F(TypeDeterminerTest, Function_NotRegisterFunctionVariable) {
  auto* var = Var("in_var", ast::StorageClass::kFunction, ty.f32());

  auto* func =
      Func("my_func", ast::VariableList{}, ty.f32(),
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::AssignmentStatement>(Expr("var"), Expr(1.f)),
           },
           ast::FunctionDecorationList{});

  Global("var", ast::StorageClass::kFunction, ty.f32());

  EXPECT_TRUE(td()->Determine()) << td()->error();

  auto* func_sem = Sem().Get(func);
  ASSERT_NE(func_sem, nullptr);

  EXPECT_EQ(func_sem->ReferencedModuleVariables().size(), 0u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("S", strct);
  Global("my_struct", ast::StorageClass::kNone, st);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct_Alias) {
  auto* strct = create<ast::Struct>(
      ast::StructMemberList{Member("first_member", ty.i32()),
                            Member("second_member", ty.f32())},
      ast::StructDecorationList{});

  auto* st = ty.struct_("alias", strct);
  auto* alias = ty.alias("alias", st);
  Global("my_struct", ast::StorageClass::kNone, alias);

  auto* mem = MemberAccessor("my_struct", "second_member");
  WrapInFunction(mem);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle) {
  Global("my_vec", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* mem = MemberAccessor("my_vec", "xy");
  WrapInFunction(mem);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  Global("my_vec", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* mem = MemberAccessor("my_vec", "x");
  WrapInFunction(mem);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Pointer>());

  auto* ptr = TypeOf(mem)->As<type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Accessor_MultiLevel) {
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
  Global("c", ast::StorageClass::kNone, stA);

  auto* mem = MemberAccessor(
      MemberAccessor(IndexAccessor(MemberAccessor("c", "mem"), 0), "foo"),
      "yx");
  WrapInFunction(mem);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(mem), nullptr);
  ASSERT_TRUE(TypeOf(mem)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(mem)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(mem)->As<type::Vector>()->size(), 2u);
}

using Expr_Binary_BitwiseTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_BitwiseTest, Scalar) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.i32());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_P(Expr_Binary_BitwiseTest, Vector) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.vec3<i32>());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::I32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
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

using Expr_Binary_LogicalTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_LogicalTest, Scalar) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.bool_());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_LogicalTest, Vector) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.vec3<bool>());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Expr_Binary_LogicalTest,
                         testing::Values(ast::BinaryOp::kLogicalAnd,
                                         ast::BinaryOp::kLogicalOr));

using Expr_Binary_CompareTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_CompareTest, Scalar) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.i32());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Expr_Binary_CompareTest, Vector) {
  auto op = GetParam();

  Global("val", ast::StorageClass::kNone, ty.vec3<i32>());

  auto* expr = create<ast::BinaryExpression>(op, Expr("val"), Expr("val"));
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Expr_Binary_CompareTest,
                         testing::Values(ast::BinaryOp::kEqual,
                                         ast::BinaryOp::kNotEqual,
                                         ast::BinaryOp::kLessThan,
                                         ast::BinaryOp::kGreaterThan,
                                         ast::BinaryOp::kLessThanEqual,
                                         ast::BinaryOp::kGreaterThanEqual));

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Scalar) {
  Global("val", ast::StorageClass::kNone, ty.i32());

  auto* expr = Mul("val", "val");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::I32>());
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Scalar) {
  Global("scalar", ast::StorageClass::kNone, ty.f32());
  Global("vector", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* expr = Mul("vector", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Vector) {
  Global("scalar", ast::StorageClass::kNone, ty.f32());
  Global("vector", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* expr = Mul("scalar", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Vector) {
  Global("vector", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* expr = Mul("vector", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Scalar) {
  Global("scalar", ast::StorageClass::kNone, ty.f32());
  Global("matrix", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* expr = Mul("matrix", "scalar");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Matrix) {
  Global("scalar", ast::StorageClass::kNone, ty.f32());
  Global("matrix", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* expr = Mul("scalar", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Vector) {
  Global("vector", ast::StorageClass::kNone, ty.vec3<f32>());
  Global("matrix", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* expr = Mul("matrix", "vector");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Matrix) {
  Global("vector", ast::StorageClass::kNone, ty.vec3<f32>());
  Global("matrix", ast::StorageClass::kNone, ty.mat2x3<f32>());

  auto* expr = Mul("vector", "matrix");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Matrix) {
  Global("mat3x4", ast::StorageClass::kNone, ty.mat3x4<f32>());
  Global("mat4x3", ast::StorageClass::kNone, ty.mat4x3<f32>());

  auto* expr = Mul("mat3x4", "mat4x3");
  WrapInFunction(expr);

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Matrix>());

  auto* mat = TypeOf(expr)->As<type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<type::F32>());
  EXPECT_EQ(mat->rows(), 4u);
  EXPECT_EQ(mat->columns(), 4u);
}

using IntrinsicDerivativeTest = TypeDeterminerTestWithParam<std::string>;
TEST_P(IntrinsicDerivativeTest, Scalar) {
  auto name = GetParam();

  Global("ident", ast::StorageClass::kNone, ty.f32());

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_P(IntrinsicDerivativeTest, Vector) {
  auto name = GetParam();
  Global("ident", ast::StorageClass::kNone, ty.vec4<f32>());

  auto* expr = Call(name, "ident");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 4u);
}

TEST_P(IntrinsicDerivativeTest, MissingParam) {
  auto name = GetParam();

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(name));
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
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

using Intrinsic = TypeDeterminerTestWithParam<std::string>;
TEST_P(Intrinsic, Test) {
  auto name = GetParam();

  Global("my_var", ast::StorageClass::kNone, ty.vec3<bool>());

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Intrinsic,
                         testing::Values("any", "all"));

using Intrinsic_FloatMethod = TypeDeterminerTestWithParam<std::string>;
TEST_P(Intrinsic_FloatMethod, Vector) {
  auto name = GetParam();

  Global("my_var", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  ASSERT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::Bool>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatMethod, Scalar) {
  auto name = GetParam();

  Global("my_var", ast::StorageClass::kNone, ty.f32());

  auto* expr = Call(name, "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Bool>());
}

TEST_P(Intrinsic_FloatMethod, MissingParam) {
  auto name = GetParam();

  Global("my_var", ast::StorageClass::kNone, ty.f32());

  auto* expr = Call(name);
  WrapInFunction(expr);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}

TEST_P(Intrinsic_FloatMethod, TooManyParams) {
  auto name = GetParam();

  Global("my_var", ast::StorageClass::kNone, ty.f32());

  auto* expr = Call(name, "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
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
    : public TypeDeterminerTestWithParam<TextureTestParams> {
 public:
  type::Type* get_coords_type(type::TextureDimension dim, type::Type* type) {
    if (dim == type::TextureDimension::k1d) {
      if (type->Is<type::I32>()) {
        return create<type::I32>();
      } else if (type->Is<type::U32>()) {
        return create<type::U32>();
      } else {
        return create<type::F32>();
      }
    } else if (dim == type::TextureDimension::k1dArray ||
               dim == type::TextureDimension::k2d) {
      return create<type::Vector>(type, 2);
    } else if (dim == type::TextureDimension::kCubeArray) {
      return create<type::Vector>(type, 4);
    } else {
      return create<type::Vector>(type, 3);
    }
  }

  void add_call_param(std::string name,
                      type::Type* type,
                      ast::ExpressionList* call_params) {
    Global(name, ast::StorageClass::kNone, type);
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

  auto* coords_type = get_coords_type(dim, ty.i32());

  auto* subtype = type::StorageTexture::SubtypeFor(format, this);
  type::Type* texture_type = create<type::StorageTexture>(dim, format, subtype);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);
  add_call_param("lod", ty.i32(), &call_params);

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

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
    TypeDeterminerTest,
    Intrinsic_StorageTextureOperation,
    testing::Values(
        TextureTestParams{type::TextureDimension::k1d, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k1d, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k1d, Texture::kF32,
                          type::ImageFormat::kR8Unorm},
        TextureTestParams{type::TextureDimension::k1dArray, Texture::kF32,
                          type::ImageFormat::kR16Float},
        TextureTestParams{type::TextureDimension::k1dArray, Texture::kI32,
                          type::ImageFormat::kR16Sint},
        TextureTestParams{type::TextureDimension::k1dArray, Texture::kF32,
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
  auto* coords_type = get_coords_type(dim, ty.i32());
  auto* texture_type = create<type::SampledTexture>(dim, s);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type, &call_params);
  add_call_param("lod", ty.i32(), &call_params);

  auto* expr = Call("textureLoad", call_params);
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

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
    TypeDeterminerTest,
    Intrinsic_SampledTextureOperation,
    testing::Values(TextureTestParams{type::TextureDimension::k2d},
                    TextureTestParams{type::TextureDimension::k2dArray},
                    TextureTestParams{type::TextureDimension::kCube},
                    TextureTestParams{type::TextureDimension::kCubeArray}));

TEST_F(TypeDeterminerTest, Intrinsic_Dot) {
  Global("my_var", ast::StorageClass::kNone, ty.vec3<f32>());

  auto* expr = Call("dot", "my_var", "my_var");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select) {
  Global("my_var", ast::StorageClass::kNone, ty.vec3<f32>());

  Global("bool_var", ast::StorageClass::kNone, ty.vec3<bool>());

  auto* expr = Call("select", "my_var", "my_var", "bool_var");
  WrapInFunction(expr);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(expr), nullptr);
  EXPECT_TRUE(TypeOf(expr)->Is<type::Vector>());
  EXPECT_EQ(TypeOf(expr)->As<type::Vector>()->size(), 3u);
  EXPECT_TRUE(TypeOf(expr)->As<type::Vector>()->type()->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select_NoParams) {
  auto* expr = Call("select");
  WrapInFunction(expr);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(
      td()->error(),
      "missing parameter 0 required for type determination in builtin select");
}

using UnaryOpExpressionTest = TypeDeterminerTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  Global("ident", ast::StorageClass::kNone, ty.vec4<f32>());
  auto* der = create<ast::UnaryOpExpression>(op, Expr("ident"));
  WrapInFunction(der);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(der), nullptr);
  ASSERT_TRUE(TypeOf(der)->Is<type::Vector>());
  EXPECT_TRUE(TypeOf(der)->As<type::Vector>()->type()->Is<type::F32>());
  EXPECT_EQ(TypeOf(der)->As<type::Vector>()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(TypeDeterminerTest, StorageClass_SetsIfMissing) {
  auto* var = Var("var", ast::StorageClass::kNone, ty.i32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kFunction);
}

TEST_F(TypeDeterminerTest, StorageClass_DoesNotSetOnConst) {
  auto* var = Const("var", ast::StorageClass::kNone, ty.i32());
  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->Determine()) << td()->error();

  EXPECT_EQ(Sem().Get(var)->StorageClass(), ast::StorageClass::kNone);
}

TEST_F(TypeDeterminerTest, StorageClass_NonFunctionClassError) {
  auto* var = Var("var", ast::StorageClass::kWorkgroup, ty.i32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  Func("func", ast::VariableList{}, ty.i32(), ast::StatementList{stmt},
       ast::FunctionDecorationList{});

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "function variable has a non-function storage class");
}

struct IntrinsicData {
  const char* name;
  semantic::Intrinsic intrinsic;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}
using IntrinsicDataTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(IntrinsicDataTest, Lookup) {
  auto param = GetParam();

  EXPECT_EQ(TypeDeterminer::MatchIntrinsic(param.name), param.intrinsic);
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    IntrinsicDataTest,
    testing::Values(
        IntrinsicData{"abs", semantic::Intrinsic::kAbs},
        IntrinsicData{"acos", semantic::Intrinsic::kAcos},
        IntrinsicData{"all", semantic::Intrinsic::kAll},
        IntrinsicData{"any", semantic::Intrinsic::kAny},
        IntrinsicData{"arrayLength", semantic::Intrinsic::kArrayLength},
        IntrinsicData{"asin", semantic::Intrinsic::kAsin},
        IntrinsicData{"atan", semantic::Intrinsic::kAtan},
        IntrinsicData{"atan2", semantic::Intrinsic::kAtan2},
        IntrinsicData{"ceil", semantic::Intrinsic::kCeil},
        IntrinsicData{"clamp", semantic::Intrinsic::kClamp},
        IntrinsicData{"cos", semantic::Intrinsic::kCos},
        IntrinsicData{"cosh", semantic::Intrinsic::kCosh},
        IntrinsicData{"countOneBits", semantic::Intrinsic::kCountOneBits},
        IntrinsicData{"cross", semantic::Intrinsic::kCross},
        IntrinsicData{"determinant", semantic::Intrinsic::kDeterminant},
        IntrinsicData{"distance", semantic::Intrinsic::kDistance},
        IntrinsicData{"dot", semantic::Intrinsic::kDot},
        IntrinsicData{"dpdx", semantic::Intrinsic::kDpdx},
        IntrinsicData{"dpdxCoarse", semantic::Intrinsic::kDpdxCoarse},
        IntrinsicData{"dpdxFine", semantic::Intrinsic::kDpdxFine},
        IntrinsicData{"dpdy", semantic::Intrinsic::kDpdy},
        IntrinsicData{"dpdyCoarse", semantic::Intrinsic::kDpdyCoarse},
        IntrinsicData{"dpdyFine", semantic::Intrinsic::kDpdyFine},
        IntrinsicData{"exp", semantic::Intrinsic::kExp},
        IntrinsicData{"exp2", semantic::Intrinsic::kExp2},
        IntrinsicData{"faceForward", semantic::Intrinsic::kFaceForward},
        IntrinsicData{"floor", semantic::Intrinsic::kFloor},
        IntrinsicData{"fma", semantic::Intrinsic::kFma},
        IntrinsicData{"fract", semantic::Intrinsic::kFract},
        IntrinsicData{"frexp", semantic::Intrinsic::kFrexp},
        IntrinsicData{"fwidth", semantic::Intrinsic::kFwidth},
        IntrinsicData{"fwidthCoarse", semantic::Intrinsic::kFwidthCoarse},
        IntrinsicData{"fwidthFine", semantic::Intrinsic::kFwidthFine},
        IntrinsicData{"inverseSqrt", semantic::Intrinsic::kInverseSqrt},
        IntrinsicData{"isFinite", semantic::Intrinsic::kIsFinite},
        IntrinsicData{"isInf", semantic::Intrinsic::kIsInf},
        IntrinsicData{"isNan", semantic::Intrinsic::kIsNan},
        IntrinsicData{"isNormal", semantic::Intrinsic::kIsNormal},
        IntrinsicData{"ldexp", semantic::Intrinsic::kLdexp},
        IntrinsicData{"length", semantic::Intrinsic::kLength},
        IntrinsicData{"log", semantic::Intrinsic::kLog},
        IntrinsicData{"log2", semantic::Intrinsic::kLog2},
        IntrinsicData{"max", semantic::Intrinsic::kMax},
        IntrinsicData{"min", semantic::Intrinsic::kMin},
        IntrinsicData{"mix", semantic::Intrinsic::kMix},
        IntrinsicData{"modf", semantic::Intrinsic::kModf},
        IntrinsicData{"normalize", semantic::Intrinsic::kNormalize},
        IntrinsicData{"pow", semantic::Intrinsic::kPow},
        IntrinsicData{"reflect", semantic::Intrinsic::kReflect},
        IntrinsicData{"reverseBits", semantic::Intrinsic::kReverseBits},
        IntrinsicData{"round", semantic::Intrinsic::kRound},
        IntrinsicData{"select", semantic::Intrinsic::kSelect},
        IntrinsicData{"sign", semantic::Intrinsic::kSign},
        IntrinsicData{"sin", semantic::Intrinsic::kSin},
        IntrinsicData{"sinh", semantic::Intrinsic::kSinh},
        IntrinsicData{"smoothStep", semantic::Intrinsic::kSmoothStep},
        IntrinsicData{"sqrt", semantic::Intrinsic::kSqrt},
        IntrinsicData{"step", semantic::Intrinsic::kStep},
        IntrinsicData{"tan", semantic::Intrinsic::kTan},
        IntrinsicData{"tanh", semantic::Intrinsic::kTanh},
        IntrinsicData{"textureDimensions",
                      semantic::Intrinsic::kTextureDimensions},
        IntrinsicData{"textureLoad", semantic::Intrinsic::kTextureLoad},
        IntrinsicData{"textureNumLayers",
                      semantic::Intrinsic::kTextureNumLayers},
        IntrinsicData{"textureNumLevels",
                      semantic::Intrinsic::kTextureNumLevels},
        IntrinsicData{"textureNumSamples",
                      semantic::Intrinsic::kTextureNumSamples},
        IntrinsicData{"textureSample", semantic::Intrinsic::kTextureSample},
        IntrinsicData{"textureSampleBias",
                      semantic::Intrinsic::kTextureSampleBias},
        IntrinsicData{"textureSampleCompare",
                      semantic::Intrinsic::kTextureSampleCompare},
        IntrinsicData{"textureSampleGrad",
                      semantic::Intrinsic::kTextureSampleGrad},
        IntrinsicData{"textureSampleLevel",
                      semantic::Intrinsic::kTextureSampleLevel},
        IntrinsicData{"trunc", semantic::Intrinsic::kTrunc}));

TEST_F(TypeDeterminerTest, MatchIntrinsicNoMatch) {
  EXPECT_EQ(TypeDeterminer::MatchIntrinsic("not_intrinsic"),
            semantic::Intrinsic::kNone);
}

using ImportData_SingleParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_SingleParamTest, Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ImportData_SingleParamTest, Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_SingleParamTest,
    testing::Values(IntrinsicData{"acos", semantic::Intrinsic::kAcos},
                    IntrinsicData{"asin", semantic::Intrinsic::kAsin},
                    IntrinsicData{"atan", semantic::Intrinsic::kAtan},
                    IntrinsicData{"ceil", semantic::Intrinsic::kCeil},
                    IntrinsicData{"cos", semantic::Intrinsic::kCos},
                    IntrinsicData{"cosh", semantic::Intrinsic::kCosh},
                    IntrinsicData{"exp", semantic::Intrinsic::kExp},
                    IntrinsicData{"exp2", semantic::Intrinsic::kExp2},
                    IntrinsicData{"floor", semantic::Intrinsic::kFloor},
                    IntrinsicData{"fract", semantic::Intrinsic::kFract},
                    IntrinsicData{"inverseSqrt",
                                  semantic::Intrinsic::kInverseSqrt},
                    IntrinsicData{"log", semantic::Intrinsic::kLog},
                    IntrinsicData{"log2", semantic::Intrinsic::kLog2},
                    IntrinsicData{"normalize", semantic::Intrinsic::kNormalize},
                    IntrinsicData{"round", semantic::Intrinsic::kRound},
                    IntrinsicData{"sign", semantic::Intrinsic::kSign},
                    IntrinsicData{"sin", semantic::Intrinsic::kSin},
                    IntrinsicData{"sinh", semantic::Intrinsic::kSinh},
                    IntrinsicData{"sqrt", semantic::Intrinsic::kSqrt},
                    IntrinsicData{"tan", semantic::Intrinsic::kTan},
                    IntrinsicData{"tanh", semantic::Intrinsic::kTanh},
                    IntrinsicData{"trunc", semantic::Intrinsic::kTrunc}));

using ImportData_SingleParam_FloatOrInt_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_SingleParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, -1);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  ast::ExpressionList vals;
  vals.push_back(Expr(1));
  vals.push_back(Expr(1));
  vals.push_back(Expr(3));

  ast::ExpressionList params;
  params.push_back(vec3<i32>(vals));

  auto* ident = Expr(param.name);
  auto* call = Call(ident, params);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  ast::ExpressionList params;
  params.push_back(Expr(1u));

  auto* ident = Expr(param.name);
  auto* call = Call(ident, params);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_SingleParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{
                             "abs", semantic::Intrinsic::kAbs}));

TEST_F(TypeDeterminerTest, ImportData_Length_Scalar) {
  auto* ident = Expr("length");

  auto* call = Call(ident, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(TypeDeterminerTest, ImportData_Length_FloatVector) {
  ast::ExpressionList params;
  params.push_back(vec3<f32>(1.0f, 1.0f, 3.0f));

  auto* ident = Expr("length");

  auto* call = Call(ident, params);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

using ImportData_TwoParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_TwoParamTest, Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ImportData_TwoParamTest, Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call =
      Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}
TEST_P(ImportData_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_TwoParamTest,
    testing::Values(IntrinsicData{"atan2", semantic::Intrinsic::kAtan2},
                    IntrinsicData{"pow", semantic::Intrinsic::kPow},
                    IntrinsicData{"step", semantic::Intrinsic::kStep},
                    IntrinsicData{"reflect", semantic::Intrinsic::kReflect}));

TEST_F(TypeDeterminerTest, ImportData_Distance_Scalar) {
  auto* ident = Expr("distance");

  auto* call = Call(ident, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Vector) {
  auto* ident = Expr("distance");

  auto* call =
      Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_F(TypeDeterminerTest, ImportData_Cross) {
  auto* ident = Expr("cross");

  auto* call =
      Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, ImportData_Cross_AutoType) {
  auto* ident = Expr("cross");

  auto* call = Call(ident);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

using ImportData_ThreeParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_ThreeParamTest, Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ImportData_ThreeParamTest, Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}
TEST_P(ImportData_ThreeParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_ThreeParamTest,
    testing::Values(
        IntrinsicData{"mix", semantic::Intrinsic::kMix},
        IntrinsicData{"smoothStep", semantic::Intrinsic::kSmoothStep},
        IntrinsicData{"fma", semantic::Intrinsic::kFma},
        IntrinsicData{"faceForward", semantic::Intrinsic::kFaceForward}));

using ImportData_ThreeParam_FloatOrInt_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.f, 1.f, 1.f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_scalar());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<f32>(1.0f, 1.0f, 3.0f),
                    vec3<f32>(1.0f, 1.0f, 3.0f), vec3<f32>(1.0f, 1.0f, 3.0f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call =
      Call(ident, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1u, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u),
                    vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_ThreeParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{
                             "clamp", semantic::Intrinsic::kClamp}));

using ImportData_Int_SingleParamTest =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_Int_SingleParamTest, Scalar) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_integer_scalar());
}

TEST_P(ImportData_Int_SingleParamTest, Vector) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_Int_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_Int_SingleParamTest,
    testing::Values(
        IntrinsicData{"countOneBits", semantic::Intrinsic::kCountOneBits},
        IntrinsicData{"reverseBits", semantic::Intrinsic::kReverseBits}));

using ImportData_FloatOrInt_TwoParamTest =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Signed) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1, 1);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::I32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Unsigned) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1u, 1u);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::U32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Float) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, 1.0f, 1.0f);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Signed) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<i32>(1, 1, 3), vec3<i32>(1, 1, 3));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_signed_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Unsigned) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<u32>(1u, 1u, 3u), vec3<u32>(1u, 1u, 3u));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_unsigned_integer_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Float) {
  auto param = GetParam();

  auto* ident = Expr(param.name);
  auto* call = Call(ident, vec3<f32>(1.f, 1.f, 3.f), vec3<f32>(1.f, 1.f, 3.f));
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->is_float_vector());
  EXPECT_EQ(TypeOf(call)->As<type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_FALSE(td()->Determine());

  EXPECT_EQ(td()->error(),
            "missing parameter 0 required for type determination in builtin " +
                std::string(param.name));
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_FloatOrInt_TwoParamTest,
    testing::Values(IntrinsicData{"min", semantic::Intrinsic::kMin},
                    IntrinsicData{"max", semantic::Intrinsic::kMax}));

TEST_F(TypeDeterminerTest, ImportData_GLSL_Determinant) {
  Global("var", ast::StorageClass::kFunction, ty.mat3x3<f32>());

  auto* ident = Expr("determinant");

  auto* call = Call(ident, "var");
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ASSERT_NE(TypeOf(call), nullptr);
  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

using ImportData_Matrix_OneParam_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;

TEST_P(ImportData_Matrix_OneParam_Test, NoParams) {
  auto param = GetParam();

  auto* call = Call(param.name);
  WrapInFunction(call);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  EXPECT_TRUE(TypeOf(call)->Is<type::F32>());
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_Matrix_OneParam_Test,
                         testing::Values(IntrinsicData{
                             "determinant",
                             semantic::Intrinsic::kDeterminant}));

TEST_F(TypeDeterminerTest, Function_EntryPoints_StageDecoration) {
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

  Global("first", ast::StorageClass::kPrivate, ty.f32());
  Global("second", ast::StorageClass::kPrivate, ty.f32());
  Global("call_a", ast::StorageClass::kPrivate, ty.f32());
  Global("call_b", ast::StorageClass::kPrivate, ty.f32());
  Global("call_c", ast::StorageClass::kPrivate, ty.f32());

  ASSERT_TRUE(td()->Determine()) << td()->error();

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

using TypeDeterminerTextureIntrinsicTest =
    TypeDeterminerTestWithParam<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    TypeDeterminerTextureIntrinsicTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

std::string to_str(const std::string& function,
                   const semantic::TextureIntrinsicCall::Parameters& params) {
  struct Parameter {
    size_t idx;
    std::string name;
  };
  std::vector<Parameter> list;
  auto maybe_add_param = [&list](size_t idx, const char* name) {
    if (idx !=
        semantic::TextureIntrinsicCall::Parameters::Parameters::kNotUsed) {
      list.emplace_back(Parameter{idx, name});
    }
  };
  maybe_add_param(params.idx.array_index, "array_index");
  maybe_add_param(params.idx.bias, "bias");
  maybe_add_param(params.idx.coords, "coords");
  maybe_add_param(params.idx.depth_ref, "depth_ref");
  maybe_add_param(params.idx.ddx, "ddx");
  maybe_add_param(params.idx.ddy, "ddy");
  maybe_add_param(params.idx.level, "level");
  maybe_add_param(params.idx.offset, "offset");
  maybe_add_param(params.idx.sampler, "sampler");
  maybe_add_param(params.idx.sample_index, "sample_index");
  maybe_add_param(params.idx.texture, "texture");
  maybe_add_param(params.idx.value, "value");
  std::sort(
      list.begin(), list.end(),
      [](const Parameter& a, const Parameter& b) { return a.idx < b.idx; });

  std::stringstream out;
  out << function << "(";
  bool first = true;
  for (auto& param : list) {
    if (!first) {
      out << ", ";
    }
    out << param.name;
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
    case ValidTextureOverload::kDimensions1dArray:
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
    case ValidTextureOverload::kDimensionsStorageRO1dArray:
    case ValidTextureOverload::kDimensionsStorageRO2d:
    case ValidTextureOverload::kDimensionsStorageRO2dArray:
    case ValidTextureOverload::kDimensionsStorageRO3d:
    case ValidTextureOverload::kDimensionsStorageWO1d:
    case ValidTextureOverload::kDimensionsStorageWO1dArray:
    case ValidTextureOverload::kDimensionsStorageWO2d:
    case ValidTextureOverload::kDimensionsStorageWO2dArray:
    case ValidTextureOverload::kDimensionsStorageWO3d:
      return R"(textureDimensions(texture))";
    case ValidTextureOverload::kNumLayers1dArray:
    case ValidTextureOverload::kNumLayers2dArray:
    case ValidTextureOverload::kNumLayersCubeArray:
    case ValidTextureOverload::kNumLayersMultisampled2dArray:
    case ValidTextureOverload::kNumLayersDepth2dArray:
    case ValidTextureOverload::kNumLayersDepthCubeArray:
    case ValidTextureOverload::kNumLayersStorageWO1dArray:
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
    case ValidTextureOverload::kSample1dArrayF32:
      return R"(textureSample(texture, sampler, coords, array_index))";
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
    case ValidTextureOverload::kLoad1dF32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad1dU32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad1dI32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad1dArrayF32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad1dArrayU32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad1dArrayI32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad2dF32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad2dU32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad2dI32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelU32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dLevelI32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoad2dArrayF32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad2dArrayU32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad2dArrayI32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoad2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelU32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad2dArrayLevelI32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoad3dF32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad3dU32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoad3dI32:
      return R"(textureLoad(texture, coords))";
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
    case ValidTextureOverload::kLoadDepth2dF32:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadDepth2dLevelF32:
      return R"(textureLoad(texture, coords, level))";
    case ValidTextureOverload::kLoadDepth2dArrayF32:
      return R"(textureLoad(texture, coords, array_index))";
    case ValidTextureOverload::kLoadDepth2dArrayLevelF32:
      return R"(textureLoad(texture, coords, array_index, level))";
    case ValidTextureOverload::kLoadStorageRO1dRgba32float:
      return R"(textureLoad(texture, coords))";
    case ValidTextureOverload::kLoadStorageRO1dArrayRgba32float:
      return R"(textureLoad(texture, coords, array_index))";
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
    case ValidTextureOverload::kStoreWO1dArrayRgba32float:
      return R"(textureStore(texture, coords, array_index, value))";
    case ValidTextureOverload::kStoreWO2dRgba32float:
      return R"(textureStore(texture, coords, value))";
    case ValidTextureOverload::kStoreWO2dArrayRgba32float:
      return R"(textureStore(texture, coords, array_index, value))";
    case ValidTextureOverload::kStoreWO3dRgba32float:
      return R"(textureStore(texture, coords, value))";
  }
  return "<unmatched texture overload>";
}

TEST_P(TypeDeterminerTextureIntrinsicTest, Call) {
  auto param = GetParam();

  param.buildTextureVariable(this);
  param.buildSamplerVariable(this);

  auto* ident = Expr(param.function);
  auto* call = Call(ident, param.args(this));
  WrapInFunction(call);

  ASSERT_TRUE(td()->Determine()) << td()->error();

  if (std::string(param.function) == "textureDimensions") {
    switch (param.texture_dimension) {
      default:
        FAIL() << "invalid texture dimensions: " << param.texture_dimension;
      case type::TextureDimension::k1d:
      case type::TextureDimension::k1dArray:
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

  auto* sem = Sem().Get(call);
  ASSERT_NE(sem, nullptr);
  auto* intrinsic = sem->As<semantic::TextureIntrinsicCall>();
  ASSERT_NE(intrinsic, nullptr);

  auto got = ::tint::to_str(param.function, intrinsic->Params());
  auto* expected = expected_texture_overload(param.overload);
  EXPECT_EQ(got, expected);
}

}  // namespace
}  // namespace tint
