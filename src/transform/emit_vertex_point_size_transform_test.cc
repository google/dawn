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

#include "src/transform/emit_vertex_point_size_transform.h"

#include <memory>
#include <utility>

#include "gtest/gtest.h"
#include "src/ast/builder.h"
#include "src/ast/call_statement.h"
#include "src/ast/stage_decoration.h"
#include "src/transform/manager.h"

namespace tint {
namespace transform {
namespace {

class EmitVertexPointSizeTransformTest : public testing::Test,
                                         public ast::BuilderWithModule {
 public:
  EmitVertexPointSizeTransformTest() {
    auto transform = std::make_unique<EmitVertexPointSizeTransform>(mod);
    manager = std::make_unique<Manager>();
    manager->append(std::move(transform));
  }

  std::unique_ptr<Manager> manager;
};

TEST_F(EmitVertexPointSizeTransformTest, VertexStageBasic) {
  auto* block = create<ast::BlockStatement>(Source{});
  block->append(create<ast::CallStatement>(create<ast::CallExpression>(
      Source{},
      create<ast::IdentifierExpression>(
          Source{}, "builtin_assignments_should_happen_before_this"),
      ast::ExpressionList{})));

  mod->AddFunction(create<ast::Function>(
      "non_entry_a", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{})));

  auto* entry = create<ast::Function>("entry", ast::VariableList{},
                                      create<ast::type::Void>(), block);
  entry->set_decorations(
      {create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{})});
  mod->AddFunction(entry);

  mod->AddFunction(create<ast::Function>(
      "non_entry_b", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{})));

  manager->Run(mod);

  auto* expected = R"(Module{
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{pointsize}
    }
    tint_pointsize
    out
    __f32
  }
  Function non_entry_a -> __void
  ()
  {
  }
  Function entry -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[__ptr_out__f32]{tint_pointsize}
      ScalarConstructor[__f32]{1.000000}
    }
    Call[not set]{
      Identifier[not set]{builtin_assignments_should_happen_before_this}
      (
      )
    }
  }
  Function non_entry_b -> __void
  ()
  {
  }
}
)";
  EXPECT_EQ(expected, mod->to_str());
}

TEST_F(EmitVertexPointSizeTransformTest, VertexStageEmpty) {
  mod->AddFunction(create<ast::Function>(
      "non_entry_a", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{})));

  auto* entry = create<ast::Function>("entry", ast::VariableList{},
                                      create<ast::type::Void>(),
                                      create<ast::BlockStatement>(Source{}));
  entry->set_decorations(
      {create<ast::StageDecoration>(ast::PipelineStage::kVertex, Source{})});
  mod->AddFunction(entry);

  mod->AddFunction(create<ast::Function>(
      "non_entry_b", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{})));

  manager->Run(mod);

  auto* expected = R"(Module{
  DecoratedVariable{
    Decorations{
      BuiltinDecoration{pointsize}
    }
    tint_pointsize
    out
    __f32
  }
  Function non_entry_a -> __void
  ()
  {
  }
  Function entry -> __void
  StageDecoration{vertex}
  ()
  {
    Assignment{
      Identifier[__ptr_out__f32]{tint_pointsize}
      ScalarConstructor[__f32]{1.000000}
    }
  }
  Function non_entry_b -> __void
  ()
  {
  }
}
)";
  EXPECT_EQ(expected, mod->to_str());
}

TEST_F(EmitVertexPointSizeTransformTest, NonVertexStage) {
  auto* fragment_entry = create<ast::Function>(
      "fragment_entry", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{}));
  fragment_entry->set_decorations(
      {create<ast::StageDecoration>(ast::PipelineStage::kFragment, Source{})});
  mod->AddFunction(fragment_entry);

  auto* compute_entry = create<ast::Function>(
      "compute_entry", ast::VariableList{}, create<ast::type::Void>(),
      create<ast::BlockStatement>(Source{}));
  compute_entry->set_decorations(
      {create<ast::StageDecoration>(ast::PipelineStage::kCompute, Source{})});
  mod->AddFunction(compute_entry);

  manager->Run(mod);

  auto* expected = R"(Module{
  Function fragment_entry -> __void
  StageDecoration{fragment}
  ()
  {
  }
  Function compute_entry -> __void
  StageDecoration{compute}
  ()
  {
  }
}
)";
  EXPECT_EQ(expected, mod->to_str());
}

}  // namespace
}  // namespace transform
}  // namespace tint
