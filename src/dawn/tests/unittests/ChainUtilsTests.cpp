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

#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/dawn_platform.h"

// Checks that we cannot find any structs in an empty chain
TEST(ChainUtilsTests, FindEmptyChain) {
    const dawn::native::PrimitiveDepthClipControl* info = nullptr;
    dawn::native::FindInChain(nullptr, &info);

    ASSERT_EQ(nullptr, info);
}

// Checks that searching a chain for a present struct returns that struct
TEST(ChainUtilsTests, FindPresentInChain) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    chain1.nextInChain = &chain2;
    const dawn::native::PrimitiveDepthClipControl* info1 = nullptr;
    const dawn::native::ShaderModuleSPIRVDescriptor* info2 = nullptr;
    dawn::native::FindInChain(&chain1, &info1);
    dawn::native::FindInChain(&chain1, &info2);

    ASSERT_NE(nullptr, info1);
    ASSERT_NE(nullptr, info2);
}

// Checks that searching a chain for a struct that doesn't exist returns a nullptr
TEST(ChainUtilsTests, FindMissingInChain) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    chain1.nextInChain = &chain2;
    const dawn::native::SurfaceDescriptorFromMetalLayer* info = nullptr;
    dawn::native::FindInChain(&chain1, &info);

    ASSERT_EQ(nullptr, info);
}

// Checks that validation rejects chains with duplicate STypes
TEST(ChainUtilsTests, ValidateDuplicateSTypes) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    dawn::native::PrimitiveDepthClipControl chain3;
    chain1.nextInChain = &chain2;
    chain2.nextInChain = &chain3;

    dawn::native::MaybeError result = dawn::native::ValidateSTypes(&chain1, {});
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that validation rejects chains that contain unspecified STypes
TEST(ChainUtilsTests, ValidateUnspecifiedSTypes) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    dawn::native::ShaderModuleWGSLDescriptor chain3;
    chain1.nextInChain = &chain2;
    chain2.nextInChain = &chain3;

    dawn::native::MaybeError result =
        dawn::native::ValidateSTypes(&chain1, {
                                                  {wgpu::SType::PrimitiveDepthClipControl},
                                                  {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                              });
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that validation rejects chains that contain multiple STypes from the same oneof
// constraint.
TEST(ChainUtilsTests, ValidateOneOfFailure) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    dawn::native::ShaderModuleWGSLDescriptor chain3;
    chain1.nextInChain = &chain2;
    chain2.nextInChain = &chain3;

    dawn::native::MaybeError result = dawn::native::ValidateSTypes(
        &chain1,
        {{wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor}});
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that validation accepts chains that match the constraints.
TEST(ChainUtilsTests, ValidateSuccess) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    chain1.nextInChain = &chain2;

    dawn::native::MaybeError result = dawn::native::ValidateSTypes(
        &chain1,
        {
            {wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor},
            {wgpu::SType::PrimitiveDepthClipControl},
            {wgpu::SType::SurfaceDescriptorFromMetalLayer},
        });
    ASSERT_TRUE(result.IsSuccess());
}

// Checks that validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateEmptyChain) {
    dawn::native::MaybeError result =
        dawn::native::ValidateSTypes(nullptr, {
                                                  {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                                  {wgpu::SType::PrimitiveDepthClipControl},
                                              });
    ASSERT_TRUE(result.IsSuccess());

    result = dawn::native::ValidateSTypes(nullptr, {});
    ASSERT_TRUE(result.IsSuccess());
}

// Checks that singleton validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateSingleEmptyChain) {
    dawn::native::MaybeError result =
        dawn::native::ValidateSingleSType(nullptr, wgpu::SType::ShaderModuleSPIRVDescriptor);
    ASSERT_TRUE(result.IsSuccess());

    result = dawn::native::ValidateSingleSType(nullptr, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                               wgpu::SType::PrimitiveDepthClipControl);
    ASSERT_TRUE(result.IsSuccess());
}

// Checks that singleton validation always fails on chains with multiple children.
TEST(ChainUtilsTests, ValidateSingleMultiChain) {
    dawn::native::PrimitiveDepthClipControl chain1;
    dawn::native::ShaderModuleSPIRVDescriptor chain2;
    chain1.nextInChain = &chain2;

    dawn::native::MaybeError result =
        dawn::native::ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl);
    ASSERT_TRUE(result.IsError());
    result.AcquireError();

    result = dawn::native::ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl,
                                               wgpu::SType::ShaderModuleSPIRVDescriptor);
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that singleton validation passes when the oneof constraint is met.
TEST(ChainUtilsTests, ValidateSingleSatisfied) {
    dawn::native::ShaderModuleWGSLDescriptor chain1;

    dawn::native::MaybeError result =
        dawn::native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
    ASSERT_TRUE(result.IsSuccess());

    result = dawn::native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                               wgpu::SType::ShaderModuleWGSLDescriptor);
    ASSERT_TRUE(result.IsSuccess());

    result = dawn::native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor,
                                               wgpu::SType::ShaderModuleSPIRVDescriptor);
    ASSERT_TRUE(result.IsSuccess());
}

// Checks that singleton validation passes when the oneof constraint is not met.
TEST(ChainUtilsTests, ValidateSingleUnsatisfied) {
    dawn::native::PrimitiveDepthClipControl chain1;

    dawn::native::MaybeError result =
        dawn::native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
    ASSERT_TRUE(result.IsError());
    result.AcquireError();

    result = dawn::native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                               wgpu::SType::ShaderModuleWGSLDescriptor);
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}
