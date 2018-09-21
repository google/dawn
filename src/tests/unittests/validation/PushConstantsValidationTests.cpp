// Copyright 2017 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "common/Constants.h"
#include "utils/DawnHelpers.h"

#include <gmock/gmock.h>

using namespace testing;

class PushConstantTest : public ValidationTest {
    protected:
        dawn::Queue queue;
        uint32_t constants[kMaxPushConstants] = {0};

        void TestCreateShaderModule(bool success, std::string vertexSource) {
            dawn::ShaderModule module;
            if (success) {
                module = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vertexSource.c_str());
            } else {
                ASSERT_DEVICE_ERROR(module = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, vertexSource.c_str()));
            }
        }

    private:
        void SetUp() override {
            ValidationTest::SetUp();
            queue = device.CreateQueue();
        }
};

// Test valid usage of the parameters to SetPushConstants
TEST_F(PushConstantTest, Success) {
    DummyRenderPass renderpassData = CreateDummyRenderPass();

    dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
    // PushConstants in a compute pass
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, constants);
        pass.EndPass();
    }

    // PushConstants in a render pass
    {
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpassData.renderPass);
        pass.SetPushConstants(dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment, 0, 1, constants);
        pass.EndPass();
    }

    // Setting all constants
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, kMaxPushConstants, constants);
        pass.EndPass();
    }

    // Setting constants at an offset
    {
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, kMaxPushConstants - 1, 1, constants);
        pass.EndPass();
    }

    builder.GetResult();
}

// Test check for constants being set out of bounds
TEST_F(PushConstantTest, SetPushConstantsOOB) {
    uint32_t constants[kMaxPushConstants] = {0};
    
    // Control case: setting all constants
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, kMaxPushConstants, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // OOB because count is too big.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, kMaxPushConstants + 1, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // OOB because of the offset.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 1, kMaxPushConstants, constants);
        pass.EndPass();
        builder.GetResult();
    }
}

// Test valid stages for compute pass
TEST_F(PushConstantTest, StageForComputePass) {
    // Control case: setting to the compute stage in compute passes
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // Graphics stages are disallowed
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::Vertex, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // A None shader stage mask is valid.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetPushConstants(dawn::ShaderStageBit::None, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }
}

// Test valid stages for render passes
TEST_F(PushConstantTest, StageForRenderPass) {
    DummyRenderPass renderpassData = CreateDummyRenderPass();

    // Control case: setting to vertex and fragment in render pass
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpassData.renderPass);
        pass.SetPushConstants(dawn::ShaderStageBit::Vertex | dawn::ShaderStageBit::Fragment, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // Compute stage is disallowed
    {
        dawn::CommandBufferBuilder builder = AssertWillBeError(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpassData.renderPass);
        pass.SetPushConstants(dawn::ShaderStageBit::Compute, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }

    // A None shader stage mask is valid.
    {
        dawn::CommandBufferBuilder builder = AssertWillBeSuccess(device.CreateCommandBufferBuilder());
        dawn::RenderPassEncoder pass = builder.BeginRenderPass(renderpassData.renderPass);
        pass.SetPushConstants(dawn::ShaderStageBit::None, 0, 1, constants);
        pass.EndPass();
        builder.GetResult();
    }
}

// Valid shaders that use pushconstants
TEST_F(PushConstantTest, ShaderCompilationSuccess) {
    // Test shader module not using any push constants
    TestCreateShaderModule(true, R"(
        #version 450
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test one push constant
    TestCreateShaderModule(true, R"(
        #version 450
        layout(push_constant) uniform ConstantsBlock {
            float a;
        } c;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test one push constant with an offset
    TestCreateShaderModule(true, R"(
        #version 450
        layout(push_constant) uniform ConstantsBlock {
            float a;
        } c;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test max push constants
    TestCreateShaderModule(true, R"(
        #version 450
        layout(push_constant) uniform ConstantsBlock {
            float a[)" + std::to_string(kMaxPushConstants) + R"(];
        } c;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}

// Test that shaders using a push constant block too big fail compilation
// TODO(cwallez@chromium.org): Currently disabled because ShaderModule error handling needs refactoring
TEST_F(PushConstantTest, DISABLED_ShaderCompilationOOB) {
    // Test one push constant over the max
    TestCreateShaderModule(false, R"(
        #version 450
        layout(push_constant) uniform ConstantsBlock {
            float a[)" + std::to_string(kMaxPushConstants + 1) + R"(];
        } c;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");

    // Test two variables in the push constant block that together overflow
    TestCreateShaderModule(false, R"(
        #version 450
        layout(push_constant) uniform ConstantsBlock {
            float a[)" + std::to_string(kMaxPushConstants) + R"(];
            float b;
        } c;
        void main() {
            gl_Position = vec4(0.0);
        }
    )");
}
