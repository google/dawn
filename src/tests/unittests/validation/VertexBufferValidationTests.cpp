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

#include <array>

#include "tests/unittests/validation/ValidationTest.h"

#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class VertexBufferValidationTest : public ValidationTest {
    protected:
        void SetUp() override {
            ValidationTest::SetUp();

            fsModule = utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, R"(
                #version 450
                layout(location = 0) out vec4 fragColor;
                void main() {
                    fragColor = vec4(0.0, 1.0, 0.0, 1.0);
                })");
        }

        wgpu::Buffer MakeVertexBuffer() {
            wgpu::BufferDescriptor descriptor;
            descriptor.size = 256;
            descriptor.usage = wgpu::BufferUsage::Vertex;

            return device.CreateBuffer(&descriptor);
        }

        wgpu::ShaderModule MakeVertexShader(unsigned int bufferCount) {
            std::ostringstream vs;
            vs << "#version 450\n";
            for (unsigned int i = 0; i < bufferCount; ++i) {
                vs << "layout(location = " << i << ") in vec3 a_position" << i << ";\n";
            }
            vs << "void main() {\n";

            vs << "gl_Position = vec4(";
            for (unsigned int i = 0; i < bufferCount; ++i) {
                vs << "a_position" << i;
                if (i != bufferCount - 1) {
                    vs << " + ";
                }
            }
            vs << ", 1.0);";

            vs << "}\n";

            return utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex,
                                             vs.str().c_str());
        }

        wgpu::RenderPipeline MakeRenderPipeline(const wgpu::ShaderModule& vsModule,
                                                unsigned int bufferCount) {
            utils::ComboRenderPipelineDescriptor descriptor(device);
            descriptor.vertexStage.module = vsModule;
            descriptor.cFragmentStage.module = fsModule;

            for (unsigned int i = 0; i < bufferCount; ++i) {
                descriptor.cVertexState.cVertexBuffers[i].attributeCount = 1;
                descriptor.cVertexState.cVertexBuffers[i].attributes =
                    &descriptor.cVertexState.cAttributes[i];
                descriptor.cVertexState.cAttributes[i].shaderLocation = i;
                descriptor.cVertexState.cAttributes[i].format = wgpu::VertexFormat::Float3;
            }
            descriptor.cVertexState.vertexBufferCount = bufferCount;

            return device.CreateRenderPipeline(&descriptor);
        }

        wgpu::ShaderModule fsModule;
};

TEST_F(VertexBufferValidationTest, VertexBuffersInheritedBetweenPipelines) {
    DummyRenderPass renderPass(device);
    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto pipeline2 = MakeRenderPipeline(vsModule2, 2);
    auto pipeline1 = MakeRenderPipeline(vsModule1, 1);

    auto vertexBuffer1 = MakeVertexBuffer();
    auto vertexBuffer2 = MakeVertexBuffer();

    // Check failure when vertex buffer is not set
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    ASSERT_DEVICE_ERROR(encoder.Finish());

    // Check success when vertex buffer is inherited from previous pipeline
    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline2);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.SetVertexBuffer(1, vertexBuffer2);
        pass.Draw(3, 1, 0, 0);
        pass.SetPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    encoder.Finish();
}

TEST_F(VertexBufferValidationTest, VertexBuffersNotInheritedBetweenRendePasses) {
    DummyRenderPass renderPass(device);
    auto vsModule2 = MakeVertexShader(2);
    auto vsModule1 = MakeVertexShader(1);

    auto pipeline2 = MakeRenderPipeline(vsModule2, 2);
    auto pipeline1 = MakeRenderPipeline(vsModule1, 1);

    auto vertexBuffer1 = MakeVertexBuffer();
    auto vertexBuffer2 = MakeVertexBuffer();

    // Check success when vertex buffer is set for each render pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline2);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.SetVertexBuffer(1, vertexBuffer2);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    encoder.Finish();

    // Check failure because vertex buffer is not inherited in second subpass
    encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline2);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.SetVertexBuffer(1, vertexBuffer2);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.Draw(3, 1, 0, 0);
        pass.EndPass();
    }
    ASSERT_DEVICE_ERROR(encoder.Finish());
}
