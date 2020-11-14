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
#include "src/ast/decorated_variable.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/location_decoration.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/module.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/context.h"
#include "src/type_determiner.h"
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

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));

  auto func = create<ast::Function>("vtx_main", std::move(params), &f32,
                                    std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.EmitEntryPointData(func_ptr)) << gen.error();
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

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));

  auto func = create<ast::Function>("vtx_main", std::move(params), &f32,
                                    std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.EmitEntryPointData(func_ptr)) << gen.error();
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

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));
  auto func =
      create<ast::Function>("main", std::move(params), &f32, std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.EmitEntryPointData(func_ptr)) << gen.error();
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

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));

  auto func =
      create<ast::Function>("main", std::move(params), &f32, std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.EmitEntryPointData(func_ptr)) << gen.error();
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

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kInput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kInput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));

  auto func =
      create<ast::Function>("main", std::move(params), &f32, std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_FALSE(gen.EmitEntryPointData(func_ptr)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
}

TEST_F(MslGeneratorImplTest, Emit_Function_EntryPointData_Compute_Output) {
  // [[location 0]] var<out> foo : f32;
  // [[location 1]] var<out> bar : i32;
  //
  // -> Error not allowed

  ast::type::F32Type f32;
  ast::type::I32Type i32;

  auto foo_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("foo", ast::StorageClass::kOutput, &f32));

  ast::VariableDecorationList decos;
  decos.push_back(create<ast::LocationDecoration>(0, Source{}));
  foo_var->set_decorations(std::move(decos));

  auto bar_var = create<ast::DecoratedVariable>(
      create<ast::Variable>("bar", ast::StorageClass::kOutput, &i32));
  decos.push_back(create<ast::LocationDecoration>(1, Source{}));
  bar_var->set_decorations(std::move(decos));

  td.RegisterVariableForTesting(foo_var.get());
  td.RegisterVariableForTesting(bar_var.get());

  mod.AddGlobalVariable(std::move(foo_var));
  mod.AddGlobalVariable(std::move(bar_var));

  ast::VariableList params;
  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("foo"),
      create<ast::IdentifierExpression>("foo")));
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("bar"),
      create<ast::IdentifierExpression>("bar")));

  auto func =
      create<ast::Function>("main", std::move(params), &f32, std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_FALSE(gen.EmitEntryPointData(func_ptr)) << gen.error();
  EXPECT_EQ(gen.error(), R"(invalid location variable for pipeline stage)");
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

  ast::type::F32Type f32;
  ast::type::VoidType void_type;
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

  auto body = create<ast::BlockStatement>();
  body->append(create<ast::AssignmentStatement>(
      create<ast::IdentifierExpression>("depth"),
      create<ast::MemberAccessorExpression>(
          create<ast::IdentifierExpression>("coord"),
          create<ast::IdentifierExpression>("x"))));

  auto func = create<ast::Function>("main", std::move(params), &void_type,
                                    std::move(body));
  func->add_decoration(
      create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{}));
  auto* func_ptr = func.get();

  mod.AddFunction(std::move(func));

  ASSERT_TRUE(td.Determine()) << td.error();

  ASSERT_TRUE(gen.EmitEntryPointData(func_ptr)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct main_out {
  float depth [[depth(any)]];
};

)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
