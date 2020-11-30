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

#include "src/ast/constant_id_decoration.h"

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using ConstantIdDecorationTest = TestHelper;

TEST_F(ConstantIdDecorationTest, Creation) {
  ConstantIdDecoration d{12, Source{}};
  EXPECT_EQ(12u, d.value());
}

TEST_F(ConstantIdDecorationTest, Is) {
  ConstantIdDecoration cd{27, Source{}};
  Decoration* d = &cd;
  EXPECT_FALSE(d->Is<BindingDecoration>());
  EXPECT_FALSE(d->Is<BuiltinDecoration>());
  EXPECT_TRUE(cd.IsConstantId());
  EXPECT_FALSE(cd.IsLocation());
  EXPECT_FALSE(cd.IsSet());
}

TEST_F(ConstantIdDecorationTest, ToStr) {
  ConstantIdDecoration d{1200, Source{}};
  std::ostringstream out;
  d.to_str(out, 0);
  EXPECT_EQ(out.str(), R"(ConstantIdDecoration{1200}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
