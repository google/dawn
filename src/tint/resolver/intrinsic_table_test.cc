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

#include <utility>

#include "gmock/gmock.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/resolver_test_helper.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/reference.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/test_helper.h"
#include "src/tint/type/texture_dimension.h"

namespace tint::resolver {
namespace {

using ::testing::HasSubstr;

using Parameter = sem::Parameter;
using ParameterUsage = sem::ParameterUsage;

using AFloatV = builder::vec<3, AFloat>;
using AIntV = builder::vec<3, AInt>;
using f32V = builder::vec<3, f32>;
using i32V = builder::vec<3, i32>;
using u32V = builder::vec<3, u32>;

class IntrinsicTableTest : public testing::Test, public ProgramBuilder {
  public:
    std::unique_ptr<IntrinsicTable> table = IntrinsicTable::Create(*this);
};

TEST_F(IntrinsicTableTest, MatchF32) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kCos, utils::Vector{f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kCos);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchF32) {
    auto* i32 = create<type::I32>();
    auto result = table->Lookup(builtin::Function::kCos, utils::Vector{i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchU32) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto result = table->Lookup(builtin::Function::kUnpack2X16Float, utils::Vector{u32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kUnpack2X16Float);
    EXPECT_EQ(result.sem->ReturnType(), vec2_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MismatchU32) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kUnpack2X16Float, utils::Vector{f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchI32) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k1d, f32);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), vec4_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MismatchI32) {
    auto* f32 = create<type::F32>();
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k1d, f32);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchIU32AsI32) {
    auto* i32 = create<type::I32>();
    auto result = table->Lookup(builtin::Function::kCountOneBits, utils::Vector{i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kCountOneBits);
    EXPECT_EQ(result.sem->ReturnType(), i32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), i32);
}

TEST_F(IntrinsicTableTest, MatchIU32AsU32) {
    auto* u32 = create<type::U32>();
    auto result = table->Lookup(builtin::Function::kCountOneBits, utils::Vector{u32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kCountOneBits);
    EXPECT_EQ(result.sem->ReturnType(), u32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MismatchIU32) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kCountOneBits, utils::Vector{f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchFIU32AsI32) {
    auto* i32 = create<type::I32>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{i32, i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kClamp);
    EXPECT_EQ(result.sem->ReturnType(), i32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
}

TEST_F(IntrinsicTableTest, MatchFIU32AsU32) {
    auto* u32 = create<type::U32>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{u32, u32, u32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kClamp);
    EXPECT_EQ(result.sem->ReturnType(), u32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), u32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), u32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), u32);
}

TEST_F(IntrinsicTableTest, MatchFIU32AsF32) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{f32, f32, f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kClamp);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchFIU32) {
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{bool_, bool_, bool_},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchBool) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(builtin::Function::kSelect, utils::Vector{f32, f32, bool_},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kSelect);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), bool_);
}

TEST_F(IntrinsicTableTest, MismatchBool) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kSelect, utils::Vector{f32, f32, f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchPointer) {
    auto* i32 = create<type::I32>();
    auto* atomicI32 = create<type::Atomic>(i32);
    auto* ptr = create<type::Pointer>(atomicI32, builtin::AddressSpace::kWorkgroup,
                                      builtin::Access::kReadWrite);
    auto result = table->Lookup(builtin::Function::kAtomicLoad, utils::Vector{ptr},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kAtomicLoad);
    EXPECT_EQ(result.sem->ReturnType(), i32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), ptr);
}

TEST_F(IntrinsicTableTest, MismatchPointer) {
    auto* i32 = create<type::I32>();
    auto* atomicI32 = create<type::Atomic>(i32);
    auto result = table->Lookup(builtin::Function::kAtomicLoad, utils::Vector{atomicI32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchArray) {
    auto* arr =
        create<type::Array>(create<type::U32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* arr_ptr =
        create<type::Pointer>(arr, builtin::AddressSpace::kStorage, builtin::Access::kReadWrite);
    auto result = table->Lookup(builtin::Function::kArrayLength, utils::Vector{arr_ptr},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kArrayLength);
    EXPECT_TRUE(result.sem->ReturnType()->Is<type::U32>());
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    auto* param_type = result.sem->Parameters()[0]->Type();
    ASSERT_TRUE(param_type->Is<type::Pointer>());
    EXPECT_TRUE(param_type->As<type::Pointer>()->StoreType()->Is<type::Array>());
}

TEST_F(IntrinsicTableTest, MismatchArray) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kArrayLength, utils::Vector{f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampler) {
    auto* f32 = create<type::F32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto* sampler = create<type::Sampler>(type::SamplerKind::kSampler);
    auto result =
        table->Lookup(builtin::Function::kTextureSample, utils::Vector{tex, sampler, vec2_f32},
                      sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureSample);
    EXPECT_EQ(result.sem->ReturnType(), vec4_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), sampler);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kSampler);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), vec2_f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kCoords);
}

TEST_F(IntrinsicTableTest, MismatchSampler) {
    auto* f32 = create<type::F32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto result =
        table->Lookup(builtin::Function::kTextureSample, utils::Vector{tex, f32, vec2_f32},
                      sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchSampledTexture) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* tex = create<type::SampledTexture>(type::TextureDimension::k2d, f32);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, vec2_i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), vec4_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MatchMultisampledTexture) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* tex = create<type::MultisampledTexture>(type::TextureDimension::k2d, f32);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, vec2_i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), vec4_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kSampleIndex);
}

TEST_F(IntrinsicTableTest, MatchDepthTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, vec2_i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kLevel);
}

TEST_F(IntrinsicTableTest, MatchDepthMultisampledTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* tex = create<type::DepthMultisampledTexture>(type::TextureDimension::k2d);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, vec2_i32, i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), i32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kSampleIndex);
}

TEST_F(IntrinsicTableTest, MatchExternalTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* tex = create<type::ExternalTexture>();
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{tex, vec2_i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureLoad);
    EXPECT_EQ(result.sem->ReturnType(), vec4_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 2u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
}

TEST_F(IntrinsicTableTest, MatchWOStorageTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto* vec4_f32 = create<type::Vector>(f32, 4u);
    auto* subtype = type::StorageTexture::SubtypeFor(builtin::TexelFormat::kR32Float, Types());
    auto* tex =
        create<type::StorageTexture>(type::TextureDimension::k2d, builtin::TexelFormat::kR32Float,
                                     builtin::Access::kWrite, subtype);

    auto result =
        table->Lookup(builtin::Function::kTextureStore, utils::Vector{tex, vec2_i32, vec4_f32},
                      sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kTextureStore);
    EXPECT_TRUE(result.sem->ReturnType()->Is<type::Void>());
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), tex);
    EXPECT_EQ(result.sem->Parameters()[0]->Usage(), ParameterUsage::kTexture);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_i32);
    EXPECT_EQ(result.sem->Parameters()[1]->Usage(), ParameterUsage::kCoords);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), vec4_f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Usage(), ParameterUsage::kValue);
}

TEST_F(IntrinsicTableTest, MismatchTexture) {
    auto* f32 = create<type::F32>();
    auto* i32 = create<type::I32>();
    auto* vec2_i32 = create<type::Vector>(i32, 2u);
    auto result = table->Lookup(builtin::Function::kTextureLoad, utils::Vector{f32, vec2_i32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, ImplicitLoadOnReference) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kCos,
                                utils::Vector{
                                    create<type::Reference>(f32, builtin::AddressSpace::kFunction,
                                                            builtin::Access::kReadWrite),
                                },
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kCos);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MatchTemplateType) {
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{f32, f32, f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kClamp);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), f32);
}

TEST_F(IntrinsicTableTest, MismatchTemplateType) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{f32, u32, f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeVector) {
    auto* f32 = create<type::F32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto result =
        table->Lookup(builtin::Function::kClamp, utils::Vector{vec2_f32, vec2_f32, vec2_f32},
                      sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kClamp);
    EXPECT_EQ(result.sem->ReturnType(), vec2_f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), vec2_f32);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), vec2_f32);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), vec2_f32);
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeVector) {
    auto* f32 = create<type::F32>();
    auto* u32 = create<type::U32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto result = table->Lookup(builtin::Function::kClamp, utils::Vector{vec2_f32, u32, vec2_f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchOpenSizeMatrix) {
    auto* f32 = create<type::F32>();
    auto* vec3_f32 = create<type::Vector>(f32, 3u);
    auto* mat3_f32 = create<type::Matrix>(vec3_f32, 3u);
    auto result = table->Lookup(builtin::Function::kDeterminant, utils::Vector{mat3_f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Type(), builtin::Function::kDeterminant);
    EXPECT_EQ(result.sem->ReturnType(), f32);
    ASSERT_EQ(result.sem->Parameters().Length(), 1u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), mat3_f32);
}

TEST_F(IntrinsicTableTest, MismatchOpenSizeMatrix) {
    auto* f32 = create<type::F32>();
    auto* vec2_f32 = create<type::Vector>(f32, 2u);
    auto* mat3x2_f32 = create<type::Matrix>(vec2_f32, 3u);
    auto result = table->Lookup(builtin::Function::kDeterminant, utils::Vector{mat3x2_f32},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, MatchDifferentArgsElementType_Builtin_ConstantEval) {
    auto* af = create<type::AbstractFloat>();
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(builtin::Function::kSelect, utils::Vector{af, af, bool_},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(result.sem->Type(), builtin::Function::kSelect);
    EXPECT_EQ(result.sem->ReturnType(), af);
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_EQ(result.sem->Parameters()[0]->Type(), af);
    EXPECT_EQ(result.sem->Parameters()[1]->Type(), af);
    EXPECT_EQ(result.sem->Parameters()[2]->Type(), bool_);
}

TEST_F(IntrinsicTableTest, MatchDifferentArgsElementType_Builtin_RuntimeEval) {
    auto* af = create<type::AbstractFloat>();
    auto* bool_ref = create<type::Reference>(create<type::Bool>(), builtin::AddressSpace::kFunction,
                                             builtin::Access::kReadWrite);
    auto result = table->Lookup(builtin::Function::kSelect, utils::Vector{af, af, bool_ref},
                                sem::EvaluationStage::kRuntime, Source{});
    ASSERT_NE(result.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.sem->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(result.sem->Type(), builtin::Function::kSelect);
    EXPECT_TRUE(result.sem->ReturnType()->Is<type::F32>());
    ASSERT_EQ(result.sem->Parameters().Length(), 3u);
    EXPECT_TRUE(result.sem->Parameters()[0]->Type()->Is<type::F32>());
    EXPECT_TRUE(result.sem->Parameters()[1]->Type()->Is<type::F32>());
    EXPECT_TRUE(result.sem->Parameters()[2]->Type()->Is<type::Bool>());
}

TEST_F(IntrinsicTableTest, MatchDifferentArgsElementType_Binary_ConstantEval) {
    auto* ai = create<type::AbstractInt>();
    auto* u32 = create<type::U32>();
    auto result = table->Lookup(ast::BinaryOp::kShiftLeft, ai, u32, sem::EvaluationStage::kConstant,
                                Source{}, false);
    ASSERT_NE(result.result, nullptr) << Diagnostics().str();
    ASSERT_NE(result.const_eval_fn, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_EQ(result.result, ai);
    EXPECT_EQ(result.lhs, ai);
    EXPECT_EQ(result.rhs, u32);
}

TEST_F(IntrinsicTableTest, MatchDifferentArgsElementType_Binary_RuntimeEval) {
    auto* ai = create<type::AbstractInt>();
    auto* u32 = create<type::U32>();
    auto result = table->Lookup(ast::BinaryOp::kShiftLeft, ai, u32, sem::EvaluationStage::kRuntime,
                                Source{}, false);
    ASSERT_NE(result.result, nullptr) << Diagnostics().str();
    ASSERT_NE(result.const_eval_fn, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");
    EXPECT_TRUE(result.result->Is<type::I32>());
    EXPECT_TRUE(result.lhs->Is<type::I32>());
    EXPECT_TRUE(result.rhs->Is<type::U32>());
}

TEST_F(IntrinsicTableTest, OverloadOrderByNumberOfParameters) {
    // None of the arguments match, so expect the overloads with 2 parameters to
    // come first
    auto* bool_ = create<type::Bool>();
    table->Lookup(builtin::Function::kTextureDimensions, utils::Vector{bool_, bool_},
                  sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(Diagnostics().str(),
              R"(error: no matching call to textureDimensions(bool, bool)

27 candidate functions:
  textureDimensions(texture: texture_1d<T>, level: L) -> u32  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_2d<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_2d_array<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_3d<T>, level: L) -> vec3<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_cube<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_cube_array<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_depth_2d, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_2d_array, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_cube, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_cube_array, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_1d<T>) -> u32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>) -> vec3<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_multisampled_2d<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d) -> vec2<u32>
  textureDimensions(texture: texture_depth_2d_array) -> vec2<u32>
  textureDimensions(texture: texture_depth_cube) -> vec2<u32>
  textureDimensions(texture: texture_depth_cube_array) -> vec2<u32>
  textureDimensions(texture: texture_depth_multisampled_2d) -> vec2<u32>
  textureDimensions(texture: texture_storage_1d<F, A>) -> u32  where: A is write
  textureDimensions(texture: texture_storage_2d<F, A>) -> vec2<u32>  where: A is write
  textureDimensions(texture: texture_storage_2d_array<F, A>) -> vec2<u32>  where: A is write
  textureDimensions(texture: texture_storage_3d<F, A>) -> vec3<u32>  where: A is write
  textureDimensions(texture: texture_external) -> vec2<u32>
)");
}

TEST_F(IntrinsicTableTest, OverloadOrderByMatchingParameter) {
    auto* tex = create<type::DepthTexture>(type::TextureDimension::k2d);
    auto* bool_ = create<type::Bool>();
    table->Lookup(builtin::Function::kTextureDimensions, utils::Vector{tex, bool_},
                  sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(Diagnostics().str(),
              R"(error: no matching call to textureDimensions(texture_depth_2d, bool)

27 candidate functions:
  textureDimensions(texture: texture_depth_2d, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_1d<T>, level: L) -> u32  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_2d<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_2d_array<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_3d<T>, level: L) -> vec3<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_cube<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_cube_array<T>, level: L) -> vec2<u32>  where: T is f32, i32 or u32, L is i32 or u32
  textureDimensions(texture: texture_depth_2d_array, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_cube, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_cube_array, level: L) -> vec2<u32>  where: L is i32 or u32
  textureDimensions(texture: texture_depth_2d) -> vec2<u32>
  textureDimensions(texture: texture_1d<T>) -> u32  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_2d_array<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_3d<T>) -> vec3<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_cube_array<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_multisampled_2d<T>) -> vec2<u32>  where: T is f32, i32 or u32
  textureDimensions(texture: texture_depth_2d_array) -> vec2<u32>
  textureDimensions(texture: texture_depth_cube) -> vec2<u32>
  textureDimensions(texture: texture_depth_cube_array) -> vec2<u32>
  textureDimensions(texture: texture_depth_multisampled_2d) -> vec2<u32>
  textureDimensions(texture: texture_storage_1d<F, A>) -> u32  where: A is write
  textureDimensions(texture: texture_storage_2d<F, A>) -> vec2<u32>  where: A is write
  textureDimensions(texture: texture_storage_2d_array<F, A>) -> vec2<u32>  where: A is write
  textureDimensions(texture: texture_storage_3d<F, A>) -> vec3<u32>  where: A is write
  textureDimensions(texture: texture_external) -> vec2<u32>
)");
}

TEST_F(IntrinsicTableTest, SameOverloadReturnsSameBuiltinPointer) {
    auto* f32 = create<type::F32>();
    auto* vec2_f32 = create<type::Vector>(create<type::F32>(), 2u);
    auto* bool_ = create<type::Bool>();
    auto a = table->Lookup(builtin::Function::kSelect, utils::Vector{f32, f32, bool_},
                           sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(a.sem, nullptr) << Diagnostics().str();

    auto b = table->Lookup(builtin::Function::kSelect, utils::Vector{f32, f32, bool_},
                           sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(b.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");

    auto c = table->Lookup(builtin::Function::kSelect, utils::Vector{vec2_f32, vec2_f32, bool_},
                           sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(c.sem, nullptr) << Diagnostics().str();
    ASSERT_EQ(Diagnostics().str(), "");

    EXPECT_EQ(a.sem, b.sem);
    EXPECT_NE(a.sem, c.sem);
    EXPECT_NE(b.sem, c.sem);
}

TEST_F(IntrinsicTableTest, MatchUnaryOp) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto result = table->Lookup(ast::UnaryOp::kNegation, vec3_i32, sem::EvaluationStage::kConstant,
                                Source{{12, 34}});
    EXPECT_EQ(result.result, vec3_i32);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MismatchUnaryOp) {
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(ast::UnaryOp::kNegation, bool_, sem::EvaluationStage::kConstant,
                                Source{{12, 34}});
    ASSERT_EQ(result.result, nullptr);
    EXPECT_EQ(Diagnostics().str(), R"(12:34 error: no matching overload for operator - (bool)

2 candidate operators:
  operator - (T) -> T  where: T is abstract-float, abstract-int, f32, i32 or f16
  operator - (vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32 or f16
)");
}

TEST_F(IntrinsicTableTest, MatchUnaryOp_Constant) {
    auto* ai = create<type::AbstractInt>();
    auto result = table->Lookup(ast::UnaryOp::kNegation, ai, sem::EvaluationStage::kConstant,
                                Source{{12, 34}});
    EXPECT_EQ(result.result, ai);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MatchUnaryOp_Runtime) {
    auto* ai = create<type::AbstractInt>();
    auto result = table->Lookup(ast::UnaryOp::kNegation, ai, sem::EvaluationStage::kRuntime,
                                Source{{12, 34}});
    EXPECT_NE(result.result, ai);
    EXPECT_TRUE(result.result->Is<type::I32>());
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MatchBinaryOp) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto result = table->Lookup(ast::BinaryOp::kMultiply, i32, vec3_i32,
                                sem::EvaluationStage::kConstant, Source{{12, 34}},
                                /* is_compound */ false);
    EXPECT_EQ(result.result, vec3_i32);
    EXPECT_EQ(result.lhs, i32);
    EXPECT_EQ(result.rhs, vec3_i32);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MismatchBinaryOp) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(ast::BinaryOp::kMultiply, f32, bool_,
                                sem::EvaluationStage::kConstant, Source{{12, 34}},
                                /* is_compound */ false);
    ASSERT_EQ(result.result, nullptr);
    EXPECT_EQ(Diagnostics().str(), R"(12:34 error: no matching overload for operator * (f32, bool)

9 candidate operators:
  operator * (T, T) -> T  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator * (vecN<T>, T) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator * (T, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator * (T, matNxM<T>) -> matNxM<T>  where: T is abstract-float, f32 or f16
  operator * (matNxM<T>, T) -> matNxM<T>  where: T is abstract-float, f32 or f16
  operator * (vecN<T>, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator * (matCxR<T>, vecC<T>) -> vecR<T>  where: T is abstract-float, f32 or f16
  operator * (vecR<T>, matCxR<T>) -> vecC<T>  where: T is abstract-float, f32 or f16
  operator * (matKxR<T>, matCxK<T>) -> matCxR<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(IntrinsicTableTest, MatchCompoundOp) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto result = table->Lookup(ast::BinaryOp::kMultiply, i32, vec3_i32,
                                sem::EvaluationStage::kConstant, Source{{12, 34}},
                                /* is_compound */ true);
    EXPECT_EQ(result.result, vec3_i32);
    EXPECT_EQ(result.lhs, i32);
    EXPECT_EQ(result.rhs, vec3_i32);
    EXPECT_EQ(Diagnostics().str(), "");
}

TEST_F(IntrinsicTableTest, MismatchCompoundOp) {
    auto* f32 = create<type::F32>();
    auto* bool_ = create<type::Bool>();
    auto result = table->Lookup(ast::BinaryOp::kMultiply, f32, bool_,
                                sem::EvaluationStage::kConstant, Source{{12, 34}},
                                /* is_compound */ true);
    ASSERT_EQ(result.result, nullptr);
    EXPECT_EQ(Diagnostics().str(), R"(12:34 error: no matching overload for operator *= (f32, bool)

9 candidate operators:
  operator *= (T, T) -> T  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator *= (vecN<T>, T) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator *= (T, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator *= (T, matNxM<T>) -> matNxM<T>  where: T is abstract-float, f32 or f16
  operator *= (matNxM<T>, T) -> matNxM<T>  where: T is abstract-float, f32 or f16
  operator *= (vecN<T>, vecN<T>) -> vecN<T>  where: T is abstract-float, abstract-int, f32, i32, u32 or f16
  operator *= (matCxR<T>, vecC<T>) -> vecR<T>  where: T is abstract-float, f32 or f16
  operator *= (vecR<T>, matCxR<T>) -> vecC<T>  where: T is abstract-float, f32 or f16
  operator *= (matKxR<T>, matCxK<T>) -> matCxR<T>  where: T is abstract-float, f32 or f16
)");
}

TEST_F(IntrinsicTableTest, MatchTypeInitializerImplicit) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, nullptr, utils::Vector{i32, i32, i32},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->ReturnType(), vec3_i32);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 3u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[2]->Type(), i32);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MatchTypeInitializerExplicit) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, i32, utils::Vector{i32, i32, i32},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->ReturnType(), vec3_i32);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 3u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[2]->Type(), i32);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MismatchTypeInitializerImplicit) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, nullptr, utils::Vector{i32, f32, i32},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_EQ(result.target, nullptr);
    EXPECT_EQ(Diagnostics().str(),
              R"(12:34 error: no matching constructor for vec3(i32, f32, i32)

7 candidate constructors:
  vec3(x: T, y: T, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(xy: vec2<T>, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(x: T, yz: vec2<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(vec3<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3() -> vec3<abstract-int>
  vec3<T>() -> vec3<T>  where: T is f32, f16, i32, u32 or bool

5 candidate conversions:
  vec3<T>(vec3<U>) -> vec3<f32>  where: T is f32, U is abstract-int, abstract-float, i32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<f16>  where: T is f16, U is abstract-int, abstract-float, f32, i32, u32 or bool
  vec3<T>(vec3<U>) -> vec3<i32>  where: T is i32, U is abstract-int, abstract-float, f32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<u32>  where: T is u32, U is abstract-int, abstract-float, f32, f16, i32 or bool
  vec3<T>(vec3<U>) -> vec3<bool>  where: T is bool, U is abstract-int, abstract-float, f32, f16, i32 or u32
)");
}

TEST_F(IntrinsicTableTest, MismatchTypeInitializerExplicit) {
    auto* i32 = create<type::I32>();
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, i32, utils::Vector{i32, f32, i32},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_EQ(result.target, nullptr);
    EXPECT_EQ(Diagnostics().str(),
              R"(12:34 error: no matching constructor for vec3<i32>(i32, f32, i32)

7 candidate constructors:
  vec3(x: T, y: T, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(x: T, yz: vec2<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(xy: vec2<T>, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(vec3<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3() -> vec3<abstract-int>
  vec3<T>() -> vec3<T>  where: T is f32, f16, i32, u32 or bool

5 candidate conversions:
  vec3<T>(vec3<U>) -> vec3<f32>  where: T is f32, U is abstract-int, abstract-float, i32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<f16>  where: T is f16, U is abstract-int, abstract-float, f32, i32, u32 or bool
  vec3<T>(vec3<U>) -> vec3<i32>  where: T is i32, U is abstract-int, abstract-float, f32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<u32>  where: T is u32, U is abstract-int, abstract-float, f32, f16, i32 or bool
  vec3<T>(vec3<U>) -> vec3<bool>  where: T is bool, U is abstract-int, abstract-float, f32, f16, i32 or u32
)");
}

TEST_F(IntrinsicTableTest, MatchTypeInitializerImplicitVecFromVecAbstract) {
    auto* ai = create<type::AbstractInt>();
    auto* vec3_ai = create<type::Vector>(ai, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, nullptr, utils::Vector{vec3_ai},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->ReturnType(), vec3_ai);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 1u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), vec3_ai);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MatchTypeInitializerImplicitMatFromVec) {
    auto* af = create<type::AbstractFloat>();
    auto* vec2_ai = create<type::Vector>(create<type::AbstractInt>(), 2u);
    auto* vec2_af = create<type::Vector>(af, 2u);
    auto* mat2x2_af = create<type::Matrix>(vec2_af, 2u);
    auto result =
        table->Lookup(CtorConvIntrinsic::kMat2x2, nullptr, utils::Vector{vec2_ai, vec2_ai},
                      sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_TYPE(result.target->ReturnType(), mat2x2_af);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 2u);
    EXPECT_TYPE(result.target->Parameters()[0]->Type(), vec2_af);
    EXPECT_TYPE(result.target->Parameters()[1]->Type(), vec2_af);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MatchTypeInitializer_ConstantEval) {
    auto* ai = create<type::AbstractInt>();
    auto* vec3_ai = create<type::Vector>(ai, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, nullptr, utils::Vector{ai, ai, ai},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(result.target->ReturnType(), vec3_ai);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 3u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), ai);
    EXPECT_EQ(result.target->Parameters()[1]->Type(), ai);
    EXPECT_EQ(result.target->Parameters()[2]->Type(), ai);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MatchTypeInitializer_RuntimeEval) {
    auto* ai = create<type::AbstractInt>();
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, nullptr, utils::Vector{ai, ai, ai},
                                sem::EvaluationStage::kRuntime, Source{{12, 34}});
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(result.target->ReturnType(), vec3_i32);
    EXPECT_TRUE(result.target->Is<sem::ValueConstructor>());
    ASSERT_EQ(result.target->Parameters().Length(), 3u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[1]->Type(), i32);
    EXPECT_EQ(result.target->Parameters()[2]->Type(), i32);
    EXPECT_NE(result.const_eval_fn, nullptr);
}

TEST_F(IntrinsicTableTest, MatchTypeConversion) {
    auto* i32 = create<type::I32>();
    auto* vec3_i32 = create<type::Vector>(i32, 3u);
    auto* f32 = create<type::F32>();
    auto* vec3_f32 = create<type::Vector>(f32, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, i32, utils::Vector{vec3_f32},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->ReturnType(), vec3_i32);
    EXPECT_TRUE(result.target->Is<sem::ValueConversion>());
    ASSERT_EQ(result.target->Parameters().Length(), 1u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), vec3_f32);
}

TEST_F(IntrinsicTableTest, MismatchTypeConversion) {
    auto* arr =
        create<type::Array>(create<type::U32>(), create<type::RuntimeArrayCount>(), 4u, 4u, 4u, 4u);
    auto* f32 = create<type::F32>();
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, f32, utils::Vector{arr},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_EQ(result.target, nullptr);
    EXPECT_EQ(Diagnostics().str(),
              R"(12:34 error: no matching constructor for vec3<f32>(array<u32>)

7 candidate constructors:
  vec3(vec3<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3() -> vec3<abstract-int>
  vec3<T>() -> vec3<T>  where: T is f32, f16, i32, u32 or bool
  vec3(xy: vec2<T>, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(x: T, yz: vec2<T>) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool
  vec3(x: T, y: T, z: T) -> vec3<T>  where: T is abstract-int, abstract-float, f32, f16, i32, u32 or bool

5 candidate conversions:
  vec3<T>(vec3<U>) -> vec3<f32>  where: T is f32, U is abstract-int, abstract-float, i32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<f16>  where: T is f16, U is abstract-int, abstract-float, f32, i32, u32 or bool
  vec3<T>(vec3<U>) -> vec3<i32>  where: T is i32, U is abstract-int, abstract-float, f32, f16, u32 or bool
  vec3<T>(vec3<U>) -> vec3<u32>  where: T is u32, U is abstract-int, abstract-float, f32, f16, i32 or bool
  vec3<T>(vec3<U>) -> vec3<bool>  where: T is bool, U is abstract-int, abstract-float, f32, f16, i32 or u32
)");
}

TEST_F(IntrinsicTableTest, MatchTypeConversion_ConstantEval) {
    auto* ai = create<type::AbstractInt>();
    auto* af = create<type::AbstractFloat>();
    auto* vec3_ai = create<type::Vector>(ai, 3u);
    auto* f32 = create<type::F32>();
    auto* vec3_f32 = create<type::Vector>(f32, 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, af, utils::Vector{vec3_ai},
                                sem::EvaluationStage::kConstant, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->Stage(), sem::EvaluationStage::kConstant);
    // NOTE: Conversions are explicit, so there's no way to have it return abstracts
    EXPECT_EQ(result.target->ReturnType(), vec3_f32);
    EXPECT_TRUE(result.target->Is<sem::ValueConversion>());
    ASSERT_EQ(result.target->Parameters().Length(), 1u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), vec3_ai);
}

TEST_F(IntrinsicTableTest, MatchTypeConversion_RuntimeEval) {
    auto* ai = create<type::AbstractInt>();
    auto* af = create<type::AbstractFloat>();
    auto* vec3_ai = create<type::Vector>(ai, 3u);
    auto* vec3_f32 = create<type::Vector>(create<type::F32>(), 3u);
    auto* vec3_i32 = create<type::Vector>(create<type::I32>(), 3u);
    auto result = table->Lookup(CtorConvIntrinsic::kVec3, af, utils::Vector{vec3_ai},
                                sem::EvaluationStage::kRuntime, Source{{12, 34}});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->Stage(), sem::EvaluationStage::kConstant);
    EXPECT_EQ(result.target->ReturnType(), vec3_f32);
    EXPECT_TRUE(result.target->Is<sem::ValueConversion>());
    ASSERT_EQ(result.target->Parameters().Length(), 1u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), vec3_i32);
}

TEST_F(IntrinsicTableTest, Err257Arguments) {  // crbug.com/1323605
    auto* f32 = create<type::F32>();
    utils::Vector<const type::Type*, 0> arg_tys;
    arg_tys.Resize(257, f32);
    auto result = table->Lookup(builtin::Function::kAbs, std::move(arg_tys),
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_EQ(result.sem, nullptr);
    ASSERT_THAT(Diagnostics().str(), HasSubstr("no matching call"));
}

TEST_F(IntrinsicTableTest, OverloadResolution) {
    // i32(abstract-int) produces candidates for both:
    //    ctor i32(i32) -> i32
    //    conv i32<T: scalar_no_i32>(T) -> i32
    // The first should win overload resolution.
    auto* ai = create<type::AbstractInt>();
    auto* i32 = create<type::I32>();
    auto result = table->Lookup(CtorConvIntrinsic::kI32, nullptr, utils::Vector{ai},
                                sem::EvaluationStage::kConstant, Source{});
    ASSERT_NE(result.target, nullptr);
    EXPECT_EQ(result.target->ReturnType(), i32);
    EXPECT_EQ(result.target->Parameters().Length(), 1u);
    EXPECT_EQ(result.target->Parameters()[0]->Type(), ai);
}

////////////////////////////////////////////////////////////////////////////////
// AbstractBinaryTests
////////////////////////////////////////////////////////////////////////////////
namespace AbstractBinaryTests {
struct Case {
    template <typename RESULT,
              typename PARAM_LHS,
              typename PARAM_RHS,
              typename ARG_LHS,
              typename ARG_RHS>
    static Case Create(bool match = true) {
        return {
            match,                              //
            builder::DataType<RESULT>::Sem,     //
            builder::DataType<PARAM_LHS>::Sem,  //
            builder::DataType<PARAM_RHS>::Sem,  //
            builder::DataType<ARG_LHS>::Sem,    //
            builder::DataType<ARG_RHS>::Sem,    //
        };
    }
    bool expected_match;
    builder::sem_type_func_ptr expected_result;
    builder::sem_type_func_ptr expected_param_lhs;
    builder::sem_type_func_ptr expected_param_rhs;
    builder::sem_type_func_ptr arg_lhs;
    builder::sem_type_func_ptr arg_rhs;
};

struct IntrinsicTableAbstractBinaryTest : public ResolverTestWithParam<Case> {
    std::unique_ptr<IntrinsicTable> table = IntrinsicTable::Create(*this);
};

TEST_P(IntrinsicTableAbstractBinaryTest, MatchAdd) {
    auto* arg_lhs = GetParam().arg_lhs(*this);
    auto* arg_rhs = GetParam().arg_rhs(*this);
    auto result = table->Lookup(ast::BinaryOp::kAdd, arg_lhs, arg_rhs,
                                sem::EvaluationStage::kConstant, Source{{12, 34}},
                                /* is_compound */ false);

    bool matched = result.result != nullptr;
    bool expected_match = GetParam().expected_match;
    EXPECT_EQ(matched, expected_match) << Diagnostics().str();

    auto* expected_result = GetParam().expected_result(*this);
    EXPECT_TYPE(result.result, expected_result);

    auto* expected_param_lhs = GetParam().expected_param_lhs(*this);
    EXPECT_TYPE(result.lhs, expected_param_lhs);

    auto* expected_param_rhs = GetParam().expected_param_rhs(*this);
    EXPECT_TYPE(result.rhs, expected_param_rhs);
}

INSTANTIATE_TEST_SUITE_P(AFloat_AInt,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<AFloat,     AFloat,     AFloat,     AFloat,     AFloat>(),
Case::Create<AFloat,     AFloat,     AFloat,     AFloat,     AInt>(),
Case::Create<AFloat,     AFloat,     AFloat,     AInt,       AFloat>(),
Case::Create<AInt,       AInt,       AInt,       AInt,       AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_VecAInt,
    IntrinsicTableAbstractBinaryTest,
    testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<AFloatV,    AFloatV,    AFloatV,    AFloatV,    AFloatV>(),
Case::Create<AFloatV,    AFloatV,    AFloatV,    AFloatV,    AIntV>(),
Case::Create<AFloatV,    AFloatV,    AFloatV,    AIntV,      AFloatV>(),
Case::Create<AIntV,      AIntV,      AIntV,      AIntV,      AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AFloat_f32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32,        f32,        f32,        AFloat,     f32>(),
Case::Create<f32,        f32,        f32,        f32,        AFloat>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAFloat_Vecf32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32V,       f32V,       f32V,       AFloatV,    f32V>(),
Case::Create<f32V,       f32V,       f32V,       f32V,       AFloatV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(
    AFloat_i32,
    IntrinsicTableAbstractBinaryTest,
    testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloat,     i32>(false),
Case::Create<void,        void,        void,        i32,        AFloat>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Veci32,
    IntrinsicTableAbstractBinaryTest,
    testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloatV,    i32V>(false),
Case::Create<void,        void,        void,        i32V,       AFloatV>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(
    AFloat_u32,
    IntrinsicTableAbstractBinaryTest,
    testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloat,     u32>(false),
Case::Create<void,        void,        void,        u32,        AFloat>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Vecu32,
    IntrinsicTableAbstractBinaryTest,
    testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<void,        void,        void,        AFloatV,    u32V>(false),
Case::Create<void,        void,        void,        u32V,       AFloatV>(false)
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_f32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32,        f32,        f32,        AInt,       f32>(),
Case::Create<f32,        f32,        f32,        f32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Vecf32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<f32V,       f32V,       f32V,       AIntV,      f32V>(),
Case::Create<f32V,       f32V,       f32V,       f32V,       AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_i32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<i32,        i32,        i32,        AInt,       i32>(),
Case::Create<i32,        i32,        i32,        i32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Veci32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<i32V,       i32V,       i32V,       AIntV,      i32V>(),
Case::Create<i32V,       i32V,       i32V,       i32V,       AIntV>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(AInt_u32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<u32,        u32,        u32,        AInt,       u32>(),
Case::Create<u32,        u32,        u32,        u32,        AInt>()
                             ));  // clang-format on

INSTANTIATE_TEST_SUITE_P(VecAInt_Vecu32,
                         IntrinsicTableAbstractBinaryTest,
                         testing::Values(  // clang-format off
//            result   | param lhs | param rhs |  arg lhs  |  arg rhs
Case::Create<u32V,       u32V,       u32V,       AIntV,      u32V>(),
Case::Create<u32V,       u32V,       u32V,       u32V,       AIntV>()
                             ));  // clang-format on

}  // namespace AbstractBinaryTests

////////////////////////////////////////////////////////////////////////////////
// AbstractTernaryTests
////////////////////////////////////////////////////////////////////////////////
namespace AbstractTernaryTests {
struct Case {
    template <typename RESULT,
              typename PARAM_A,
              typename PARAM_B,
              typename PARAM_C,
              typename ARG_A,
              typename ARG_B,
              typename ARG_C>
    static Case Create(bool match = true) {
        return {
            match,
            builder::DataType<RESULT>::Sem,   //
            builder::DataType<PARAM_A>::Sem,  //
            builder::DataType<PARAM_B>::Sem,  //
            builder::DataType<PARAM_C>::Sem,  //
            builder::DataType<ARG_A>::Sem,    //
            builder::DataType<ARG_B>::Sem,    //
            builder::DataType<ARG_C>::Sem,    //
        };
    }
    bool expected_match;
    builder::sem_type_func_ptr expected_result;
    builder::sem_type_func_ptr expected_param_a;
    builder::sem_type_func_ptr expected_param_b;
    builder::sem_type_func_ptr expected_param_c;
    builder::sem_type_func_ptr arg_a;
    builder::sem_type_func_ptr arg_b;
    builder::sem_type_func_ptr arg_c;
};

struct IntrinsicTableAbstractTernaryTest : public ResolverTestWithParam<Case> {
    std::unique_ptr<IntrinsicTable> table = IntrinsicTable::Create(*this);
};

TEST_P(IntrinsicTableAbstractTernaryTest, MatchClamp) {
    auto* arg_a = GetParam().arg_a(*this);
    auto* arg_b = GetParam().arg_b(*this);
    auto* arg_c = GetParam().arg_c(*this);
    auto builtin = table->Lookup(builtin::Function::kClamp, utils::Vector{arg_a, arg_b, arg_c},
                                 sem::EvaluationStage::kConstant, Source{{12, 34}});

    bool matched = builtin.sem != nullptr;
    bool expected_match = GetParam().expected_match;
    EXPECT_EQ(matched, expected_match) << Diagnostics().str();

    auto* result = builtin.sem ? builtin.sem->ReturnType() : nullptr;
    auto* expected_result = GetParam().expected_result(*this);
    EXPECT_TYPE(result, expected_result);

    auto* param_a = builtin.sem ? builtin.sem->Parameters()[0]->Type() : nullptr;
    auto* expected_param_a = GetParam().expected_param_a(*this);
    EXPECT_TYPE(param_a, expected_param_a);

    auto* param_b = builtin.sem ? builtin.sem->Parameters()[1]->Type() : nullptr;
    auto* expected_param_b = GetParam().expected_param_b(*this);
    EXPECT_TYPE(param_b, expected_param_b);

    auto* param_c = builtin.sem ? builtin.sem->Parameters()[2]->Type() : nullptr;
    auto* expected_param_c = GetParam().expected_param_c(*this);
    EXPECT_TYPE(param_c, expected_param_c);
}

INSTANTIATE_TEST_SUITE_P(
    AFloat_AInt,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AFloat,   AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AFloat,   AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AInt,     AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AFloat,   AInt,     AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AFloat,   AFloat>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AFloat,   AInt>(),
Case::Create<AFloat,    AFloat,  AFloat,   AFloat,   AInt,     AInt,     AFloat>(),
Case::Create<AInt,      AInt,    AInt,     AInt,     AInt,     AInt,     AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_VecAInt,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV,  AFloatV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AFloatV,  AIntV>(),
Case::Create<AFloatV,  AFloatV,  AFloatV,  AFloatV,  AIntV,    AIntV,    AFloatV>(),
Case::Create<AIntV,    AIntV,    AIntV,    AIntV,    AIntV,    AIntV,    AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_f32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32,      f32,      f32,      f32,      AFloat,   AFloat,   f32>(),
Case::Create<f32,      f32,      f32,      f32,      AFloat,   f32,      AFloat>(),
Case::Create<f32,      f32,      f32,      f32,      AFloat,   f32,      f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AFloat,   AFloat>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AFloat,   f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      f32,      AFloat>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Vecf32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  AFloatV,  f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V,     AFloatV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V,     f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV,  AFloatV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV,  f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     f32V,     AFloatV> ()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_i32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloat,   AFloat,   i32>(false),
Case::Create<void,     void,     void,     void,     AFloat,   i32,      AFloat>(false),
Case::Create<void,     void,     void,     void,     AFloat,   i32,      i32>(false),
Case::Create<void,     void,     void,     void,     i32,      AFloat,   AFloat>(false),
Case::Create<void,     void,     void,     void,     i32,      AFloat,   i32>(false),
Case::Create<void,     void,     void,     void,     i32,      i32,      AFloat>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Veci32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloatV,  AFloatV,  i32V>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  i32V,     AFloatV>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  i32V,     i32V>(false),
Case::Create<void,     void,     void,     void,     i32V,     AFloatV,  AFloatV>(false),
Case::Create<void,     void,     void,     void,     i32V,     AFloatV,  i32V>(false),
Case::Create<void,     void,     void,     void,     i32V,     i32V,     AFloatV>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AFloat_u32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloat,   AFloat,   u32>(false),
Case::Create<void,     void,     void,     void,     AFloat,   u32,      AFloat>(false),
Case::Create<void,     void,     void,     void,     AFloat,   u32,      u32>(false),
Case::Create<void,     void,     void,     void,     u32,      AFloat,   AFloat>(false),
Case::Create<void,     void,     void,     void,     u32,      AFloat,   u32>(false),
Case::Create<void,     void,     void,     void,     u32,      u32,      AFloat>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAFloat_Vecu32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<void,     void,     void,     void,     AFloatV,  AFloatV,  u32V>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  u32V,     AFloatV>(false),
Case::Create<void,     void,     void,     void,     AFloatV,  u32V,     u32V>(false),
Case::Create<void,     void,     void,     void,     u32V,     AFloatV,  AFloatV>(false),
Case::Create<void,     void,     void,     void,     u32V,     AFloatV,  u32V>(false),
Case::Create<void,     void,     void,     void,     u32V,     u32V,     AFloatV>(false)
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_f32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32,      f32,      f32,      f32,      AInt,     AInt,     f32>(),
Case::Create<f32,      f32,      f32,      f32,      AInt,     f32,      AInt>(),
Case::Create<f32,      f32,      f32,      f32,      AInt,     f32,      f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AInt,     AInt>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      AInt,     f32>(),
Case::Create<f32,      f32,      f32,      f32,      f32,      f32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Vecf32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    AIntV,    f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    f32V,     AIntV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     AIntV,    f32V,     f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AIntV,    AIntV>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     AIntV,    f32V>(),
Case::Create<f32V,     f32V,     f32V,     f32V,     f32V,     f32V,     AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_i32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<i32,      i32,      i32,      i32,      AInt,     AInt,     i32>(),
Case::Create<i32,      i32,      i32,      i32,      AInt,     i32,      AInt>(),
Case::Create<i32,      i32,      i32,      i32,      AInt,     i32,      i32>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      AInt,     AInt>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      AInt,     i32>(),
Case::Create<i32,      i32,      i32,      i32,      i32,      i32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Veci32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    AIntV,     i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    i32V,      AIntV>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     AIntV,    i32V,      i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     AIntV,     AIntV>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     AIntV,     i32V>(),
Case::Create<i32V,     i32V,     i32V,     i32V,     i32V,     i32V,      AIntV>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    AInt_u32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<u32,      u32,      u32,      u32,      AInt,     AInt,     u32>(),
Case::Create<u32,      u32,      u32,      u32,      AInt,     u32,      AInt>(),
Case::Create<u32,      u32,      u32,      u32,      AInt,     u32,      u32>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      AInt,     AInt>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      AInt,     u32>(),
Case::Create<u32,      u32,      u32,      u32,      u32,      u32,      AInt>()
        // clang-format on
        ));

INSTANTIATE_TEST_SUITE_P(
    VecAInt_Vecu32,
    IntrinsicTableAbstractTernaryTest,
    testing::Values(  // clang-format off
//           result  | param a | param b | param c |  arg a  |  arg b  |  arg c
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    AIntV,    u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    u32V,     AIntV>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     AIntV,    u32V,     u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     AIntV,    AIntV>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     AIntV,    u32V>(),
Case::Create<u32V,     u32V,     u32V,     u32V,     u32V,     u32V,     AIntV>()
        // clang-format on
        ));

}  // namespace AbstractTernaryTests

}  // namespace
}  // namespace tint::resolver
