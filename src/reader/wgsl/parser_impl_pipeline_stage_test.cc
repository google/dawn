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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct PipelineStageData {
  std::string input;
  ast::PipelineStage result;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageData data) {
  return out << data.input;
}

class PipelineStageTest : public ParserImplTestWithParam<PipelineStageData> {};

TEST_P(PipelineStageTest, Parses) {
  auto params = GetParam();
  auto p = parser(params.input);

  auto stage = p->expect_pipeline_stage();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(stage.errored);
  EXPECT_EQ(stage.value, params.result);
  EXPECT_EQ(stage.source.range.begin.line, 1u);
  EXPECT_EQ(stage.source.range.begin.column, 1u);
  EXPECT_EQ(stage.source.range.end.line, 1u);
  EXPECT_EQ(stage.source.range.end.column, 1u + params.input.size());

  auto t = p->next();
  EXPECT_TRUE(t.IsEof());
}
INSTANTIATE_TEST_SUITE_P(
    ParserImplTest,
    PipelineStageTest,
    testing::Values(
        PipelineStageData{"vertex", ast::PipelineStage::kVertex},
        PipelineStageData{"fragment", ast::PipelineStage::kFragment},
        PipelineStageData{"compute", ast::PipelineStage::kCompute}));

TEST_F(ParserImplTest, PipelineStage_NoMatch) {
  auto p = parser("not-a-stage");
  auto stage = p->expect_pipeline_stage();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(stage.errored);
  ASSERT_EQ(p->error(), "1:1: invalid value for stage decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
