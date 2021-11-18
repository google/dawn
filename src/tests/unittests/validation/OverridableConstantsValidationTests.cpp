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

#include "common/Constants.h"
#include "tests/unittests/validation/ValidationTest.h"
#include "utils/WGPUHelpers.h"

class ComputePipelineOverridableConstantsValidationTest : public ValidationTest {
  protected:
    void SetUpShadersWithDefaultValueConstants() {
        computeModule = utils::CreateShaderModule(device, R"(
[[override]] let c0: bool = true;      // type: bool
[[override]] let c1: bool = false;      // default override
[[override]] let c2: f32 = 0.0;         // type: float32
[[override]] let c3: f32 = 0.0;         // default override
[[override]] let c4: f32 = 4.0;         // default
[[override]] let c5: i32 = 0;           // type: int32
[[override]] let c6: i32 = 0;           // default override
[[override]] let c7: i32 = 7;           // default
[[override]] let c8: u32 = 0u;          // type: uint32
[[override]] let c9: u32 = 0u;          // default override
[[override(1000)]] let c10: u32 = 10u;  // default

[[stage(compute), workgroup_size(1)]] fn main() {
    // make sure the overridable constants are not optimized out
    _ = u32(c0);
    _ = u32(c1);
    _ = u32(c2);
    _ = u32(c3);
    _ = u32(c4);
    _ = u32(c5);
    _ = u32(c6);
    _ = u32(c7);
    _ = u32(c8);
    _ = u32(c9);
    _ = u32(c10);
})");
    }

    void SetUpShadersWithUninitializedConstants() {
        computeModule = utils::CreateShaderModule(device, R"(
[[override]] let c0: bool;              // type: bool
[[override]] let c1: bool = false;      // default override
[[override]] let c2: f32;               // type: float32
[[override]] let c3: f32 = 0.0;         // default override
[[override]] let c4: f32 = 4.0;         // default
[[override]] let c5: i32;               // type: int32
[[override]] let c6: i32 = 0;           // default override
[[override]] let c7: i32 = 7;           // default
[[override]] let c8: u32;               // type: uint32
[[override]] let c9: u32 = 0u;          // default override
[[override(1000)]] let c10: u32 = 10u;  // default

[[stage(compute), workgroup_size(1)]] fn main() {
    // make sure the overridable constants are not optimized out
    _ = u32(c0);
    _ = u32(c1);
    _ = u32(c2);
    _ = u32(c3);
    _ = u32(c4);
    _ = u32(c5);
    _ = u32(c6);
    _ = u32(c7);
    _ = u32(c8);
    _ = u32(c9);
    _ = u32(c10);
})");
    }

    void TestCreatePipeline(const std::vector<wgpu::ConstantEntry>& constants) {
        wgpu::ComputePipelineDescriptor csDesc;
        csDesc.compute.module = computeModule;
        csDesc.compute.entryPoint = "main";
        csDesc.compute.constants = constants.data();
        csDesc.compute.constantCount = constants.size();
        wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);
    }

    wgpu::ShaderModule computeModule;
    wgpu::Buffer buffer;
};

// Basic constants lookup tests
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierLookUp) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Valid: no constants specified
        std::vector<wgpu::ConstantEntry> constants;
        TestCreatePipeline(constants);
    }
    {
        // Valid: find by constant name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c0", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: set the same constant twice
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0},
            {nullptr, "c0", 1},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: find by constant numeric id
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "1000", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: constant numeric id not specified
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "9999", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant name doesn't exit
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c99", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that it is invalid to leave any constants uninitialized
TEST_F(ComputePipelineOverridableConstantsValidationTest, UninitializedConstants) {
    SetUpShadersWithUninitializedConstants();
    {
        // Error: uninitialized constants exist
        std::vector<wgpu::ConstantEntry> constants;
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: uninitialized constants exist
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", false},
            {nullptr, "c2", 1},
            // c5 is missing
            {nullptr, "c8", 1},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Valid: all constants initialized
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", false},
            {nullptr, "c2", 1},
            {nullptr, "c5", 1},
            {nullptr, "c8", 1},
        };
        TestCreatePipeline(constants);
    }
    {
        // Error: duplicate initializations
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", false}, {nullptr, "c2", 1}, {nullptr, "c5", 1},
            {nullptr, "c8", 1},     {nullptr, "c2", 2},
        };
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that only explicitly specified numeric ID can be referenced
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierExplicitNumericID) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "0", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "1", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "2", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
    {
        // Error: constant numeric id not explicitly specified
        // But could be impliciltly assigned to one of the constants
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "3", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}

// Test that identifiers are unique
TEST_F(ComputePipelineOverridableConstantsValidationTest, ConstantsIdentifierUnique) {
    SetUpShadersWithDefaultValueConstants();
    {
        // Valid: constant without numeric id can be referenced with variable name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c0", 0}};
        TestCreatePipeline(constants);
    }
    {
        // Error: constant with numeric id cannot be referenced with variable name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c10", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}