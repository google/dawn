// Copyright 2020 The Tint Authors.
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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, VecPrefix_Vec2) {
    auto p = parser("vec2");
    auto t = p->vec_prefix();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_EQ(2u, t.value);
}

TEST_F(ParserImplTest, VecPrefix_Vec3) {
    auto p = parser("vec3");
    auto t = p->vec_prefix();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_EQ(3u, t.value);
}

TEST_F(ParserImplTest, VecPrefix_Vec4) {
    auto p = parser("vec4");
    auto t = p->vec_prefix();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_EQ(4u, t.value);
}

TEST_F(ParserImplTest, VecPrefix_NoMatch) {
    auto p = parser("mat2x2");
    auto t = p->vec_prefix();
    EXPECT_FALSE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_EQ(0u, t.value);
}

struct MatData {
    std::string name;
    uint32_t columns;
    uint32_t rows;
};
class MatPrefixTest : public ParserImplTestWithParam<MatData> {};
TEST_P(MatPrefixTest, Parse) {
    auto params = GetParam();

    auto p = parser(params.name);
    auto t = p->mat_prefix();
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    auto dims = t.value;
    EXPECT_EQ(params.columns, dims.columns);
    EXPECT_EQ(params.rows, dims.rows);
}
INSTANTIATE_TEST_SUITE_P(ParserImplTest,
                         MatPrefixTest,
                         testing::Values(MatData{"mat2x2", 2, 2},
                                         MatData{"mat2x3", 2, 3},
                                         MatData{"mat2x4", 2, 4},
                                         MatData{"mat3x2", 3, 2},
                                         MatData{"mat3x3", 3, 3},
                                         MatData{"mat3x4", 3, 4},
                                         MatData{"mat4x2", 4, 2},
                                         MatData{"mat4x3", 4, 3},
                                         MatData{"mat4x4", 4, 4}));

TEST_F(ParserImplTest, MatPrefix_NoMatch) {
    auto p = parser("vec2");
    auto t = p->mat_prefix();
    EXPECT_FALSE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();

    EXPECT_EQ(0u, t.value.columns);
    EXPECT_EQ(0u, t.value.rows);
}

}  // namespace
}  // namespace tint::reader::wgsl
