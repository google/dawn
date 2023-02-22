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

#include "src/tint/utils/slice.h"

#include "gmock/gmock.h"

namespace tint::utils {
namespace {

class C0 : public Castable<C0> {};
class C1 : public Castable<C1, C0> {};
class C2a : public Castable<C2a, C1> {};
class C2b : public Castable<C2b, C1> {};

////////////////////////////////////////////////////////////////////////////////
// Static asserts
////////////////////////////////////////////////////////////////////////////////
// Non-pointer
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int, int>, "same type");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int const, int>, "apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int, int const>, "remove const");

// Non-castable pointers
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int* const, int*>, "apply ptr const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int*, int* const>, "remove ptr const");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, int const*, int*>, "apply el const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, int*, int const*>, "remove el const");

// Castable
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, C0*>, "apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C0*, const C0*>, "remove const");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, C0*, C1*>, "up cast");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, const C1*>, "up cast");
static_assert(CanReinterpretSlice<ReinterpretMode::kSafe, const C0*, C1*>, "up cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C0*, const C1*>,
              "up cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, C0*>, "down cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, const C0*>, "down cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, C0*>,
              "down cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, const C0*>,
              "down cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C1*, C0*>,
              "down cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C1*, const C0*>,
              "down cast, remove const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C2a*, C2b*>, "sideways cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C2a*, const C2b*>,
              "sideways cast");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, const C2a*, C2b*>,
              "sideways cast, apply const");
static_assert(!CanReinterpretSlice<ReinterpretMode::kSafe, C2a*, const C2b*>,
              "sideways cast, remove const");

TEST(TintSliceTest, Ctor) {
    Slice<int> slice;
    EXPECT_EQ(slice.data, nullptr);
    EXPECT_EQ(slice.len, 0u);
    EXPECT_EQ(slice.cap, 0u);
    EXPECT_TRUE(slice.IsEmpty());
}

TEST(TintSliceTest, CtorEmpty) {
    Slice<int> slice{Empty};
    EXPECT_EQ(slice.data, nullptr);
    EXPECT_EQ(slice.len, 0u);
    EXPECT_EQ(slice.cap, 0u);
    EXPECT_TRUE(slice.IsEmpty());
}

TEST(TintSliceTest, CtorCArray) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    EXPECT_EQ(slice.data, elements);
    EXPECT_EQ(slice.len, 3u);
    EXPECT_EQ(slice.cap, 3u);
    EXPECT_FALSE(slice.IsEmpty());
}

TEST(TintSliceTest, Index) {
    int elements[] = {1, 2, 3};

    auto slice = Slice{elements};
    EXPECT_EQ(slice[0], 1);
    EXPECT_EQ(slice[1], 2);
    EXPECT_EQ(slice[2], 3);
}

TEST(TintSliceTest, Front) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_EQ(slice.Front(), 1);
}

TEST(TintSliceTest, Back) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_EQ(slice.Back(), 3);
}

TEST(TintSliceTest, BeginEnd) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    EXPECT_THAT(slice, testing::ElementsAre(1, 2, 3));
}

TEST(TintSliceTest, ReverseBeginEnd) {
    int elements[] = {1, 2, 3};
    auto slice = Slice{elements};
    size_t i = 0;
    for (auto it = slice.rbegin(); it != slice.rend(); it++) {
        EXPECT_EQ(*it, elements[2 - i]);
        i++;
    }
}

}  // namespace
}  // namespace tint::utils
