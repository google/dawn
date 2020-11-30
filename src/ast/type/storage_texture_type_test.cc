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

#include <memory>

#include "src/ast/identifier_expression.h"
#include "src/ast/test_helper.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/type_determiner.h"

namespace tint {
namespace ast {
namespace type {
namespace {

using StorageTextureTypeTest = TestHelper;

TEST_F(StorageTextureTypeTest, Is) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  Type* ty = &s;
  EXPECT_FALSE(ty->Is<AccessControlType>());
  EXPECT_FALSE(ty->Is<AliasType>());
  EXPECT_FALSE(ty->Is<ArrayType>());
  EXPECT_FALSE(ty->Is<BoolType>());
  EXPECT_FALSE(ty->Is<F32Type>());
  EXPECT_FALSE(ty->Is<I32Type>());
  EXPECT_FALSE(ty->Is<MatrixType>());
  EXPECT_FALSE(ty->Is<PointerType>());
  EXPECT_FALSE(ty->IsSampler());
  EXPECT_FALSE(ty->IsStruct());
  EXPECT_TRUE(ty->IsTexture());
  EXPECT_FALSE(ty->IsU32());
  EXPECT_FALSE(ty->IsVector());
}

TEST_F(StorageTextureTypeTest, IsTextureType) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  EXPECT_FALSE(s.IsDepth());
  EXPECT_FALSE(s.IsSampled());
  EXPECT_TRUE(s.IsStorage());
}

TEST_F(StorageTextureTypeTest, Dim) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTypeTest, Access) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.access(), AccessControl::kReadOnly);
}

TEST_F(StorageTextureTypeTest, Format) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.image_format(), ImageFormat::kRgba32Float);
}

TEST_F(StorageTextureTypeTest, TypeName) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Float);
  EXPECT_EQ(s.type_name(), "__storage_texture_read_only_2d_array_rgba32float");
}

TEST_F(StorageTextureTypeTest, F32Type) {
  Context ctx;
  ast::Module mod;
  ast::type::Type* s = mod.create<StorageTextureType>(
      TextureDimension::k2dArray, AccessControl::kReadOnly,
      ImageFormat::kRgba32Float);
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->Is<F32Type>());
}

TEST_F(StorageTextureTypeTest, U32Type) {
  Context ctx;
  ast::Module mod;
  ast::type::Type* s = mod.create<StorageTextureType>(
      TextureDimension::k2dArray, AccessControl::kReadOnly,
      ImageFormat::kRg32Uint);
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->IsU32());
}

TEST_F(StorageTextureTypeTest, I32Type) {
  Context ctx;
  ast::Module mod;
  ast::type::Type* s = mod.create<StorageTextureType>(
      TextureDimension::k2dArray, AccessControl::kReadOnly,
      ImageFormat::kRgba32Sint);
  TypeDeterminer td(&ctx, &mod);

  ASSERT_TRUE(td.Determine()) << td.error();
  ASSERT_TRUE(s->IsTexture());
  ASSERT_TRUE(s->AsTexture()->IsStorage());
  EXPECT_TRUE(s->AsTexture()->AsStorage()->type()->Is<I32Type>());
}

TEST_F(StorageTextureTypeTest, MinBufferBindingSize) {
  StorageTextureType s(TextureDimension::k2dArray, AccessControl::kReadOnly,
                       ImageFormat::kRgba32Sint);
  EXPECT_EQ(0u, s.MinBufferBindingSize(MemoryLayout::kUniformBuffer));
}

}  // namespace
}  // namespace type
}  // namespace ast
}  // namespace tint
