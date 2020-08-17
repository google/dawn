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

#include "src/validator_impl.h"

#include "gtest/gtest.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type_determiner.h"

namespace tint {
namespace {

class TypeDeterminerHelper {
  // TODO(sarahM0): move this form this test file, type_determiner_test.cc and
  // validator_test.cc to its own file.
 public:
  TypeDeterminerHelper()
      : td_(std::make_unique<TypeDeterminer>(&ctx_, &mod_)) {}

  TypeDeterminer* td() const { return td_.get(); }
  ast::Module* mod() { return &mod_; }

 private:
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
};

class ValidateFunctionTest : public TypeDeterminerHelper,
                             public testing::Test {};

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> void { var a:i32 = 2; }

  ast::type::I32Type i32;
  auto var =
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &i32);
  var->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2)));

  ast::VariableList params;
  ast::type::VoidType void_type;
  auto func = std::make_unique<ast::Function>(Source{12, 34}, "func",
                                              std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.Validate(mod()));
  EXPECT_EQ(v.error(),
            "12:34: v-0002: function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
  // fn func -> void {}
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>(Source{12, 34}, "func",
                                              std::move(params), &void_type);
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.Validate(mod()));
  EXPECT_EQ(v.error(),
            "12:34: v-0002: function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_Pass) {
  // fn func -> void { return; }
  ast::type::VoidType void_type;
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_TRUE(v.Validate(mod())) << v.error();
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func -> void { return 2; }
  ast::type::VoidType void_type;
  ast::type::I32Type i32;
  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("func", std::move(params), &void_type);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(Source{12, 34},
                                                      std::move(return_expr)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.Validate(mod()));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34: v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__void'");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  ast::type::I32Type i32;
  ast::type::F32Type f32;
  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("func", std::move(params), &f32);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(Source{12, 34},
                                                      std::move(return_expr)));
  func->set_body(std::move(body));
  mod()->AddFunction(std::move(func));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.Validate(mod()));
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34: v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__f32'");
}

TEST_F(ValidateFunctionTest, FunctionNamesMustBeUnique_fail) {
  // fn func -> i32 { return 2; }
  // fn func -> i32 { return 2; }
  ast::type::VoidType void_type;
  ast::type::I32Type i32;

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("func", std::move(params), &i32);
  auto body = std::make_unique<ast::BlockStatement>();
  auto return_expr = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body->append(std::make_unique<ast::ReturnStatement>(std::move(return_expr)));
  func->set_body(std::move(body));

  ast::VariableList params_copy;
  auto func_copy = std::make_unique<ast::Function>(
      Source{12, 34}, "func", std::move(params_copy), &i32);
  auto body_copy = std::make_unique<ast::BlockStatement>();
  auto return_expr_copy = std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::SintLiteral>(&i32, 2));

  body_copy->append(
      std::make_unique<ast::ReturnStatement>(std::move(return_expr_copy)));
  func_copy->set_body(std::move(body_copy));

  mod()->AddFunction(std::move(func));
  mod()->AddFunction(std::move(func_copy));

  EXPECT_TRUE(td()->Determine()) << td()->error();
  tint::ValidatorImpl v;
  EXPECT_FALSE(v.Validate(mod()));
  EXPECT_EQ(v.error(), "12:34: v-0016: function names must be unique 'func'");
}

}  // namespace
}  // namespace tint
