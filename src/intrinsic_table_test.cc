// Copyright 2021 The Tint Authors.
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

#include "src/intrinsic_table.h"

#include "gmock/gmock.h"
#include "src/program_builder.h"
#include "src/type/access_control_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"

namespace tint {
namespace {

using ::testing::ElementsAre;
using ::testing::HasSubstr;

using IntrinsicType = sem::IntrinsicType;
using Parameter = sem::Parameter;

class IntrinsicTableTest : public testing::Test, public ProgramBuilder {
 public:
  std::unique_ptr<IntrinsicTable> table = IntrinsicTable::Create();
};

TEST_F(IntrinsicTableTest, MatchF32) {
  auto result = table->Lookup(*this, IntrinsicType::kCos, {ty.f32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCos);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.f32()}));
}

TEST_F(IntrinsicTableTest, MismatchF32) {
  auto result = table->Lookup(*this, IntrinsicType::kCos, {ty.i32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchU32) {
  auto result = table->Lookup(*this, IntrinsicType::kUnpack2x16Float,
                              {ty.u32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kUnpack2x16Float);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec2<f32>());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.u32()}));
}

TEST_F(IntrinsicTableTest, MismatchU32) {
  auto result = table->Lookup(*this, IntrinsicType::kUnpack2x16Float,
                              {ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchI32) {
  auto* tex =
      create<type::SampledTexture>(type::TextureDimension::k1d, ty.f32());
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, ty.i32(), ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec4<f32>());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{ty.i32(), Parameter::Usage::kCoords},
                          Parameter{ty.i32(), Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MismatchI32) {
  auto* tex =
      create<type::SampledTexture>(type::TextureDimension::k1d, ty.f32());
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchIU32AsI32) {
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCountOneBits);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.i32());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.i32()}));
}

TEST_F(IntrinsicTableTest, MatchIU32AsU32) {
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {ty.u32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCountOneBits);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.u32());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.u32()}));
}

TEST_F(IntrinsicTableTest, MismatchIU32) {
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsI32) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.i32(), ty.i32(), ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.i32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.i32()}, Parameter{ty.i32()},
                          Parameter{ty.i32()}));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsU32) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.u32(), ty.u32(), ty.u32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.u32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.u32()}, Parameter{ty.u32()},
                          Parameter{ty.u32()}));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsF32) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.f32(), ty.f32(), ty.f32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.f32()}, Parameter{ty.f32()},
                          Parameter{ty.f32()}));
}

TEST_F(IntrinsicTableTest, MismatchFIU32) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.bool_(), ty.bool_(), ty.bool_()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchBool) {
  auto result = table->Lookup(*this, IntrinsicType::kSelect,
                              {ty.f32(), ty.f32(), ty.bool_()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kSelect);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.f32()}, Parameter{ty.f32()},
                          Parameter{ty.bool_()}));
}

TEST_F(IntrinsicTableTest, MismatchBool) {
  auto result = table->Lookup(*this, IntrinsicType::kSelect,
                              {ty.f32(), ty.f32(), ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchPointer) {
  auto result = table->Lookup(
      *this, IntrinsicType::kModf,
      {ty.f32(), ty.pointer<f32>(ast::StorageClass::kNone)}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kModf);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(
      result.intrinsic->Parameters(),
      ElementsAre(Parameter{ty.f32()},
                  Parameter{ty.pointer<f32>(ast::StorageClass::kNone)}));
}

TEST_F(IntrinsicTableTest, MismatchPointer) {
  auto result = table->Lookup(*this, IntrinsicType::kModf, {ty.f32(), ty.f32()},
                              Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchArray) {
  auto result = table->Lookup(*this, IntrinsicType::kArrayLength,
                              {ty.array<f32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kArrayLength);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.u32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.array<f32>()}));
}

TEST_F(IntrinsicTableTest, MismatchArray) {
  auto result =
      table->Lookup(*this, IntrinsicType::kArrayLength, {ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampler) {
  auto* tex =
      create<type::SampledTexture>(type::TextureDimension::k2d, ty.f32());
  auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);
  auto result = table->Lookup(*this, IntrinsicType::kTextureSample,
                              {tex, sampler, ty.vec2<f32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureSample);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec4<f32>());
  EXPECT_THAT(
      result.intrinsic->Parameters(),
      ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                  Parameter{sampler, Parameter::Usage::kSampler},
                  Parameter{ty.vec2<f32>(), Parameter::Usage::kCoords}));
}

TEST_F(IntrinsicTableTest, MismatchSampler) {
  auto* tex =
      create<type::SampledTexture>(type::TextureDimension::k2d, ty.f32());
  auto result = table->Lookup(*this, IntrinsicType::kTextureSample,
                              {tex, ty.f32(), ty.vec2<f32>()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampledTexture) {
  auto* tex =
      create<type::SampledTexture>(type::TextureDimension::k2d, ty.f32());
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, ty.vec2<i32>(), ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec4<f32>());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{ty.vec2<i32>(), Parameter::Usage::kCoords},
                          Parameter{ty.i32(), Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MatchMultisampledTexture) {
  auto* tex =
      create<type::MultisampledTexture>(type::TextureDimension::k2d, ty.f32());
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, ty.vec2<i32>(), ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec4<f32>());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{ty.vec2<i32>(), Parameter::Usage::kCoords},
                          Parameter{ty.i32(), Parameter::Usage::kSampleIndex}));
}

TEST_F(IntrinsicTableTest, MatchDepthTexture) {
  auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, ty.vec2<i32>(), ty.i32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{ty.vec2<i32>(), Parameter::Usage::kCoords},
                          Parameter{ty.i32(), Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MatchROStorageTexture) {
  auto* tex = create<type::StorageTexture>(
      type::TextureDimension::k2d, type::ImageFormat::kR16Float,
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types()));
  auto* tex_ac =
      create<type::AccessControl>(ast::AccessControl::kReadOnly, tex);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex_ac, ty.vec2<i32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec4<f32>());
  EXPECT_THAT(
      result.intrinsic->Parameters(),
      ElementsAre(Parameter{tex_ac, Parameter::Usage::kTexture},
                  Parameter{ty.vec2<i32>(), Parameter::Usage::kCoords}));
}

TEST_F(IntrinsicTableTest, MatchWOStorageTexture) {
  auto* tex = create<type::StorageTexture>(
      type::TextureDimension::k2d, type::ImageFormat::kR16Float,
      type::StorageTexture::SubtypeFor(type::ImageFormat::kR16Float, Types()));
  auto* tex_ac =
      create<type::AccessControl>(ast::AccessControl::kWriteOnly, tex);
  auto result =
      table->Lookup(*this, IntrinsicType::kTextureStore,
                    {tex_ac, ty.vec2<i32>(), ty.vec4<f32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureStore);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.void_());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex_ac, Parameter::Usage::kTexture},
                          Parameter{ty.vec2<i32>(), Parameter::Usage::kCoords},
                          Parameter{ty.vec4<f32>(), Parameter::Usage::kValue}));
}

TEST_F(IntrinsicTableTest, MismatchTexture) {
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {ty.f32(), ty.vec2<i32>()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchAutoPointerDereference) {
  auto result =
      table->Lookup(*this, IntrinsicType::kCos,
                    {ty.pointer<f32>(ast::StorageClass::kNone)}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCos);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.f32()}));
}

TEST_F(IntrinsicTableTest, MatchWithAliasUnwrapping) {
  auto* alias_a = ty.alias("alias_a", ty.f32());
  auto* alias_b = ty.alias("alias_b", alias_a);
  auto* alias_c = ty.alias("alias_c", alias_b);
  auto result = table->Lookup(*this, IntrinsicType::kCos, {alias_c}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCos);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{ty.f32()}));
}

TEST_F(IntrinsicTableTest, MatchWithNestedAliasUnwrapping) {
  auto* alias_a = ty.alias("alias_a", ty.bool_());
  auto* alias_b = ty.alias("alias_b", alias_a);
  auto* alias_c = ty.alias("alias_c", alias_b);
  auto* vec4_of_c = ty.vec4(alias_c);
  auto* alias_d = ty.alias("alias_d", vec4_of_c);
  auto* alias_e = ty.alias("alias_e", alias_d);

  auto result = table->Lookup(*this, IntrinsicType::kAll, {alias_e}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kAll);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.bool_());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.vec4<bool>()}));
}

TEST_F(IntrinsicTableTest, MatchOpenType) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.f32(), ty.f32(), ty.f32()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.f32()}, Parameter{ty.f32()},
                          Parameter{ty.f32()}));
}

TEST_F(IntrinsicTableTest, MismatchOpenType) {
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {ty.f32(), ty.u32(), ty.f32()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeVector) {
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp,
                    {ty.vec2<f32>(), ty.vec2<f32>(), ty.vec2<f32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.vec2<f32>());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.vec2<f32>()}, Parameter{ty.vec2<f32>()},
                          Parameter{ty.vec2<f32>()}));
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeVector) {
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp,
                    {ty.vec2<f32>(), ty.vec2<u32>(), ty.vec2<f32>()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeMatrix) {
  auto result = table->Lookup(*this, IntrinsicType::kDeterminant,
                              {ty.mat3x3<f32>()}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kDeterminant);
  EXPECT_THAT(result.intrinsic->ReturnType(), ty.f32());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{ty.mat3x3<f32>()}));
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeMatrix) {
  auto result = table->Lookup(*this, IntrinsicType::kDeterminant,
                              {ty.mat3x2<f32>()}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, OverloadOrderByNumberOfParameters) {
  // None of the arguments match, so expect the overloads with 2 parameters to
  // come first
  auto result = table->Lookup(*this, IntrinsicType::kTextureDimensions,
                              {ty.bool_(), ty.bool_()}, Source{});
  ASSERT_EQ(result.diagnostics.str(),
            R"(error: no matching call to textureDimensions(bool, bool)

25 candidate functions:
  textureDimensions(texture : texture_2d<T>, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_2d_array<T>, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_3d<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_cube<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_cube_array<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_depth_2d, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_depth_2d_array, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_depth_cube, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_depth_cube_array, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_1d<T>) -> i32
  textureDimensions(texture : texture_2d<T>) -> vec2<i32>
  textureDimensions(texture : texture_2d_array<T>) -> vec2<i32>
  textureDimensions(texture : texture_3d<T>) -> vec3<i32>
  textureDimensions(texture : texture_cube<T>) -> vec3<i32>
  textureDimensions(texture : texture_cube_array<T>) -> vec3<i32>
  textureDimensions(texture : texture_multisampled_2d<T>) -> vec2<i32>
  textureDimensions(texture : texture_multisampled_2d_array<T>) -> vec2<i32>
  textureDimensions(texture : texture_depth_2d) -> vec2<i32>
  textureDimensions(texture : texture_depth_2d_array) -> vec2<i32>
  textureDimensions(texture : texture_depth_cube) -> vec3<i32>
  textureDimensions(texture : texture_depth_cube_array) -> vec3<i32>
  textureDimensions(texture : texture_storage_1d<F>) -> i32
  textureDimensions(texture : texture_storage_2d<F>) -> vec2<i32>
  textureDimensions(texture : texture_storage_2d_array<F>) -> vec2<i32>
  textureDimensions(texture : texture_storage_3d<F>) -> vec3<i32>
)");
}

TEST_F(IntrinsicTableTest, OverloadOrderByMatchingParameter) {
  auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
  auto result = table->Lookup(*this, IntrinsicType::kTextureDimensions,
                              {tex, ty.bool_()}, Source{});
  ASSERT_EQ(
      result.diagnostics.str(),
      R"(error: no matching call to textureDimensions(texture_depth_2d, bool)

25 candidate functions:
  textureDimensions(texture : texture_depth_2d, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_depth_2d) -> vec2<i32>
  textureDimensions(texture : texture_2d<T>, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_2d_array<T>, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_3d<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_cube<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_cube_array<T>, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_depth_2d_array, level : i32) -> vec2<i32>
  textureDimensions(texture : texture_depth_cube, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_depth_cube_array, level : i32) -> vec3<i32>
  textureDimensions(texture : texture_1d<T>) -> i32
  textureDimensions(texture : texture_2d<T>) -> vec2<i32>
  textureDimensions(texture : texture_2d_array<T>) -> vec2<i32>
  textureDimensions(texture : texture_3d<T>) -> vec3<i32>
  textureDimensions(texture : texture_cube<T>) -> vec3<i32>
  textureDimensions(texture : texture_cube_array<T>) -> vec3<i32>
  textureDimensions(texture : texture_multisampled_2d<T>) -> vec2<i32>
  textureDimensions(texture : texture_multisampled_2d_array<T>) -> vec2<i32>
  textureDimensions(texture : texture_depth_2d_array) -> vec2<i32>
  textureDimensions(texture : texture_depth_cube) -> vec3<i32>
  textureDimensions(texture : texture_depth_cube_array) -> vec3<i32>
  textureDimensions(texture : texture_storage_1d<F>) -> i32
  textureDimensions(texture : texture_storage_2d<F>) -> vec2<i32>
  textureDimensions(texture : texture_storage_2d_array<F>) -> vec2<i32>
  textureDimensions(texture : texture_storage_3d<F>) -> vec3<i32>
)");
}

}  // namespace
}  // namespace tint
