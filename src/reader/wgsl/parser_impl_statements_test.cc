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

#include "gtest/gtest.h"
#include "src/ast/statement.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Statements) {
  auto p = parser("nop; kill; return;");
  auto e = p->statements();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e.size(), 3);
  EXPECT_TRUE(e[0]->IsNop());
  EXPECT_TRUE(e[1]->IsKill());
  EXPECT_TRUE(e[2]->IsReturn());
}

TEST_F(ParserImplTest, Statements_Empty) {
  auto p = parser("");
  auto e = p->statements();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_EQ(e.size(), 0);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
