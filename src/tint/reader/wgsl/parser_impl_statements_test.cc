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

TEST_F(ParserImplTest, Statements) {
    auto p = parser("discard; return;");
    auto e = p->expect_statements();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e->Length(), 2u);
    EXPECT_TRUE(e.value[0]->Is<ast::DiscardStatement>());
    EXPECT_TRUE(e.value[1]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, Statements_Empty) {
    auto p = parser("");
    auto e = p->expect_statements();
    EXPECT_FALSE(e.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(e->Length(), 0u);
}

}  // namespace
}  // namespace tint::reader::wgsl
