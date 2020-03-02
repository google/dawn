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

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, LoopStmt_BodyNoContinuing) {
  ParserImpl p{"loop { nop; }"};
  auto e = p.loop_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_EQ(e->body().size(), 1);
  EXPECT_TRUE(e->body()[0]->IsNop());

  EXPECT_EQ(e->continuing().size(), 0);
}

TEST_F(ParserImplTest, LoopStmt_BodyWithContinuing) {
  ParserImpl p{"loop { nop; continuing { kill; }}"};
  auto e = p.loop_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);

  ASSERT_EQ(e->body().size(), 1);
  EXPECT_TRUE(e->body()[0]->IsNop());

  EXPECT_EQ(e->continuing().size(), 1);
  EXPECT_TRUE(e->continuing()[0]->IsKill());
}

TEST_F(ParserImplTest, LoopStmt_NoBodyNoContinuing) {
  ParserImpl p{"loop { }"};
  auto e = p.loop_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_EQ(e->body().size(), 0);
  ASSERT_EQ(e->continuing().size(), 0);
}

TEST_F(ParserImplTest, LoopStmt_NoBodyWithContinuing) {
  ParserImpl p{"loop { continuing { kill; }}"};
  auto e = p.loop_stmt();
  ASSERT_FALSE(p.has_error()) << p.error();
  ASSERT_NE(e, nullptr);
  ASSERT_EQ(e->body().size(), 0);
  ASSERT_EQ(e->continuing().size(), 1);
  EXPECT_TRUE(e->continuing()[0]->IsKill());
}

TEST_F(ParserImplTest, LoopStmt_MissingBracketLeft) {
  ParserImpl p{"loop kill; }"};
  auto e = p.loop_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:6: missing { for loop");
}

TEST_F(ParserImplTest, LoopStmt_MissingBracketRight) {
  ParserImpl p{"loop { kill; "};
  auto e = p.loop_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:14: missing } for loop");
}

TEST_F(ParserImplTest, LoopStmt_InvalidStatements) {
  ParserImpl p{"loop { kill }"};
  auto e = p.loop_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:13: missing ;");
}

TEST_F(ParserImplTest, LoopStmt_InvalidContinuing) {
  ParserImpl p{"loop { continuing { kill }}"};
  auto e = p.loop_stmt();
  ASSERT_TRUE(p.has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p.error(), "1:26: missing ;");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
