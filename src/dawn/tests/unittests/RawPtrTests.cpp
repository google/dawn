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

#include <memory>
#include <set>
#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/dawn/common/Compiler.h"

namespace dawn {
namespace {

struct Base {
    virtual ~Base() = default;
    int base_val = 1;
};

struct Derived : public Base {
    int derived_val = 2;
};

#ifdef DAWN_ENABLE_PARTITION_ALLOC
// Check Dawn is configured to crash when a raw_ptr becomes dangling.
TEST(RawPtrTests, DanglingPointerCauseCrash) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool> ptr = owner.get();
    (void)ptr;  // Unused

    ASSERT_DEATH_IF_SUPPORTED(
        {
            owner.reset();  // DanglingRawPtrDetectedFn handler => no-op.
            ptr = nullptr;  // DanglingRawPtrReleasedFn handler => crash.
        },
        "(Memory was freed at:|The free stack trace wasn't recorded)(.|\n)*"
        "Dangling raw_ptr was released at:");
}
#endif

// The flag `DisableDanglingPtrDetection` must allow a raw_ptr to dangle.
TEST(RawPtrTests, DisableDanglingPtrDetection) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool, DisableDanglingPtrDetection> ptr = owner.get();
    (void)ptr;  // Unused
    owner.reset();
}

// The flag `DanglingUntriaged` must allow a raw_ptr to dangle.
TEST(RawPtrTests, DanglingUntriaged) {
    std::unique_ptr<bool> owner = std::make_unique<bool>(true);
    raw_ptr<bool, DanglingUntriaged> ptr = owner.get();
    (void)ptr;  // Unused
    owner.reset();
}

TEST(RawPtrTests, DefaultConstructor) {
    raw_ptr<int> p;
    EXPECT_EQ(p, nullptr);
    EXPECT_FALSE(p);
}

TEST(RawPtrTests, NullptrConstructor) {
    raw_ptr<int> p = nullptr;
    EXPECT_EQ(p, nullptr);
    EXPECT_FALSE(p);
}

TEST(RawPtrTests, RawPointerConstructor) {
    int val = 42;
    raw_ptr<int> p = &val;
    EXPECT_EQ(p, &val);
    EXPECT_TRUE(p);
    EXPECT_EQ(*p, 42);
}

TEST(RawPtrTests, CopyAndMove) {
    int val = 42;
    raw_ptr<int> p1 = &val;

    // Copy.
    raw_ptr<int> p2 = p1;
    EXPECT_EQ(p2, &val);
    EXPECT_EQ(p1, &val);

    // Move.
    raw_ptr<int> p3 = std::move(p2);
    EXPECT_EQ(p3, &val);
    EXPECT_EQ(p2, nullptr);
}

TEST(RawPtrTests, Assignment) {
    int val1 = 42;
    int val2 = 84;
    raw_ptr<int> p1 = &val1;
    raw_ptr<int> p2 = &val2;

    p1 = p2;
    EXPECT_EQ(p1, &val2);

    p1 = nullptr;
    EXPECT_EQ(p1, nullptr);

    p1 = &val1;
    EXPECT_EQ(p1, &val1);
}

TEST(RawPtrTests, SelfAssignment) {
    int val = 42;
    raw_ptr<int> p = &val;

    // Self-copy-assignment.
    raw_ptr<int>* p_ptr = &p;
    p = *p_ptr;
    EXPECT_EQ(p, &val);

    // Self-move-assignment.
    p = std::move(*p_ptr);
    EXPECT_EQ(p, &val);
}

TEST(RawPtrTests, Upcasting) {
    Derived derived;
    raw_ptr<Derived> p_derived = &derived;

    // Copy construct Derived -> Base.
    raw_ptr<Base> p_base(p_derived);
    EXPECT_EQ(p_base, &derived);
    EXPECT_EQ(p_base->base_val, 1);

    // Assignment Derived -> Base.
    raw_ptr<Base> p_base2;
    p_base2 = p_derived;
    EXPECT_EQ(p_base2, &derived);

    // Move construct Derived -> Base.
    raw_ptr<Derived> p_derived_move = &derived;
    raw_ptr<Base> p_base_move(std::move(p_derived_move));
    EXPECT_EQ(p_base_move, &derived);
    EXPECT_EQ(p_derived_move, nullptr);

    // Move assignment Derived -> Base.
    raw_ptr<Derived> p_derived_move2 = &derived;
    raw_ptr<Base> p_base_move2;
    p_base_move2 = std::move(p_derived_move2);
    EXPECT_EQ(p_base_move2, &derived);
    EXPECT_EQ(p_derived_move2, nullptr);
}

TEST(RawPtrTests, DereferenceAndMemberAccess) {
    struct Foo {
        int x = 42;
    };
    Foo foo;
    raw_ptr<Foo> p = &foo;
    EXPECT_EQ(p->x, 42);
    EXPECT_EQ((*p).x, 42);
    EXPECT_EQ(p, &foo);
}

TEST(RawPtrTests, PointerArithmetic) {
    int arr[] = {10, 20, 30};
    int* expected = arr;
    raw_ptr<int, AllowPtrArithmetic> p = arr;

    // Verify the initial state.
    EXPECT_EQ(p, expected);

    // Test pre-increment.
    // SAFETY: Testing raw pointer increment.
    DAWN_UNSAFE_BUFFERS(++expected);
    // SAFETY: Testing raw_ptr increment.
    raw_ptr<int, AllowPtrArithmetic>& p_ref = DAWN_UNSAFE_BUFFERS(++p);
    EXPECT_EQ(&p_ref, &p);
    EXPECT_EQ(p, expected);

    // Test post-increment.
    // SAFETY: Testing raw pointer post-increment.
    int* expected_old = DAWN_UNSAFE_BUFFERS(expected++);
    // SAFETY: Testing raw_ptr post-increment.
    raw_ptr<int, AllowPtrArithmetic> p_old = DAWN_UNSAFE_BUFFERS(p++);
    EXPECT_EQ(p_old, expected_old);
    EXPECT_EQ(p, expected);

    // Test pre-decrement.
    // SAFETY: Testing raw pointer decrement.
    DAWN_UNSAFE_BUFFERS(--expected);
    // SAFETY: Testing raw_ptr decrement.
    raw_ptr<int, AllowPtrArithmetic>& p_ref2 = DAWN_UNSAFE_BUFFERS(--p);
    EXPECT_EQ(&p_ref2, &p);
    EXPECT_EQ(p, expected);

    // Test post-decrement.
    // SAFETY: Testing raw pointer post-decrement.
    expected_old = DAWN_UNSAFE_BUFFERS(expected--);
    // SAFETY: Testing raw_ptr post-decrement.
    p_old = DAWN_UNSAFE_BUFFERS(p--);
    EXPECT_EQ(p_old, expected_old);
    EXPECT_EQ(p, expected);

    // Test addition.
    // SAFETY: Testing raw_ptr addition which is unsafe.
    int* p_plus_1 = DAWN_UNSAFE_BUFFERS(p + 1);
    // SAFETY: Testing raw pointer addition.
    int* expected_plus_1 = DAWN_UNSAFE_BUFFERS(expected + 1);
    EXPECT_EQ(p_plus_1, expected_plus_1);

    // SAFETY: Testing raw_ptr addition.
    int* p_plus_2 = DAWN_UNSAFE_BUFFERS(p + 2);
    // SAFETY: Testing raw pointer addition.
    int* expected_plus_2 = DAWN_UNSAFE_BUFFERS(expected + 2);
    EXPECT_EQ(p_plus_2, expected_plus_2);

    // Test subtraction.
    // SAFETY: Testing raw_ptr addition.
    raw_ptr<int, AllowPtrArithmetic> p2 = DAWN_UNSAFE_BUFFERS(p + 2);
    // SAFETY: Testing raw pointer addition.
    int* expected2 = DAWN_UNSAFE_BUFFERS(expected + 2);

    // SAFETY: Testing raw_ptr subtraction.
    int* p2_minus_1 = DAWN_UNSAFE_BUFFERS(p2 - 1);
    // SAFETY: Testing raw pointer subtraction.
    int* expected2_minus_1 = DAWN_UNSAFE_BUFFERS(expected2 - 1);
    EXPECT_EQ(p2_minus_1, expected2_minus_1);

    // SAFETY: Testing raw_ptr subtraction.
    int* p2_minus_2 = DAWN_UNSAFE_BUFFERS(p2 - 2);
    // SAFETY: Testing raw pointer subtraction.
    int* expected2_minus_2 = DAWN_UNSAFE_BUFFERS(expected2 - 2);
    EXPECT_EQ(p2_minus_2, expected2_minus_2);

    // Test difference.
    // SAFETY: Testing raw_ptr difference.
    ptrdiff_t p_diff = DAWN_UNSAFE_BUFFERS(p2 - p);
    // SAFETY: Testing raw pointer difference.
    ptrdiff_t expected_diff = DAWN_UNSAFE_BUFFERS(expected2 - expected);
    EXPECT_EQ(p_diff, expected_diff);
}

TEST(RawPtrTests, EqualityComparisons) {
    int val1 = 42;
    int val2 = 84;
    raw_ptr<int> p1 = &val1;
    raw_ptr<int> p2 = &val2;
    raw_ptr<int> null_p = nullptr;

    EXPECT_EQ(p1, p1);
    EXPECT_NE(p1, p2);

    EXPECT_EQ(p1, &val1);
    EXPECT_EQ(&val1, p1);
    EXPECT_NE(p1, &val2);
    EXPECT_NE(&val2, p1);

    EXPECT_EQ(null_p, nullptr);
    EXPECT_EQ(nullptr, null_p);
    EXPECT_NE(p1, nullptr);
    EXPECT_NE(nullptr, p1);
}

TEST(RawPtrTests, RelationalComparisons) {
    int arr[2] = {42, 84};
    raw_ptr<int> p0 = &arr[0];
    raw_ptr<int> p1_arr = &arr[1];

    EXPECT_LT(p0, p1_arr);
    EXPECT_LE(p0, p1_arr);
    EXPECT_GT(p1_arr, p0);
    EXPECT_GE(p1_arr, p0);

    EXPECT_FALSE(p1_arr < p0);
    EXPECT_FALSE(p1_arr <= p0);
    EXPECT_FALSE(p0 > p1_arr);
    EXPECT_FALSE(p0 >= p1_arr);

    EXPECT_LT(p0, &arr[1]);
    EXPECT_LE(p0, &arr[1]);
    EXPECT_GT(&arr[1], p0);
    EXPECT_GE(&arr[1], p0);
}

TEST(RawPtrTests, Void) {
    int val = 42;
    raw_ptr<void> p = &val;
    EXPECT_EQ(p, &val);
    EXPECT_TRUE(p);

    raw_ptr<const void> cp = &val;
    EXPECT_EQ(cp, &val);
}

TEST(RawPtrTests, Stl) {
    int a = 1;
    int b = 2;
    raw_ptr<int> pa = &a;
    raw_ptr<int> pb = &b;

    // std::set.
    std::set<raw_ptr<int>> ptr_set;
    ptr_set.insert(pa);
    ptr_set.insert(pb);
    EXPECT_EQ(ptr_set.size(), 2u);
    EXPECT_NE(ptr_set.find(pa), ptr_set.end());
    EXPECT_NE(ptr_set.find(&a), ptr_set.end());

    // std::unordered_set.
    std::unordered_set<raw_ptr<int>> ptr_uset;
    ptr_uset.insert(pa);
    ptr_uset.insert(pb);
    EXPECT_EQ(ptr_uset.size(), 2u);
    EXPECT_NE(ptr_uset.find(pa), ptr_uset.end());

    // std::to_address.
    EXPECT_EQ(std::to_address(pa), &a);
}

static_assert(sizeof(raw_ptr<int>) == sizeof(int*));

}  // anonymous namespace
}  // namespace dawn
