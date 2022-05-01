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
    SpvExecutionModel model;
    bool expect_success;
    ast::PipelineStage expected;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageCase psc) {
    out << "PipelineStageCase{ SpvExecutionModel:" << int(psc.model)
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
                         testing::Values(PipelineStageCase{SpvExecutionModelVertex, true,
                                                           ast::PipelineStage::kVertex},
                                         PipelineStageCase{SpvExecutionModelFragment, true,
                                                           ast::PipelineStage::kFragment},
                                         PipelineStageCase{SpvExecutionModelGLCompute, true,
                                                           ast::PipelineStage::kCompute}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvPipelineStageTest,
                         testing::Values(PipelineStageCase{static_cast<SpvExecutionModel>(9999),
                                                           false, ast::PipelineStage::kNone},
                                         PipelineStageCase{SpvExecutionModelTessellationControl,
                                                           false, ast::PipelineStage::kNone}));

// Storage class

struct StorageClassCase {
    SpvStorageClass sc;
    bool expect_success;
    ast::StorageClass expected;
};
inline std::ostream& operator<<(std::ostream& out, StorageClassCase scc) {
    out << "StorageClassCase{ SpvStorageClass:" << int(scc.sc)
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

    const auto result = converter_.ToStorageClass(params.sc);
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
        StorageClassCase{SpvStorageClassInput, true, ast::StorageClass::kInput},
        StorageClassCase{SpvStorageClassOutput, true, ast::StorageClass::kOutput},
        StorageClassCase{SpvStorageClassUniform, true, ast::StorageClass::kUniform},
        StorageClassCase{SpvStorageClassWorkgroup, true, ast::StorageClass::kWorkgroup},
        StorageClassCase{SpvStorageClassUniformConstant, true, ast::StorageClass::kNone},
        StorageClassCase{SpvStorageClassStorageBuffer, true, ast::StorageClass::kStorage},
        StorageClassCase{SpvStorageClassPrivate, true, ast::StorageClass::kPrivate},
        StorageClassCase{SpvStorageClassFunction, true, ast::StorageClass::kFunction}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvStorageClassTest,
                         testing::Values(StorageClassCase{static_cast<SpvStorageClass>(9999), false,
                                                          ast::StorageClass::kInvalid}));

// Builtin

struct BuiltinCase {
    SpvBuiltIn builtin;
    bool expect_success;
    ast::Builtin expected;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinCase bc) {
    out << "BuiltinCase{ SpvBuiltIn:" << int(bc.builtin)
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
        BuiltinCase{SpvBuiltInPosition, true, ast::Builtin::kPosition},
        BuiltinCase{SpvBuiltInInstanceIndex, true, ast::Builtin::kInstanceIndex},
        BuiltinCase{SpvBuiltInFrontFacing, true, ast::Builtin::kFrontFacing},
        BuiltinCase{SpvBuiltInFragCoord, true, ast::Builtin::kPosition},
        BuiltinCase{SpvBuiltInLocalInvocationId, true, ast::Builtin::kLocalInvocationId},
        BuiltinCase{SpvBuiltInLocalInvocationIndex, true, ast::Builtin::kLocalInvocationIndex},
        BuiltinCase{SpvBuiltInGlobalInvocationId, true, ast::Builtin::kGlobalInvocationId},
        BuiltinCase{SpvBuiltInWorkgroupId, true, ast::Builtin::kWorkgroupId},
        BuiltinCase{SpvBuiltInSampleId, true, ast::Builtin::kSampleIndex},
        BuiltinCase{SpvBuiltInSampleMask, true, ast::Builtin::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Output,
    SpvBuiltinTest,
    testing::Values(BuiltinCase{SpvBuiltInPosition, true, ast::Builtin::kPosition},
                    BuiltinCase{SpvBuiltInFragDepth, true, ast::Builtin::kFragDepth},
                    BuiltinCase{SpvBuiltInSampleMask, true, ast::Builtin::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvBuiltinTest,
    testing::Values(BuiltinCase{static_cast<SpvBuiltIn>(9999), false, ast::Builtin::kNone},
                    BuiltinCase{static_cast<SpvBuiltIn>(9999), false, ast::Builtin::kNone},
                    BuiltinCase{SpvBuiltInNumWorkgroups, false, ast::Builtin::kNone}));

// Dim

struct DimCase {
    SpvDim dim;
    bool arrayed;
    bool expect_success;
    ast::TextureDimension expected;
};
inline std::ostream& operator<<(std::ostream& out, DimCase dc) {
    out << "DimCase{ SpvDim:" << int(dc.dim) << " arrayed?:" << int(dc.arrayed)
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
                             DimCase{SpvDim1D, false, true, ast::TextureDimension::k1d},
                             DimCase{SpvDim2D, false, true, ast::TextureDimension::k2d},
                             DimCase{SpvDim3D, false, true, ast::TextureDimension::k3d},
                             DimCase{SpvDimCube, false, true, ast::TextureDimension::kCube},
                             // Arrayed
                             DimCase{SpvDim2D, true, true, ast::TextureDimension::k2dArray},
                             DimCase{SpvDimCube, true, true, ast::TextureDimension::kCubeArray}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvDimTest,
                         testing::Values(
                             // Invalid SPIR-V dimensionality.
                             DimCase{SpvDimMax, false, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimMax, true, false, ast::TextureDimension::kNone},
                             // Vulkan non-arrayed dimensionalities not supported by WGSL.
                             DimCase{SpvDimRect, false, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimBuffer, false, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimSubpassData, false, false, ast::TextureDimension::kNone},
                             // Arrayed dimensionalities not supported by WGSL
                             DimCase{SpvDim3D, true, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimRect, true, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimBuffer, true, false, ast::TextureDimension::kNone},
                             DimCase{SpvDimSubpassData, true, false,
                                     ast::TextureDimension::kNone}));

// TexelFormat

struct TexelFormatCase {
    SpvImageFormat format;
    bool expect_success;
    ast::TexelFormat expected;
};
inline std::ostream& operator<<(std::ostream& out, TexelFormatCase ifc) {
    out << "TexelFormatCase{ SpvImageFormat:" << int(ifc.format)
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
        TexelFormatCase{SpvImageFormatUnknown, true, ast::TexelFormat::kNone},
        // 8 bit channels
        TexelFormatCase{SpvImageFormatRgba8, true, ast::TexelFormat::kRgba8Unorm},
        TexelFormatCase{SpvImageFormatRgba8Snorm, true, ast::TexelFormat::kRgba8Snorm},
        TexelFormatCase{SpvImageFormatRgba8ui, true, ast::TexelFormat::kRgba8Uint},
        TexelFormatCase{SpvImageFormatRgba8i, true, ast::TexelFormat::kRgba8Sint},
        // 16 bit channels
        TexelFormatCase{SpvImageFormatRgba16ui, true, ast::TexelFormat::kRgba16Uint},
        TexelFormatCase{SpvImageFormatRgba16i, true, ast::TexelFormat::kRgba16Sint},
        TexelFormatCase{SpvImageFormatRgba16f, true, ast::TexelFormat::kRgba16Float},
        // 32 bit channels
        // ... 1 channel
        TexelFormatCase{SpvImageFormatR32ui, true, ast::TexelFormat::kR32Uint},
        TexelFormatCase{SpvImageFormatR32i, true, ast::TexelFormat::kR32Sint},
        TexelFormatCase{SpvImageFormatR32f, true, ast::TexelFormat::kR32Float},
        // ... 2 channels
        TexelFormatCase{SpvImageFormatRg32ui, true, ast::TexelFormat::kRg32Uint},
        TexelFormatCase{SpvImageFormatRg32i, true, ast::TexelFormat::kRg32Sint},
        TexelFormatCase{SpvImageFormatRg32f, true, ast::TexelFormat::kRg32Float},
        // ... 4 channels
        TexelFormatCase{SpvImageFormatRgba32ui, true, ast::TexelFormat::kRgba32Uint},
        TexelFormatCase{SpvImageFormatRgba32i, true, ast::TexelFormat::kRgba32Sint},
        TexelFormatCase{SpvImageFormatRgba32f, true, ast::TexelFormat::kRgba32Float}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvImageFormatTest,
    testing::Values(
        // Scanning in order from the SPIR-V spec.
        TexelFormatCase{SpvImageFormatRg16f, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatR11fG11fB10f, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatR16f, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRgb10A2, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg16, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg8, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatR16, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatR8, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRgba16Snorm, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg16Snorm, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg8Snorm, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg16i, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg8i, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatR8i, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRgb10a2ui, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg16ui, false, ast::TexelFormat::kNone},
        TexelFormatCase{SpvImageFormatRg8ui, false, ast::TexelFormat::kNone}));

}  // namespace
}  // namespace tint::reader::spirv
