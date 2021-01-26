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
#include "src/ast/group_decoration.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program.h"
#include "src/type/access_control_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
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

  mod->Functions().Add(func);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void my_func() {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_Name_Collision) {
  auto* func = Func("main", ast::VariableList{}, ty.void_,
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});

  mod->Functions().Add(func);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void main_tint_0() {
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

  mod->Functions().Add(func);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(MslGeneratorImplTest, Emit_FunctionDecoration_EntryPoint_NoReturn_Void) {
  auto* func = Func("main", ast::VariableList{}, ty.void_,
                    ast::StatementList{/* no explicit return */},
                    ast::FunctionDecorationList{create<ast::StageDecoration>(
                        ast::PipelineStage::kFragment)});

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

fragment void main_tint_0() {
  return;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_NoReturn_InOut) {
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

  auto* func =
      Func("main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
               /* no explicit return */},
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment)});

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct main_in {
  float foo [[user(locn0)]];
};

struct main_out {
  float bar [[color(1)]];
};

fragment main_out main_tint_0(main_in tint_in [[stage_in]]) {
  main_out tint_out = {};
  tint_out.bar = tint_in.foo;
  return tint_out;
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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorage, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32, {MemberOffset(0)}),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("Data", str);
  type::AccessControl ac(ast::AccessControl::kReadOnly, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorage, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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

  mod->Functions().Add(sub_func);

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("bar"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };
  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->Functions().Add(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  tint_out.bar = sub_func_ep_1(tint_in, tint_out, 1.0f);
  return tint_out;
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

  mod->Functions().Add(sub_func);

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };

  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->Functions().Add(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  tint_out.depth = sub_func(1.0f);
  return tint_out;
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

  mod->Functions().Add(sub_func);

  body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("depth"), Call("sub_func", 1.0f)),
      create<ast::ReturnStatement>(),
  };
  auto* func_1 =
      Func("ep_1", ast::VariableList{}, ty.void_, body,
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->Functions().Add(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  tint_out.depth = sub_func_ep_1(tint_out, coord, 1.0f);
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_Called_By_EntryPoint_With_Uniform) {
  auto* coord_var =
      Var("coord", ast::StorageClass::kUniform, ty.vec4<f32>(), nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(Var("param", ast::StorageClass::kFunction, ty.f32));

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
  };
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->Functions().Add(sub_func);

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

float sub_func(constant float4& coord, float param) {
  return coord.x;
}

fragment void frag_main(constant float4& coord [[buffer(0)]]) {
  float v = sub_func(coord, 1.0f);
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
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorage, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      Var("param", ast::StorageClass::kFunction, ty.f32));  // decorations

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->Functions().Add(sub_func);

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  float v = sub_func(coord, 1.0f);
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
  type::AccessControl ac(ast::AccessControl::kReadOnly, s);
  mod->AddConstructedType(s);

  auto* coord_var =
      Var("coord", ast::StorageClass::kStorage, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(1)});

  td.RegisterVariableForTesting(coord_var);
  mod->AddGlobalVariable(coord_var);

  ast::VariableList params;
  params.push_back(
      Var("param", ast::StorageClass::kFunction, ty.f32));  // decorations

  auto body = ast::StatementList{
      create<ast::ReturnStatement>(MemberAccessor("coord", "b"))};
  auto* sub_func =
      Func("sub_func", params, ty.f32, body, ast::FunctionDecorationList{});

  mod->Functions().Add(sub_func);

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

  mod->Functions().Add(func);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
  float v = sub_func(coord, 1.0f);
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

  mod->Functions().Add(func_1);

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

struct ep_1_out {
  float bar [[color(1)]];
};

fragment ep_1_out ep_1() {
  ep_1_out tint_out = {};
  tint_out.bar = 1.0f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");
}

TEST_F(MslGeneratorImplTest,
       Emit_FunctionDecoration_EntryPoint_WithNameCollision) {
  auto* func =
      Func("main", ast::VariableList{}, ty.void_, ast::StatementList{},
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           });

  mod->Functions().Add(func);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate()) << gen.error();
  EXPECT_EQ(gen.result(), R"(#include <metal_stdlib>

kernel void main_tint_0() {
  return;
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

  mod->Functions().Add(func);

  GeneratorImpl& gen = Build();

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
  // [[binding(0), group(0)]] var<storage> data : Data;
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
  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  auto* data_var =
      Var("data", ast::StorageClass::kStorage, &ac, nullptr,
          ast::VariableDecorationList{create<ast::BindingDecoration>(0),
                                      create<ast::GroupDecoration>(0)});

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

    mod->Functions().Add(func);
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

    mod->Functions().Add(func);
  }

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

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
