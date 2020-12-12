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
#include "src/ast/builder.h"
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
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
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
  explicit FakeStmt(Source source) : ast::Statement(source) {}
  FakeStmt* Clone(ast::CloneContext*) const override { return nullptr; }
  bool IsValid() const override { return true; }
  void to_str(std::ostream& out, size_t) const override { out << "Fake"; }
};

class FakeExpr : public ast::Expression {
 public:
  explicit FakeExpr(Source source) : ast::Expression(source) {}
  FakeExpr* Clone(ast::CloneContext*) const override { return nullptr; }
  bool IsValid() const override { return true; }
  void to_str(std::ostream&, size_t) const override {}
};

class TypeDeterminerHelper : public ast::BuilderWithModule {
 public:
  TypeDeterminerHelper() : td_(std::make_unique<TypeDeterminer>(mod)) {}

  TypeDeterminer* td() const { return td_.get(); }

 private:
  void OnVariableBuilt(ast::Variable* var) override {
    td_->RegisterVariableForTesting(var);
  }

  std::unique_ptr<TypeDeterminer> td_;
};

class TypeDeterminerTest : public TypeDeterminerHelper, public testing::Test {};

template <typename T>
class TypeDeterminerTestWithParam : public TypeDeterminerHelper,
                                    public testing::TestWithParam<T> {};

TEST_F(TypeDeterminerTest, Error_WithEmptySource) {
  FakeStmt s(Source{});

  EXPECT_FALSE(td()->DetermineResultType(&s));
  EXPECT_EQ(td()->error(),
            "unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Error_Unknown) {
  FakeStmt s(Source{Source::Location{2, 30}});

  EXPECT_FALSE(td()->DetermineResultType(&s));
  EXPECT_EQ(td()->error(),
            "2:30: unknown statement type for type determination: Fake");
}

TEST_F(TypeDeterminerTest, Stmt_Assign) {
  ast::type::F32 f32;
  ast::type::I32 i32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  ast::AssignmentStatement assign(lhs, rhs);

  EXPECT_TRUE(td()->DetermineResultType(&assign));
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Case) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(lhs, rhs));

  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(&i32, 3));
  ast::CaseStatement cse(lit, body);

  EXPECT_TRUE(td()->DetermineResultType(&cse));
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Block) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  ast::BlockStatement block;
  block.append(create<ast::AssignmentStatement>(lhs, rhs));

  EXPECT_TRUE(td()->DetermineResultType(&block));
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Else) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(lhs, rhs));

  ast::ElseStatement stmt(create<ast::ScalarConstructorExpression>(
                              create<ast::SintLiteral>(&i32, 3)),
                          body);

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_If) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* else_lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* else_rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* else_body = create<ast::BlockStatement>();
  else_body->append(create<ast::AssignmentStatement>(else_lhs, else_rhs));

  auto* else_stmt =
      create<ast::ElseStatement>(create<ast::ScalarConstructorExpression>(
                                     create<ast::SintLiteral>(&i32, 3)),
                                 else_body);

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(lhs, rhs));

  ast::IfStatement stmt(Source{},
                        create<ast::ScalarConstructorExpression>(
                            create<ast::SintLiteral>(&i32, 3)),
                        body, ast::ElseStatementList{else_stmt});

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(else_lhs->result_type(), nullptr);
  ASSERT_NE(else_rhs->result_type(), nullptr);
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);
  EXPECT_TRUE(stmt.condition()->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(else_lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(else_rhs->result_type()->Is<ast::type::F32>());
  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Loop) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* body_lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* body_rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(body_lhs, body_rhs));

  auto* continuing_lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* continuing_rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* continuing = create<ast::BlockStatement>();
  continuing->append(
      create<ast::AssignmentStatement>(continuing_lhs, continuing_rhs));

  ast::LoopStatement stmt(body, continuing);

  EXPECT_TRUE(td()->DetermineResultType(&stmt));
  ASSERT_NE(body_lhs->result_type(), nullptr);
  ASSERT_NE(body_rhs->result_type(), nullptr);
  ASSERT_NE(continuing_lhs->result_type(), nullptr);
  ASSERT_NE(continuing_rhs->result_type(), nullptr);
  EXPECT_TRUE(body_lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(body_rhs->result_type()->Is<ast::type::F32>());
  EXPECT_TRUE(continuing_lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(continuing_rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Return) {
  ast::type::I32 i32;

  auto* cond = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));

  ast::ReturnStatement ret(Source{}, cond);

  EXPECT_TRUE(td()->DetermineResultType(&ret));
  ASSERT_NE(cond->result_type(), nullptr);
  EXPECT_TRUE(cond->result_type()->Is<ast::type::I32>());
}

TEST_F(TypeDeterminerTest, Stmt_Return_WithoutValue) {
  ast::type::I32 i32;
  ast::ReturnStatement ret(Source{});
  EXPECT_TRUE(td()->DetermineResultType(&ret));
}

TEST_F(TypeDeterminerTest, Stmt_Switch) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  auto* lhs = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* rhs = create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.3f));

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(lhs, rhs));

  ast::CaseSelectorList lit;
  lit.push_back(create<ast::SintLiteral>(&i32, 3));

  ast::CaseStatementList cases;
  cases.push_back(create<ast::CaseStatement>(lit, body));

  ast::SwitchStatement stmt(create<ast::ScalarConstructorExpression>(
                                create<ast::SintLiteral>(&i32, 2)),
                            cases);

  EXPECT_TRUE(td()->DetermineResultType(&stmt)) << td()->error();
  ASSERT_NE(stmt.condition()->result_type(), nullptr);
  ASSERT_NE(lhs->result_type(), nullptr);
  ASSERT_NE(rhs->result_type(), nullptr);

  EXPECT_TRUE(stmt.condition()->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(lhs->result_type()->Is<ast::type::I32>());
  EXPECT_TRUE(rhs->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Call) {
  ast::type::F32 f32;

  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{}, mod->RegisterSymbol("my_func"), "my_func", params, &f32,
      create<ast::BlockStatement>(), ast::FunctionDecorationList{});
  mod->AddFunction(func);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  auto* expr = create<ast::CallExpression>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("my_func"),
                                        "my_func"),
      call_params);

  ast::CallStatement call(expr);
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(expr->result_type(), nullptr);
  EXPECT_TRUE(expr->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Stmt_Call_undeclared) {
  // fn main() -> void {func(); return; }
  // fn func() -> void { return; }
  ast::type::F32 f32;
  ast::ExpressionList call_params;
  auto* call_expr = create<ast::CallExpression>(
      create<ast::IdentifierExpression>(Source{Source::Location{12, 34}},
                                        mod->RegisterSymbol("func"), "func"),
      call_params);
  ast::VariableList params0;
  auto* main_body = create<ast::BlockStatement>();
  main_body->append(create<ast::CallStatement>(call_expr));
  main_body->append(create<ast::ReturnStatement>(Source{}));
  auto* func_main = create<ast::Function>(Source{}, mod->RegisterSymbol("main"),
                                          "main", params0, &f32, main_body,
                                          ast::FunctionDecorationList{});
  mod->AddFunction(func_main);

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>(Source{}));
  auto* func =
      create<ast::Function>(Source{}, mod->RegisterSymbol("func"), "func",
                            params0, &f32, body, ast::FunctionDecorationList{});
  mod->AddFunction(func);

  EXPECT_FALSE(td()->Determine()) << td()->error();
  EXPECT_EQ(td()->error(),
            "12:34: v-0006: identifier must be declared before use: func");
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl) {
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "my_var",                  // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2)),  // constructor
      ast::VariableDecorationList{});          // decorations
  auto* init = var->constructor();

  ast::VariableDeclStatement decl(var);

  EXPECT_TRUE(td()->DetermineResultType(&decl));
  ASSERT_NE(init->result_type(), nullptr);
  EXPECT_TRUE(init->result_type()->Is<ast::type::I32>());
}

TEST_F(TypeDeterminerTest, Stmt_VariableDecl_ModuleScope) {
  ast::type::I32 i32;
  auto* var = create<ast::Variable>(
      Source{},                  // source
      "my_var",                  // name
      ast::StorageClass::kNone,  // storage_class
      &i32,                      // type
      false,                     // is_const
      create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2)),  // constructor
      ast::VariableDecorationList{});          // decorations
  auto* init = var->constructor();

  mod->AddGlobalVariable(var);

  EXPECT_TRUE(td()->Determine());
  ASSERT_NE(init->result_type(), nullptr);
  EXPECT_TRUE(init->result_type()->Is<ast::type::I32>());
}

TEST_F(TypeDeterminerTest, Expr_Error_Unknown) {
  FakeExpr e(Source{Source::Location{2, 30}});

  EXPECT_FALSE(td()->DetermineResultType(&e));
  EXPECT_EQ(td()->error(), "2:30: unknown expression for type determination");
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 3, ast::ArrayDecorationList{});

  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kFunction,    // storage_class
                            &ary,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(create<ast::IdentifierExpression>(
                                       mod->RegisterSymbol("my_var"), "my_var"),
                                   idx);
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->Is<ast::type::Pointer>());

  auto* ptr = acc.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Alias_Array) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 3, ast::ArrayDecorationList{});
  ast::type::Alias aary(mod->RegisterSymbol("myarrty"), "myarrty", &ary);

  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kFunction,    // storage_class
                            &aary,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(create<ast::IdentifierExpression>(
                                       mod->RegisterSymbol("my_var"), "my_var"),
                                   idx);
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->Is<ast::type::Pointer>());

  auto* ptr = acc.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Array_Constant) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 3, ast::ArrayDecorationList{});

  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kFunction,    // storage_class
                            &ary,                            // type
                            true,                            // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(create<ast::IdentifierExpression>(
                                       mod->RegisterSymbol("my_var"), "my_var"),
                                   idx);
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  EXPECT_TRUE(acc.result_type()->Is<ast::type::F32>())
      << acc.result_type()->type_name();
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 2);

  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(create<ast::IdentifierExpression>(
                                       mod->RegisterSymbol("my_var"), "my_var"),
                                   idx);
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->Is<ast::type::Pointer>());

  auto* ptr = acc.result_type()->As<ast::type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<ast::type::Vector>());
  EXPECT_EQ(ptr->type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Matrix_BothDimensions) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 2);

  auto* idx1 = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* idx2 = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(
      create<ast::ArrayAccessorExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("my_var"),
                                            "my_var"),
          idx1),
      idx2);

  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->Is<ast::type::Pointer>());

  auto* ptr = acc.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_ArrayAccessor_Vector) {
  ast::type::I32 i32;
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2));
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ArrayAccessorExpression acc(create<ast::IdentifierExpression>(
                                       mod->RegisterSymbol("my_var"), "my_var"),
                                   idx);
  EXPECT_TRUE(td()->DetermineResultType(&acc));
  ASSERT_NE(acc.result_type(), nullptr);
  ASSERT_TRUE(acc.result_type()->Is<ast::type::Pointer>());

  auto* ptr = acc.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Bitcast) {
  ast::type::F32 f32;
  ast::BitcastExpression bitcast(
      &f32,
      create<ast::IdentifierExpression>(mod->RegisterSymbol("name"), "name"));

  ast::Variable v(Source{}, "name", ast::StorageClass::kPrivate, &f32, false,
                  nullptr, ast::VariableDecorationList{});
  td()->RegisterVariableForTesting(&v);

  EXPECT_TRUE(td()->DetermineResultType(&bitcast));
  ASSERT_NE(bitcast.result_type(), nullptr);
  EXPECT_TRUE(bitcast.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call) {
  ast::type::F32 f32;

  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{}, mod->RegisterSymbol("my_func"), "my_func", params, &f32,
      create<ast::BlockStatement>(), ast::FunctionDecorationList{});
  mod->AddFunction(func);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  ast::CallExpression call(create<ast::IdentifierExpression>(
                               mod->RegisterSymbol("my_func"), "my_func"),
                           call_params);
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(call.result_type(), nullptr);
  EXPECT_TRUE(call.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call_WithParams) {
  ast::type::F32 f32;

  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{}, mod->RegisterSymbol("my_func"), "my_func", params, &f32,
      create<ast::BlockStatement>(), ast::FunctionDecorationList{});
  mod->AddFunction(func);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.4)));

  auto* param = call_params.back();

  ast::CallExpression call(create<ast::IdentifierExpression>(
                               mod->RegisterSymbol("my_func"), "my_func"),
                           call_params);
  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(param->result_type(), nullptr);
  EXPECT_TRUE(param->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Call_Intrinsic) {
  ast::type::F32 f32;

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 2.4)));

  ast::CallExpression call(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("round"), "round"),
      call_params);

  EXPECT_TRUE(td()->DetermineResultType(&call));
  ASSERT_NE(call.result_type(), nullptr);
  EXPECT_TRUE(call.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Cast) {
  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("name"), "name"));
  ast::TypeConstructorExpression cast(&f32, params);

  ast::Variable v(Source{}, "name", ast::StorageClass::kPrivate, &f32, false,
                  nullptr, ast::VariableDecorationList{});
  td()->RegisterVariableForTesting(&v);

  EXPECT_TRUE(td()->DetermineResultType(&cast));
  ASSERT_NE(cast.result_type(), nullptr);
  EXPECT_TRUE(cast.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Scalar) {
  ast::type::F32 f32;
  ast::ScalarConstructorExpression s(create<ast::FloatLiteral>(&f32, 1.0f));

  EXPECT_TRUE(td()->DetermineResultType(&s));
  ASSERT_NE(s.result_type(), nullptr);
  EXPECT_TRUE(s.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Constructor_Type) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::TypeConstructorExpression tc(&vec, vals);

  EXPECT_TRUE(td()->DetermineResultType(&tc));
  ASSERT_NE(tc.result_type(), nullptr);
  ASSERT_TRUE(tc.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(
      tc.result_type()->As<ast::type::Vector>()->type()->Is<ast::type::F32>());
  EXPECT_EQ(tc.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalVariable) {
  ast::type::F32 f32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident(mod->RegisterSymbol("my_var"), "my_var");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->Is<ast::type::Pointer>());
  EXPECT_TRUE(ident.result_type()
                  ->As<ast::type::Pointer>()
                  ->type()
                  ->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_GlobalConstant) {
  ast::type::F32 f32;
  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "my_var",                         // name
                            ast::StorageClass::kNone,         // storage_class
                            &f32,                             // type
                            true,                             // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident(mod->RegisterSymbol("my_var"), "my_var");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable_Const) {
  ast::type::F32 f32;

  auto* my_var = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var");

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            true,                            // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::AssignmentStatement>(
      my_var, create<ast::IdentifierExpression>(mod->RegisterSymbol("my_var"),
                                                "my_var")));

  ast::Function f(Source{}, mod->RegisterSymbol("my_func"), "my_func", {}, &f32,
                  body, ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->DetermineFunction(&f));

  ASSERT_NE(my_var->result_type(), nullptr);
  EXPECT_TRUE(my_var->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_FunctionVariable) {
  ast::type::F32 f32;

  auto* my_var = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var");

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(
      create<ast::Variable>(Source{},                          // source
                            "my_var",                          // name
                            ast::StorageClass::kNone,          // storage_class
                            &f32,                              // type
                            false,                             // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{})));  // decorations

  body->append(create<ast::AssignmentStatement>(
      my_var, create<ast::IdentifierExpression>(mod->RegisterSymbol("my_var"),
                                                "my_var")));

  ast::Function f(Source{}, mod->RegisterSymbol("myfunc"), "my_func", {}, &f32,
                  body, ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->DetermineFunction(&f));

  ASSERT_NE(my_var->result_type(), nullptr);
  EXPECT_TRUE(my_var->result_type()->Is<ast::type::Pointer>());
  EXPECT_TRUE(my_var->result_type()
                  ->As<ast::type::Pointer>()
                  ->type()
                  ->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Function_Ptr) {
  ast::type::F32 f32;
  ast::type::Pointer ptr(&f32, ast::StorageClass::kFunction);

  auto* my_var = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var");

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(
      create<ast::Variable>(Source{},                          // source
                            "my_var",                          // name
                            ast::StorageClass::kNone,          // storage_class
                            &ptr,                              // type
                            false,                             // is_const
                            nullptr,                           // constructor
                            ast::VariableDecorationList{})));  // decorations

  body->append(create<ast::AssignmentStatement>(
      my_var, create<ast::IdentifierExpression>(mod->RegisterSymbol("my_var"),
                                                "my_var")));

  ast::Function f(Source{}, mod->RegisterSymbol("my_func"), "my_func", {}, &f32,
                  body, ast::FunctionDecorationList{});

  EXPECT_TRUE(td()->DetermineFunction(&f));

  ASSERT_NE(my_var->result_type(), nullptr);
  EXPECT_TRUE(my_var->result_type()->Is<ast::type::Pointer>());
  EXPECT_TRUE(my_var->result_type()
                  ->As<ast::type::Pointer>()
                  ->type()
                  ->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Function) {
  ast::type::F32 f32;

  ast::VariableList params;
  auto* func = create<ast::Function>(
      Source{}, mod->RegisterSymbol("my_func"), "my_func", params, &f32,
      create<ast::BlockStatement>(), ast::FunctionDecorationList{});
  mod->AddFunction(func);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  ast::IdentifierExpression ident(mod->RegisterSymbol("my_func"), "my_func");
  EXPECT_TRUE(td()->DetermineResultType(&ident));
  ASSERT_NE(ident.result_type(), nullptr);
  EXPECT_TRUE(ident.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_Identifier_Unknown) {
  ast::IdentifierExpression a(mod->RegisterSymbol("a"), "a");
  EXPECT_FALSE(td()->DetermineResultType(&a));
}

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables) {
  ast::type::F32 f32;

  auto* in_var =
      create<ast::Variable>(Source{},                        // source
                            "in_var",                        // name
                            ast::StorageClass::kInput,       // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* out_var =
      create<ast::Variable>(Source{},                        // source
                            "out_var",                       // name
                            ast::StorageClass::kOutput,      // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* sb_var =
      create<ast::Variable>(Source{},                           // source
                            "sb_var",                           // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &f32,                               // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations
  auto* wg_var =
      create<ast::Variable>(Source{},                        // source
                            "wg_var",                        // name
                            ast::StorageClass::kWorkgroup,   // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* priv_var =
      create<ast::Variable>(Source{},                        // source
                            "priv_var",                      // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  mod->AddGlobalVariable(in_var);
  mod->AddGlobalVariable(out_var);
  mod->AddGlobalVariable(sb_var);
  mod->AddGlobalVariable(wg_var);
  mod->AddGlobalVariable(priv_var);

  ast::VariableList params;
  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("out_var"),
                                        "out_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("in_var"),
                                        "in_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("wg_var"),
                                        "wg_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("wg_var"),
                                        "wg_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("sb_var"),
                                        "sb_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("sb_var"),
                                        "sb_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("priv_var"),
                                        "priv_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("priv_var"),
                                        "priv_var")));
  auto* func =
      create<ast::Function>(Source{}, mod->RegisterSymbol("my_func"), "my_func",
                            params, &f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(func);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  const auto& vars = func->referenced_module_variables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0], out_var);
  EXPECT_EQ(vars[1], in_var);
  EXPECT_EQ(vars[2], wg_var);
  EXPECT_EQ(vars[3], sb_var);
  EXPECT_EQ(vars[4], priv_var);
}

TEST_F(TypeDeterminerTest, Function_RegisterInputOutputVariables_SubFunction) {
  ast::type::F32 f32;

  auto* in_var =
      create<ast::Variable>(Source{},                        // source
                            "in_var",                        // name
                            ast::StorageClass::kInput,       // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* out_var =
      create<ast::Variable>(Source{},                        // source
                            "out_var",                       // name
                            ast::StorageClass::kOutput,      // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* sb_var =
      create<ast::Variable>(Source{},                           // source
                            "sb_var",                           // name
                            ast::StorageClass::kStorageBuffer,  // storage_class
                            &f32,                               // type
                            false,                              // is_const
                            nullptr,                            // constructor
                            ast::VariableDecorationList{});     // decorations
  auto* wg_var =
      create<ast::Variable>(Source{},                        // source
                            "wg_var",                        // name
                            ast::StorageClass::kWorkgroup,   // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* priv_var =
      create<ast::Variable>(Source{},                        // source
                            "priv_var",                      // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  mod->AddGlobalVariable(in_var);
  mod->AddGlobalVariable(out_var);
  mod->AddGlobalVariable(sb_var);
  mod->AddGlobalVariable(wg_var);
  mod->AddGlobalVariable(priv_var);

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("out_var"),
                                        "out_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("in_var"),
                                        "in_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("wg_var"),
                                        "wg_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("wg_var"),
                                        "wg_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("sb_var"),
                                        "sb_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("sb_var"),
                                        "sb_var")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("priv_var"),
                                        "priv_var"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("priv_var"),
                                        "priv_var")));
  ast::VariableList params;
  auto* func =
      create<ast::Function>(Source{}, mod->RegisterSymbol("my_func"), "my_func",
                            params, &f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(func);

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("out_var"),
                                        "out_var"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("my_func"),
                                            "my_func"),
          ast::ExpressionList{})));
  auto* func2 =
      create<ast::Function>(Source{}, mod->RegisterSymbol("func"), "func",
                            params, &f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(func2);

  // Register the function
  EXPECT_TRUE(td()->Determine());

  const auto& vars = func2->referenced_module_variables();
  ASSERT_EQ(vars.size(), 5u);
  EXPECT_EQ(vars[0], out_var);
  EXPECT_EQ(vars[1], in_var);
  EXPECT_EQ(vars[2], wg_var);
  EXPECT_EQ(vars[3], sb_var);
  EXPECT_EQ(vars[4], priv_var);
}

TEST_F(TypeDeterminerTest, Function_NotRegisterFunctionVariable) {
  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "in_var",                        // name
                            ast::StorageClass::kFunction,    // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(var));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("var"), "var"),
      create<ast::ScalarConstructorExpression>(
          create<ast::FloatLiteral>(&f32, 1.f))));

  ast::VariableList params;
  auto* func =
      create<ast::Function>(Source{}, mod->RegisterSymbol("my_func"), "my_func",
                            params, &f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(func);

  ast::Variable v(Source{}, "var", ast::StorageClass::kFunction, &f32, false,
                  nullptr, ast::VariableDecorationList{});
  td()->RegisterVariableForTesting(&v);

  // Register the function
  EXPECT_TRUE(td()->Determine()) << td()->error();

  EXPECT_EQ(func->referenced_module_variables().size(), 0u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("first_member", &i32, decos));
  members.push_back(create<ast::StructMember>("second_member", &f32, decos));

  auto* strct = create<ast::Struct>(members);

  ast::type::Struct st("S", strct);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_struct",                     // name
                            ast::StorageClass::kNone,        // storage_class
                            &st,                             // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_struct"), "my_struct");
  auto* mem_ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("second_member"), "second_member");

  ast::MemberAccessorExpression mem(ident, mem_ident);
  EXPECT_TRUE(td()->DetermineResultType(&mem));
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->Is<ast::type::Pointer>());

  auto* ptr = mem.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_Struct_Alias) {
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::StructMemberDecorationList decos;
  ast::StructMemberList members;
  members.push_back(create<ast::StructMember>("first_member", &i32, decos));
  members.push_back(create<ast::StructMember>("second_member", &f32, decos));

  auto* strct = create<ast::Struct>(members);

  auto st = std::make_unique<ast::type::Struct>("alias", strct);
  ast::type::Alias alias(mod->RegisterSymbol("alias"), "alias", st.get());

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_struct",                     // name
                            ast::StorageClass::kNone,        // storage_class
                            &alias,                          // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_struct"), "my_struct");
  auto* mem_ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("second_member"), "second_member");

  ast::MemberAccessorExpression mem(ident, mem_ident);
  EXPECT_TRUE(td()->DetermineResultType(&mem));
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->Is<ast::type::Pointer>());

  auto* ptr = mem.result_type()->As<ast::type::Pointer>();
  EXPECT_TRUE(ptr->type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_vec",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("my_vec"),
                                                  "my_vec");
  auto* swizzle =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("xy"), "xy");

  ast::MemberAccessorExpression mem(ident, swizzle);
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(
      mem.result_type()->As<ast::type::Vector>()->type()->Is<ast::type::F32>());
  EXPECT_EQ(mem.result_type()->As<ast::type::Vector>()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_MemberAccessor_VectorSwizzle_SingleElement) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_vec",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("my_vec"),
                                                  "my_vec");
  auto* swizzle =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("x"), "x");

  ast::MemberAccessorExpression mem(ident, swizzle);
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();
  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->Is<ast::type::Pointer>());

  auto* ptr = mem.result_type()->As<ast::type::Pointer>();
  ASSERT_TRUE(ptr->type()->Is<ast::type::F32>());
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
  ast::type::I32 i32;
  ast::type::F32 f32;

  ast::type::Vector vec4(&f32, 4);

  ast::StructMemberDecorationList decos;
  ast::StructMemberList b_members;
  b_members.push_back(create<ast::StructMember>("foo", &vec4, decos));

  auto* strctB = create<ast::Struct>(b_members);
  ast::type::Struct stB("B", strctB);

  ast::type::Vector vecB(&stB, 3);

  ast::StructMemberList a_members;
  a_members.push_back(create<ast::StructMember>("mem", &vecB, decos));

  auto* strctA = create<ast::Struct>(a_members);

  ast::type::Struct stA("A", strctA);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "c",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &stA,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("c"), "c");
  auto* mem_ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("mem"), "mem");
  auto* foo_ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("foo"), "foo");
  auto* idx = create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 0));
  auto* swizzle =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("yx"), "yx");

  ast::MemberAccessorExpression mem(
      create<ast::MemberAccessorExpression>(
          create<ast::ArrayAccessorExpression>(
              create<ast::MemberAccessorExpression>(ident, mem_ident), idx),
          foo_ident),
      swizzle);
  EXPECT_TRUE(td()->DetermineResultType(&mem)) << td()->error();

  ASSERT_NE(mem.result_type(), nullptr);
  ASSERT_TRUE(mem.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(
      mem.result_type()->As<ast::type::Vector>()->type()->Is<ast::type::F32>());
  EXPECT_EQ(mem.result_type()->As<ast::type::Vector>()->size(), 2u);
}

using Expr_Binary_BitwiseTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_BitwiseTest, Scalar) {
  auto op = GetParam();

  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::I32>());
}

TEST_P(Expr_Binary_BitwiseTest, Vector) {
  auto op = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec3(&i32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::I32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
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

  ast::type::Bool bool_type;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &bool_type,                      // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::Bool>());
}

TEST_P(Expr_Binary_LogicalTest, Vector) {
  auto op = GetParam();

  ast::type::Bool bool_type;
  ast::type::Vector vec3(&bool_type, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::Bool>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Expr_Binary_LogicalTest,
                         testing::Values(ast::BinaryOp::kLogicalAnd,
                                         ast::BinaryOp::kLogicalOr));

using Expr_Binary_CompareTest = TypeDeterminerTestWithParam<ast::BinaryOp>;
TEST_P(Expr_Binary_CompareTest, Scalar) {
  auto op = GetParam();

  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::Bool>());
}

TEST_P(Expr_Binary_CompareTest, Vector) {
  auto op = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec3(&i32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      op, create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::Bool>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
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
  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "val",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(
      ast::BinaryOp::kMultiply,
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"),
      create<ast::IdentifierExpression>(mod->RegisterSymbol("val"), "val"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::I32>());
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Scalar) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* scalar =
      create<ast::Variable>(Source{},                        // source
                            "scalar",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* vector =
      create<ast::Variable>(Source{},                        // source
                            "vector",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(scalar);
  mod->AddGlobalVariable(vector);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("scalar"), "scalar"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Vector) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* scalar =
      create<ast::Variable>(Source{},                        // source
                            "scalar",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* vector =
      create<ast::Variable>(Source{},                        // source
                            "vector",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(scalar);
  mod->AddGlobalVariable(vector);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("scalar"), "scalar"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Vector) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* vector =
      create<ast::Variable>(Source{},                        // source
                            "vector",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(vector);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Scalar) {
  ast::type::F32 f32;
  ast::type::Matrix mat3x2(&f32, 3, 2);

  auto* scalar =
      create<ast::Variable>(Source{},                        // source
                            "scalar",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* matrix =
      create<ast::Variable>(Source{},                        // source
                            "matrix",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat3x2,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(scalar);
  mod->AddGlobalVariable(matrix);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("matrix"), "matrix"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("scalar"), "scalar"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Matrix>());

  auto* mat = expr.result_type()->As<ast::type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<ast::type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Scalar_Matrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat3x2(&f32, 3, 2);

  auto* scalar =
      create<ast::Variable>(Source{},                        // source
                            "scalar",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* matrix =
      create<ast::Variable>(Source{},                        // source
                            "matrix",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat3x2,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(scalar);
  mod->AddGlobalVariable(matrix);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("scalar"), "scalar"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("matrix"), "matrix"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Matrix>());

  auto* mat = expr.result_type()->As<ast::type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<ast::type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Vector) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 2);
  ast::type::Matrix mat3x2(&f32, 3, 2);

  auto* vector =
      create<ast::Variable>(Source{},                        // source
                            "vector",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* matrix =
      create<ast::Variable>(Source{},                        // source
                            "matrix",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat3x2,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(vector);
  mod->AddGlobalVariable(matrix);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("matrix"), "matrix"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Vector_Matrix) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Matrix mat3x2(&f32, 3, 2);

  auto* vector =
      create<ast::Variable>(Source{},                        // source
                            "vector",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* matrix =
      create<ast::Variable>(Source{},                        // source
                            "matrix",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat3x2,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(vector);
  mod->AddGlobalVariable(matrix);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("vector"), "vector"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("matrix"), "matrix"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 2u);
}

TEST_F(TypeDeterminerTest, Expr_Binary_Multiply_Matrix_Matrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat4x3(&f32, 4, 3);
  ast::type::Matrix mat3x4(&f32, 3, 4);

  auto* matrix1 =
      create<ast::Variable>(Source{},                        // source
                            "mat4x3",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat4x3,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* matrix2 =
      create<ast::Variable>(Source{},                        // source
                            "mat3x4",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &mat3x4,                         // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(matrix1);
  mod->AddGlobalVariable(matrix2);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::BinaryExpression expr(ast::BinaryOp::kMultiply,
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("mat4x3"), "mat4x3"),
                             create<ast::IdentifierExpression>(
                                 mod->RegisterSymbol("mat3x4"), "mat3x4"));

  ASSERT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Matrix>());

  auto* mat = expr.result_type()->As<ast::type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<ast::type::F32>());
  EXPECT_EQ(mat->rows(), 4u);
  EXPECT_EQ(mat->columns(), 4u);
}

using IntrinsicDerivativeTest = TypeDeterminerTestWithParam<std::string>;
TEST_P(IntrinsicDerivativeTest, Scalar) {
  auto name = GetParam();

  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "ident",                         // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("ident"), "ident"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::F32>());
}

TEST_P(IntrinsicDerivativeTest, Vector) {
  auto name = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "ident",                         // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec4,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("ident"), "ident"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 4u);
}

TEST_P(IntrinsicDerivativeTest, MissingParam) {
  auto name = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}

TEST_P(IntrinsicDerivativeTest, ToomManyParams) {
  auto name = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec4(&f32, 4);

  auto* var1 =
      create<ast::Variable>(Source{},                        // source
                            "ident1",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec4,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* var2 =
      create<ast::Variable>(Source{},                        // source
                            "ident2",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec4,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var1);
  mod->AddGlobalVariable(var2);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("ident1"), "ident1"));
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("ident2"), "ident2"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
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

  ast::type::Bool bool_type;
  ast::type::Vector vec3(&bool_type, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());

  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::Bool>());
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         Intrinsic,
                         testing::Values("any", "all"));

using Intrinsic_FloatMethod = TypeDeterminerTestWithParam<std::string>;
TEST_P(Intrinsic_FloatMethod, Vector) {
  auto name = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::Bool>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(Intrinsic_FloatMethod, Scalar) {
  auto name = GetParam();

  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::Bool>());
}

TEST_P(Intrinsic_FloatMethod, MissingParam) {
  auto name = GetParam();

  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for " + name);
}

TEST_P(Intrinsic_FloatMethod, TooManyParams) {
  auto name = GetParam();

  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
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
  ast::type::TextureDimension dim;
  Texture type = Texture::kF32;
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
      if (type->Is<ast::type::I32>()) {
        return std::make_unique<ast::type::I32>();
      } else if (type->Is<ast::type::U32>()) {
        return std::make_unique<ast::type::U32>();
      } else {
        return std::make_unique<ast::type::F32>();
      }
    } else if (dim == ast::type::TextureDimension::k1dArray ||
               dim == ast::type::TextureDimension::k2d) {
      return std::make_unique<ast::type::Vector>(type, 2);
    } else if (dim == ast::type::TextureDimension::kCubeArray) {
      return std::make_unique<ast::type::Vector>(type, 4);
    } else {
      return std::make_unique<ast::type::Vector>(type, 3);
    }
  }

  void add_call_param(std::string name,
                      ast::type::Type* type,
                      ast::ExpressionList* call_params) {
    auto* var =
        create<ast::Variable>(Source{},                        // source
                              name,                            // name
                              ast::StorageClass::kNone,        // storage_class
                              type,                            // type
                              false,                           // is_const
                              nullptr,                         // constructor
                              ast::VariableDecorationList{});  // decorations
    mod->AddGlobalVariable(var);
    call_params->push_back(
        create<ast::IdentifierExpression>(mod->RegisterSymbol(name), name));
  }

  std::unique_ptr<ast::type::Type> subtype(Texture type) {
    if (type == Texture::kF32) {
      return std::make_unique<ast::type::F32>();
    }
    if (type == Texture::kI32) {
      return std::make_unique<ast::type::I32>();
    }
    return std::make_unique<ast::type::U32>();
  }
};

using Intrinsic_StorageTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_StorageTextureOperation, TextureLoadRo) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;
  auto format = GetParam().format;

  ast::type::I32 i32;
  auto coords_type = get_coords_type(dim, &i32);

  ast::type::Type* texture_type = mod->create<ast::type::StorageTexture>(
      dim, ast::AccessControl::kReadOnly, format);

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type, &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("textureLoad"),
                                        "textureLoad"),
      call_params);

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::I32>());
  } else {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::U32>());
  }
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_StorageTextureOperation,
    testing::Values(
        TextureTestParams{ast::type::TextureDimension::k1d, Texture::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1d, Texture::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1d, Texture::kF32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k1dArray, Texture::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k1dArray, Texture::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k1dArray, Texture::kF32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2d, Texture::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2d, Texture::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2d, Texture::kF32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k2dArray, Texture::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k2dArray, Texture::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k2dArray, Texture::kF32,
                          ast::type::ImageFormat::kR8Unorm},
        TextureTestParams{ast::type::TextureDimension::k3d, Texture::kF32,
                          ast::type::ImageFormat::kR16Float},
        TextureTestParams{ast::type::TextureDimension::k3d, Texture::kI32,
                          ast::type::ImageFormat::kR16Sint},
        TextureTestParams{ast::type::TextureDimension::k3d, Texture::kF32,
                          ast::type::ImageFormat::kR8Unorm}));

using Intrinsic_SampledTextureOperation = Intrinsic_TextureOperation;
TEST_P(Intrinsic_SampledTextureOperation, TextureLoadSampled) {
  auto dim = GetParam().dim;
  auto type = GetParam().type;

  ast::type::I32 i32;
  std::unique_ptr<ast::type::Type> s = subtype(type);
  auto coords_type = get_coords_type(dim, &i32);
  auto texture_type = std::make_unique<ast::type::SampledTexture>(dim, s.get());

  ast::ExpressionList call_params;

  add_call_param("texture", texture_type.get(), &call_params);
  add_call_param("coords", coords_type.get(), &call_params);
  add_call_param("lod", &i32, &call_params);

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("textureLoad"),
                                        "textureLoad"),
      call_params);

  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  if (type == Texture::kF32) {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::F32>());
  } else if (type == Texture::kI32) {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::I32>());
  } else {
    EXPECT_TRUE(expr.result_type()
                    ->As<ast::type::Vector>()
                    ->type()
                    ->Is<ast::type::U32>());
  }
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 4u);
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    Intrinsic_SampledTextureOperation,
    testing::Values(TextureTestParams{ast::type::TextureDimension::k2d},
                    TextureTestParams{ast::type::TextureDimension::k2dArray},
                    TextureTestParams{ast::type::TextureDimension::kCube},
                    TextureTestParams{
                        ast::type::TextureDimension::kCubeArray}));

TEST_F(TypeDeterminerTest, Intrinsic_Dot) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("dot"), "dot"),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select) {
  ast::type::F32 f32;
  ast::type::Bool bool_type;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Vector bool_vec3(&bool_type, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "my_var",                        // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* bool_var =
      create<ast::Variable>(Source{},                        // source
                            "bool_var",                      // name
                            ast::StorageClass::kNone,        // storage_class
                            &bool_vec3,                      // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);
  mod->AddGlobalVariable(bool_var);

  ast::ExpressionList call_params;
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("my_var"), "my_var"));
  call_params.push_back(create<ast::IdentifierExpression>(
      mod->RegisterSymbol("bool_var"), "bool_var"));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod->RegisterSymbol("select"), "select"),
                           call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr)) << td()->error();
  ASSERT_NE(expr.result_type(), nullptr);
  EXPECT_TRUE(expr.result_type()->Is<ast::type::Vector>());
  EXPECT_EQ(expr.result_type()->As<ast::type::Vector>()->size(), 3u);
  EXPECT_TRUE(expr.result_type()
                  ->As<ast::type::Vector>()
                  ->type()
                  ->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, Intrinsic_Select_TooFewParams) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod->RegisterSymbol("select"), "select"),
                           call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for select expected 3 got 1");
}

TEST_F(TypeDeterminerTest, Intrinsic_Select_TooManyParams) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "v",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v"), "v"));

  ast::CallExpression expr(create<ast::IdentifierExpression>(
                               mod->RegisterSymbol("select"), "select"),
                           call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for select expected 3 got 4");
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Vector vec2(&f32, 2);

  auto* var1 =
      create<ast::Variable>(Source{},                        // source
                            "v3",                            // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* var2 =
      create<ast::Variable>(Source{},                        // source
                            "v2",                            // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec2,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var1);
  mod->AddGlobalVariable(var2);

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v3"), "v3"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v2"), "v2"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("outerProduct"),
                                        "outerProduct"),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_TRUE(td()->DetermineResultType(&expr));

  ASSERT_NE(expr.result_type(), nullptr);
  ASSERT_TRUE(expr.result_type()->Is<ast::type::Matrix>());

  auto* mat = expr.result_type()->As<ast::type::Matrix>();
  EXPECT_TRUE(mat->type()->Is<ast::type::F32>());
  EXPECT_EQ(mat->rows(), 3u);
  EXPECT_EQ(mat->columns(), 2u);
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct_TooFewParams) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Vector vec2(&f32, 2);

  auto* var2 =
      create<ast::Variable>(Source{},                        // source
                            "v2",                            // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec2,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var2);

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v2"), "v2"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("outerProduct"),
                                        "outerProduct"),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for outerProduct");
}

TEST_F(TypeDeterminerTest, Intrinsic_OuterProduct_TooManyParams) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Vector vec2(&f32, 2);

  auto* var2 =
      create<ast::Variable>(Source{},                        // source
                            "v2",                            // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec2,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var2);

  ast::ExpressionList call_params;
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v2"), "v2"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v2"), "v2"));
  call_params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("v2"), "v2"));

  ast::CallExpression expr(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("outerProduct"),
                                        "outerProduct"),
      call_params);

  // Register the variable
  EXPECT_TRUE(td()->Determine());
  EXPECT_FALSE(td()->DetermineResultType(&expr));
  EXPECT_EQ(td()->error(), "incorrect number of parameters for outerProduct");
}

using UnaryOpExpressionTest = TypeDeterminerTestWithParam<ast::UnaryOp>;
TEST_P(UnaryOpExpressionTest, Expr_UnaryOp) {
  auto op = GetParam();

  ast::type::F32 f32;

  ast::type::Vector vec4(&f32, 4);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "ident",                         // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec4,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  EXPECT_TRUE(td()->Determine());

  ast::UnaryOpExpression der(op, create<ast::IdentifierExpression>(
                                     mod->RegisterSymbol("ident"), "ident"));
  EXPECT_TRUE(td()->DetermineResultType(&der));
  ASSERT_NE(der.result_type(), nullptr);
  ASSERT_TRUE(der.result_type()->Is<ast::type::Vector>());
  EXPECT_TRUE(
      der.result_type()->As<ast::type::Vector>()->type()->Is<ast::type::F32>());
  EXPECT_EQ(der.result_type()->As<ast::type::Vector>()->size(), 4u);
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         UnaryOpExpressionTest,
                         testing::Values(ast::UnaryOp::kNegation,
                                         ast::UnaryOp::kNot));

TEST_F(TypeDeterminerTest, StorageClass_SetsIfMissing) {
  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* stmt = create<ast::VariableDeclStatement>(var);

  auto* body = create<ast::BlockStatement>();
  body->append(stmt);
  auto* func = create<ast::Function>(Source{}, mod->RegisterSymbol("func"),
                                     "func", ast::VariableList{}, &i32, body,
                                     ast::FunctionDecorationList{});

  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_EQ(var->storage_class(), ast::StorageClass::kFunction);
}

TEST_F(TypeDeterminerTest, StorageClass_DoesNotSetOnConst) {
  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kNone,        // storage_class
                            &i32,                            // type
                            true,                            // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* stmt = create<ast::VariableDeclStatement>(var);

  auto* body = create<ast::BlockStatement>();
  body->append(stmt);
  auto* func = create<ast::Function>(Source{}, mod->RegisterSymbol("func"),
                                     "func", ast::VariableList{}, &i32, body,
                                     ast::FunctionDecorationList{});

  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_EQ(var->storage_class(), ast::StorageClass::kNone);
}

TEST_F(TypeDeterminerTest, StorageClass_NonFunctionClassError) {
  ast::type::I32 i32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kWorkgroup,   // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* stmt = create<ast::VariableDeclStatement>(var);

  auto* body = create<ast::BlockStatement>();
  body->append(stmt);
  auto* func = create<ast::Function>(Source{}, mod->RegisterSymbol("func"),
                                     "func", ast::VariableList{}, &i32, body,
                                     ast::FunctionDecorationList{});

  mod->AddFunction(func);

  EXPECT_FALSE(td()->Determine());
  EXPECT_EQ(td()->error(),
            "function variable has a non-function storage class");
}

struct IntrinsicData {
  const char* name;
  ast::Intrinsic intrinsic;
};
inline std::ostream& operator<<(std::ostream& out, IntrinsicData data) {
  out << data.name;
  return out;
}
using IntrinsicDataTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(IntrinsicDataTest, Lookup) {
  auto param = GetParam();

  ast::IdentifierExpression ident(mod->RegisterSymbol(param.name), param.name);
  EXPECT_TRUE(td()->SetIntrinsicIfNeeded(&ident));
  EXPECT_EQ(ident.intrinsic(), param.intrinsic);
  EXPECT_TRUE(ident.IsIntrinsic());
}
INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    IntrinsicDataTest,
    testing::Values(
        IntrinsicData{"abs", ast::Intrinsic::kAbs},
        IntrinsicData{"acos", ast::Intrinsic::kAcos},
        IntrinsicData{"all", ast::Intrinsic::kAll},
        IntrinsicData{"any", ast::Intrinsic::kAny},
        IntrinsicData{"arrayLength", ast::Intrinsic::kArrayLength},
        IntrinsicData{"asin", ast::Intrinsic::kAsin},
        IntrinsicData{"atan", ast::Intrinsic::kAtan},
        IntrinsicData{"atan2", ast::Intrinsic::kAtan2},
        IntrinsicData{"ceil", ast::Intrinsic::kCeil},
        IntrinsicData{"clamp", ast::Intrinsic::kClamp},
        IntrinsicData{"cos", ast::Intrinsic::kCos},
        IntrinsicData{"cosh", ast::Intrinsic::kCosh},
        IntrinsicData{"countOneBits", ast::Intrinsic::kCountOneBits},
        IntrinsicData{"cross", ast::Intrinsic::kCross},
        IntrinsicData{"determinant", ast::Intrinsic::kDeterminant},
        IntrinsicData{"distance", ast::Intrinsic::kDistance},
        IntrinsicData{"dot", ast::Intrinsic::kDot},
        IntrinsicData{"dpdx", ast::Intrinsic::kDpdx},
        IntrinsicData{"dpdxCoarse", ast::Intrinsic::kDpdxCoarse},
        IntrinsicData{"dpdxFine", ast::Intrinsic::kDpdxFine},
        IntrinsicData{"dpdy", ast::Intrinsic::kDpdy},
        IntrinsicData{"dpdyCoarse", ast::Intrinsic::kDpdyCoarse},
        IntrinsicData{"dpdyFine", ast::Intrinsic::kDpdyFine},
        IntrinsicData{"exp", ast::Intrinsic::kExp},
        IntrinsicData{"exp2", ast::Intrinsic::kExp2},
        IntrinsicData{"faceForward", ast::Intrinsic::kFaceForward},
        IntrinsicData{"floor", ast::Intrinsic::kFloor},
        IntrinsicData{"fma", ast::Intrinsic::kFma},
        IntrinsicData{"fract", ast::Intrinsic::kFract},
        IntrinsicData{"frexp", ast::Intrinsic::kFrexp},
        IntrinsicData{"fwidth", ast::Intrinsic::kFwidth},
        IntrinsicData{"fwidthCoarse", ast::Intrinsic::kFwidthCoarse},
        IntrinsicData{"fwidthFine", ast::Intrinsic::kFwidthFine},
        IntrinsicData{"inverseSqrt", ast::Intrinsic::kInverseSqrt},
        IntrinsicData{"isFinite", ast::Intrinsic::kIsFinite},
        IntrinsicData{"isInf", ast::Intrinsic::kIsInf},
        IntrinsicData{"isNan", ast::Intrinsic::kIsNan},
        IntrinsicData{"isNormal", ast::Intrinsic::kIsNormal},
        IntrinsicData{"ldexp", ast::Intrinsic::kLdexp},
        IntrinsicData{"length", ast::Intrinsic::kLength},
        IntrinsicData{"log", ast::Intrinsic::kLog},
        IntrinsicData{"log2", ast::Intrinsic::kLog2},
        IntrinsicData{"max", ast::Intrinsic::kMax},
        IntrinsicData{"min", ast::Intrinsic::kMin},
        IntrinsicData{"mix", ast::Intrinsic::kMix},
        IntrinsicData{"modf", ast::Intrinsic::kModf},
        IntrinsicData{"normalize", ast::Intrinsic::kNormalize},
        IntrinsicData{"outerProduct", ast::Intrinsic::kOuterProduct},
        IntrinsicData{"pow", ast::Intrinsic::kPow},
        IntrinsicData{"reflect", ast::Intrinsic::kReflect},
        IntrinsicData{"reverseBits", ast::Intrinsic::kReverseBits},
        IntrinsicData{"round", ast::Intrinsic::kRound},
        IntrinsicData{"select", ast::Intrinsic::kSelect},
        IntrinsicData{"sign", ast::Intrinsic::kSign},
        IntrinsicData{"sin", ast::Intrinsic::kSin},
        IntrinsicData{"sinh", ast::Intrinsic::kSinh},
        IntrinsicData{"smoothStep", ast::Intrinsic::kSmoothStep},
        IntrinsicData{"sqrt", ast::Intrinsic::kSqrt},
        IntrinsicData{"step", ast::Intrinsic::kStep},
        IntrinsicData{"tan", ast::Intrinsic::kTan},
        IntrinsicData{"tanh", ast::Intrinsic::kTanh},
        IntrinsicData{"textureLoad", ast::Intrinsic::kTextureLoad},
        IntrinsicData{"textureSample", ast::Intrinsic::kTextureSample},
        IntrinsicData{"textureSampleBias", ast::Intrinsic::kTextureSampleBias},
        IntrinsicData{"textureSampleCompare",
                      ast::Intrinsic::kTextureSampleCompare},
        IntrinsicData{"textureSampleGrad", ast::Intrinsic::kTextureSampleGrad},
        IntrinsicData{"textureSampleLevel",
                      ast::Intrinsic::kTextureSampleLevel},
        IntrinsicData{"trunc", ast::Intrinsic::kTrunc}));

TEST_F(TypeDeterminerTest, IntrinsicNotSetIfNotMatched) {
  ast::IdentifierExpression ident(mod->RegisterSymbol("not_intrinsic"),
                                  "not_intrinsic");
  EXPECT_FALSE(td()->SetIntrinsicIfNeeded(&ident));
  EXPECT_EQ(ident.intrinsic(), ast::Intrinsic::kNone);
  EXPECT_FALSE(ident.IsIntrinsic());
}

using ImportData_SingleParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_SingleParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_P(ImportData_SingleParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_SingleParamTest, Error_MultipleParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_SingleParamTest,
    testing::Values(IntrinsicData{"acos", ast::Intrinsic::kAcos},
                    IntrinsicData{"asin", ast::Intrinsic::kAsin},
                    IntrinsicData{"atan", ast::Intrinsic::kAtan},
                    IntrinsicData{"ceil", ast::Intrinsic::kCeil},
                    IntrinsicData{"cos", ast::Intrinsic::kCos},
                    IntrinsicData{"cosh", ast::Intrinsic::kCosh},
                    IntrinsicData{"exp", ast::Intrinsic::kExp},
                    IntrinsicData{"exp2", ast::Intrinsic::kExp2},
                    IntrinsicData{"floor", ast::Intrinsic::kFloor},
                    IntrinsicData{"fract", ast::Intrinsic::kFract},
                    IntrinsicData{"inverseSqrt", ast::Intrinsic::kInverseSqrt},
                    IntrinsicData{"log", ast::Intrinsic::kLog},
                    IntrinsicData{"log2", ast::Intrinsic::kLog2},
                    IntrinsicData{"normalize", ast::Intrinsic::kNormalize},
                    IntrinsicData{"round", ast::Intrinsic::kRound},
                    IntrinsicData{"sign", ast::Intrinsic::kSign},
                    IntrinsicData{"sin", ast::Intrinsic::kSin},
                    IntrinsicData{"sinh", ast::Intrinsic::kSinh},
                    IntrinsicData{"sqrt", ast::Intrinsic::kSqrt},
                    IntrinsicData{"tan", ast::Intrinsic::kTan},
                    IntrinsicData{"tanh", ast::Intrinsic::kTanh},
                    IntrinsicData{"trunc", ast::Intrinsic::kTrunc}));

using ImportData_SingleParam_FloatOrInt_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_SingleParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, -11)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::I32>());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_signed_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  ast::type::U32 u32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::U32>());
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  ast::type::U32 u32;
  ast::type::Vector vec(&u32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_unsigned_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Error_Bool) {
  auto param = GetParam();

  ast::type::Bool bool_type;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, false)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float or int, scalar or vector values");
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_SingleParam_FloatOrInt_Test, Error_MultipleParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 3");
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_SingleParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{"abs",
                                                       ast::Intrinsic::kAbs}));

TEST_F(TypeDeterminerTest, ImportData_Length_Scalar) {
  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  ASSERT_TRUE(td()->DetermineResultType(params)) << td()->error();

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("length"),
                                                  "length");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_F(TypeDeterminerTest, ImportData_Length_FloatVector) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("length"),
                                                  "length");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_Integer) {
  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("length"),
                                                  "length");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect type for length. Requires float scalar or float vector "
            "values");
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_NoParams) {
  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("length"),
                                                  "length");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for length. Expected 1 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Length_Error_MultipleParams) {
  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(mod->RegisterSymbol("length"),
                                                  "length");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for length. Expected 1 got 3");
}

using ImportData_TwoParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_TwoParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_P(ImportData_TwoParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_TwoParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 0");
}

TEST_P(ImportData_TwoParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 1");
}

TEST_P(ImportData_TwoParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec2(&f32, 2);
  ast::type::Vector vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec2, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_TwoParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_TwoParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_TwoParamTest,
    testing::Values(IntrinsicData{"atan2", ast::Intrinsic::kAtan2},
                    IntrinsicData{"pow", ast::Intrinsic::kPow},
                    IntrinsicData{"step", ast::Intrinsic::kStep},
                    IntrinsicData{"reflect", ast::Intrinsic::kReflect}));

TEST_F(TypeDeterminerTest, ImportData_Distance_Scalar) {
  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Vector) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::F32>());
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_Integer) {
  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect type for distance. Requires float scalar or float "
            "vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_NoParams) {
  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_OneParam) {
  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 1");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_MismatchedParamCount) {
  ast::type::F32 f32;
  ast::type::Vector vec2(&f32, 2);
  ast::type::Vector vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec2, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), "mismatched parameter types for distance");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_MismatchedParamType) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), "mismatched parameter types for distance");
}

TEST_F(TypeDeterminerTest, ImportData_Distance_Error_TooManyParams) {
  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("distance"), "distance");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for distance. Expected 2 got 3");
}

TEST_F(TypeDeterminerTest, ImportData_Cross) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_Scalar) {
  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect type for cross. Requires float vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_IntType) {
  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect type for cross. Requires float vector values");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_MissingParams) {
  ast::ExpressionList params;
  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 0");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_TooFewParams) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 1");
}

TEST_F(TypeDeterminerTest, ImportData_Cross_Error_TooManyParams) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_3));

  auto* ident =
      create<ast::IdentifierExpression>(mod->RegisterSymbol("cross"), "cross");
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            "incorrect number of parameters for cross. Expected 2 got 3");
}

using ImportData_ThreeParamTest = TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_ThreeParamTest, Scalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_P(ImportData_ThreeParamTest, Vector) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParamTest, Error_Integer) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 2)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float scalar or float vector values");
}

TEST_P(ImportData_ThreeParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 0");
}

TEST_P(ImportData_ThreeParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 1");
}

TEST_P(ImportData_ThreeParamTest, Error_TwoParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 2");
}

TEST_P(ImportData_ThreeParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec2(&f32, 2);
  ast::type::Vector vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec2, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 4");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_ThreeParamTest,
    testing::Values(IntrinsicData{"mix", ast::Intrinsic::kMix},
                    IntrinsicData{"smoothStep", ast::Intrinsic::kSmoothStep},
                    IntrinsicData{"fma", ast::Intrinsic::kFma},
                    IntrinsicData{"faceForward",
                                  ast::Intrinsic::kFaceForward}));

using ImportData_ThreeParam_FloatOrInt_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Float_Scalar) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_scalar());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Float_Vector) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Sint_Scalar) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::I32>());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Sint_Vector) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_signed_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Uint_Scalar) {
  auto param = GetParam();

  ast::type::U32 u32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::U32>());
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Uint_Vector) {
  auto param = GetParam();

  ast::type::U32 u32;
  ast::type::Vector vec(&u32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_unsigned_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_Bool) {
  auto param = GetParam();

  ast::type::Bool bool_type;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, false)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float or int, scalar or vector values");
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 0");
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_OneParam) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 1");
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_TwoParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 2");
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec2(&f32, 2);
  ast::type::Vector vec3(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList vals_3;
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals_3.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec2, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_2));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_3));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3.0f)));

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_ThreeParam_FloatOrInt_Test, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 3 got 4");
}

INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_ThreeParam_FloatOrInt_Test,
                         testing::Values(IntrinsicData{
                             "clamp", ast::Intrinsic::kClamp}));

using ImportData_Int_SingleParamTest =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_Int_SingleParamTest, Scalar) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_integer_scalar());
}

TEST_P(ImportData_Int_SingleParamTest, Vector) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_signed_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_Int_SingleParamTest, Error_Float) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.f)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires integer scalar or integer vector values");
}

TEST_P(ImportData_Int_SingleParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_Int_SingleParamTest, Error_MultipleParams) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_Int_SingleParamTest,
    testing::Values(
        IntrinsicData{"countOneBits", ast::Intrinsic::kCountOneBits},
        IntrinsicData{"reverseBits", ast::Intrinsic::kReverseBits}));

using ImportData_FloatOrInt_TwoParamTest =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Signed) {
  auto param = GetParam();

  ast::type::I32 i32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::I32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Unsigned) {
  auto param = GetParam();

  ast::type::U32 u32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::U32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Scalar_Float) {
  auto param = GetParam();

  ast::type::F32 f32;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::F32>());
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Signed) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_signed_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Unsigned) {
  auto param = GetParam();

  ast::type::U32 u32;
  ast::type::Vector vec(&u32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::UintLiteral>(&u32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_unsigned_integer_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Vector_Float) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->is_float_vector());
  EXPECT_EQ(ident->result_type()->As<ast::type::Vector>()->size(), 3u);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_Bool) {
  auto param = GetParam();

  ast::type::Bool bool_type;

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, true)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::BoolLiteral>(&bool_type, false)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("incorrect type for ") + param.name +
                ". Requires float or int, scalar or vector values");
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 0");
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_OneParam) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 1");
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_MismatchedParamCount) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec2(&i32, 2);
  ast::type::Vector vec3(&i32, 3);

  ast::ExpressionList vals_1;
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_1.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  ast::ExpressionList vals_2;
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals_2.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::TypeConstructorExpression>(&vec2, vals_1));
  params.push_back(create<ast::TypeConstructorExpression>(&vec3, vals_2));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_MismatchedParamType) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::type::Vector vec(&i32, 3);

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 3)));

  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::TypeConstructorExpression>(&vec, vals));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(),
            std::string("mismatched parameter types for ") + param.name);
}

TEST_P(ImportData_FloatOrInt_TwoParamTest, Error_TooManyParams) {
  auto param = GetParam();

  ast::type::I32 i32;
  ast::ExpressionList params;
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));
  params.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::SintLiteral>(&i32, 1)));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 2 got 3");
}

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    ImportData_FloatOrInt_TwoParamTest,
    testing::Values(IntrinsicData{"min", ast::Intrinsic::kMin},
                    IntrinsicData{"max", ast::Intrinsic::kMax}));

TEST_F(TypeDeterminerTest, ImportData_GLSL_Determinant) {
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("var"), "var"));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol("determinant"), "determinant");

  ast::CallExpression call(ident, params);

  EXPECT_TRUE(td()->DetermineResultType(&call)) << td()->error();
  ASSERT_NE(ident->result_type(), nullptr);
  EXPECT_TRUE(ident->result_type()->Is<ast::type::F32>());
}

using ImportData_Matrix_OneParam_Test =
    TypeDeterminerTestWithParam<IntrinsicData>;
TEST_P(ImportData_Matrix_OneParam_Test, Error_Float) {
  auto param = GetParam();

  ast::type::F32 f32;

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("var"), "var"));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect type for ") + param.name +
                               ". Requires matrix value");
}

TEST_P(ImportData_Matrix_OneParam_Test, NoParams) {
  auto param = GetParam();

  ast::ExpressionList params;

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 0");
}

TEST_P(ImportData_Matrix_OneParam_Test, TooManyParams) {
  auto param = GetParam();

  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "var",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  mod->AddGlobalVariable(var);

  // Register the global
  ASSERT_TRUE(td()->Determine()) << td()->error();

  ast::ExpressionList params;
  params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("var"), "var"));
  params.push_back(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("var"), "var"));

  auto* ident = create<ast::IdentifierExpression>(
      mod->RegisterSymbol(param.name), param.name);
  ast::CallExpression call(ident, params);

  EXPECT_FALSE(td()->DetermineResultType(&call));
  EXPECT_EQ(td()->error(), std::string("incorrect number of parameters for ") +
                               param.name + ". Expected 1 got 2");
}
INSTANTIATE_TEST_SUITE_P(TypeDeterminerTest,
                         ImportData_Matrix_OneParam_Test,
                         testing::Values(IntrinsicData{
                             "determinant", ast::Intrinsic::kDeterminant}));

TEST_F(TypeDeterminerTest, Function_EntryPoints_StageDecoration) {
  ast::type::F32 f32;

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
  auto* body = create<ast::BlockStatement>();
  auto* func_b =
      create<ast::Function>(Source{}, mod->RegisterSymbol("b"), "b", params,
                            &f32, body, ast::FunctionDecorationList{});

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("second"),
                                        "second"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("b"), "b"),
          ast::ExpressionList{})));
  auto* func_c =
      create<ast::Function>(Source{}, mod->RegisterSymbol("c"), "c", params,
                            &f32, body, ast::FunctionDecorationList{});

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("first"), "first"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("c"), "c"),
          ast::ExpressionList{})));
  auto* func_a =
      create<ast::Function>(Source{}, mod->RegisterSymbol("a"), "a", params,
                            &f32, body, ast::FunctionDecorationList{});

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("call_a"),
                                        "call_a"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("a"), "a"),
          ast::ExpressionList{})));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("call_b"),
                                        "call_b"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("b"), "b"),
          ast::ExpressionList{})));
  auto* ep_1 = create<ast::Function>(
      Source{}, mod->RegisterSymbol("ep_1"), "ep_1", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>(mod->RegisterSymbol("call_c"),
                                        "call_c"),
      create<ast::CallExpression>(
          create<ast::IdentifierExpression>(mod->RegisterSymbol("c"), "c"),
          ast::ExpressionList{})));
  auto* ep_2 = create<ast::Function>(
      Source{}, mod->RegisterSymbol("ep_2"), "ep_2", params, &f32, body,
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}),
      });

  mod->AddFunction(func_b);
  mod->AddFunction(func_c);
  mod->AddFunction(func_a);
  mod->AddFunction(ep_1);
  mod->AddFunction(ep_2);

  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "first",                          // name
                            ast::StorageClass::kPrivate,      // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "second",                         // name
                            ast::StorageClass::kPrivate,      // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "call_a",                         // name
                            ast::StorageClass::kPrivate,      // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "call_b",                         // name
                            ast::StorageClass::kPrivate,      // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations
  mod->AddGlobalVariable(
      create<ast::Variable>(Source{},                         // source
                            "call_c",                         // name
                            ast::StorageClass::kPrivate,      // storage_class
                            &f32,                             // type
                            false,                            // is_const
                            nullptr,                          // constructor
                            ast::VariableDecorationList{}));  // decorations

  // Register the functions and calculate the callers
  ASSERT_TRUE(td()->Determine()) << td()->error();

  const auto& b_eps = func_b->ancestor_entry_points();
  ASSERT_EQ(2u, b_eps.size());
  EXPECT_EQ(mod->RegisterSymbol("ep_1"), b_eps[0]);
  EXPECT_EQ(mod->RegisterSymbol("ep_2"), b_eps[1]);

  const auto& a_eps = func_a->ancestor_entry_points();
  ASSERT_EQ(1u, a_eps.size());
  EXPECT_EQ(mod->RegisterSymbol("ep_1"), a_eps[0]);

  const auto& c_eps = func_c->ancestor_entry_points();
  ASSERT_EQ(2u, c_eps.size());
  EXPECT_EQ(mod->RegisterSymbol("ep_1"), c_eps[0]);
  EXPECT_EQ(mod->RegisterSymbol("ep_2"), c_eps[1]);

  EXPECT_TRUE(ep_1->ancestor_entry_points().empty());
  EXPECT_TRUE(ep_2->ancestor_entry_points().empty());
}

using TypeDeterminerTextureIntrinsicTest =
    TypeDeterminerTestWithParam<ast::intrinsic::test::TextureOverloadCase>;

INSTANTIATE_TEST_SUITE_P(
    TypeDeterminerTest,
    TypeDeterminerTextureIntrinsicTest,
    testing::ValuesIn(ast::intrinsic::test::TextureOverloadCase::ValidCases()));

std::string to_str(const std::string& function,
                   const ast::intrinsic::TextureSignature* sig) {
  struct Parameter {
    size_t idx;
    std::string name;
  };
  std::vector<Parameter> params;
  auto maybe_add_param = [&params](size_t idx, const char* name) {
    if (idx != ast::intrinsic::TextureSignature::Parameters::kNotUsed) {
      params.emplace_back(Parameter{idx, name});
    }
  };
  maybe_add_param(sig->params.idx.array_index, "array_index");
  maybe_add_param(sig->params.idx.bias, "bias");
  maybe_add_param(sig->params.idx.coords, "coords");
  maybe_add_param(sig->params.idx.depth_ref, "depth_ref");
  maybe_add_param(sig->params.idx.ddx, "ddx");
  maybe_add_param(sig->params.idx.ddy, "ddy");
  maybe_add_param(sig->params.idx.level, "level");
  maybe_add_param(sig->params.idx.offset, "offset");
  maybe_add_param(sig->params.idx.sampler, "sampler");
  maybe_add_param(sig->params.idx.sample_index, "sample_index");
  maybe_add_param(sig->params.idx.texture, "texture");
  maybe_add_param(sig->params.idx.value, "value");
  std::sort(
      params.begin(), params.end(),
      [](const Parameter& a, const Parameter& b) { return a.idx < b.idx; });

  std::stringstream out;
  out << function << "(";
  bool first = true;
  for (auto& param : params) {
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
    case ValidTextureOverload::kSampleGradDepth2dF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleGradDepth2dOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref, offset))";
    case ValidTextureOverload::kSampleGradDepth2dArrayF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref))";
    case ValidTextureOverload::kSampleGradDepth2dArrayOffsetF32:
      return R"(textureSampleCompare(texture, sampler, coords, array_index, depth_ref, offset))";
    case ValidTextureOverload::kSampleGradDepthCubeF32:
      return R"(textureSampleCompare(texture, sampler, coords, depth_ref))";
    case ValidTextureOverload::kSampleGradDepthCubeArrayF32:
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
  ast::CallExpression call{ident, param.args(this)};

  ASSERT_TRUE(td()->Determine()) << td()->error();
  ASSERT_TRUE(td()->DetermineResultType(&call)) << td()->error();

  if (std::string(param.function) == "textureStore") {
    EXPECT_EQ(call.result_type(), ty.void_);
  } else {
    switch (param.texture_kind) {
      case ast::intrinsic::test::TextureKind::kRegular:
      case ast::intrinsic::test::TextureKind::kMultisampled:
      case ast::intrinsic::test::TextureKind::kStorage: {
        auto* datatype = param.resultVectorComponentType(this);
        ASSERT_TRUE(call.result_type()->Is<ast::type::Vector>());
        EXPECT_EQ(call.result_type()->As<ast::type::Vector>()->type(),
                  datatype);
        break;
      }
      case ast::intrinsic::test::TextureKind::kDepth: {
        EXPECT_EQ(call.result_type(), ty.f32);
        break;
      }
    }
  }

  auto* sig = static_cast<const ast::intrinsic::TextureSignature*>(
      ident->intrinsic_signature());
  ASSERT_NE(sig, nullptr);

  auto got = to_str(param.function, sig);
  auto* expected = expected_texture_overload(param.overload);
  EXPECT_EQ(got, expected);
}

}  // namespace
}  // namespace tint
