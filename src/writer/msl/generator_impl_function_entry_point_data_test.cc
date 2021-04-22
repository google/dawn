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
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Vertex_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct vtx_main_in {
  //   float foo [[attribute(0)]];
  //   int bar [[attribute(1)]];
  // };

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
      Return(Construct(ty.vec4<f32>())),
  };

  Func("vtx_main", ast::VariableList{}, ty.vec4<f32>(), body,
       {Stage(ast::PipelineStage::kVertex)},
       {Builtin(ast::Builtin::kPosition)});

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct vtx_main_in {
  float foo [[attribute(0)]];
  int bar [[attribute(1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Vertex_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct vtx_main_out {
  //   float foo [[user(locn0)]];
  //   int bar [[user(locn1)]];
  // };

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
      Return(Construct(ty.vec4<f32>())),
  };

  Func("vtx_main", ast::VariableList{}, ty.vec4<f32>(), body,
       {Stage(ast::PipelineStage::kVertex)},
       {Builtin(ast::Builtin::kPosition)});

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct vtx_main_out {
  float foo [[user(locn0)]];
  int bar [[user(locn1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Fragment_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // struct frag_main_in {
  //   float foo [[user(locn0)]];
  //   int bar [[user(locn1)]];
  // };

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
  };

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct main_in {
  float foo [[user(locn0)]];
  int bar [[user(locn1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Fragment_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // struct frag_main_out {
  //   float foo [[color(0)]];
  //   int bar [[color(1)]];
  // };

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
  };

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct main_out {
  float foo [[color(0)]];
  int bar [[color(1)]];
};

)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Compute_Input) {
  // [[location 0]] var<in> foo : f32;
  // [[location 1]] var<in> bar : i32;
  //
  // -> Error, not allowed

  Global("foo", ty.f32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
  };

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_FALSE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.error(),
            R"(error: invalid location variable for pipeline stage)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  Global("foo", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Location(1)});

  auto body = ast::StatementList{
      Assign("foo", "foo"),
      Assign("bar", "bar"),
  };

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           Stage(ast::PipelineStage::kCompute),
       });

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_FALSE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.error(),
            R"(error: invalid location variable for pipeline stage)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Builtins) {
  // Output builtins go in the output struct, input builtins will be passed
  // as input parameters to the entry point function.

  // [[builtin position]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_out {
  //   float depth [[depth(any)]];
  // };

  Global("coord", ty.vec4<f32>(), ast::StorageClass::kInput, nullptr,
         ast::DecorationList{Builtin(ast::Builtin::kPosition)});

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::DecorationList{Builtin(ast::Builtin::kFragDepth)});

  auto body = ast::StatementList{Assign("depth", MemberAccessor("coord", "x"))};

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::DecorationList{
           Stage(ast::PipelineStage::kFragment),
       });

  GeneratorImpl& gen = Build();

  auto* func = program->AST().Functions()[0];
  ASSERT_TRUE(gen.EmitEntryPointData(func)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct main_out {
  float depth [[depth(any)]];
};

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
