// Copyright 2021 The Dawn & Tint Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dawn/native/ChainUtils.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {
namespace {

using ::testing::HasSubstr;

// Checks that we cannot find any structs in an empty chain
TEST(ChainUtilsTests, FindEmptyChain) {
    {
        const PrimitiveDepthClipControl* info = nullptr;
        const ChainedStruct* chained = nullptr;
        FindInChain(chained, &info);

        ASSERT_EQ(nullptr, info);
    }

    {
        DawnAdapterPropertiesPowerPreference* info = nullptr;
        ChainedStructOut* chained = nullptr;
        FindInChain(chained, &info);

        ASSERT_EQ(nullptr, info);
    }
}

// Checks that searching a chain for a present struct returns that struct
TEST(ChainUtilsTests, FindPresentInChain) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;
        const PrimitiveDepthClipControl* info1 = nullptr;
        const ShaderModuleSPIRVDescriptor* info2 = nullptr;
        FindInChain(&chain1, &info1);
        FindInChain(&chain1, &info2);

        ASSERT_NE(nullptr, info1);
        ASSERT_NE(nullptr, info2);
    }

    {
        DawnAdapterPropertiesPowerPreference chain;
        DawnAdapterPropertiesPowerPreference* output = nullptr;
        FindInChain(&chain, &output);

        ASSERT_NE(nullptr, output);
    }
}

// Checks that searching a chain for a struct that doesn't exist returns a nullptr
TEST(ChainUtilsTests, FindMissingInChain) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;
        const SurfaceDescriptorFromMetalLayer* info = nullptr;
        FindInChain(&chain1, &info);

        ASSERT_EQ(nullptr, info);
    }

    {
        AdapterProperties adapterProperties;
        DawnAdapterPropertiesPowerPreference* output = nullptr;
        FindInChain(adapterProperties.nextInChain, &output);

        ASSERT_EQ(nullptr, output);
    }
}

// Checks that validation rejects chains with duplicate STypes
TEST(ChainUtilsTests, ValidateDuplicateSTypes) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        PrimitiveDepthClipControl chain3;
        chain1.nextInChain = &chain2;
        chain2.nextInChain = &chain3;

        MaybeError result = ValidateSTypes(&chain1, {});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        DawnAdapterPropertiesPowerPreference chain1;
        DawnAdapterPropertiesPowerPreference chain2;
        chain1.nextInChain = &chain2;

        MaybeError result = ValidateSTypes(&chain1, {});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that validation rejects chains that contain unspecified STypes
TEST(ChainUtilsTests, ValidateUnspecifiedSTypes) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        ShaderModuleWGSLDescriptor chain3;
        chain1.nextInChain = &chain2;
        chain2.nextInChain = &chain3;

        MaybeError result = ValidateSTypes(&chain1, {
                                                        {wgpu::SType::PrimitiveDepthClipControl},
                                                        {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                                    });
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        DawnAdapterPropertiesPowerPreference chain1;
        ChainedStructOut chain2;
        chain2.sType = wgpu::SType::RenderPassDescriptorMaxDrawCount;
        chain1.nextInChain = &chain2;

        MaybeError result =
            ValidateSTypes(&chain1, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that validation rejects chains that contain multiple STypes from the same oneof
// constraint.
TEST(ChainUtilsTests, ValidateOneOfFailure) {
    PrimitiveDepthClipControl chain1;
    ShaderModuleSPIRVDescriptor chain2;
    ShaderModuleWGSLDescriptor chain3;
    chain1.nextInChain = &chain2;
    chain2.nextInChain = &chain3;

    MaybeError result = ValidateSTypes(&chain1, {{wgpu::SType::ShaderModuleSPIRVDescriptor,
                                                  wgpu::SType::ShaderModuleWGSLDescriptor}});
    ASSERT_TRUE(result.IsError());
    result.AcquireError();
}

// Checks that validation accepts chains that match the constraints.
TEST(ChainUtilsTests, ValidateSuccess) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;

        MaybeError result = ValidateSTypes(
            &chain1,
            {
                {wgpu::SType::ShaderModuleSPIRVDescriptor, wgpu::SType::ShaderModuleWGSLDescriptor},
                {wgpu::SType::PrimitiveDepthClipControl},
                {wgpu::SType::SurfaceDescriptorFromMetalLayer},
            });
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        DawnAdapterPropertiesPowerPreference chain1;
        MaybeError result =
            ValidateSTypes(&chain1, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateEmptyChain) {
    {
        const ChainedStruct* chain = nullptr;
        MaybeError result = ValidateSTypes(chain, {
                                                      {wgpu::SType::ShaderModuleSPIRVDescriptor},
                                                      {wgpu::SType::PrimitiveDepthClipControl},
                                                  });
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSTypes(chain, {});
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        ChainedStructOut* chain = nullptr;
        MaybeError result =
            ValidateSTypes(chain, {{wgpu::SType::DawnAdapterPropertiesPowerPreference}});
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSTypes(chain, {});
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation always passes on empty chains.
TEST(ChainUtilsTests, ValidateSingleEmptyChain) {
    {
        const ChainedStruct* chain = nullptr;
        MaybeError result = ValidateSingleSType(chain, wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSingleSType(chain, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                     wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        ChainedStructOut* chain = nullptr;
        MaybeError result =
            ValidateSingleSType(chain, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSingleSType(chain, wgpu::SType::DawnAdapterPropertiesPowerPreference,
                                     wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation always fails on chains with multiple children.
TEST(ChainUtilsTests, ValidateSingleMultiChain) {
    {
        PrimitiveDepthClipControl chain1;
        ShaderModuleSPIRVDescriptor chain2;
        chain1.nextInChain = &chain2;

        MaybeError result = ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();

        result = ValidateSingleSType(&chain1, wgpu::SType::PrimitiveDepthClipControl,
                                     wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        DawnAdapterPropertiesPowerPreference chain1;
        DawnAdapterPropertiesPowerPreference chain2;
        chain1.nextInChain = &chain2;

        MaybeError result =
            ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Checks that singleton validation passes when the one of constraint is met.
TEST(ChainUtilsTests, ValidateSingleSatisfied) {
    {
        ShaderModuleWGSLDescriptor chain1;

        MaybeError result = ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                     wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsSuccess());

        result = ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor,
                                     wgpu::SType::ShaderModuleSPIRVDescriptor);
        ASSERT_TRUE(result.IsSuccess());
    }

    {
        DawnAdapterPropertiesPowerPreference chain1;
        MaybeError result =
            ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsSuccess());
    }
}

// Checks that singleton validation passes when the oneof constraint is not met.
TEST(ChainUtilsTests, ValidateSingleUnsatisfied) {
    {
        PrimitiveDepthClipControl chain1;

        MaybeError result = ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();

        result = ValidateSingleSType(&chain1, wgpu::SType::ShaderModuleSPIRVDescriptor,
                                     wgpu::SType::ShaderModuleWGSLDescriptor);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }

    {
        ChainedStructOut chain1;
        chain1.sType = wgpu::SType::ShaderModuleWGSLDescriptor;

        MaybeError result =
            ValidateSingleSType(&chain1, wgpu::SType::DawnAdapterPropertiesPowerPreference);
        ASSERT_TRUE(result.IsError());
        result.AcquireError();
    }
}

// Empty chain on roots that have and don't have valid extensions should not fail validation and all
// values should be nullptr.
TEST(ChainUtilsTests, ValidateAndUnpackEmpty) {
    {
        // TextureViewDescriptor (as of when this test was written) does not have any valid chains
        // in the JSON nor via additional extensions.
        TextureViewDescriptor desc;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        static_assert(std::tuple_size_v<decltype(unpacked)> == 0);
        std::apply(
            [](const auto*... args) {
                (([&](const auto* arg) { EXPECT_EQ(args, nullptr); }(args)), ...);
            },
            unpacked);
    }
    {
        // InstanceDescriptor has at least 1 valid chain extension.
        InstanceDescriptor desc;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        std::apply(
            [](const auto*... args) {
                (([&](const auto* arg) { EXPECT_EQ(args, nullptr); }(args)), ...);
            },
            unpacked);
    }
}

// Invalid chain extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackUnexpected) {
    {
        // TextureViewDescriptor (as of when this test was written) does not have any valid chains
        // in the JSON nor via additional extensions.
        TextureViewDescriptor desc;
        ChainedStruct chain;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpackChain(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("Unexpected"));
    }
    {
        // InstanceDescriptor has at least 1 valid chain extension.
        InstanceDescriptor desc;
        ChainedStruct chain;
        desc.nextInChain = &chain;
        EXPECT_THAT(ValidateAndUnpackChain(&desc).AcquireError()->GetFormattedMessage(),
                    HasSubstr("Unexpected"));
    }
}

// Nominal unpacking valid descriptors should return the expected descriptors in the unpacked type.
TEST(ChainUtilsTests, ValidateAndUnpack) {
    // DawnTogglesDescriptor is a valid extension for InstanceDescriptor.
    InstanceDescriptor desc;
    DawnTogglesDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
    EXPECT_EQ(std::get<const DawnTogglesDescriptor*>(unpacked), &chain);
}

// Duplicate valid extensions cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackDuplicate) {
    // DawnTogglesDescriptor is a valid extension for InstanceDescriptor.
    InstanceDescriptor desc;
    DawnTogglesDescriptor chain1;
    DawnTogglesDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    EXPECT_THAT(ValidateAndUnpackChain(&desc).AcquireError()->GetFormattedMessage(),
                HasSubstr("Duplicate"));
}

// Additional extensions added via template specialization and not specified in the JSON unpack
// properly.
TEST(ChainUtilsTests, ValidateAndUnpackAdditionalExtensions) {
    // DawnInstanceDescriptor is an extension on InstanceDescriptor added in ChainUtilsImpl.inl.
    InstanceDescriptor desc;
    DawnInstanceDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
    EXPECT_EQ(std::get<const DawnInstanceDescriptor*>(unpacked), &chain);
}

// Duplicate additional extensions added via template specialization should cause an error.
TEST(ChainUtilsTests, ValidateAndUnpackDuplicateAdditionalExtensions) {
    // DawnInstanceDescriptor is an extension on InstanceDescriptor added in ChainUtilsImpl.inl.
    InstanceDescriptor desc;
    DawnInstanceDescriptor chain1;
    DawnInstanceDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    EXPECT_THAT(ValidateAndUnpackChain(&desc).AcquireError()->GetFormattedMessage(),
                HasSubstr("Duplicate"));
}

using NoExtensionBranches =
    BranchList<Branch<ShaderModuleWGSLDescriptor>, Branch<ShaderModuleSPIRVDescriptor>>;
using ExtensionBranches =
    BranchList<Branch<ShaderModuleWGSLDescriptor>,
               Branch<ShaderModuleSPIRVDescriptor, DawnShaderModuleSPIRVOptionsDescriptor>>;

// Validates exacly 1 branch and ensures that there are no other extensions.
TEST(ChainUtilsTests, ValidateBranchesOneValidBranch) {
    ShaderModuleDescriptor desc;
    // Either allowed branches should validate successfully and return the expected enum.
    {
        ShaderModuleWGSLDescriptor chain;
        desc.nextInChain = &chain;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        EXPECT_EQ((ValidateBranches<NoExtensionBranches>(unpacked).AcquireSuccess()),
                  wgpu::SType::ShaderModuleWGSLDescriptor);
    }
    {
        ShaderModuleSPIRVDescriptor chain;
        desc.nextInChain = &chain;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        EXPECT_EQ((ValidateBranches<NoExtensionBranches>(unpacked).AcquireSuccess()),
                  wgpu::SType::ShaderModuleSPIRVDescriptor);

        // Extensions are optional so validation should still pass when the extension is not
        // provided.
        EXPECT_EQ((ValidateBranches<ExtensionBranches>(unpacked).AcquireSuccess()),
                  wgpu::SType::ShaderModuleSPIRVDescriptor);
    }
}

// An allowed chain that is not one of the branches causes an error.
TEST(ChainUtilsTests, ValidateBranchesInvalidBranch) {
    ShaderModuleDescriptor desc;
    DawnShaderModuleSPIRVOptionsDescriptor chain;
    desc.nextInChain = &chain;
    auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
    EXPECT_NE((ValidateBranches<NoExtensionBranches>(unpacked).AcquireError()), nullptr);
    EXPECT_NE((ValidateBranches<ExtensionBranches>(unpacked).AcquireError()), nullptr);
}

// Additional chains should cause an error when branches don't allow extensions.
TEST(ChainUtilsTests, ValidateBranchesInvalidExtension) {
    ShaderModuleDescriptor desc;
    {
        ShaderModuleWGSLDescriptor chain1;
        DawnShaderModuleSPIRVOptionsDescriptor chain2;
        desc.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        EXPECT_NE((ValidateBranches<NoExtensionBranches>(unpacked).AcquireError()), nullptr);
        EXPECT_NE((ValidateBranches<ExtensionBranches>(unpacked).AcquireError()), nullptr);
    }
    {
        ShaderModuleSPIRVDescriptor chain1;
        DawnShaderModuleSPIRVOptionsDescriptor chain2;
        desc.nextInChain = &chain1;
        chain1.nextInChain = &chain2;
        auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
        EXPECT_NE((ValidateBranches<NoExtensionBranches>(unpacked).AcquireError()), nullptr);
    }
}

// Branches that allow extensions pass successfully.
TEST(ChainUtilsTests, ValidateBranchesAllowedExtensions) {
    ShaderModuleDescriptor desc;
    ShaderModuleSPIRVDescriptor chain1;
    DawnShaderModuleSPIRVOptionsDescriptor chain2;
    desc.nextInChain = &chain1;
    chain1.nextInChain = &chain2;
    auto unpacked = ValidateAndUnpackChain(&desc).AcquireSuccess();
    EXPECT_EQ((ValidateBranches<ExtensionBranches>(unpacked).AcquireSuccess()),
              wgpu::SType::ShaderModuleSPIRVDescriptor);
}

}  // anonymous namespace
}  // namespace dawn::native
