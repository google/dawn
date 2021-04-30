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

#include "src/ast/override_decoration.h"

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using OverrideDecorationTest = TestHelper;

TEST_F(OverrideDecorationTest, Creation) {
  auto* d = create<OverrideDecoration>(12);
  EXPECT_EQ(12u, d->value());
}

TEST_F(OverrideDecorationTest, Is) {
  Decoration* d = create<OverrideDecoration>(27);
  EXPECT_FALSE(d->Is<BindingDecoration>());
  EXPECT_FALSE(d->Is<BuiltinDecoration>());
  EXPECT_TRUE(d->Is<OverrideDecoration>());
  EXPECT_FALSE(d->Is<LocationDecoration>());
  EXPECT_FALSE(d->Is<GroupDecoration>());
}

TEST_F(OverrideDecorationTest, ToStr) {
  auto* d = create<OverrideDecoration>(1200);
  EXPECT_EQ(str(d), R"(OverrideDecoration{1200}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
