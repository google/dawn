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

#include "utils/ComboRenderBundleEncoderDescriptor.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class VertexBufferValidationTest : public ValidationTest {
  protected:
    void SetUp() override {
        ValidationTest::SetUp();

        fsModule = utils::CreateShaderModule(device, R"(
            [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
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
        vs << "[[stage(vertex)]] fn main(\n";
        for (unsigned int i = 0; i < bufferCount; ++i) {
            // TODO(cwallez@chromium.org): remove this special handling of 0 once Tint supports
            // trailing commas in argument lists.
            if (i != 0) {
                vs << ", ";
            }
            vs << "[[location(" << i << ")]] a_position" << i << " : vec3<f32>\n";
        }
        vs << ") -> [[builtin(position)]] vec4<f32> {";

        vs << "return vec4<f32>(";
        for (unsigned int i = 0; i < bufferCount; ++i) {
            vs << "a_position" << i;
            if (i != bufferCount - 1) {
                vs << " + ";
            }
        }
        vs << ", 1.0);";

        vs << "}\n";

        return utils::CreateShaderModule(device, vs.str().c_str());
    }

    wgpu::RenderPipeline MakeRenderPipeline(const wgpu::ShaderModule& vsModule,
                                            unsigned int bufferCount) {
        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;

        for (unsigned int i = 0; i < bufferCount; ++i) {
            descriptor.cBuffers[i].attributeCount = 1;
            descriptor.cBuffers[i].attributes = &descriptor.cAttributes[i];
            descriptor.cAttributes[i].shaderLocation = i;
            descriptor.cAttributes[i].format = wgpu::VertexFormat::Float32x3;
        }
        descriptor.vertex.bufferCount = bufferCount;

        return device.CreateRenderPipeline(&descriptor);
    }

    wgpu::ShaderModule fsModule;
};

// Check that vertex buffers still count as bound if we switch the pipeline.
TEST_F(VertexBufferValidationTest, VertexBuffersInheritedBetweenPipelines) {
    DummyRenderPass renderPass(device);
    wgpu::ShaderModule vsModule2 = MakeVertexShader(2);
    wgpu::ShaderModule vsModule1 = MakeVertexShader(1);

    wgpu::RenderPipeline pipeline2 = MakeRenderPipeline(vsModule2, 2);
    wgpu::RenderPipeline pipeline1 = MakeRenderPipeline(vsModule1, 1);

    wgpu::Buffer vertexBuffer1 = MakeVertexBuffer();
    wgpu::Buffer vertexBuffer2 = MakeVertexBuffer();

    // Check failure when vertex buffer is not set
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.Draw(3);
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
        pass.Draw(3);
        pass.SetPipeline(pipeline1);
        pass.Draw(3);
        pass.EndPass();
    }
    encoder.Finish();
}

// Check that vertex buffers that are set are reset between render passes.
TEST_F(VertexBufferValidationTest, VertexBuffersNotInheritedBetweenRenderPasses) {
    DummyRenderPass renderPass(device);
    wgpu::ShaderModule vsModule2 = MakeVertexShader(2);
    wgpu::ShaderModule vsModule1 = MakeVertexShader(1);

    wgpu::RenderPipeline pipeline2 = MakeRenderPipeline(vsModule2, 2);
    wgpu::RenderPipeline pipeline1 = MakeRenderPipeline(vsModule1, 1);

    wgpu::Buffer vertexBuffer1 = MakeVertexBuffer();
    wgpu::Buffer vertexBuffer2 = MakeVertexBuffer();

    // Check success when vertex buffer is set for each render pass
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline2);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.SetVertexBuffer(1, vertexBuffer2);
        pass.Draw(3);
        pass.EndPass();
    }
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.SetVertexBuffer(0, vertexBuffer1);
        pass.Draw(3);
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
        pass.Draw(3);
        pass.EndPass();
    }
    {
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetPipeline(pipeline1);
        pass.Draw(3);
        pass.EndPass();
    }
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Check validation of the vertex buffer slot for OOB.
TEST_F(VertexBufferValidationTest, VertexBufferSlotValidation) {
    wgpu::Buffer buffer = MakeVertexBuffer();

    DummyRenderPass renderPass(device);

    // Control case: using the last vertex buffer slot in render passes is ok.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(kMaxVertexBuffers - 1, buffer, 0);
        pass.EndPass();
        encoder.Finish();
    }

    // Error case: using past the last vertex buffer slot in render pass fails.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(kMaxVertexBuffers, buffer, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatsCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    // Control case: using the last vertex buffer slot in render bundles is ok.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(kMaxVertexBuffers - 1, buffer, 0);
        encoder.Finish();
    }

    // Error case: using past the last vertex buffer slot in render bundle fails.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(kMaxVertexBuffers, buffer, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that for OOB validation of vertex buffer offset and size.
TEST_F(VertexBufferValidationTest, VertexBufferOffsetOOBValidation) {
    wgpu::Buffer buffer = MakeVertexBuffer();

    DummyRenderPass renderPass(device);
    // Control case, using the full buffer, with or without an explicit size is valid.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        // Explicit size
        pass.SetVertexBuffer(0, buffer, 0, 256);
        // Implicit size
        pass.SetVertexBuffer(0, buffer, 0, wgpu::kWholeSize);
        pass.SetVertexBuffer(0, buffer, 256 - 4, wgpu::kWholeSize);
        pass.SetVertexBuffer(0, buffer, 4, wgpu::kWholeSize);
        // Implicit size of zero
        pass.SetVertexBuffer(0, buffer, 256, wgpu::kWholeSize);
        pass.EndPass();
        encoder.Finish();
    }

    // Bad case, offset + size is larger than the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, buffer, 4, 256);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Bad case, size is 0 but the offset is larger than the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, buffer, 256 + 4, 0);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatsCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    // Control case, using the full buffer, with or without an explicit size is valid.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        // Explicit size
        encoder.SetVertexBuffer(0, buffer, 0, 256);
        // Implicit size
        encoder.SetVertexBuffer(0, buffer, 0, wgpu::kWholeSize);
        encoder.SetVertexBuffer(0, buffer, 256 - 4, wgpu::kWholeSize);
        encoder.SetVertexBuffer(0, buffer, 4, wgpu::kWholeSize);
        // Implicit size of zero
        encoder.SetVertexBuffer(0, buffer, 256, wgpu::kWholeSize);
        encoder.Finish();
    }

    // Bad case, offset + size is larger than the buffer
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(0, buffer, 4, 256);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Bad case, size is 0 but the offset is larger than the buffer
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(0, buffer, 256 + 4, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that the vertex buffer must have the Vertex usage.
TEST_F(VertexBufferValidationTest, InvalidUsage) {
    wgpu::Buffer vertexBuffer = MakeVertexBuffer();
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 0, 0});

    DummyRenderPass renderPass(device);
    // Control case: using the vertex buffer is valid.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.EndPass();
        encoder.Finish();
    }
    // Error case: using the index buffer is an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, indexBuffer);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatsCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    // Control case: using the vertex buffer is valid.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(0, vertexBuffer);
        encoder.Finish();
    }
    // Error case: using the index buffer is an error.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetVertexBuffer(0, indexBuffer);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check the alignment constraint on the index buffer offset.
TEST_F(VertexBufferValidationTest, OffsetAlignment) {
    wgpu::Buffer vertexBuffer = MakeVertexBuffer();

    DummyRenderPass renderPass(device);
    // Control cases: vertex buffer offset is a multiple of 4
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, vertexBuffer, 0);
        pass.SetVertexBuffer(0, vertexBuffer, 4);
        pass.SetVertexBuffer(0, vertexBuffer, 12);
        pass.EndPass();
        encoder.Finish();
    }

    // Error case: vertex buffer offset isn't a multiple of 4
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetVertexBuffer(0, vertexBuffer, 2);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}
