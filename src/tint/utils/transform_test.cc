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

#include "src/tint/utils/transform.h"

#include <string>
#include <type_traits>

#include "gmock/gmock.h"

#define CHECK_ELEMENT_TYPE(vector, expected)                                   \
    static_assert(std::is_same<decltype(vector)::value_type, expected>::value, \
                  "unexpected result vector element type")

namespace tint::utils {
namespace {

TEST(TransformTest, Empty) {
    const std::vector<int> empty{};
    {
        auto transformed = Transform(empty, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
    {
        auto transformed = Transform(empty, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
}

TEST(TransformTest, Identity) {
    const std::vector<int> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, IdentityWithIndex) {
    const std::vector<int> input{1, 2, 3, 4};
    auto transformed = Transform(input, [](int i, size_t) { return i; });
    CHECK_ELEMENT_TYPE(transformed, int);
    EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
}

TEST(TransformTest, Index) {
    const std::vector<int> input{10, 20, 30, 40};
    {
        auto transformed = Transform(input, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformTest, TransformSameType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformTest, TransformDifferentType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = Transform(input, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

TEST(TransformNTest, Empty) {
    const std::vector<int> empty{};
    {
        auto transformed = TransformN(empty, 4u, [](int) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
    {
        auto transformed = TransformN(empty, 4u, [](int, size_t) -> int {
            [] { FAIL() << "Callback should not be called for empty vector"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_EQ(transformed.size(), 0u);
    }
}

TEST(TransformNTest, Identity) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2));
    }
    {
        auto transformed = TransformN(input, 6u, [](int i) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
    }
}

TEST(TransformNTest, IdentityWithIndex) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 3u, [](int i, size_t) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i, size_t) { return i; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(1, 2, 3, 4));
    }
}

TEST(TransformNTest, Index) {
    const std::vector<int> input{10, 20, 30, 40};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return static_cast<size_t>(0);
        });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u));
    }
    {
        auto transformed = TransformN(input, 9u, [](int, size_t idx) { return idx; });
        CHECK_ELEMENT_TYPE(transformed, size_t);
        EXPECT_THAT(transformed, testing::ElementsAre(0u, 1u, 2u, 3u));
    }
}

TEST(TransformNTest, TransformSameType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int, size_t) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return 0;
        });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i) { return i * 10; });
        CHECK_ELEMENT_TYPE(transformed, int);
        EXPECT_THAT(transformed, testing::ElementsAre(10, 20, 30, 40));
    }
}

TEST(TransformNTest, TransformDifferentType) {
    const std::vector<int> input{1, 2, 3, 4};
    {
        auto transformed = TransformN(input, 0u, [](int) {
            [] { FAIL() << "Callback should not call the transform when n == 0"; }();
            return std::string();
        });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_TRUE(transformed.empty());
    }
    {
        auto transformed = TransformN(input, 2u, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2"));
    }
    {
        auto transformed = TransformN(input, 9u, [](int i) { return std::to_string(i); });
        CHECK_ELEMENT_TYPE(transformed, std::string);
        EXPECT_THAT(transformed, testing::ElementsAre("1", "2", "3", "4"));
    }
}

}  // namespace
}  // namespace tint::utils
