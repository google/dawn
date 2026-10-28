// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/math/hash.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/utils/containers/vector.h"

namespace tint {
namespace {

TEST(HashTests, Basic) {
    EXPECT_EQ(Hash(123), Hash(123));
    EXPECT_EQ(Hash(123, 456), Hash(123, 456));
    EXPECT_EQ(Hash(123, 456, false), Hash(123, 456, false));
    EXPECT_EQ(Hash(std::string("hello")), Hash(std::string("hello")));
}

TEST(HashTests, PointerBasic) {
    const int* null_ptr = nullptr;
    EXPECT_EQ(Hash(null_ptr), Hash(null_ptr));

    int a = 1;
    int b = 2;
    EXPECT_EQ(Hash(&a), Hash(&a));
    EXPECT_NE(Hash(&a), Hash(&b));

    if constexpr (sizeof(uintptr_t) > 4) {
        // Pointers differing only in upper 32 bits must not collide.
        auto* hi_a = reinterpret_cast<const void*>(static_cast<uintptr_t>(0x00007fff00001000ULL));
        auto* hi_b = reinterpret_cast<const void*>(static_cast<uintptr_t>(0x00007ffe00001000ULL));
        EXPECT_NE(Hash(hi_a), Hash(hi_b));

        // Pointers with saturated high bits (0x7fff in bits 32..47) differing only in bits 4..19
        // must not have their low-bit entropy wiped out.
        auto* low_a = reinterpret_cast<const void*>(static_cast<uintptr_t>(0x00007fff00010010ULL));
        auto* low_b = reinterpret_cast<const void*>(static_cast<uintptr_t>(0x00007fff00020020ULL));
        EXPECT_NE(Hash(low_a), Hash(low_b));
    }
}

TEST(HashTests, PointerDistributionModuloHashmapSlots) {
    constexpr size_t kNumPointers = 4096;
    // HashmapBase::NumSlots(8192) == (8192 * 75) / 100 == 6144 (3 * 2^11).
    constexpr size_t kNumSlots = 6144;
    constexpr uintptr_t kBaseAddress = (sizeof(uintptr_t) > 4)
                                           ? static_cast<uintptr_t>(0x00007fff00000000ULL)
                                           : static_cast<uintptr_t>(0x7fff0000U);

    for (size_t stride : {size_t{48}, size_t{64}}) {
        std::vector<size_t> bucket_counts(kNumSlots, 0);
        for (size_t i = 0; i < kNumPointers; i++) {
            auto* ptr = reinterpret_cast<const void*>(kBaseAddress + i * stride);
            HashCode h = Hash(ptr);
            bucket_counts[h % kNumSlots]++;
        }

        size_t occupied_buckets = 0;
        size_t max_chain_length = 0;
        for (size_t count : bucket_counts) {
            if (count > 0) {
                occupied_buckets++;
                max_chain_length = std::max(max_chain_length, count);
            }
        }

        EXPECT_GT(occupied_buckets, 2000u) << "stride: " << stride;
        EXPECT_LE(max_chain_length, 8u) << "stride: " << stride;
    }
}

TEST(HashTests, StdVector) {
    EXPECT_EQ(Hash(std::vector<int>({})), Hash(std::vector<int>({})));
    EXPECT_EQ(Hash(std::vector<int>({1, 2, 3})), Hash(std::vector<int>({1, 2, 3})));
}

TEST(HashTests, TintVector) {
    EXPECT_EQ(Hash(Vector<int, 0>({})), Hash(Vector<int, 0>({})));
    EXPECT_EQ(Hash(Vector<int, 0>({1, 2, 3})), Hash(Vector<int, 0>({1, 2, 3})));
    EXPECT_EQ(Hash(Vector<int, 3>({1, 2, 3})), Hash(Vector<int, 4>({1, 2, 3})));
    EXPECT_EQ(Hash(Vector<int, 3>({1, 2, 3})), Hash(Vector<int, 2>({1, 2, 3})));
}

TEST(HashTests, TintVectorRef) {
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 0>({}))), Hash(VectorRef<int>(Vector<int, 0>({}))));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 0>({1, 2, 3}))),
              Hash(VectorRef<int>(Vector<int, 0>({1, 2, 3}))));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 3>({1, 2, 3}))),
              Hash(VectorRef<int>(Vector<int, 4>({1, 2, 3}))));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 3>({1, 2, 3}))),
              Hash(VectorRef<int>(Vector<int, 2>({1, 2, 3}))));

    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 0>({}))), Hash(Vector<int, 0>({})));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 0>({1, 2, 3}))), Hash(Vector<int, 0>({1, 2, 3})));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 3>({1, 2, 3}))), Hash(Vector<int, 4>({1, 2, 3})));
    EXPECT_EQ(Hash(VectorRef<int>(Vector<int, 3>({1, 2, 3}))), Hash(Vector<int, 2>({1, 2, 3})));
}

TEST(HashTests, Tuple) {
    EXPECT_EQ(Hash(std::make_tuple(1)), Hash(std::make_tuple(1)));
    EXPECT_EQ(Hash(std::make_tuple(1, 2, 3)), Hash(std::make_tuple(1, 2, 3)));
}

TEST(HashTests, UnorderedKeyWrapper) {
    using W = UnorderedKeyWrapper<std::vector<int>>;

    std::unordered_map<W, int> m;

    m.emplace(W{{1, 2}}, -1);
    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m[W({1, 2})], -1);

    m.emplace(W{{3, 2}}, 1);
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m[W({3, 2})], 1);
    EXPECT_EQ(m[W({1, 2})], -1);

    m.emplace(W{{100}}, 100);
    EXPECT_EQ(m.size(), 3u);
    EXPECT_EQ(m[W({100})], 100);
    EXPECT_EQ(m[W({3, 2})], 1);
    EXPECT_EQ(m[W({1, 2})], -1);

    // Reversed vector element order
    EXPECT_EQ(m[W({2, 3})], 0);
    EXPECT_EQ(m[W({2, 1})], 0);
}

TEST(EqualTo, String) {
    std::string str_a = "hello";
    std::string str_b = "world";
    const char* cstr_a = "hello";
    const char* cstr_b = "world";
    std::string_view sv_a = "hello";
    std::string_view sv_b = "world";
    EXPECT_TRUE(EqualTo<std::string>()(str_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, cstr_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, sv_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(cstr_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(sv_a, str_a));

    EXPECT_FALSE(EqualTo<std::string>()(str_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, cstr_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, sv_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(cstr_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(sv_a, str_b));

    EXPECT_FALSE(EqualTo<std::string>()(str_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, cstr_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, sv_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(cstr_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(sv_b, str_a));

    EXPECT_TRUE(EqualTo<std::string>()(str_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, cstr_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, sv_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(cstr_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(sv_b, str_b));
}

}  // namespace
}  // namespace tint
