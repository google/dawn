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
  auto* d = create<AccessDecoration>(ast::AccessControl::kWriteOnly);
  EXPECT_EQ(ast::AccessControl::kWriteOnly, d->value());
}

TEST_F(AccessDecorationTest, Is) {
  auto* d = create<AccessDecoration>(ast::AccessControl::kReadWrite);
  EXPECT_TRUE(d->Is<ast::AccessDecoration>());
}

TEST_F(AccessDecorationTest, ToStr) {
  auto* d = create<AccessDecoration>(ast::AccessControl::kReadOnly);
  std::ostringstream out;
  d->to_str(Sem(), out, 0);
  EXPECT_EQ(out.str(), R"(AccessDecoration{read_only}
)");
}

}  // namespace
}  // namespace ast
}  // namespace tint
