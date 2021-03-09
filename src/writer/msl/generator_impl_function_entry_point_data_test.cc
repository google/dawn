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

#include "gtest/gtest.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/variable.h"
#include "src/program.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/vector_type.h"
#include "src/type/void_type.h"
#include "src/writer/msl/generator_impl.h"
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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("vtx_main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("vtx_main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kInput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
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
         ast::VariableDecorationList{create<ast::LocationDecoration>(0)});

  Global("bar", ty.i32(), ast::StorageClass::kOutput, nullptr,
         ast::VariableDecorationList{create<ast::LocationDecoration>(1)});

  auto body = ast::StatementList{
      create<ast::AssignmentStatement>(Expr("foo"), Expr("foo")),
      create<ast::AssignmentStatement>(Expr("bar"), Expr("bar")),
  };

  Func("main", ast::VariableList{}, ty.f32(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kCompute),
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

  // [[builtin frag_coord]] var<in> coord : vec4<f32>;
  // [[builtin frag_depth]] var<out> depth : f32;
  //
  // struct main_out {
  //   float depth [[depth(any)]];
  // };

  Global("coord", ty.vec4<f32>(), ast::StorageClass::kInput, nullptr,
         ast::VariableDecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragCoord)});

  Global("depth", ty.f32(), ast::StorageClass::kOutput, nullptr,
         ast::VariableDecorationList{
             create<ast::BuiltinDecoration>(ast::Builtin::kFragDepth)});

  auto body = ast::StatementList{create<ast::AssignmentStatement>(
      Expr("depth"), MemberAccessor("coord", "x"))};

  Func("main", ast::VariableList{}, ty.void_(), body,
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
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
