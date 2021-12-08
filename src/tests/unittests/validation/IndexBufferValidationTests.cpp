// Copyright 2020 The Dawn Authors
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

#include "utils/ComboRenderBundleEncoderDescriptor.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class IndexBufferValidationTest : public ValidationTest {
    protected:
    wgpu::RenderPipeline MakeTestPipeline(wgpu::IndexFormat format,
        wgpu::PrimitiveTopology primitiveTopology) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, R"(
            [[stage(vertex)]] fn main() -> [[builtin(position)]] vec4<f32> {
                return vec4<f32>(0.0, 0.0, 0.0, 1.0);
            })");

        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, R"(
            [[stage(fragment)]] fn main() -> [[location(0)]] vec4<f32> {
                return vec4<f32>(0.0, 1.0, 0.0, 1.0);
            })");

        utils::ComboRenderPipelineDescriptor descriptor;
        descriptor.vertex.module = vsModule;
        descriptor.cFragment.module = fsModule;
        descriptor.primitive.topology = primitiveTopology;
        descriptor.primitive.stripIndexFormat = format;
        descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        return device.CreateRenderPipeline(&descriptor);
    }
};

// Test that IndexFormat::Undefined is disallowed.
TEST_F(IndexBufferValidationTest, UndefinedIndexFormat) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Index;
    bufferDesc.size = 256;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    DummyRenderPass renderPass(device);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Undefined);
    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that an invalid index format is disallowed.
TEST_F(IndexBufferValidationTest, InvalidIndexFormat) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Index;
    bufferDesc.size = 256;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    DummyRenderPass renderPass(device);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
    pass.SetIndexBuffer(buffer, static_cast<wgpu::IndexFormat>(404));
    pass.EndPass();
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

// Test that for OOB validation of index buffer offset and size.
TEST_F(IndexBufferValidationTest, IndexBufferOffsetOOBValidation) {
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.usage = wgpu::BufferUsage::Index;
    bufferDesc.size = 256;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    DummyRenderPass renderPass(device);
    // Control case, using the full buffer, with or without an explicit size is valid.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        // Explicit size
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, 256);
        // Implicit size
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, wgpu::kWholeSize);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256 - 4, wgpu::kWholeSize);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 4, wgpu::kWholeSize);
        // Implicit size of zero
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256, wgpu::kWholeSize);
        pass.EndPass();
        encoder.Finish();
    }

    // Bad case, offset + size is larger than the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 4, 256);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Bad case, size is 0 but the offset is larger than the buffer
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256 + 4, 0);
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
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, 256);
        // Implicit size
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 0, wgpu::kWholeSize);
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256 - 4, wgpu::kWholeSize);
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 4, wgpu::kWholeSize);
        // Implicit size of zero
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256, wgpu::kWholeSize);
        encoder.Finish();
    }

    // Bad case, offset + size is larger than the buffer
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 4, 256);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Bad case, size is 0 but the offset is larger than the buffer
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(buffer, wgpu::IndexFormat::Uint32, 256 + 4, 0);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that formats given when setting an index buffers must match the format specified on the
// pipeline for strip primitive topologies.
TEST_F(IndexBufferValidationTest, IndexBufferFormatMatchesPipelineStripFormat) {
    wgpu::RenderPipeline pipeline32 = MakeTestPipeline(wgpu::IndexFormat::Uint32,
                                                       wgpu::PrimitiveTopology::TriangleStrip);
    wgpu::RenderPipeline pipeline16 = MakeTestPipeline(wgpu::IndexFormat::Uint16,
                                                       wgpu::PrimitiveTopology::LineStrip);
    wgpu::RenderPipeline pipelineUndef =
        MakeTestPipeline(wgpu::IndexFormat::Undefined, wgpu::PrimitiveTopology::LineStrip);

    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});

    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatsCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;

    // Expected to fail because pipeline and index formats don't match.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        encoder.SetPipeline(pipeline32);
        encoder.DrawIndexed(3);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        encoder.SetPipeline(pipeline16);
        encoder.DrawIndexed(3);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Expected to succeed because pipeline and index formats match.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        encoder.SetPipeline(pipeline16);
        encoder.DrawIndexed(3);
        encoder.Finish();
    }

    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        encoder.SetPipeline(pipeline32);
        encoder.DrawIndexed(3);
        encoder.Finish();
    }

    // Expected to fail because pipeline doesn't specify an index format.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16);
        encoder.SetPipeline(pipelineUndef);
        encoder.DrawIndexed(3);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        encoder.SetPipeline(pipelineUndef);
        encoder.DrawIndexed(3);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Expected to succeed because non-indexed draw calls don't require a pipeline index format.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetPipeline(pipelineUndef);
        encoder.Draw(3);
        encoder.Finish();
    }
}

// Check that the index buffer must have the Index usage.
TEST_F(IndexBufferValidationTest, InvalidUsage) {
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});
    wgpu::Buffer copyBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::CopySrc, {0, 1, 2});

    DummyRenderPass renderPass(device);
    // Control case: using the index buffer is valid.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.EndPass();
        encoder.Finish();
    }
    // Error case: using the copy buffer is an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(copyBuffer, wgpu::IndexFormat::Uint32);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    utils::ComboRenderBundleEncoderDescriptor renderBundleDesc = {};
    renderBundleDesc.colorFormatsCount = 1;
    renderBundleDesc.cColorFormats[0] = wgpu::TextureFormat::RGBA8Unorm;
    // Control case: using the index buffer is valid.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        encoder.Finish();
    }
    // Error case: using the copy buffer is an error.
    {
        wgpu::RenderBundleEncoder encoder = device.CreateRenderBundleEncoder(&renderBundleDesc);
        encoder.SetIndexBuffer(copyBuffer, wgpu::IndexFormat::Uint32);
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check the alignment constraint on the index buffer offset.
TEST_F(IndexBufferValidationTest, OffsetAlignment) {
    wgpu::Buffer indexBuffer =
        utils::CreateBufferFromData<uint32_t>(device, wgpu::BufferUsage::Index, {0, 1, 2});

    DummyRenderPass renderPass(device);
    // Control cases: index buffer offset is a multiple of the index format size
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 0);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 4);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16, 0);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16, 2);
        pass.EndPass();
        encoder.Finish();
    }

    // Error case: index buffer offset isn't a multiple of 4 for IndexFormat::Uint32
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32, 2);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
    // Error case: index buffer offset isn't a multiple of 2 for IndexFormat::Uint16
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint16, 1);
        pass.EndPass();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}
