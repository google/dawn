// Copyright 2020 The Tint Authors->
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

#include "src/sem/storage_texture_type.h"

#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/test_helper.h"

namespace tint {
namespace sem {
namespace {

using StorageTextureTest = TestHelper;

TEST_F(StorageTextureTest, Dim) {
  auto* subtype =
      StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Float, Types());
  auto* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Float,
                                   ast::AccessControl::kReadWrite, subtype);
  EXPECT_EQ(s->dim(), ast::TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
  auto* subtype =
      StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Float, Types());
  auto* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Float,
                                   ast::AccessControl::kReadWrite, subtype);
  EXPECT_EQ(s->image_format(), ast::ImageFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, TypeName) {
  auto* subtype =
      StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Float, Types());
  auto* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Float,
                                   ast::AccessControl::kReadWrite, subtype);
  EXPECT_EQ(s->type_name(),
            "__storage_texture_2d_array_rgba32float_read_write");
}

TEST_F(StorageTextureTest, FriendlyName) {
  auto* subtype =
      StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Float, Types());
  auto* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Float,
                                   ast::AccessControl::kReadWrite, subtype);
  EXPECT_EQ(s->FriendlyName(Symbols()),
            "texture_storage_2d_array<rgba32float, read_write>");
}

TEST_F(StorageTextureTest, F32) {
  auto* subtype =
      sem::StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Float, Types());
  Type* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Float,
                                   ast::AccessControl::kReadWrite, subtype);

  auto program = Build();

  ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
  auto* subtype =
      sem::StorageTexture::SubtypeFor(ast::ImageFormat::kRg32Uint, Types());
  Type* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRg32Uint,
                                   ast::AccessControl::kReadWrite, subtype);

  auto program = Build();

  ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
  auto* subtype =
      sem::StorageTexture::SubtypeFor(ast::ImageFormat::kRgba32Sint, Types());
  Type* s = create<StorageTexture>(ast::TextureDimension::k2dArray,
                                   ast::ImageFormat::kRgba32Sint,
                                   ast::AccessControl::kReadWrite, subtype);

  auto program = Build();

  ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
  ASSERT_TRUE(s->Is<Texture>());
  ASSERT_TRUE(s->Is<StorageTexture>());
  EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<I32>());
}

}  // namespace
}  // namespace sem
}  // namespace tint
