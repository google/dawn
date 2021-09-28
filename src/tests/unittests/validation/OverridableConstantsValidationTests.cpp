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
    void SetUp() override {
        ValidationTest::SetUp();

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

[[block]] struct Buf {
    data : array<u32, 11>;
};

[[group(0), binding(0)]] var<storage, read_write> buf : Buf;

[[stage(compute), workgroup_size(1)]] fn main() {
    // make sure the overridable constants are not optimized out
    buf.data[0] = u32(c0);
    buf.data[1] = u32(c1);
    buf.data[2] = u32(c2);
    buf.data[3] = u32(c3);
    buf.data[4] = u32(c4);
    buf.data[5] = u32(c5);
    buf.data[6] = u32(c6);
    buf.data[7] = u32(c7);
    buf.data[8] = u32(c8);
    buf.data[9] = u32(c9);
    buf.data[10] = u32(c10);
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
        // Valid: set the same constant twice
        std::vector<wgpu::ConstantEntry> constants{
            {nullptr, "c0", 0},
            {nullptr, "c0", 1},
        };
        TestCreatePipeline(constants);
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

// Test that only explicitly specified numeric ID can be referenced
// TODO(tint:1155): missing feature in tint to differentiate explicitly specified numeric ID
TEST_F(ComputePipelineOverridableConstantsValidationTest,
       DISABLED_ConstantsIdentifierExplicitNumericID) {
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
// TODO(tint:1155): missing feature in tint to differentiate explicitly specified numeric ID
TEST_F(ComputePipelineOverridableConstantsValidationTest, DISABLED_ConstantsIdentifierUnique) {
    {
        // Error: constant with numeric id cannot be referenced with variable name
        std::vector<wgpu::ConstantEntry> constants{{nullptr, "c10", 0}};
        ASSERT_DEVICE_ERROR(TestCreatePipeline(constants));
    }
}