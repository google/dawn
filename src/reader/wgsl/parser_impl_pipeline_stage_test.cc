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
#include "src/ast/pipeline_stage.h"
#include "src/reader/wgsl/parser_impl.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

struct PipelineStageData {
  const char* input;
  ast::PipelineStage result;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageData data) {
  out << std::string(data.input);
  return out;
}
using PipelineStageTest = testing::TestWithParam<PipelineStageData>;
TEST_P(PipelineStageTest, Parses) {
  auto params = GetParam();
  ParserImpl p{params.input};

  auto stage = p.pipeline_stage();
  ASSERT_FALSE(p.has_error());
  EXPECT_EQ(stage, params.result);

  auto t = p.next();
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
  ParserImpl p{"not-a-stage"};
  auto stage = p.pipeline_stage();
  ASSERT_EQ(stage, ast::PipelineStage::kNone);

  auto t = p.next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.to_str(), "not");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
