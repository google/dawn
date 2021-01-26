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

#include <memory>
#include <unordered_set>

#include "src/ast/assignment_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable.h"
#include "src/program.h"
#include "src/type_determiner.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_EntryPoint = TestHelper;

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Vertex_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct vtx_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  auto* foo_var = Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kInput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("vtx_main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct vtx_main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Vertex_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct vtx_main_out {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  auto* foo_var = Var("foo", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("vtx_main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct vtx_main_out {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Fragment_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct frag_main_in {
  //   float foo : TEXCOORD0;
  //   int bar : TEXCOORD1;
  // };

  auto* foo_var = Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kInput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_in {
  float foo : TEXCOORD0;
  int bar : TEXCOORD1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Fragment_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct frag_main_out {
  //   float foo : SV_Target0;
  //   int bar : SV_Target1;
  // };

  auto* foo_var = Var("foo", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_out {
  float foo : SV_Target0;
  int bar : SV_Target1;
};

)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Compute_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // -> Error, not allowed

  auto* foo_var = Var("foo", ast::StorageClass::kInput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kInput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  auto* foo_var = Var("foo", ast::StorageClass::kOutput, ty.f32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(0),
                      });

  auto* bar_var = Var("bar", ast::StorageClass::kOutput, ty.i32, nullptr,
                      ast::VariableDecorationList{
                          create<ast::LocationDecoration>(1),
                      });

  td.RegisterVariableForTesting(foo_var);
  td.RegisterVariableForTesting(bar_var);

  mod->AddGlobalVariable(foo_var);
  mod->AddGlobalVariable(bar_var);

  auto* func =
      Func("main", ast::VariableList{}, ty.f32,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
               create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kCompute),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Builtins) {
  // [[builtin frag_coord]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_in {
  //   float4 coord : SV_Position;
  // };
  //
  // struct main_out {
  //   float depth : SV_Depth;
  // };

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
      Func("main", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::AssignmentStatement>(Expr("depth"),
                                                MemberAccessor("coord", "x")),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kFragment),
           });

  mod->Functions().Add(func);

  std::unordered_set<Symbol> globals;

  ASSERT_TRUE(td.Determine()) << td.error();

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(result(), R"(struct main_in {
  float4 coord : SV_Position;
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
