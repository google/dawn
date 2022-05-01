// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/atomic.h"

#include "src/tint/ast/i32.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using AstAtomicTest = TestHelper;

TEST_F(AstAtomicTest, Creation) {
    auto* i32 = create<I32>();
    auto* p = create<Atomic>(i32);
    EXPECT_EQ(p->type, i32);
}

TEST_F(AstAtomicTest, FriendlyName) {
    auto* i32 = create<I32>();
    auto* p = create<Atomic>(i32);
    EXPECT_EQ(p->FriendlyName(Symbols()), "atomic<i32>");
}

}  // namespace
}  // namespace tint::ast
