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

#include "D3D12Backend.h"

#include "CommandBufferD3D12.h"
#include "PipelineD3D12.h"
#include "PipelineLayoutD3D12.h"
#include "QueueD3D12.h"
#include "ShaderModuleD3D12.h"

namespace backend {
namespace d3d12 {

    nxtProcTable GetNonValidatingProcs();
    nxtProcTable GetValidatingProcs();

    void Init(ComPtr<ID3D12Device> d3d12Device, nxtProcTable* procs, nxtDevice* device) {
        *device = nullptr;
        *procs = GetValidatingProcs();
        *device = reinterpret_cast<nxtDevice>(new Device(d3d12Device));
    }

    ComPtr<ID3D12CommandQueue> GetCommandQueue(nxtDevice device) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        return backendDevice->GetCommandQueue();
    }

    void SetNextRenderTarget(nxtDevice device, ComPtr<ID3D12Resource> renderTargetResource, D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor) {
        Device* backendDevice = reinterpret_cast<Device*>(device);
        backendDevice->SetNextRenderTarget(renderTargetResource, renderTargetDescriptor);
    }

    void ASSERT_SUCCESS(HRESULT hr) {
        ASSERT(SUCCEEDED(hr));
    }

    Device::Device(ComPtr<ID3D12Device> d3d12Device) : d3d12Device(d3d12Device) {
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ASSERT_SUCCESS(d3d12Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
    }

    Device::~Device() {
    }

    ComPtr<ID3D12Device> Device::GetD3D12Device() {
        return d3d12Device;
    }

    ComPtr<ID3D12CommandQueue> Device::GetCommandQueue() {
        return commandQueue;
    }

    ComPtr<ID3D12Resource> Device::GetNextRenderTarget() {
        return renderTargetResource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE Device::GetNextRenderTargetDescriptor() {
        return renderTargetDescriptor;
    }

    void Device::SetNextRenderTarget(ComPtr<ID3D12Resource> renderTargetResource, D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDescriptor) {
        this->renderTargetResource = renderTargetResource;
        this->renderTargetDescriptor = renderTargetDescriptor;
    }

    BindGroupBase* Device::CreateBindGroup(BindGroupBuilder* builder) {
        return new BindGroup(this, builder);
    }
    BindGroupLayoutBase* Device::CreateBindGroupLayout(BindGroupLayoutBuilder* builder) {
        return new BindGroupLayout(this, builder);
    }
    BufferBase* Device::CreateBuffer(BufferBuilder* builder) {
        return new Buffer(this, builder);
    }
    BufferViewBase* Device::CreateBufferView(BufferViewBuilder* builder) {
        return new BufferView(this, builder);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(this, builder);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(this, builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(this, builder);
    }
    FramebufferBase* Device::CreateFramebuffer(FramebufferBuilder* builder) {
        return new Framebuffer(this, builder);
    }
    PipelineBase* Device::CreatePipeline(PipelineBuilder* builder) {
        return new Pipeline(this, builder);
    }
    PipelineLayoutBase* Device::CreatePipelineLayout(PipelineLayoutBuilder* builder) {
        return new PipelineLayout(this, builder);
    }
    QueueBase* Device::CreateQueue(QueueBuilder* builder) {
        return new Queue(this, builder);
    }
    RenderPassBase* Device::CreateRenderPass(RenderPassBuilder* builder) {
        return new RenderPass(this, builder);
    }
    SamplerBase* Device::CreateSampler(SamplerBuilder* builder) {
        return new Sampler(this, builder);
    }
    ShaderModuleBase* Device::CreateShaderModule(ShaderModuleBuilder* builder) {
        return new ShaderModule(this, builder);
    }
    TextureBase* Device::CreateTexture(TextureBuilder* builder) {
        return new Texture(this, builder);
    }
    TextureViewBase* Device::CreateTextureView(TextureViewBuilder* builder) {
        return new TextureView(this, builder);
    }

    void Device::Reference() {
    }

    void Device::Release() {
    }

    // Bind Group

    BindGroup::BindGroup(Device* device, BindGroupBuilder* builder)
        : BindGroupBase(builder), device(device) {
    }

    // Bind Group Layout

    BindGroupLayout::BindGroupLayout(Device* device, BindGroupLayoutBuilder* builder)
        : BindGroupLayoutBase(builder), device(device) {
    }

    // Buffer

    Buffer::Buffer(Device* device, BufferBuilder* builder)
        : BufferBase(builder), device(device) {
    }

    void Buffer::SetSubDataImpl(uint32_t start, uint32_t count, const uint32_t* data) {
    }

    // BufferView

    BufferView::BufferView(Device* device, BufferViewBuilder* builder)
        : BufferViewBase(builder), device(device) {
    }

    // DepthStencilState

    DepthStencilState::DepthStencilState(Device* device, DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder), device(device) {
    }

    // Framebuffer

    Framebuffer::Framebuffer(Device* device, FramebufferBuilder* builder)
        : FramebufferBase(builder), device(device) {
    }

    // InputState

    InputState::InputState(Device* device, InputStateBuilder * builder)
        : InputStateBase(builder), device(device) {
    }

    // RenderPass

    RenderPass::RenderPass(Device* device, RenderPassBuilder* builder)
        : RenderPassBase(builder), device(device) {
    }

    // Sampler

    Sampler::Sampler(Device* device, SamplerBuilder* builder)
        : SamplerBase(builder), device(device) {
    }

    // Texture

    Texture::Texture(Device* device, TextureBuilder* builder)
        : TextureBase(builder), device(device) {
    }

    // TextureView

    TextureView::TextureView(Device* device, TextureViewBuilder* builder)
        : TextureViewBase(builder), device(device) {
    }

}
}
