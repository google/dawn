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

#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/writer/wgsl/test_helper.h"

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitType_Alias) {
  auto* alias = Alias("alias", ty.f32());
  auto* alias_ty = ty.Of(alias);
  WrapInFunction(Var("make_reachable", alias_ty));

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, alias_ty)) << gen.error();
  EXPECT_EQ(out.str(), "alias");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array) {
  auto* arr = ty.array<bool, 4>();
  Alias("make_type_reachable", arr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, arr)) << gen.error();
  EXPECT_EQ(out.str(), "array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_Attribute) {
  auto* a = ty.array(ty.bool_(), 4, 16u);
  Alias("make_type_reachable", a);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, a)) << gen.error();
  EXPECT_EQ(out.str(), "@stride(16) array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_RuntimeArray) {
  auto* a = ty.array(ty.bool_());
  Alias("make_type_reachable", a);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, a)) << gen.error();
  EXPECT_EQ(out.str(), "array<bool>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Bool) {
  auto* bool_ = ty.bool_();
  Alias("make_type_reachable", bool_);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, bool_)) << gen.error();
  EXPECT_EQ(out.str(), "bool");
}

TEST_F(WgslGeneratorImplTest, EmitType_F32) {
  auto* f32 = ty.f32();
  Alias("make_type_reachable", f32);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, f32)) << gen.error();
  EXPECT_EQ(out.str(), "f32");
}

TEST_F(WgslGeneratorImplTest, EmitType_I32) {
  auto* i32 = ty.i32();
  Alias("make_type_reachable", i32);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, i32)) << gen.error();
  EXPECT_EQ(out.str(), "i32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Matrix) {
  auto* mat2x3 = ty.mat2x3<f32>();
  Alias("make_type_reachable", mat2x3);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, mat2x3)) << gen.error();
  EXPECT_EQ(out.str(), "mat2x3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Pointer) {
  auto* p = ty.pointer<f32>(ast::StorageClass::kWorkgroup);
  Alias("make_type_reachable", p);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, p)) << gen.error();
  EXPECT_EQ(out.str(), "ptr<workgroup, f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_PointerAccessMode) {
  auto* p =
      ty.pointer<f32>(ast::StorageClass::kStorage, ast::Access::kReadWrite);
  Alias("make_type_reachable", p);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, p)) << gen.error();
  EXPECT_EQ(out.str(), "ptr<storage, f32, read_write>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32()),
                           });
  auto* s_ty = ty.Of(s);
  WrapInFunction(Var("make_reachable", s_ty));

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, s_ty)) << gen.error();
  EXPECT_EQ(out.str(), "S");
}

TEST_F(WgslGeneratorImplTest, EmitType_StructOffsetDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32(), {MemberOffset(8)}),
                               Member("b", ty.f32(), {MemberOffset(16)}),
                           });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  @size(8)
  padding : u32,
  a : i32,
  @size(4)
  padding_1 : u32,
  b : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_StructOffsetDecl_WithSymbolCollisions) {
  auto* s =
      Structure("S", {
                         Member("tint_0_padding", ty.i32(), {MemberOffset(8)}),
                         Member("tint_2_padding", ty.f32(), {MemberOffset(16)}),
                     });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  @size(8)
  padding : u32,
  tint_0_padding : i32,
  @size(4)
  padding_1 : u32,
  tint_2_padding : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_StructAlignDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32(), {MemberAlign(8)}),
                               Member("b", ty.f32(), {MemberAlign(16)}),
                           });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  @align(8)
  a : i32,
  @align(16)
  b : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_StructSizeDecl) {
  auto* s = Structure("S", {
                               Member("a", ty.i32(), {MemberSize(16)}),
                               Member("b", ty.f32(), {MemberSize(32)}),
                           });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  @size(16)
  a : i32,
  @size(32)
  b : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct_WithAttribute) {
  auto* s = Structure("S", {
                               Member("a", ty.i32()),
                               Member("b", ty.f32(), {MemberAlign(8)}),
                           });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  a : i32,
  @align(8)
  b : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct_WithEntryPointAttributes) {
  auto* s = Structure(
      "S", ast::StructMemberList{
               Member("a", ty.u32(), {Builtin(ast::Builtin::kVertexIndex)}),
               Member("b", ty.f32(), {Location(2u)})});

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  @builtin(vertex_index)
  a : u32,
  @location(2)
  b : f32,
}
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_U32) {
  auto* u32 = ty.u32();
  Alias("make_type_reachable", u32);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, u32)) << gen.error();
  EXPECT_EQ(out.str(), "u32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Vector) {
  auto* vec3 = ty.vec3<f32>();
  Alias("make_type_reachable", vec3);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, vec3)) << gen.error();
  EXPECT_EQ(out.str(), "vec3<f32>");
}

struct TextureData {
  ast::TextureDimension dim;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, TextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_DepthTextureTest = TestParamHelper<TextureData>;

TEST_P(WgslGenerator_DepthTextureTest, EmitType_DepthTexture) {
  auto param = GetParam();

  auto* d = ty.depth_texture(param.dim);
  Alias("make_type_reachable", d);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, d)) << gen.error();
  EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_DepthTextureTest,
    testing::Values(
        TextureData{ast::TextureDimension::k2d, "texture_depth_2d"},
        TextureData{ast::TextureDimension::k2dArray, "texture_depth_2d_array"},
        TextureData{ast::TextureDimension::kCube, "texture_depth_cube"},
        TextureData{ast::TextureDimension::kCubeArray,
                    "texture_depth_cube_array"}));

using WgslGenerator_SampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_F32) {
  auto param = GetParam();

  auto* t = ty.sampled_texture(param.dim, ty.f32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_I32) {
  auto param = GetParam();

  auto* t = ty.sampled_texture(param.dim, ty.i32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_U32) {
  auto param = GetParam();

  auto* t = ty.sampled_texture(param.dim, ty.u32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_SampledTextureTest,
    testing::Values(
        TextureData{ast::TextureDimension::k1d, "texture_1d"},
        TextureData{ast::TextureDimension::k2d, "texture_2d"},
        TextureData{ast::TextureDimension::k2dArray, "texture_2d_array"},
        TextureData{ast::TextureDimension::k3d, "texture_3d"},
        TextureData{ast::TextureDimension::kCube, "texture_cube"},
        TextureData{ast::TextureDimension::kCubeArray, "texture_cube_array"}));

using WgslGenerator_MultiampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_F32) {
  auto param = GetParam();

  auto* t = ty.multisampled_texture(param.dim, ty.f32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_I32) {
  auto param = GetParam();

  auto* t = ty.multisampled_texture(param.dim, ty.i32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_U32) {
  auto param = GetParam();

  auto* t = ty.multisampled_texture(param.dim, ty.u32());
  Alias("make_type_reachable", t);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(WgslGeneratorImplTest,
                         WgslGenerator_MultiampledTextureTest,
                         testing::Values(TextureData{
                             ast::TextureDimension::k2d,
                             "texture_multisampled_2d"}));

struct StorageTextureData {
  ast::TexelFormat fmt;
  ast::TextureDimension dim;
  ast::Access access;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, StorageTextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_StorageTextureTest = TestParamHelper<StorageTextureData>;
TEST_P(WgslGenerator_StorageTextureTest, EmitType_StorageTexture) {
  auto param = GetParam();

  auto* t = ty.storage_texture(param.dim, param.fmt, param.access);
  Global("g", t,
         ast::AttributeList{
             create<ast::BindingAttribute>(1),
             create<ast::GroupAttribute>(2),
         });

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, t)) << gen.error();
  EXPECT_EQ(out.str(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_StorageTextureTest,
    testing::Values(
        StorageTextureData{ast::TexelFormat::kRgba8Sint,
                           ast::TextureDimension::k1d, ast::Access::kWrite,
                           "texture_storage_1d<rgba8sint, write>"},
        StorageTextureData{ast::TexelFormat::kRgba8Sint,
                           ast::TextureDimension::k2d, ast::Access::kWrite,
                           "texture_storage_2d<rgba8sint, write>"},
        StorageTextureData{ast::TexelFormat::kRgba8Sint,
                           ast::TextureDimension::k2dArray, ast::Access::kWrite,
                           "texture_storage_2d_array<rgba8sint, write>"},
        StorageTextureData{ast::TexelFormat::kRgba8Sint,
                           ast::TextureDimension::k3d, ast::Access::kWrite,
                           "texture_storage_3d<rgba8sint, write>"}));

struct ImageFormatData {
  ast::TexelFormat fmt;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatData data) {
  out << data.name;
  return out;
}
using WgslGenerator_ImageFormatTest = TestParamHelper<ImageFormatData>;
TEST_P(WgslGenerator_ImageFormatTest, EmitType_StorageTexture_ImageFormat) {
  auto param = GetParam();

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitImageFormat(out, param.fmt)) << gen.error();
  EXPECT_EQ(out.str(), param.name);
}

INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_ImageFormatTest,
    testing::Values(
        ImageFormatData{ast::TexelFormat::kR32Uint, "r32uint"},
        ImageFormatData{ast::TexelFormat::kR32Sint, "r32sint"},
        ImageFormatData{ast::TexelFormat::kR32Float, "r32float"},
        ImageFormatData{ast::TexelFormat::kRgba8Unorm, "rgba8unorm"},
        ImageFormatData{ast::TexelFormat::kRgba8Snorm, "rgba8snorm"},
        ImageFormatData{ast::TexelFormat::kRgba8Uint, "rgba8uint"},
        ImageFormatData{ast::TexelFormat::kRgba8Sint, "rgba8sint"},
        ImageFormatData{ast::TexelFormat::kRg32Uint, "rg32uint"},
        ImageFormatData{ast::TexelFormat::kRg32Sint, "rg32sint"},
        ImageFormatData{ast::TexelFormat::kRg32Float, "rg32float"},
        ImageFormatData{ast::TexelFormat::kRgba16Uint, "rgba16uint"},
        ImageFormatData{ast::TexelFormat::kRgba16Sint, "rgba16sint"},
        ImageFormatData{ast::TexelFormat::kRgba16Float, "rgba16float"},
        ImageFormatData{ast::TexelFormat::kRgba32Uint, "rgba32uint"},
        ImageFormatData{ast::TexelFormat::kRgba32Sint, "rgba32sint"},
        ImageFormatData{ast::TexelFormat::kRgba32Float, "rgba32float"}));

TEST_F(WgslGeneratorImplTest, EmitType_Sampler) {
  auto* sampler = ty.sampler(ast::SamplerKind::kSampler);
  Alias("make_type_reachable", sampler);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, sampler)) << gen.error();
  EXPECT_EQ(out.str(), "sampler");
}

TEST_F(WgslGeneratorImplTest, EmitType_SamplerComparison) {
  auto* sampler = ty.sampler(ast::SamplerKind::kComparisonSampler);
  Alias("make_type_reachable", sampler);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitType(out, sampler)) << gen.error();
  EXPECT_EQ(out.str(), "sampler_comparison");
}

}  // namespace
}  // namespace tint::writer::wgsl
