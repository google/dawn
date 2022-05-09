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

#include "src/tint/resolver/intrinsic_table.h"

#include "gmock/gmock.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"

namespace tint {
namespace {

using ::testing::HasSubstr;

using BuiltinType = sem::BuiltinType;
using Parameter = sem::Parameter;
using ParameterUsage = sem::ParameterUsage;

class IntrinsicTableTest : public testing::Test, public ProgramBuilder {
  public:
    std::unique_ptr<IntrinsicTable> table = IntrinsicTable::Create(*this);
};

TEST_F(IntrinsicTableTest, MatchF32) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kCos, {f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kCos);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchF32) {
    auto* i32 = create<sem::I32>();
    auto* result = table->Lookup(BuiltinType::kCos, {i32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchU32) {
    auto* f32 = create<sem::F32>();
    auto* u32 = create<sem::U32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* result = table->Lookup(BuiltinType::kUnpack2x16float, {u32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kUnpack2x16float);
    EXPECT_EQ(result->ReturnType(), vec2_f32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MismatchU32) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kUnpack2x16float, {f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchI32) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k1d, f32);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), vec4_f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MismatchI32) {
    auto* f32 = create<sem::F32>();
    auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k1d, f32);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchIU32AsI32) {
    auto* i32 = create<sem::I32>();
    auto* result = table->Lookup(BuiltinType::kCountOneBits, {i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kCountOneBits);
    EXPECT_EQ(result->ReturnType(), i32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), i32);
}

TEST_F(IntrinsicTableTest, MatchIU32AsU32) {
    auto* u32 = create<sem::U32>();
    auto* result = table->Lookup(BuiltinType::kCountOneBits, {u32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kCountOneBits);
    EXPECT_EQ(result->ReturnType(), u32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MismatchIU32) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kCountOneBits, {f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsI32) {
    auto* i32 = create<sem::I32>();
    auto* result = table->Lookup(BuiltinType::kClamp, {i32, i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kClamp);
    EXPECT_EQ(result->ReturnType(), i32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), i32);
    EXPECT_EQ(result->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
}

TEST_F(IntrinsicTableTest, MatchFIU32AsU32) {
    auto* u32 = create<sem::U32>();
    auto* result = table->Lookup(BuiltinType::kClamp, {u32, u32, u32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kClamp);
    EXPECT_EQ(result->ReturnType(), u32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), u32);
    EXPECT_EQ(result->Parameters()[1]->Type(), u32);
    EXPECT_EQ(result->Parameters()[2]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MatchFIU32AsF32) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kClamp, {f32, f32, f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kClamp);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result->Parameters()[2]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchFIU32) {
    auto* bool_ = create<sem::Bool>();
    auto* result = table->Lookup(BuiltinType::kClamp, {bool_, bool_, bool_}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchBool) {
    auto* f32 = create<sem::F32>();
    auto* bool_ = create<sem::Bool>();
    auto* result = table->Lookup(BuiltinType::kSelect, {f32, f32, bool_}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kSelect);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result->Parameters()[2]->Type(), bool_);
}

TEST_F(IntrinsicTableTest, MismatchBool) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kSelect, {f32, f32, f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchPointer) {
    auto* i32 = create<sem::I32>();
    auto* atomicI32 = create<sem::Atomic>(i32);
    auto* ptr =
        create<sem::Pointer>(atomicI32, ast::StorageClass::kWorkgroup, ast::Access::kReadWrite);
    auto* result = table->Lookup(BuiltinType::kAtomicLoad, {ptr}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kAtomicLoad);
    EXPECT_EQ(result->ReturnType(), i32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), ptr);
}

TEST_F(IntrinsicTableTest, MismatchPointer) {
    auto* i32 = create<sem::I32>();
    auto* atomicI32 = create<sem::Atomic>(i32);
    auto* result = table->Lookup(BuiltinType::kAtomicLoad, {atomicI32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchArray) {
    auto* arr = create<sem::Array>(create<sem::U32>(), 0u, 4u, 4u, 4u, 4u);
    auto* arr_ptr = create<sem::Pointer>(arr, ast::StorageClass::kStorage, ast::Access::kReadWrite);
    auto* result = table->Lookup(BuiltinType::kArrayLength, {arr_ptr}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kArrayLength);
    EXPECT_TRUE(result->ReturnType()->Is<sem::U32>());
    ASSERT_EQ(result->Parameters().size(), 1u);
    auto* param_type = result->Parameters()[0]->Type();
    ASSERT_TRUE(param_type->Is<sem::Pointer>());
    EXPECT_TRUE(param_type->As<sem::Pointer>()->StoreType()->Is<sem::Array>());
}

TEST_F(IntrinsicTableTest, MismatchArray) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kArrayLength, {f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampler) {
    auto* f32 = create<sem::F32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
    auto* sampler = create<sem::Sampler>(ast::SamplerKind::kSampler);
    auto* result = table->Lookup(BuiltinType::kTextureSample, {tex, sampler, vec2_f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureSample);
    EXPECT_EQ(result->ReturnType(), vec4_f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), sampler);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kSampler);
    EXPECT_EQ(result->Parameters()[2]->Type(), vec2_f32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kCoords);
}

TEST_F(IntrinsicTableTest, MismatchSampler) {
    auto* f32 = create<sem::F32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
    auto* result = table->Lookup(BuiltinType::kTextureSample, {tex, f32, vec2_f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampledTexture) {
    auto* i32 = create<sem::I32>();
    auto* f32 = create<sem::F32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* tex = create<sem::SampledTexture>(ast::TextureDimension::k2d, f32);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, vec2_i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), vec4_f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MatchMultisampledTexture) {
    auto* i32 = create<sem::I32>();
    auto* f32 = create<sem::F32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* tex = create<sem::MultisampledTexture>(ast::TextureDimension::k2d, f32);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, vec2_i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), vec4_f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kSampleIndex);
}

TEST_F(IntrinsicTableTest, MatchDepthTexture) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* tex = create<sem::DepthTexture>(ast::TextureDimension::k2d);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, vec2_i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MatchDepthMultisampledTexture) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* tex = create<sem::DepthMultisampledTexture>(ast::TextureDimension::k2d);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, vec2_i32, i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kSampleIndex);
}

TEST_F(IntrinsicTableTest, MatchExternalTexture) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* tex = create<sem::ExternalTexture>();
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {tex, vec2_i32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureLoad);
    EXPECT_EQ(result->ReturnType(), vec4_f32);
    ASSERT_EQ(result->Parameters().size(), 2u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
}

TEST_F(IntrinsicTableTest, MatchWOStorageTexture) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* vec4_f32 = create<sem::Vector>(f32, 4u);
    auto* subtype = sem::StorageTexture::SubtypeFor(ast::TexelFormat::kR32Float, Types());
    auto* tex = create<sem::StorageTexture>(ast::TextureDimension::k2d, ast::TexelFormat::kR32Float,
                                            ast::Access::kWrite, subtype);

    auto* result = table->Lookup(BuiltinType::kTextureStore, {tex, vec2_i32, vec4_f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kTextureStore);
    EXPECT_TRUE(result->ReturnType()->Is<sem::Void>());
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result->Parameters()[2]->Type(), vec4_f32);
    EXPECT_EQ(result->Parameters()[2]->Usage(), ParameterUsage::kValue);
}

TEST_F(IntrinsicTableTest, MismatchTexture) {
    auto* f32 = create<sem::F32>();
    auto* i32 = create<sem::I32>();
    auto* vec2_i32 = create<sem::Vector>(i32, 2u);
    auto* result = table->Lookup(BuiltinType::kTextureLoad, {f32, vec2_i32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, ImplicitLoadOnReference) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(
        BuiltinType::kCos,
        {create<sem::Reference>(f32, ast::StorageClass::kFunction, ast::Access::kReadWrite)},
        Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kCos);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MatchOpenType) {
    auto* f32 = create<sem::F32>();
    auto* result = table->Lookup(BuiltinType::kClamp, {f32, f32, f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kClamp);
    EXPECT_EQ(result->ReturnType(), f32);
    EXPECT_EQ(result->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result->Parameters()[2]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchOpenType) {
    auto* f32 = create<sem::F32>();
    auto* u32 = create<sem::U32>();
    auto* result = table->Lookup(BuiltinType::kClamp, {f32, u32, f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeVector) {
    auto* f32 = create<sem::F32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* result = table->Lookup(BuiltinType::kClamp, {vec2_f32, vec2_f32, vec2_f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kClamp);
    EXPECT_EQ(result->ReturnType(), vec2_f32);
    ASSERT_EQ(result->Parameters().size(), 3u);
    EXPECT_EQ(result->Parameters()[0]->Type(), vec2_f32);
    EXPECT_EQ(result->Parameters()[1]->Type(), vec2_f32);
    EXPECT_EQ(result->Parameters()[2]->Type(), vec2_f32);
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeVector) {
    auto* f32 = create<sem::F32>();
    auto* u32 = create<sem::U32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* result = table->Lookup(BuiltinType::kClamp, {vec2_f32, u32, vec2_f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeMatrix) {
    auto* f32 = create<sem::F32>();
    auto* vec3_f32 = create<sem::Vector>(f32, 3u);
    auto* mat3_f32 = create<sem::Matrix>(vec3_f32, 3u);
    auto* result = table->Lookup(BuiltinType::kDeterminant, {mat3_f32}, Source{});
    ASSERT_NE(result, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result->Type(), BuiltinType::kDeterminant);
    EXPECT_EQ(result->ReturnType(), f32);
    ASSERT_EQ(result->Parameters().size(), 1u);
    EXPECT_EQ(result->Parameters()[0]->Type(), mat3_f32);
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeMatrix) {
    auto* f32 = create<sem::F32>();
    auto* vec2_f32 = create<sem::Vector>(f32, 2u);
    auto* mat3x2_f32 = create<sem::Matrix>(vec2_f32, 3u);
    auto* result = table->Lookup(BuiltinType::kDeterminant, {mat3x2_f32}, Source{});
    ASSERT_EQ(result, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, OverloadOrderByNumberOfParameters) {
    // None of the arguments match, so expect the overloads with 2 parameters to
    // come first
    auto* bool_ = create<sem::Bool>();
    table->Lookup(BuiltinType::kTextureDimensions, {bool_, bool_}, Source{});
    ASSERT_EQ(Diagnostics().str(),
              R"(error: no matching call to textureDimensions(bool, bool)

27 candidate functions:
  textureDimensions(texture: texture_1d<T>, level: i32) -> i32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>, level: i32) -> vec3<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_2d_array, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube_array, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_1d<T>) -> i32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>) -> vec3<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_multisampled_2d<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d) -> vec2<i32>
  textureDimensions(texture: texture_depth_2d_array) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube_array) -> vec2<i32>
  textureDimensions(texture: texture_depth_multisampled_2d) -> vec2<i32>
  textureDimensions(texture: texture_storage_1d<F, A>) -> i32  where: A is write
  textureDimensions(texture: texture_storage_2d<F, A>) -> vec2<i32>  where: A is write
  textureDimensions(texture: texture_storage_2d_array<F, A>) -> vec2<i32>  where: A is write
  textureDimensions(texture: texture_storage_3d<F, A>) -> vec3<i32>  where: A is write
  textureDimensions(texture: texture_external) -> vec2<i32>
)");
}

TEST_F(IntrinsicTableTest, OverloadOrderByMatchingParameter) {
    auto* tex = create<sem::DepthTexture>(ast::TextureDimension::k2d);
    auto* bool_ = create<sem::Bool>();
    table->Lookup(BuiltinType::kTextureDimensions, {tex, bool_}, Source{});
    ASSERT_EQ(Diagnostics().str(),
              R"(error: no matching call to textureDimensions(texture_depth_2d, bool)

27 candidate functions:
  textureDimensions(texture: texture_depth_2d, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_2d) -> vec2<i32>
  textureDimensions(texture: texture_1d<T>, level: i32) -> i32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>, level: i32) -> vec3<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>, level: i32) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d_array, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube_array, level: i32) -> vec2<i32>
  textureDimensions(texture: texture_1d<T>) -> i32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>) -> vec3<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_multisampled_2d<T>) -> vec2<i32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d_array) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube) -> vec2<i32>
  textureDimensions(texture: texture_depth_cube_array) -> vec2<i32>
  textureDimensions(texture: texture_depth_multisampled_2d) -> vec2<i32>
  textureDimensions(texture: texture_storage_1d<F, A>) -> i32  where: A is write
  textureDimensions(texture: texture_storage_2d<F, A>) -> vec2<i32>  where: A is write
  textureDimensions(texture: texture_storage_2d_array<F, A>) -> vec2<i32>  where: A is write
  textureDimensions(texture: texture_storage_3d<F, A>) -> vec3<i32>  where: A is write
  textureDimensions(texture: texture_external) -> vec2<i32>
)");
}

TEST_F(IntrinsicTableTest, SameOverloadReturnsSameBuiltinPointer) {
    auto* f32 = create<sem::F32>();
    auto* vec2_f32 = create<sem::Vector>(create<sem::F32>(), 2u);
    auto* bool_ = create<sem::Bool>();
    auto* a = table->Lookup(BuiltinType::kSelect, {f32, f32, bool_}, Source{});
    ASSERT_NE(a, nullptr) << Diagnostics().str();

    auto* b = table->Lookup(BuiltinType::kSelect, {f32, f32, bool_}, Source{});
    ASSERT_NE(b, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");

    auto* c = table->Lookup(BuiltinType::kSelect, {vec2_f32, vec2_f32, bool_}, Source{});
    ASSERT_NE(c, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");

    EXPECT_EQ(a, b);
    EXPECT_NE(a, c);
    EXPECT_NE(b, c);
}

TEST_F(IntrinsicTableTest, MatchBinaryOp) {
    auto* i32 = create<sem::I32>();
    auto* vec3_i32 = create<sem::Vector>(i32, 3u);
    auto result = table->Lookup(ast::BinaryOp::kMultiply, i32, vec3_i32, Source{{12, 34}},
                                /* is_compound */ false);
    EXPECT_EQ(result.result, vec3_i32);
    EXPECT_EQ(result.lhs, i32);
    EXPECT_EQ(result.rhs, vec3_i32);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MismatchBinaryOp) {
    auto* f32 = create<sem::F32>();
    auto* bool_ = create<sem::Bool>();
    auto result = table->Lookup(ast::BinaryOp::kMultiply, f32, bool_, Source{{12, 34}},
                                /* is_compound */ false);
    ASSERT_EQ(result.result, nullptr);
    EXPECT_EQ(Diagnostics().str(), R"(12:34 error: no matching overload for operator * (f32, bool)

9 candidate operators:
  operator * (T, T) -> T  where: T is f32, i32 or u32
  operator * (vecN<T>, T) -> vecN<T>  where: T is f32, i32 or u32
  operator * (T, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32
  operator * (f32, matNxM<f32>) -> matNxM<f32>
  operator * (vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32
  operator * (matNxM<f32>, f32) -> matNxM<f32>
  operator * (matCxR<f32>, vecC<f32>) -> vecR<f32>
  operator * (vecR<f32>, matCxR<f32>) -> vecC<f32>
  operator * (matKxR<f32>, matCxK<f32>) -> matCxR<f32>
)");
}

TEST_F(IntrinsicTableTest, MatchCompoundOp) {
    auto* i32 = create<sem::I32>();
    auto* vec3_i32 = create<sem::Vector>(i32, 3u);
    auto result = table->Lookup(ast::BinaryOp::kMultiply, i32, vec3_i32, Source{{12, 34}},
                                /* is_compound */ true);
    EXPECT_EQ(result.result, vec3_i32);
    EXPECT_EQ(result.lhs, i32);
    EXPECT_EQ(result.rhs, vec3_i32);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MismatchCompoundOp) {
    auto* f32 = create<sem::F32>();
    auto* bool_ = create<sem::Bool>();
    auto result = table->Lookup(ast::BinaryOp::kMultiply, f32, bool_, Source{{12, 34}},
                                /* is_compound */ true);
    ASSERT_EQ(result.result, nullptr);
    EXPECT_EQ(Diagnostics().str(), R"(12:34 error: no matching overload for operator *= (f32, bool)

9 candidate operators:
  operator *= (T, T) -> T  where: T is f32, i32 or u32
  operator *= (vecN<T>, T) -> vecN<T>  where: T is f32, i32 or u32
  operator *= (T, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32
  operator *= (f32, matNxM<f32>) -> matNxM<f32>
  operator *= (vecN<T>, vecN<T>) -> vecN<T>  where: T is f32, i32 or u32
  operator *= (matNxM<f32>, f32) -> matNxM<f32>
  operator *= (matCxR<f32>, vecC<f32>) -> vecR<f32>
  operator *= (vecR<f32>, matCxR<f32>) -> vecC<f32>
  operator *= (matKxR<f32>, matCxK<f32>) -> matCxR<f32>
)");
}

}  // namespace
}  // namespace tint
