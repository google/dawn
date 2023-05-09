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

#include "gmock/gmock.h"

#include "src/tint/utils/bitcast.h"
#include "src/tint/utils/predicates.h"
#include "src/tint/utils/string_stream.h"

namespace tint::utils {
namespace {

class C0 : public Castable<C0> {};
class C1 : public Castable<C1, C0> {};
class C2a : public Castable<C2a, C1> {};
class C2b : public Castable<C2b, C1> {};

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

////////////////////////////////////////////////////////////////////////////////
// Static asserts
////////////////////////////////////////////////////////////////////////////////
static_assert(std::is_same_v<VectorCommonType<int>, int>);
static_assert(std::is_same_v<VectorCommonType<int, int>, int>);
static_assert(std::is_same_v<VectorCommonType<int, float>, float>);

static_assert(std::is_same_v<VectorCommonType<C0*>, C0*>);
static_assert(std::is_same_v<VectorCommonType<const C0*>, const C0*>);

static_assert(std::is_same_v<VectorCommonType<C0*, C1*>, C0*>);
static_assert(std::is_same_v<VectorCommonType<const C0*, C1*>, const C0*>);
static_assert(std::is_same_v<VectorCommonType<C0*, const C1*>, const C0*>);
static_assert(std::is_same_v<VectorCommonType<const C0*, const C1*>, const C0*>);

static_assert(std::is_same_v<VectorCommonType<C2a*, C2b*>, C1*>);
static_assert(std::is_same_v<VectorCommonType<const C2a*, C2b*>, const C1*>);
static_assert(std::is_same_v<VectorCommonType<C2a*, const C2b*>, const C1*>);
static_assert(std::is_same_v<VectorCommonType<const C2a*, const C2b*>, const C1*>);

////////////////////////////////////////////////////////////////////////////////
// TintVectorTest
////////////////////////////////////////////////////////////////////////////////
TEST(TintVectorTest, SmallArray_Empty) {
    Vector<int, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
}

TEST(TintVectorTest, NoSmallArray) {
    Vector<int, 0> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
}

TEST(TintVectorTest, Empty_SmallArray_Empty) {
    Vector<int, 2> vec(Empty);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
}

TEST(TintVectorTest, Empty_NoSmallArray) {
    Vector<int, 0> vec(Empty);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
}

TEST(TintVectorTest, InitializerList_NoSpill) {
    Vector<std::string, 2> vec{"one", "two"};
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InitializerList_WithSpill) {
    Vector<std::string, 2> vec{"one", "two", "three"};
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_EQ(vec[2], "three");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, InitializerList_NoSmallArray) {
    Vector<std::string, 0> vec{"one", "two"};
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Push_NoSmallArray) {
    Vector<std::string, 0> vec;
    vec.Push("one");
    vec.Push("two");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "one");
    EXPECT_EQ(vec[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_1CString) {
    auto vec = Vector{"one"};
    static_assert(std::is_same_v<decltype(vec)::value_type, const char*>);
    static_assert(decltype(vec)::static_length == 1u);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_STREQ(vec[0], "one");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_2CStrings) {
    auto vec = Vector{"one", "two"};
    static_assert(std::is_same_v<decltype(vec)::value_type, const char*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_STREQ(vec[0], "one");
    EXPECT_STREQ(vec[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_IntFloat) {
    auto vec = Vector{1, 2.0f};
    static_assert(std::is_same_v<decltype(vec)::value_type, float>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], 1.0f);
    EXPECT_EQ(vec[1], 2.0f);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_IntDoubleIntDouble) {
    auto vec = Vector{1, 2.0, 3, 4.0};
    static_assert(std::is_same_v<decltype(vec)::value_type, double>);
    static_assert(decltype(vec)::static_length == 4u);
    EXPECT_EQ(vec.Length(), 4u);
    EXPECT_EQ(vec.Capacity(), 4u);
    EXPECT_EQ(vec[0], 1.0);
    EXPECT_EQ(vec[1], 2.0);
    EXPECT_EQ(vec[2], 3.0);
    EXPECT_EQ(vec[3], 4.0);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_C0) {
    C0 c0;
    auto vec = Vector{&c0};
    static_assert(std::is_same_v<decltype(vec)::value_type, C0*>);
    static_assert(decltype(vec)::static_length == 1u);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_ConstC0) {
    const C0 c0;
    auto vec = Vector{&c0};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C0*>);
    static_assert(decltype(vec)::static_length == 1u);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_C0C1) {
    C0 c0;
    C1 c1;
    auto vec = Vector{&c0, &c1};
    static_assert(std::is_same_v<decltype(vec)::value_type, C0*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_EQ(vec[1], &c1);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_ConstC0C1) {
    const C0 c0;
    C1 c1;
    auto vec = Vector{&c0, &c1};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C0*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_EQ(vec[1], &c1);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_C0ConstC1) {
    C0 c0;
    const C1 c1;
    auto vec = Vector{&c0, &c1};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C0*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_EQ(vec[1], &c1);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_ConstC0ConstC1) {
    const C0 c0;
    const C1 c1;
    auto vec = Vector{&c0, &c1};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C0*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c0);
    EXPECT_EQ(vec[1], &c1);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_C2aC2b) {
    C2a c2a;
    C2b c2b;
    auto vec = Vector{&c2a, &c2b};
    static_assert(std::is_same_v<decltype(vec)::value_type, C1*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c2a);
    EXPECT_EQ(vec[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_ConstC2aC2b) {
    const C2a c2a;
    C2b c2b;
    auto vec = Vector{&c2a, &c2b};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C1*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c2a);
    EXPECT_EQ(vec[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_C2aConstC2b) {
    C2a c2a;
    const C2b c2b;
    auto vec = Vector{&c2a, &c2b};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C1*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c2a);
    EXPECT_EQ(vec[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, InferTN_ConstC2aConstC2b) {
    const C2a c2a;
    const C2b c2b;
    auto vec = Vector{&c2a, &c2b};
    static_assert(std::is_same_v<decltype(vec)::value_type, const C1*>);
    static_assert(decltype(vec)::static_length == 2u);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], &c2a);
    EXPECT_EQ(vec[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, CopyVector_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyVector_NoMoveUpcast_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<C0*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, CopyVector_NoMoveUpcast_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<C0*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, CopyVector_NoMoveAddConst_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<const C1*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, CopyVector_NoMoveAddConst_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<const C1*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, CopyVector_NoMoveUpcastAndAddConst_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<const C0*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, CopyVector_NoMoveUpcastAndAddConst_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<const C0*, 2> vec_b(vec_a);  // No move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, MoveVector_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 2> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 2> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 1> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 3> vec_b(std::move(vec_a));
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveVector_Upcast_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<C0*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, MoveVector_Upcast_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<C0*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorTest, MoveVector_AddConst_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<const C1*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, MoveVector_AddConst_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<const C1*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorTest, MoveVector_UpcastAndAddConst_NoSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 2> vec_a{&c2a, &c2b};
    Vector<const C0*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorTest, MoveVector_UpcastAndAddConst_WithSpill) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    Vector<const C0*, 2> vec_b(std::move(vec_a));  // Move
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
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

TEST(TintVectorTest, CopyAssign_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 0> vec_b;
    vec_b = vec_a;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssign_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 0> vec_b;
    vec_b = vec_a;
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

TEST(TintVectorTest, MoveAssign_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    Vector<std::string, 0> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssign_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    Vector<std::string, 0> vec_b;
    vec_b = std::move(vec_a);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
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

TEST(TintVectorTest, RepeatMoveAssign_WithSpill) {
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

TEST(TintVectorTest, CopyAssignRef_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 2> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 2> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 1> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_WithSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 1> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 3> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 3> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 0> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 0> vec_b;
    vec_b = ref;
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, CopyAssignRef_Self_NoSpill) {
    Vector<std::string, 2> vec{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec)};
    vec = ref;
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, CopyAssignRef_Self_WithSpill) {
    Vector<std::string, 1> vec{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec)};
    vec = ref;
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, MoveAssignRef_NoSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 2> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_WithSpill_N2_to_N2) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 2> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_NoSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 1> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_SpillSpill_N2_to_N1) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 1> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_NoSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 3> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_WithSpill_N2_to_N3) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 3> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_NoSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 0> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 2u);
    EXPECT_EQ(vec_b.Capacity(), 2u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_WithSpill_N2_to_N0) {
    Vector<std::string, 2> vec_a{"hello", "world", "spill"};
    VectorRef<std::string> ref{std::move(vec_a)};
    Vector<std::string, 0> vec_b;
    vec_b = std::move(ref);
    EXPECT_EQ(vec_b.Length(), 3u);
    EXPECT_EQ(vec_b.Capacity(), 3u);
    EXPECT_EQ(vec_b[0], "hello");
    EXPECT_EQ(vec_b[1], "world");
    EXPECT_EQ(vec_b[2], "spill");
    EXPECT_TRUE(AllExternallyHeld(vec_b));
}

TEST(TintVectorTest, MoveAssignRef_Self_NoSpill) {
    Vector<std::string, 2> vec{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec)};
    vec = std::move(ref);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, MoveAssignRef_Self_WithSpill) {
    Vector<std::string, 1> vec{"hello", "world"};
    VectorRef<std::string> ref{std::move(vec)};
    vec = std::move(ref);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, RepeatMoveAssignRef_NoSpill) {
    Vector<std::string, 3> vec_a{"hello", "world"};
    Vector<std::string, 3> vec_b{"Ciao", "mondo"};
    Vector<std::string, 3> vec_c{"Bonjour", "le", "monde"};
    VectorRef<std::string> ref_a{std::move(vec_a)};
    VectorRef<std::string> ref_b{std::move(vec_b)};
    VectorRef<std::string> ref_c{std::move(vec_c)};
    Vector<std::string, 3> vec;
    vec = std::move(ref_a);
    vec = std::move(ref_b);
    vec = std::move(ref_c);
    EXPECT_EQ(vec.Length(), 3u);
    EXPECT_EQ(vec.Capacity(), 3u);
    EXPECT_EQ(vec[0], "Bonjour");
    EXPECT_EQ(vec[1], "le");
    EXPECT_EQ(vec[2], "monde");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, RepeatMoveAssignRef_WithSpill) {
    Vector<std::string, 1> vec_a{"hello", "world"};
    Vector<std::string, 1> vec_b{"Ciao", "mondo"};
    Vector<std::string, 1> vec_c{"bonjour", "le", "monde"};
    VectorRef<std::string> ref_a{std::move(vec_a)};
    VectorRef<std::string> ref_b{std::move(vec_b)};
    VectorRef<std::string> ref_c{std::move(vec_c)};
    Vector<std::string, 1> vec;
    vec = std::move(ref_a);
    vec = std::move(ref_b);
    vec = std::move(ref_c);
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

TEST(TintVectorTest, Reserve_NoSpill) {
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

TEST(TintVectorTest, Reserve_WithSpill) {
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

TEST(TintVectorTest, ResizeZero_NoSpill) {
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

TEST(TintVectorTest, ResizeZero_WithSpill) {
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

TEST(TintVectorTest, ResizeValue_NoSpill) {
    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 2u);
    vec.Resize(1, "meow");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "meow");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[0] = "hello";
    vec.Resize(2, "woof");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "woof");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[1] = "world";
    vec.Resize(1, "quack");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec.Resize(2, "hiss");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "hiss");
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, ResizeValue_WithSpill) {
    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_EQ(vec.Capacity(), 1u);
    vec.Resize(1, "meow");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 1u);
    EXPECT_EQ(vec[0], "meow");
    EXPECT_TRUE(AllInternallyHeld(vec));
    vec[0] = "hello";
    vec.Resize(2, "woof");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "woof");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec[1] = "world";
    vec.Resize(1, "quack");
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(AllExternallyHeld(vec));
    vec.Resize(2, "hiss");
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_EQ(vec.Capacity(), 2u);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "hiss");
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, Reserve_NoSmallArray) {
    Vector<std::string, 0> vec;
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
    Vector<std::string, 0> vec;
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
    const std::string hello = "hello";
    const std::string world = "world";

    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(hello);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(world);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), world);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), hello);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));
}

TEST(TintVectorTest, PushPop_StringWithSpill) {
    const std::string hello = "hello";
    const std::string world = "world";

    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(hello);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(world);
    EXPECT_EQ(vec.Length(), 2u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), world);
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllExternallyHeld(vec));

    EXPECT_EQ(vec.Pop(), hello);
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllExternallyHeld(vec));
}

TEST(TintVectorTest, PushPop_StringMoveNoSpill) {
    std::string hello = "hello";
    std::string world = "world";

    Vector<std::string, 2> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(std::move(hello));
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

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
    std::string hello = "hello";
    std::string world = "world";

    Vector<std::string, 1> vec;
    EXPECT_EQ(vec.Length(), 0u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(std::move(hello));
    EXPECT_EQ(vec.Length(), 1u);
    EXPECT_TRUE(AllInternallyHeld(vec));

    vec.Push(std::move(world));
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

TEST(TintVectorTest, Equality) {
    EXPECT_EQ((Vector<int, 2>{1, 2}), (Vector<int, 2>{1, 2}));
    EXPECT_EQ((Vector<int, 1>{1, 2}), (Vector<int, 3>{1, 2}));
    EXPECT_NE((Vector{1, 2}), (Vector{1}));
    EXPECT_NE((Vector{1}), (Vector{1, 2}));
    EXPECT_NE((Vector{1, 2}), (Vector{2, 1}));
    EXPECT_NE((Vector{2, 1}), (Vector{1, 2}));
}

TEST(TintVectorTest, Sort) {
    Vector vec{1, 5, 3, 4, 2};
    vec.Sort();
    EXPECT_THAT(vec, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(TintVectorTest, Any) {
    Vector vec{1, 7, 5, 9};
    EXPECT_TRUE(vec.Any(Eq(1)));
    EXPECT_FALSE(vec.Any(Eq(2)));
    EXPECT_FALSE(vec.Any(Eq(3)));
    EXPECT_FALSE(vec.Any(Eq(4)));
    EXPECT_TRUE(vec.Any(Eq(5)));
    EXPECT_FALSE(vec.Any(Eq(6)));
    EXPECT_TRUE(vec.Any(Eq(7)));
    EXPECT_FALSE(vec.Any(Eq(8)));
    EXPECT_TRUE(vec.Any(Eq(9)));
}

TEST(TintVectorTest, All) {
    Vector vec{1, 7, 5, 9};
    EXPECT_FALSE(vec.All(Ne(1)));
    EXPECT_TRUE(vec.All(Ne(2)));
    EXPECT_TRUE(vec.All(Ne(3)));
    EXPECT_TRUE(vec.All(Ne(4)));
    EXPECT_FALSE(vec.All(Ne(5)));
    EXPECT_TRUE(vec.All(Ne(6)));
    EXPECT_FALSE(vec.All(Ne(7)));
    EXPECT_TRUE(vec.All(Ne(8)));
    EXPECT_FALSE(vec.All(Ne(9)));
}

TEST(TintVectorTest, ostream) {
    utils::StringStream ss;
    ss << Vector{1, 2, 3};
    EXPECT_EQ(ss.str(), "[1, 2, 3]");
}

////////////////////////////////////////////////////////////////////////////////
// TintVectorRefTest
////////////////////////////////////////////////////////////////////////////////
TEST(TintVectorRefTest, CopyVectorRef) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref_a(std::move(vec_a));
    VectorRef<std::string> vec_ref_b(vec_ref_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVectorRef_Upcast) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<C0*> vec_ref_b(vec_ref_a);  // No-move. Up-cast
    Vector<C0*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVectorRef_AddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<const C1*> vec_ref_b(vec_ref_a);  // No-move. Up-cast
    Vector<const C1*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVectorRef_UpcastAndAddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<const C0*> vec_ref_b(vec_ref_a);  // No-move. Up-cast
    Vector<const C0*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, MoveVectorRef) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref_a(std::move(vec_a));  // Move
    VectorRef<std::string> vec_ref_b(std::move(vec_ref_a));
    Vector<std::string, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVectorRef_Upcast) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<C0*> vec_ref_b(std::move(vec_ref_a));  // Moved. Up-cast
    Vector<C0*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVectorRef_AddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<const C1*> vec_ref_b(std::move(vec_ref_a));  // Moved. Up-cast
    Vector<const C1*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVectorRef_UpcastAndAddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C1*> vec_ref_a(std::move(vec_a));
    VectorRef<const C0*> vec_ref_b(std::move(vec_ref_a));  // Moved. Up-cast
    Vector<const C0*, 2> vec_b(std::move(vec_ref_b));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, CopyVector) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref(vec_a);  // No move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVector_Upcast) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C0*> vec_ref(vec_a);  // No move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<C0*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVector_AddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<const C1*> vec_ref(vec_a);  // No move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<const C1*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, CopyVector_UpcastAndAddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<const C0*> vec_ref(vec_a);  // No move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<const C0*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllInternallyHeld(vec_b));  // Copied, not moved
}

TEST(TintVectorRefTest, MoveVector) {
    Vector<std::string, 1> vec_a{"one", "two"};
    VectorRef<std::string> vec_ref(std::move(vec_a));  // Move
    Vector<std::string, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], "one");
    EXPECT_EQ(vec_b[1], "two");
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVector_Upcast) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<C0*> vec_ref(std::move(vec_a));  // Move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<C0*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVector_AddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<const C1*> vec_ref(std::move(vec_a));  // Move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<const C1*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVector_UpcastAndAddConst) {
    C2a c2a;
    C2b c2b;
    Vector<C1*, 1> vec_a{&c2a, &c2b};
    VectorRef<const C0*> vec_ref(std::move(vec_a));  // Move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    Vector<const C0*, 2> vec_b(std::move(vec_ref));
    EXPECT_EQ(vec_b[0], &c2a);
    EXPECT_EQ(vec_b[1], &c2b);
    EXPECT_TRUE(AllExternallyHeld(vec_b));  // Moved, not copied
}

TEST(TintVectorRefTest, MoveVector_ReinterpretCast) {
    C2a c2a;
    C2b c2b;
    Vector<C0*, 1> vec_a{&c2a, &c2b};
    VectorRef<const C0*> vec_ref(std::move(vec_a));  // Move
    EXPECT_EQ(vec_ref[0], &c2a);
    EXPECT_EQ(vec_ref[1], &c2b);
    VectorRef<const C1*> reinterpret = vec_ref.ReinterpretCast<const C1*>();
    EXPECT_EQ(reinterpret[0], &c2a);
    EXPECT_EQ(reinterpret[1], &c2b);
}

TEST(TintVectorRefTest, Index) {
    Vector<std::string, 2> vec{"one", "two"};
    VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref[0])>>);
    EXPECT_EQ(vec_ref[0], "one");
    EXPECT_EQ(vec_ref[1], "two");
}

TEST(TintVectorRefTest, SortPredicate) {
    Vector vec{1, 5, 3, 4, 2};
    vec.Sort([](int a, int b) { return b < a; });
    EXPECT_THAT(vec, testing::ElementsAre(5, 4, 3, 2, 1));
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
    const VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Front())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(vec_ref.Back())>>);
    EXPECT_EQ(vec_ref.Front(), "front");
    EXPECT_EQ(vec_ref.Back(), "back");
}

TEST(TintVectorRefTest, BeginEnd) {
    Vector<std::string, 3> vec{"front", "mid", "back"};
    const VectorRef<std::string> vec_ref(vec);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.begin())>>);
    static_assert(std::is_const_v<std::remove_reference_t<decltype(*vec_ref.end())>>);
    EXPECT_EQ(vec_ref.begin(), &vec[0]);
    EXPECT_EQ(vec_ref.end(), &vec[0] + 3);
}

TEST(TintVectorRefTest, ostream) {
    utils::StringStream ss;
    Vector vec{1, 2, 3};
    const VectorRef<int> vec_ref(vec);
    ss << vec_ref;
    EXPECT_EQ(ss.str(), "[1, 2, 3]");
}

}  // namespace
}  // namespace tint::utils

TINT_INSTANTIATE_TYPEINFO(tint::utils::C0);
TINT_INSTANTIATE_TYPEINFO(tint::utils::C1);
TINT_INSTANTIATE_TYPEINFO(tint::utils::C2a);
TINT_INSTANTIATE_TYPEINFO(tint::utils::C2b);
