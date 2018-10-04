// Copyright 2018 The Dawn Authors
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

#include "tests/DawnTest.h"
#include "utils/DawnHelpers.h"

class BindGroupTests : public DawnTest {
protected:
    dawn::CommandBuffer CreateSimpleComputeCommandBuffer(
            const dawn::ComputePipeline& pipeline, const dawn::BindGroup& bindGroup) {
        dawn::CommandBufferBuilder builder = device.CreateCommandBufferBuilder();
        dawn::ComputePassEncoder pass = builder.BeginComputePass();
        pass.SetComputePipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.Dispatch(1, 1, 1);
        pass.EndPass();
        return builder.GetResult();
    }
};

// Test a bindgroup reused in two command buffers in the same call to queue.Submit().
// This test passes by not asserting or crashing.
TEST_P(BindGroupTests, ReusedBindGroupSingleSubmit) {
    dawn::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        device,
        {
            {0, dawn::ShaderStageBit::Compute, dawn::BindingType::UniformBuffer },
        }
    );
    dawn::PipelineLayout pl = utils::MakeBasicPipelineLayout(device, &bgl);

    const char* shader = R"(
        #version 450
        layout(std140, set = 0, binding = 0) uniform Contents {
            float f;
        } contents;
        void main() {
        }
    )";

    dawn::ShaderModule module =
        utils::CreateShaderModule(device, dawn::ShaderStage::Compute, shader);
    dawn::ComputePipelineDescriptor cpDesc;
    cpDesc.module = module.Clone();
    cpDesc.entryPoint = "main";
    cpDesc.layout = pl.Clone();
    dawn::ComputePipeline cp = device.CreateComputePipeline(&cpDesc);

    dawn::BufferDescriptor bufferDesc;
    bufferDesc.size = sizeof(float);
    bufferDesc.usage = dawn::BufferUsageBit::TransferDst |
                       dawn::BufferUsageBit::Uniform;
    dawn::Buffer buffer = device.CreateBuffer(&bufferDesc);
    dawn::BufferView bufferView =
        buffer.CreateBufferViewBuilder().SetExtent(0, sizeof(float)).GetResult();
    dawn::BindGroup bindGroup = device.CreateBindGroupBuilder()
        .SetLayout(bgl)
        .SetBufferViews(0, 1, &bufferView)
        .GetResult();

    dawn::CommandBuffer cb[2];
    cb[0] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    cb[1] = CreateSimpleComputeCommandBuffer(cp, bindGroup);
    queue.Submit(2, cb);
}

DAWN_INSTANTIATE_TEST(BindGroupTests, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend);
