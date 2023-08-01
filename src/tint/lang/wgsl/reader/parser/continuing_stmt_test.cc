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

#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, ContinuingStmt) {
    auto p = parser("continuing { discard; }");
    auto e = p->continuing_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e->statements.Length(), 1u);
    ASSERT_TRUE(e->statements[0]->Is<ast::DiscardStatement>());
}

TEST_F(WGSLParserTest, ContinuingStmt_WithAttributes) {
    auto p = parser("continuing @diagnostic(off, derivative_uniformity) { discard; }");
    auto e = p->continuing_statement();
    EXPECT_TRUE(e.matched);
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e->attributes.Length(), 1u);
    EXPECT_TRUE(e->attributes[0]->Is<ast::DiagnosticAttribute>());
}

TEST_F(WGSLParserTest, ContinuingStmt_InvalidBody) {
    auto p = parser("continuing { discard }");
    auto e = p->continuing_statement();
    EXPECT_FALSE(e.matched);
    EXPECT_TRUE(e.errored);
    EXPECT_EQ(e.value, nullptr);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:22: expected ';' for discard statement");
}

}  // namespace
}  // namespace tint::wgsl::reader
