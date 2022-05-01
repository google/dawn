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

TEST_F(ParserImplTest, ImageStorageType_Invalid) {
    auto p = parser("1234");
    auto t = p->expect_texel_format("test");
    EXPECT_TRUE(t.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:1: invalid format for test");
}

TEST_F(ParserImplTest, ImageStorageType_R32Uint) {
    auto p = parser("r32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R32Sint) {
    auto p = parser("r32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R32Float) {
    auto p = parser("r32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kR32Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Unorm) {
    auto p = parser("rgba8unorm");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Unorm);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Snorm) {
    auto p = parser("rgba8snorm");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Snorm);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Uint) {
    auto p = parser("rgba8uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Sint) {
    auto p = parser("rgba8sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba8Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Uint) {
    auto p = parser("rg32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Sint) {
    auto p = parser("rg32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Float) {
    auto p = parser("rg32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRg32Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Uint) {
    auto p = parser("rgba16uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Sint) {
    auto p = parser("rgba16sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Float) {
    auto p = parser("rgba16float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba16Float);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Uint) {
    auto p = parser("rgba32uint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Uint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Sint) {
    auto p = parser("rgba32sint");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Sint);
    EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Float) {
    auto p = parser("rgba32float");
    auto t = p->expect_texel_format("test");
    EXPECT_FALSE(t.errored);
    EXPECT_EQ(t.value, ast::TexelFormat::kRgba32Float);
    EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace tint::reader::wgsl
