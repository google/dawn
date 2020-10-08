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

#include "src/inspector.h"

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/workgroup_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"

namespace tint {
namespace inspector {
namespace {

class InspectorHelper {
 public:
  InspectorHelper()
      : td_(std::make_unique<TypeDeterminer>(&ctx_, &mod_)),
        inspector_(std::make_unique<Inspector>(mod_)) {}

  /// Generates an empty function
  /// @param name name of the function created
  /// @returns a function object
  std::unique_ptr<ast::Function> GenerateEmptyBodyFunction(std::string name) {
    auto body = std::make_unique<ast::BlockStatement>();
    body->append(std::make_unique<ast::ReturnStatement>());
    std::unique_ptr<ast::Function> func =
        std::make_unique<ast::Function>(name, ast::VariableList(), void_type());
    func->set_body(std::move(body));
    return func;
  }

  /// Generates a function that calls another
  /// @param caller name of the function created
  /// @param callee name of the function to be called
  /// @returns a function object
  std::unique_ptr<ast::Function> GenerateCallerBodyFunction(
      std::string caller,
      std::string callee) {
    auto body = std::make_unique<ast::BlockStatement>();
    auto ident_expr = std::make_unique<ast::IdentifierExpression>(callee);
    auto call_expr = std::make_unique<ast::CallExpression>(
        std::move(ident_expr), ast::ExpressionList());
    body->append(std::make_unique<ast::CallStatement>(std::move(call_expr)));
    body->append(std::make_unique<ast::ReturnStatement>());
    std::unique_ptr<ast::Function> func = std::make_unique<ast::Function>(
        caller, ast::VariableList(), void_type());
    func->set_body(std::move(body));
    return func;
  }

  /// Add In/Out variables to the global variables
  /// @param inout_vars tuples of {in, out} that will be added as entries to the
  ///                   global variables
  void CreateInOutVariables(
      std::vector<std::tuple<std::string, std::string>> inout_vars) {
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      auto in_var = std::make_unique<ast::Variable>(
          in, ast::StorageClass::kInput, u32_type());
      auto out_var = std::make_unique<ast::Variable>(
          out, ast::StorageClass::kOutput, u32_type());
      mod()->AddGlobalVariable(std::move(in_var));
      mod()->AddGlobalVariable(std::move(out_var));
    }
  }

  /// Generates a function that references in/out variables
  /// @param name name of the function created
  /// @param inout_vars tuples of {in, out} that will be converted into out = in
  ///                   calls in the function body
  /// @returns a function object
  std::unique_ptr<ast::Function> GenerateInOutVariableBodyFunction(
      std::string name,
      std::vector<std::tuple<std::string, std::string>> inout_vars) {
    auto body = std::make_unique<ast::BlockStatement>();
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      body->append(std::make_unique<ast::AssignmentStatement>(
          std::make_unique<ast::IdentifierExpression>(out),
          std::make_unique<ast::IdentifierExpression>(in)));
    }
    body->append(std::make_unique<ast::ReturnStatement>());
    auto func =
        std::make_unique<ast::Function>(name, ast::VariableList(), void_type());
    func->set_body(std::move(body));
    return func;
  }

  /// Generates a function that references in/out variables and calls another
  /// function.
  /// @param caller name of the function created
  /// @param callee name of the function to be called
  /// @param inout_vars tuples of {in, out} that will be converted into out = in
  ///                   calls in the function body
  /// @returns a function object
  std::unique_ptr<ast::Function> GenerateInOutVariableCallerBodyFunction(
      std::string caller,
      std::string callee,
      std::vector<std::tuple<std::string, std::string>> inout_vars) {
    auto body = std::make_unique<ast::BlockStatement>();
    for (auto inout : inout_vars) {
      std::string in, out;
      std::tie(in, out) = inout;
      body->append(std::make_unique<ast::AssignmentStatement>(
          std::make_unique<ast::IdentifierExpression>(out),
          std::make_unique<ast::IdentifierExpression>(in)));
    }
    auto ident_expr = std::make_unique<ast::IdentifierExpression>(callee);
    auto call_expr = std::make_unique<ast::CallExpression>(
        std::move(ident_expr), ast::ExpressionList());
    body->append(std::make_unique<ast::CallStatement>(std::move(call_expr)));
    body->append(std::make_unique<ast::ReturnStatement>());
    auto func = std::make_unique<ast::Function>(caller, ast::VariableList(),
                                                void_type());
    func->set_body(std::move(body));
    return func;
  }

  bool ContainsString(const std::vector<std::string>& vec,
                      const std::string& str) {
    for (auto& s : vec) {
      if (s == str) {
        return true;
      }
    }
    return false;
  }

  ast::Module* mod() { return &mod_; }
  TypeDeterminer* td() { return td_.get(); }
  Inspector* inspector() { return inspector_.get(); }

  ast::type::VoidType* void_type() { return &void_type_; }
  ast::type::U32Type* u32_type() { return &u32_type_; }

 private:
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
  std::unique_ptr<Inspector> inspector_;

  ast::type::VoidType void_type_;
  ast::type::U32Type u32_type_;
};

class InspectorTest : public InspectorHelper, public testing::Test {};

class InspectorGetEntryPointTest : public InspectorTest {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  mod()->AddFunction(GenerateEmptyBodyFunction("foo"));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  auto foo = GenerateEmptyBodyFunction("foo");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
  auto foo = GenerateEmptyBodyFunction("foo");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = GenerateEmptyBodyFunction("bar");
  bar->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  mod()->AddFunction(std::move(bar));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
  auto func = GenerateEmptyBodyFunction("func");
  mod()->AddFunction(std::move(func));

  auto foo = GenerateCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = GenerateCallerBodyFunction("bar", "func");
  bar->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  mod()->AddFunction(std::move(bar));

  auto result = inspector()->GetEntryPoints();
  EXPECT_FALSE(inspector()->has_error());

  ASSERT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ(ast::PipelineStage::kFragment, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, DefaultWorkgroupSize) {
  auto foo = GenerateCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(1u, x);
  EXPECT_EQ(1u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NonDefaultWorkgroupSize) {
  auto foo = GenerateEmptyBodyFunction("foo");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  foo->add_decoration(std::make_unique<ast::WorkgroupDecoration>(8u, 2u, 1u));
  mod()->AddFunction(std::move(foo));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());
  uint32_t x, y, z;
  std::tie(x, y, z) = result[0].workgroup_size();
  EXPECT_EQ(8u, x);
  EXPECT_EQ(2u, y);
  EXPECT_EQ(1u, z);
}

TEST_F(InspectorGetEntryPointTest, NoInOutVariables) {
  auto func = GenerateEmptyBodyFunction("func");
  mod()->AddFunction(std::move(func));

  auto foo = GenerateCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());
  EXPECT_EQ(0u, result[0].input_variables.size());
  EXPECT_EQ(0u, result[0].output_variables.size());
}

TEST_F(InspectorGetEntryPointTest, EntryPointInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}});

  auto foo = GenerateInOutVariableBodyFunction("foo", {{"in_var", "out_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, FunctionInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}});

  auto func =
      GenerateInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = GenerateCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, RepeatedInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}});

  auto func =
      GenerateInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = GenerateInOutVariableCallerBodyFunction("foo", "func",
                                                     {{"in_var", "out_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out_var", result[0].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, EntryPointMultipleInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto foo = GenerateInOutVariableBodyFunction(
      "foo", {{"in_var", "out_var"}, {"in2_var", "out2_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));
}

TEST_F(InspectorGetEntryPointTest, FunctionMultipleInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto func = GenerateInOutVariableBodyFunction(
      "func", {{"in_var", "out_var"}, {"in2_var", "out2_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = GenerateCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(1u, result.size());

  ASSERT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  ASSERT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto foo = GenerateInOutVariableBodyFunction("foo", {{"in_var", "out2_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = GenerateInOutVariableBodyFunction("bar", {{"in2_var", "out_var"}});
  bar->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  mod()->AddFunction(std::move(bar));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  ASSERT_EQ(1u, result[0].input_variables.size());
  EXPECT_EQ("in_var", result[0].input_variables[0]);
  ASSERT_EQ(1u, result[0].output_variables.size());
  EXPECT_EQ("out2_var", result[0].output_variables[0]);

  ASSERT_EQ("bar", result[1].name);
  ASSERT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0]);
  ASSERT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out_var", result[1].output_variables[0]);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPointsSharedInOutVariables) {
  CreateInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto func =
      GenerateInOutVariableBodyFunction("func", {{"in2_var", "out2_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = GenerateInOutVariableCallerBodyFunction("foo", "func",
                                                     {{"in_var", "out_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = GenerateCallerBodyFunction("bar", "func");
  bar->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  mod()->AddFunction(std::move(bar));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  ASSERT_EQ(2u, result.size());

  ASSERT_EQ("foo", result[0].name);
  EXPECT_EQ(2u, result[0].input_variables.size());
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in_var"));
  EXPECT_TRUE(ContainsString(result[0].input_variables, "in2_var"));
  EXPECT_EQ(2u, result[0].output_variables.size());
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out_var"));
  EXPECT_TRUE(ContainsString(result[0].output_variables, "out2_var"));

  ASSERT_EQ("bar", result[1].name);
  EXPECT_EQ(1u, result[1].input_variables.size());
  EXPECT_EQ("in2_var", result[1].input_variables[0]);
  EXPECT_EQ(1u, result[1].output_variables.size());
  EXPECT_EQ("out2_var", result[1].output_variables[0]);
}

}  // namespace
}  // namespace inspector
}  // namespace tint
