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

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using StrideAttributeTest = TestHelper;

TEST_F(StrideAttributeTest, Creation) {
    auto* d = create<StrideAttribute>(2u);
    EXPECT_EQ(2u, d->stride);
}

TEST_F(StrideAttributeTest, Source) {
    auto* d = create<StrideAttribute>(
        Source{Source::Range{Source::Location{1, 2}, Source::Location{3, 4}}}, 2u);
    EXPECT_EQ(d->source.range.begin.line, 1u);
    EXPECT_EQ(d->source.range.begin.column, 2u);
    EXPECT_EQ(d->source.range.end.line, 3u);
    EXPECT_EQ(d->source.range.end.column, 4u);
}

}  // namespace
}  // namespace tint::ast
