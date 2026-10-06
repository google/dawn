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

#include <ostream>
#include <span>
#include <utility>
#include <vector>

#include "dawn/webgpu_cpp_print.h"
#include "gtest/gtest.h"
#include "src/dawn/common/GPUInfo.h"
#include "src/dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "src/utils/assert.h"

namespace dawn::native {

std::ostream& operator<<(std::ostream& o, const SubgroupMatrixConfig& config) {
    o << config.M << "x" << config.N << "x" << config.K << " " << config.componentType << " -> "
      << config.resultComponentType << " [" << config.minSubgroupSize << ", "
      << config.maxSubgroupSize << "]";
    return o;
}

}  // namespace dawn::native

namespace dawn::native::d3d12 {
namespace {

using enum wgpu::SubgroupMatrixComponentType;

class SubgroupMatrixConfigTests : public testing::Test {
  protected:
    struct Shape {
        uint32_t M;
        uint32_t N;
        uint32_t K;
    };

    struct SubgroupSizeRange {
        uint32_t min;
        uint32_t max;
    };

    D3D12_LINEAR_ALGEBRA_DATATYPE ToD3D12(wgpu::SubgroupMatrixComponentType type) {
        switch (type) {
            case I32:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32;
            case U32:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_UINT32;
            case F16:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16;
            case F32:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32;
            case I8:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_SINT8;
            case U8:
                return D3D12_LINEAR_ALGEBRA_DATATYPE_UINT8;
            default:
                DAWN_UNREACHABLE();
        }
    }

    D3D12DeviceInfo::LinAlgWMMSupport MakeSupport(uint32_t waveSize,
                                                  wgpu::SubgroupMatrixComponentType typeAB,
                                                  wgpu::SubgroupMatrixComponentType typeAcc,
                                                  std::initializer_list<Shape> shapes) {
        D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_INPUTS inputs{};
        inputs.WaveSize = waveSize;
        inputs.MatrixAComponentType = ToD3D12(typeAB);
        inputs.MatrixBComponentType = ToD3D12(typeAB);
        inputs.AccumulatorComponentType = ToD3D12(typeAcc);

        std::vector<D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE> d3dShapes;
        for (const auto& s : shapes) {
            D3D12_LINEAR_ALGEBRA_MATRIX_MULTIPLY_SHAPE d3dShape{};
            d3dShape.M = s.M;
            d3dShape.N = s.N;
            d3dShape.K = s.K;
            d3dShapes.push_back(d3dShape);
        }

        return {inputs, D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED,
                std::move(d3dShapes)};
    }

    SubgroupMatrixConfig MakeConfig(wgpu::SubgroupMatrixComponentType componentType,
                                    wgpu::SubgroupMatrixComponentType resultComponentType,
                                    Shape shape,
                                    SubgroupSizeRange subgroupSizeRange) {
        SubgroupMatrixConfig config;
        config.componentType = componentType;
        config.resultComponentType = resultComponentType;
        config.M = shape.M;
        config.N = shape.N;
        config.K = shape.K;
        config.minSubgroupSize = subgroupSizeRange.min;
        config.maxSubgroupSize = subgroupSizeRange.max;
        return config;
    }

    std::vector<SubgroupMatrixConfig> EnumerateSubgroupMatrixConfigs(
        std::span<const D3D12DeviceInfo::LinAlgWMMSupport> supports,
        uint32_t vendorId = 0,
        uint32_t deviceId = 0,
        bool supportsShaderF16 = true) {
        return PhysicalDevice::EnumerateSubgroupMatrixConfigs(supports, vendorId, deviceId,
                                                              supportsShaderF16);
    }
};

// Identical base shapes across consecutive power-of-two wave sizes merge into a single config.
TEST_F(SubgroupMatrixConfigTests, SameShapeConsecutiveWaveSizes) {
    std::vector supports = {
        MakeSupport(16, F16, F32, {{16, 16, 16}}),
        MakeSupport(32, F16, F32, {{16, 16, 16}}),
        MakeSupport(64, F16, F32, {{16, 16, 16}}),
    };
    std::vector expected = {
        MakeConfig(F16, F32, {16, 16, 16}, {16, 64}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// Identical base shapes with a gap in wave sizes only merge consecutive powers of two.
TEST_F(SubgroupMatrixConfigTests, SameShapeNonConsecutiveWaveSizes) {
    std::vector supports = {
        MakeSupport(4, F16, F32, {{16, 16, 16}}),
        MakeSupport(16, F16, F32, {{16, 16, 16}}),
        MakeSupport(32, F16, F32, {{16, 16, 16}}),
    };
    std::vector expected = {
        MakeConfig(F16, F32, {16, 16, 16}, {4, 4}),
        MakeConfig(F16, F32, {16, 16, 16}, {16, 32}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// When a larger wave size reports a shape that is a multiple of a smaller wave size's shape
// (e.g. WARP reporting 4x4x4 at WaveSize 4 and 8x8x4 at WaveSize 8), the larger shape's
// minSubgroupSize extends down to the smaller wave size, while the smaller shape stays at its own
// wave size (e.g WARP's 8x8x4 range should be [4,8]).
TEST_F(SubgroupMatrixConfigTests, LargerWaveSizeIsMultipleOfSmallerWaveSize) {
    std::vector supports = {
        MakeSupport(4, F32, F32, {{4, 4, 4}}),
        MakeSupport(8, F32, F32, {{8, 8, 4}}),
        MakeSupport(16, F32, F32, {{16, 16, 4}}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),
        MakeConfig(F32, F32, {8, 8, 4}, {4, 8}),
        MakeConfig(F32, F32, {16, 16, 4}, {4, 16}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// When a smaller wave size reports a shape that is a multiple of a larger wave size's shape, the
// larger shape's maxSubgroupSize extends up to the larger wave size. This doesn't seem to happen in
// practice, but support it in case it does.
TEST_F(SubgroupMatrixConfigTests, SmallerWaveSizeIsMultipleOfLargerWaveSize) {
    std::vector supports = {
        MakeSupport(4, F32, F32, {{16, 16, 4}}),
        MakeSupport(8, F32, F32, {{8, 8, 4}}),
        MakeSupport(16, F32, F32, {{4, 4, 4}}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {16, 16, 4}, {4, 16}),
        MakeConfig(F32, F32, {8, 8, 4}, {8, 16}),
        MakeConfig(F32, F32, {4, 4, 4}, {16, 16}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// Shape multiples across non-consecutive wave sizes do not merge across a missing power of two.
TEST_F(SubgroupMatrixConfigTests, NonConsecutiveMultipleWaveSizes) {
    std::vector supports = {
        MakeSupport(4, F32, F32, {{4, 4, 4}}),
        MakeSupport(16, F32, F32, {{8, 8, 4}}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),
        MakeConfig(F32, F32, {8, 8, 4}, {16, 16}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// Alternating smaller and larger multiple shapes across consecutive wave sizes (combining
// LargerWaveSizeIsMultipleOfSmallerWaveSize and SmallerWaveSizeIsMultipleOfLargerWaveSize).
TEST_F(SubgroupMatrixConfigTests, AlternatingMultipleShapesAcrossWaveSizes) {
    {
        // 4 -> 8x8x8, 8 -> 16x16x16, 16 -> 8x8x8:
        // 16x16x16 is supported at 4, 8, and 16, while 8x8x8 is only supported at 4 and 16.
        std::vector supports = {
            MakeSupport(4, F32, F32, {{8, 8, 8}}),
            MakeSupport(8, F32, F32, {{16, 16, 16}}),
            MakeSupport(16, F32, F32, {{8, 8, 8}}),
        };
        std::vector expected = {
            MakeConfig(F32, F32, {8, 8, 8}, {4, 4}),
            MakeConfig(F32, F32, {16, 16, 16}, {4, 16}),
            MakeConfig(F32, F32, {8, 8, 8}, {16, 16}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports);
        EXPECT_EQ(configs, expected);
    }
    {
        // 4 -> 16x16x16, 8 -> 8x8x8, 16 -> 16x16x16:
        // 16x16x16 is supported at 4, 8, and 16 (single merged entry), while 8x8x8 is only
        // supported at 8.
        std::vector supports = {
            MakeSupport(4, F32, F32, {{16, 16, 16}}),
            MakeSupport(8, F32, F32, {{8, 8, 8}}),
            MakeSupport(16, F32, F32, {{16, 16, 16}}),
        };
        std::vector expected = {
            MakeConfig(F32, F32, {16, 16, 16}, {4, 16}),
            MakeConfig(F32, F32, {8, 8, 8}, {8, 8}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports);
        EXPECT_EQ(configs, expected);
    }
    {
        // 4 -> 4x4x4, 8 -> 16x16x4, 16 -> 8x8x4:
        // 16x16x4 extends down to 4 (multiple of 4x4x4) and up to 16 (multiple of 8x8x4).
        std::vector supports = {
            MakeSupport(4, F32, F32, {{4, 4, 4}}),
            MakeSupport(8, F32, F32, {{16, 16, 4}}),
            MakeSupport(16, F32, F32, {{8, 8, 4}}),
        };
        std::vector expected = {
            MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),
            MakeConfig(F32, F32, {16, 16, 4}, {4, 16}),
            MakeConfig(F32, F32, {8, 8, 4}, {16, 16}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports);
        EXPECT_EQ(configs, expected);
    }
    {
        // 4 -> 4x4x4, 8 -> 8x8x4, 16 -> 4x4x4, 32 -> 16x16x4:
        // 8x8x4 extends down to 4 and up to 16, and 16x16x4 extends all the way down to 4.
        std::vector supports = {
            MakeSupport(4, F32, F32, {{4, 4, 4}}),
            MakeSupport(8, F32, F32, {{8, 8, 4}}),
            MakeSupport(16, F32, F32, {{4, 4, 4}}),
            MakeSupport(32, F32, F32, {{16, 16, 4}}),
        };
        std::vector expected = {
            MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),
            MakeConfig(F32, F32, {8, 8, 4}, {4, 16}),
            MakeConfig(F32, F32, {4, 4, 4}, {16, 16}),
            MakeConfig(F32, F32, {16, 16, 4}, {4, 32}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports);
        EXPECT_EQ(configs, expected);
    }
}

// Shapes where neither is a multiple of the other in all dimensions remain independent.
TEST_F(SubgroupMatrixConfigTests, NonMultipleShapesAreIndependent) {
    std::vector supports = {
        MakeSupport(4, F32, F32, {{4, 8, 4}}),
        MakeSupport(8, F32, F32, {{8, 4, 4}}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 8, 4}, {4, 4}),
        MakeConfig(F32, F32, {8, 4, 4}, {8, 8}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// Configs with different component types or result component types do not merge with each other.
TEST_F(SubgroupMatrixConfigTests, DifferentComponentTypesDoNotMerge) {
    std::vector supports = {
        MakeSupport(4, F32, F32, {{4, 4, 4}}),
        // Different componentType and resultComponentType (same shape):
        MakeSupport(8, I32, I32, {{4, 4, 4}}),
        // Different componentType, same resultComponentType (multiple shape):
        MakeSupport(8, F16, F32, {{8, 8, 4}}),
        // Same componentType, different resultComponentType (same shape and multiple shape):
        MakeSupport(16, F16, F16, {{8, 8, 4}}),
        MakeSupport(16, I32, U32, {{8, 8, 4}}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),   MakeConfig(I32, I32, {4, 4, 4}, {8, 8}),
        MakeConfig(F16, F32, {8, 8, 4}, {8, 8}),   MakeConfig(F16, F16, {8, 8, 4}, {16, 16}),
        MakeConfig(I32, U32, {8, 8, 4}, {16, 16}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(supports);
    EXPECT_EQ(configs, expected);
}

// Filtering rules for unsupported type combinations, ShaderF16, and WARP 8-bit types.
TEST_F(SubgroupMatrixConfigTests, TypeAndFeatureFiltering) {
    std::vector supports = {
        // Valid integer and float combinations:
        MakeSupport(32, I32, I32, {{16, 16, 16}}),
        MakeSupport(32, I8, I32, {{16, 16, 16}}),
        MakeSupport(32, U8, U32, {{16, 16, 16}}),
        MakeSupport(32, F16, F32, {{16, 16, 16}}),
        MakeSupport(32, F32, F32, {{16, 16, 16}}),
        // Invalid: mixed int/float:
        MakeSupport(32, I32, F32, {{16, 16, 16}}),
        MakeSupport(32, F16, I32, {{16, 16, 16}}),
        // Invalid: input byte size > accumulator byte size:
        MakeSupport(32, F32, F16, {{16, 16, 16}}),
        MakeSupport(32, I32, I8, {{16, 16, 16}}),
    };

    // Default (non-WARP, ShaderF16 supported):
    {
        std::vector expected = {
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(I8, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(U8, U32, {16, 16, 16}, {32, 32}),
            MakeConfig(F16, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(F32, F32, {16, 16, 16}, {32, 32}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports, /*vendorId=*/0,
                                                      /*deviceId=*/0,
                                                      /*supportsShaderF16=*/true);
        EXPECT_EQ(configs, expected);
    }

    // Without ShaderF16: float configs are filtered out.
    {
        std::vector expected = {
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(I8, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(U8, U32, {16, 16, 16}, {32, 32}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(supports, /*vendorId=*/0,
                                                      /*deviceId=*/0,
                                                      /*supportsShaderF16=*/false);
        EXPECT_EQ(configs, expected);
    }

    // On WARP: 8-bit configs are filtered out.
    {
        constexpr uint32_t kDeviceID_WARP = 0x8c;
        ASSERT_TRUE(gpu_info::IsMicrosoftWARP(gpu_info::kVendorID_Microsoft, kDeviceID_WARP));
        std::vector expected = {
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(F16, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(F32, F32, {16, 16, 16}, {32, 32}),
        };
        auto configs =
            EnumerateSubgroupMatrixConfigs(supports, gpu_info::kVendorID_Microsoft, kDeviceID_WARP,
                                           /*supportsShaderF16=*/true);
        EXPECT_EQ(configs, expected);
    }
}

}  // namespace
}  // namespace dawn::native::d3d12
