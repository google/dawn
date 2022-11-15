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

#include "src/tint/reader/spirv/enum_converter.h"

#include <string>

#include "gmock/gmock.h"

namespace tint::reader::spirv {
namespace {

// Pipeline stage

struct PipelineStageCase {
    spv::ExecutionModel model;
    bool expect_success;
    ast::PipelineStage expected;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageCase psc) {
    out << "PipelineStageCase{ spv::ExecutionModel:::" << int(psc.model)
        << " expect_success?:" << int(psc.expect_success) << " expected:" << int(psc.expected)
        << "}";
    return out;
}

class SpvPipelineStageTest : public testing::TestWithParam<PipelineStageCase> {
  public:
    SpvPipelineStageTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    std::stringstream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvPipelineStageTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToPipelineStage(params.model);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V execution model:"));
    }
}

INSTANTIATE_TEST_SUITE_P(EnumConverterGood,
                         SpvPipelineStageTest,
                         testing::Values(PipelineStageCase{spv::ExecutionModel::Vertex, true,
                                                           ast::PipelineStage::kVertex},
                                         PipelineStageCase{spv::ExecutionModel::Fragment, true,
                                                           ast::PipelineStage::kFragment},
                                         PipelineStageCase{spv::ExecutionModel::GLCompute, true,
                                                           ast::PipelineStage::kCompute}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvPipelineStageTest,
                         testing::Values(PipelineStageCase{static_cast<spv::ExecutionModel>(9999),
                                                           false, ast::PipelineStage::kNone},
                                         PipelineStageCase{spv::ExecutionModel::TessellationControl,
                                                           false, ast::PipelineStage::kNone}));

// Storage class

struct StorageClassCase {
    spv::StorageClass sc;
    bool expect_success;
    ast::AddressSpace expected;
};
inline std::ostream& operator<<(std::ostream& out, StorageClassCase scc) {
    out << "StorageClassCase{ spv::StorageClass:::" << int(scc.sc)
        << " expect_success?:" << int(scc.expect_success) << " expected:" << int(scc.expected)
        << "}";
    return out;
}

class SpvStorageClassTest : public testing::TestWithParam<StorageClassCase> {
  public:
    SpvStorageClassTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    std::stringstream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvStorageClassTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToAddressSpace(params.sc);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V storage class: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvStorageClassTest,
    testing::Values(
        StorageClassCase{spv::StorageClass::Input, true, ast::AddressSpace::kIn},
        StorageClassCase{spv::StorageClass::Output, true, ast::AddressSpace::kOut},
        StorageClassCase{spv::StorageClass::Uniform, true, ast::AddressSpace::kUniform},
        StorageClassCase{spv::StorageClass::Workgroup, true, ast::AddressSpace::kWorkgroup},
        StorageClassCase{spv::StorageClass::UniformConstant, true, ast::AddressSpace::kNone},
        StorageClassCase{spv::StorageClass::StorageBuffer, true, ast::AddressSpace::kStorage},
        StorageClassCase{spv::StorageClass::Private, true, ast::AddressSpace::kPrivate},
        StorageClassCase{spv::StorageClass::Function, true, ast::AddressSpace::kFunction}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvStorageClassTest,
                         testing::Values(StorageClassCase{static_cast<spv::StorageClass>(9999),
                                                          false, ast::AddressSpace::kUndefined}));

// Builtin

struct BuiltinCase {
    spv::BuiltIn builtin;
    bool expect_success;
    ast::BuiltinValue expected;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinCase bc) {
    out << "BuiltinCase{ spv::BuiltIn::" << int(bc.builtin)
        << " expect_success?:" << int(bc.expect_success) << " expected:" << int(bc.expected) << "}";
    return out;
}

class SpvBuiltinTest : public testing::TestWithParam<BuiltinCase> {
  public:
    SpvBuiltinTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    std::stringstream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvBuiltinTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToBuiltin(params.builtin);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("unknown SPIR-V builtin: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Input,
    SpvBuiltinTest,
    testing::Values(
        BuiltinCase{spv::BuiltIn::Position, true, ast::BuiltinValue::kPosition},
        BuiltinCase{spv::BuiltIn::InstanceIndex, true, ast::BuiltinValue::kInstanceIndex},
        BuiltinCase{spv::BuiltIn::FrontFacing, true, ast::BuiltinValue::kFrontFacing},
        BuiltinCase{spv::BuiltIn::FragCoord, true, ast::BuiltinValue::kPosition},
        BuiltinCase{spv::BuiltIn::LocalInvocationId, true, ast::BuiltinValue::kLocalInvocationId},
        BuiltinCase{spv::BuiltIn::LocalInvocationIndex, true,
                    ast::BuiltinValue::kLocalInvocationIndex},
        BuiltinCase{spv::BuiltIn::GlobalInvocationId, true, ast::BuiltinValue::kGlobalInvocationId},
        BuiltinCase{spv::BuiltIn::NumWorkgroups, true, ast::BuiltinValue::kNumWorkgroups},
        BuiltinCase{spv::BuiltIn::WorkgroupId, true, ast::BuiltinValue::kWorkgroupId},
        BuiltinCase{spv::BuiltIn::SampleId, true, ast::BuiltinValue::kSampleIndex},
        BuiltinCase{spv::BuiltIn::SampleMask, true, ast::BuiltinValue::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Output,
    SpvBuiltinTest,
    testing::Values(BuiltinCase{spv::BuiltIn::Position, true, ast::BuiltinValue::kPosition},
                    BuiltinCase{spv::BuiltIn::FragDepth, true, ast::BuiltinValue::kFragDepth},
                    BuiltinCase{spv::BuiltIn::SampleMask, true, ast::BuiltinValue::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvBuiltinTest,
                         testing::Values(BuiltinCase{static_cast<spv::BuiltIn>(9999), false,
                                                     ast::BuiltinValue::kUndefined},
                                         BuiltinCase{static_cast<spv::BuiltIn>(9999), false,
                                                     ast::BuiltinValue::kUndefined}));

// Dim

struct DimCase {
    spv::Dim dim;
    bool arrayed;
    bool expect_success;
    ast::TextureDimension expected;
};
inline std::ostream& operator<<(std::ostream& out, DimCase dc) {
    out << "DimCase{ spv::Dim:::" << int(dc.dim) << " arrayed?:" << int(dc.arrayed)
        << " expect_success?:" << int(dc.expect_success) << " expected:" << int(dc.expected) << "}";
    return out;
}

class SpvDimTest : public testing::TestWithParam<DimCase> {
  public:
    SpvDimTest() : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    std::stringstream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvDimTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToDim(params.dim, params.arrayed);
    EXPECT_EQ(success_, params.expect_success);
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::HasSubstr("dimension"));
    }
}

INSTANTIATE_TEST_SUITE_P(EnumConverterGood,
                         SpvDimTest,
                         testing::Values(
                             // Non-arrayed
                             DimCase{spv::Dim::Dim1D, false, true, ast::TextureDimension::k1d},
                             DimCase{spv::Dim::Dim2D, false, true, ast::TextureDimension::k2d},
                             DimCase{spv::Dim::Dim3D, false, true, ast::TextureDimension::k3d},
                             DimCase{spv::Dim::Cube, false, true, ast::TextureDimension::kCube},
                             // Arrayed
                             DimCase{spv::Dim::Dim2D, true, true, ast::TextureDimension::k2dArray},
                             DimCase{spv::Dim::Cube, true, true,
                                     ast::TextureDimension::kCubeArray}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvDimTest,
    testing::Values(
        // Invalid SPIR-V dimensionality.
        DimCase{spv::Dim::Max, false, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::Max, true, false, ast::TextureDimension::kNone},
        // Vulkan non-arrayed dimensionalities not supported by WGSL.
        DimCase{spv::Dim::Rect, false, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::Buffer, false, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::SubpassData, false, false, ast::TextureDimension::kNone},
        // Arrayed dimensionalities not supported by WGSL
        DimCase{spv::Dim::Dim3D, true, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::Rect, true, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::Buffer, true, false, ast::TextureDimension::kNone},
        DimCase{spv::Dim::SubpassData, true, false, ast::TextureDimension::kNone}));

// TexelFormat

struct TexelFormatCase {
    spv::ImageFormat format;
    bool expect_success;
    ast::TexelFormat expected;
};
inline std::ostream& operator<<(std::ostream& out, TexelFormatCase ifc) {
    out << "TexelFormatCase{ spv::ImageFormat:::" << int(ifc.format)
        << " expect_success?:" << int(ifc.expect_success) << " expected:" << int(ifc.expected)
        << "}";
    return out;
}

class SpvImageFormatTest : public testing::TestWithParam<TexelFormatCase> {
  public:
    SpvImageFormatTest()
        : success_(true), fail_stream_(&success_, &errors_), converter_(fail_stream_) {}

    std::string error() const { return errors_.str(); }

  protected:
    bool success_ = true;
    std::stringstream errors_;
    FailStream fail_stream_;
    EnumConverter converter_;
};

TEST_P(SpvImageFormatTest, Samples) {
    const auto params = GetParam();

    const auto result = converter_.ToTexelFormat(params.format);
    EXPECT_EQ(success_, params.expect_success) << params;
    if (params.expect_success) {
        EXPECT_EQ(result, params.expected);
        EXPECT_TRUE(error().empty());
    } else {
        EXPECT_EQ(result, params.expected);
        EXPECT_THAT(error(), ::testing::StartsWith("invalid image format: "));
    }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvImageFormatTest,
    testing::Values(
        // Unknown.  This is used for sampled images.
        TexelFormatCase{spv::ImageFormat::Unknown, true, ast::TexelFormat::kUndefined},
        // 8 bit channels
        TexelFormatCase{spv::ImageFormat::Rgba8, true, ast::TexelFormat::kRgba8Unorm},
        TexelFormatCase{spv::ImageFormat::Rgba8Snorm, true, ast::TexelFormat::kRgba8Snorm},
        TexelFormatCase{spv::ImageFormat::Rgba8ui, true, ast::TexelFormat::kRgba8Uint},
        TexelFormatCase{spv::ImageFormat::Rgba8i, true, ast::TexelFormat::kRgba8Sint},
        // 16 bit channels
        TexelFormatCase{spv::ImageFormat::Rgba16ui, true, ast::TexelFormat::kRgba16Uint},
        TexelFormatCase{spv::ImageFormat::Rgba16i, true, ast::TexelFormat::kRgba16Sint},
        TexelFormatCase{spv::ImageFormat::Rgba16f, true, ast::TexelFormat::kRgba16Float},
        // 32 bit channels
        // ... 1 channel
        TexelFormatCase{spv::ImageFormat::R32ui, true, ast::TexelFormat::kR32Uint},
        TexelFormatCase{spv::ImageFormat::R32i, true, ast::TexelFormat::kR32Sint},
        TexelFormatCase{spv::ImageFormat::R32f, true, ast::TexelFormat::kR32Float},
        // ... 2 channels
        TexelFormatCase{spv::ImageFormat::Rg32ui, true, ast::TexelFormat::kRg32Uint},
        TexelFormatCase{spv::ImageFormat::Rg32i, true, ast::TexelFormat::kRg32Sint},
        TexelFormatCase{spv::ImageFormat::Rg32f, true, ast::TexelFormat::kRg32Float},
        // ... 4 channels
        TexelFormatCase{spv::ImageFormat::Rgba32ui, true, ast::TexelFormat::kRgba32Uint},
        TexelFormatCase{spv::ImageFormat::Rgba32i, true, ast::TexelFormat::kRgba32Sint},
        TexelFormatCase{spv::ImageFormat::Rgba32f, true, ast::TexelFormat::kRgba32Float}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvImageFormatTest,
    testing::Values(
        // Scanning in order from the SPIR-V spec.
        TexelFormatCase{spv::ImageFormat::Rg16f, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R11fG11fB10f, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R16f, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgb10A2, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R16, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R8, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgba16Snorm, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16Snorm, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8Snorm, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16i, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8i, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::R8i, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rgb10a2ui, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg16ui, false, ast::TexelFormat::kUndefined},
        TexelFormatCase{spv::ImageFormat::Rg8ui, false, ast::TexelFormat::kUndefined}));

}  // namespace
}  // namespace tint::reader::spirv
