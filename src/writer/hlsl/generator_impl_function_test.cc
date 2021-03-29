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

#include "gmock/gmock.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/ast/workgroup_decoration.h"
#include "src/type/access_control_type.h"
#include "src/writer/hlsl/test_helper.h"

using ::testing::HasSubstr;

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Function = TestHelper;

TEST_F(HlslGeneratorImplTest_Function, Emit_Function) {
  Func("my_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_Name_Collision) {
  Func("GeometryShader", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void GeometryShader_tint_0() {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithParams) {
  Func("my_func",
       ast::VariableList{Var("a", ty.f32(), ast::StorageClass::kNone),
                         Var("b", ty.i32(), ast::StorageClass::kNone)},
       ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a, int b) {
    return;
  }

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_NoReturn_Void) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{/* no explicit return */},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_NoReturn_InOut) {
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{
                         create<ast::LocationDecoration>(0),
                     });

  // TODO(jrprice): Make this the return value when supported.
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  Func("main", ast::VariableList{foo_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
           /* no explicit return */},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_2 {
  float foo : TEXCOORD0;
};

struct main_out {
  float bar : SV_Target1;
};

main_out main(tint_symbol_2 tint_symbol_3) {
  main_out tint_out;
  const float foo = tint_symbol_3.foo;
  tint_out.bar = foo;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithInOutVars) {
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{
                         create<ast::LocationDecoration>(0),
                     });

  // TODO(jrprice): Make this the return value when supported.
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  Func("frag_main", ast::VariableList{foo_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_2 {
  float foo : TEXCOORD0;
};

struct frag_main_out {
  float bar : SV_Target1;
};

frag_main_out frag_main(tint_symbol_2 tint_symbol_3) {
  frag_main_out tint_out;
  const float foo = tint_symbol_3.foo;
  tint_out.bar = foo;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithInOut_Builtins) {
  auto* coord_in =
      Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone, nullptr,
          ast::DecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord),
          });

  // TODO(jrprice): Make this the return value when supported.
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
         });

  Func("frag_main", ast::VariableList{coord_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("depth"),
                                            MemberAccessor("coord", "x")),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_2 {
  float4 coord : SV_Position;
};

struct frag_main_out {
  float depth : SV_Depth;
};

frag_main_out frag_main(tint_symbol_2 tint_symbol_3) {
  frag_main_out tint_out;
  const float4 coord = tint_symbol_3.coord;
  tint_out.depth = coord.x;
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "x"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  float4 coord;
};

void frag_main() {
  float v = coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_UniformStruct) {
  auto* s = Structure("Uniforms", {Member("coord", ty.vec4<f32>())});

  Global("uniforms", s, ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  create<ast::MemberAccessorExpression>(
                      MemberAccessor("uniforms", "coord"), Expr("x")));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Uniforms {
  float4 coord;
};

ConstantBuffer<Uniforms> uniforms : register(b0, space1);

void frag_main() {
  float v = uniforms.coord.x;
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_RW_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
})"));
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_RO_StorageBuffer_Read) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                  MemberAccessor("coord", "b"));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(ByteAddressBuffer coord : register(t0, space1);

void frag_main() {
  float v = asfloat(coord.Load(4));
  return;
})"));
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_WO_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kWriteOnly, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(MemberAccessor("coord", "b"),
                                            Expr(2.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4, asuint(2.0f));
  return;
})"));
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_With_StorageBuffer_Store) {
  auto* s = Structure("Data", {
                                  Member("a", ty.i32()),
                                  Member("b", ty.f32()),
                              });

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(MemberAccessor("coord", "b"),
                                            Expr(2.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

void frag_main() {
  coord.Store(4, asuint(2.0f));
  return;
})"));
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_Decoration_Called_By_EntryPoints_WithLocationGlobals_And_Params) {  // NOLINT
  auto* foo_in = Var("foo", ty.f32(), ast::StorageClass::kNone, nullptr,
                     ast::DecorationList{
                         create<ast::LocationDecoration>(0),
                     });

  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  Global("val", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(0),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kNone),
                         Var("foo", ty.f32(), ast::StorageClass::kNone)},
       ty.f32(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("bar"), Expr("foo")),
           create<ast::AssignmentStatement>(Expr("val"), Expr("param")),
           create<ast::ReturnStatement>(Expr("foo")),
       },
       ast::DecorationList{});

  Func("ep_1", ast::VariableList{foo_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(
               Expr("bar"), Call("sub_func", 1.0f, Expr("foo"))),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_2 {
  float foo : TEXCOORD0;
};

struct ep_1_out {
  float bar : SV_Target1;
  float val : SV_Target0;
};

float sub_func_ep_1(out ep_1_out tint_out, float param, float foo) {
  tint_out.bar = foo;
  tint_out.val = param;
  return foo;
}

ep_1_out ep_1(tint_symbol_2 tint_symbol_3) {
  ep_1_out tint_out;
  const float foo = tint_symbol_3.foo;
  tint_out.bar = sub_func_ep_1(tint_out, 1.0f, foo);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoints_NoUsedGlobals) {
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr("param")),
       },
       ast::DecorationList{});

  Func("ep_1", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(Expr("depth"),
                                            Call("sub_func", 1.0f)),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func(float param) {
  return param;
}

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.depth = sub_func(1.0f);
  return tint_out;
}

)");
}

TEST_F(
    HlslGeneratorImplTest_Function,
    Emit_Decoration_Called_By_EntryPoints_WithBuiltinGlobals_And_Params) {  // NOLINT
  auto* coord_in =
      Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone, nullptr,
          ast::DecorationList{
              create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord),
          });

  // TODO(jrprice): Make this the return value when supported.
  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth),
         });

  Func(
      "sub_func",
      ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kNone),
                        Var("coord", ty.vec4<f32>(), ast::StorageClass::kNone)},
      ty.f32(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("depth"),
                                           MemberAccessor("coord", "x")),
          create<ast::ReturnStatement>(Expr("param")),
      },
      ast::DecorationList{});

  Func("ep_1", ast::VariableList{coord_in}, ty.void_(),
       ast::StatementList{
           create<ast::AssignmentStatement>(
               Expr("depth"), Call("sub_func", 1.0f, Expr("coord"))),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct tint_symbol_2 {
  float4 coord : SV_Position;
};

struct ep_1_out {
  float depth : SV_Depth;
};

float sub_func_ep_1(out ep_1_out tint_out, float param, float4 coord) {
  tint_out.depth = coord.x;
  return param;
}

ep_1_out ep_1(tint_symbol_2 tint_symbol_3) {
  ep_1_out tint_out;
  const float4 coord = tint_symbol_3.coord;
  tint_out.depth = sub_func_ep_1(tint_out, 1.0f, coord);
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoint_With_Uniform) {
  Global("coord", ty.vec4<f32>(), ast::StorageClass::kUniform, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
       },
       ast::DecorationList{});

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(cbuffer cbuffer_coord : register(b0, space1) {
  float4 coord;
};

float sub_func(float param) {
  return coord.x;
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_Called_By_EntryPoint_With_StorageBuffer) {
  type::AccessControl ac(ast::AccessControl::kReadWrite, ty.vec4<f32>());
  Global("coord", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(1),
         });

  Func("sub_func",
       ast::VariableList{Var("param", ty.f32(), ast::StorageClass::kFunction)},
       ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(MemberAccessor("coord", "x")),
       },
       ast::DecorationList{});

  auto* var =
      Var("v", ty.f32(), ast::StorageClass::kFunction, Call("sub_func", 1.0f));

  Func("frag_main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr(R"(RWByteAddressBuffer coord : register(u0, space1);

float sub_func(float param) {
  return asfloat(coord.Load((4 * 0)));
}

void frag_main() {
  float v = sub_func(1.0f);
  return;
})"));
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoints_WithGlobal_Nested_Return) {
  Global("bar", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             create<ast::LocationDecoration>(1),
         });

  auto* list = create<ast::BlockStatement>(ast::StatementList{
      create<ast::ReturnStatement>(),
  });

  Func(
      "ep_1", ast::VariableList{}, ty.void_(),
      ast::StatementList{
          create<ast::AssignmentStatement>(Expr("bar"), Expr(1.0f)),
          create<ast::IfStatement>(create<ast::BinaryExpression>(
                                       ast::BinaryOp::kEqual, Expr(1), Expr(1)),
                                   list, ast::ElseStatementList{}),
          create<ast::ReturnStatement>(),
      },
      ast::DecorationList{
          create<ast::StageDecoration>(ast::PipelineStage::kFragment),
      });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct ep_1_out {
  float bar : SV_Target1;
};

ep_1_out ep_1() {
  ep_1_out tint_out;
  tint_out.bar = 1.0f;
  if ((1 == 1)) {
    return tint_out;
  }
  return tint_out;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_WithNameCollision) {
  Func("GeometryShader", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(void GeometryShader_tint_0() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Decoration_EntryPoint_Compute) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(1, 1, 1)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function,
       Emit_Decoration_EntryPoint_Compute_WithWorkgroup) {
  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           create<ast::WorkgroupDecoration>(2u, 4u, 6u),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"([numthreads(2, 4, 6)]
void main() {
  return;
}

)");
}

TEST_F(HlslGeneratorImplTest_Function, Emit_Function_WithArrayParams) {
  Func(
      "my_func",
      ast::VariableList{Var("a", ty.array<f32, 5>(), ast::StorageClass::kNone)},
      ty.void_(),
      ast::StatementList{
          create<ast::ReturnStatement>(),
      },
      ast::DecorationList{});

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(  void my_func(float a[5]) {
    return;
  }

)");
}

// https://crbug.com/tint/297
TEST_F(HlslGeneratorImplTest_Function,
       Emit_Multiple_EntryPoint_With_Same_ModuleVar) {
  // [[block]] struct Data {
  //   d : f32;
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

  auto* s =
      Structure("Data", {Member("d", ty.f32())},
                ast::DecorationList{create<ast::StructBlockDecoration>()});

  type::AccessControl ac(ast::AccessControl::kReadWrite, s);

  Global("data", &ac, ast::StorageClass::kStorage, nullptr,
         ast::DecorationList{
             create<ast::BindingDecoration>(0),
             create<ast::GroupDecoration>(0),
         });

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("a", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  {
    auto* var = Var("v", ty.f32(), ast::StorageClass::kFunction,
                    MemberAccessor("data", "d"));

    Func("b", ast::VariableList{}, ty.void_(),
         ast::StatementList{
             create<ast::VariableDeclStatement>(var),
             create<ast::ReturnStatement>(),
         },
         ast::DecorationList{
             create<ast::StageDecoration>(ast::PipelineStage::kCompute),
         });
  }

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_EQ(result(), R"(struct Data {
  float d;
};

RWByteAddressBuffer data : register(u0, space0);

[numthreads(1, 1, 1)]
void a() {
  float v = asfloat(data.Load(0));
  return;
}

[numthreads(1, 1, 1)]
void b() {
  float v = asfloat(data.Load(0));
  return;
}

)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
