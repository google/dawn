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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, CaseBody_Empty) {
    auto p = parser("");
    auto e = p->case_body();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    EXPECT_EQ(e->statements.Length(), 0u);
}

TEST_F(ParserImplTest, CaseBody_Statements) {
    auto p = parser(R"(
  var a: i32;
  a = 2;)");

    auto e = p->case_body();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_FALSE(e.errored);
    EXPECT_TRUE(e.matched);
    ASSERT_EQ(e->statements.Length(), 2u);
    EXPECT_TRUE(e->statements[0]->Is<ast::VariableDeclStatement>());
    EXPECT_TRUE(e->statements[1]->Is<ast::AssignmentStatement>());
}

TEST_F(ParserImplTest, CaseBody_InvalidStatement) {
    auto p = parser("a =");
    auto e = p->case_body();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(e.errored);
    EXPECT_FALSE(e.matched);
    EXPECT_EQ(e.value, nullptr);
}

}  // namespace
}  // namespace tint::reader::wgsl
