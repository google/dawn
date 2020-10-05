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

#include "src/inspector.h"

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/type/void_type.h"
#include "src/context.h"

namespace tint {
namespace inspector {
namespace {

class InspectorHelper {
 public:
  InspectorHelper()
      : mod_(std::make_unique<ast::Module>()),
        inspector_(std::make_unique<Inspector>(*mod_)) {}

  void AddFunction(const std::string& name, ast::PipelineStage stage) {
    auto func = std::make_unique<ast::Function>(
        name, ast::VariableList{},
        ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>()));
    if (stage != ast::PipelineStage::kNone) {
      func->add_decoration(std::make_unique<ast::StageDecoration>(stage));
    }
    mod()->AddFunction(std::move(func));
  }

  ast::Module* mod() { return mod_.get(); }
  Inspector* inspector() { return inspector_.get(); }

 private:
  Context ctx_;
  std::unique_ptr<ast::Module> mod_;
  std::unique_ptr<Inspector> inspector_;
};

class InspectorTest : public InspectorHelper, public testing::Test {};

class InspectorGetEntryPointTest : public InspectorTest {};

TEST_F(InspectorGetEntryPointTest, NoFunctions) {
  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, NoEntryPoints) {
  AddFunction("foo", ast::PipelineStage::kNone);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(0u, result.size());
}

TEST_F(InspectorGetEntryPointTest, OneEntryPoint) {
  AddFunction("foo", ast::PipelineStage::kVertex);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(1u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
}

TEST_F(InspectorGetEntryPointTest, MultipleEntryPoints) {
  AddFunction("foo", ast::PipelineStage::kVertex);
  AddFunction("bar", ast::PipelineStage::kCompute);

  auto result = inspector()->GetEntryPoints();
  ASSERT_FALSE(inspector()->has_error());

  EXPECT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

TEST_F(InspectorGetEntryPointTest, MixFunctionsAndEntryPoints) {
  AddFunction("foo", ast::PipelineStage::kVertex);
  AddFunction("func", ast::PipelineStage::kNone);
  AddFunction("bar", ast::PipelineStage::kCompute);

  auto result = inspector()->GetEntryPoints();
  EXPECT_FALSE(inspector()->has_error());

  EXPECT_EQ(2u, result.size());
  EXPECT_EQ("foo", result[0].name);
  EXPECT_EQ(ast::PipelineStage::kVertex, result[0].stage);
  EXPECT_EQ("bar", result[1].name);
  EXPECT_EQ(ast::PipelineStage::kCompute, result[1].stage);
}

}  // namespace
}  // namespace inspector
}  // namespace tint
