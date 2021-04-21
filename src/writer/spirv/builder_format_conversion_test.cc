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

#include "src/writer/spirv/spv_dump.h"
#include "src/writer/spirv/test_helper.h"

namespace tint {
namespace writer {
namespace spirv {
namespace {

struct TestData {
  ast::ImageFormat ast_format;
  SpvImageFormat_ spv_format;
  bool extended_format = false;
};
inline std::ostream& operator<<(std::ostream& out, TestData data) {
  out << data.ast_format;
  return out;
}
using ImageFormatConversionTest = TestParamHelper<TestData>;

TEST_P(ImageFormatConversionTest, ImageFormatConversion) {
  auto param = GetParam();

  spirv::Builder& b = Build();

  EXPECT_EQ(b.convert_image_format_to_spv(param.ast_format), param.spv_format);

  if (param.extended_format) {
    EXPECT_EQ(DumpInstructions(b.capabilities()),
              R"(OpCapability StorageImageExtendedFormats
)");
  } else {
    EXPECT_EQ(DumpInstructions(b.capabilities()), "");
  }
}

INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    ImageFormatConversionTest,
    testing::Values(
        TestData{ast::ImageFormat::kR8Unorm, SpvImageFormatR8, true},
        TestData{ast::ImageFormat::kR8Snorm, SpvImageFormatR8Snorm, true},
        TestData{ast::ImageFormat::kR8Uint, SpvImageFormatR8ui, true},
        TestData{ast::ImageFormat::kR8Sint, SpvImageFormatR8i, true},
        TestData{ast::ImageFormat::kR16Uint, SpvImageFormatR16ui, true},
        TestData{ast::ImageFormat::kR16Sint, SpvImageFormatR16i, true},
        TestData{ast::ImageFormat::kR16Float, SpvImageFormatR16f, true},
        TestData{ast::ImageFormat::kRg8Unorm, SpvImageFormatRg8, true},
        TestData{ast::ImageFormat::kRg8Snorm, SpvImageFormatRg8Snorm, true},
        TestData{ast::ImageFormat::kRg8Uint, SpvImageFormatRg8ui, true},
        TestData{ast::ImageFormat::kRg8Sint, SpvImageFormatRg8i, true},
        TestData{ast::ImageFormat::kR32Uint, SpvImageFormatR32ui},
        TestData{ast::ImageFormat::kR32Sint, SpvImageFormatR32i},
        TestData{ast::ImageFormat::kR32Float, SpvImageFormatR32f},
        TestData{ast::ImageFormat::kRg16Uint, SpvImageFormatRg16ui, true},
        TestData{ast::ImageFormat::kRg16Sint, SpvImageFormatRg16i, true},
        TestData{ast::ImageFormat::kRg16Float, SpvImageFormatRg16f, true},
        TestData{ast::ImageFormat::kRgba8Unorm, SpvImageFormatRgba8},
        TestData{ast::ImageFormat::kRgba8UnormSrgb, SpvImageFormatUnknown},
        TestData{ast::ImageFormat::kRgba8Snorm, SpvImageFormatRgba8Snorm},
        TestData{ast::ImageFormat::kRgba8Uint, SpvImageFormatRgba8ui},
        TestData{ast::ImageFormat::kRgba8Sint, SpvImageFormatRgba8i},
        TestData{ast::ImageFormat::kBgra8Unorm, SpvImageFormatUnknown},
        TestData{ast::ImageFormat::kBgra8UnormSrgb, SpvImageFormatUnknown},
        TestData{ast::ImageFormat::kRgb10A2Unorm, SpvImageFormatRgb10A2, true},
        TestData{ast::ImageFormat::kRg11B10Float, SpvImageFormatR11fG11fB10f,
                 true},
        TestData{ast::ImageFormat::kRg32Uint, SpvImageFormatRg32ui, true},
        TestData{ast::ImageFormat::kRg32Sint, SpvImageFormatRg32i, true},
        TestData{ast::ImageFormat::kRg32Float, SpvImageFormatRg32f, true},
        TestData{ast::ImageFormat::kRgba16Uint, SpvImageFormatRgba16ui},
        TestData{ast::ImageFormat::kRgba16Sint, SpvImageFormatRgba16i},
        TestData{ast::ImageFormat::kRgba16Float, SpvImageFormatRgba16f},
        TestData{ast::ImageFormat::kRgba32Uint, SpvImageFormatRgba32ui},
        TestData{ast::ImageFormat::kRgba32Sint, SpvImageFormatRgba32i},
        TestData{ast::ImageFormat::kRgba32Float, SpvImageFormatRgba32f}));

}  // namespace
}  // namespace spirv
}  // namespace writer
}  // namespace tint
