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

#include "src/ast/workgroup_decoration.h"

#include <sstream>

#include "src/ast/stage_decoration.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using WorkgroupDecorationTest = TestHelper;

TEST_F(WorkgroupDecorationTest, Creation_1param) {
  WorkgroupDecoration d{2, Source{}};
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = d.values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 1u);
  EXPECT_EQ(z, 1u);
}
TEST_F(WorkgroupDecorationTest, Creation_2param) {
  WorkgroupDecoration d{2, 4, Source{}};
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = d.values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 4u);
  EXPECT_EQ(z, 1u);
}

TEST_F(WorkgroupDecorationTest, Creation_3param) {
  WorkgroupDecoration d{2, 4, 6, Source{}};
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = d.values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 4u);
  EXPECT_EQ(z, 6u);
}

TEST_F(WorkgroupDecorationTest, Is) {
  WorkgroupDecoration wd{2, 4, 6, Source{}};
  Decoration* d = &wd;
  EXPECT_TRUE(wd.IsWorkgroup());
  EXPECT_FALSE(d->Is<StageDecoration>());
}

TEST_F(WorkgroupDecorationTest, ToStr) {
  WorkgroupDecoration d{2, 4, 6, Source{}};
  std::ostringstream out;
  d.to_str(out, 0);
  EXPECT_EQ(out.str(), R"(WorkgroupDecoration{2 4 6}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
