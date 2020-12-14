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

#include "src/ast/test_helper.h"
#include "src/ast/workgroup_decoration.h"

namespace tint {
namespace ast {
namespace {

using StageDecorationTest = TestHelper;

TEST_F(StageDecorationTest, Creation_1param) {
  auto* d = create<StageDecoration>(PipelineStage::kFragment);
  EXPECT_EQ(d->value(), PipelineStage::kFragment);
}

TEST_F(StageDecorationTest, Is) {
  Decoration* d = create<StageDecoration>(PipelineStage::kFragment);
  EXPECT_FALSE(d->Is<WorkgroupDecoration>());
  EXPECT_TRUE(d->Is<StageDecoration>());
}

TEST_F(StageDecorationTest, ToStr) {
  auto* d = create<StageDecoration>(PipelineStage::kFragment);
  std::ostringstream out;
  d->to_str(out, 0);
  EXPECT_EQ(out.str(), R"(StageDecoration{fragment}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
