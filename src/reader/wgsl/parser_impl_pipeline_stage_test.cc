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
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

struct PipelineStageData {
  const char* input;
  ast::PipelineStage result;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageData data) {
  out << std::string(data.input);
  return out;
}

class PipelineStageTest : public testing::TestWithParam<PipelineStageData> {
 public:
  PipelineStageTest() = default;
  ~PipelineStageTest() override = default;

  void SetUp() override { ctx_.Reset(); }

  void TearDown() override { impl_ = nullptr; }

  ParserImpl* parser(const std::string& str) {
    impl_ = std::make_unique<ParserImpl>(&ctx_, str);
    return impl_.get();
  }

 private:
  std::unique_ptr<ParserImpl> impl_;
  Context ctx_;
};

TEST_P(PipelineStageTest, Parses) {
  auto params = GetParam();
  auto* p = parser(params.input);

  auto stage = p->pipeline_stage();
  ASSERT_FALSE(p->has_error());
  EXPECT_EQ(stage, params.result);

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
  auto* p = parser("not-a-stage");
  auto stage = p->pipeline_stage();
  ASSERT_EQ(stage, ast::PipelineStage::kNone);

  auto t = p->next();
  EXPECT_TRUE(t.IsIdentifier());
  EXPECT_EQ(t.to_str(), "not");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
