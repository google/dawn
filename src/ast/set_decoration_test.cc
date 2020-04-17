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

#include "src/ast/set_decoration.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using SetDecorationTest = testing::Test;

TEST_F(SetDecorationTest, Creation) {
  SetDecoration d{2};
  EXPECT_EQ(2u, d.value());
}

TEST_F(SetDecorationTest, Is) {
  SetDecoration d{2};
  EXPECT_FALSE(d.IsBinding());
  EXPECT_FALSE(d.IsBuiltin());
  EXPECT_FALSE(d.IsLocation());
  EXPECT_TRUE(d.IsSet());
}

TEST_F(SetDecorationTest, ToStr) {
  SetDecoration d{2};
  std::ostringstream out;
  d.to_str(out);
  EXPECT_EQ(out.str(), R"(SetDecoration{2}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
