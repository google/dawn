// Copyright 2023 The Tint Authors.
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

using RequiresDirectiveTest = ParserImplTest;

// Test a valid require directive.
// There currently are no valid require directives
TEST_F(RequiresDirectiveTest, DISABLED_Valid) {
    auto p = parser("requires <sometime>;");
    p->requires_directive();
    EXPECT_FALSE(p->has_error()) << p->error();
}

// Test an unknown require identifier.
TEST_F(RequiresDirectiveTest, InvalidIdentifier) {
    auto p = parser("requires NotAValidRequireName;");
    p->requires_directive();
    // Error when unknown require found
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:10: feature 'NotAValidRequireName' is not supported)");
}

// Test the special error message when require are used with parenthesis.
TEST_F(RequiresDirectiveTest, ParenthesisSpecialCase) {
    auto p = parser("requires(Something);");
    p->translation_unit();
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:9: requires directives don't take parenthesis");
}

// Test using invalid tokens in an require directive.
TEST_F(RequiresDirectiveTest, InvalidTokens) {
    {
        auto p = parser("requires <Something;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:10: invalid feature name for requires)");
    }
    {
        auto p = parser("requires =;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:10: invalid feature name for requires)");
    }

    {
        auto p = parser("requires;");
        p->translation_unit();
        EXPECT_TRUE(p->has_error());
        EXPECT_EQ(p->error(), R"(1:9: missing feature names in requires directive)");
    }
}

}  // namespace
}  // namespace tint::reader::wgsl
