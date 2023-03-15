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

using EnableTest = TestHelper;

TEST_F(EnableTest, Creation) {
    auto* ext = Enable(Source{{{20, 2}, {20, 5}}}, builtin::Extension::kF16);
    EXPECT_EQ(ext->source.range.begin.line, 20u);
    EXPECT_EQ(ext->source.range.begin.column, 2u);
    EXPECT_EQ(ext->source.range.end.line, 20u);
    EXPECT_EQ(ext->source.range.end.column, 5u);
    ASSERT_EQ(ext->extensions.Length(), 1u);
    EXPECT_EQ(ext->extensions[0]->name, builtin::Extension::kF16);
}

TEST_F(EnableTest, HasExtension) {
    auto* ext = Enable(Source{{{20, 2}, {20, 5}}}, builtin::Extension::kF16);
    EXPECT_TRUE(ext->HasExtension(builtin::Extension::kF16));
    EXPECT_FALSE(ext->HasExtension(builtin::Extension::kChromiumDisableUniformityAnalysis));
}

}  // namespace
}  // namespace tint::ast
