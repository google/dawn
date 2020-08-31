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

#include "src/ast/type/storage_texture_type.h"

#include "src/ast/identifier_expression.h"
#include "src/type_determiner.h"

#include "gtest/gtest.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using StorageTextureTypeTest = testing::Test;

TEST_F(StorageTextureTypeTest, Is) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_FALSE(s.IsAlias());
  EXPECT_FALSE(s.IsArray());
  EXPECT_FALSE(s.IsBool());
  EXPECT_FALSE(s.IsF32());
  EXPECT_FALSE(s.IsI32());
  EXPECT_FALSE(s.IsMatrix());
  EXPECT_FALSE(s.IsPointer());
  EXPECT_FALSE(s.IsSampler());
  EXPECT_FALSE(s.IsStruct());
  EXPECT_TRUE(s.IsTexture());
  EXPECT_FALSE(s.IsU32());
  EXPECT_FALSE(s.IsVector());
}

TEST_F(StorageTextureTypeTest, IsTextureType) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_FALSE(s.IsDepth());
  EXPECT_FALSE(s.IsSampled());
  EXPECT_TRUE(s.IsStorage());
}

TEST_F(StorageTextureTypeTest, Dim) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTypeTest, Access) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.access(), StorageAccess::kRead);
}

TEST_F(StorageTextureTypeTest, Format) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.image_format(), ImageFormat::kRgba32Float);
}

TEST_F(StorageTextureTypeTest, TypeName) {
  StorageTextureType s(TextureDimension::k2dArray, StorageAccess::kRead,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.type_name(), "__storage_texture_read_2d_array_rgba32float");
}

TEST_F(StorageTextureTypeTest, F32Type) {
  Context ctx;
  ast::type::Type* s = ctx.type_mgr().Get(std::make_unique<StorageTextureType>(
      TextureDimension::k2dArray, StorageAccess::kRead,
      ImageFormat::kRgba32Float));
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->IsF32());
}

TEST_F(StorageTextureTypeTest, U32Type) {
  Context ctx;
  ast::type::Type* s = ctx.type_mgr().Get(std::make_unique<StorageTextureType>(
      TextureDimension::k2dArray, StorageAccess::kRead,
      ImageFormat::kRgba8Unorm));
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->IsU32());
}

TEST_F(StorageTextureTypeTest, I32Type) {
  Context ctx;
  ast::type::Type* s = ctx.type_mgr().Get(std::make_unique<StorageTextureType>(
      TextureDimension::k2dArray, StorageAccess::kRead,
      ImageFormat::kRgba32Sint));
  ast::Module mod;
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->IsI32());
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
