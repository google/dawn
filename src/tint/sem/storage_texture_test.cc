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

#include "src/tint/sem/storage_texture.h"

#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/test_helper.h"

namespace tint::sem {
namespace {

struct StorageTextureTest : public TestHelper {
    StorageTexture* Create(ast::TextureDimension dims, ast::TexelFormat fmt, ast::Access access) {
        auto* subtype = StorageTexture::SubtypeFor(fmt, Types());
        return create<StorageTexture>(dims, fmt, access, subtype);
    }
};

TEST_F(StorageTextureTest, Creation) {
    auto* a = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* b = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* c =
        Create(ast::TextureDimension::k2d, ast::TexelFormat::kRgba32Float, ast::Access::kReadWrite);
    auto* d =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kR32Float, ast::Access::kReadWrite);
    auto* e =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float, ast::Access::kRead);

    EXPECT_TRUE(a->type()->Is<F32>());
    EXPECT_EQ(a->dim(), ast::TextureDimension::kCube);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(StorageTextureTest, Hash) {
    auto* a = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* b = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* c =
        Create(ast::TextureDimension::k2d, ast::TexelFormat::kRgba32Float, ast::Access::kReadWrite);
    auto* d =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kR32Float, ast::Access::kReadWrite);
    auto* e =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float, ast::Access::kRead);

    EXPECT_EQ(a->Hash(), b->Hash());
    EXPECT_NE(a->Hash(), c->Hash());
    EXPECT_NE(a->Hash(), d->Hash());
    EXPECT_NE(a->Hash(), e->Hash());
}

TEST_F(StorageTextureTest, Equals) {
    auto* a = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* b = Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    auto* c =
        Create(ast::TextureDimension::k2d, ast::TexelFormat::kRgba32Float, ast::Access::kReadWrite);
    auto* d =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kR32Float, ast::Access::kReadWrite);
    auto* e =
        Create(ast::TextureDimension::kCube, ast::TexelFormat::kRgba32Float, ast::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(StorageTextureTest, Dim) {
    auto* s = Create(ast::TextureDimension::k2dArray, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    EXPECT_EQ(s->dim(), ast::TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
    auto* s = Create(ast::TextureDimension::k2dArray, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    EXPECT_EQ(s->texel_format(), ast::TexelFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, FriendlyName) {
    auto* s = Create(ast::TextureDimension::k2dArray, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);
    EXPECT_EQ(s->FriendlyName(Symbols()), "texture_storage_2d_array<rgba32float, read_write>");
}

TEST_F(StorageTextureTest, F32) {
    Type* s = Create(ast::TextureDimension::k2dArray, ast::TexelFormat::kRgba32Float,
                     ast::Access::kReadWrite);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
    auto* subtype = sem::StorageTexture::SubtypeFor(ast::TexelFormat::kRg32Uint, Types());
    Type* s = create<StorageTexture>(ast::TextureDimension::k2dArray, ast::TexelFormat::kRg32Uint,
                                     ast::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
    auto* subtype = sem::StorageTexture::SubtypeFor(ast::TexelFormat::kRgba32Sint, Types());
    Type* s = create<StorageTexture>(ast::TextureDimension::k2dArray, ast::TexelFormat::kRgba32Sint,
                                     ast::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<I32>());
}

}  // namespace
}  // namespace tint::sem
