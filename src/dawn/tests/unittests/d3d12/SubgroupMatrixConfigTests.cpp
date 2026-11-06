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

using ::testing::Test;

using enum wgpu::SubgroupMatrixComponentType;

class SubgroupMatrixConfigTests : public Test {
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

    D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_ENUMERATION_ENTRY MakeEntry(
        SubgroupSizeRange waveSizeRange,
        D3D12_LINEAR_ALGEBRA_DATATYPE typeA,
        D3D12_LINEAR_ALGEBRA_DATATYPE typeB,
        D3D12_LINEAR_ALGEBRA_DATATYPE typeAcc,
        Shape shape,
        D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAGS flags =
            D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED) {
        D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_ENUMERATION_ENTRY entry{};
        entry.MinWaveSize = waveSizeRange.min;
        entry.MaxWaveSize = waveSizeRange.max;
        entry.MatrixAComponentType = typeA;
        entry.MatrixBComponentType = typeB;
        entry.AccumulatorComponentType = typeAcc;
        entry.SupportFlags = flags;
        entry.Shape.M = shape.M;
        entry.Shape.N = shape.N;
        entry.Shape.K = shape.K;
        return entry;
    }

    D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_ENUMERATION_ENTRY MakeEntry(
        SubgroupSizeRange waveSizeRange,
        wgpu::SubgroupMatrixComponentType typeAB,
        wgpu::SubgroupMatrixComponentType typeAcc,
        Shape shape) {
        return MakeEntry(waveSizeRange, ToD3D12(typeAB), ToD3D12(typeAB), ToD3D12(typeAcc), shape);
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
        std::span<const D3D12_LINEAR_ALGEBRA_WAVE_MATRIX_MULTIPLY_ENUMERATION_ENTRY> entries,
        uint32_t vendorId = 0,
        uint32_t deviceId = 0,
        bool supportsShaderF16 = true) {
        return PhysicalDevice::EnumerateSubgroupMatrixConfigs(entries, vendorId, deviceId,
                                                              supportsShaderF16);
    }
};

// Each enumeration entry maps to one config, with the entry's wave size range as the subgroup size
// range.
TEST_F(SubgroupMatrixConfigTests, EntriesMapToConfigs) {
    std::vector entries = {
        MakeEntry({4, 4}, F32, F32, {4, 4, 4}),
        MakeEntry({4, 8}, F32, F32, {8, 8, 4}),
        MakeEntry({16, 32}, F16, F16, {8, 16, 16}),
        MakeEntry({32, 64}, I32, U32, {16, 16, 16}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 4, 4}, {4, 4}),
        MakeConfig(F32, F32, {8, 8, 4}, {4, 8}),
        MakeConfig(F16, F16, {8, 16, 16}, {16, 32}),
        MakeConfig(I32, U32, {16, 16, 16}, {32, 64}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(entries);
    EXPECT_EQ(configs, expected);
}

// The result is sorted and free of duplicates regardless of the enumeration order.
TEST_F(SubgroupMatrixConfigTests, ConfigsAreSortedAndDeduplicated) {
    std::vector entries = {
        // Grouped by shape first, as some drivers enumerate.
        MakeEntry({4, 8}, I32, I32, {8, 8, 4}),
        MakeEntry({4, 4}, I32, I32, {4, 4, 4}),
        MakeEntry({4, 8}, F32, F32, {8, 8, 4}),
        MakeEntry({4, 4}, F32, F32, {4, 4, 4}),
        // Exact duplicate of the previous entry, differing only in the support flags.
        MakeEntry({4, 4}, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
                  {4, 4, 4},
                  D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_SUPPORTED |
                      D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_EMULATED_OUTPUTS),
        // Same types and shape but a different subgroup size range is not a duplicate.
        MakeEntry({16, 32}, F32, F32, {4, 4, 4}),
    };
    std::vector expected = {
        MakeConfig(F32, F32, {4, 4, 4}, {4, 4}), MakeConfig(F32, F32, {4, 4, 4}, {16, 32}),
        MakeConfig(F32, F32, {8, 8, 4}, {4, 8}), MakeConfig(I32, I32, {4, 4, 4}, {4, 4}),
        MakeConfig(I32, I32, {8, 8, 4}, {4, 8}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(entries);
    EXPECT_EQ(configs, expected);
}

// Entries without the SUPPORTED flag, with different A and B types, or with component types that
// WebGPU does not expose are skipped.
TEST_F(SubgroupMatrixConfigTests, UnsupportedEntriesAreSkipped) {
    std::vector entries = {
        MakeEntry({16, 32}, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
                  {16, 16, 16}, D3D12_LINEAR_ALGEBRA_MULTIPLICATION_SUPPORT_FLAG_NONE),
        MakeEntry({16, 32}, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT16,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32,
                  {16, 16, 16}),
        MakeEntry({16, 32}, D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_SINT16, D3D12_LINEAR_ALGEBRA_DATATYPE_SINT32,
                  {16, 16, 16}),
        MakeEntry({16, 32}, D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT8_E4M3FN,
                  D3D12_LINEAR_ALGEBRA_DATATYPE_FLOAT32, {16, 16, 16}),
        MakeEntry({16, 32}, F16, F32, {16, 16, 16}),
    };
    std::vector expected = {
        MakeConfig(F16, F32, {16, 16, 16}, {16, 32}),
    };
    auto configs = EnumerateSubgroupMatrixConfigs(entries);
    EXPECT_EQ(configs, expected);
}

// Filtering rules for unsupported type combinations, ShaderF16, and WARP 8-bit types.
TEST_F(SubgroupMatrixConfigTests, TypeAndFeatureFiltering) {
    std::vector entries = {
        // Valid integer and float combinations:
        MakeEntry({32, 32}, I32, I32, {16, 16, 16}),
        MakeEntry({32, 32}, I8, I32, {16, 16, 16}),
        MakeEntry({32, 32}, U8, U32, {16, 16, 16}),
        MakeEntry({32, 32}, F16, F32, {16, 16, 16}),
        MakeEntry({32, 32}, F32, F32, {16, 16, 16}),
        // Invalid: mixed int/float:
        MakeEntry({32, 32}, I32, F32, {16, 16, 16}),
        MakeEntry({32, 32}, F16, I32, {16, 16, 16}),
        // Invalid: input byte size > accumulator byte size:
        MakeEntry({32, 32}, F32, F16, {16, 16, 16}),
        MakeEntry({32, 32}, I32, I8, {16, 16, 16}),
    };

    // Default (non-WARP, ShaderF16 supported):
    {
        std::vector expected = {
            MakeConfig(F32, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(F16, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(U8, U32, {16, 16, 16}, {32, 32}),
            MakeConfig(I8, I32, {16, 16, 16}, {32, 32}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(entries, /*vendorId=*/0,
                                                      /*deviceId=*/0,
                                                      /*supportsShaderF16=*/true);
        EXPECT_EQ(configs, expected);
    }

    // Without ShaderF16: float configs are filtered out.
    {
        std::vector expected = {
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
            MakeConfig(U8, U32, {16, 16, 16}, {32, 32}),
            MakeConfig(I8, I32, {16, 16, 16}, {32, 32}),
        };
        auto configs = EnumerateSubgroupMatrixConfigs(entries, /*vendorId=*/0,
                                                      /*deviceId=*/0,
                                                      /*supportsShaderF16=*/false);
        EXPECT_EQ(configs, expected);
    }

    // On WARP: 8-bit configs are filtered out.
    {
        constexpr uint32_t kDeviceID_WARP = 0x8c;
        ASSERT_TRUE(gpu_info::IsMicrosoftWARP(gpu_info::kVendorID_Microsoft, kDeviceID_WARP));
        std::vector expected = {
            MakeConfig(F32, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(F16, F32, {16, 16, 16}, {32, 32}),
            MakeConfig(I32, I32, {16, 16, 16}, {32, 32}),
        };
        auto configs =
            EnumerateSubgroupMatrixConfigs(entries, gpu_info::kVendorID_Microsoft, kDeviceID_WARP,
                                           /*supportsShaderF16=*/true);
        EXPECT_EQ(configs, expected);
    }
}

}  // namespace
}  // namespace dawn::native::d3d12
