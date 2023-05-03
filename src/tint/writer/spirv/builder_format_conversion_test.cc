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

#include "src/tint/writer/spirv/spv_dump.h"
#include "src/tint/writer/spirv/test_helper.h"

namespace tint::writer::spirv {
namespace {

struct TestData {
    builtin::TexelFormat ast_format;
    SpvImageFormat_ spv_format;
    bool extended_format = false;
};
inline std::ostream& operator<<(std::ostream& out, TestData data) {
    utils::StringStream str;
    str << data.ast_format;
    out << str.str();
    return out;
}
using ImageFormatConversionTest = TestParamHelper<TestData>;

TEST_P(ImageFormatConversionTest, ImageFormatConversion) {
    auto param = GetParam();

    spirv::Builder& b = Build();

    EXPECT_EQ(b.convert_texel_format_to_spv(param.ast_format), param.spv_format);

    if (param.extended_format) {
        EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
                  R"(OpCapability StorageImageExtendedFormats
)");
    } else {
        EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
    }
}

INSTANTIATE_TEST_SUITE_P(
    BuilderTest,
    ImageFormatConversionTest,
    testing::Values(
        /* WGSL unsupported formats
  TestData{builtin::TexelFormat::kR8Unorm, SpvImageFormatR8, true},
  TestData{builtin::TexelFormat::kR8Snorm, SpvImageFormatR8Snorm, true},
  TestData{builtin::TexelFormat::kR8Uint, SpvImageFormatR8ui, true},
  TestData{builtin::TexelFormat::kR8Sint, SpvImageFormatR8i, true},
  TestData{builtin::TexelFormat::kR16Uint, SpvImageFormatR16ui, true},
  TestData{builtin::TexelFormat::kR16Sint, SpvImageFormatR16i, true},
  TestData{builtin::TexelFormat::kR16Float, SpvImageFormatR16f, true},
  TestData{builtin::TexelFormat::kRg8Unorm, SpvImageFormatRg8, true},
  TestData{builtin::TexelFormat::kRg8Snorm, SpvImageFormatRg8Snorm, true},
  TestData{builtin::TexelFormat::kRg8Uint, SpvImageFormatRg8ui, true},
  TestData{builtin::TexelFormat::kRg8Sint, SpvImageFormatRg8i, true},
  TestData{builtin::TexelFormat::kRg16Uint, SpvImageFormatRg16ui, true},
  TestData{builtin::TexelFormat::kRg16Sint, SpvImageFormatRg16i, true},
  TestData{builtin::TexelFormat::kRg16Float, SpvImageFormatRg16f, true},
  TestData{builtin::TexelFormat::kRgba8UnormSrgb, SpvImageFormatUnknown},
  TestData{builtin::TexelFormat::kBgra8Unorm, SpvImageFormatUnknown},
  TestData{builtin::TexelFormat::kBgra8UnormSrgb, SpvImageFormatUnknown},
  TestData{builtin::TexelFormat::kRgb10A2Unorm, SpvImageFormatRgb10A2, true},
  TestData{builtin::TexelFormat::kRg11B10Float, SpvImageFormatR11fG11fB10f, true},
*/
        TestData{builtin::TexelFormat::kR32Uint, SpvImageFormatR32ui},
        TestData{builtin::TexelFormat::kR32Sint, SpvImageFormatR32i},
        TestData{builtin::TexelFormat::kR32Float, SpvImageFormatR32f},
        TestData{builtin::TexelFormat::kRgba8Unorm, SpvImageFormatRgba8},
        TestData{builtin::TexelFormat::kRgba8Snorm, SpvImageFormatRgba8Snorm},
        TestData{builtin::TexelFormat::kRgba8Uint, SpvImageFormatRgba8ui},
        TestData{builtin::TexelFormat::kRgba8Sint, SpvImageFormatRgba8i},
        TestData{builtin::TexelFormat::kRg32Uint, SpvImageFormatRg32ui, true},
        TestData{builtin::TexelFormat::kRg32Sint, SpvImageFormatRg32i, true},
        TestData{builtin::TexelFormat::kRg32Float, SpvImageFormatRg32f, true},
        TestData{builtin::TexelFormat::kRgba16Uint, SpvImageFormatRgba16ui},
        TestData{builtin::TexelFormat::kRgba16Sint, SpvImageFormatRgba16i},
        TestData{builtin::TexelFormat::kRgba16Float, SpvImageFormatRgba16f},
        TestData{builtin::TexelFormat::kRgba32Uint, SpvImageFormatRgba32ui},
        TestData{builtin::TexelFormat::kRgba32Sint, SpvImageFormatRgba32i},
        TestData{builtin::TexelFormat::kRgba32Float, SpvImageFormatRgba32f}));

}  // namespace
}  // namespace tint::writer::spirv
