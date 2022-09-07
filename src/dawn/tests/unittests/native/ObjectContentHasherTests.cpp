// Copyright 2022 The Dawn Authors
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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/ObjectContentHasher.h"
#include "dawn/tests/DawnNativeTest.h"

namespace dawn::native {

namespace {

class ObjectContentHasherTests : public DawnNativeTest {};

#define EXPECT_IF_HASH_EQ(eq, a, b)                                \
    do {                                                           \
        ObjectContentHasher ra, rb;                                \
        ra.Record(a);                                              \
        rb.Record(b);                                              \
        EXPECT_EQ(eq, ra.GetContentHash() == rb.GetContentHash()); \
    } while (0)

TEST(ObjectContentHasherTests, Pair) {
    EXPECT_IF_HASH_EQ(true, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"a", 1}));
    EXPECT_IF_HASH_EQ(false, (std::pair<uint8_t, std::string>{1, "a"}),
                      (std::pair<std::string, uint8_t>{"a", 1}));
    EXPECT_IF_HASH_EQ(false, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"a", 2}));
    EXPECT_IF_HASH_EQ(false, (std::pair<std::string, uint8_t>{"a", 1}),
                      (std::pair<std::string, uint8_t>{"b", 1}));
}

TEST(ObjectContentHasherTests, Vector) {
    EXPECT_IF_HASH_EQ(true, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{0, 1}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{0, 1, 2}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{1, 0}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<uint8_t>{}));
    EXPECT_IF_HASH_EQ(false, (std::vector<uint8_t>{0, 1}), (std::vector<float>{0, 1}));
}

TEST(ObjectContentHasherTests, Map) {
    EXPECT_IF_HASH_EQ(true, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"b", 2}, {"a", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"a", 2}, {"b", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}, {"c", 1}}));
    EXPECT_IF_HASH_EQ(false, (std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}}),
                      (std::map<std::string, uint8_t>{}));
}

TEST(ObjectContentHasherTests, HashCombine) {
    ObjectContentHasher ra, rb;

    ra.Record(std::vector<uint8_t>{0, 1});
    ra.Record(std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}});

    rb.Record(std::map<std::string, uint8_t>{{"a", 1}, {"b", 2}});
    rb.Record(std::vector<uint8_t>{0, 1});

    EXPECT_NE(ra.GetContentHash(), rb.GetContentHash());
}

}  // namespace

}  // namespace dawn::native
