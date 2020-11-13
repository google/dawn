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

#include "gtest/gtest.h"
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
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
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
#include "src/context.h"
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Function) {
  ast::type::VoidType void_type;

  auto func = create<ast::Function>("my_func", ast::VariableList{}, &void_type);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_Name_Collision) {
  ast::type::VoidType void_type;

  auto func = create<ast::Function>("main", ast::VariableList{}, &void_type);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void main_tint_0() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithParams) {
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::VariableList params;
  params.push_back(create<ast::Variable>("a", ast::StorageClass::kNone, &f32));
  params.push_back(create<ast::Variable>("b", ast::StorageClass::kNone, &i32));

  ast::type::VoidType void_type;
  auto func = create<ast::Function>("my_func", std::move(params), &void_type);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionDecoration_EntryPoint_WithInOutVars) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct frag_main_in {
  float foo [[user(locn0)]];
};

struct frag_main_out {
  float bar [[color(1)]];
};

fragment frag_main_out frag_main(frag_main_in tint_in [[stage_in]]) {
  frag_main_out tint_out = {};
  tint_out.bar = tint_in.foo;
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_WithInOut_Builtins) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kInput, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord, Source{}));
  coord_var->set_decorations(std::move(decos));

  auto depth_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("depth", ast::StorageClass::kOutput, &f32));
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}));
  depth_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());
  td.RegisterVariableForTesting(depth_var.get());

  mod.AddGlobalVariable(std::move(coord_var));
  mod.AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("depth"),
      create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("x"))));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct frag_main_out {
  float depth [[depth(any)]];
};

fragment frag_main_out frag_main(float4 coord [[position]]) {
  frag_main_out tint_out = {};
  tint_out.depth = coord.x;
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionDecoration_EntryPoint_With_Uniform) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kUniform, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());

  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::MemberAccessorExpression>(
      create<ast::IdentifierExpression>("coord"),
      create<ast::IdentifierExpression>("x")));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

fragment void frag_main(constant float4& coord [[buffer(0)]]) {
  float v = coord.x;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = create<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  mod.AddConstructedType(&s);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());

  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::MemberAccessorExpression>(
      create<ast::IdentifierExpression>("coord"),
      create<ast::IdentifierExpression>("b")));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct Data {
  int a;
  float b;
};

fragment void frag_main(device Data& coord [[buffer(0)]]) {
  float v = coord.b;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = create<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadOnly, &s);

  mod.AddConstructedType(&s);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());

  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::MemberAccessorExpression>(
      create<ast::IdentifierExpression>("coord"),
      create<ast::IdentifierExpression>("b")));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct Data {
  int a;
  float b;
};

fragment void frag_main(const device Data& coord [[buffer(0)]]) {
  float v = coord.b;
  return;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  auto val_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("val", ast::StorageClass::kOutput, &f32));
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  val_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());
  td.RegisterVariableForTesting(val_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));
  mod.AddGlobalVariable(std::move(val_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("val"),
      create<ast::IdentifierExpression>("param")));
  body->append(
      create<ast::ReturnStatement>(create<ast::IdentifierExpression>("foo")));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func_1 = create<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::CallExpression>(create<ast::IdentifierExpression>("sub_func"),
                                  std::move(expr))));
  body->append(create<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod.AddFunction(std::move(func_1));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct ep_1_in {
  float foo [[user(locn0)]];
};

struct ep_1_out {
  float bar [[color(1)]];
  float val [[color(0)]];
};

float sub_func_ep_1(thread ep_1_in& tint_in, thread ep_1_out& tint_out, float param) {
  tint_out.bar = tint_in.foo;
  tint_out.val = param;
  return tint_in.foo;
}

fragment ep_1_out ep_1(ep_1_in tint_in [[stage_in]]) {
  ep_1_out tint_out = {};
  tint_out.bar = sub_func_ep_1(tint_in, tint_out, 1.00000000f);
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoints_NoUsedGlobals) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto depth_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("depth", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}));
  depth_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(depth_var.get());

  mod.AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::IdentifierExpression>("param")));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func_1 = create<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("depth"),
      create<ast::CallExpression>(create<ast::IdentifierExpression>("sub_func"),
                                  std::move(expr))));
  body->append(create<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod.AddFunction(std::move(func_1));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct ep_1_out {
  float depth [[depth(any)]];
};

float sub_func(float param) {
  return param;
}

fragment ep_1_out ep_1() {
  ep_1_out tint_out = {};
  tint_out.depth = sub_func(1.00000000f);
  return tint_out;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kInput, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord, Source{}));
  coord_var->set_decorations(std::move(decos));

  auto depth_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("depth", ast::StorageClass::kOutput, &f32));
  decos.push_back(
      create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth, Source{}));
  depth_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());
  td.RegisterVariableForTesting(depth_var.get());

  mod.AddGlobalVariable(std::move(coord_var));
  mod.AddGlobalVariable(std::move(depth_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("depth"),
      create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("x"))));
  body->append(
      create<ast::ReturnStatement>(create<ast::IdentifierExpression>("param")));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func_1 = create<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("depth"),
      create<ast::CallExpression>(create<ast::IdentifierExpression>("sub_func"),
                                  std::move(expr))));
  body->append(create<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod.AddFunction(std::move(func_1));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct ep_1_out {
  float depth [[depth(any)]];
};

float sub_func_ep_1(thread ep_1_out& tint_out, thread float4& coord, float param) {
  tint_out.depth = coord.x;
  return param;
}

fragment ep_1_out ep_1(float4 coord [[position]]) {
  ep_1_out tint_out = {};
  tint_out.depth = sub_func_ep_1(tint_out, coord, 1.00000000f);
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::VectorType vec4(&f32, 4);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kUniform, &vec4));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());

  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("x"))));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::CallExpression>(
      create<ast::IdentifierExpression>("sub_func"), std::move(expr)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

float sub_func(constant float4& coord, float param) {
  return coord.x;
}

fragment void frag_main(constant float4& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.00000000f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RW_StorageBuffer) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = create<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  mod.AddConstructedType(&s);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());
  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("b"))));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::CallExpression>(
      create<ast::IdentifierExpression>("sub_func"), std::move(expr)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct Data {
  int a;
  float b;
};

float sub_func(device Data& coord, float param) {
  return coord.b;
}

fragment void frag_main(device Data& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.00000000f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RO_StorageBuffer) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("a", &i32, std::move(a_deco)));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = create<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadOnly, &s);

  mod.AddConstructedType(&s);

  auto coord_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("coord", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(1, Source{}));
  coord_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(coord_var.get());
  mod.AddGlobalVariable(std::move(coord_var));

  ast::VariableList params;
  params.push_back(
      create<ast::Variable>("param", ast::StorageClass::kFunction, &f32));
  auto sub_func = create<ast::Function>("sub_func", std::move(params), &f32);

  auto body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("b"))));
  sub_func->set_body(std::move(body));

  mod.AddFunction(std::move(sub_func));

  auto func = create<ast::Function>("frag_main", std::move(params), &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  ast::ExpressionList expr;
  expr.push_back(create<ast::ScalarConstructorExpression>(
      create<ast::FloatLiteral>(&f32, 1.0f)));

  auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
  var->set_constructor(create<ast::CallExpression>(
      create<ast::IdentifierExpression>("sub_func"), std::move(expr)));

  body = create<ast::BlockStatement>();
  body->append(create<ast::VariableDeclStatement>(std::move(var)));
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct Data {
  int a;
  float b;
};

float sub_func(const device Data& coord, float param) {
  return coord.b;
}

fragment void frag_main(const device Data& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.00000000f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoints_WithGlobal_Nested_Return) {
  ast::type::VoidType void_type;
  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &f32));
  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(bar_var.get());
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto func_1 = create<ast::Function>("ep_1", std::move(params), &void_type);
  func_1->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::ScalarConstructorExpression>(
          create<ast::FloatLiteral>(&f32, 1.0f))));

  auto list = create<ast::BlockStatement>();
  list->append(create<ast::ReturnStatement>());

  body->append(create<ast::IfStatement>(
      create<ast::BinaryExpression>(ast::BinaryOp::kEqual,
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::SintLiteral>(&i32, 1)),
                                    create<ast::ScalarConstructorExpression>(
                                        create<ast::SintLiteral>(&i32, 1))),
      std::move(list)));

  body->append(create<ast::ReturnStatement>());
  func_1->set_body(std::move(body));

  mod.AddFunction(std::move(func_1));

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct ep_1_out {
  float bar [[color(1)]];
};

fragment ep_1_out ep_1() {
  ep_1_out tint_out = {};
  tint_out.bar = 1.00000000f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_WithNameCollision) {
  ast::type::VoidType void_type;

  auto func = create<ast::Function>("main", ast::VariableList{}, &void_type);
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

kernel void main_tint_0() {
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayParams) {
  ast::type::F32Type f32;
  ast::type::ArrayType ary(&f32, 5);

  ast::VariableList params;
  params.push_back(create<ast::Variable>("a", ast::StorageClass::kNone, &ary));

  ast::type::VoidType void_type;
  auto func = create<ast::Function>("my_func", std::move(params), &void_type);

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::ReturnStatement>());
  func->set_body(std::move(body));

  mod.AddFunction(std::move(func));

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void my_func(float a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(MslGeneratorImplTest,
       Emit_Function_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   [[offset(0)]] d : f32;
  // };
  // [[binding(0), set(0)]] var<storage_buffer> data : Data;
  //
  // [[stage(compute)]]
  // fn a() -> void {
  //   return;
  // }
  //
  // [[stage(compute)]]
  // fn b() -> void {
  //   return;
  // }

  ast::type::VoidType void_type;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  ast::StructMemberDecorationList a_deco;
  a_deco.push_back(create<ast::StructMemberOffsetDecoration>(0, Source{}));
  members.push_back(create<ast::StructMember>("d", &f32, std::move(a_deco)));

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>(Source{}));

  auto str = create<ast::Struct>(std::move(s_decos), std::move(members));

  ast::type::StructType s("Data", std::move(str));
  ast::type::AccessControlType ac(ast::AccessControl::kReadWrite, &s);

  auto data_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("data", ast::StorageClass::kStorageBuffer, &ac));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::BindingDecoration>(0, Source{}));
  decos.push_back(create<ast::SetDecoration>(0, Source{}));
  data_var->set_decorations(std::move(decos));

  mod.AddConstructedType(&s);

  td.RegisterVariableForTesting(data_var.get());
  mod.AddGlobalVariable(std::move(data_var));

  {
    ast::VariableList params;
    auto func = create<ast::Function>("a", std::move(params), &void_type);
    func->add_decoration(
        create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));

    auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(std::move(var)));
    body->append(create<ast::ReturnStatement>());
    func->set_body(std::move(body));

    mod.AddFunction(std::move(func));
  }

  {
    ast::VariableList params;
    auto func = create<ast::Function>("b", std::move(params), &void_type);
    func->add_decoration(
        create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));

    auto var = create<ast::Variable>("v", ast::StorageClass::kFunction, &f32);
    var->set_constructor(create<ast::MemberAccessorExpression>(
        create<ast::IdentifierExpression>("data"),
        create<ast::IdentifierExpression>("d")));

    auto body = create<ast::BlockStatement>();
    body->append(create<ast::VariableDeclStatement>(std::move(var)));
    body->append(create<ast::ReturnStatement>());
    func->set_body(std::move(body));

    mod.AddFunction(std::move(func));
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct Data {
  float d;
};

kernel void a(device Data& data [[buffer(0)]]) {
  float v = data.d;
  return;
}

kernel void b(device Data& data [[buffer(0)]]) {
  float v = data.d;
  return;
}

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
