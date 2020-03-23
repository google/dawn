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

#include <string>

#include "gtest/gtest.h"
#include "spirv/unified1/spirv.h"
#include "spirv/unified1/spirv.hpp11"
#include "src/ast/entry_point.h"
#include "src/ast/pipeline_stage.h"
#include "src/writer/spirv/builder.h"

namespace tint {
namespace writer {
namespace spirv {

using BuilderTest = testing::Test;

TEST_F(BuilderTest, EntryPoint) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  Builder b;
  b.set_func_name_to_id("frag_main", 2);
  ASSERT_TRUE(b.GenerateEntryPoint(&ep));

  auto preamble = b.preamble();
  ASSERT_EQ(preamble.size(), 1);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_TRUE(preamble[0].operands().size() >= 3);
  EXPECT_EQ(preamble[0].operands()[0].to_i(), SpvExecutionModelFragment);
  EXPECT_EQ(preamble[0].operands()[1].to_i(), 2);
  EXPECT_EQ(preamble[0].operands()[2].to_s(), "main");
}

TEST_F(BuilderTest, EntryPoint_WithoutName) {
  ast::EntryPoint ep(ast::PipelineStage::kCompute, "", "compute_main");

  Builder b;
  b.set_func_name_to_id("compute_main", 3);
  ASSERT_TRUE(b.GenerateEntryPoint(&ep));

  auto preamble = b.preamble();
  ASSERT_EQ(preamble.size(), 1);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_TRUE(preamble[0].operands().size() >= 3);
  EXPECT_EQ(preamble[0].operands()[0].to_i(), SpvExecutionModelGLCompute);
  EXPECT_EQ(preamble[0].operands()[1].to_i(), 3);
  EXPECT_EQ(preamble[0].operands()[2].to_s(), "compute_main");
}

TEST_F(BuilderTest, EntryPoint_BadFunction) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  Builder b;
  EXPECT_FALSE(b.GenerateEntryPoint(&ep));
  EXPECT_EQ(b.error(), "unable to find ID for function: frag_main");
}

struct EntryPointStageData {
  ast::PipelineStage stage;
  SpvExecutionModel model;
};
inline std::ostream& operator<<(std::ostream& out, EntryPointStageData data) {
  out << data.stage;
  return out;
}
using EntryPointStageTest = testing::TestWithParam<EntryPointStageData>;
TEST_P(EntryPointStageTest, Emit) {
  auto params = GetParam();

  ast::EntryPoint ep(params.stage, "", "main");

  Builder b;
  b.set_func_name_to_id("main", 3);
  ASSERT_TRUE(b.GenerateEntryPoint(&ep));

  auto preamble = b.preamble();
  ASSERT_EQ(preamble.size(), 1);
  EXPECT_EQ(preamble[0].opcode(), spv::Op::OpEntryPoint);

  ASSERT_TRUE(preamble[0].operands().size() >= 3);
  EXPECT_EQ(preamble[0].operands()[0].to_i(), params.model);
}
INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    EntryPointStageTest,
    testing::Values(EntryPointStageData{ast::PipelineStage::kVertex,
                                        SpvExecutionModelVertex},
                    EntryPointStageData{ast::PipelineStage::kFragment,
                                        SpvExecutionModelFragment},
                    EntryPointStageData{ast::PipelineStage::kCompute,
                                        SpvExecutionModelGLCompute}));

// TODO(http://crbug.com/tint/28)
TEST_F(BuilderTest, DISABLED_EntryPoint_WithInterfaceIds) {}

}  // namespace spirv
}  // namespace writer
}  // namespace tint
