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

#include "src/tint/reflection.h"
#include "gtest/gtest.h"

namespace tint {
namespace {

struct S {
    int i;
    unsigned u;
    bool b;
    TINT_REFLECT(i, u, b);
};

static_assert(!HasReflection<int>);
static_assert(HasReflection<S>);

TEST(ReflectionTest, ForeachFieldConst) {
    const S s{1, 2, true};
    size_t field_idx = 0;
    ForeachField(s, [&](auto& field) {
        using T = std::decay_t<decltype(field)>;
        switch (field_idx) {
            case 0:
                EXPECT_TRUE((std::is_same_v<T, int>));
                EXPECT_EQ(field, static_cast<T>(1));
                break;
            case 1:
                EXPECT_TRUE((std::is_same_v<T, unsigned>));
                EXPECT_EQ(field, static_cast<T>(2));
                break;
            case 2:
                EXPECT_TRUE((std::is_same_v<T, bool>));
                EXPECT_EQ(field, static_cast<T>(true));
                break;
            default:
                FAIL() << "unexpected field";
                break;
        }
        field_idx++;
    });
}

TEST(ReflectionTest, ForeachFieldNonConst) {
    S s{1, 2, true};
    size_t field_idx = 0;
    ForeachField(s, [&](auto& field) {
        using T = std::decay_t<decltype(field)>;
        switch (field_idx) {
            case 0:
                EXPECT_TRUE((std::is_same_v<T, int>));
                EXPECT_EQ(field, static_cast<T>(1));
                field = static_cast<T>(10);
                break;
            case 1:
                EXPECT_TRUE((std::is_same_v<T, unsigned>));
                EXPECT_EQ(field, static_cast<T>(2));
                field = static_cast<T>(20);
                break;
            case 2:
                EXPECT_TRUE((std::is_same_v<T, bool>));
                EXPECT_EQ(field, static_cast<T>(true));
                field = static_cast<T>(false);
                break;
            default:
                FAIL() << "unexpected field";
                break;
        }
        field_idx++;
    });

    EXPECT_EQ(s.i, 10);
    EXPECT_EQ(s.u, 20u);
    EXPECT_EQ(s.b, false);
}

}  // namespace
}  // namespace tint
