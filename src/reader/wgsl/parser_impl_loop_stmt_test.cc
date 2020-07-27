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

TEST_F(ParserImplTest, LoopStmt_BodyNoContinuing) {
  auto* p = parser("loop { discard; }");
  auto e = p->loop_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_EQ(e->body()->size(), 1u);
  EXPECT_TRUE(e->body()->get(0)->IsDiscard());

  EXPECT_EQ(e->continuing()->size(), 0u);
}

TEST_F(ParserImplTest, LoopStmt_BodyWithContinuing) {
  auto* p = parser("loop { discard; continuing { discard; }}");
  auto e = p->loop_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  ASSERT_EQ(e->body()->size(), 1u);
  EXPECT_TRUE(e->body()->get(0)->IsDiscard());

  EXPECT_EQ(e->continuing()->size(), 1u);
  EXPECT_TRUE(e->continuing()->get(0)->IsDiscard());
}

TEST_F(ParserImplTest, LoopStmt_NoBodyNoContinuing) {
  auto* p = parser("loop { }");
  auto e = p->loop_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_EQ(e->body()->size(), 0u);
  ASSERT_EQ(e->continuing()->size(), 0u);
}

TEST_F(ParserImplTest, LoopStmt_NoBodyWithContinuing) {
  auto* p = parser("loop { continuing { discard; }}");
  auto e = p->loop_stmt();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_EQ(e->body()->size(), 0u);
  ASSERT_EQ(e->continuing()->size(), 1u);
  EXPECT_TRUE(e->continuing()->get(0)->IsDiscard());
}

TEST_F(ParserImplTest, LoopStmt_MissingBracketLeft) {
  auto* p = parser("loop discard; }");
  auto e = p->loop_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:6: missing { for loop");
}

TEST_F(ParserImplTest, LoopStmt_MissingBracketRight) {
  auto* p = parser("loop { discard; ");
  auto e = p->loop_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:17: missing } for loop");
}

TEST_F(ParserImplTest, LoopStmt_InvalidStatements) {
  auto* p = parser("loop { discard }");
  auto e = p->loop_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: missing ;");
}

TEST_F(ParserImplTest, LoopStmt_InvalidContinuing) {
  auto* p = parser("loop { continuing { discard }}");
  auto e = p->loop_stmt();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:29: missing ;");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
