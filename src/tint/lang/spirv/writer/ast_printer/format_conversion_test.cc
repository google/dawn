// Copyright 2020 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/writer/ast_printer/helper_test.h"
#include "src/tint/lang/spirv/writer/common/spv_dump_test.h"

namespace tint::spirv::writer {
namespace {

struct TestData {
    core::TexelFormat ast_format;
    SpvImageFormat_ spv_format;
    bool extended_format = false;
};
inline std::ostream& operator<<(std::ostream& out, TestData data) {
    StringStream str;
    str << data.ast_format;
    out << str.str();
    return out;
}
using ImageFormatConversionTest = TestParamHelper<TestData>;

TEST_P(ImageFormatConversionTest, ImageFormatConversion) {
    auto param = GetParam();

    Builder& b = Build();

    EXPECT_EQ(b.convert_texel_format_to_spv(param.ast_format), param.spv_format);

    if (param.extended_format) {
        EXPECT_EQ(DumpInstructions(b.Module().Capabilities()),
                  R"(OpCapability StorageImageExtendedFormats
)");
    } else {
        EXPECT_EQ(DumpInstructions(b.Module().Capabilities()), "");
    }
}

INSTANTIATE_TEST_SUITE_P(SpirvASTPrinterTest,
                         ImageFormatConversionTest,
                         testing::Values(
                             /* WGSL unsupported formats
                       TestData{core::TexelFormat::kR8Unorm, SpvImageFormatR8, true},
                       TestData{core::TexelFormat::kR8Snorm, SpvImageFormatR8Snorm, true},
                       TestData{core::TexelFormat::kR8Uint, SpvImageFormatR8ui, true},
                       TestData{core::TexelFormat::kR8Sint, SpvImageFormatR8i, true},
                       TestData{core::TexelFormat::kR16Uint, SpvImageFormatR16ui, true},
                       TestData{core::TexelFormat::kR16Sint, SpvImageFormatR16i, true},
                       TestData{core::TexelFormat::kR16Float, SpvImageFormatR16f, true},
                       TestData{core::TexelFormat::kRg8Unorm, SpvImageFormatRg8, true},
                       TestData{core::TexelFormat::kRg8Snorm, SpvImageFormatRg8Snorm, true},
                       TestData{core::TexelFormat::kRg8Uint, SpvImageFormatRg8ui, true},
                       TestData{core::TexelFormat::kRg8Sint, SpvImageFormatRg8i, true},
                       TestData{core::TexelFormat::kRg16Uint, SpvImageFormatRg16ui, true},
                       TestData{core::TexelFormat::kRg16Sint, SpvImageFormatRg16i, true},
                       TestData{core::TexelFormat::kRg16Float, SpvImageFormatRg16f, true},
                       TestData{core::TexelFormat::kRgba8UnormSrgb, SpvImageFormatUnknown},
                       TestData{core::TexelFormat::kBgra8Unorm, SpvImageFormatUnknown},
                       TestData{core::TexelFormat::kBgra8UnormSrgb, SpvImageFormatUnknown},
                       TestData{core::TexelFormat::kRgb10A2Unorm, SpvImageFormatRgb10A2, true},
                       TestData{core::TexelFormat::kRg11B10Float, SpvImageFormatR11fG11fB10f, true},
                     */
                             TestData{core::TexelFormat::kR32Uint, SpvImageFormatR32ui},
                             TestData{core::TexelFormat::kR32Sint, SpvImageFormatR32i},
                             TestData{core::TexelFormat::kR32Float, SpvImageFormatR32f},
                             TestData{core::TexelFormat::kRgba8Unorm, SpvImageFormatRgba8},
                             TestData{core::TexelFormat::kRgba8Snorm, SpvImageFormatRgba8Snorm},
                             TestData{core::TexelFormat::kRgba8Uint, SpvImageFormatRgba8ui},
                             TestData{core::TexelFormat::kRgba8Sint, SpvImageFormatRgba8i},
                             TestData{core::TexelFormat::kRg32Uint, SpvImageFormatRg32ui, true},
                             TestData{core::TexelFormat::kRg32Sint, SpvImageFormatRg32i, true},
                             TestData{core::TexelFormat::kRg32Float, SpvImageFormatRg32f, true},
                             TestData{core::TexelFormat::kRgba16Uint, SpvImageFormatRgba16ui},
                             TestData{core::TexelFormat::kRgba16Sint, SpvImageFormatRgba16i},
                             TestData{core::TexelFormat::kRgba16Float, SpvImageFormatRgba16f},
                             TestData{core::TexelFormat::kRgba32Uint, SpvImageFormatRgba32ui},
                             TestData{core::TexelFormat::kRgba32Sint, SpvImageFormatRgba32i},
                             TestData{core::TexelFormat::kRgba32Float, SpvImageFormatRgba32f}));

}  // namespace
}  // namespace tint::spirv::writer
