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
#include "src/type_determiner.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Function) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void test_my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithParams) {
  ast::VariableList params;
  params.push_back(Var("a", ast::StorageClass::kNone, ty.f32));
  params.push_back(Var("b", ast::StorageClass::kNone, ty.i32));

  auto* func = Func("my_func", params, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void test_my_func(float test_a, int test_b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionDecoration_EntryPoint_WithInOutVars) {
  auto* foo_var =
      Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  auto* bar_var =
      Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
      create<ast::ReturnStatement>(),
  };
  auto* func = Func("frag_main", ast::VariableList{}, ty.void_, body,
                    ast::FunctionDecorationList{create<ast::StageDecoration>(
                        ast::PipelineStage::kFragment)});

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_frag_main_in {
  float test_foo [[user(locn0)]];
};

struct test_frag_main_out {
  float test_bar [[color(1)]];
};

fragment test_frag_main_out test_frag_main(test_frag_main_in test_tint_in [[stage_in]]) {
  test_frag_main_out test_tint_out = {};
  test_tint_out.test_bar = test_tint_in.test_foo;
  return test_tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_WithInOut_Builtins) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kInput, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});

  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod->AddGlobalVariable(coord_var);
  mod->AddGlobalVariable(depth_var);

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"),
                                       MemberAccessor("coord", "x")),
      create<ast::ReturnStatement>(),
  };
  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_frag_main_out {
  float test_depth [[depth(any)]];
};

fragment test_frag_main_out test_frag_main(float4 test_coord [[position]]) {
  test_frag_main_out test_tint_out = {};
  test_tint_out.test_depth = test_coord.x;
  return test_tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionDecoration_EntryPoint_With_Uniform) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);

  mod->AddGlobalVariable(coord_var);

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  MemberAccessor("coord", "x"), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

fragment void test_frag_main(constant float4& test_coord [[buffer(0)]]) {
  float test_v = test_coord.x;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);

  mod->AddGlobalVariable(coord_var);

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  MemberAccessor("coord", "b"), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_Data {
  int test_a;
  float test_b;
};

fragment void test_frag_main(device test_Data& test_coord [[buffer(0)]]) {
  float test_v = test_coord.test_b;
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("x", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadOnly, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  MemberAccessor("coord", "x"), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_Data {
  int test_a;
  float test_x;
};

fragment void test_frag_main(const device test_Data& test_coord [[buffer(0)]]) {
  float test_v = test_coord.test_x;
  return;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  auto* foo_var =
      Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  auto* bar_var =
      Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto* val_var =
      Var("val", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);
  td.RegisterVariableForTesting(val_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);
  mod->AddGlobalVariable(val_var);

  ast::VariableList params;
  params.push_back(Var("param", ast::StorageClass::kFunction, ty.f32));

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("val"), Expr("param")),
      create<ast::ReturnStatement>(Expr("foo"))};
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };
  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_ep_1_in {
  float test_foo [[user(locn0)]];
};

struct test_ep_1_out {
  float test_bar [[color(1)]];
  float test_val [[color(0)]];
};

float test_sub_func_ep_1(thread test_ep_1_in& test_tint_in, thread test_ep_1_out& test_tint_out, float test_param) {
  test_tint_out.test_bar = test_tint_in.test_foo;
  test_tint_out.test_val = test_param;
  return test_tint_in.test_foo;
}

fragment test_ep_1_out test_ep_1(test_ep_1_in test_tint_in [[stage_in]]) {
  test_ep_1_out test_tint_out = {};
  test_tint_out.test_bar = test_sub_func_ep_1(test_tint_in, test_tint_out, 1.0f);
  return test_tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoints_NoUsedGlobals) {
  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  td.RegisterVariableForTesting(depth_var);
  mod->AddGlobalVariable(depth_var);

  ast::VariableList params;
  params.push_back(Var("param", ast::StorageClass::kFunction, ty.f32));

  auto* sub_func = Func("sub_func", params, ty.f32,
                        ast::StatementList{
                            create<ast::ReturnStatement>(Expr("param")),
                        },
                        ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };

  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_ep_1_out {
  float test_depth [[depth(any)]];
};

float test_sub_func(float test_param) {
  return test_param;
}

fragment test_ep_1_out test_ep_1() {
  test_ep_1_out test_tint_out = {};
  test_tint_out.test_depth = test_sub_func(1.0f);
  return test_tint_out;
}

)");
}

TEST_F(
    MslGeneratorImplTest,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  auto* coord_var =
      Var("coord", ast::StorageClass::kInput, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});

  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod->AddGlobalVariable(coord_var);
  mod->AddGlobalVariable(depth_var);

  ast::VariableList params;
  params.push_back(Var("param", ast::StorageClass::kFunction, ty.f32));

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"),
                                       MemberAccessor("coord", "x")),
      create<ast::ReturnStatement>(Expr("param")),
  };
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };
  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_ep_1_out {
  float test_depth [[depth(any)]];
};

float test_sub_func_ep_1(thread test_ep_1_out& test_tint_out, thread float4& test_coord, float test_param) {
  test_tint_out.test_depth = test_coord.x;
  return test_param;
}

fragment test_ep_1_out test_ep_1(float4 test_coord [[position]]) {
  test_ep_1_out test_tint_out = {};
  test_tint_out.test_depth = test_sub_func_ep_1(test_tint_out, test_coord, 1.0f);
  return test_tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(Var("param", ast::StorageClass::kFunction, ty.f32));

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
  };
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(Expr(1.0f));

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  Call("sub_func", 1.0f), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

float test_sub_func(constant float4& test_coord, float test_param) {
  return test_coord.x;
}

fragment void test_frag_main(constant float4& test_coord [[buffer(0)]]) {
  float test_v = test_sub_func(test_coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RW_StorageBuffer) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      Var("param", ast::StorageClass::kFunction, ty.f32));  // decorations

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  Call("sub_func", 1.0f), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_Data {
  int test_a;
  float test_b;
};

float test_sub_func(device test_Data& test_coord, float test_param) {
  return test_coord.test_b;
}

fragment void test_frag_main(device test_Data& test_coord [[buffer(0)]]) {
  float test_v = test_sub_func(test_coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_RO_StorageBuffer) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadOnly, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      Var("param", ast::StorageClass::kFunction, ty.f32));  // decorations

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  ast::ExpressionList expr;
  expr.push_back(Expr(1.0f));

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  Call("sub_func", 1.0f), ast::VariableDecorationList{});

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(var),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_Data {
  int test_a;
  float test_b;
};

float test_sub_func(const device test_Data& test_coord, float test_param) {
  return test_coord.test_b;
}

fragment void test_frag_main(const device test_Data& test_coord [[buffer(0)]]) {
  float test_v = test_sub_func(test_coord, 1.0f);
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoints_WithGlobal_Nested_Return) {
  auto* bar_var =
      Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  td.RegisterVariableForTesting(bar_var);
  mod->AddGlobalVariable(bar_var);

  auto* list = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Expr(1.f)),
      create<ast::IfStatement>(create<ast::BinaryExpression>(
                                   ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                               list, ast::ElseStatementList{}),
      create<ast::ReturnStatement>(),
  };

  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_ep_1_out {
  float test_bar [[color(1)]];
};

fragment test_ep_1_out test_ep_1() {
  test_ep_1_out test_tint_out = {};
  test_tint_out.test_bar = 1.0f;
  if ((1 == 1)) {
    return test_tint_out;
  }
  return test_tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_WithArrayParams) {
  ast::VariableList params;
  params.push_back(Var("a", ast::StorageClass::kNone, ty.array<f32, 5>()));

  auto* func = Func("my_func", params, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void test_my_func(float test_a[5]) {
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

  ast::StructDecorationList s_decos;
  s_decos.push_back(create<ast::StructBlockDecoration>());

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("d", ty.f32, {MemberOffset(0)})}, s_decos);

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* data_var =
      Var("data", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::SetDecoration>(0)});

  mod->AddConstructedType(s);
  td.RegisterVariableForTesting(data_var);
  mod->AddGlobalVariable(data_var);

  {
    auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                    MemberAccessor("data", "d"), ast::VariableDecorationList{});

    auto* func =
        Func("a", ast::VariableList{}, ty.void_,
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->AddFunction(func);
  }

  {
    auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                    MemberAccessor("data", "d"), ast::VariableDecorationList{});

    auto* func =
        Func("b", ast::VariableList{}, ty.void_,
             ast::StatementList{create<ast::VariableDeclStatement>(var),
                                create<ast::ReturnStatement>()},
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute)});

    mod->AddFunction(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct test_Data {
  float test_d;
};

kernel void test_a(device test_Data& test_data [[buffer(0)]]) {
  float test_v = test_data.test_d;
  return;
}

kernel void test_b(device test_Data& test_data [[buffer(0)]]) {
  float test_v = test_data.test_d;
  return;
}

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
