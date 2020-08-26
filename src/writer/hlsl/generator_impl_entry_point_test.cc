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

#include "src/ast/assignment_statement.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/entry_point.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_EntryPoint = TestHelper;

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Vertex_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct vtx_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("vtx_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kVertex, "",
                                              "vtx_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(result(), R"(struct vtx_main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Vertex_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct vtx_main_out {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("vtx_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kVertex, "",
                                              "vtx_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(result(), R"(struct vtx_main_out {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Fragment_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct frag_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("frag_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kFragment,
                                              "main", "frag_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(result(), R"(struct main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Fragment_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct frag_main_out {
  //   float foo : SV_Target0;
  //   int bar : SV_Target1;
  // };

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("frag_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kFragment,
                                              "main", "frag_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(result(), R"(struct main_out {
  float foo : SV_Target0;
  int bar : SV_Target1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Compute_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // -> Error, not allowed

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("comp_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kCompute,
                                              "main", "comp_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_FALSE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(gen().error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("comp_main", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("foo"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("bar")));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kCompute,
                                              "main", "comp_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_FALSE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(gen().error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint, EmitEntryPointData_Builtins) {
  // [[builtin frag_coord]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_in {
  //   vector<float, 4> coord : SV_Position;
  // };
  //
  // struct main_out {
  //   float depth : SV_Depth;
  // };

  ast::type::F32Type f32;
  ast::type::VoidType void_type;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kInput, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragCoord));
  coord_var->set_decorations(std::move(decos));

  auto depth_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "depth", ast::StorageClass::kOutput, &f32));
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragDepth));
  depth_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());
  td().RegisterVariableForTesting(depth_var.get());

  mod()->AddGlobalVariable(std::move(coord_var));
  mod()->AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kFragment,
                                              "main", "frag_main");
  auto* ep_ptr = ep.get();

  mod()->AddEntryPoint(std::move(ep));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().EmitEntryPointData(out(), ep_ptr)) << gen().error();
  EXPECT_EQ(result(), R"(struct main_in {
  vector<float, 4> coord : SV_Position;
};

struct main_out {
  float depth : SV_Depth;
};

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
