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

TEST_F(ParserImplTest, TexelFormat_Invalid) {
    auto p = parser("1234");
    auto t = p->expect_texel_format("test");
    EXPECT_TRUE(t.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), R"(1:1: expected texel format for test
Possible values: 'r32float', 'r32sint', 'r32uint', 'rg32float', 'rg32sint', 'rg32uint', 'rgba16float', 'rgba16sint', 'rgba16uint', 'rgba32float', 'rgba32sint', 'rgba32uint', 'rgba8sint', 'rgba8snorm', 'rgba8uint', 'rgba8unorm')");
}

TEST_F(ParserImplTest, TexelFormat_R32Uint) {
    auto p = parser("r32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_R32Sint) {
    auto p = parser("r32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_R32Float) {
    auto p = parser("r32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba8Unorm) {
    auto p = parser("rgba8unorm");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Unorm);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba8Snorm) {
    auto p = parser("rgba8snorm");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Snorm);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba8Uint) {
    auto p = parser("rgba8uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba8Sint) {
    auto p = parser("rgba8sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rg32Uint) {
    auto p = parser("rg32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rg32Sint) {
    auto p = parser("rg32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rg32Float) {
    auto p = parser("rg32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba16Uint) {
    auto p = parser("rgba16uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba16Sint) {
    auto p = parser("rgba16sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba16Float) {
    auto p = parser("rgba16float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba32Uint) {
    auto p = parser("rgba32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba32Sint) {
    auto p = parser("rgba32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, TexelFormat_Rgba32Float) {
    auto p = parser("rgba32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Float);
    EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace tint::reader::wgsl
