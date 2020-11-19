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

#include "src/ast/access_decoration.h"

#include <sstream>

#include "src/ast/test_helper.h"

namespace tint {
namespace ast {
namespace {

using AccessDecorationTest = TestHelper;

TEST_F(AccessDecorationTest, Creation) {
  AccessDecoration d{AccessControl::kWriteOnly, Source{}};
  EXPECT_EQ(AccessControl::kWriteOnly, d.value());
}

TEST_F(AccessDecorationTest, Is) {
  AccessDecoration d{AccessControl::kReadWrite, Source{}};
  EXPECT_FALSE(d.IsAccess());
}

TEST_F(AccessDecorationTest, ToStr) {
  AccessDecoration d{AccessControl::kReadOnly, Source{}};
  std::ostringstream out;
  d.to_str(out, 0);
  EXPECT_EQ(out.str(), R"(AccessDecoration{read}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
