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
#include "src/ast/binary_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/call_expression.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/set_decoration.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("my_func", ast::VariableList{},
                                              &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));
  gen().increment_indent();

  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(  void my_func() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("GeometryShader",
                                              ast::VariableList{}, &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));
  gen().increment_indent();

  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(  void GeometryShader_tint_0() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32));
  params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kNone, &i32));

  ast::type::VoidType void_type;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));
  gen().increment_indent();

  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOutVars) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct frag_main_in {
  float foo : TEXCOORD0;
};

struct frag_main_out {
  float bar : SV_Target1;
};

frag_main_out frag_main(frag_main_in tint_in) {
  frag_main_out tint_out;
  tint_out.bar = tint_in.foo;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOut_Builtins) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
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
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct frag_main_in {
  vector<float, 4> coord : SV_Position;
};

struct frag_main_out {
  float depth : SV_Depth;
};

frag_main_out frag_main(frag_main_in tint_in) {
  frag_main_out tint_out;
  tint_out.depth = tint_in.coord.x;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_Uniform) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kUniform, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("coord"),
      std::make_unique<ast::IdentifierExpression>("x")));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(cbuffer : register(b0) {
  vector<float, 4> coord;
};

void frag_main() {
  float v = coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_UniformStruct) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "coord", &vec4, ast::StructMemberDecorationList{}));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Uniforms", std::move(str));

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "uniforms", ast::StorageClass::kUniform, &s));

  mod()->AddConstructedType(&s);

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("uniforms"),
          std::make_unique<ast::IdentifierExpression>("coord")),
      std::make_unique<ast::IdentifierExpression>("x")));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct Uniforms {
  vector<float, 4> coord;
};

ConstantBuffer<Uniforms> uniforms : register(b0);

void frag_main() {
  float v = uniforms.coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer_Read) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("coord"),
      std::make_unique<ast::IdentifierExpression>("b")));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer_Read) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadOnly, &s);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());
  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::MemberAccessorExpression>(
      std::make_unique<ast::IdentifierExpression>("coord"),
      std::make_unique<ast::IdentifierExpression>("b")));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(ByteAddressBuffer coord : register(u0);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_StorageBuffer_Store) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(0));
  members.push_back(
      std::make_unique<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());

  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto assign = std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("b")),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 2.0f)));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::move(assign));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

void frag_main() {
  coord.Store(4, asuint(2.00000000f));
  return;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  auto foo_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  auto val_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("val", ast::StorageClass::kOutput, &f32));
  decos.push_back(std::make_unique<ast::LocationDecoration>(0));
  val_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(foo_var.get());
  td().RegisterVariableForTesting(bar_var.get());
  td().RegisterVariableForTesting(val_var.get());

  mod()->AddGlobalVariable(std::move(foo_var));
  mod()->AddGlobalVariable(std::move(bar_var));
  mod()->AddGlobalVariable(std::move(val_var));

  ast::VariableList params;
  params.push_back(std::make_unique<ast::Variable>(
      "param", ast::StorageClass::kFunction, &f32));
  auto sub_func =
      std::make_unique<ast::Function>("sub_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::IdentifierExpression>("foo")));
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("val"),
      std::make_unique<ast::IdentifierExpression>("param")));
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::IdentifierExpression>("foo")));
  sub_func->set_body(std::move(body));

  mod()->AddFunction(std::move(sub_func));

  auto func_1 =
      std::make_unique<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  ast::ExpressionList expr;
  expr.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("sub_func"),
          std::move(expr))));
  body->append(std::make_unique<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod()->AddFunction(std::move(func_1));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  float foo : TEXCOORD0;
};

struct ep_1_out {
  float bar : SV_Target1;
  float val : SV_Target0;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.bar = tint_in.foo;
  tint_out.val = param;
  return tint_in.foo;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out;
  tint_out.bar = sub_func_ep_1(tint_in, tint_out, 1.00000000f);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoints_NoUsedGlobals) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto depth_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "depth", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kFragDepth));
  depth_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(depth_var.get());

  mod()->AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  params.push_back(std::make_unique<ast::Variable>(
      "param", ast::StorageClass::kFunction, &f32));
  auto sub_func =
      std::make_unique<ast::Function>("sub_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::IdentifierExpression>("param")));
  sub_func->set_body(std::move(body));

  mod()->AddFunction(std::move(sub_func));

  auto func_1 =
      std::make_unique<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  ast::ExpressionList expr;
  expr.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("sub_func"),
          std::move(expr))));
  body->append(std::make_unique<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod()->AddFunction(std::move(func_1));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func(float param) {
  return param;
}

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.depth = sub_func(1.00000000f);
  return tint_out;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
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
  params.push_back(std::make_unique<ast::Variable>(
      "param", ast::StorageClass::kFunction, &f32));
  auto sub_func =
      std::make_unique<ast::Function>("sub_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::IdentifierExpression>("param")));
  sub_func->set_body(std::move(body));

  mod()->AddFunction(std::move(sub_func));

  auto func_1 =
      std::make_unique<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  ast::ExpressionList expr;
  expr.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("depth"),
      std::make_unique<ast::CallExpression>(
          std::make_unique<ast::IdentifierExpression>("sub_func"),
          std::move(expr))));
  body->append(std::make_unique<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod()->AddFunction(std::move(func_1));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct ep_1_in {
  vector<float, 4> coord : SV_Position;
};

struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func_ep_1(in ep_1_in tint_in, out ep_1_out tint_out, float param) {
  tint_out.depth = tint_in.coord.x;
  return param;
}

ep_1_out ep_1(ep_1_in tint_in) {
  ep_1_out tint_out;
  tint_out.depth = sub_func_ep_1(tint_in, tint_out, 1.00000000f);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kUniform, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());

  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  params.push_back(std::make_unique<ast::Variable>(
      "param", ast::StorageClass::kFunction, &f32));
  auto sub_func =
      std::make_unique<ast::Function>("sub_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  sub_func->set_body(std::move(body));

  mod()->AddFunction(std::move(sub_func));

  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  ast::ExpressionList expr;
  expr.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::CallExpression>(
      std::make_unique<ast::IdentifierExpression>("sub_func"),
      std::move(expr)));

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(cbuffer : register(b0) {
  vector<float, 4> coord;
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.00000000f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_StorageBuffer) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &vec4);
  auto coord_var =
      std::make_unique<ast::DecoratedVariable>(std::make_unique<ast::Variable>(
          "coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  coord_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(coord_var.get());

  mod()->AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  params.push_back(std::make_unique<ast::Variable>(
      "param", ast::StorageClass::kFunction, &f32));
  auto sub_func =
      std::make_unique<ast::Function>("sub_func", std::move(params), &f32);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>(
      std::make_unique<ast::MemberAccessorExpression>(
          std::make_unique<ast::IdentifierExpression>("coord"),
          std::make_unique<ast::IdentifierExpression>("x"))));
  sub_func->set_body(std::move(body));

  mod()->AddFunction(std::move(sub_func));

  auto func = std::make_unique<ast::Function>("frag_main", std::move(params),
                                              &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  ast::ExpressionList expr;
  expr.push_back(std::make_unique<ast::ScalarConstructorExpression>(
      std::make_unique<ast::FloatLiteral>(&f32, 1.0f)));

  auto var =
      std::make_unique<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(std::make_unique<ast::CallExpression>(
      std::make_unique<ast::IdentifierExpression>("sub_func"),
      std::move(expr)));

  body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::VariableDeclStatement>(std::move(var)));
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer coord : register(u0);

float sub_func(float param) {
  return asfloat(coord.Load((4 * 0)));
}

void frag_main() {
  float v = sub_func(1.00000000f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoints_WithGlobal_Nested_Return) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto bar_var = std::make_unique<ast::DecoratedVariable>(
      std::make_unique<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  ast::VariableDecorationList decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(1));
  bar_var->set_decorations(std::move(decos));

  td().RegisterVariableForTesting(bar_var.get());
  mod()->AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func_1 =
      std::make_unique<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::AssignmentStatement>(
      std::make_unique<ast::IdentifierExpression>("bar"),
      std::make_unique<ast::ScalarConstructorExpression>(
          std::make_unique<ast::FloatLiteral>(&f32, 1.0f))));

  auto list = std::make_unique<ast::BlockStatement>();
  list->append(std::make_unique<ast::ReturnStatement>());

  body->append(std::make_unique<ast::IfStatement>(
      std::make_unique<ast::BinaryExpression>(
          ast::BinaryOp::kEqual,
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 1)),
          std::make_unique<ast::ScalarConstructorExpression>(
              std::make_unique<ast::SintLiteral>(&i32, 1))),
      std::move(list)));

  body->append(std::make_unique<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod()->AddFunction(std::move(func_1));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float bar : SV_Target1;
};

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.bar = 1.00000000f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithNameCollision) {
  ast::type::VoidType void_type;

  auto func = std::make_unique<ast::Function>("GeometryShader",
                                              ast::VariableList{}, &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(void GeometryShader_tint_0() {
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute) {
  ast::type::VoidType void_type;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("main", std::move(params), &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute_WithWorkgroup) {
  ast::type::VoidType void_type;

  ast::VariableList params;
  auto func =
      std::make_unique<ast::Function>("main", std::move(params), &void_type);
  func->add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kCompute));
  func->add_decoration(std::make_unique<ast::WorkgroupDecoration>(2u, 4u, 6u));

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));

  ASSERT_TRUE(td().Determine()) << td().error();
  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &ary));

  ast::type::VoidType void_type;
  auto func =
      std::make_unique<ast::Function>("my_func", std::move(params), &void_type);

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod()->AddFunction(std::move(func));
  gen().increment_indent();

  ASSERT_TRUE(gen().Generate(out())) << gen().error();
  EXPECT_EQ(result(), R"(  void my_func(float a[5]) {
    return;
  }

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
