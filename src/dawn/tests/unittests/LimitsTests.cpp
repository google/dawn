// Copyright 2021 The Dawn Authors
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

#include <gtest/gtest.h>

#include "dawn/common/Constants.h"
#include "dawn/native/Limits.h"

namespace dawn {
namespace {

// Test |GetDefaultLimits| returns the default.
TEST(Limits, GetDefaultLimits) {
    native::Limits limits = {};
    EXPECT_NE(limits.maxBindGroups, 4u);

    native::GetDefaultLimits(&limits);

    EXPECT_EQ(limits.maxBindGroups, 4u);
}

// Test |ReifyDefaultLimits| populates the default if
// values are undefined.
TEST(Limits, ReifyDefaultLimits_PopulatesDefault) {
    native::Limits limits;
    limits.maxComputeWorkgroupStorageSize = wgpu::kLimitU32Undefined;
    limits.maxStorageBufferBindingSize = wgpu::kLimitU64Undefined;

    native::Limits reified = native::ReifyDefaultLimits(limits);
    EXPECT_EQ(reified.maxComputeWorkgroupStorageSize, 16384u);
    EXPECT_EQ(reified.maxStorageBufferBindingSize, 134217728ul);
}

// Test |ReifyDefaultLimits| clamps to the default if
// values are worse than the default.
TEST(Limits, ReifyDefaultLimits_Clamps) {
    native::Limits limits;
    limits.maxStorageBuffersPerShaderStage = 4;
    limits.minUniformBufferOffsetAlignment = 512;

    native::Limits reified = native::ReifyDefaultLimits(limits);
    EXPECT_EQ(reified.maxStorageBuffersPerShaderStage, 8u);
    EXPECT_EQ(reified.minUniformBufferOffsetAlignment, 256u);
}

// Test |ValidateLimits| works to validate limits are not better
// than supported.
TEST(Limits, ValidateLimits) {
    // Start with the default for supported.
    native::Limits defaults;
    native::GetDefaultLimits(&defaults);

    // Test supported == required is valid.
    {
        native::Limits required = defaults;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test supported == required is valid, when they are not default.
    {
        native::Limits supported = defaults;
        native::Limits required = defaults;
        supported.maxBindGroups += 1;
        required.maxBindGroups += 1;
        EXPECT_TRUE(ValidateLimits(supported, required).IsSuccess());
    }

    // Test that default-initialized (all undefined) is valid.
    {
        native::Limits required = {};
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test that better than supported is invalid for "maximum" limits.
    {
        native::Limits required = {};
        required.maxTextureDimension3D = defaults.maxTextureDimension3D + 1;
        native::MaybeError err = ValidateLimits(defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than supported is valid for "maximum" limits.
    {
        native::Limits required = {};
        required.maxComputeWorkgroupSizeX = defaults.maxComputeWorkgroupSizeX - 1;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test that better than min is invalid for "alignment" limits.
    {
        native::Limits required = {};
        required.minUniformBufferOffsetAlignment = defaults.minUniformBufferOffsetAlignment / 2;
        native::MaybeError err = ValidateLimits(defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than min and a power of two is valid for "alignment" limits.
    {
        native::Limits required = {};
        required.minStorageBufferOffsetAlignment = defaults.minStorageBufferOffsetAlignment * 2;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test that worse than min and not a power of two is invalid for "alignment" limits.
    {
        native::Limits required = {};
        required.minStorageBufferOffsetAlignment = defaults.minStorageBufferOffsetAlignment * 3;
        native::MaybeError err = ValidateLimits(defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that |ApplyLimitTiers| degrades limits to the next best tier.
TEST(Limits, ApplyLimitTiers) {
    auto SetLimitsStorageBufferBindingSizeTier2 = [](native::Limits* limits) {
        // Tier 2 of maxStorageBufferBindingSize is 1GB
        limits->maxStorageBufferBindingSize = 1073741824;
        // Also set the maxBufferSize to be large enough, as ApplyLimitTiers ensures tired
        // maxStorageBufferBindingSize no larger than tiered maxBufferSize.
        limits->maxBufferSize = 2147483648;
    };
    native::Limits limitsStorageBufferBindingSizeTier2;
    native::GetDefaultLimits(&limitsStorageBufferBindingSizeTier2);
    SetLimitsStorageBufferBindingSizeTier2(&limitsStorageBufferBindingSizeTier2);

    auto SetLimitsStorageBufferBindingSizeTier3 = [](native::Limits* limits) {
        // Tier 3 of maxStorageBufferBindingSize is 2GB-4
        limits->maxStorageBufferBindingSize = 2147483644;
        // Also set the maxBufferSize to be large enough, as ApplyLimitTiers ensures tired
        // maxStorageBufferBindingSize no larger than tiered maxBufferSize.
        limits->maxBufferSize = 2147483648;
    };
    native::Limits limitsStorageBufferBindingSizeTier3;
    native::GetDefaultLimits(&limitsStorageBufferBindingSizeTier3);
    SetLimitsStorageBufferBindingSizeTier3(&limitsStorageBufferBindingSizeTier3);

    auto SetLimitsComputeWorkgroupStorageSizeTier1 = [](native::Limits* limits) {
        limits->maxComputeWorkgroupStorageSize = 16384;
    };
    native::Limits limitsComputeWorkgroupStorageSizeTier1;
    native::GetDefaultLimits(&limitsComputeWorkgroupStorageSizeTier1);
    SetLimitsComputeWorkgroupStorageSizeTier1(&limitsComputeWorkgroupStorageSizeTier1);

    auto SetLimitsComputeWorkgroupStorageSizeTier3 = [](native::Limits* limits) {
        limits->maxComputeWorkgroupStorageSize = 65536;
    };
    native::Limits limitsComputeWorkgroupStorageSizeTier3;
    native::GetDefaultLimits(&limitsComputeWorkgroupStorageSizeTier3);
    SetLimitsComputeWorkgroupStorageSizeTier3(&limitsComputeWorkgroupStorageSizeTier3);

    // Test that applying tiers to limits that are exactly
    // equal to a tier returns the same values.
    {
        native::Limits limits = limitsStorageBufferBindingSizeTier2;
        EXPECT_EQ(ApplyLimitTiers(limits), limits);

        limits = limitsStorageBufferBindingSizeTier3;
        EXPECT_EQ(ApplyLimitTiers(limits), limits);
    }

    // Test all limits slightly worse than tier 3.
    {
        native::Limits limits = limitsStorageBufferBindingSizeTier3;
        limits.maxStorageBufferBindingSize -= 1;
        EXPECT_EQ(ApplyLimitTiers(limits), limitsStorageBufferBindingSizeTier2);
    }

    // Test that limits may match one tier exactly and be degraded in another tier.
    // Degrading to one tier does not affect the other tier.
    {
        native::Limits limits = limitsComputeWorkgroupStorageSizeTier3;
        // Set tier 3 and change one limit to be insufficent.
        SetLimitsStorageBufferBindingSizeTier3(&limits);
        limits.maxStorageBufferBindingSize -= 1;

        native::Limits tiered = ApplyLimitTiers(limits);

        // Check that |tiered| has the limits of memorySize tier 2
        native::Limits tieredWithMemorySizeTier2 = tiered;
        SetLimitsStorageBufferBindingSizeTier2(&tieredWithMemorySizeTier2);
        EXPECT_EQ(tiered, tieredWithMemorySizeTier2);

        // Check that |tiered| has the limits of bindingSpace tier 3
        native::Limits tieredWithBindingSpaceTier3 = tiered;
        SetLimitsComputeWorkgroupStorageSizeTier3(&tieredWithBindingSpaceTier3);
        EXPECT_EQ(tiered, tieredWithBindingSpaceTier3);
    }

    // Test that limits may be simultaneously degraded in two tiers independently.
    {
        native::Limits limits;
        native::GetDefaultLimits(&limits);
        SetLimitsComputeWorkgroupStorageSizeTier3(&limits);
        SetLimitsStorageBufferBindingSizeTier3(&limits);
        limits.maxComputeWorkgroupStorageSize =
            limitsComputeWorkgroupStorageSizeTier1.maxComputeWorkgroupStorageSize + 1;
        limits.maxStorageBufferBindingSize =
            limitsStorageBufferBindingSizeTier2.maxStorageBufferBindingSize + 1;

        native::Limits tiered = ApplyLimitTiers(limits);

        native::Limits expected = tiered;
        SetLimitsComputeWorkgroupStorageSizeTier1(&expected);
        SetLimitsStorageBufferBindingSizeTier2(&expected);
        EXPECT_EQ(tiered, expected);
    }
}

// Test that |ApplyLimitTiers| will hold the maxStorageBufferBindingSize no larger than
// maxBufferSize restriction.
TEST(Limits, TieredMaxStorageBufferBindingSizeNoLargerThanMaxBufferSize) {
    // Start with the default for supported.
    native::Limits defaults;
    native::GetDefaultLimits(&defaults);

    // Test reported maxStorageBufferBindingSize around 128MB, 1GB, 2GB-4 and 4GB-4.
    constexpr uint64_t storageSizeTier1 = 134217728ull;   // 128MB
    constexpr uint64_t storageSizeTier2 = 1073741824ull;  // 1GB
    constexpr uint64_t storageSizeTier3 = 2147483644ull;  // 2GB-4
    constexpr uint64_t storageSizeTier4 = 4294967292ull;  // 4GB-4
    constexpr uint64_t possibleReportedMaxStorageBufferBindingSizes[] = {
        storageSizeTier1,     storageSizeTier1 + 1, storageSizeTier2 - 1, storageSizeTier2,
        storageSizeTier2 + 1, storageSizeTier3 - 1, storageSizeTier3,     storageSizeTier3 + 1,
        storageSizeTier4 - 1, storageSizeTier4,     storageSizeTier4 + 1};
    // Test reported maxBufferSize around 256MB, 1GB, 2GB and 4GB, and a large 256GB.
    constexpr uint64_t bufferSizeTier1 = 0x10000000ull;    // 256MB
    constexpr uint64_t bufferSizeTier2 = 0x40000000ull;    // 1GB
    constexpr uint64_t bufferSizeTier3 = 0x80000000ull;    // 2GB
    constexpr uint64_t bufferSizeTier4 = 0x100000000ull;   // 4GB
    constexpr uint64_t bufferSizeLarge = 0x4000000000ull;  // 256GB
    constexpr uint64_t possibleReportedMaxBufferSizes[] = {
        bufferSizeTier1,     bufferSizeTier1 + 1, bufferSizeTier2 - 1, bufferSizeTier2,
        bufferSizeTier2 + 1, bufferSizeTier3 - 1, bufferSizeTier3,     bufferSizeTier3 + 1,
        bufferSizeTier4 - 1, bufferSizeTier4,     bufferSizeTier4 + 1, bufferSizeLarge};

    // Test that tiered maxStorageBufferBindingSize is no larger than tiered maxBufferSize.
    for (uint64_t reportedMaxStorageBufferBindingSizes :
         possibleReportedMaxStorageBufferBindingSizes) {
        for (uint64_t reportedMaxBufferSizes : possibleReportedMaxBufferSizes) {
            native::Limits limits = defaults;
            limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSizes;
            limits.maxBufferSize = reportedMaxBufferSizes;

            native::Limits tiered = ApplyLimitTiers(limits);

            EXPECT_LE(tiered.maxStorageBufferBindingSize, tiered.maxBufferSize);
        }
    }
}

// Test that |ApplyLimitTiers| will hold the maxUniformBufferBindingSize no larger than
// maxBufferSize restriction.
TEST(Limits, TieredMaxUniformBufferBindingSizeNoLargerThanMaxBufferSize) {
    // Start with the default for supported.
    native::Limits defaults;
    native::GetDefaultLimits(&defaults);

    // Test reported maxStorageBufferBindingSize around 64KB, and a large 1GB.
    constexpr uint64_t uniformSizeTier1 = 65536ull;       // 64KB
    constexpr uint64_t uniformSizeLarge = 1073741824ull;  // 1GB
    constexpr uint64_t possibleReportedMaxUniformBufferBindingSizes[] = {
        uniformSizeTier1, uniformSizeTier1 + 1, uniformSizeLarge};
    // Test reported maxBufferSize around 256MB, 1GB, 2GB and 4GB, and a large 256GB.
    constexpr uint64_t bufferSizeTier1 = 0x10000000ull;    // 256MB
    constexpr uint64_t bufferSizeTier2 = 0x40000000ull;    // 1GB
    constexpr uint64_t bufferSizeTier3 = 0x80000000ull;    // 2GB
    constexpr uint64_t bufferSizeTier4 = 0x100000000ull;   // 4GB
    constexpr uint64_t bufferSizeLarge = 0x4000000000ull;  // 256GB
    constexpr uint64_t possibleReportedMaxBufferSizes[] = {
        bufferSizeTier1,     bufferSizeTier1 + 1, bufferSizeTier2 - 1, bufferSizeTier2,
        bufferSizeTier2 + 1, bufferSizeTier3 - 1, bufferSizeTier3,     bufferSizeTier3 + 1,
        bufferSizeTier4 - 1, bufferSizeTier4,     bufferSizeTier4 + 1, bufferSizeLarge};

    // Test that tiered maxUniformBufferBindingSize is no larger than tiered maxBufferSize.
    for (uint64_t reportedMaxUniformBufferBindingSizes :
         possibleReportedMaxUniformBufferBindingSizes) {
        for (uint64_t reportedMaxBufferSizes : possibleReportedMaxBufferSizes) {
            native::Limits limits = defaults;
            limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSizes;
            limits.maxBufferSize = reportedMaxBufferSizes;

            native::Limits tiered = ApplyLimitTiers(limits);

            EXPECT_LE(tiered.maxUniformBufferBindingSize, tiered.maxBufferSize);
        }
    }
}

// Test |NormalizeLimits| works to enforce restriction of limits.
TEST(Limits, NormalizeLimits) {
    // Start with the default for supported.
    native::Limits defaults;
    native::GetDefaultLimits(&defaults);

    // Test specific limit values are clamped to internal Dawn constants.
    {
        native::Limits limits = defaults;
        limits.maxVertexBufferArrayStride = kMaxVertexBufferArrayStride + 1;
        limits.maxColorAttachments = uint32_t(kMaxColorAttachments) + 1;
        limits.maxBindGroups = kMaxBindGroups + 1;
        limits.maxVertexAttributes = uint32_t(kMaxVertexAttributes) + 1;
        limits.maxVertexBuffers = uint32_t(kMaxVertexBuffers) + 1;
        limits.maxInterStageShaderComponents = kMaxInterStageShaderComponents + 1;
        limits.maxSampledTexturesPerShaderStage = kMaxSampledTexturesPerShaderStage + 1;
        limits.maxSamplersPerShaderStage = kMaxSamplersPerShaderStage + 1;
        limits.maxStorageBuffersPerShaderStage = kMaxStorageBuffersPerShaderStage + 1;
        limits.maxStorageTexturesPerShaderStage = kMaxStorageTexturesPerShaderStage + 1;
        limits.maxUniformBuffersPerShaderStage = kMaxUniformBuffersPerShaderStage + 1;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxVertexBufferArrayStride, kMaxVertexBufferArrayStride);
        EXPECT_EQ(limits.maxColorAttachments, uint32_t(kMaxColorAttachments));
        EXPECT_EQ(limits.maxBindGroups, kMaxBindGroups);
        EXPECT_EQ(limits.maxVertexAttributes, uint32_t(kMaxVertexAttributes));
        EXPECT_EQ(limits.maxVertexBuffers, uint32_t(kMaxVertexBuffers));
        EXPECT_EQ(limits.maxInterStageShaderComponents, kMaxInterStageShaderComponents);
        EXPECT_EQ(limits.maxSampledTexturesPerShaderStage, kMaxSampledTexturesPerShaderStage);
        EXPECT_EQ(limits.maxSamplersPerShaderStage, kMaxSamplersPerShaderStage);
        EXPECT_EQ(limits.maxStorageBuffersPerShaderStage, kMaxStorageBuffersPerShaderStage);
        EXPECT_EQ(limits.maxStorageTexturesPerShaderStage, kMaxStorageTexturesPerShaderStage);
        EXPECT_EQ(limits.maxUniformBuffersPerShaderStage, kMaxUniformBuffersPerShaderStage);
    }

    // Test maxStorageBufferBindingSize is clamped to maxBufferSize.
    // maxStorageBufferBindingSize is no larger than maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize;
        native::Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxStorageBufferBindingSize);
    }
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize - 1;
        native::Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxStorageBufferBindingSize);
    }
    // maxStorageBufferBindingSize is equal to maxBufferSize+1, expect clamping to maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = reportedMaxBufferSize + 1;
        native::Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxBufferSize);
    }
    // maxStorageBufferBindingSize is much larger than maxBufferSize, expect clamping to
    // maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxStorageBufferBindingSize = 4294967295;
        native::Limits limits = defaults;
        limits.maxStorageBufferBindingSize = reportedMaxStorageBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxStorageBufferBindingSize, reportedMaxBufferSize);
    }

    // Test maxUniformBufferBindingSize is clamped to maxBufferSize.
    // maxUniformBufferBindingSize is no larger than maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize - 1;
        native::Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxUniformBufferBindingSize);
    }
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize;
        native::Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxUniformBufferBindingSize);
    }
    // maxUniformBufferBindingSize is larger than maxBufferSize, expect clamping to maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = reportedMaxBufferSize + 1;
        native::Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxBufferSize);
    }
    // maxUniformBufferBindingSize is much larger than maxBufferSize, expect clamping to
    // maxBufferSize
    {
        constexpr uint64_t reportedMaxBufferSize = 2147483648;
        constexpr uint64_t reportedMaxUniformBufferBindingSize = 4294967295;
        native::Limits limits = defaults;
        limits.maxUniformBufferBindingSize = reportedMaxUniformBufferBindingSize;
        limits.maxBufferSize = reportedMaxBufferSize;

        NormalizeLimits(&limits);

        EXPECT_EQ(limits.maxBufferSize, reportedMaxBufferSize);
        EXPECT_EQ(limits.maxUniformBufferBindingSize, reportedMaxBufferSize);
    }
}

}  // anonymous namespace
}  // namespace dawn
