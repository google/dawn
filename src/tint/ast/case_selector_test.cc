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

#include "src/tint/ast/case_selector.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using CaseSelectorTest = TestHelper;

TEST_F(CaseSelectorTest, NonDefault) {
    auto* e = Expr(2_i);
    auto* c = CaseSelector(e);
    EXPECT_FALSE(c->IsDefault());
    EXPECT_EQ(e, c->expr);
}

TEST_F(CaseSelectorTest, Default) {
    auto* c = DefaultCaseSelector();
    EXPECT_TRUE(c->IsDefault());
}

}  // namespace
}  // namespace tint::ast
