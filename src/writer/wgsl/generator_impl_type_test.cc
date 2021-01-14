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

#include "gtest/gtest.h"
#include "src/ast/access_control.h"
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/access_control_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/multisampled_texture_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/sampled_texture_type.h"
#include "src/ast/type/sampler_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/writer/wgsl/generator_impl.h"
#include "src/writer/wgsl/test_helper.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, EmitType_Alias) {
  auto* alias = ty.alias("alias", ty.f32);

  ASSERT_TRUE(gen.EmitType(alias)) << gen.error();
  EXPECT_EQ(gen.result(), "alias");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array) {
  ASSERT_TRUE(gen.EmitType(ty.array<bool, 4>())) << gen.error();
  EXPECT_EQ(gen.result(), "array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_AccessControl_Read) {
  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::StructDecorationList decos;
  decos.push_back(block_deco);

  auto* str =
      create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32)}, decos);
  auto* s = ty.struct_("S", str);

  ast::type::AccessControl a(ast::AccessControl::kReadOnly, s);

  ASSERT_TRUE(gen.EmitType(&a)) << gen.error();
  EXPECT_EQ(gen.result(), "[[access(read)]]\nS");
}

TEST_F(WgslGeneratorImplTest, EmitType_AccessControl_ReadWrite) {
  auto* block_deco = create<ast::StructBlockDecoration>();
  ast::StructDecorationList decos;
  decos.push_back(block_deco);

  auto* str =
      create<ast::Struct>(ast::StructMemberList{Member("a", ty.i32)}, decos);
  auto* s = ty.struct_("S", str);

  ast::type::AccessControl a(ast::AccessControl::kReadWrite, s);

  ASSERT_TRUE(gen.EmitType(&a)) << gen.error();
  EXPECT_EQ(gen.result(), "[[access(read_write)]]\nS");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_Decoration) {
  ast::type::Array a(ty.bool_, 4,
                     ast::ArrayDecorationList{
                         create<ast::StrideDecoration>(16u),
                     });

  ASSERT_TRUE(gen.EmitType(&a)) << gen.error();
  EXPECT_EQ(gen.result(), "[[stride(16)]] array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_MultipleDecorations) {
  ast::type::Array a(ty.bool_, 4,
                     ast::ArrayDecorationList{
                         create<ast::StrideDecoration>(16u),
                         create<ast::StrideDecoration>(32u),
                     });

  ASSERT_TRUE(gen.EmitType(&a)) << gen.error();
  EXPECT_EQ(gen.result(), "[[stride(16)]] [[stride(32)]] array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::Array a(ty.bool_, 0, ast::ArrayDecorationList{});

  ASSERT_TRUE(gen.EmitType(&a)) << gen.error();
  EXPECT_EQ(gen.result(), "array<bool>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Bool) {
  ASSERT_TRUE(gen.EmitType(ty.bool_)) << gen.error();
  EXPECT_EQ(gen.result(), "bool");
}

TEST_F(WgslGeneratorImplTest, EmitType_F32) {
  ASSERT_TRUE(gen.EmitType(ty.f32)) << gen.error();
  EXPECT_EQ(gen.result(), "f32");
}

TEST_F(WgslGeneratorImplTest, EmitType_I32) {
  ASSERT_TRUE(gen.EmitType(ty.i32)) << gen.error();
  EXPECT_EQ(gen.result(), "i32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Matrix) {
  ASSERT_TRUE(gen.EmitType(ty.mat2x3<f32>())) << gen.error();
  EXPECT_EQ(gen.result(), "mat2x3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Pointer) {
  ast::type::Pointer p(ty.f32, ast::StorageClass::kWorkgroup);

  ASSERT_TRUE(gen.EmitType(&p)) << gen.error();
  EXPECT_EQ(gen.result(), "ptr<workgroup, f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);
  ASSERT_TRUE(gen.EmitType(s)) << gen.error();
  EXPECT_EQ(gen.result(), "S");
}

TEST_F(WgslGeneratorImplTest, EmitType_StructDecl) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);
  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(struct S {
  a : i32;
  [[offset(4)]]
  b : f32;
};
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct_WithDecoration) {
  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());

  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.i32),
                            Member("b", ty.f32, {MemberOffset(4)})},
      decos);

  auto* s = ty.struct_("S", str);
  ASSERT_TRUE(gen.EmitStructType(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"([[block]]
struct S {
  a : i32;
  [[offset(4)]]
  b : f32;
};
)");
}

TEST_F(WgslGeneratorImplTest, EmitType_U32) {
  ASSERT_TRUE(gen.EmitType(ty.u32)) << gen.error();
  EXPECT_EQ(gen.result(), "u32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Vector) {
  ASSERT_TRUE(gen.EmitType(ty.vec3<f32>())) << gen.error();
  EXPECT_EQ(gen.result(), "vec3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Void) {
  ASSERT_TRUE(gen.EmitType(ty.void_)) << gen.error();
  EXPECT_EQ(gen.result(), "void");
}

struct TextureData {
  ast::type::TextureDimension dim;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, TextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_DepthTextureTest = TestParamHelper<TextureData>;

TEST_P(WgslGenerator_DepthTextureTest, EmitType_DepthTexture) {
  auto param = GetParam();

  ast::type::DepthTexture d(param.dim);

  ASSERT_TRUE(gen.EmitType(&d)) << gen.error();
  EXPECT_EQ(gen.result(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_DepthTextureTest,
    testing::Values(
        TextureData{ast::type::TextureDimension::k2d, "texture_depth_2d"},
        TextureData{ast::type::TextureDimension::k2dArray,
                    "texture_depth_2d_array"},
        TextureData{ast::type::TextureDimension::kCube, "texture_depth_cube"},
        TextureData{ast::type::TextureDimension::kCubeArray,
                    "texture_depth_cube_array"}));

using WgslGenerator_SampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_F32) {
  auto param = GetParam();

  ast::type::SampledTexture t(param.dim, ty.f32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_I32) {
  auto param = GetParam();

  ast::type::SampledTexture t(param.dim, ty.i32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_U32) {
  auto param = GetParam();

  ast::type::SampledTexture t(param.dim, ty.u32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_SampledTextureTest,
    testing::Values(
        TextureData{ast::type::TextureDimension::k1d, "texture_1d"},
        TextureData{ast::type::TextureDimension::k1dArray, "texture_1d_array"},
        TextureData{ast::type::TextureDimension::k2d, "texture_2d"},
        TextureData{ast::type::TextureDimension::k2dArray, "texture_2d_array"},
        TextureData{ast::type::TextureDimension::k3d, "texture_3d"},
        TextureData{ast::type::TextureDimension::kCube, "texture_cube"},
        TextureData{ast::type::TextureDimension::kCubeArray,
                    "texture_cube_array"}));

using WgslGenerator_MultiampledTextureTest = TestParamHelper<TextureData>;
TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_F32) {
  auto param = GetParam();

  ast::type::MultisampledTexture t(param.dim, ty.f32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_I32) {
  auto param = GetParam();

  ast::type::MultisampledTexture t(param.dim, ty.i32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_U32) {
  auto param = GetParam();

  ast::type::MultisampledTexture t(param.dim, ty.u32);

  ASSERT_TRUE(gen.EmitType(&t)) << gen.error();
  EXPECT_EQ(gen.result(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(WgslGeneratorImplTest,
                         WgslGenerator_MultiampledTextureTest,
                         testing::Values(TextureData{
                             ast::type::TextureDimension::k2d,
                             "texture_multisampled_2d"}));

struct StorageTextureData {
  ast::type::ImageFormat fmt;
  ast::type::TextureDimension dim;
  ast::AccessControl access;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, StorageTextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_StorageTextureTest = TestParamHelper<StorageTextureData>;
TEST_P(WgslGenerator_StorageTextureTest, EmitType_StorageTexture) {
  auto param = GetParam();

  ast::type::StorageTexture t(param.dim, param.fmt);
  ast::type::AccessControl ac(param.access, &t);

  ASSERT_TRUE(gen.EmitType(&ac)) << gen.error();
  EXPECT_EQ(gen.result(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_StorageTextureTest,
    testing::Values(
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k1d,
                           ast::AccessControl::kReadOnly,
                           "[[access(read)]]\ntexture_storage_1d<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm,
            ast::type::TextureDimension::k1dArray,
            ast::AccessControl::kReadOnly,
            "[[access(read)]]\ntexture_storage_1d_array<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k2d,
                           ast::AccessControl::kReadOnly,
                           "[[access(read)]]\ntexture_storage_2d<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm,
            ast::type::TextureDimension::k2dArray,
            ast::AccessControl::kReadOnly,
            "[[access(read)]]\ntexture_storage_2d_array<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k3d,
                           ast::AccessControl::kReadOnly,
                           "[[access(read)]]\ntexture_storage_3d<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k1d,
                           ast::AccessControl::kWriteOnly,
                           "[[access(write)]]\ntexture_storage_1d<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm,
            ast::type::TextureDimension::k1dArray,
            ast::AccessControl::kWriteOnly,
            "[[access(write)]]\ntexture_storage_1d_array<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k2d,
                           ast::AccessControl::kWriteOnly,
                           "[[access(write)]]\ntexture_storage_2d<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm,
            ast::type::TextureDimension::k2dArray,
            ast::AccessControl::kWriteOnly,
            "[[access(write)]]\ntexture_storage_2d_array<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k3d,
                           ast::AccessControl::kWriteOnly,
                           "[[access(write)]]\ntexture_storage_3d<r8unorm>"}));

struct ImageFormatData {
  ast::type::ImageFormat fmt;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatData data) {
  out << data.name;
  return out;
}
using WgslGenerator_ImageFormatTest = TestParamHelper<ImageFormatData>;
TEST_P(WgslGenerator_ImageFormatTest, EmitType_StorageTexture_ImageFormat) {
  auto param = GetParam();

  ASSERT_TRUE(gen.EmitImageFormat(param.fmt)) << gen.error();
  EXPECT_EQ(gen.result(), param.name);
}

INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_ImageFormatTest,
    testing::Values(
        ImageFormatData{ast::type::ImageFormat::kR8Unorm, "r8unorm"},
        ImageFormatData{ast::type::ImageFormat::kR8Snorm, "r8snorm"},
        ImageFormatData{ast::type::ImageFormat::kR8Uint, "r8uint"},
        ImageFormatData{ast::type::ImageFormat::kR8Sint, "r8sint"},
        ImageFormatData{ast::type::ImageFormat::kR16Uint, "r16uint"},
        ImageFormatData{ast::type::ImageFormat::kR16Sint, "r16sint"},
        ImageFormatData{ast::type::ImageFormat::kR16Float, "r16float"},
        ImageFormatData{ast::type::ImageFormat::kRg8Unorm, "rg8unorm"},
        ImageFormatData{ast::type::ImageFormat::kRg8Snorm, "rg8snorm"},
        ImageFormatData{ast::type::ImageFormat::kRg8Uint, "rg8uint"},
        ImageFormatData{ast::type::ImageFormat::kRg8Sint, "rg8sint"},
        ImageFormatData{ast::type::ImageFormat::kR32Uint, "r32uint"},
        ImageFormatData{ast::type::ImageFormat::kR32Sint, "r32sint"},
        ImageFormatData{ast::type::ImageFormat::kR32Float, "r32float"},
        ImageFormatData{ast::type::ImageFormat::kRg16Uint, "rg16uint"},
        ImageFormatData{ast::type::ImageFormat::kRg16Sint, "rg16sint"},
        ImageFormatData{ast::type::ImageFormat::kRg16Float, "rg16float"},
        ImageFormatData{ast::type::ImageFormat::kRgba8Unorm, "rgba8unorm"},
        ImageFormatData{ast::type::ImageFormat::kRgba8UnormSrgb,
                        "rgba8unorm_srgb"},
        ImageFormatData{ast::type::ImageFormat::kRgba8Snorm, "rgba8snorm"},
        ImageFormatData{ast::type::ImageFormat::kRgba8Uint, "rgba8uint"},
        ImageFormatData{ast::type::ImageFormat::kRgba8Sint, "rgba8sint"},
        ImageFormatData{ast::type::ImageFormat::kBgra8Unorm, "bgra8unorm"},
        ImageFormatData{ast::type::ImageFormat::kBgra8UnormSrgb,
                        "bgra8unorm_srgb"},
        ImageFormatData{ast::type::ImageFormat::kRgb10A2Unorm, "rgb10a2unorm"},
        ImageFormatData{ast::type::ImageFormat::kRg11B10Float, "rg11b10float"},
        ImageFormatData{ast::type::ImageFormat::kRg32Uint, "rg32uint"},
        ImageFormatData{ast::type::ImageFormat::kRg32Sint, "rg32sint"},
        ImageFormatData{ast::type::ImageFormat::kRg32Float, "rg32float"},
        ImageFormatData{ast::type::ImageFormat::kRgba16Uint, "rgba16uint"},
        ImageFormatData{ast::type::ImageFormat::kRgba16Sint, "rgba16sint"},
        ImageFormatData{ast::type::ImageFormat::kRgba16Float, "rgba16float"},
        ImageFormatData{ast::type::ImageFormat::kRgba32Uint, "rgba32uint"},
        ImageFormatData{ast::type::ImageFormat::kRgba32Sint, "rgba32sint"},
        ImageFormatData{ast::type::ImageFormat::kRgba32Float, "rgba32float"}));

TEST_F(WgslGeneratorImplTest, EmitType_Sampler) {
  ast::type::Sampler sampler(ast::type::SamplerKind::kSampler);

  ASSERT_TRUE(gen.EmitType(&sampler)) << gen.error();
  EXPECT_EQ(gen.result(), "sampler");
}

TEST_F(WgslGeneratorImplTest, EmitType_SamplerComparison) {
  ast::type::Sampler sampler(ast::type::SamplerKind::kComparisonSampler);

  ASSERT_TRUE(gen.EmitType(&sampler)) << gen.error();
  EXPECT_EQ(gen.result(), "sampler_comparison");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
