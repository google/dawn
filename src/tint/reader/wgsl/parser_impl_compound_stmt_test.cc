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

#include "src/tint/ast/discard_statement.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, CompoundStmt) {
    auto p = parser(R"({
  discard;
  return 1 + b / 2;
})");
    auto e = p->expect_compound_statement("");

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 4u);
    EXPECT_EQ(e->source.range.end.column, 2u);

    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_EQ(e->statements.Length(), 2u);
    EXPECT_TRUE(e->statements[0]->Is<ast::DiscardStatement>());
    EXPECT_TRUE(e->statements[1]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, CompoundStmt_Empty) {
    auto p = parser("{}");
    auto e = p->expect_compound_statement("");

    EXPECT_EQ(e->source.range.begin.line, 1u);
    EXPECT_EQ(e->source.range.begin.column, 1u);
    EXPECT_EQ(e->source.range.end.line, 1u);
    EXPECT_EQ(e->source.range.end.column, 3u);

    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e->statements.Length(), 0u);
}

TEST_F(ParserImplTest, CompoundStmt_InvalidStmt) {
    auto p = parser("{fn main() {}}");
    auto e = p->expect_compound_statement("");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:2: expected '}'");
}

TEST_F(ParserImplTest, CompoundStmt_MissingRightParen) {
    auto p = parser("{return;");
    auto e = p->expect_compound_statement("");
    ASSERT_TRUE(p->has_error());
    ASSERT_TRUE(e.errored);
    EXPECT_EQ(p->error(), "1:9: expected '}'");
}

}  // namespace
}  // namespace tint::reader::wgsl
