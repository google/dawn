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

#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/block_statement.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/cast_expression.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable_decl_statement.h"

namespace tint {
namespace {

class FakeStmt : public ast::Statement {
 public:
  bool IsValid() const override { return true; }
  void to_str(std::ostream& out, size_t) const override { out << "Fake"; }
};

class FakeExpr : public ast::Expression {
 public:
  bool IsValid() const override { return true; }
  void to_str(std::ostream&, size_t) const override {}
};

class TypeDeterminerHelper {
 public:
  TypeDeterminerHelper()
      : td_(std::make_unique<TypeDeterminer>(&ctx_, &mod_)) {}

  TypeDeterminer* td() const { return td_.get(); }
  ast::Module* mod() { return &mod_; }
  Context* ctx() { return &ctx_; }

 private:
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
};

class TypeDeterminerTest : public TypeDeterminerHelper, public testing::Test {};

template <typename T>
class TypeDeterminerTestWithParam : public TypeDeterminerHelper,
                                    public testing::TestWithParam<T> {};

TEST_F(TypeDeterminerTest, Error_WithEmptySource) {
  FakeStmt s;
  s.set_source(Source{0, 0});

  EXPECT_FALSE(td()->DetermineResultType(&s));
  EXPECT_EQ(td()->error(),
            "unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Error_Unknown) {
  FakeStmt s;
  s.set_source(Source{2, 30});

  EXPECT_FALSE(td()->DetermineResultType(&s));
  EXPECT_EQ(td()->error(),
            "2:30: unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Assign) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  ast::AssignmentStatement assign(std::move(lhs), std::move(rhs));

  EXPECT_TRUE(td()->DetermineResultType(&assign));
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Case) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                          std::move(rhs)));

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 3));
  ast::CaseStatement cse(std::move(lit), std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&cse));
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Block) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  ast::BlockStatement block;
  block.append(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                          std::move(rhs)));

  EXPECT_TRUE(td()->DetermineResultType(&block));
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Else) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                          std::move(rhs)));

  ast::ElseStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                              std::make_unique<ast::SintLiteral>(&i32, 3)),
                          std::move(body));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_If) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto else_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* else_lhs_ptr = else_lhs.get();

  auto else_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* else_rhs_ptr = else_rhs.get();

  auto else_body = std::make_unique<ast::BlockStatement>();
  else_body->append(std::make_unique<ast::AssignmentStatement>(
      std::move(else_lhs), std::move(else_rhs)));

  auto else_stmt = std::make_unique<ast::ElseStatement>(
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::SintLiteral>(&i32, 3)),
      std::move(else_body));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(std::move(else_stmt));

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                          std::move(rhs)));

  ast::IfStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                            std::make_unique<ast::SintLiteral>(&i32, 3)),
                        std::move(body));
  stmt.set_else_statements(std::move(else_stmts));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(else_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(else_rhs_ptr->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(else_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(else_rhs_ptr->result_type()->IsF32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Loop) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto body_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* body_lhs_ptr = body_lhs.get();

  auto body_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* body_rhs_ptr = body_rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(std::move(body_lhs),
                                                          std::move(body_rhs)));

  auto continuing_lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* continuing_lhs_ptr = continuing_lhs.get();

  auto continuing_rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* continuing_rhs_ptr = continuing_rhs.get();

  auto continuing = std::make_unique<ast::BlockStatement>();
  continuing->append(std::make_unique<ast::AssignmentStatement>(
      std::move(continuing_lhs), std::move(continuing_rhs)));

  ast::LoopStatement stmt(std::move(body), std::move(continuing));

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(body_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(body_rhs_ptr->result_type(), nullptr);
  ASSERT_NE(continuing_lhs_ptr->result_type(), nullptr);
  ASSERT_NE(continuing_rhs_ptr->result_type(), nullptr);
  EXPECT_TRUE(body_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(body_rhs_ptr->result_type()->IsF32());
  EXPECT_TRUE(continuing_lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(continuing_rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Return) {
  ast::type::I32Type i32;

  auto cond = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* cond_ptr = cond.get();

  ast::ReturnStatement ret(std::move(cond));

  EXPECT_TRUE(td()->DetermineResultType(&ret));
  ASSERT_NE(cond_ptr->result_type(), nullptr);
  EXPECT_TRUE(cond_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Stmt_Return_WithoutValue) {
  ast::type::I32Type i32;
  ast::ReturnStatement ret;
  EXPECT_TRUE(td()->DetermineResultType(&ret));
}

TEST_F(TypeDeterminerTest, Stmt_Switch) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  auto lhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto* lhs_ptr = lhs.get();

  auto rhs = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.3f));
  auto* rhs_ptr = rhs.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(std::move(lhs),
                                                          std::move(rhs)));

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 3));

  ast::CaseStatementList cases;
  cases.push_back(
      std::make_unique<ast::CaseStatement>(std::move(lit), std::move(body)));

  ast::SwitchStatement stmt(std::make_unique<ast::ScalarConstructorExpression>(
                                std::make_unique<ast::SintLiteral>(&i32, 2)),
                            std::move(cases));

  EXPECT_TRUE(td()->DetermineResultType(&stmt)) << td()->error();
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs_ptr->result_type(), nullptr);
  ASSERT_NE(rhs_ptr->result_type(), nullptr);

  EXPECT_TRUE(stmt.condition()->result_type()->IsI32());
  EXPECT_TRUE(lhs_ptr->result_type()->IsI32());
  EXPECT_TRUE(rhs_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Call) {
  ast::type::F32Type f32;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  auto expr = std::make_unique<ast::CallExpression>(
      std::make_unique<ast::IdentifierExpression>("my_func"),
      std::move(call_params));
  auto* expr_ptr = expr.get();

  ast::CallStatement call(std::move(expr));
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(expr_ptr->result_type(), nullptr);
  EXPECT_TRUE(expr_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Stmt_Call_undeclared) {
  // fn main() -> void {func(); return; }
  // fn func() -> void { return; }
  ast::type::F32Type f32;
  ast::ExpressionList call_params;
  auto call_expr = std::make_unique<ast::CallExpression>(
      Source{12, 34}, std::make_unique<ast::IdentifierExpression>("func"),
      std::move(call_params));
  ast::VariableList params0;
  auto func_main =
      std::make_unique<ast::Function>("main", std::move(params0), &f32);
  auto main_body = std::make_unique<ast::BlockStatement>();
  main_body->append(std::make_unique<ast::CallStatement>(std::move(call_expr)));
  main_body->append(std::make_unique<ast::ReturnStatement>());
  func_main->set_body(std::move(main_body));
  mod()->AddFunction(std::move(func_main));

  auto func = std::make_unique<ast::Function>("func", std::move(params0), &f32);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_FALSE(td()->Determine()) << td()->error();
  EXPECT_EQ(td()->error(),
            "12:34: v-0005: function must be declared before use: 'func'");
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl) {
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  auto* init_ptr = var->constructor();

  ast::VariableDeclStatement decl(std::move(var));

  EXPECT_TRUE(td()->DetermineResultType(&decl));
  ASSERT_NE(init_ptr->result_type(), nullptr);
  EXPECT_TRUE(init_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl_ModuleScope) {
  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  auto* init_ptr = var->constructor();

  mod()->AddGlobalVariable(std::move(var));

  EXPECT_TRUE(td()->Determine());
  ASSERT_NE(init_ptr->result_type(), nullptr);
  EXPECT_TRUE(init_ptr->result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Expr_Error_Unknown) {
  FakeExpr e;
  e.set_source(Source{2, 30});

  EXPECT_FALSE(td()->DetermineResultType(&e));
  EXPECT_EQ(td()->error(), "2:30: unknown expression for type determination");
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 3);

  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto var = std::make_unique<ast::Variable>(
      "my_var", ast::StorageClass::kFunction, &ary);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::IdentifierExpression>("my_var"), std::move(idx));
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->IsPointer());

  auto* ptr = acc.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Alias_Array) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 3);
  ast::type::AliasType aary("myarrty", &ary);

  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto var = std::make_unique<ast::Variable>(
      "my_var", ast::StorageClass::kFunction, &aary);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::IdentifierExpression>("my_var"), std::move(idx));
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->IsPointer());

  auto* ptr = acc.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array_Constant) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 3);

  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto var = std::make_unique<ast::Variable>(
      "my_var", ast::StorageClass::kFunction, &ary);
  var->set_is_const(true);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::IdentifierExpression>("my_var"), std::move(idx));
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  EXPECT_TRUE(acc.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);

  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &mat);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::IdentifierExpression>("my_var"), std::move(idx));
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->IsPointer());

  auto* ptr = acc.result_type()->AsPointer();
  ASSERT_TRUE(ptr->type()->IsVector());
  EXPECT_EQ(ptr->type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);

  auto idx1 = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto idx2 = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1));
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &mat);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::ArrayAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("my_var"),
          std::move(idx1)),
      std::move(idx2));

  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->IsPointer());

  auto* ptr = acc.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Vector) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &vec);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      std::make_unique<ast::IdentifierExpression>("my_var"), std::move(idx));
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->IsPointer());

  auto* ptr = acc.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_As) {
  ast::type::F32Type f32;
  ast::AsExpression as(&f32,
                       std::make_unique<ast::IdentifierExpression>("name"));

  EXPECT_TRUE(td()->DetermineResultType(&as));
  ASSERT_NE(as.result_type(), nullptr);
  EXPECT_TRUE(as.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Call) {
  ast::type::F32Type f32;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  ast::CallExpression call(
      std::make_unique<ast::IdentifierExpression>("my_func"),
      std::move(call_params));
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(call.result_type(), nullptr);
  EXPECT_TRUE(call.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Call_WithParams) {
  ast::type::F32Type f32;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  auto* param_ptr = call_params.back().get();

  ast::CallExpression call(
      std::make_unique<ast::IdentifierExpression>("my_func"),
      std::move(call_params));
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(param_ptr->result_type(), nullptr);
  EXPECT_TRUE(param_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Call_GLSLImport) {
  ast::type::F32Type f32;

  mod()->AddImport(std::make_unique<ast::Import>("GLSL.std.450", "std"));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.4)));

  std::vector<std::string> name{"std", "round"};
  ast::CallExpression call(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(call.result_type(), nullptr);
  EXPECT_TRUE(call.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Cast) {
  ast::type::F32Type f32;
  ast::CastExpression cast(&f32,
                           std::make_unique<ast::IdentifierExpression>("name"));

  EXPECT_TRUE(td()->DetermineResultType(&cast));
  ASSERT_NE(cast.result_type(), nullptr);
  EXPECT_TRUE(cast.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Scalar) {
  ast::type::F32Type f32;
  ast::ScalarConstructorExpression s(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(&s));
  ASSERT_NE(s.result_type(), nullptr);
  EXPECT_TRUE(s.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Type) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression tc(&vec, std::move(vals));

  EXPECT_TRUE(td()->DetermineResultType(&tc));
  ASSERT_NE(tc.result_type(), nullptr);
  ASSERT_TRUE(tc.result_type()->IsVector());
  EXPECT_TRUE(tc.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(tc.result_type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalVariable) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident("my_var");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->IsPointer());
  EXPECT_TRUE(ident.result_type()->AsPointer()->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalConstant) {
  ast::type::F32Type f32;
  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  var->set_is_const(true);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident("my_var");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable_Const) {
  ast::type::F32Type f32;

  auto my_var = std::make_unique<ast::IdentifierExpression>("my_var");
  auto* my_var_ptr = my_var.get();

  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  var->set_is_const(true);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::move(my_var),
      std::make_unique<ast::IdentifierExpression>("my_var")));

  ast::Function f("my_func", {}, &f32);
  f.set_body(std::move(body));

  EXPECT_TRUE(td()->DetermineFunction(&f));

  ASSERT_NE(my_var_ptr->result_type(), nullptr);
  EXPECT_TRUE(my_var_ptr->result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable) {
  ast::type::F32Type f32;

  auto my_var = std::make_unique<ast::IdentifierExpression>("my_var");
  auto* my_var_ptr = my_var.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone,
                                      &f32)));

  body->append(std::make_unique<ast::AssignmentStatement>(
      std::move(my_var),
      std::make_unique<ast::IdentifierExpression>("my_var")));

  ast::Function f("my_func", {}, &f32);
  f.set_body(std::move(body));

  EXPECT_TRUE(td()->DetermineFunction(&f));

  ASSERT_NE(my_var_ptr->result_type(), nullptr);
  EXPECT_TRUE(my_var_ptr->result_type()->IsPointer());
  EXPECT_TRUE(my_var_ptr->result_type()->AsPointer()->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Function) {
  ast::type::F32Type f32;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident("my_func");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables) {
  ast::type::F32Type f32;

  auto in_var = std::make_unique<ast::Variable>(
      "in_var", ast::StorageClass::kInput, &f32);
  auto out_var = std::make_unique<ast::Variable>(
      "out_var", ast::StorageClass::kOutput, &f32);
  auto sb_var = std::make_unique<ast::Variable>(
      "sb_var", ast::StorageClass::kStorageBuffer, &f32);
  auto wg_var = std::make_unique<ast::Variable>(
      "wg_var", ast::StorageClass::kWorkgroup, &f32);
  auto priv_var = std::make_unique<ast::Variable>(
      "priv_var", ast::StorageClass::kPrivate, &f32);

  auto* in_ptr = in_var.get();
  auto* out_ptr = out_var.get();
  auto* sb_ptr = sb_var.get();
  auto* wg_ptr = wg_var.get();
  auto* priv_ptr = priv_var.get();

  mod()->AddGlobalVariable(std::move(in_var));
  mod()->AddGlobalVariable(std::move(out_var));
  mod()->AddGlobalVariable(std::move(sb_var));
  mod()->AddGlobalVariable(std::move(wg_var));
  mod()->AddGlobalVariable(std::move(priv_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("out_var"),
      std::make_unique<ast::IdentifierExpression>("in_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("wg_var"),
      std::make_unique<ast::IdentifierExpression>("wg_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("sb_var"),
      std::make_unique<ast::IdentifierExpression>("sb_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("priv_var"),
      std::make_unique<ast::IdentifierExpression>("priv_var")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  const auto& vars = func_ptr->referenced_module_variables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0], out_ptr);
  EXPECT_EQ(vars[1], in_ptr);
  EXPECT_EQ(vars[2], wg_ptr);
  EXPECT_EQ(vars[3], sb_ptr);
  EXPECT_EQ(vars[4], priv_ptr);
}

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables_SubFunction) {
  ast::type::F32Type f32;

  auto in_var = std::make_unique<ast::Variable>(
      "in_var", ast::StorageClass::kInput, &f32);
  auto out_var = std::make_unique<ast::Variable>(
      "out_var", ast::StorageClass::kOutput, &f32);
  auto sb_var = std::make_unique<ast::Variable>(
      "sb_var", ast::StorageClass::kStorageBuffer, &f32);
  auto wg_var = std::make_unique<ast::Variable>(
      "wg_var", ast::StorageClass::kWorkgroup, &f32);
  auto priv_var = std::make_unique<ast::Variable>(
      "priv_var", ast::StorageClass::kPrivate, &f32);

  auto* in_ptr = in_var.get();
  auto* out_ptr = out_var.get();
  auto* sb_ptr = sb_var.get();
  auto* wg_ptr = wg_var.get();
  auto* priv_ptr = priv_var.get();

  mod()->AddGlobalVariable(std::move(in_var));
  mod()->AddGlobalVariable(std::move(out_var));
  mod()->AddGlobalVariable(std::move(sb_var));
  mod()->AddGlobalVariable(std::move(wg_var));
  mod()->AddGlobalVariable(std::move(priv_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("out_var"),
      std::make_unique<ast::IdentifierExpression>("in_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("wg_var"),
      std::make_unique<ast::IdentifierExpression>("wg_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("sb_var"),
      std::make_unique<ast::IdentifierExpression>("sb_var")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("priv_var"),
      std::make_unique<ast::IdentifierExpression>("priv_var")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto func2 = std::make_unique<ast::Function>("func", std::move(params), &f32);
  auto* func2_ptr = func2.get();

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("out_var"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("my_func"),
          ast::ExpressionList{})));
  func2->set_body(std::move(body));

  mod()->AddFunction(std::move(func2));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  const auto& vars = func2_ptr->referenced_module_variables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0], out_ptr);
  EXPECT_EQ(vars[1], in_ptr);
  EXPECT_EQ(vars[2], wg_ptr);
  EXPECT_EQ(vars[3], sb_ptr);
  EXPECT_EQ(vars[4], priv_ptr);
}

TEST_F(TypeDeterminerTest, Function_NotRegisterFunctionVariable) {
  ast::type::F32Type f32;

  auto var = std::make_unique<ast::Variable>(
      "in_var", ast::StorageClass::kFunction, &f32);

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &f32);
  auto* func_ptr = func.get();

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("var"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 1.f))));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  // Register the function
  EXPECT_TRUE(td()->Determine());

  EXPECT_EQ(func_ptr->referenced_module_variables().size(), 0u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>("first_member", &i32,
                                                        std::move(decos)));
  members.push_back(std::make_unique<ast::StructMember>("second_member", &f32,
                                                        std::move(decos)));

  auto strct = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                             std::move(members));

  ast::type::StructType st(std::move(strct));

  auto var = std::make_unique<ast::Variable>("my_struct",
                                             ast::StorageClass::kNone, &st);

  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto ident = std::make_unique<ast::IdentifierExpression>("my_struct");
  auto mem_ident = std::make_unique<ast::IdentifierExpression>("second_member");

  ast::MemberAccessorExpression mem(std::move(ident), std::move(mem_ident));
  EXPECT_TRUE(td()->DetermineResultType(&mem));
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->IsPointer());

  auto* ptr = mem.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct_Alias) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>("first_member", &i32,
                                                        std::move(decos)));
  members.push_back(std::make_unique<ast::StructMember>("second_member", &f32,
                                                        std::move(decos)));

  auto strct = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                             std::move(members));

  auto st = std::make_unique<ast::type::StructType>(std::move(strct));
  ast::type::AliasType alias("alias", st.get());

  auto var = std::make_unique<ast::Variable>("my_struct",
                                             ast::StorageClass::kNone, &alias);

  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto ident = std::make_unique<ast::IdentifierExpression>("my_struct");
  auto mem_ident = std::make_unique<ast::IdentifierExpression>("second_member");

  ast::MemberAccessorExpression mem(std::move(ident), std::move(mem_ident));
  EXPECT_TRUE(td()->DetermineResultType(&mem));
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->IsPointer());

  auto* ptr = mem.result_type()->AsPointer();
  EXPECT_TRUE(ptr->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var = std::make_unique<ast::Variable>("my_vec", ast::StorageClass::kNone,
                                             &vec3);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto ident = std::make_unique<ast::IdentifierExpression>("my_vec");
  auto swizzle = std::make_unique<ast::IdentifierExpression>("xy");

  ast::MemberAccessorExpression mem(std::move(ident), std::move(swizzle));
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->IsVector());
  EXPECT_TRUE(mem.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(mem.result_type()->AsVector()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var = std::make_unique<ast::Variable>("my_vec", ast::StorageClass::kNone,
                                             &vec3);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto ident = std::make_unique<ast::IdentifierExpression>("my_vec");
  auto swizzle = std::make_unique<ast::IdentifierExpression>("x");

  ast::MemberAccessorExpression mem(std::move(ident), std::move(swizzle));
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->IsPointer());

  auto* ptr = mem.result_type()->AsPointer();
  ASSERT_TRUE(ptr->type()->IsF32());
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
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::type::VectorType vec4(&f32, 4);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList b_members;
  b_members.push_back(
      std::make_unique<ast::StructMember>("foo", &vec4, std::move(decos)));

  auto strctB = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                              std::move(b_members));
  ast::type::StructType stB(std::move(strctB));
  stB.set_name("B");

  ast::type::VectorType vecB(&stB, 3);

  ast::StructMemberList a_members;
  a_members.push_back(
      std::make_unique<ast::StructMember>("mem", &vecB, std::move(decos)));

  auto strctA = std::make_unique<ast::Struct>(ast::StructDecoration::kNone,
                                              std::move(a_members));

  ast::type::StructType stA(std::move(strctA));
  stA.set_name("A");

  auto var =
      std::make_unique<ast::Variable>("c", ast::StorageClass::kNone, &stA);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto ident = std::make_unique<ast::IdentifierExpression>("c");
  auto mem_ident = std::make_unique<ast::IdentifierExpression>("mem");
  auto foo_ident = std::make_unique<ast::IdentifierExpression>("foo");
  auto idx = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 0));
  auto swizzle = std::make_unique<ast::IdentifierExpression>("yx");

  ast::MemberAccessorExpression mem(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::ArrayAccessorExpression>(
              std::make_unique<ast::MemberAccessorExpression>(
                  std::move(ident), std::move(mem_ident)),
              std::move(idx)),
          std::move(foo_ident)),
      std::move(swizzle));
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();

  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->IsVector());
  EXPECT_TRUE(mem.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(mem.result_type()->AsVector()->size(), 2u);
}

using Expr_Binary_BitwiseTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_BitwiseTest, Scalar) {
  auto op = GetParam();

  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &i32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsI32());
}

TEST_P(Expr_Binary_BitwiseTest, Vector) {
  auto op = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
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

  ast::type::BoolType bool_type;

  auto var = std::make_unique<ast::Variable>("val", ast::StorageClass::kNone,
                                             &bool_type);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsBool());
}

TEST_P(Expr_Binary_LogicalTest, Vector) {
  auto op = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&bool_type, 3);

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsBool());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Expr_Binary_LogicalTest,
                         testing::Values(ast::BinaryOp::kLogicalAnd,
                                         ast::BinaryOp::kLogicalOr));

using Expr_Binary_CompareTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_CompareTest, Scalar) {
  auto op = GetParam();

  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &i32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsBool());
}

TEST_P(Expr_Binary_CompareTest, Vector) {
  auto op = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec3(&i32, 3);

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsBool());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
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
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("val", ast::StorageClass::kNone, &i32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsI32());
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Scalar) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto scalar =
      std::make_unique<ast::Variable>("scalar", ast::StorageClass::kNone, &f32);
  auto vector = std::make_unique<ast::Variable>(
      "vector", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(scalar));
  mod()->AddGlobalVariable(std::move(vector));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("vector"),
      std::make_unique<ast::IdentifierExpression>("scalar"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto scalar =
      std::make_unique<ast::Variable>("scalar", ast::StorageClass::kNone, &f32);
  auto vector = std::make_unique<ast::Variable>(
      "vector", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(scalar));
  mod()->AddGlobalVariable(std::move(vector));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("scalar"),
      std::make_unique<ast::IdentifierExpression>("vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto vector = std::make_unique<ast::Variable>(
      "vector", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(vector));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("vector"),
      std::make_unique<ast::IdentifierExpression>("vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Scalar) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat3x2(&f32, 3, 2);

  auto scalar =
      std::make_unique<ast::Variable>("scalar", ast::StorageClass::kNone, &f32);
  auto matrix = std::make_unique<ast::Variable>(
      "matrix", ast::StorageClass::kNone, &mat3x2);
  mod()->AddGlobalVariable(std::move(scalar));
  mod()->AddGlobalVariable(std::move(matrix));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("matrix"),
      std::make_unique<ast::IdentifierExpression>("scalar"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsMatrix());

  auto* mat = expr.result_type()->AsMatrix();
  EXPECT_TRUE(mat->type()->IsF32());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat3x2(&f32, 3, 2);

  auto scalar =
      std::make_unique<ast::Variable>("scalar", ast::StorageClass::kNone, &f32);
  auto matrix = std::make_unique<ast::Variable>(
      "matrix", ast::StorageClass::kNone, &mat3x2);
  mod()->AddGlobalVariable(std::move(scalar));
  mod()->AddGlobalVariable(std::move(matrix));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("scalar"),
      std::make_unique<ast::IdentifierExpression>("matrix"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsMatrix());

  auto* mat = expr.result_type()->AsMatrix();
  EXPECT_TRUE(mat->type()->IsF32());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 2);
  ast::type::MatrixType mat3x2(&f32, 3, 2);

  auto vector = std::make_unique<ast::Variable>(
      "vector", ast::StorageClass::kNone, &vec3);
  auto matrix = std::make_unique<ast::Variable>(
      "matrix", ast::StorageClass::kNone, &mat3x2);
  mod()->AddGlobalVariable(std::move(vector));
  mod()->AddGlobalVariable(std::move(matrix));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("matrix"),
      std::make_unique<ast::IdentifierExpression>("vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Matrix) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::MatrixType mat3x2(&f32, 3, 2);

  auto vector = std::make_unique<ast::Variable>(
      "vector", ast::StorageClass::kNone, &vec3);
  auto matrix = std::make_unique<ast::Variable>(
      "matrix", ast::StorageClass::kNone, &mat3x2);
  mod()->AddGlobalVariable(std::move(vector));
  mod()->AddGlobalVariable(std::move(matrix));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("vector"),
      std::make_unique<ast::IdentifierExpression>("matrix"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat4x3(&f32, 4, 3);
  ast::type::MatrixType mat3x4(&f32, 3, 4);

  auto matrix1 = std::make_unique<ast::Variable>(
      "mat4x3", ast::StorageClass::kNone, &mat4x3);
  auto matrix2 = std::make_unique<ast::Variable>(
      "mat3x4", ast::StorageClass::kNone, &mat3x4);
  mod()->AddGlobalVariable(std::move(matrix1));
  mod()->AddGlobalVariable(std::move(matrix2));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      std::make_unique<ast::IdentifierExpression>("mat4x3"),
      std::make_unique<ast::IdentifierExpression>("mat3x4"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsMatrix());

  auto* mat = expr.result_type()->AsMatrix();
  EXPECT_TRUE(mat->type()->IsF32());
  EXPECT_EQ(mat->rows(), 4u);
  EXPECT_EQ(mat->columns(), 4u);
}

using IntrinsicDerivativeTest = TypeDeterminerTestWithParam<std::string>;
TEST_P(IntrinsicDerivativeTest, Scalar) {
  auto name = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("ident", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsF32());
}

TEST_P(IntrinsicDerivativeTest, Vector) {
  auto name = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto var =
      std::make_unique<ast::Variable>("ident", ast::StorageClass::kNone, &vec4);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("ident"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

TEST_P(IntrinsicDerivativeTest, MissingParam) {
  auto name = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}

TEST_P(IntrinsicDerivativeTest, ToomManyParams) {
  auto name = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto var1 = std::make_unique<ast::Variable>("ident1",
                                              ast::StorageClass::kNone, &vec4);
  auto var2 = std::make_unique<ast::Variable>("ident2",
                                              ast::StorageClass::kNone, &vec4);
  mod()->AddGlobalVariable(std::move(var1));
  mod()->AddGlobalVariable(std::move(var2));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("ident1"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("ident2"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         IntrinsicDerivativeTest,
                         testing::Values("dpdx",
                                         "dpdx_coarse",
                                         "dpdx_fine",
                                         "dpdy",
                                         "dpdy_coarse",
                                         "dpdy_fine",
                                         "fwidth",
                                         "fwidth_coarse",
                                         "fwidth_fine"));

using Intrinsic = TypeDeterminerTestWithParam<std::string>;
TEST_P(Intrinsic, Test) {
  auto name = GetParam();

  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&bool_type, 3);

  auto var = std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone,
                                             &vec3);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());

  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsBool());
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Intrinsic,
                         testing::Values("any", "all"));

using Intrinsic_FloatMethod = TypeDeterminerTestWithParam<std::string>;
TEST_P(Intrinsic_FloatMethod, Vector) {
  auto name = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var = std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone,
                                             &vec3);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsBool());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
}

TEST_P(Intrinsic_FloatMethod, Scalar) {
  auto name = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsBool());
}

TEST_P(Intrinsic_FloatMethod, MissingParam) {
  auto name = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}

TEST_P(Intrinsic_FloatMethod, TooManyParams) {
  auto name = GetParam();

  ast::type::F32Type f32;

  auto var =
      std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone, &f32);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>(name),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_FloatMethod,
    testing::Values("is_inf", "is_nan", "is_finite", "is_normal"));

enum class TextureType { kF32, kI32, kU32 };
inline std::ostream& operator<<(std::ostream& out, TextureType data) {
  if (data == TextureType::kF32) {
    out << "f32";
  } else if (data == TextureType::kI32) {
    out << "i32";
  } else {
    out << "u32";
  }
  return out;
}

struct TextureTestParams {
  ast::type::TextureDimension dim;
  TextureType type = TextureType::kF32;
  ast::type::ImageFormat format = ast::type::ImageFormat::kR16Float;
};
inline std::ostream& operator<<(std::ostream& out, TextureTestParams data) {
  out << data.dim << "_" << data.type;
  return out;
}

class Intrinsic_TextureOperation
    : public TypeDeterminerTestWithParam<TextureTestParams> {
 public:
  std::unique_ptr<ast::type::Type> get_coords_type(
      ast::type::TextureDimension dim,
      ast::type::Type* type) {
    if (dim == ast::type::TextureDimension::k1d) {
      if (type->IsI32()) {
        return std::make_unique<ast::type::I32Type>();
      } else if (type->IsU32()) {
        return std::make_unique<ast::type::U32Type>();
      } else {
        return std::make_unique<ast::type::F32Type>();
      }
    } else if (dim == ast::type::TextureDimension::k1dArray ||
               dim == ast::type::TextureDimension::k2d ||
               dim == ast::type::TextureDimension::k2dMs) {
      return std::make_unique<ast::type::VectorType>(type, 2);
    } else if (dim == ast::type::TextureDimension::kCubeArray) {
      return std::make_unique<ast::type::VectorType>(type, 4);
    } else {
      return std::make_unique<ast::type::VectorType>(type, 3);
    }
  }

  void add_call_param(std::string name,
                      ast::type::Type* type,
                      ast::ExpressionList* call_params) {
    auto var =
        std::make_unique<ast::Variable>(name, ast::StorageClass::kNone, type);
    mod()->AddGlobalVariable(std::move(var));
    call_params->push_back(std::make_unique<ast::IdentifierExpression>(name));
  }

  std::unique_ptr<ast::type::Type> subtype(TextureType type) {
    if (type == TextureType::kF32) {
      return std::make_unique<ast::type::F32Type>();
    }
    if (type == TextureType::kI32) {
      return std::make_unique<ast::type::I32Type>();
    }
    return std::make_unique<ast::type::U32Type>();
  }
};

using Intrinsic_StorageTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_StorageTextureOperation, TextureLoadRo) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  ast::type::I32Type i32;
  auto coords_type = get_coords_type(dim, &i32);

  ast::type::Type* texture_type =
      ctx()->type_mgr().Get(std::make_unique<ast::type::StorageTextureType>(
          dim, ast::type::StorageAccess::kRead, format));

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_load"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  if (type == TextureType::kF32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  } else if (type == TextureType::kI32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  } else {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsU32());
  }
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_StorageTextureOperation,
    testing::Values(
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kF32, ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kI32, ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kU32, ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kF32, ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kI32, ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kU32, ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kU32,
                          ast::type::ImageFormat::kR8Unorm}));

using Intrinsic_SampledTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_SampledTextureOperation, TextureLoadSampled) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::I32Type i32;
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto coords_type = get_coords_type(dim, &i32);
  auto texture_type =
      std::make_unique<ast::type::SampledTextureType>(dim, s.get());

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_load"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  if (type == TextureType::kF32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  } else if (type == TextureType::kI32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  } else {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsU32());
  }
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

TEST_P(Intrinsic_SampledTextureOperation, TextureSample) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  auto s = subtype(type);
  ast::type::F32Type f32;
  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  auto coords_type = get_coords_type(dim, &f32);
  auto texture_type =
      std::make_unique<ast::type::SampledTextureType>(dim, s.get());

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_sample"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  if (type == TextureType::kF32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  } else if (type == TextureType::kI32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  } else {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsU32());
  }
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

TEST_P(Intrinsic_SampledTextureOperation, TextureSampleLevel) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  auto s = subtype(type);
  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  auto coords_type = get_coords_type(dim, &f32);
  auto texture_type =
      std::make_unique<ast::type::SampledTextureType>(dim, s.get());

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_sample_level"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  if (type == TextureType::kF32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  } else if (type == TextureType::kI32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  } else {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsU32());
  }
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

TEST_P(Intrinsic_SampledTextureOperation, TextureSampleBias) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::F32Type f32;
  auto s = subtype(type);
  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kSampler);
  auto coords_type = get_coords_type(dim, &f32);
  auto texture_type =
      std::make_unique<ast::type::SampledTextureType>(dim, s.get());

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("sampler", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("bias", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_sample_bias"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsVector());
  if (type == TextureType::kF32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
  } else if (type == TextureType::kI32) {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsI32());
  } else {
    EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsU32());
  }
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_SampledTextureOperation,
    testing::Values(
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k1d, TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k1dArray,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2d, TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2dArray,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2dMs,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2dMs,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2dMs,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k2dMsArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k2dMsArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k2dMsArray,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::k3d, TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::kCube,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::kCube,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::kCube,
                          TextureType::kU32},
        TextureTestParams{ast::type::TextureDimension::kCubeArray,
                          TextureType::kF32},
        TextureTestParams{ast::type::TextureDimension::kCubeArray,
                          TextureType::kI32},
        TextureTestParams{ast::type::TextureDimension::kCubeArray,
                          TextureType::kU32}));

using Intrinsic_DepthTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_DepthTextureOperation, TextureSampleCompare) {
  auto dim = GetParam().dim;

  ast::type::F32Type f32;
  auto sampler_type = std::make_unique<ast::type::SamplerType>(
      ast::type::SamplerKind::kComparisonSampler);
  auto coords_type = get_coords_type(dim, &f32);
  auto texture_type = std::make_unique<ast::type::DepthTextureType>(dim);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("sampler_comparison", sampler_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("depth_reference", &f32, &call_params);

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("texture_sample_compare"),
      std::move(call_params));

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsF32());
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_DepthTextureOperation,
    testing::Values(TextureTestParams{ast::type::TextureDimension::k2d},
                    TextureTestParams{ast::type::TextureDimension::k2dArray},
                    TextureTestParams{ast::type::TextureDimension::kCube},
                    TextureTestParams{
                        ast::type::TextureDimension::kCubeArray}));

TEST_F(TypeDeterminerTest, Intrinsic_Dot) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var = std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone,
                                             &vec3);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));

  ast::CallExpression expr(std::make_unique<ast::IdentifierExpression>("dot"),
                           std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsF32());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select) {
  ast::type::F32Type f32;
  ast::type::BoolType bool_type;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType bool_vec3(&bool_type, 3);

  auto var = std::make_unique<ast::Variable>("my_var", ast::StorageClass::kNone,
                                             &vec3);
  auto bool_var = std::make_unique<ast::Variable>(
      "bool_var", ast::StorageClass::kNone, &bool_vec3);
  mod()->AddGlobalVariable(std::move(var));
  mod()->AddGlobalVariable(std::move(bool_var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("my_var"));
  call_params.push_back(
      std::make_unique<ast::IdentifierExpression>("bool_var"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->IsVector());
  EXPECT_EQ(expr.result_type()->AsVector()->size(), 3u);
  EXPECT_TRUE(expr.result_type()->AsVector()->type()->IsF32());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select_TooFewParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for select expected 3 got 1");
}

TEST_F(TypeDeterminerTest, Intrinsic_Select_TooManyParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kNone, &vec3);
  mod()->AddGlobalVariable(std::move(var));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("select"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for select expected 3 got 4");
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  auto var1 =
      std::make_unique<ast::Variable>("v3", ast::StorageClass::kNone, &vec3);
  auto var2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kNone, &vec2);
  mod()->AddGlobalVariable(std::move(var1));
  mod()->AddGlobalVariable(std::move(var2));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v3"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outer_product"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->IsMatrix());

  auto* mat = expr.result_type()->AsMatrix();
  EXPECT_TRUE(mat->type()->IsF32());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct_TooFewParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  auto var2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kNone, &vec2);
  mod()->AddGlobalVariable(std::move(var2));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outer_product"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for outer_product");
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct_TooManyParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec3(&f32, 3);
  ast::type::VectorType vec2(&f32, 2);

  auto var2 =
      std::make_unique<ast::Variable>("v2", ast::StorageClass::kNone, &vec2);
  mod()->AddGlobalVariable(std::move(var2));

  ast::ExpressionList call_params;
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));
  call_params.push_back(std::make_unique<ast::IdentifierExpression>("v2"));

  ast::CallExpression expr(
      std::make_unique<ast::IdentifierExpression>("outer_product"),
      std::move(call_params));

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for outer_product");
}

using UnaryOpExpressionTest = TypeDeterminerTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  ast::type::F32Type f32;

  ast::type::VectorType vec4(&f32, 4);

  auto var =
      std::make_unique<ast::Variable>("ident", ast::StorageClass::kNone, &vec4);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::UnaryOpExpression der(
      op, std::make_unique<ast::IdentifierExpression>("ident"));
  EXPECT_TRUE(td()->DetermineResultType(&der));
  ASSERT_NE(der.result_type(), nullptr);
  ASSERT_TRUE(der.result_type()->IsVector());
  EXPECT_TRUE(der.result_type()->AsVector()->type()->IsF32());
  EXPECT_EQ(der.result_type()->AsVector()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(TypeDeterminerTest, StorageClass_SetsIfMissing) {
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kNone, &i32);
  auto* var_ptr = var.get();
  auto stmt = std::make_unique<ast::VariableDeclStatement>(std::move(var));

  auto func =
      std::make_unique<ast::Function>("func", ast::VariableList{}, &i32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::move(stmt));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_EQ(var_ptr->storage_class(), ast::StorageClass::kFunction);
}

TEST_F(TypeDeterminerTest, StorageClass_DoesNotSetOnConst) {
  ast::type::I32Type i32;

  auto var =
      std::make_unique<ast::Variable>("var", ast::StorageClass::kNone, &i32);
  var->set_is_const(true);
  auto* var_ptr = var.get();
  auto stmt = std::make_unique<ast::VariableDeclStatement>(std::move(var));

  auto func =
      std::make_unique<ast::Function>("func", ast::VariableList{}, &i32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::move(stmt));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_EQ(var_ptr->storage_class(), ast::StorageClass::kNone);
}

TEST_F(TypeDeterminerTest, StorageClass_NonFunctionClassError) {
  ast::type::I32Type i32;

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kWorkgroup, &i32);
  auto stmt = std::make_unique<ast::VariableDeclStatement>(std::move(var));

  auto func =
      std::make_unique<ast::Function>("func", ast::VariableList{}, &i32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::move(stmt));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  EXPECT_FALSE(td()->Determine());
  EXPECT_EQ(td()->error(),
            "function variable has a non-function storage class");
}

struct GLSLData {
  const char* name;
  uint32_t value;
};
inline std::ostream& operator<<(std::ostream& out, GLSLData data) {
  out << data.name;
  return out;
}
using ImportData_SingleParamTest = TypeDeterminerTestWithParam<GLSLData>;

TEST_P(ImportData_SingleParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_SingleParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_SingleParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_SingleParamTest, Error_MultipleParams) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_SingleParamTest,
    testing::Values(GLSLData{"round", GLSLstd450Round},
                    GLSLData{"roundeven", GLSLstd450RoundEven},
                    GLSLData{"trunc", GLSLstd450Trunc},
                    GLSLData{"fabs", GLSLstd450FAbs},
                    GLSLData{"fsign", GLSLstd450FSign},
                    GLSLData{"floor", GLSLstd450Floor},
                    GLSLData{"ceil", GLSLstd450Ceil},
                    GLSLData{"fract", GLSLstd450Fract},
                    GLSLData{"radians", GLSLstd450Radians},
                    GLSLData{"degrees", GLSLstd450Degrees},
                    GLSLData{"sin", GLSLstd450Sin},
                    GLSLData{"cos", GLSLstd450Cos},
                    GLSLData{"tan", GLSLstd450Tan},
                    GLSLData{"asin", GLSLstd450Asin},
                    GLSLData{"acos", GLSLstd450Acos},
                    GLSLData{"atan", GLSLstd450Atan},
                    GLSLData{"sinh", GLSLstd450Sinh},
                    GLSLData{"cosh", GLSLstd450Cosh},
                    GLSLData{"tanh", GLSLstd450Tanh},
                    GLSLData{"asinh", GLSLstd450Asinh},
                    GLSLData{"acosh", GLSLstd450Acosh},
                    GLSLData{"atanh", GLSLstd450Atanh},
                    GLSLData{"exp", GLSLstd450Exp},
                    GLSLData{"log", GLSLstd450Log},
                    GLSLData{"exp2", GLSLstd450Exp2},
                    GLSLData{"log2", GLSLstd450Log2},
                    GLSLData{"sqrt", GLSLstd450Sqrt},
                    GLSLData{"inversesqrt", GLSLstd450InverseSqrt},
                    GLSLData{"normalize", GLSLstd450Normalize},
                    GLSLData{"interpolateatcentroid",
                             GLSLstd450InterpolateAtCentroid}));

TEST_F(TypeDeterminerTest, ImportData_Length_Scalar) {
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "length", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, GLSLstd450Length);
}

TEST_F(TypeDeterminerTest, ImportData_Length_FloatVector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "length", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, GLSLstd450Length);
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_Integer) {
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "length", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect type for length. Requires float scalar or float vector "
            "values");
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_NoParams) {
  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "length", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for length. Expected 1 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_MultipleParams) {
  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "length", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for length. Expected 1 got 3");
}

using ImportData_TwoParamTest = TypeDeterminerTestWithParam<GLSLData>;

TEST_P(ImportData_TwoParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_TwoParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_TwoParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 0");
}

TEST_P(ImportData_TwoParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 1");
}

TEST_P(ImportData_TwoParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_TwoParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_TwoParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 3");
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_TwoParamTest,
                         testing::Values(GLSLData{"atan2", GLSLstd450Atan2},
                                         GLSLData{"pow", GLSLstd450Pow},
                                         GLSLData{"fmin", GLSLstd450FMin},
                                         GLSLData{"fmax", GLSLstd450FMax},
                                         GLSLData{"step", GLSLstd450Step},
                                         GLSLData{"reflect", GLSLstd450Reflect},
                                         GLSLData{"nmin", GLSLstd450NMin},
                                         GLSLData{"nmax", GLSLstd450NMax}));

TEST_F(TypeDeterminerTest, ImportData_Distance_Scalar) {
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, GLSLstd450Distance);
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsF32());
  EXPECT_EQ(id, GLSLstd450Distance);
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_Integer) {
  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect type for distance. Requires float scalar or float "
            "vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_NoParams) {
  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_OneParam) {
  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 1");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_MismatchedParamCount) {
  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), "mismatched parameter types for distance");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_MismatchedParamType) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), "mismatched parameter types for distance");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_TooManyParams) {
  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "distance", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 3");
}

TEST_F(TypeDeterminerTest, ImportData_Cross) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, GLSLstd450Cross);
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_Scalar) {
  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect type for cross. Requires float vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_IntType) {
  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect type for cross. Requires float vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_MissingParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList params;
  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_TooFewParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 1");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_TooManyParams) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "cross", params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 3");
}

using ImportData_ThreeParamTest = TypeDeterminerTestWithParam<GLSLData>;
TEST_P(ImportData_ThreeParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_scalar());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_ThreeParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_float_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_ThreeParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_ThreeParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 0");
}

TEST_P(ImportData_ThreeParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 1");
}

TEST_P(ImportData_ThreeParamTest, Error_TwoParams) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 2");
}

TEST_P(ImportData_ThreeParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec2(&f32, 2);
  ast::type::VectorType vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 4");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_ThreeParamTest,
    testing::Values(GLSLData{"fclamp", GLSLstd450FClamp},
                    GLSLData{"fmix", GLSLstd450FMix},
                    GLSLData{"smoothstep", GLSLstd450SmoothStep},
                    GLSLData{"fma", GLSLstd450Fma},
                    GLSLData{"faceforward", GLSLstd450FaceForward},
                    GLSLData{"nclamp", GLSLstd450NClamp}));

using ImportData_Int_SingleParamTest = TypeDeterminerTestWithParam<GLSLData>;
TEST_P(ImportData_Int_SingleParamTest, Scalar) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_integer_scalar());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_SingleParamTest, Vector) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_signed_integer_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_SingleParamTest, Error_Float) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires integer scalar or integer vector values");
}

TEST_P(ImportData_Int_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_Int_SingleParamTest, Error_MultipleParams) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_Int_SingleParamTest,
    testing::Values(GLSLData{"sabs", GLSLstd450SAbs},
                    GLSLData{"ssign", GLSLstd450SSign},
                    GLSLData{"findilsb", GLSLstd450FindILsb},
                    GLSLData{"findumsb", GLSLstd450FindUMsb},
                    GLSLData{"findsmsb", GLSLstd450FindSMsb}));

using ImportData_Int_TwoParamTest = TypeDeterminerTestWithParam<GLSLData>;
TEST_P(ImportData_Int_TwoParamTest, Scalar_Signed) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsI32());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_TwoParamTest, Scalar_Unsigned) {
  auto param = GetParam();

  ast::type::U32Type u32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsU32());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_TwoParamTest, Vector_Signed) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_signed_integer_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_TwoParamTest, Vector_Unsigned) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_unsigned_integer_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_Int_TwoParamTest, Error_Float) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires integer scalar or integer vector values");
}

TEST_P(ImportData_Int_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 0");
}

TEST_P(ImportData_Int_TwoParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 1");
}

TEST_P(ImportData_Int_TwoParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec2(&i32, 2);
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_2)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_Int_TwoParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_Int_TwoParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 3");
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_Int_TwoParamTest,
                         testing::Values(GLSLData{"umin", GLSLstd450UMin},
                                         GLSLData{"smin", GLSLstd450SMin},
                                         GLSLData{"umax", GLSLstd450UMax},
                                         GLSLData{"smax", GLSLstd450SMax}));

TEST_F(TypeDeterminerTest, ImportData_GLSL_Determinant) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "determinant", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsF32());
  EXPECT_EQ(id, GLSLstd450Determinant);
}

TEST_F(TypeDeterminerTest, ImportData_GLSL_MatrixInverse) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", "matrixinverse", params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsMatrix());
  EXPECT_TRUE(type->AsMatrix()->type()->IsF32());
  EXPECT_EQ(type->AsMatrix()->rows(), 3u);
  EXPECT_EQ(type->AsMatrix()->columns(), 3u);
  EXPECT_EQ(id, GLSLstd450MatrixInverse);
}

using ImportData_Matrix_OneParam_Test = TypeDeterminerTestWithParam<GLSLData>;
TEST_P(ImportData_Matrix_OneParam_Test, Error_Float) {
  auto param = GetParam();

  ast::type::F32Type f32;

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &f32);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect type for ") + param.name +
                               ". Requires matrix value");
}

TEST_P(ImportData_Matrix_OneParam_Test, NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_Matrix_OneParam_Test, TooManyParams) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 3);

  auto var = std::make_unique<ast::Variable>(
      "var", ast::StorageClass::kFunction, &mat);
  mod()->AddGlobalVariable(std::move(var));

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));
  params.push_back(std::make_unique<ast::IdentifierExpression>("var"));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 2");
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_Matrix_OneParam_Test,
    testing::Values(GLSLData{"determinant", GLSLstd450Determinant},
                    GLSLData{"matrixinverse", GLSLstd450MatrixInverse}));

using ImportData_IntThreeParamTest = TypeDeterminerTestWithParam<GLSLData>;
TEST_P(ImportData_IntThreeParamTest, Scalar_Signed) {
  auto param = GetParam();

  ast::type::I32Type i32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsI32());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_IntThreeParamTest, Scalar_Unsigned) {
  auto param = GetParam();

  ast::type::U32Type u32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->IsU32());
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_IntThreeParamTest, Vector_Signed) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_signed_integer_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_IntThreeParamTest, Vector_Unsigned) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::VectorType vec(&u32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 3u)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 3u)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 1u)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::UintLiteral>(&u32, 3u)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_NE(type, nullptr);
  EXPECT_TRUE(type->is_unsigned_integer_vector());
  EXPECT_EQ(type->AsVector()->size(), 3u);
  EXPECT_EQ(id, param.value);
}

TEST_P(ImportData_IntThreeParamTest, Error_Float) {
  auto param = GetParam();

  ast::type::F32Type f32;

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 2.f)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 3.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires integer scalar or integer vector values");
}

TEST_P(ImportData_IntThreeParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;
  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 0");
}

TEST_P(ImportData_IntThreeParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 1");
}

TEST_P(ImportData_IntThreeParamTest, Error_TwoParams) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 2");
}

TEST_P(ImportData_IntThreeParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec2(&i32, 2);
  ast::type::VectorType vec3(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_3;
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec2, std::move(vals_1)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_2)));
  params.push_back(std::make_unique<ast::TypeConstructorExpression>(
      &vec3, std::move(vals_3)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_IntThreeParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::VectorType vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  vals.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(
      std::make_unique<ast::TypeConstructorExpression>(&vec, std::move(vals)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_IntThreeParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::ExpressionList params;
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));
  params.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 1)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  uint32_t id = 0;
  auto* type =
      td()->GetImportData({0, 0}, "GLSL.std.450", param.name, params, &id);
  ASSERT_EQ(type, nullptr);
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 4");
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_IntThreeParamTest,
                         testing::Values(GLSLData{"sclamp", GLSLstd450SClamp},
                                         GLSLData{"uclamp", GLSLstd450UClamp}));

TEST_F(TypeDeterminerTest, Function_EntryPoints) {
  ast::type::F32Type f32;

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
  auto func_b = std::make_unique<ast::Function>("b", std::move(params), &f32);
  auto* func_b_ptr = func_b.get();

  auto body = std::make_unique<ast::BlockStatement>();
  func_b->set_body(std::move(body));

  auto func_c = std::make_unique<ast::Function>("c", std::move(params), &f32);
  auto* func_c_ptr = func_c.get();

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("second"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("b"),
          ast::ExpressionList{})));
  func_c->set_body(std::move(body));

  auto func_a = std::make_unique<ast::Function>("a", std::move(params), &f32);
  auto* func_a_ptr = func_a.get();

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("first"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("c"),
          ast::ExpressionList{})));
  func_a->set_body(std::move(body));

  auto ep_1_func =
      std::make_unique<ast::Function>("ep_1_func", std::move(params), &f32);
  auto* ep_1_func_ptr = ep_1_func.get();

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("call_a"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("a"),
          ast::ExpressionList{})));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("call_b"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("b"),
          ast::ExpressionList{})));
  ep_1_func->set_body(std::move(body));

  auto ep_2_func =
      std::make_unique<ast::Function>("ep_2_func", std::move(params), &f32);
  auto* ep_2_func_ptr = ep_2_func.get();

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("call_c"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("c"),
          ast::ExpressionList{})));
  ep_2_func->set_body(std::move(body));

  auto ep_1 = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kVertex,
                                                "ep_1", "ep_1_func");
  auto ep_2 = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kVertex,
                                                "ep_2", "ep_2_func");

  mod()->AddFunction(std::move(func_b));
  mod()->AddFunction(std::move(func_c));
  mod()->AddFunction(std::move(func_a));
  mod()->AddFunction(std::move(ep_1_func));
  mod()->AddFunction(std::move(ep_2_func));

  mod()->AddEntryPoint(std::move(ep_1));
  mod()->AddEntryPoint(std::move(ep_2));

  // Register the functions and calculate the callers
  ASSERT_TRUE(td()->Determine()) << td()->error();

  const auto& b_eps = func_b_ptr->ancestor_entry_points();
  ASSERT_EQ(2u, b_eps.size());
  EXPECT_EQ("ep_1", b_eps[0]);
  EXPECT_EQ("ep_2", b_eps[1]);

  const auto& a_eps = func_a_ptr->ancestor_entry_points();
  ASSERT_EQ(1u, a_eps.size());
  EXPECT_EQ("ep_1", a_eps[0]);

  const auto& c_eps = func_c_ptr->ancestor_entry_points();
  ASSERT_EQ(2u, c_eps.size());
  EXPECT_EQ("ep_1", c_eps[0]);
  EXPECT_EQ("ep_2", c_eps[1]);

  EXPECT_TRUE(ep_1_func_ptr->ancestor_entry_points().empty());
  EXPECT_TRUE(ep_2_func_ptr->ancestor_entry_points().empty());
}

}  // namespace
}  // namespace tint
