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

#include "src/ast/binding_decoration.h"

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using BindingDecorationTest = TestHelper;

TEST_F(BindingDecorationTest, Creation) {
  BindingDecoration d{2, Source{}};
  EXPECT_EQ(2u, d.value());
}

TEST_F(BindingDecorationTest, Is) {
  BindingDecoration d{2, Source{}};
  EXPECT_TRUE(d.Is<BindingDecoration>());
  EXPECT_FALSE(d.IsBuiltin());
  EXPECT_FALSE(d.IsConstantId());
  EXPECT_FALSE(d.IsLocation());
  EXPECT_FALSE(d.IsSet());
}

TEST_F(BindingDecorationTest, ToStr) {
  BindingDecoration d{2, Source{}};
  std::ostringstream out;
  d.to_str(out, 0);
  EXPECT_EQ(out.str(), R"(BindingDecoration{2}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
