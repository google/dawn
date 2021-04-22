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

#include "src/ast/stage_decoration.h"
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

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("vtx_main", ast::VariableList{}, ty.vec4<f32>(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
           Return(Construct(ty.vec4<f32>())),
       },
       {Stage(ast::PipelineStage::kVertex)},
       {Builtin(ast::Builtin::kPosition)});

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
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

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("vtx_main", ast::VariableList{}, ty.vec4<f32>(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
           Return(Construct(ty.vec4<f32>())),
       },
       {Stage(ast::PipelineStage::kVertex)},
       {Builtin(ast::Builtin::kPosition)});

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
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

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
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

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
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

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(),
            R"(error: invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(0),
         });

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Location(1),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("foo", "foo"),
           Assign("bar", "bar"),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_FALSE(gen.EmitEntryPointData(out, func, globals)) << gen.error();
  EXPECT_EQ(gen.error(),
            R"(error: invalid location variable for pipeline stage)");
}

TEST_F(HlslGeneratorImplTest_EntryPoint,
       Emit_Function_EntryPointData_Builtins) {
  // [[builtin position]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_in {
  //   float4 coord : SV_Position;
  // };
  //
  // struct main_out {
  //   float depth : SV_Depth;
  // };

  Global("coord", ty.vec4<f32>(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{
             Builtin(ast::Builtin::kPosition),
         });

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{
             Builtin(ast::Builtin::kFragDepth),
         });

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           Assign("depth", MemberAccessor("coord", "x")),
       },
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  std::unordered_set<Symbol> globals;

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
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
