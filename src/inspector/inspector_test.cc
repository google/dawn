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
#include "src/ast/member_accessor_expression.h"
#include "src/ast/null_literal.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
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
  std::unique_ptr<ast::Function> MakeEmptyBodyFunction(std::string name) {
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
  std::unique_ptr<ast::Function> MakeCallerBodyFunction(std::string caller,
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
  void AddInOutVariables(
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
  std::unique_ptr<ast::Function> MakeInOutVariableBodyFunction(
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
  std::unique_ptr<ast::Function> MakeInOutVariableCallerBodyFunction(
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
  void AddConstantID(std::string name,
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

  /// Generates an ast::Literal for the given value
  /// @tparam T C++ type of the literal, must agree with type
  /// @returns a Literal of the expected type and value
  template <class T>
  std::unique_ptr<ast::Literal> MakeLiteral(ast::type::Type*, T*) {
    return nullptr;
  }

  /// @param type AST type of the literal, must resolve to BoolLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<bool>(ast::type::Type* type,
                                                  bool* val) {
    return std::make_unique<ast::BoolLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to UIntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<uint32_t>(ast::type::Type* type,
                                                      uint32_t* val) {
    return std::make_unique<ast::UintLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to IntLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<int32_t>(ast::type::Type* type,
                                                     int32_t* val) {
    return std::make_unique<ast::SintLiteral>(type, *val);
  }

  /// @param type AST type of the literal, must resolve to FloattLiteral
  /// @param val scalar value for the literal to contain
  /// @returns a Literal of the expected type and value
  template <>
  std::unique_ptr<ast::Literal> MakeLiteral<float>(ast::type::Type* type,
                                                   float* val) {
    return std::make_unique<ast::FloatLiteral>(type, *val);
  }

  /// @param vec Vector of strings to be searched
  /// @param str String to be searching for
  /// @returns true if str is in vec, otherwise false
  bool ContainsString(const std::vector<std::string>& vec,
                      const std::string& str) {
    for (auto& s : vec) {
      if (s == str) {
        return true;
      }
    }
    return false;
  }

  /// Builds a string for accessing a member in a generated struct
  /// @param idx index of member
  /// @param type type of member
  /// @returns a string for the member
  std::string StructMemberName(size_t idx, ast::type::Type* type) {
    return std::to_string(idx) + type->type_name();
  }

  /// Generates a struct type appropriate for using in an uniform buffer
  /// @param name name for the type
  /// @param members_info a vector of {type, offset} where each entry is the
  ///                     type and offset of a member of the struct
  /// @returns a struct type suitable to use for an uniform buffer
  std::unique_ptr<ast::type::StructType> MakeUniformBufferStructType(
      const std::string& name,
      std::vector<std::tuple<ast::type::Type*, uint32_t>> members_info) {
    ast::StructMemberList members;
    for (auto& member_info : members_info) {
      ast::type::Type* type;
      uint32_t offset;
      std::tie(type, offset) = member_info;

      ast::StructMemberDecorationList deco;
      deco.push_back(
          std::make_unique<ast::StructMemberOffsetDecoration>(offset));

      members.push_back(std::make_unique<ast::StructMember>(
          StructMemberName(members.size(), type), type, std::move(deco)));
    }
    ast::StructDecorationList decos;
    decos.push_back(ast::StructDecoration::kBlock);

    auto str =
        std::make_unique<ast::Struct>(std::move(decos), std::move(members));

    return std::make_unique<ast::type::StructType>(name, std::move(str));
  }

  /// Adds an uniform buffer variable to the module
  /// @param name the name of the variable
  /// @param struct_type the type to use
  /// @param set the binding group/set to use for the uniform buffer
  /// @param binding the binding number to use for the uniform buffer
  void AddUniformBuffer(const std::string& name,
                        ast::type::StructType* struct_type,
                        uint32_t set,
                        uint32_t binding) {
    auto var = std::make_unique<ast::DecoratedVariable>(
        std::make_unique<ast::Variable>(name, ast::StorageClass::kUniform,
                                        struct_type));
    ast::VariableDecorationList decorations;

    decorations.push_back(std::make_unique<ast::BindingDecoration>(binding));
    decorations.push_back(std::make_unique<ast::SetDecoration>(set));
    var->set_decorations(std::move(decorations));

    mod()->AddGlobalVariable(std::move(var));
  }

  /// Generates a function that references a specific uniform buffer
  /// @param func_name name of the function created
  /// @param ub_name name of the uniform buffer to be accessed
  /// @param members list of members to access, by index and type
  /// @returns a function that references all of the ub members specified
  std::unique_ptr<ast::Function> MakeUniformBufferReferenceBodyFunction(
      std::string func_name,
      std::string ub_name,
      std::vector<std::tuple<size_t, ast::type::Type*>> members) {
    auto body = std::make_unique<ast::BlockStatement>();

    for (auto member : members) {
      size_t member_idx;
      ast::type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);
      body->append(std::make_unique<ast::VariableDeclStatement>(
          std::make_unique<ast::Variable>(
              "local" + member_name, ast::StorageClass::kNone, member_type)));
    }

    for (auto member : members) {
      size_t member_idx;
      ast::type::Type* member_type;
      std::tie(member_idx, member_type) = member;
      std::string member_name = StructMemberName(member_idx, member_type);
      body->append(std::make_unique<ast::AssignmentStatement>(
          std::make_unique<ast::IdentifierExpression>("local" + member_name),
          std::make_unique<ast::MemberAccessorExpression>(
              std::make_unique<ast::IdentifierExpression>(ub_name),
              std::make_unique<ast::IdentifierExpression>(member_name))));
    }

    body->append(std::make_unique<ast::ReturnStatement>());
    auto func = std::make_unique<ast::Function>(func_name, ast::VariableList(),
                                                void_type());
    func->set_body(std::move(body));
    return func;
  }

  ast::Module* mod() { return &mod_; }
  TypeDeterminer* td() { return td_.get(); }
  Inspector* inspector() { return inspector_.get(); }

  ast::type::BoolType* bool_type() { return &bool_type_; }
  ast::type::F32Type* f32_type() { return &f32_type_; }
  ast::type::I32Type* i32_type() { return &i32_type_; }
  ast::type::U32Type* u32_type() { return &u32_type_; }
  ast::type::ArrayType* u32_array_type(uint32_t count) {
    if (array_type_memo_.find(count) == array_type_memo_.end()) {
      array_type_memo_[count] =
          std::make_unique<ast::type::ArrayType>(u32_type(), count);
    }
    return array_type_memo_[count].get();
  }
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
  std::map<uint32_t, std::unique_ptr<ast::type::ArrayType>> array_type_memo_;
};

class InspectorTest : public InspectorHelper, public testing::Test {};

class InspectorGetEntryPointTest : public InspectorTest {};
class InspectorGetConstantIDsTest : public InspectorTest {};
class InspectorGetUniformBufferResourceBindings : public InspectorTest {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  mod()->AddFunction(MakeEmptyBodyFunction("foo"));

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  auto foo = MakeEmptyBodyFunction("foo");
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
  auto foo = MakeEmptyBodyFunction("foo");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = MakeEmptyBodyFunction("bar");
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
  auto func = MakeEmptyBodyFunction("func");
  mod()->AddFunction(std::move(func));

  auto foo = MakeCallerBodyFunction("foo", "func");
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = MakeCallerBodyFunction("bar", "func");
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
  auto foo = MakeCallerBodyFunction("foo", "func");
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
  auto foo = MakeEmptyBodyFunction("foo");
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
  auto func = MakeEmptyBodyFunction("func");
  mod()->AddFunction(std::move(func));

  auto foo = MakeCallerBodyFunction("foo", "func");
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
  AddInOutVariables({{"in_var", "out_var"}});

  auto foo = MakeInOutVariableBodyFunction("foo", {{"in_var", "out_var"}});
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
  AddInOutVariables({{"in_var", "out_var"}});

  auto func = MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = MakeCallerBodyFunction("foo", "func");
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
  AddInOutVariables({{"in_var", "out_var"}});

  auto func = MakeInOutVariableBodyFunction("func", {{"in_var", "out_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = MakeInOutVariableCallerBodyFunction("foo", "func",
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
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto foo = MakeInOutVariableBodyFunction(
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
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto func = MakeInOutVariableBodyFunction(
      "func", {{"in_var", "out_var"}, {"in2_var", "out2_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = MakeCallerBodyFunction("foo", "func");
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
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto foo = MakeInOutVariableBodyFunction("foo", {{"in_var", "out2_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = MakeInOutVariableBodyFunction("bar", {{"in2_var", "out_var"}});
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
  AddInOutVariables({{"in_var", "out_var"}, {"in2_var", "out2_var"}});

  auto func = MakeInOutVariableBodyFunction("func", {{"in2_var", "out2_var"}});
  mod()->AddFunction(std::move(func));

  auto foo = MakeInOutVariableCallerBodyFunction("foo", "func",
                                                 {{"in_var", "out_var"}});
  foo->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(foo));

  auto bar = MakeCallerBodyFunction("bar", "func");
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

TEST_F(InspectorGetConstantIDsTest, Bool) {
  bool val_true = true;
  bool val_false = false;
  AddConstantID<bool>("foo", 1, bool_type(), nullptr);
  AddConstantID<bool>("bar", 20, bool_type(), &val_true);
  AddConstantID<bool>("baz", 300, bool_type(), &val_false);

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

TEST_F(InspectorGetConstantIDsTest, U32) {
  uint32_t val = 42;
  AddConstantID<uint32_t>("foo", 1, u32_type(), nullptr);
  AddConstantID<uint32_t>("bar", 20, u32_type(), &val);

  auto result = inspector()->GetConstantIDs();
  ASSERT_EQ(2u, result.size());

  ASSERT_TRUE(result.find(1) != result.end());
  EXPECT_TRUE(result[1].IsNull());

  ASSERT_TRUE(result.find(20) != result.end());
  EXPECT_TRUE(result[20].IsU32());
  EXPECT_EQ(42u, result[20].AsU32());
}

TEST_F(InspectorGetConstantIDsTest, I32) {
  int32_t val_neg = -42;
  int32_t val_pos = 42;
  AddConstantID<int32_t>("foo", 1, i32_type(), nullptr);
  AddConstantID<int32_t>("bar", 20, i32_type(), &val_neg);
  AddConstantID<int32_t>("baz", 300, i32_type(), &val_pos);

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

TEST_F(InspectorGetConstantIDsTest, Float) {
  float val_zero = 0.0f;
  float val_neg = -10.0f;
  float val_pos = 15.0f;
  AddConstantID<float>("foo", 1, f32_type(), nullptr);
  AddConstantID<float>("bar", 20, f32_type(), &val_zero);
  AddConstantID<float>("baz", 300, f32_type(), &val_neg);
  AddConstantID<float>("x", 4000, f32_type(), &val_pos);

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

TEST_F(InspectorGetUniformBufferResourceBindings, MissingEntryPoint) {
  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_TRUE(inspector()->has_error());
  std::string error = inspector()->error();
  EXPECT_TRUE(error.find("not found") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindings, NonEntryPointFunc) {
  auto foo_type = MakeUniformBufferStructType("foo_type", {{i32_type(), 0}});
  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto ub_func = MakeUniformBufferReferenceBodyFunction("ub_func", "foo_ub",
                                                        {{0, i32_type()}});
  mod()->AddFunction(std::move(ub_func));

  auto ep_func = MakeCallerBodyFunction("ep_func", "ub_func");
  ep_func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(ep_func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ub_func");
  std::string error = inspector()->error();
  EXPECT_TRUE(error.find("not an entry point") != std::string::npos);
}

TEST_F(InspectorGetUniformBufferResourceBindings, MissingBlockDeco) {
  ast::StructMemberList members;
  ast::StructMemberDecorationList deco;
  deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));

  members.push_back(std::make_unique<ast::StructMember>(
      StructMemberName(members.size(), i32_type()), i32_type(),
      std::move(deco)));

  ast::StructDecorationList decos;

  auto str =
      std::make_unique<ast::Struct>(std::move(decos), std::move(members));
  auto foo_type =
      std::make_unique<ast::type::StructType>("foo_type", std::move(str));

  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto ub_func = MakeUniformBufferReferenceBodyFunction("ub_func", "foo_ub",
                                                        {{0, i32_type()}});
  mod()->AddFunction(std::move(ub_func));

  auto ep_func = MakeCallerBodyFunction("ep_func", "ub_func");
  ep_func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(ep_func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error());
  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetUniformBufferResourceBindings, Simple) {
  auto foo_type = MakeUniformBufferStructType("foo_type", {{i32_type(), 0}});
  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto ub_func = MakeUniformBufferReferenceBodyFunction("ub_func", "foo_ub",
                                                        {{0, i32_type()}});
  mod()->AddFunction(std::move(ub_func));

  auto ep_func = MakeCallerBodyFunction("ep_func", "ub_func");
  ep_func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(ep_func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error());
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(4u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindings, MultipleMembers) {
  auto foo_type = MakeUniformBufferStructType(
      "foo_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto ub_func = MakeUniformBufferReferenceBodyFunction(
      "ub_func", "foo_ub", {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
  mod()->AddFunction(std::move(ub_func));

  auto ep_func = MakeCallerBodyFunction("ep_func", "ub_func");
  ep_func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(ep_func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error());
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindings, MultipleUniformBufferS) {
  auto ub_type = MakeUniformBufferStructType(
      "ub_type", {{i32_type(), 0}, {u32_type(), 4}, {f32_type(), 8}});
  AddUniformBuffer("ub_foo", ub_type.get(), 0, 0);
  AddUniformBuffer("ub_bar", ub_type.get(), 0, 1);
  AddUniformBuffer("ub_baz", ub_type.get(), 2, 0);

  auto AddReferenceFunc = [this](const std::string& func_name,
                                 const std::string& var_name) {
    auto ub_func = MakeUniformBufferReferenceBodyFunction(
        func_name, var_name,
        {{0, i32_type()}, {1, u32_type()}, {2, f32_type()}});
    mod()->AddFunction(std::move(ub_func));
  };
  AddReferenceFunc("ub_foo_func", "ub_foo");
  AddReferenceFunc("ub_bar_func", "ub_bar");
  AddReferenceFunc("ub_baz_func", "ub_baz");

  auto AddFuncCall = [](ast::BlockStatement* body, const std::string& callee) {
    auto ident_expr = std::make_unique<ast::IdentifierExpression>(callee);
    auto call_expr = std::make_unique<ast::CallExpression>(
        std::move(ident_expr), ast::ExpressionList());
    body->append(std::make_unique<ast::CallStatement>(std::move(call_expr)));
  };
  auto body = std::make_unique<ast::BlockStatement>();

  AddFuncCall(body.get(), "ub_foo_func");
  AddFuncCall(body.get(), "ub_bar_func");
  AddFuncCall(body.get(), "ub_baz_func");

  body->append(std::make_unique<ast::ReturnStatement>());
  std::unique_ptr<ast::Function> func = std::make_unique<ast::Function>(
      "ep_func", ast::VariableList(), void_type());
  func->set_body(std::move(body));

  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error());
  ASSERT_EQ(3u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(12u, result[0].min_buffer_binding_size);

  EXPECT_EQ(0u, result[1].bind_group);
  EXPECT_EQ(1u, result[1].binding);
  EXPECT_EQ(12u, result[1].min_buffer_binding_size);

  EXPECT_EQ(2u, result[2].bind_group);
  EXPECT_EQ(0u, result[2].binding);
  EXPECT_EQ(12u, result[2].min_buffer_binding_size);
}

TEST_F(InspectorGetUniformBufferResourceBindings, ContainingArray) {
  auto foo_type = MakeUniformBufferStructType(
      "foo_type", {{i32_type(), 0}, {u32_array_type(4), 4}});
  AddUniformBuffer("foo_ub", foo_type.get(), 0, 0);

  auto ub_func = MakeUniformBufferReferenceBodyFunction("ub_func", "foo_ub",
                                                        {{0, i32_type()}});
  mod()->AddFunction(std::move(ub_func));

  auto ep_func = MakeCallerBodyFunction("ep_func", "ub_func");
  ep_func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kVertex));
  mod()->AddFunction(std::move(ep_func));

  ASSERT_TRUE(td()->Determine()) << td()->error();

  auto result = inspector()->GetUniformBufferResourceBindings("ep_func");
  ASSERT_FALSE(inspector()->has_error());
  ASSERT_EQ(1u, result.size());

  EXPECT_EQ(0u, result[0].bind_group);
  EXPECT_EQ(0u, result[0].binding);
  EXPECT_EQ(20u, result[0].min_buffer_binding_size);
}

}  // namespace
}  // namespace inspector
}  // namespace tint
