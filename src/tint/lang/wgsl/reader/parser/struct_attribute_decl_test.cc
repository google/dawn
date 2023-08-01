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

#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

TEST_F(WGSLParserTest, AttributeDecl_Parses) {
    auto p = parser("@invariant");
    auto attrs = p->attribute_list();
    EXPECT_FALSE(p->has_error());
    EXPECT_FALSE(attrs.errored);
    EXPECT_TRUE(attrs.matched);
    ASSERT_EQ(attrs.value.Length(), 1u);
    auto* invariant = attrs.value[0]->As<ast::Attribute>();
    EXPECT_TRUE(invariant->Is<ast::InvariantAttribute>());
}

TEST_F(WGSLParserTest, AttributeDecl_MissingParenLeft) {
    auto p = parser("@location 1)");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:11: expected '(' for location attribute");
}

TEST_F(WGSLParserTest, AttributeDecl_MissingValue) {
    auto p = parser("@location()");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:2: location expects 1 argument");
}

TEST_F(WGSLParserTest, AttributeDecl_MissingParenRight) {
    auto p = parser("@location(1");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
    EXPECT_EQ(p->error(), "1:12: expected ')' for location attribute");
}

TEST_F(WGSLParserTest, AttributeDecl_Invalidattribute) {
    auto p = parser("@invalid");
    auto attrs = p->attribute_list();
    EXPECT_TRUE(p->has_error());
    EXPECT_TRUE(attrs.errored);
    EXPECT_FALSE(attrs.matched);
    EXPECT_TRUE(attrs.value.IsEmpty());
}

}  // namespace
}  // namespace tint::wgsl::reader
