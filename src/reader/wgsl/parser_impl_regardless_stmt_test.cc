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
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, RegardlessStmt) {
  auto p = parser("regardless (a) { kill; }");
  auto e = p->regardless_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsRegardless());
  ASSERT_NE(e->condition(), nullptr);
  EXPECT_TRUE(e->condition()->IsIdentifier());
  ASSERT_EQ(e->body().size(), 1);
  EXPECT_TRUE(e->body()[0]->IsKill());
}

TEST_F(ParserImplTest, RegardlessStmt_InvalidCondition) {
  auto p = parser("regardless(if(a){}) {}");
  auto e = p->regardless_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: unable to parse expression");
}

TEST_F(ParserImplTest, RegardlessStmt_EmptyCondition) {
  auto p = parser("regardless() {}");
  auto e = p->regardless_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:12: unable to parse expression");
}

TEST_F(ParserImplTest, RegardlessStmt_InvalidBody) {
  auto p = parser("regardless(a + 2 - 5 == true) { kill }");
  auto e = p->regardless_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:38: missing ;");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
