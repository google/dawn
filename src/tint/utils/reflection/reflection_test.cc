// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/utils/reflection/reflection.h"
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
