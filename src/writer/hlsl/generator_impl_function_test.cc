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
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  auto* func = Func("my_func", ast::VariableList{}, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void test_my_func() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  auto* func =
      Func("my_func",
           ast::VariableList{Var("a", ast::StorageClass::kNone, ty.f32),
                             Var("b", ast::StorageClass::kNone, ty.i32)},
           ty.void_,
           ast::StatementList{
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void test_my_func(float test_a, int test_b) {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOutVars) {
  auto* foo_var = Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_frag_main_in {
  float test_foo : TEXCOORD0;
};

struct test_frag_main_out {
  float test_bar : SV_Target1;
};

test_frag_main_out test_frag_main(test_frag_main_in test_tint_in) {
  test_frag_main_out test_tint_out;
  test_tint_out.test_bar = test_tint_in.test_foo;
  return test_tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_WithInOut_Builtins) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kInput, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord),
          });

  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
          });

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod->AddGlobalVariable(coord_var);
  mod->AddGlobalVariable(depth_var);

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("depth"),
                                                MemberAccessor("coord", "x")),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_frag_main_in {
  float4 test_coord : SV_Position;
};

struct test_frag_main_out {
  float test_depth : SV_Depth;
};

test_frag_main_out test_frag_main(test_frag_main_in test_tint_in) {
  test_frag_main_out test_tint_out;
  test_tint_out.test_depth = test_tint_in.test_coord.x;
  return test_tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_Uniform) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_test_coord : register(b0) {
  float4 test_coord;
};

void test_frag_main() {
  float test_v = test_coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_UniformStruct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("coord", ty.vec4<f32>())},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Uniforms", str);

  auto* coord_var = Var("uniforms", ast::StorageClass::kUniform, s, nullptr,
                        ast::VariableDecorationList{
                            create<ast::BindingDecoration>(0),
                            create<ast::SetDecoration>(1),
                        });

  mod->AddConstructedType(s);

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  auto* var = Var("v", ast::StorageClass::kFunction, ty.f32,
                  create<ast::MemberAccessorExpression>(
                      MemberAccessor("uniforms", "coord"), Expr("x")),
                  ast::VariableDecorationList{});

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_Uniforms {
  float4 test_coord;
};

ConstantBuffer<test_Uniforms> test_uniforms : register(b0);

void test_frag_main() {
  float test_v = test_uniforms.test_coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RW_StorageBuffer_Read) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer test_coord : register(u0);

void test_frag_main() {
  float test_v = asfloat(test_coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_RO_StorageBuffer_Read) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadOnly, s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{
              // decorations
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(ByteAddressBuffer test_coord : register(u0);

void test_frag_main() {
  float test_v = asfloat(test_coord.Load(4));
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_With_StorageBuffer_Store) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  auto* func =
      Func("frag_main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(MemberAccessor("coord", "b"),
                                                Expr(2.0f)),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer test_coord : register(u0);

void test_frag_main() {
  test_coord.Store(4, asuint(2.0f));
  return;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT

  auto* foo_var = Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  auto* val_var = Var("val", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);
  td.RegisterVariableForTesting(val_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);
  mod->AddGlobalVariable(val_var);

  auto* sub_func = Func(
      "sub_func",
      ast::VariableList{Var("param", ast::StorageClass::kFunction, ty.f32)},
      ty.f32,
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
          create<ast::AssignmentStatement>(Expr("val"), Expr("param")),
          create<ast::ReturnStatement>(Expr("foo")),
      },
      ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  auto* func_1 = Func(
      "ep_1", ast::VariableList{}, ty.void_,
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Call("sub_func", 1.0f)),
          create<ast::ReturnStatement>(),
      },
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_ep_1_in {
  float test_foo : TEXCOORD0;
};

struct test_ep_1_out {
  float test_bar : SV_Target1;
  float test_val : SV_Target0;
};

float test_sub_func_ep_1(in test_ep_1_in test_tint_in, out test_ep_1_out test_tint_out, float test_param) {
  test_tint_out.test_bar = test_tint_in.test_foo;
  test_tint_out.test_val = test_param;
  return test_tint_in.test_foo;
}

test_ep_1_out test_ep_1(test_ep_1_in test_tint_in) {
  test_ep_1_out test_tint_out;
  test_tint_out.test_bar = test_sub_func_ep_1(test_tint_in, test_tint_out, 1.0f);
  return test_tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoints_NoUsedGlobals) {
  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
          });

  td.RegisterVariableForTesting(depth_var);

  mod->AddGlobalVariable(depth_var);

  auto* sub_func = Func(
      "sub_func",
      ast::VariableList{Var("param", ast::StorageClass::kFunction, ty.f32)},
      ty.f32,
      ast::StatementList{
          create<ast::ReturnStatement>(Expr("param")),
      },
      ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("depth"),
                                                Call("sub_func", 1.0f)),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_ep_1_out {
  float test_depth : SV_Depth;
};

float test_sub_func(float test_param) {
  return test_param;
}

test_ep_1_out test_ep_1() {
  test_ep_1_out test_tint_out;
  test_tint_out.test_depth = test_sub_func(1.0f);
  return test_tint_out;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_FunctionDecoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT

  auto* coord_var =
      Var("coord", ast::StorageClass::kInput, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord),
          });

  auto* depth_var =
      Var("depth", ast::StorageClass::kOutput, ty.f32, nullptr,
          ast::VariableDecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
          });

  td.RegisterVariableForTesting(coord_var);
  td.RegisterVariableForTesting(depth_var);

  mod->AddGlobalVariable(coord_var);
  mod->AddGlobalVariable(depth_var);

  auto* sub_func = Func(
      "sub_func",
      ast::VariableList{Var("param", ast::StorageClass::kFunction, ty.f32)},
      ty.f32,
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("depth"),
                                           MemberAccessor("coord", "x")),
          create<ast::ReturnStatement>(Expr("param")),
      },
      ast::FunctionDecorationList{});

  mod->AddFunction(sub_func);

  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("depth"),
                                                Call("sub_func", 1.0f)),
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_ep_1_in {
  float4 test_coord : SV_Position;
};

struct test_ep_1_out {
  float test_depth : SV_Depth;
};

float test_sub_func_ep_1(in test_ep_1_in test_tint_in, out test_ep_1_out test_tint_out, float test_param) {
  test_tint_out.test_depth = test_tint_in.test_coord.x;
  return test_param;
}

test_ep_1_out test_ep_1(test_ep_1_in test_tint_in) {
  test_ep_1_out test_tint_out;
  test_tint_out.test_depth = test_sub_func_ep_1(test_tint_in, test_tint_out, 1.0f);
  return test_tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

  td.RegisterVariableForTesting(coord_var);

  mod->AddGlobalVariable(coord_var);

  auto* sub_func = Func(
      "sub_func",
      ast::VariableList{Var("param", ast::StorageClass::kFunction, ty.f32)},
      ty.f32,
      ast::StatementList{
          create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
      },
      ast::FunctionDecorationList{});

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_test_coord : register(b0) {
  float4 test_coord;
};

float test_sub_func(float test_param) {
  return test_coord.x;
}

void test_frag_main() {
  float test_v = test_sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_StorageBuffer) {
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, ty.vec4<f32>());
  auto* coord_var =
      Var("coord", ast::StorageClass::kStorageBuffer, &ac, nullptr,
          ast::VariableDecorationList{
              create<ast::BindingDecoration>(0),
              create<ast::SetDecoration>(1),
          });

  td.RegisterVariableForTesting(coord_var);

  mod->AddGlobalVariable(coord_var);

  auto* sub_func = Func(
      "sub_func",
      ast::VariableList{Var("param", ast::StorageClass::kFunction, ty.f32)},
      ty.f32,
      ast::StatementList{
          create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
      },
      ast::FunctionDecorationList{});

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
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(RWByteAddressBuffer test_coord : register(u0);

float test_sub_func(float test_param) {
  return asfloat(test_coord.Load((4 * 0)));
}

void test_frag_main() {
  float test_v = test_sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoints_WithGlobal_Nested_Return) {
  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(bar_var);
  mod->AddGlobalVariable(bar_var);

  auto* list = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  auto* func_1 = Func(
      "ep_1", ast::VariableList{}, ty.void_,
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Expr(1.0f)),
          create<ast::IfStatement>(create<ast::BinaryExpression>(
                                       ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                                   list, ast::ElseStatementList{}),
          create<ast::ReturnStatement>(),
      },
      ast::FunctionDecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  mod->AddFunction(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_ep_1_out {
  float test_bar : SV_Target1;
};

test_ep_1_out test_ep_1() {
  test_ep_1_out test_tint_out;
  test_tint_out.test_bar = 1.0f;
  if ((1 == 1)) {
    return test_tint_out;
  }
  return test_tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute) {
  auto* func =
      Func("main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void test_main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_FunctionDecoration_EntryPoint_Compute_WithWorkgroup) {
  auto* func =
      Func("main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kCompute),
               create<ast::WorkgroupDecoration>(2u, 4u, 6u),
           });

  mod->AddFunction(func);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(2, 4, 6)]
void test_main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  auto* func = Func(
      "my_func",
      ast::VariableList{Var("a", ast::StorageClass::kNone, ty.array<f32, 5>())},
      ty.void_,
      ast::StatementList{
          create<ast::ReturnStatement>(),
      },
      ast::FunctionDecorationList{});

  mod->AddFunction(func);
  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void test_my_func(float test_a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(HlslGeneratorImplTest_Function,
       Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
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

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("d", ty.f32, {MemberOffset(0)})},
      ast::StructDecorationList{create<ast::StructBlockDecoration>()});

  auto* s = ty.struct_("Data", str);
  ast::type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* data_var = Var("data", ast::StorageClass::kStorageBuffer, &ac, nullptr,
                       ast::VariableDecorationList{
                           create<ast::BindingDecoration>(0),
                           create<ast::SetDecoration>(0),
                       });

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
             ast::StatementList{
                 create<ast::VariableDeclStatement>(var),
                 create<ast::ReturnStatement>(),
             },
             ast::FunctionDecorationList{
                 create<ast::StageDecoration>(ast::PipelineStage::kCompute),
             });

    mod->AddFunction(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct test_Data {
  float test_d;
};

RWByteAddressBuffer test_data : register(u0);

[numthreads(1, 1, 1)]
void test_a() {
  float test_v = asfloat(test_data.Load(0));
  return;
}

[numthreads(1, 1, 1)]
void test_b() {
  float test_v = asfloat(test_data.Load(0));
  return;
}

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
