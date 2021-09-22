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

#include "dawn_native/Limits.h"

// Test |GetDefaultLimits| returns the default.
TEST(Limits, GetDefaultLimits) {
    dawn_native::Limits limits = {};
    EXPECT_NE(limits.maxBindGroups, 4u);

    dawn_native::GetDefaultLimits(&limits);

    EXPECT_EQ(limits.maxBindGroups, 4u);
}

// Test |ReifyDefaultLimits| populates the default if
// values are undefined.
TEST(Limits, ReifyDefaultLimits_PopulatesDefault) {
    dawn_native::Limits limits;
    limits.maxComputeWorkgroupStorageSize = wgpu::kLimitU32Undefined;
    limits.maxStorageBufferBindingSize = wgpu::kLimitU64Undefined;

    dawn_native::Limits reified = dawn_native::ReifyDefaultLimits(limits);
    EXPECT_EQ(reified.maxComputeWorkgroupStorageSize, 16352u);
    EXPECT_EQ(reified.maxStorageBufferBindingSize, 134217728ul);
}

// Test |ReifyDefaultLimits| clamps to the default if
// values are worse than the default.
TEST(Limits, ReifyDefaultLimits_Clamps) {
    dawn_native::Limits limits;
    limits.maxStorageBuffersPerShaderStage = 4;
    limits.minUniformBufferOffsetAlignment = 512;

    dawn_native::Limits reified = dawn_native::ReifyDefaultLimits(limits);
    EXPECT_EQ(reified.maxStorageBuffersPerShaderStage, 8u);
    EXPECT_EQ(reified.minUniformBufferOffsetAlignment, 256u);
}

// Test |ValidateLimits| works to validate limits are not better
// than supported.
TEST(Limits, ValidateLimits) {
    // Start with the default for supported.
    dawn_native::Limits defaults;
    dawn_native::GetDefaultLimits(&defaults);

    // Test supported == required is valid.
    {
        dawn_native::Limits required = defaults;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test supported == required is valid, when they are not default.
    {
        dawn_native::Limits supported = defaults;
        dawn_native::Limits required = defaults;
        supported.maxBindGroups += 1;
        required.maxBindGroups += 1;
        EXPECT_TRUE(ValidateLimits(supported, required).IsSuccess());
    }

    // Test that default-initialized (all undefined) is valid.
    {
        dawn_native::Limits required = {};
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test that better than max is invalid.
    {
        dawn_native::Limits required = {};
        required.maxTextureDimension3D = defaults.maxTextureDimension3D + 1;
        dawn_native::MaybeError err = ValidateLimits(defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than max is valid.
    {
        dawn_native::Limits required = {};
        required.maxComputeWorkgroupSizeX = defaults.maxComputeWorkgroupSizeX - 1;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }

    // Test that better than min is invalid.
    {
        dawn_native::Limits required = {};
        required.minUniformBufferOffsetAlignment = defaults.minUniformBufferOffsetAlignment / 2;
        dawn_native::MaybeError err = ValidateLimits(defaults, required);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }

    // Test that worse than min is valid.
    {
        dawn_native::Limits required = {};
        required.minStorageBufferOffsetAlignment = defaults.minStorageBufferOffsetAlignment * 2;
        EXPECT_TRUE(ValidateLimits(defaults, required).IsSuccess());
    }
}
