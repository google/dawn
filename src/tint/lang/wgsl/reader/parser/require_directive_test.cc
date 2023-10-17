// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

namespace tint::wgsl::reader {
namespace {

using RequiresDirectiveTest = WGSLParserTest;

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
}  // namespace tint::wgsl::reader
