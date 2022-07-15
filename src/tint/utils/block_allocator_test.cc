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

#include "src/tint/utils/block_allocator.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

struct LifetimeCounter {
    explicit LifetimeCounter(size_t* count) : count_(count) { (*count)++; }
    ~LifetimeCounter() { (*count_)--; }

    size_t* const count_;
};

using BlockAllocatorTest = testing::Test;

TEST_F(BlockAllocatorTest, Empty) {
    using Allocator = BlockAllocator<int>;

    Allocator allocator;

    EXPECT_EQ(allocator.Count(), 0u);
    for (int* i : allocator.Objects()) {
        (void)i;
        if ((true)) {  // Workaround for "error: loop will run at most once"
            FAIL() << "BlockAllocator should be empty";
        }
    }
    for (const int* i : static_cast<const Allocator&>(allocator).Objects()) {
        (void)i;
        if ((true)) {  // Workaround for "error: loop will run at most once"
            FAIL() << "BlockAllocator should be empty";
        }
    }
}

TEST_F(BlockAllocatorTest, Count) {
    using Allocator = BlockAllocator<int>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        Allocator allocator;
        EXPECT_EQ(allocator.Count(), 0u);
        for (size_t i = 0; i < n; i++) {
            allocator.Create(123);
        }
        EXPECT_EQ(allocator.Count(), n);
    }
}

TEST_F(BlockAllocatorTest, ObjectLifetime) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    size_t count = 0;
    {
        Allocator allocator;
        EXPECT_EQ(count, 0u);
        allocator.Create(&count);
        EXPECT_EQ(count, 1u);
        allocator.Create(&count);
        EXPECT_EQ(count, 2u);
        allocator.Create(&count);
        EXPECT_EQ(count, 3u);
    }
    EXPECT_EQ(count, 0u);
}

TEST_F(BlockAllocatorTest, MoveConstruct) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        size_t count = 0;
        {
            Allocator allocator_a;
            for (size_t i = 0; i < n; i++) {
                allocator_a.Create(&count);
            }
            EXPECT_EQ(count, n);
            EXPECT_EQ(allocator_a.Count(), n);

            Allocator allocator_b{std::move(allocator_a)};
            EXPECT_EQ(count, n);
            EXPECT_EQ(allocator_b.Count(), n);
        }

        EXPECT_EQ(count, 0u);
    }
}

TEST_F(BlockAllocatorTest, MoveAssign) {
    using Allocator = BlockAllocator<LifetimeCounter>;

    for (size_t n : {0u, 1u, 10u, 16u, 20u, 32u, 50u, 64u, 100u, 256u, 300u, 512u, 500u, 512u}) {
        size_t count_a = 0;
        size_t count_b = 0;

        {
            Allocator allocator_a;
            for (size_t i = 0; i < n; i++) {
                allocator_a.Create(&count_a);
            }
            EXPECT_EQ(count_a, n);
            EXPECT_EQ(allocator_a.Count(), n);

            Allocator allocator_b;
            for (size_t i = 0; i < n; i++) {
                allocator_b.Create(&count_b);
            }
            EXPECT_EQ(count_b, n);
            EXPECT_EQ(allocator_b.Count(), n);

            allocator_b = std::move(allocator_a);
            EXPECT_EQ(count_a, n);
            EXPECT_EQ(count_b, 0u);
            EXPECT_EQ(allocator_b.Count(), n);
        }

        EXPECT_EQ(count_a, 0u);
        EXPECT_EQ(count_b, 0u);
    }
}

TEST_F(BlockAllocatorTest, ObjectOrder) {
    using Allocator = BlockAllocator<int>;

    Allocator allocator;
    constexpr int N = 10000;
    for (int i = 0; i < N; i++) {
        allocator.Create(i);
    }

    {
        int i = 0;
        for (int* p : allocator.Objects()) {
            EXPECT_EQ(*p, i);
            i++;
        }
        EXPECT_EQ(i, N);
    }
    {
        int i = 0;
        for (const int* p : static_cast<const Allocator&>(allocator).Objects()) {
            EXPECT_EQ(*p, i);
            i++;
        }
        EXPECT_EQ(i, N);
    }
}

}  // namespace
}  // namespace tint::utils
