// Copyright 2025 The Dawn & Tint Authors
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

#include <cstdint>
#include <map>
#include <string>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboLimits.h"

// The purpose of these tests is to prevent regressions of features and limits on architectures that
// are known to have these supported capabilities.
//
// Why? Because of the nature of feature/limit support in WebGPU we can easily accidentally drop
// (regress) optional capabilities for specific devices and dawn/CTS testing machinery and even
// external webgpu clients will respond gracefully. We (dawn) never want to silently regress
// features on critical platforms. If we are regressing it should be a deliberate decision which may
// involve changes to the tests below.
//
// How? Currently it is difficult to know our features/limit support for all hardware (could even be
// driver version dependent). So here we conservatively assert that specific architectures have
// specific features/limits.
//
// When? This file may need to be updated with new conservative tests if the CQ changes hardware or
// if we have deliberately chosen to regress specific features/limits on some devices.
//
// Not an official dawn/webgpu resource but this page can provide insights into features/limits:
// https://web3dsurvey.com/webgpu

namespace dawn {
namespace {

using TierArchInfoTestBase = DawnTestWithParams<>;

class TierArchInfoTest_TieredMaxLimits : public TierArchInfoTestBase {
  protected:
    bool GetRequireUseTieredLimits() override { return true; }
    void GetRequiredLimits(const dawn::utils::ComboLimits& supported,
                           dawn::utils::ComboLimits& required) override {
        supported.UnlinkedCopyTo(&required);
    }
};

#define ENUM_LIST_LIMITS                                           \
    /*                   field name                  */            \
    ENUM_LIMIT_PROPERTY(maxTextureDimension1D)                     \
    ENUM_LIMIT_PROPERTY(maxTextureDimension2D)                     \
    ENUM_LIMIT_PROPERTY(maxTextureDimension3D)                     \
    ENUM_LIMIT_PROPERTY(maxTextureArrayLayers)                     \
    ENUM_LIMIT_PROPERTY(maxBindGroups)                             \
    ENUM_LIMIT_PROPERTY(maxBindGroupsPlusVertexBuffers)            \
    ENUM_LIMIT_PROPERTY(maxBindingsPerBindGroup)                   \
    ENUM_LIMIT_PROPERTY(maxDynamicUniformBuffersPerPipelineLayout) \
    ENUM_LIMIT_PROPERTY(maxDynamicStorageBuffersPerPipelineLayout) \
    ENUM_LIMIT_PROPERTY(maxSampledTexturesPerShaderStage)          \
    ENUM_LIMIT_PROPERTY(maxSamplersPerShaderStage)                 \
    ENUM_LIMIT_PROPERTY(maxStorageBuffersPerShaderStage)           \
    ENUM_LIMIT_PROPERTY(maxStorageTexturesPerShaderStage)          \
    ENUM_LIMIT_PROPERTY(maxUniformBuffersPerShaderStage)           \
    ENUM_LIMIT_PROPERTY(maxUniformBufferBindingSize)               \
    ENUM_LIMIT_PROPERTY(maxStorageBufferBindingSize)               \
    ENUM_LIMIT_PROPERTY(minUniformBufferOffsetAlignment)           \
    ENUM_LIMIT_PROPERTY(minStorageBufferOffsetAlignment)           \
    ENUM_LIMIT_PROPERTY(maxVertexBuffers)                          \
    ENUM_LIMIT_PROPERTY(maxBufferSize)                             \
    ENUM_LIMIT_PROPERTY(maxVertexAttributes)                       \
    ENUM_LIMIT_PROPERTY(maxVertexBufferArrayStride)                \
    ENUM_LIMIT_PROPERTY(maxInterStageShaderVariables)              \
    ENUM_LIMIT_PROPERTY(maxColorAttachments)                       \
    ENUM_LIMIT_PROPERTY(maxColorAttachmentBytesPerSample)          \
    ENUM_LIMIT_PROPERTY(maxComputeWorkgroupStorageSize)            \
    ENUM_LIMIT_PROPERTY(maxComputeInvocationsPerWorkgroup)         \
    ENUM_LIMIT_PROPERTY(maxComputeWorkgroupSizeX)                  \
    ENUM_LIMIT_PROPERTY(maxComputeWorkgroupSizeY)                  \
    ENUM_LIMIT_PROPERTY(maxComputeWorkgroupSizeZ)                  \
    ENUM_LIMIT_PROPERTY(maxComputeWorkgroupsPerDimension)          \
    ENUM_LIMIT_PROPERTY(maxImmediateSize)                          \
    ENUM_LIMIT_PROPERTY(maxStorageBuffersInVertexStage)            \
    ENUM_LIMIT_PROPERTY(maxStorageTexturesInVertexStage)           \
    ENUM_LIMIT_PROPERTY(maxStorageBuffersInFragmentStage)          \
    ENUM_LIMIT_PROPERTY(maxStorageTexturesInFragmentStage)

static const int kNumElements =
#define ENUM_LIMIT_PROPERTY(fieldName) 1 +
    ENUM_LIST_LIMITS + 0;

#undef ENUM_LIMIT_PROPERTY

static_assert(
    kNumElements == utils::ComboLimits::kMemberCount,
    "Combo limits structure has a different number of limits than the above ENUM_LIST_LIMITS");

TEST_P(TierArchInfoTest_TieredMaxLimits, ExhaustiveTestAllFields) {
    // SwiftShader will return a lower limit than any modern device on CQ.
    DAWN_TEST_UNSUPPORTED_IF(IsSwiftshader());

    std::stringstream param_names_all;
    param_names_all << this->GetParam();
    DawnTestBase::PrintToStringParamName param_printer{""};
    std::string full_param =
        param_printer.SanitizeParamName(param_names_all.str(), this->GetAdapterProperties(), 0);

    std::map<std::string, std::array<uint64_t, kNumElements>> device_map;
    // clang-format off
device_map["D3D12_NVIDIA_GeForce_GTX_1660"]                 = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Vulkan_llvmpipe__LLVM_19_1_7__256_bits"]        = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 30, 2048, 28, 8,  32, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__Google__Vulkan_1_3_0__SwiftShader_Device__Subzero___0x0000C0DE____SwiftShader_driver_5_0_0__compat"]
                                                            = {   8192,  8192, 2048,  256, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 16384, 1073741824, 256, 256, 8, 2147483648, 16, 2048, 16, 4,  32, 32768,  256,  256,  256, 64, 65535,  0, 10, 8, 10, 8,};
device_map["Vulkan_NVIDIA_GeForce_GTX_1660"]                = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 4294967292, 256, 256, 8, 4294967296, 30, 2048, 28, 8,  32, 49152, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_Adreno__TM__640_compat"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000,  8, 4, 16, 16,  8, 4, 12, 65536,  536870912, 256, 256, 8, 2147483648, 30, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535,  0,  0, 4,  4, 4,};
device_map["D3D11_Microsoft_Basic_Render_Driver_Integrated_GPU"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["D3D12_Microsoft_Basic_Render_Driver_Integrated_GPU"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Metal_Intel_R__UHD_Graphics_630"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Metal_AMD_Radeon_Pro_5300M"]                    = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["D3D11_NVIDIA_GeForce_GTX_1660"]                 = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["D3D12_Intel_R__UHD_Graphics_630"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["D3D11_Intel_R__UHD_Graphics_630"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__Intel__Intel_R__UHD_Graphics_630__0x00009BC5__Direct3D11_vs_5_0_ps_5_0__D3D11_31_0_101_2127__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535,  0, 10, 8, 10, 8,};
device_map["OpenGLES_Mali_G78_compat"]                      = {   8192,  8192, 2048,  256, 4, 24, 1000,  8, 4, 16, 16,  8, 4, 12, 65536,  268435456, 256, 256, 8, 2147483648, 30, 2048, 16, 8,  32, 32768,  256,  256,  256, 64, 65535,  0,  0, 0,  4, 4,};
device_map["OpenGLES_ANGLE__NVIDIA__NVIDIA_GeForce_GTX_1660__0x00002184__Direct3D11_vs_5_0_ps_5_0__D3D11_31_0_15_4601__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535,  0, 10, 8, 10, 8,};
device_map["Vulkan_Intel_R__UHD_Graphics_630"]              = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  536870912, 256, 256, 8, 4294967296, 30, 2048, 28, 8,  32, 32768,  256,  256,  256, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__Intel__Intel_R__UHD_Graphics_630__0x00009BC5__Direct3D11_vs_5_0_ps_5_0__D3D11_31_0_101_2127__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535,  0, 10, 8, 10, 8,};
device_map["Vulkan_Intel_R__UHD_Graphics_770__ADL_S_GT1"]   = {  16384, 16384, 2048, 2048, 4, 24, 1000,  8, 4, 16, 16, 10, 4, 12, 65536, 4294967292, 256, 256, 8, 4294967296, 16, 2048, 28, 8,  32, 65536, 1024, 1024, 1024, 64, 65535, 32, 10, 4, 10, 4,};
device_map["Metal_AMD_Radeon_Pro_555X"]                     = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 0, 10, 8, 10, 8,};
device_map["D3D11_Intel_R__UHD_Graphics_770"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__Intel__Intel_R__UHD_Graphics_770__0x00004680__Direct3D11_vs_5_0_ps_5_0__D3D11_31_0_101_5333__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535, 0, 10, 8, 10, 8,};
device_map["Vulkan_Intel_R__UHD_Graphics_770"]              = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  536870912, 256, 256, 8, 4294967296, 30, 2048, 28, 8,  32, 32768,  256,  256,  256, 64, 65535, 32, 10, 8, 10, 8,};
device_map["D3D12_Intel_R__UHD_Graphics_770"]               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__Intel__Intel_R__UHD_Graphics_770__0x00004680__Direct3D11_vs_5_0_ps_5_0__D3D11_31_0_101_5333__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535, 0, 10, 8, 10, 8,};
device_map["OpenGLES_ANGLE__NVIDIA__NVIDIA_GeForce_GTX_1660__0x00002184__Direct3D11_vs_5_0_ps_5_0__D3D11_32_0_15_7602__compat"]
                                                            = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  134217728, 256, 256, 8, 2147483648, 16, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535,  0, 10, 8, 10, 8,};
device_map["Vulkan_Adreno__TM__640"]                        = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  536870912, 256, 256, 8, 1073741824, 30, 2048, 16, 8,  32, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Vulkan_Mali_G78"]                               = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536,  268435456, 256, 256, 8, 4294967296, 30, 2048, 28, 8,  32, 32768,  256,  256,  256, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Vulkan_GeForce_GTX_1660"]                       = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 2147483644, 256, 256, 8, 2147483648, 30, 2048, 28, 8,  32, 49152, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
device_map["Vulkan_Intel_R__UHD_Graphics_630__CML_GT2"]     = {  16384, 16384, 2048, 2048, 4, 24, 1000,  8, 4, 16, 16, 10, 4, 12, 65536, 4294967292, 256, 256, 8, 4294967296, 16, 2048, 28, 8,  32, 65536,  256,  256,  256, 64, 65535, 32, 10, 4, 10, 4,};
device_map["Metal_Apple_M2"]                                = {  16384, 16384, 2048, 2048, 4, 24, 1000, 10, 8, 16, 16, 10, 8, 12, 65536, 4294967292, 256, 256, 8, 4294967296, 30, 2048, 28, 8, 128, 32768, 1024, 1024, 1024, 64, 65535, 32, 10, 8, 10, 8,};
    // clang-format on
    auto& supported_limits = GetSupportedLimits();

    bool encountered_error = false;
    auto iter_curr_limits = device_map.find(full_param);
    if (iter_curr_limits == device_map.end()) {
        // CQ bots will pass in '--test-launcher-bot-mode' as a command line parameter.
        // In this case, we didn't find any results, so we want to make sure we report an error as
        // this is a new CQ bot and we need to update the  test results.
        if (IsTestLauncherBotMode()) {
            encountered_error = true;
        } else {
            // Skipping the test when the device params does not match limits us to only testing
            // known CQ (and try) bots.
            GTEST_SKIP();
        }
    }

    int curr_idx = 0;
#define ENUM_LIMIT_PROPERTY(fieldName)                                                           \
    if (!encountered_error &&                                                                    \
        supported_limits.fieldName != static_cast<decltype(supported_limits.fieldName)>(         \
                                          (iter_curr_limits->second)[curr_idx])) {               \
        EXPECT_EQ(supported_limits.fieldName, static_cast<decltype(supported_limits.fieldName)>( \
                                                  (iter_curr_limits->second)[curr_idx]));        \
        /*Report first error only*/                                                              \
        encountered_error = true;                                                                \
    }                                                                                            \
    curr_idx++;

    ENUM_LIST_LIMITS
#undef ENUM_LIMIT_PROPERTY

    if (encountered_error) {
        std::string expected_str =
            "\n Mismatch found! The full set of correct limits for device are:\n "
            "device_map[\"" +
            full_param + "\"] = { " +
#define ENUM_LIMIT_PROPERTY(fieldName) " " + std::to_string(supported_limits.fieldName) + "," +
            ENUM_LIST_LIMITS
#undef ENUM_LIMIT_PROPERTY
            "};";
        SCOPED_TRACE(expected_str);
        EXPECT_FALSE(encountered_error);
    }
}

DAWN_INSTANTIATE_TEST(TierArchInfoTest_TieredMaxLimits,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
