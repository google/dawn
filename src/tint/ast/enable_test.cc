// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/enable.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstExtensionTest = TestHelper;

TEST_F(AstExtensionTest, Creation) {
    auto* ext =
        create<Enable>(Source{Source::Range{Source::Location{20, 2}, Source::Location{20, 5}}},
                       "InternalExtensionForTesting");
    EXPECT_EQ(ext->source.range.begin.line, 20u);
    EXPECT_EQ(ext->source.range.begin.column, 2u);
    EXPECT_EQ(ext->source.range.end.line, 20u);
    EXPECT_EQ(ext->source.range.end.column, 5u);
    EXPECT_EQ(ext->kind, ast::Enable::ExtensionKind::kInternalExtensionForTesting);
}

TEST_F(AstExtensionTest, Creation_InvalidName) {
    auto* ext = create<Enable>(
        Source{Source::Range{Source::Location{20, 2}, Source::Location{20, 5}}}, std::string());
    EXPECT_EQ(ext->source.range.begin.line, 20u);
    EXPECT_EQ(ext->source.range.begin.column, 2u);
    EXPECT_EQ(ext->source.range.end.line, 20u);
    EXPECT_EQ(ext->source.range.end.column, 5u);
    EXPECT_EQ(ext->kind, ast::Enable::ExtensionKind::kNotAnExtension);
}

TEST_F(AstExtensionTest, NameToKind_InvalidName) {
    EXPECT_EQ(ast::Enable::NameToKind(std::string()), ast::Enable::ExtensionKind::kNotAnExtension);
    EXPECT_EQ(ast::Enable::NameToKind("__ImpossibleExtensionName"),
              ast::Enable::ExtensionKind::kNotAnExtension);
    EXPECT_EQ(ast::Enable::NameToKind("123"), ast::Enable::ExtensionKind::kNotAnExtension);
}

TEST_F(AstExtensionTest, KindToName) {
    EXPECT_EQ(ast::Enable::KindToName(ast::Enable::ExtensionKind::kInternalExtensionForTesting),
              "InternalExtensionForTesting");
    EXPECT_EQ(ast::Enable::KindToName(ast::Enable::ExtensionKind::kNotAnExtension), std::string());
}

}  // namespace
}  // namespace tint::ast
