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

#include "src/inspector/inspector.h"

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/bool_literal.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/constant_id_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/null_literal.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/uint_literal.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/variable_decoration.h"
#include "src/ast/workgroup_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"

#include "tint/tint.h"

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

  /// Add a Constant ID to the global variables.
  /// @param name name of the variable to add
  /// @param id id number for the constant id
  /// @param type type of the variable
  /// @param val value to initialize the variable with, if NULL no initializer
  ///            will be added.
  template <class T>
  void CreateConstantID(std::string name,
                        uint32_t id,
                        ast::type::Type* type,
                        T* val) {
    auto dvar = std::make_unique<ast::DecoratedVariable>(
        std::make_unique<ast::Variable>(name, ast::StorageClass::kNone, type));
    dvar->set_is_const(true);
    ast::VariableDecorationList decos;
    decos.push_back(std::make_unique<ast::ConstantIdDecoration>(id));
    dvar->set_decorations(std::move(decos));
    if (val) {
      dvar->set_constructor(std::make_unique<ast::ScalarConstructorExpression>(
          MakeLiteral<T>(type, val)));
    }
    mod()->AddGlobalVariable(std::move(dvar));
  }

  template <class T>
  std::unique_ptr<ast::Literal> MakeLiteral(ast::type::Type*, T*) {
    return nullptr;
  }

  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<bool>(ast::type::Type* type,
                                                  bool* val) {
    return std::make_unique<ast::BoolLiteral>(type, *val);
  }

  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<uint32_t>(ast::type::Type* type,
                                                      uint32_t* val) {
    return std::make_unique<ast::UintLiteral>(type, *val);
  }

  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<int32_t>(ast::type::Type* type,
                                                     int32_t* val) {
    return std::make_unique<ast::SintLiteral>(type, *val);
  }

  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<float>(ast::type::Type* type,
                                                   float* val) {
    return std::make_unique<ast::FloatLiteral>(type, *val);
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

  ast::type::BoolType* bool_type() { return &bool_type_; }
  ast::type::F32Type* f32_type() { return &f32_type_; }
  ast::type::I32Type* i32_type() { return &i32_type_; }
  ast::type::U32Type* u32_type() { return &u32_type_; }
  ast::type::VoidType* void_type() { return &void_type_; }

 private:
  Context ctx_;
  ast::Module mod_;
  std::unique_ptr<TypeDeterminer> td_;
  std::unique_ptr<Inspector> inspector_;

  ast::type::BoolType bool_type_;
  ast::type::F32Type f32_type_;
  ast::type::I32Type i32_type_;
  ast::type::U32Type u32_type_;
  ast::type::VoidType void_type_;
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

TEST_F(InspectorGetEntryPointTest, BoolConstantIDs) {
  bool val_true = true;
  bool val_false = false;
  CreateConstantID<bool>("foo", 1, bool_type(), nullptr);
  CreateConstantID<bool>("bar", 20, bool_type(), &val_true);
  CreateConstantID<bool>("baz", 300, bool_type(), &val_false);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsBool());
  EXPECT_TRUE(result[20].AsBool());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsBool());
  EXPECT_FALSE(result[300].AsBool());
}

TEST_F(InspectorGetEntryPointTest, U32ConstantIDs) {
  uint32_t val = 42;
  CreateConstantID<uint32_t>("foo", 1, u32_type(), nullptr);
  CreateConstantID<uint32_t>("bar", 20, u32_type(), &val);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(2u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsU32());
  EXPECT_EQ(42u, result[20].AsU32());
}

TEST_F(InspectorGetEntryPointTest, I32ConstantIDs) {
  int32_t val_neg = -42;
  int32_t val_pos = 42;
  CreateConstantID<int32_t>("foo", 1, i32_type(), nullptr);
  CreateConstantID<int32_t>("bar", 20, i32_type(), &val_neg);
  CreateConstantID<int32_t>("baz", 300, i32_type(), &val_pos);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(3u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsI32());
  EXPECT_EQ(-42, result[20].AsI32());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsI32());
  EXPECT_EQ(42, result[300].AsI32());
}

TEST_F(InspectorGetEntryPointTest, FloatConstantIDs) {
  float val_zero = 0.0f;
  float val_neg = -10.0f;
  float val_pos = 15.0f;
  CreateConstantID<float>("foo", 1, f32_type(), nullptr);
  CreateConstantID<float>("bar", 20, f32_type(), &val_zero);
  CreateConstantID<float>("baz", 300, f32_type(), &val_neg);
  CreateConstantID<float>("x", 4000, f32_type(), &val_pos);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(4u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsFloat());
  EXPECT_FLOAT_EQ(0.0, result[20].AsFloat());

  ASSERT_TRUE(result.find(300) != result.end());
  EXPECT_TRUE(result[300].IsFloat());
  EXPECT_FLOAT_EQ(-10.0, result[300].AsFloat());

  ASSERT_TRUE(result.find(4000) != result.end());
  EXPECT_TRUE(result[4000].IsFloat());
  EXPECT_FLOAT_EQ(15.0, result[4000].AsFloat());
}

}  // namespace
}  // namespace inspector
}  // namespace tint
