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
                         ast::StorageClass::kUniformConstant},
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
                             ast::StorageClass::kNone}));

// Builtin

struct BuiltinCase {
  SpvBuiltIn builtin;
  ast::StorageClass sc;
  bool expect_success;
  ast::Builtin expected;
};
inline std::ostream& operator<<(std::ostream& out, BuiltinCase bc) {
  out << "BuiltinCase{ SpvBuiltIn:" << int(bc.builtin) << " sc:" << int(bc.sc)
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

  const auto result = converter_.ToBuiltin(params.builtin, params.sc);
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
        BuiltinCase{SpvBuiltInPosition, ast::StorageClass::kInput, true,
                    ast::Builtin::kPosition},
        BuiltinCase{SpvBuiltInInstanceIndex, ast::StorageClass::kInput, true,
                    ast::Builtin::kInstanceIndex},
        BuiltinCase{SpvBuiltInFrontFacing, ast::StorageClass::kInput, true,
                    ast::Builtin::kFrontFacing},
        BuiltinCase{SpvBuiltInFragCoord, ast::StorageClass::kInput, true,
                    ast::Builtin::kFragCoord},
        BuiltinCase{SpvBuiltInLocalInvocationId, ast::StorageClass::kInput,
                    true, ast::Builtin::kLocalInvocationId},
        BuiltinCase{SpvBuiltInLocalInvocationIndex, ast::StorageClass::kInput,
                    true, ast::Builtin::kLocalInvocationIndex},
        BuiltinCase{SpvBuiltInGlobalInvocationId, ast::StorageClass::kInput,
                    true, ast::Builtin::kGlobalInvocationId},
        BuiltinCase{SpvBuiltInSampleId, ast::StorageClass::kInput, true,
                    ast::Builtin::kSampleIndex},
        BuiltinCase{SpvBuiltInSampleMask, ast::StorageClass::kInput, true,
                    ast::Builtin::kSampleMaskIn}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterGood_Output,
    SpvBuiltinTest,
    testing::Values(BuiltinCase{SpvBuiltInPosition, ast::StorageClass::kOutput,
                                true, ast::Builtin::kPosition},
                    BuiltinCase{SpvBuiltInFragDepth, ast::StorageClass::kOutput,
                                true, ast::Builtin::kFragDepth},
                    BuiltinCase{SpvBuiltInSampleMask,
                                ast::StorageClass::kOutput, true,
                                ast::Builtin::kSampleMaskOut}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvBuiltinTest,
    testing::Values(
        BuiltinCase{static_cast<SpvBuiltIn>(9999), ast::StorageClass::kInput,
                    false, ast::Builtin::kNone},
        BuiltinCase{static_cast<SpvBuiltIn>(9999), ast::StorageClass::kOutput,
                    false, ast::Builtin::kNone},
        BuiltinCase{SpvBuiltInNumWorkgroups, ast::StorageClass::kInput, false,
                    ast::Builtin::kNone}));

// Dim

struct DimCase {
  SpvDim dim;
  bool arrayed;
  bool expect_success;
  type::TextureDimension expected;
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
        DimCase{SpvDim1D, false, true, type::TextureDimension::k1d},
        DimCase{SpvDim2D, false, true, type::TextureDimension::k2d},
        DimCase{SpvDim3D, false, true, type::TextureDimension::k3d},
        DimCase{SpvDimCube, false, true, type::TextureDimension::kCube},
        // Arrayed
        DimCase{SpvDim2D, true, true, type::TextureDimension::k2dArray},
        DimCase{SpvDimCube, true, true, type::TextureDimension::kCubeArray}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvDimTest,
    testing::Values(
        // Invalid SPIR-V dimensionality.
        DimCase{SpvDimMax, false, false, type::TextureDimension::kNone},
        DimCase{SpvDimMax, true, false, type::TextureDimension::kNone},
        // Vulkan non-arrayed dimensionalities not supported by WGSL.
        DimCase{SpvDimRect, false, false, type::TextureDimension::kNone},
        DimCase{SpvDimBuffer, false, false, type::TextureDimension::kNone},
        DimCase{SpvDimSubpassData, false, false, type::TextureDimension::kNone},
        // Arrayed dimensionalities not supported by WGSL
        DimCase{SpvDim3D, true, false, type::TextureDimension::kNone},
        DimCase{SpvDimRect, true, false, type::TextureDimension::kNone},
        DimCase{SpvDimBuffer, true, false, type::TextureDimension::kNone},
        DimCase{SpvDimSubpassData, true, false,
                type::TextureDimension::kNone}));

// ImageFormat

struct ImageFormatCase {
  SpvImageFormat format;
  bool expect_success;
  type::ImageFormat expected;
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
        ImageFormatCase{SpvImageFormatUnknown, true, type::ImageFormat::kNone},
        // 8 bit channels
        ImageFormatCase{SpvImageFormatRgba8, true,
                        type::ImageFormat::kRgba8Unorm},
        ImageFormatCase{SpvImageFormatRgba8Snorm, true,
                        type::ImageFormat::kRgba8Snorm},
        ImageFormatCase{SpvImageFormatRgba8ui, true,
                        type::ImageFormat::kRgba8Uint},
        ImageFormatCase{SpvImageFormatRgba8i, true,
                        type::ImageFormat::kRgba8Sint},
        // 16 bit channels
        ImageFormatCase{SpvImageFormatRgba16ui, true,
                        type::ImageFormat::kRgba16Uint},
        ImageFormatCase{SpvImageFormatRgba16i, true,
                        type::ImageFormat::kRgba16Sint},
        ImageFormatCase{SpvImageFormatRgba16f, true,
                        type::ImageFormat::kRgba16Float},
        // 32 bit channels
        // ... 1 channel
        ImageFormatCase{SpvImageFormatR32ui, true, type::ImageFormat::kR32Uint},
        ImageFormatCase{SpvImageFormatR32i, true, type::ImageFormat::kR32Sint},
        ImageFormatCase{SpvImageFormatR32f, true, type::ImageFormat::kR32Float},
        // ... 2 channels
        ImageFormatCase{SpvImageFormatRg32ui, true,
                        type::ImageFormat::kRg32Uint},
        ImageFormatCase{SpvImageFormatRg32i, true,
                        type::ImageFormat::kRg32Sint},
        ImageFormatCase{SpvImageFormatRg32f, true,
                        type::ImageFormat::kRg32Float},
        // ... 4 channels
        ImageFormatCase{SpvImageFormatRgba32ui, true,
                        type::ImageFormat::kRgba32Uint},
        ImageFormatCase{SpvImageFormatRgba32i, true,
                        type::ImageFormat::kRgba32Sint},
        ImageFormatCase{SpvImageFormatRgba32f, true,
                        type::ImageFormat::kRgba32Float}));

INSTANTIATE_TEST_SUITE_P(
    EnumConverterBad,
    SpvImageFormatTest,
    testing::Values(
        // Scanning in order from the SPIR-V spec.
        ImageFormatCase{SpvImageFormatRg16f, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR11fG11fB10f, false,
                        type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR16f, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgb10A2, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR16, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR8, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgba16Snorm, false,
                        type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16Snorm, false,
                        type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8Snorm, false,
                        type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16i, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8i, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatR8i, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRgb10a2ui, false,
                        type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg16ui, false, type::ImageFormat::kNone},
        ImageFormatCase{SpvImageFormatRg8ui, false, type::ImageFormat::kNone}));

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
