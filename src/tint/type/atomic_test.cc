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

#include "src/tint/type/atomic.h"

#include "src/tint/type/test_helper.h"

namespace tint::type {
namespace {

using AtomicTest = TestHelper;

TEST_F(AtomicTest, Creation) {
    auto* a = create<Atomic>(create<I32>());
    auto* b = create<Atomic>(create<I32>());
    auto* c = create<Atomic>(create<U32>());
    EXPECT_TRUE(a->Type()->Is<I32>());
    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
}

TEST_F(AtomicTest, Hash) {
    auto* a = create<Atomic>(create<I32>());
    auto* b = create<Atomic>(create<I32>());
    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(AtomicTest, Equals) {
    auto* a = create<Atomic>(create<I32>());
    auto* b = create<Atomic>(create<I32>());
    auto* c = create<Atomic>(create<U32>());
    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(AtomicTest, FriendlyName) {
    auto* a = create<Atomic>(create<I32>());
    EXPECT_EQ(a->FriendlyName(), "atomic<i32>");
}

TEST_F(AtomicTest, Clone) {
    auto* atomic = create<Atomic>(create<I32>());

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* val = atomic->Clone(ctx);
    EXPECT_TRUE(val->Type()->Is<I32>());
}

}  // namespace
}  // namespace tint::type
