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
#include "src/ast/discard_statement.h"
#include "src/ast/function.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/return_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"
#include "src/ast/workgroup_decoration.h"
#include "src/writer/wgsl/generator_impl.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, Emit_Function) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::type::VoidType void_type;
  ast::Function func("my_func", {}, &void_type);
  func.set_body(std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitFunction(&func));
  EXPECT_EQ(g.result(), R"(  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithParams) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::type::F32Type f32;
  ast::type::I32Type i32;
  ast::VariableList params;
  params.push_back(
      std::make_unique<ast::Variable>("a", ast::StorageClass::kNone, &f32));
  params.push_back(
      std::make_unique<ast::Variable>("b", ast::StorageClass::kNone, &i32));

  ast::type::VoidType void_type;
  ast::Function func("my_func", std::move(params), &void_type);
  func.set_body(std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitFunction(&func));
  EXPECT_EQ(g.result(), R"(  fn my_func(a : f32, b : i32) -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_WorkgroupSize) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::type::VoidType void_type;
  ast::Function func("my_func", {}, &void_type);
  func.add_decoration(std::make_unique<ast::WorkgroupDecoration>(2u, 4u, 6u));
  func.set_body(std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitFunction(&func));
  EXPECT_EQ(g.result(), R"(  [[workgroup_size(2, 4, 6)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Stage) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::type::VoidType void_type;
  ast::Function func("my_func", {}, &void_type);
  func.add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  func.set_body(std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitFunction(&func));
  EXPECT_EQ(g.result(), R"(  [[stage(fragment)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Function_WithDecoration_Multiple) {
  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::DiscardStatement>());
  body->append(std::make_unique<ast::ReturnStatement>());

  ast::type::VoidType void_type;
  ast::Function func("my_func", {}, &void_type);
  func.add_decoration(
      std::make_unique<ast::StageDecoration>(ast::PipelineStage::kFragment));
  func.add_decoration(std::make_unique<ast::WorkgroupDecoration>(2u, 4u, 6u));
  func.set_body(std::move(body));

  GeneratorImpl g;
  g.increment_indent();

  ASSERT_TRUE(g.EmitFunction(&func));
  EXPECT_EQ(g.result(), R"(  [[stage(fragment)]]
  [[workgroup_size(2, 4, 6)]]
  fn my_func() -> void {
    discard;
    return;
  }
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
