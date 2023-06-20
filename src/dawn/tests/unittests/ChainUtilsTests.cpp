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

#include "dawn/native/ChainUtils.h"
#include "dawn/native/dawn_platform.h"

namespace dawn {
namespace {

// Checks that we cannot find any structs in an empty chain
TEST(ChainUtilsTests, FindEmptyChain) {
    {
        const native::PrimitiveDepthClipControl* info = nullptr;
        const native::ChainedStruct* chained = nullptr;
        native::FindInChain(chained, &info);

        ASSERT_EQ(nullptr, info);
    }

    {
        native::DawnAdapterPropertiesPowerPreference* info = nullptr;
        native::ChainedStructOut* chained = nullptr;
        native::FindInChain(chained, &info);

        ASSERT_EQ(nullptr, info);
    }
}

// Checks that searching a chain for a present struct returns that struct
TEST(ChainUtilsTests, FindPresentInChain) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;
        const native::PrimitiveDepthClipControl* info1 = nullptr;
        const native::ShaderModuleSPIRVDescriptor* info2 = nullptr;
        native::FindInChain(&chain1, &info1);
        native::FindInChain(&chain1, &info2);

        ASSERT_NE(nullptr, info1);
        ASSERT_NE(nullptr, info2);
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain;
        native::DawnAdapterPropertiesPowerPreference* output = nullptr;
        native::FindInChain(&chain, &output);

        ASSERT_NE(nullptr, output);
    }
}

// Checks that searching a chain for a struct that doesn't exist returns a nullptr
TEST(ChainUtilsTests, FindMissingInChain) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;
        const native::SurfaceDescriptorFromMetalLayer* info = nullptr;
        native::FindInChain(&chain1, &info);

        ASSERT_EQ(nullptr, info);
    }

    {
        native::AdapterProperties adapterProperties;
        native::DawnAdapterPropertiesPowerPreference* output = nullptr;
        native::FindInChain(adapterProperties.nextInChain, &output);

        ASSERT_EQ(nullptr, output);
    }
}

// Checks that validation rejects chains with duplicate STypes
TEST(ChainUtilsTests, ValidateDuplicateSTypes) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        native::PrimitiveDepthClipControl chain3;
        chain1.nextInChain = &chain2;
        chain2.nextInChain = &chain3;

        native::MaybeError result = native::ValidateSTypes(&chain1, {});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain1;
        native::DawnAdapterPropertiesPowerPreference chain2;
        chain1.nextInChain = &chain2;

        native::MaybeError result = native::ValidateSTypes(&chain1, {});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that validation rejects chains that contain unspecified STypes
TEST(ChainUtilsTests, ValidateUnspecifiedSTypes) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        native::ShaderModuleWGSLDescriptor chain3;
        chain1.nextInChain = &chain2;
        chain2.nextInChain = &chain3;

        native::MaybeError result =
            native::ValidateSTypes(&chain1, {
                                                {wgpu::SType::PrimitiveDepthClipControl},
                                                {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                            });
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain1;
        native::ChainedStructOut chain2;
        chain2.sType = wgpu::SType::RenderPassDescriptorMaxDrawCount;
        chain1.nextInChain = &chain2;

        native::MaybeError result =
            native::ValidateSTypes(&chain1, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that validation rejects chains that contain multiple STypes from the same oneof
// constraint.
TEST(ChainUtilsTests, ValidateOneOfFailure) {
    native::PrimitiveDepthClipControl chain1;
    native::ShaderModuleSPIRVDescriptor chain2;
    native::ShaderModuleWGSLDescriptor chain3;
    chain1.nextInChain = &chain2;
    chain2.nextInChain = &chain3;

    native::MaybeError result = native::ValidateSTypes(
        &chain1,
        {{wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor}});
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that validation accepts chains that match the constraints.
TEST(ChainUtilsTests, ValidateSuccess) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;

        native::MaybeError result = native::ValidateSTypes(
            &chain1,
            {
                {wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor},
                {wgpu::SType::PrimitiveDepthClipControl},
                {wgpu::SType::SurfaceDescriptorFromMetalLayer},
            });
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain1;
        native::MaybeError result =
            native::ValidateSTypes(&chain1, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateEmptyChain) {
    {
        const native::ChainedStruct* chain = nullptr;
        native::MaybeError result =
            native::ValidateSTypes(chain, {
                                              {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                              {wgpu::SType::PrimitiveDepthClipControl},
                                          });
        ASSERT_TRUE(result.IsSuccess());

        result = native::ValidateSTypes(chain, {});
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        native::ChainedStructOut* chain = nullptr;
        native::MaybeError result =
            native::ValidateSTypes(chain, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsSuccess());

        result = native::ValidateSTypes(chain, {});
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateSingleEmptyChain) {
    {
        const native::ChainedStruct* chain = nullptr;
        native::MaybeError result =
            native::ValidateSingleSType(chain, wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = native::ValidateSingleSType(chain, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                             wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        native::ChainedStructOut* chain = nullptr;
        native::MaybeError result =
            native::ValidateSingleSType(chain, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsSuccess());

        result =
            native::ValidateSingleSType(chain, wgpu::SType::DawnAdapterPropertiesPowerPreference,
                                        wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation always fails on chains with multiple children.
TEST(ChainUtilsTests, ValidateSingleMultiChain) {
    {
        native::PrimitiveDepthClipControl chain1;
        native::ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;

        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();

        result = native::ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl,
                                             wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain1;
        native::DawnAdapterPropertiesPowerPreference chain2;
        chain1.nextInChain = &chain2;

        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that singleton validation passes when the one of constraint is met.
TEST(ChainUtilsTests, ValidateSingleSatisfied) {
    {
        native::ShaderModuleWGSLDescriptor chain1;

        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                             wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor,
                                             wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        native::DawnAdapterPropertiesPowerPreference chain1;
        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation passes when the oneof constraint is not met.
TEST(ChainUtilsTests, ValidateSingleUnsatisfied) {
    {
        native::PrimitiveDepthClipControl chain1;

        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();

        result = native::ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                             wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        native::ChainedStructOut chain1;
        chain1.sType = wgpu::SType::ShaderModuleWGSLDescriptor;

        native::MaybeError result =
            native::ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

}  // anonymous namespace
}  // namespace dawn
