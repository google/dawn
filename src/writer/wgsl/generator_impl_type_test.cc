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
#include "src/ast/stride_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
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

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using WgslGeneratorImplTest = testing::Test;

TEST_F(WgslGeneratorImplTest, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&alias)) << g.error();
  EXPECT_EQ(g.result(), "alias");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_Decoration) {
  ast::type::BoolType b;
  ast::ArrayDecorationList decos;
  decos.push_back(std::make_unique<ast::StrideDecoration>(16u));

  ast::type::ArrayType a(&b, 4);
  a.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "[[stride(16)]] array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Array_MultipleDecorations) {
  ast::type::BoolType b;
  ast::ArrayDecorationList decos;
  decos.push_back(std::make_unique<ast::StrideDecoration>(16u));
  decos.push_back(std::make_unique<ast::StrideDecoration>(32u));

  ast::type::ArrayType a(&b, 4);
  a.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "[[stride(16)]] [[stride(32)]] array<bool, 4>");
}

TEST_F(WgslGeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Bool) {
  ast::type::BoolType b;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&b)) << g.error();
  EXPECT_EQ(g.result(), "bool");
}

TEST_F(WgslGeneratorImplTest, EmitType_F32) {
  ast::type::F32Type f32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&f32)) << g.error();
  EXPECT_EQ(g.result(), "f32");
}

TEST_F(WgslGeneratorImplTest, EmitType_I32) {
  ast::type::I32Type i32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&i32)) << g.error();
  EXPECT_EQ(g.result(), "i32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&m)) << g.error();
  EXPECT_EQ(g.result(), "mat2x3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&p)) << g.error();
  EXPECT_EQ(g.result(), "ptr<workgroup, f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s("S", std::move(str));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
  EXPECT_EQ(g.result(), R"(struct {
  a : i32;
  [[offset(4)]]
  b : f32;
})");
}

TEST_F(WgslGeneratorImplTest, EmitType_Struct_WithDecoration) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  ast::StructMemberList members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  ast::StructDecorationList decos;
  decos.push_back(ast::StructDecoration::kBlock);

  auto str =
      std::make_unique<ast::Struct>(std::move(decos), std::move(members));

  ast::type::StructType s("S", std::move(str));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
  EXPECT_EQ(g.result(), R"([[block]]
struct {
  a : i32;
  [[offset(4)]]
  b : f32;
})");
}

TEST_F(WgslGeneratorImplTest, EmitType_U32) {
  ast::type::U32Type u32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&u32)) << g.error();
  EXPECT_EQ(g.result(), "u32");
}

TEST_F(WgslGeneratorImplTest, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "vec3<f32>");
}

TEST_F(WgslGeneratorImplTest, EmitType_Void) {
  ast::type::VoidType v;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "void");
}

struct TextureData {
  ast::type::TextureDimension dim;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, TextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_DepthTextureTest = testing::TestWithParam<TextureData>;

TEST_P(WgslGenerator_DepthTextureTest, EmitType_DepthTexture) {
  auto param = GetParam();

  ast::type::DepthTextureType d(param.dim);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&d)) << g.error();
  EXPECT_EQ(g.result(), param.name);
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

using WgslGenerator_SampledTextureTest = testing::TestWithParam<TextureData>;
TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_F32) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::SampledTextureType t(param.dim, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_I32) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::SampledTextureType t(param.dim, &i32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_SampledTextureTest, EmitType_SampledTexture_U32) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::SampledTextureType t(param.dim, &u32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_SampledTextureTest,
    testing::Values(
        TextureData{ast::type::TextureDimension::k1d, "texture_sampled_1d"},
        TextureData{ast::type::TextureDimension::k1dArray,
                    "texture_sampled_1d_array"},
        TextureData{ast::type::TextureDimension::k2d, "texture_sampled_2d"},
        TextureData{ast::type::TextureDimension::k2dArray,
                    "texture_sampled_2d_array"},
        TextureData{ast::type::TextureDimension::k3d, "texture_sampled_3d"},
        TextureData{ast::type::TextureDimension::kCube, "texture_sampled_cube"},
        TextureData{ast::type::TextureDimension::kCubeArray,
                    "texture_sampled_cube_array"}));

using WgslGenerator_MultiampledTextureTest =
    testing::TestWithParam<TextureData>;
TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_F32) {
  auto param = GetParam();

  ast::type::F32Type f32;
  ast::type::MultisampledTextureType t(param.dim, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<f32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_I32) {
  auto param = GetParam();

  ast::type::I32Type i32;
  ast::type::MultisampledTextureType t(param.dim, &i32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<i32>");
}

TEST_P(WgslGenerator_MultiampledTextureTest, EmitType_MultisampledTexture_U32) {
  auto param = GetParam();

  ast::type::U32Type u32;
  ast::type::MultisampledTextureType t(param.dim, &u32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), std::string(param.name) + "<u32>");
}
INSTANTIATE_TEST_SUITE_P(WgslGeneratorImplTest,
                         WgslGenerator_MultiampledTextureTest,
                         testing::Values(TextureData{
                             ast::type::TextureDimension::k2d,
                             "texture_multisampled_2d"}));

struct StorageTextureData {
  ast::type::ImageFormat fmt;
  ast::type::TextureDimension dim;
  ast::type::StorageAccess access;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, StorageTextureData data) {
  out << data.name;
  return out;
}
using WgslGenerator_StorageTextureTest =
    testing::TestWithParam<StorageTextureData>;
TEST_P(WgslGenerator_StorageTextureTest, EmitType_StorageTexture) {
  auto param = GetParam();

  ast::type::StorageTextureType t(param.dim, param.access, param.fmt);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&t)) << g.error();
  EXPECT_EQ(g.result(), param.name);
}
INSTANTIATE_TEST_SUITE_P(
    WgslGeneratorImplTest,
    WgslGenerator_StorageTextureTest,
    testing::Values(
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k1d,
            ast::type::StorageAccess::kRead, "texture_ro_1d<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k1dArray,
                           ast::type::StorageAccess::kRead,
                           "texture_ro_1d_array<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k2d,
            ast::type::StorageAccess::kRead, "texture_ro_2d<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k2dArray,
                           ast::type::StorageAccess::kRead,
                           "texture_ro_2d_array<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k3d,
            ast::type::StorageAccess::kRead, "texture_ro_3d<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k1d,
            ast::type::StorageAccess::kWrite, "texture_wo_1d<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k1dArray,
                           ast::type::StorageAccess::kWrite,
                           "texture_wo_1d_array<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k2d,
            ast::type::StorageAccess::kWrite, "texture_wo_2d<r8unorm>"},
        StorageTextureData{ast::type::ImageFormat::kR8Unorm,
                           ast::type::TextureDimension::k2dArray,
                           ast::type::StorageAccess::kWrite,
                           "texture_wo_2d_array<r8unorm>"},
        StorageTextureData{
            ast::type::ImageFormat::kR8Unorm, ast::type::TextureDimension::k3d,
            ast::type::StorageAccess::kWrite, "texture_wo_3d<r8unorm>"}));

struct ImageFormatData {
  ast::type::ImageFormat fmt;
  const char* name;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatData data) {
  out << data.name;
  return out;
}
using WgslGenerator_ImageFormatTest = testing::TestWithParam<ImageFormatData>;
TEST_P(WgslGenerator_ImageFormatTest, EmitType_StorageTexture_ImageFormat) {
  auto param = GetParam();

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitImageFormat(param.fmt)) << g.error();
  EXPECT_EQ(g.result(), param.name);
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
  ast::type::SamplerType sampler(ast::type::SamplerKind::kSampler);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&sampler)) << g.error();
  EXPECT_EQ(g.result(), "sampler");
}

TEST_F(WgslGeneratorImplTest, EmitType_SamplerComparison) {
  ast::type::SamplerType sampler(ast::type::SamplerKind::kComparisonSampler);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&sampler)) << g.error();
  EXPECT_EQ(g.result(), "sampler_comparison");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
