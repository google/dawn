// Copyright 2022 The Tint Authors
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

#include "src/tint/lang/core/type/storage_texture.h"

#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/helper_test.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"

namespace tint::type {
namespace {

struct StorageTextureTest : public TestHelper {
    StorageTexture* Create(TextureDimension dims, core::TexelFormat fmt, core::Access access) {
        auto* subtype = StorageTexture::SubtypeFor(fmt, Types());
        return create<StorageTexture>(dims, fmt, access, subtype);
    }
};

TEST_F(StorageTextureTest, Creation) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* c =
        Create(TextureDimension::k2d, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* d =
        Create(TextureDimension::kCube, core::TexelFormat::kR32Float, core::Access::kReadWrite);
    auto* e = Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kRead);

    EXPECT_TRUE(a->type()->Is<F32>());
    EXPECT_EQ(a->dim(), TextureDimension::kCube);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(StorageTextureTest, Hash) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(StorageTextureTest, Equals) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* b =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* c =
        Create(TextureDimension::k2d, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);
    auto* d =
        Create(TextureDimension::kCube, core::TexelFormat::kR32Float, core::Access::kReadWrite);
    auto* e = Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(StorageTextureTest, Dim) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->texel_format(), core::TexelFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, FriendlyName) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);
    EXPECT_EQ(s->FriendlyName(), "texture_storage_2d_array<rgba32float, read_write>");
}

TEST_F(StorageTextureTest, F32) {
    auto* s = Create(TextureDimension::k2dArray, core::TexelFormat::kRgba32Float,
                     core::Access::kReadWrite);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
    auto* subtype = StorageTexture::SubtypeFor(core::TexelFormat::kRg32Uint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, core::TexelFormat::kRg32Uint,
                                     core::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
    auto* subtype = StorageTexture::SubtypeFor(core::TexelFormat::kRgba32Sint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, core::TexelFormat::kRgba32Sint,
                                     core::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<I32>());
}

TEST_F(StorageTextureTest, Clone) {
    auto* a =
        Create(TextureDimension::kCube, core::TexelFormat::kRgba32Float, core::Access::kReadWrite);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->dim(), TextureDimension::kCube);
    EXPECT_EQ(mt->texel_format(), core::TexelFormat::kRgba32Float);
    EXPECT_TRUE(mt->type()->Is<F32>());
}

}  // namespace
}  // namespace tint::type
