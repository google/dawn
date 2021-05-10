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

#include "src/reader/spirv/enum_converter.h"

#include <string>

#include "gmock/gmock.h"

namespace tint {
namespace reader {
namespace spirv {
namespace {

// Pipeline stage

struct PipelineStageCase {
  SpvExecutionModel model;
  bool expect_success;
  ast::PipelineStage expected;
};
inline std::ostream& operator<<(std::ostream& out, PipelineStageCase psc) {
  out << "PipelineStageCase{ SpvExecutionModel:" << int(psc.model)
      << " expect_success?:" << int(psc.expect_success)
      << " expected:" << int(psc.expected) << "}";
  return out;
}

class SpvPipelineStageTest : public testing::TestWithParam<PipelineStageCase> {
 public:
  SpvPipelineStageTest()
      : success_(true),
        fail_stream_(&success_, &errors_),
        converter_(fail_stream_) {}

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
    EXPECT_THAT(error(),
                ::testing::StartsWith("unknown SPIR-V execution model:"));
  }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvPipelineStageTest,
    testing::Values(PipelineStageCase{SpvExecutionModelVertex, true,
                                      ast::PipelineStage::kVertex},
                    PipelineStageCase{SpvExecutionModelFragment, true,
                                      ast::PipelineStage::kFragment},
                    PipelineStageCase{SpvExecutionModelGLCompute, true,
                                      ast::PipelineStage::kCompute}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
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
      << " expect_success?:" << int(scc.expect_success)
      << " expected:" << int(scc.expected) << "}";
  return out;
}

class SpvStorageClassTest : public testing::TestWithParam<StorageClassCase> {
 public:
  SpvStorageClassTest()
      : success_(true),
        fail_stream_(&success_, &errors_),
        converter_(fail_stream_) {}

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
    EXPECT_THAT(error(),
                ::testing::StartsWith("unknown SPIR-V storage class: "));
  }
}

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
    SpvStorageClassTest,
    testing::Values(
        StorageClassCase{SpvStorageClassInput, true, ast::StorageClass::kInput},
        StorageClassCase{SpvStorageClassOutput, true,
                         ast::StorageClass::kOutput},
        StorageClassCase{SpvStorageClassUniform, true,
                         ast::StorageClass::kUniform},
        StorageClassCase{SpvStorageClassWorkgroup, true,
                         ast::StorageClass::kWorkgroup},
        StorageClassCase{SpvStorageClassUniformConstant, true,
                         ast::StorageClass::kNone},
        StorageClassCase{SpvStorageClassStorageBuffer, true,
                         ast::StorageClass::kStorage},
        StorageClassCase{SpvStorageClassImage, true, ast::StorageClass::kImage},
        StorageClassCase{SpvStorageClassPrivate, true,
                         ast::StorageClass::kPrivate},
        StorageClassCase{SpvStorageClassFunction, true,
                         ast::StorageClass::kFunction}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvStorageClassTest,
                         testing::Values(StorageClassCase{
                             static_cast<SpvStorageClass>(9999), false,
                             ast::StorageClass::kInvalid}));

// Builtin

struct BuiltinCase {
  SpvBuiltIn builtin;
  bool expect_success;
  ast::Builtin expected;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinCase bc) {
  out << "BuiltinCase{ SpvBuiltIn:" << int(bc.builtin)
      << " expect_success?:" << int(bc.expect_success)
      << " expected:" << int(bc.expected) << "}";
  return out;
}

class SpvBuiltinTest : public testing::TestWithParam<BuiltinCase> {
 public:
  SpvBuiltinTest()
      : success_(true),
        fail_stream_(&success_, &errors_),
        converter_(fail_stream_) {}

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
        BuiltinCase{SpvBuiltInInstanceIndex, true,
                    ast::Builtin::kInstanceIndex},
        BuiltinCase{SpvBuiltInFrontFacing, true, ast::Builtin::kFrontFacing},
        BuiltinCase{SpvBuiltInFragCoord, true, ast::Builtin::kPosition},
        BuiltinCase{SpvBuiltInLocalInvocationId, true,
                    ast::Builtin::kLocalInvocationId},
        BuiltinCase{SpvBuiltInLocalInvocationIndex, true,
                    ast::Builtin::kLocalInvocationIndex},
        BuiltinCase{SpvBuiltInGlobalInvocationId, true,
                    ast::Builtin::kGlobalInvocationId},
        BuiltinCase{SpvBuiltInWorkgroupId, true, ast::Builtin::kWorkgroupId},
        BuiltinCase{SpvBuiltInSampleId, true, ast::Builtin::kSampleIndex},
        BuiltinCase{SpvBuiltInSampleMask, true, ast::Builtin::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Output,
    SpvBuiltinTest,
    testing::Values(
        BuiltinCase{SpvBuiltInPosition, true, ast::Builtin::kPosition},
        BuiltinCase{SpvBuiltInFragDepth, true, ast::Builtin::kFragDepth},
        BuiltinCase{SpvBuiltInSampleMask, true, ast::Builtin::kSampleMask}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvBuiltinTest,
    testing::Values(
        BuiltinCase{static_cast<SpvBuiltIn>(9999), false, ast::Builtin::kNone},
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
      << " expect_success?:" << int(dc.expect_success)
      << " expected:" << int(dc.expected) << "}";
  return out;
}

class SpvDimTest : public testing::TestWithParam<DimCase> {
 public:
  SpvDimTest()
      : success_(true),
        fail_stream_(&success_, &errors_),
        converter_(fail_stream_) {}

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

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood,
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

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
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
        DimCase{SpvDimSubpassData, true, false, ast::TextureDimension::kNone}));

// ImageFormat

struct ImageFormatCase {
  SpvImageFormat format;
  bool expect_success;
  ast::ImageFormat expected;
};
inline std::ostream& operator<<(std::ostream& out, ImageFormatCase ifc) {
  out << "ImageFormatCase{ SpvImageFormat:" << int(ifc.format)
      << " expect_success?:" << int(ifc.expect_success)
      << " expected:" << int(ifc.expected) << "}";
  return out;
}

class SpvImageFormatTest : public testing::TestWithParam<ImageFormatCase> {
 public:
  SpvImageFormatTest()
      : success_(true),
        fail_stream_(&success_, &errors_),
        converter_(fail_stream_) {}

  std::string error() const { return errors_.str(); }

 protected:
  bool success_ = true;
  std::stringstream errors_;
  FailStream fail_stream_;
  EnumConverter converter_;
};

TEST_P(SpvImageFormatTest, Samples) {
  const auto params = GetParam();

  const auto result = converter_.ToImageFormat(params.format);
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
        ImageFormatCase{SpvImageFormatUnknown, true, ast::ImageFormat::kNone},
        // 8 bit channels
        ImageFormatCase{SpvImageFormatRgba8, true,
                        ast::ImageFormat::kRgba8Unorm},
        ImageFormatCase{SpvImageFormatRgba8Snorm, true,
                        ast::ImageFormat::kRgba8Snorm},
        ImageFormatCase{SpvImageFormatRgba8ui, true,
                        ast::ImageFormat::kRgba8Uint},
        ImageFormatCase{SpvImageFormatRgba8i, true,
                        ast::ImageFormat::kRgba8Sint},
        // 16 bit channels
        ImageFormatCase{SpvImageFormatRgba16ui, true,
                        ast::ImageFormat::kRgba16Uint},
        ImageFormatCase{SpvImageFormatRgba16i, true,
                        ast::ImageFormat::kRgba16Sint},
        ImageFormatCase{SpvImageFormatRgba16f, true,
                        ast::ImageFormat::kRgba16Float},
        // 32 bit channels
        // ... 1 channel
        ImageFormatCase{SpvImageFormatR32ui, true, ast::ImageFormat::kR32Uint},
        ImageFormatCase{SpvImageFormatR32i, true, ast::ImageFormat::kR32Sint},
        ImageFormatCase{SpvImageFormatR32f, true, ast::ImageFormat::kR32Float},
        // ... 2 channels
        ImageFormatCase{SpvImageFormatRg32ui, true,
                        ast::ImageFormat::kRg32Uint},
        ImageFormatCase{SpvImageFormatRg32i, true, ast::ImageFormat::kRg32Sint},
        ImageFormatCase{SpvImageFormatRg32f, true,
                        ast::ImageFormat::kRg32Float},
        // ... 4 channels
        ImageFormatCase{SpvImageFormatRgba32ui, true,
                        ast::ImageFormat::kRgba32Uint},
        ImageFormatCase{SpvImageFormatRgba32i, true,
                        ast::ImageFormat::kRgba32Sint},
        ImageFormatCase{SpvImageFormatRgba32f, true,
                        ast::ImageFormat::kRgba32Float}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvImageFormatTest,
    testing::Values(
        // Scanning in order from the SPIR-V spec.
        ImageFormatCase{SpvImageFormatRg16f, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR11fG11fB10f, false,
                        ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR16f, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgb10A2, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR16, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR8, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgba16Snorm, false,
                        ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16Snorm, false,
                        ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8Snorm, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16i, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8i, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR8i, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgb10a2ui, false,
                        ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16ui, false, ast::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8ui, false, ast::ImageFormat::kNone}));

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
