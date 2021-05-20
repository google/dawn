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
#include "src/sem/depth_texture_type.h"
#include "src/sem/external_texture_type.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/reference_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/storage_texture_type.h"

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
  auto* f32 = create<sem::F32>();
  auto result = table->Lookup(*this, IntrinsicType::kCos, {f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCos);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{f32}));
}

TEST_F(IntrinsicTableTest, MismatchF32) {
  auto* i32 = create<sem::I32>();
  auto result = table->Lookup(*this, IntrinsicType::kCos, {i32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchU32) {
  auto* f32 = create<sem::F32>();
  auto* u32 = create<sem::U32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto result =
      table->Lookup(*this, IntrinsicType::kUnpack2x16Float, {u32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kUnpack2x16Float);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec2_f32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{u32}));
}

TEST_F(IntrinsicTableTest, MismatchU32) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kUnpack2x16Float, {f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchI32) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k1d, f32);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, i32, i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{i32, Parameter::Usage::kCoords},
                          Parameter{i32, Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MismatchI32) {
  auto* f32 = create<sem::F32>();
  auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k1d, f32);
  auto result =
      table->Lookup(*this, IntrinsicType::kTextureLoad, {tex, f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchIU32AsI32) {
  auto* i32 = create<sem::I32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCountOneBits);
  EXPECT_THAT(result.intrinsic->ReturnType(), i32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{i32}));
}

TEST_F(IntrinsicTableTest, MatchIU32AsU32) {
  auto* u32 = create<sem::U32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {u32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCountOneBits);
  EXPECT_THAT(result.intrinsic->ReturnType(), u32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{u32}));
}

TEST_F(IntrinsicTableTest, MismatchIU32) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kCountOneBits, {f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsI32) {
  auto* i32 = create<sem::I32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp, {i32, i32, i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), i32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{i32}, Parameter{i32}, Parameter{i32}));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsU32) {
  auto* u32 = create<sem::U32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp, {u32, u32, u32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), u32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{u32}, Parameter{u32}, Parameter{u32}));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsF32) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp, {f32, f32, f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{f32}, Parameter{f32}, Parameter{f32}));
}

TEST_F(IntrinsicTableTest, MismatchFIU32) {
  auto* bool_ = create<sem::Bool>();
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {bool_, bool_, bool_}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchBool) {
  auto* f32 = create<sem::F32>();
  auto* bool_ = create<sem::Bool>();
  auto result =
      table->Lookup(*this, IntrinsicType::kSelect, {f32, f32, bool_}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kSelect);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{f32}, Parameter{f32}, Parameter{bool_}));
}

TEST_F(IntrinsicTableTest, MismatchBool) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kSelect, {f32, f32, f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchPointer) {
  auto* f32 = create<sem::F32>();
  auto* ptr = create<sem::Pointer>(f32, ast::StorageClass::kNone);
  auto result =
      table->Lookup(*this, IntrinsicType::kModf, {f32, ptr}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kModf);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{f32}, Parameter{ptr}));
}

TEST_F(IntrinsicTableTest, MismatchPointer) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kModf, {f32, f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchArray) {
  auto* arr = create<sem::Array>(create<sem::U32>(), 0, 4, 4, 4, true);
  auto result =
      table->Lookup(*this, IntrinsicType::kArrayLength, {arr}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kArrayLength);
  EXPECT_TRUE(result.intrinsic->ReturnType()->Is<sem::U32>());
  ASSERT_EQ(result.intrinsic->Parameters().size(), 1u);
  EXPECT_TRUE(result.intrinsic->Parameters()[0].type->Is<sem::Array>());
}

TEST_F(IntrinsicTableTest, MismatchArray) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kArrayLength, {f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampler) {
  auto* f32 = create<sem::F32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
  auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);
  auto result = table->Lookup(*this, IntrinsicType::kTextureSample,
                              {tex, sampler, vec2_f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureSample);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{sampler, Parameter::Usage::kSampler},
                          Parameter{vec2_f32, Parameter::Usage::kCoords}));
}

TEST_F(IntrinsicTableTest, MismatchSampler) {
  auto* f32 = create<sem::F32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
  auto result = table->Lookup(*this, IntrinsicType::kTextureSample,
                              {tex, f32, vec2_f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampledTexture) {
  auto* i32 = create<sem::I32>();
  auto* f32 = create<sem::F32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, vec2_i32, i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords},
                          Parameter{i32, Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MatchMultisampledTexture) {
  auto* i32 = create<sem::I32>();
  auto* f32 = create<sem::F32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* tex = create<sem::MultisampledTexture>(ast::TextureDimension::k2d, f32);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, vec2_i32, i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords},
                          Parameter{i32, Parameter::Usage::kSampleIndex}));
}

TEST_F(IntrinsicTableTest, MatchDepthTexture) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* tex = create<sem::DepthTexture>(ast::TextureDimension::k2d);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, vec2_i32, i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords},
                          Parameter{i32, Parameter::Usage::kLevel}));
}

TEST_F(IntrinsicTableTest, MatchExternalTexture) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* tex = create<sem::ExternalTexture>();
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, vec2_i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords}));
}

TEST_F(IntrinsicTableTest, MatchROStorageTexture) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* subtype =
      sem::StorageTexture::SubtypeFor(ast::ImageFormat::kR16Float, Types());
  auto* tex = create<sem::StorageTexture>(
      ast::TextureDimension::k2d, ast::ImageFormat::kR16Float,
      ast::AccessControl::kReadOnly, subtype);

  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {tex, vec2_i32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureLoad);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec4_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords}));
}

TEST_F(IntrinsicTableTest, MatchWOStorageTexture) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto* vec4_f32 = create<sem::Vector>(f32, 4);
  auto* subtype =
      sem::StorageTexture::SubtypeFor(ast::ImageFormat::kR16Float, Types());
  auto* tex = create<sem::StorageTexture>(
      ast::TextureDimension::k2d, ast::ImageFormat::kR16Float,
      ast::AccessControl::kWriteOnly, subtype);

  auto result = table->Lookup(*this, IntrinsicType::kTextureStore,
                              {tex, vec2_i32, vec4_f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kTextureStore);
  EXPECT_TRUE(result.intrinsic->ReturnType()->Is<sem::Void>());
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{tex, Parameter::Usage::kTexture},
                          Parameter{vec2_i32, Parameter::Usage::kCoords},
                          Parameter{vec4_f32, Parameter::Usage::kValue}));
}

TEST_F(IntrinsicTableTest, MismatchTexture) {
  auto* f32 = create<sem::F32>();
  auto* i32 = create<sem::I32>();
  auto* vec2_i32 = create<sem::Vector>(i32, 2);
  auto result = table->Lookup(*this, IntrinsicType::kTextureLoad,
                              {f32, vec2_i32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, ImplicitLoadOnReference) {
  auto* f32 = create<sem::F32>();
  auto result = table->Lookup(
      *this, IntrinsicType::kCos,
      {create<sem::Reference>(f32, ast::StorageClass::kNone)}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kCos);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{f32}));
}

TEST_F(IntrinsicTableTest, MatchOpenType) {
  auto* f32 = create<sem::F32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp, {f32, f32, f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{f32}, Parameter{f32}, Parameter{f32}));
}

TEST_F(IntrinsicTableTest, MismatchOpenType) {
  auto* f32 = create<sem::F32>();
  auto* u32 = create<sem::U32>();
  auto result =
      table->Lookup(*this, IntrinsicType::kClamp, {f32, u32, f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeVector) {
  auto* f32 = create<sem::F32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {vec2_f32, vec2_f32, vec2_f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kClamp);
  EXPECT_THAT(result.intrinsic->ReturnType(), vec2_f32);
  EXPECT_THAT(result.intrinsic->Parameters(),
              ElementsAre(Parameter{vec2_f32}, Parameter{vec2_f32},
                          Parameter{vec2_f32}));
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeVector) {
  auto* f32 = create<sem::F32>();
  auto* u32 = create<sem::U32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto result = table->Lookup(*this, IntrinsicType::kClamp,
                              {vec2_f32, u32, vec2_f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeMatrix) {
  auto* f32 = create<sem::F32>();
  auto* vec3_f32 = create<sem::Vector>(f32, 3);
  auto* mat3_f32 = create<sem::Matrix>(vec3_f32, 3);
  auto result =
      table->Lookup(*this, IntrinsicType::kDeterminant, {mat3_f32}, Source{});
  ASSERT_NE(result.intrinsic, nullptr);
  ASSERT_EQ(result.diagnostics.str(), "");
  EXPECT_THAT(result.intrinsic->Type(), IntrinsicType::kDeterminant);
  EXPECT_THAT(result.intrinsic->ReturnType(), f32);
  EXPECT_THAT(result.intrinsic->Parameters(), ElementsAre(Parameter{mat3_f32}));
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeMatrix) {
  auto* f32 = create<sem::F32>();
  auto* vec2_f32 = create<sem::Vector>(f32, 2);
  auto* mat3x2_f32 = create<sem::Matrix>(vec2_f32, 3);
  auto result =
      table->Lookup(*this, IntrinsicType::kDeterminant, {mat3x2_f32}, Source{});
  ASSERT_EQ(result.intrinsic, nullptr);
  ASSERT_THAT(result.diagnostics.str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, OverloadOrderByNumberOfParameters) {
  // None of the arguments match, so expect the overloads with 2 parameters to
  // come first
  auto* bool_ = create<sem::Bool>();
  auto result = table->Lookup(*this, IntrinsicType::kTextureDimensions,
                              {bool_, bool_}, Source{});
  ASSERT_EQ(result.diagnostics.str(),
            R"(error: no matching call to textureDimensions(bool, bool)

26 candidate functions:
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
  textureDimensions(texture : texture_storage_1d<F, A>) -> i32
  textureDimensions(texture : texture_storage_2d<F, A>) -> vec2<i32>
  textureDimensions(texture : texture_storage_2d_array<F, A>) -> vec2<i32>
  textureDimensions(texture : texture_storage_3d<F, A>) -> vec3<i32>
  textureDimensions(texture : texture_external) -> vec2<i32>
)");
}

TEST_F(IntrinsicTableTest, OverloadOrderByMatchingParameter) {
  auto* tex = create<sem::DepthTexture>(ast::TextureDimension::k2d);
  auto* bool_ = create<sem::Bool>();
  auto result = table->Lookup(*this, IntrinsicType::kTextureDimensions,
                              {tex, bool_}, Source{});
  ASSERT_EQ(
      result.diagnostics.str(),
      R"(error: no matching call to textureDimensions(texture_depth_2d, bool)

26 candidate functions:
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
  textureDimensions(texture : texture_storage_1d<F, A>) -> i32
  textureDimensions(texture : texture_storage_2d<F, A>) -> vec2<i32>
  textureDimensions(texture : texture_storage_2d_array<F, A>) -> vec2<i32>
  textureDimensions(texture : texture_storage_3d<F, A>) -> vec3<i32>
  textureDimensions(texture : texture_external) -> vec2<i32>
)");
}

}  // namespace
}  // namespace tint
