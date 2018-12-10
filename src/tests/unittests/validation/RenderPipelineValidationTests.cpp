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
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/DawnHelpers.h"

class RenderPipelineValidationTest : public ValidationTest {
    protected:
        void SetUp() override {
            ValidationTest::SetUp();

            renderpass = CreateSimpleRenderPass();

            vsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Vertex, R"(
                #version 450
                void main() {
                    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
                })"
            );

            fsModule = utils::CreateShaderModule(device, dawn::ShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })");
        }

        dawn::RenderPassDescriptor renderpass;
        dawn::ShaderModule vsModule;
        dawn::ShaderModule fsModule;
};

// Test cases where creation should succeed
TEST_F(RenderPipelineValidationTest, CreationSuccess) {
    utils::ComboRenderPipelineDescriptor descriptor(device);
    descriptor.cVertexStage.module = vsModule;
    descriptor.cFragmentStage.module = fsModule;

    device.CreateRenderPipeline(&descriptor);
}

TEST_F(RenderPipelineValidationTest, BlendState) {

    {
        // This one succeeds because attachment 0 is the color attachment
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.cVertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.numBlendStates = 1;

        device.CreateRenderPipeline(&descriptor);
    }

    {   // Fail because lack of blend states for color attachments
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.cVertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.numBlendStates = 0;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }

    {
        // Fail because set blend states for empty color attachments
        utils::ComboRenderPipelineDescriptor descriptor(device);
        descriptor.cVertexStage.module = vsModule;
        descriptor.cFragmentStage.module = fsModule;
        descriptor.numBlendStates = 2;

        ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&descriptor));
    }
}

