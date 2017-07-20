// Copyright 2017 The NXT Authors
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
#include "utils/NXTHelpers.h"

#include <gmock/gmock.h>

using namespace testing;

class PushConstantTest : public ValidationTest {
    protected:
        nxt::Queue queue;
        uint32_t constants[kMaxPushConstants] = {0};

        void TestCreateShaderModule(bool success, std::string vertexSource) {
            nxt::ShaderModuleBuilder builder;
            if (success) {
                builder = AssertWillBeSuccess(device.CreateShaderModuleBuilder());
            } else {
                builder = AssertWillBeError(device.CreateShaderModuleBuilder());
            }
            utils::FillShaderModuleBuilder(builder, nxt::ShaderStage::Vertex, vertexSource.c_str());
            builder.GetResult();
        }

    private:
        void SetUp() override {
            ValidationTest::SetUp();
            queue = device.CreateQueueBuilder().GetResult();
        }
};

// Test valid usage of the parameters to SetPushConstants
TEST_F(PushConstantTest, Success) {
    DummyRenderPass renderpassData = CreateDummyRenderPass();

    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        // PushConstants in a compute pass
        .BeginComputePass()
        .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, constants)
        .EndComputePass()

        // PushConstants in a render subpass
        .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
        .BeginRenderSubpass()
        .SetPushConstants(nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment, 0, 1, constants)
        .EndRenderSubpass()
        .EndRenderPass()

        // Setting all constants
        .BeginComputePass()
        .SetPushConstants(nxt::ShaderStageBit::Compute, 0, kMaxPushConstants, constants)
        .EndComputePass()

        // Setting constants at an offset
        .BeginComputePass()
        .SetPushConstants(nxt::ShaderStageBit::Compute, kMaxPushConstants - 1, 1, constants)
        .EndComputePass()

        .GetResult();
}

// Test check for constants being set out of bounds
TEST_F(PushConstantTest, SetPushConstantsOOB) {
    uint32_t constants[kMaxPushConstants] = {0};
    
    // Control case: setting all constants
    {
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, kMaxPushConstants, constants)
            .EndComputePass()
            .GetResult();
    }

    // OOB because count is too big.
    {
        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, kMaxPushConstants + 1, constants)
            .EndComputePass()
            .GetResult();
    }

    // OOB because of the offset.
    {
        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 1, kMaxPushConstants, constants)
            .EndComputePass()
            .GetResult();
    }
}

// Test which places push constants can be set
TEST_F(PushConstantTest, NotInPass) {
    DummyRenderPass renderpassData = CreateDummyRenderPass();

    // Setting outside of any pass is invalid.
    {
        AssertWillBeError(device.CreateCommandBufferBuilder())
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, constants)
            .GetResult();

        AssertWillBeError(device.CreateCommandBufferBuilder())
            .SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 1, constants)
            .GetResult();
    }

    // Setting in renderpass but outside subpass is invalid
    {
        // Control to check the error is caused by the SetPushConstants
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .BeginRenderSubpass()
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();

        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 1, constants)
            .BeginRenderSubpass()
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();

        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .BeginRenderSubpass()
            .EndRenderSubpass()
            .SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 1, constants)
            .EndRenderPass()
            .GetResult();
    }
}

// Test valid stages for subpass
TEST_F(PushConstantTest, StageForComputePass) {
    // Control case: setting to the compute stage in compute passes
    {
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, constants)
            .EndComputePass()
            .GetResult();
    }

    // Graphics stages are disallowed
    {
        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::Vertex, 0, 1, constants)
            .EndComputePass()
            .GetResult();
    }

    // A None shader stage mask is valid.
    {
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginComputePass()
            .SetPushConstants(nxt::ShaderStageBit::None, 0, 1, constants)
            .EndComputePass()
            .GetResult();
    }
}

// Test valid stages for compute passes
TEST_F(PushConstantTest, StageForSubpass) {
    DummyRenderPass renderpassData = CreateDummyRenderPass();

    // Control case: setting to vertex and fragment in subpass
    {
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .BeginRenderSubpass()
            .SetPushConstants(nxt::ShaderStageBit::Vertex | nxt::ShaderStageBit::Fragment, 0, 1, constants)
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();
    }

    // Compute stage is disallowed
    {
        AssertWillBeError(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .BeginRenderSubpass()
            .SetPushConstants(nxt::ShaderStageBit::Compute, 0, 1, constants)
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();
    }

    // A None shader stage mask is valid.
    {
        AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .BeginRenderPass(renderpassData.renderPass, renderpassData.framebuffer)
            .BeginRenderSubpass()
            .SetPushConstants(nxt::ShaderStageBit::None, 0, 1, constants)
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();
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
TEST_F(PushConstantTest, ShaderCompilationOOB_DISABLED) {
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
