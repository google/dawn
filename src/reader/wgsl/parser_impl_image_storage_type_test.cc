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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ImageStorageType_Invalid) {
  auto p = parser("1234");
  auto t = p->expect_image_storage_type("test");
  EXPECT_TRUE(t.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:1: invalid format for test");
}

TEST_F(ParserImplTest, ImageStorageType_R8Unorm) {
  auto p = parser("r8unorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR8Unorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R8Snorm) {
  auto p = parser("r8snorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR8Snorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R8Uint) {
  auto p = parser("r8uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR8Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R8Sint) {
  auto p = parser("r8sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR8Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R16Uint) {
  auto p = parser("r16uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR16Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R16Sint) {
  auto p = parser("r16sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR16Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R16Float) {
  auto p = parser("r16float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR16Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg8Unorm) {
  auto p = parser("rg8unorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg8Unorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg8Snorm) {
  auto p = parser("rg8snorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg8Snorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg8Uint) {
  auto p = parser("rg8uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg8Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg8Sint) {
  auto p = parser("rg8sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg8Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R32Uint) {
  auto p = parser("r32uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR32Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R32Sint) {
  auto p = parser("r32sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR32Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_R32Float) {
  auto p = parser("r32float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kR32Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg16Uint) {
  auto p = parser("rg16uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg16Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg16Sint) {
  auto p = parser("rg16sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg16Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg16Float) {
  auto p = parser("rg16float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg16Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Unorm) {
  auto p = parser("rgba8unorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba8Unorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8UnormSrgb) {
  auto p = parser("rgba8unorm_srgb");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba8UnormSrgb);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Snorm) {
  auto p = parser("rgba8snorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba8Snorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Uint) {
  auto p = parser("rgba8uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba8Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba8Sint) {
  auto p = parser("rgba8sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba8Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Bgra8Unorm) {
  auto p = parser("bgra8unorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kBgra8Unorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Bgra8UnormSrgb) {
  auto p = parser("bgra8unorm_srgb");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kBgra8UnormSrgb);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgb10A2Unorm) {
  auto p = parser("rgb10a2unorm");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgb10A2Unorm);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg11B10Float) {
  auto p = parser("rg11b10float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg11B10Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Uint) {
  auto p = parser("rg32uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg32Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Sint) {
  auto p = parser("rg32sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg32Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rg32Float) {
  auto p = parser("rg32float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRg32Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Uint) {
  auto p = parser("rgba16uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba16Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Sint) {
  auto p = parser("rgba16sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba16Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba16Float) {
  auto p = parser("rgba16float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba16Float);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Uint) {
  auto p = parser("rgba32uint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba32Uint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Sint) {
  auto p = parser("rgba32sint");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba32Sint);
  EXPECT_FALSE(p->has_error());
}

TEST_F(ParserImplTest, ImageStorageType_Rgba32Float) {
  auto p = parser("rgba32float");
  auto t = p->expect_image_storage_type("test");
  EXPECT_FALSE(t.errored);
  EXPECT_EQ(t.value, ast::ImageFormat::kRgba32Float);
  EXPECT_FALSE(p->has_error());
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
