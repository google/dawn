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

#include <ostream>
#include <string>

#include "gmock/gmock.h"
#include "spirv/unified1/spirv.h"
#include "src/ast/pipeline_stage.h"

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
    testing::Values(PipelineStageCase{SpvExecutionModel(9999), false,
                                      ast::PipelineStage::kNone},
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
                         ast::StorageClass::kStorageBuffer},
        StorageClassCase{SpvStorageClassImage, true, ast::StorageClass::kImage},
        StorageClassCase{SpvStorageClassPushConstant, true,
                         ast::StorageClass::kPushConstant},
        StorageClassCase{SpvStorageClassPrivate, true,
                         ast::StorageClass::kPrivate},
        StorageClassCase{SpvStorageClassFunction, true,
                         ast::StorageClass::kFunction}));

INSTANTIATE_TEST_SUITE_P(EnumConverterBad,
                         SpvStorageClassTest,
                         testing::Values(StorageClassCase{
                             SpvStorageClass(9999), false,
                             ast::StorageClass::kNone}));

}  // namespace
}  // namespace spirv
}  // namespace reader
}  // namespace tint
