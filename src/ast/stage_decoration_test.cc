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

#include "src/ast/stage_decoration.h"

#include <sstream>

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using StageDecorationTest = testing::Test;

TEST_F(StageDecorationTest, Creation_1param) {
  StageDecoration d{ast::PipelineStage::kFragment};
  EXPECT_EQ(d.value(), ast::PipelineStage::kFragment);
}

TEST_F(StageDecorationTest, Is) {
  StageDecoration d{ast::PipelineStage::kFragment};
  EXPECT_FALSE(d.IsWorkgroup());
  EXPECT_TRUE(d.IsStage());
}

TEST_F(StageDecorationTest, ToStr) {
  StageDecoration d{ast::PipelineStage::kFragment};
  std::ostringstream out;
  d.to_str(out);
  EXPECT_EQ(out.str(), R"(StageDecoration{fragment}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
