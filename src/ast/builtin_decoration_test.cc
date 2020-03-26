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

#include "src/ast/builtin_decoration.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace {

using BuiltinDecorationTest = testing::Test;

TEST_F(BuiltinDecorationTest, Creation) {
  BuiltinDecoration d{Builtin::kFragDepth};
  EXPECT_EQ(Builtin::kFragDepth, d.value());
}

TEST_F(BuiltinDecorationTest, Is) {
  BuiltinDecoration d{Builtin::kFragDepth};
  EXPECT_FALSE(d.IsBinding());
  EXPECT_TRUE(d.IsBuiltin());
  EXPECT_FALSE(d.IsLocation());
  EXPECT_FALSE(d.IsSet());
}

TEST_F(BuiltinDecorationTest, ToStr) {
  BuiltinDecoration d{Builtin::kFragDepth};
  std::ostringstream out;
  d.to_str(out);
  EXPECT_EQ(out.str(), R"(BuiltinDecoration{frag_depth}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
