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

#include "src/ast/constant_id_decoration.h"
#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using BuiltinDecorationTest = TestHelper;

TEST_F(BuiltinDecorationTest, Creation) {
  BuiltinDecoration d{Builtin::kFragDepth, Source{}};
  EXPECT_EQ(Builtin::kFragDepth, d.value());
}

TEST_F(BuiltinDecorationTest, Is) {
  BuiltinDecoration bd{Builtin::kFragDepth, Source{}};
  Decoration* d = &bd;
  EXPECT_FALSE(d->Is<BindingDecoration>());
  EXPECT_TRUE(d->Is<BuiltinDecoration>());
  EXPECT_FALSE(d->Is<ConstantIdDecoration>());
  EXPECT_FALSE(d->Is<LocationDecoration>());
  EXPECT_FALSE(bd.IsSet());
}

TEST_F(BuiltinDecorationTest, ToStr) {
  BuiltinDecoration d{Builtin::kFragDepth, Source{}};
  std::ostringstream out;
  d.to_str(out, 0);
  EXPECT_EQ(out.str(), R"(BuiltinDecoration{frag_depth}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
