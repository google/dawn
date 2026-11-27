// Copyright 2026 The Dawn & Tint Authors
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

#include <gtest/gtest.h>

#include "src/dawn/common/ExternalTextureParams.h"

namespace dawn {
namespace {

// Verify ComputeExternalTextureParams sets doYuvToRgbConversionOnly when and only when the source
// and destination color spaces produce a no-op color-space conversion.
TEST(ExternalTextureParamsTests, DoYuvToRgbConversionOnly) {
    EXPECT_TRUE(GetNoopColorSpaceParams().doYuvToRgbConversionOnly);

    auto Check = [](bool expected, wgpu::ColorSpacePrimariesDawn primaries,
                    wgpu::ColorSpaceTransferDawn transfer, wgpu::PredefinedColorSpace dst) {
        ExternalTextureColorSpaceParams params;
        params.doYuvToRgbConversionOnly = !expected;
        wgpu::ColorSpaceDawn src = {
            .primaries = primaries,
            .transfer = transfer,
            .yCbCrRange = wgpu::ColorSpaceYCbCrRangeDawn::Narrow,
            .yCbCrMatrix = wgpu::ColorSpaceYCbCrMatrixDawn::Rec709,
        };
        ASSERT_EQ(wgpu::Status::Success, ComputeExternalTextureParams(src, dst, &params));
        EXPECT_EQ(expected, params.doYuvToRgbConversionOnly);
    };

    // A source with the same primaries and transfer function as the destination is a no-op.
    struct PredefinedColorSpaceComponents {
        wgpu::PredefinedColorSpace colorSpace;
        wgpu::ColorSpacePrimariesDawn primaries;
        wgpu::ColorSpaceTransferDawn transfer;
    };
    constexpr PredefinedColorSpaceComponents kPredefinedColorSpaces[] = {
        {wgpu::PredefinedColorSpace::SRGB, wgpu::ColorSpacePrimariesDawn::SRGB,
         wgpu::ColorSpaceTransferDawn::SRGB},
        {wgpu::PredefinedColorSpace::DisplayP3, wgpu::ColorSpacePrimariesDawn::DisplayP3,
         wgpu::ColorSpaceTransferDawn::DisplayP3},
        {wgpu::PredefinedColorSpace::SRGBLinear, wgpu::ColorSpacePrimariesDawn::SRGB,
         wgpu::ColorSpaceTransferDawn::Identity},
        {wgpu::PredefinedColorSpace::DisplayP3Linear, wgpu::ColorSpacePrimariesDawn::DisplayP3,
         wgpu::ColorSpaceTransferDawn::Identity},
        {wgpu::PredefinedColorSpace::Rec2020Linear, wgpu::ColorSpacePrimariesDawn::Rec2020,
         wgpu::ColorSpaceTransferDawn::Identity},
    };
    for (const auto& c : kPredefinedColorSpaces) {
        Check(true, c.primaries, c.transfer, c.colorSpace);
    }

    // Different primaries.
    Check(false, wgpu::ColorSpacePrimariesDawn::DisplayP3, wgpu::ColorSpaceTransferDawn::SRGB,
          wgpu::PredefinedColorSpace::SRGB);
    // Different transfer function.
    Check(false, wgpu::ColorSpacePrimariesDawn::SRGB, wgpu::ColorSpaceTransferDawn::SRGB,
          wgpu::PredefinedColorSpace::SRGBLinear);
}

}  // anonymous namespace
}  // namespace dawn
