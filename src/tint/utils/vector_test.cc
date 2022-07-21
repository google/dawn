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

#include "src/tint/utils/vector.h"

#include <string>
#include <tuple>

#include "gtest/gtest.h"

#include "src/tint/utils/bitcast.h"

namespace tint::utils {
namespace {

/// @returns true if the address of el is within the memory of the vector vec.
template <typename T, size_t N, typename E>
bool IsInternal(Vector<T, N>& vec, E& el) {
    auto ptr = Bitcast<uintptr_t>(&el);
    auto base = Bitcast<uintptr_t>(&vec);
    return ptr >= base && ptr < base + sizeof(vec);
}

/// @returns true if all elements of the vector `vec` are held within the memory of `vec`.
template <typename T, size_t N>
bool AllInternallyHeld(Vector<T, N>& vec) {
    for (auto& el : vec) {
        if (!IsInternal(vec, el)) {
            return false;
        }
    }
    return true;
}

/// @returns true if all elements of the vector `vec` are held outside the memory of `vec`.
template <typename T, size_t N>
bool AllExternallyHeld(Vector<T, N>& vec) {
    for (auto& el : vec) {
        if (IsInternal(vec, el)) {
            return false;
        }
    }
    return true;
}

TEST(TintVectorTest, SmallArray_Empty) {
    Vector<int, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
}

TEST(TintVectorTest, Empty_NoSmallArray) {
    Vector<int> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
}

TEST(TintVectorTest, SmallArray_ConstructLength_NoSpill) {
    Vector<int, 2> vec(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_ConstructLength_WithSpill) {
    Vector<int, 2> vec(3);
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec[2], 0);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_ConstructLengthValue_NoSpill) {
    Vector<std::string, 2> vec(2, "abc");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "abc");
    EXPECT_EQ(vec[1], "abc");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_ConstructLengthValue_WithSpill) {
    Vector<std::string, 2> vec(3, "abc");
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "abc");
    EXPECT_EQ(vec[1], "abc");
    EXPECT_EQ(vec[2], "abc");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, ConstructLength_NoSmallArray) {
    Vector<int> vec(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 0);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, ConstructInitializerList_NoSpill) {
    Vector<std::string, 2> vec{"one", "two"};
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, ConstructInitializerList_WithSpill) {
    Vector<std::string, 2> vec{"one", "two", "three"};
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_EQ(vec[2], "three");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, ConstructInitializerList_NoSmallArray) {
    Vector<std::string> vec{"one", "two"};
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, CopyCtor_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyCtor_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyCtor_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyCtor_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyCtor_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyCtor_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b{vec_a};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveCtor_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b{std::move(vec_a)};
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_SpillSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_Self_NoSpill) {
    Vector<std::string, 2> vec{"hello", "world"};
    auto* vec_ptr = &vec;  // Used to avoid -Wself-assign-overloaded
    vec = *vec_ptr;
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, CopyAssign_Self_WithSpill) {
    Vector<std::string, 1> vec{"hello", "world"};
    auto* vec_ptr = &vec;  // Used to avoid -Wself-assign-overloaded
    vec = *vec_ptr;
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, MoveAssign_Self_NoSpill) {
    Vector<std::string, 2> vec{"hello", "world"};
    auto* vec_ptr = &vec;  // Used to avoid -Wself-move
    vec = std::move(*vec_ptr);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, MoveAssign_Self_WithSpill) {
    Vector<std::string, 1> vec{"hello", "world"};
    auto* vec_ptr = &vec;  // Used to avoid -Wself-move
    vec = std::move(*vec_ptr);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, RepeatMoveAssign_NoSpill) {
    Vector<std::string, 3> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{"Ciao", "mondo"};
    Vector<std::string, 3> vec_c{"Bonjour", "le", "monde"};
    Vector<std::string, 3> vec;
    vec = std::move(vec_a);
    vec = std::move(vec_b);
    vec = std::move(vec_c);
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "Bonjour");
    EXPECT_EQ(vec[1], "le");
    EXPECT_EQ(vec[2], "monde");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, DoubleMoveAssign_WithSpill) {
    Vector<std::string, 1> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{"Ciao", "mondo"};
    Vector<std::string, 1> vec_c{"bonjour", "le", "monde"};
    Vector<std::string, 1> vec;
    vec = std::move(vec_a);
    vec = std::move(vec_b);
    vec = std::move(vec_c);
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "bonjour");
    EXPECT_EQ(vec[1], "le");
    EXPECT_EQ(vec[2], "monde");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Index) {
    Vector<std::string, 2> vec{"hello", "world"};
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec[0])>>);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
}

TEST(TintVectorTest, ConstIndex) {
    const Vector<std::string, 2> vec{"hello", "world"};
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec[0])>>);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
}

TEST(TintVectorTest, SmallArray_Reserve_NoSpill) {
    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Reserve(2);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Push("hello");
    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_Reserve_WithSpill) {
    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 1u);
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 1u);
    vec.Reserve(2);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Push("hello");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_Resize_NoSpill) {
    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[0] = "hello";
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[1] = "world";
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, SmallArray_Resize_WithSpill) {
    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 1u);
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_EQ(vec[0], "");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[0] = "hello";
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec[1] = "world";
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Reserve_NoSmallArray) {
    Vector<std::string> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 1u);
    vec.Reserve(2);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Push("hello");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Reserve(1);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Resize_NoSmallArray) {
    Vector<std::string> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_EQ(vec[0], "");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec[0] = "hello";
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec[1] = "world";
    vec.Resize(1);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Resize(2);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N2_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N2_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b{"hallo", "wereld"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N2_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b{"hallo", "wereld", "spill"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N2_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N2_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b{"hallo", "wereld"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N2_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b{"hallo", "wereld", "morsen"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N1_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N1_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{"hallo"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N1_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{"hallo", "morsen"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N1_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N1_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b{"hallo"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N1_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b{"hallo", "wereld"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N3_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N3_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{"hallo", "fijne", "wereld"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_NoSpill_N2_to_N3_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{"hallo", "fijne", "wereld", "morsen"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 4u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N3_Empty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b;
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N3_NonEmpty) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b{"hallo", "fijne", "wereld"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, Copy_WithSpill_N2_to_N3_Spill) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b{"hallo", "fijne", "wereld", "morsen"};
    vec_b.Copy(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 4u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, Clear_Empty) {
    Vector<std::string, 2> vec;
    vec.Clear();
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
}

TEST(TintVectorTest, Clear_NoSpill) {
    Vector<std::string, 2> vec{"hello", "world"};
    vec.Clear();
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
}

TEST(TintVectorTest, Clear_WithSpill) {
    Vector<std::string, 2> vec{"hello", "world", "spill"};
    vec.Clear();
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 3u);
}

TEST(TintVectorTest, PushPop_StringNoSpill) {
    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("hello");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "world");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "hello");
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, PushPop_StringWithSpill) {
    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("hello");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "world");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "hello");
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, PushPop_StringMoveNoSpill) {
    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    std::string hello = "hello";
    vec.Push(std::move(hello));
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    std::string world = "world";
    vec.Push(std::move(world));
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "world");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "hello");
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, PushPop_StringMoveWithSpill) {
    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("hello");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push("world");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "world");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), "hello");
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, EmplacePop_TupleVarArgNoSpill) {
    Vector<std::tuple<int, float, bool>, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Emplace(1, 2.0, false);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Emplace(3, 4.0, true);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), std::make_tuple(3, 4.0, true));
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), std::make_tuple(1, 2.0, false));
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, EmplacePop_TupleVarArgWithSpill) {
    Vector<std::tuple<int, float, bool>, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Emplace(1, 2.0, false);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Emplace(3, 4.0, true);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), std::make_tuple(3, 4.0, true));
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), std::make_tuple(1, 2.0, false));
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, IsEmpty) {
    Vector<std::string, 1> vec;
    EXPECT_TRUE(vec.IsEmpty());
    vec.Push("one");
    EXPECT_FALSE(vec.IsEmpty());
    vec.Pop();
    EXPECT_TRUE(vec.IsEmpty());
}

TEST(TintVectorTest, FrontBack_NoSpill) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec.Front())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec.Back())>>);
    EXPECT_EQ(vec.Front(), "front");
    EXPECT_EQ(vec.Back(), "back");
}

TEST(TintVectorTest, FrontBack_WithSpill) {
    Vector<std::string, 2> vec{"front", "mid", "back"};
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec.Front())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec.Back())>>);
    EXPECT_EQ(vec.Front(), "front");
    EXPECT_EQ(vec.Back(), "back");
}

TEST(TintVectorTest, ConstFrontBack_NoSpill) {
    const Vector<std::string, 3> vec{"front", "mid", "back"};
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec.Back())>>);
    EXPECT_EQ(vec.Front(), "front");
    EXPECT_EQ(vec.Back(), "back");
}

TEST(TintVectorTest, ConstFrontBack_WithSpill) {
    const Vector<std::string, 2> vec{"front", "mid", "back"};
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec.Back())>>);
    EXPECT_EQ(vec.Front(), "front");
    EXPECT_EQ(vec.Back(), "back");
}

TEST(TintVectorTest, BeginEnd_NoSpill) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.end())>>);
    EXPECT_EQ(vec.begin(), &vec[0]);
    EXPECT_EQ(vec.end(), &vec[0] + 3);
}

TEST(TintVectorTest, BeginEnd_WithSpill) {
    Vector<std::string, 2> vec{"front", "mid", "back"};
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.end())>>);
    EXPECT_EQ(vec.begin(), &vec[0]);
    EXPECT_EQ(vec.end(), &vec[0] + 3);
}

TEST(TintVectorTest, ConstBeginEnd_NoSpill) {
    const Vector<std::string, 3> vec{"front", "mid", "back"};
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec.end())>>);
    EXPECT_EQ(vec.begin(), &vec[0]);
    EXPECT_EQ(vec.end(), &vec[0] + 3);
}

TEST(TintVectorTest, ConstBeginEnd_WithSpill) {
    const Vector<std::string, 2> vec{"front", "mid", "back"};
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec.end())>>);
    EXPECT_EQ(vec.begin(), &vec[0]);
    EXPECT_EQ(vec.end(), &vec[0] + 3);
}

TEST(TintVectorRefTest, CtorVectorNoMove) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref(vec_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorRefTest, CtorVectorMove) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref(std::move(vec_a));  // Move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Move, no copy
}

TEST(TintVectorRefTest, CopyCtor) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref_a(std::move(vec_a));
    VectorRef<std::string> vec_ref_b(vec_ref_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorRefTest, MoveCtor) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref_a(std::move(vec_a));  // Move
    VectorRef<std::string> vec_ref_b(std::move(vec_ref_a));
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Move, no copy
}

TEST(TintVectorRefTest, Index) {
    Vector<std::string, 2> vec{"one", "two"};
    VectorRef<std::string> vec_ref(vec);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec_ref[0])>>);
    EXPECT_EQ(vec_ref[0], "one");
    EXPECT_EQ(vec_ref[1], "two");
}

TEST(TintVectorRefTest, ConstIndex) {
    Vector<std::string, 2> vec{"one", "two"};
    const VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref[0])>>);
    EXPECT_EQ(vec_ref[0], "one");
    EXPECT_EQ(vec_ref[1], "two");
}

TEST(TintVectorRefTest, Length) {
    Vector<std::string, 2> vec{"one", "two", "three"};
    VectorRef<std::string> vec_ref(vec);
    EXPECT_EQ(vec_ref.Length(), 3u);
}

TEST(TintVectorRefTest, Capacity) {
    Vector<std::string, 5> vec{"one", "two", "three"};
    VectorRef<std::string> vec_ref(vec);
    EXPECT_EQ(vec_ref.Capacity(), 5u);
}

TEST(TintVectorRefTest, IsEmpty) {
    Vector<std::string, 1> vec;
    VectorRef<std::string> vec_ref(vec);
    EXPECT_TRUE(vec_ref.IsEmpty());
    vec.Push("one");
    EXPECT_FALSE(vec_ref.IsEmpty());
    vec.Pop();
    EXPECT_TRUE(vec_ref.IsEmpty());
}

TEST(TintVectorRefTest, FrontBack) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    VectorRef<std::string> vec_ref(vec);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec_ref.Front())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(vec_ref.Back())>>);
    EXPECT_EQ(vec_ref.Front(), "front");
    EXPECT_EQ(vec_ref.Back(), "back");
}

TEST(TintVectorRefTest, ConstFrontBack) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    const VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Back())>>);
    EXPECT_EQ(vec_ref.Front(), "front");
    EXPECT_EQ(vec_ref.Back(), "back");
}

TEST(TintVectorRefTest, BeginEnd) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    VectorRef<std::string> vec_ref(vec);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec_ref.begin())>>);
    static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec_ref.end())>>);
    EXPECT_EQ(vec_ref.begin(), &vec[0]);
    EXPECT_EQ(vec_ref.end(), &vec[0] + 3);
}

TEST(TintVectorRefTest, ConstBeginEnd) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    const VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.end())>>);
    EXPECT_EQ(vec_ref.begin(), &vec[0]);
    EXPECT_EQ(vec_ref.end(), &vec[0] + 3);
}

TEST(TintVectorConstRefTest, CtorVectorNoMove) {
    Vector<std::string, 1> vec_a{"one", "two"};
    ConstVectorRef<std::string> vec_ref(vec_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorConstRefTest, CtorVectorMove) {
    Vector<std::string, 1> vec_a{"one", "two"};
    ConstVectorRef<std::string> vec_ref(std::move(vec_a));  // Move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorConstRefTest, CopyCtor) {
    Vector<std::string, 1> vec_a{"one", "two"};
    ConstVectorRef<std::string> vec_ref_a(std::move(vec_a));
    ConstVectorRef<std::string> vec_ref_b(vec_ref_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorConstRefTest, MoveCtor) {
    Vector<std::string, 1> vec_a{"one", "two"};
    ConstVectorRef<std::string> vec_ref_a(std::move(vec_a));  // Move
    ConstVectorRef<std::string> vec_ref_b(std::move(vec_ref_a));
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copy, no move
}

TEST(TintVectorConstRefTest, Index) {
    Vector<std::string, 2> vec{"one", "two"};
    ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref[0])>>);
    EXPECT_EQ(vec_ref[0], "one");
    EXPECT_EQ(vec_ref[1], "two");
}

TEST(TintVectorConstRefTest, ConstIndex) {
    Vector<std::string, 2> vec{"one", "two"};
    const ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref[0])>>);
    EXPECT_EQ(vec_ref[0], "one");
    EXPECT_EQ(vec_ref[1], "two");
}

TEST(TintVectorConstRefTest, Length) {
    Vector<std::string, 2> vec{"one", "two", "three"};
    ConstVectorRef<std::string> vec_ref(vec);
    EXPECT_EQ(vec_ref.Length(), 3u);
}

TEST(TintVectorConstRefTest, Capacity) {
    Vector<std::string, 5> vec{"one", "two", "three"};
    ConstVectorRef<std::string> vec_ref(vec);
    EXPECT_EQ(vec_ref.Capacity(), 5u);
}

TEST(TintVectorConstRefTest, IsEmpty) {
    Vector<std::string, 1> vec;
    ConstVectorRef<std::string> vec_ref(vec);
    EXPECT_TRUE(vec_ref.IsEmpty());
    vec.Push("one");
    EXPECT_FALSE(vec_ref.IsEmpty());
    vec.Pop();
    EXPECT_TRUE(vec_ref.IsEmpty());
}

TEST(TintVectorConstRefTest, FrontBack) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Back())>>);
    EXPECT_EQ(vec_ref.Front(), "front");
    EXPECT_EQ(vec_ref.Back(), "back");
}

TEST(TintVectorConstRefTest, ConstFrontBack) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    const ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Back())>>);
    EXPECT_EQ(vec_ref.Front(), "front");
    EXPECT_EQ(vec_ref.Back(), "back");
}

TEST(TintVectorConstRefTest, BeginEnd) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.end())>>);
    EXPECT_EQ(vec_ref.begin(), &vec[0]);
    EXPECT_EQ(vec_ref.end(), &vec[0] + 3);
}

TEST(TintVectorConstRefTest, ConstBeginEnd) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    const ConstVectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.end())>>);
    EXPECT_EQ(vec_ref.begin(), &vec[0]);
    EXPECT_EQ(vec_ref.end(), &vec[0] + 3);
}

}  // namespace
}  // namespace tint::utils
