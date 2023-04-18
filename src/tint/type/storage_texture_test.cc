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

#include "src/tint/type/storage_texture.h"

#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::type {
namespace {

struct StorageTextureTest : public TestHelper {
    StorageTexture* Create(TextureDimension dims,
                           builtin::TexelFormat fmt,
                           builtin::Access access) {
        auto* subtype = StorageTexture::SubtypeFor(fmt, Types());
        return create<StorageTexture>(dims, fmt, access, subtype);
    }
};

TEST_F(StorageTextureTest, Creation) {
    auto* a = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* b = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* c = Create(TextureDimension::k2d, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* d = Create(TextureDimension::kCube, builtin::TexelFormat::kR32Float,
                     builtin::Access::kReadWrite);
    auto* e =
        Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float, builtin::Access::kRead);

    EXPECT_TRUE(a->type()->Is<F32>());
    EXPECT_EQ(a->dim(), TextureDimension::kCube);

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(a, d);
    EXPECT_NE(a, e);
}

TEST_F(StorageTextureTest, Hash) {
    auto* a = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* b = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);

    EXPECT_EQ(a->unique_hash, b->unique_hash);
}

TEST_F(StorageTextureTest, Equals) {
    auto* a = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* b = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* c = Create(TextureDimension::k2d, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    auto* d = Create(TextureDimension::kCube, builtin::TexelFormat::kR32Float,
                     builtin::Access::kReadWrite);
    auto* e =
        Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float, builtin::Access::kRead);

    EXPECT_TRUE(a->Equals(*b));
    EXPECT_FALSE(a->Equals(*c));
    EXPECT_FALSE(a->Equals(*d));
    EXPECT_FALSE(a->Equals(*e));
    EXPECT_FALSE(a->Equals(Void{}));
}

TEST_F(StorageTextureTest, Dim) {
    auto* s = Create(TextureDimension::k2dArray, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    EXPECT_EQ(s->dim(), TextureDimension::k2dArray);
}

TEST_F(StorageTextureTest, Format) {
    auto* s = Create(TextureDimension::k2dArray, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    EXPECT_EQ(s->texel_format(), builtin::TexelFormat::kRgba32Float);
}

TEST_F(StorageTextureTest, FriendlyName) {
    auto* s = Create(TextureDimension::k2dArray, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);
    EXPECT_EQ(s->FriendlyName(), "texture_storage_2d_array<rgba32float, read_write>");
}

TEST_F(StorageTextureTest, F32) {
    auto* s = Create(TextureDimension::k2dArray, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<F32>());
}

TEST_F(StorageTextureTest, U32) {
    auto* subtype = StorageTexture::SubtypeFor(builtin::TexelFormat::kRg32Uint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, builtin::TexelFormat::kRg32Uint,
                                     builtin::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<U32>());
}

TEST_F(StorageTextureTest, I32) {
    auto* subtype = StorageTexture::SubtypeFor(builtin::TexelFormat::kRgba32Sint, Types());
    auto* s = create<StorageTexture>(TextureDimension::k2dArray, builtin::TexelFormat::kRgba32Sint,
                                     builtin::Access::kReadWrite, subtype);

    auto program = Build();

    ASSERT_TRUE(program.IsValid()) << program.Diagnostics().str();
    ASSERT_TRUE(s->Is<Texture>());
    ASSERT_TRUE(s->Is<StorageTexture>());
    EXPECT_TRUE(s->As<StorageTexture>()->type()->Is<I32>());
}

TEST_F(StorageTextureTest, Clone) {
    auto* a = Create(TextureDimension::kCube, builtin::TexelFormat::kRgba32Float,
                     builtin::Access::kReadWrite);

    type::Manager mgr;
    type::CloneContext ctx{{nullptr}, {nullptr, &mgr}};

    auto* mt = a->Clone(ctx);
    EXPECT_EQ(mt->dim(), TextureDimension::kCube);
    EXPECT_EQ(mt->texel_format(), builtin::TexelFormat::kRgba32Float);
    EXPECT_TRUE(mt->type()->Is<F32>());
}

}  // namespace
}  // namespace tint::type
