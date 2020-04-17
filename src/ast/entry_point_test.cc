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

#include "src/ast/entry_point.h"

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using EntryPointTest = testing::Test;

TEST_F(EntryPointTest, Creation) {
  EntryPoint e(PipelineStage::kVertex, "main", "vtx_main");

  EXPECT_EQ(e.name(), "main");
  EXPECT_EQ(e.function_name(), "vtx_main");
  EXPECT_EQ(e.stage(), PipelineStage::kVertex);
  EXPECT_EQ(e.line(), 0u);
  EXPECT_EQ(e.column(), 0u);
}

TEST_F(EntryPointTest, CreationWithSource) {
  Source s{27, 4};
  EntryPoint e(s, PipelineStage::kVertex, "main", "vtx_main");

  EXPECT_EQ(e.name(), "main");
  EXPECT_EQ(e.function_name(), "vtx_main");
  EXPECT_EQ(e.stage(), PipelineStage::kVertex);
  EXPECT_EQ(e.line(), 27u);
  EXPECT_EQ(e.column(), 4u);
}

TEST_F(EntryPointTest, CreationEmpty) {
  Source s{27, 4};
  EntryPoint e;
  e.set_source(s);
  e.set_pipeline_stage(PipelineStage::kFragment);
  e.set_function_name("my_func");
  e.set_name("a_name");

  EXPECT_EQ(e.function_name(), "my_func");
  EXPECT_EQ(e.name(), "a_name");
  EXPECT_EQ(e.stage(), PipelineStage::kFragment);
  EXPECT_EQ(e.line(), 27u);
  EXPECT_EQ(e.column(), 4u);
}

TEST_F(EntryPointTest, IsValid) {
  EntryPoint e(PipelineStage::kVertex, "", "vtx_main");
  EXPECT_TRUE(e.IsValid());
}

TEST_F(EntryPointTest, IsValid_MissingFunctionName) {
  EntryPoint e(PipelineStage::kVertex, "main", "");
  EXPECT_FALSE(e.IsValid());
}

TEST_F(EntryPointTest, IsValid_MissingStage) {
  EntryPoint e(PipelineStage::kNone, "main", "fn");
  EXPECT_FALSE(e.IsValid());
}

TEST_F(EntryPointTest, IsValid_MissingBoth) {
  EntryPoint e;
  EXPECT_FALSE(e.IsValid());
}

TEST_F(EntryPointTest, ToStr) {
  EntryPoint e(PipelineStage::kVertex, "text", "vtx_main");
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  EntryPoint{vertex as text = vtx_main}
)");
}

TEST_F(EntryPointTest, ToStr_NoName) {
  EntryPoint e(PipelineStage::kVertex, "", "vtx_main");
  std::ostringstream out;
  e.to_str(out, 2);
  EXPECT_EQ(out.str(), R"(  EntryPoint{vertex = vtx_main}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
