// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/replay/Replay.h"

#include <algorithm>

#include "dawn/common/Constants.h"
#include "dawn/replay/Deserialization.h"

namespace dawn::replay {

namespace {

wgpu::Origin3D ToWGPU(const schema::Origin3D& origin) {
    return wgpu::Origin3D{
        .x = origin.x,
        .y = origin.y,
        .z = origin.z,
    };
}

wgpu::Extent3D ToWGPU(const schema::Extent3D& extent) {
    return wgpu::Extent3D{
        .width = extent.width,
        .height = extent.height,
        .depthOrArrayLayers = extent.depthOrArrayLayers,
    };
}

wgpu::Color ToWGPU(const schema::Color& color) {
    return wgpu::Color{
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };
}

wgpu::PassTimestampWrites ToWGPU(const Replay& replay, const schema::TimestampWrites& writes) {
    return wgpu::PassTimestampWrites{
        .nextInChain = nullptr,
        .querySet = replay.GetObjectById<wgpu::QuerySet>(writes.querySetId),
        .beginningOfPassWriteIndex = writes.beginningOfPassWriteIndex,
        .endOfPassWriteIndex = writes.endOfPassWriteIndex,
    };
}

std::vector<wgpu::ConstantEntry> ToWGPU(const std::vector<schema::PipelineConstant>& constants) {
    std::vector<wgpu::ConstantEntry> entries;
    entries.reserve(constants.size());
    std::transform(constants.begin(), constants.end(), std::back_inserter(entries),
                   [](const auto& constant) {
                       return wgpu::ConstantEntry{
                           .key = wgpu::StringView(constant.name),
                           .value = constant.value,
                       };
                   });
    return entries;
}

wgpu::TexelCopyBufferLayout ToWGPU(const schema::TexelCopyBufferLayout& info) {
    return wgpu::TexelCopyBufferLayout{
        .offset = info.offset,
        .bytesPerRow = info.bytesPerRow,
        .rowsPerImage = info.rowsPerImage,
    };
}

wgpu::TexelCopyBufferInfo ToWGPU(const Replay& replay, const schema::TexelCopyBufferInfo& info) {
    return wgpu::TexelCopyBufferInfo{
        .layout = ToWGPU(info.layout),
        .buffer = replay.GetObjectById<wgpu::Buffer>(info.bufferId),
    };
}

wgpu::TexelCopyTextureInfo ToWGPU(const Replay& replay, const schema::TexelCopyTextureInfo& info) {
    return wgpu::TexelCopyTextureInfo{
        .texture = replay.GetObjectById<wgpu::Texture>(info.textureId),
        .mipLevel = info.mipLevel,
        .origin = ToWGPU(info.origin),
        .aspect = static_cast<wgpu::TextureAspect>(info.aspect),
    };
}

wgpu::StencilFaceState ToWGPU(const schema::StencilFaceState& state) {
    return wgpu::StencilFaceState{
        .compare = state.compare,
        .failOp = state.failOp,
        .depthFailOp = state.depthFailOp,
        .passOp = state.passOp,
    };
}

wgpu::BlendComponent ToWGPU(const schema::BlendComponent& component) {
    return wgpu::BlendComponent{
        .operation = component.operation,
        .srcFactor = component.srcFactor,
        .dstFactor = component.dstFactor,
    };
}

wgpu::BlendState ToWGPU(const schema::BlendState& state) {
    return wgpu::BlendState{
        .color = ToWGPU(state.color),
        .alpha = ToWGPU(state.alpha),
    };
}

bool IsBlendComponentEnabled(const wgpu::BlendComponent& component) {
    return component.operation != wgpu::BlendOperation::Add ||
           component.srcFactor != wgpu::BlendFactor::One ||
           component.dstFactor != wgpu::BlendFactor::Zero;
}

bool IsBlendEnabled(const wgpu::BlendState& blend) {
    return IsBlendComponentEnabled(blend.color) || IsBlendComponentEnabled(blend.alpha);
}

MaybeError ReadContentIntoBuffer(ReadHead& readHead,
                                 wgpu::Device device,
                                 wgpu::Buffer buffer,
                                 uint64_t bufferOffset,
                                 uint64_t size) {
    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(size));

    device.GetQueue().WriteBuffer(buffer, bufferOffset, data, size);
    return {};
}

MaybeError MapContentIntoBuffer(ReadHead& readHead,
                                wgpu::Device device,
                                wgpu::Buffer buffer,
                                uint64_t bufferOffset,
                                uint64_t size) {
    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(size));

    // Note: We could call MapAsync here, wait for it to map, put in the data, then unmap.
    // To do so we'd have to change the code in Replay::CreateBuffer to leave the buffer
    // as MapWrite|CopySrc. That would be more inline with what the user actually did
    // though it might be slower as it would be synchronous.
    device.GetQueue().WriteBuffer(buffer, bufferOffset, data, size);
    return {};
}

MaybeError ReadContentIntoTexture(const Replay& replay,
                                  ReadHead& readHead,
                                  wgpu::Device device,
                                  const schema::RootCommandWriteTextureCmdData& cmdData) {
    const uint32_t* data;
    DAWN_TRY_ASSIGN(data, readHead.GetData(cmdData.dataSize));

    wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, cmdData.destination);
    wgpu::TexelCopyBufferLayout layout = ToWGPU(cmdData.layout);
    wgpu::Extent3D size = ToWGPU(cmdData.size);
    device.GetQueue().WriteTexture(&dst, data, cmdData.dataSize, &layout, &size);
    return {};
}

ResultOrError<wgpu::BindGroup> CreateBindGroup(const Replay& replay,
                                               wgpu::Device device,
                                               ReadHead& readHead,
                                               const std::string& label) {
    schema::BindGroup bg;
    DAWN_TRY(Deserialize(readHead, &bg));

    std::vector<wgpu::BindGroupEntry> entries;
    for (uint32_t i = 0; i < bg.numEntries; ++i) {
        schema::BindGroupLayoutEntryType entryType;
        uint32_t binding;
        DAWN_TRY(Deserialize(readHead, &entryType));
        DAWN_TRY(Deserialize(readHead, &binding));

        switch (entryType) {
            case schema::BindGroupLayoutEntryType::BufferBinding: {
                schema::BindGroupEntryTypeBufferBindingData data;
                DAWN_TRY(Deserialize(readHead, &data));
                entries.push_back(wgpu::BindGroupEntry{
                    .binding = binding,
                    .buffer = replay.GetObjectById<wgpu::Buffer>(data.bufferId),
                    .offset = data.offset,
                    .size = data.size,
                });
                break;
            }
            case schema::BindGroupLayoutEntryType::TextureBinding: {
                schema::BindGroupEntryTypeTextureBindingData data;
                DAWN_TRY(Deserialize(readHead, &data));
                entries.push_back(wgpu::BindGroupEntry{
                    .binding = binding,
                    .textureView = replay.GetObjectById<wgpu::TextureView>(data.textureViewId),
                });
                break;
            }
            default:
                return DAWN_INTERNAL_ERROR("unsupported bind group entry type");
        }
    }

    wgpu::BindGroupDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::BindGroupLayout>(bg.layoutId),
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&desc);
    return {bindGroup};
}

ResultOrError<wgpu::BindGroupLayout> CreateBindGroupLayout(const Replay& replay,
                                                           wgpu::Device device,
                                                           ReadHead& readHead,
                                                           const std::string& label) {
    schema::BindGroupLayout bgl;
    DAWN_TRY(Deserialize(readHead, &bgl));

    std::vector<wgpu::BindGroupLayoutEntry> entries;
    for (uint32_t i = 0; i < bgl.numEntries; ++i) {
        schema::BindGroupLayoutEntryType entryType;
        schema::BindGroupLayoutBinding binding;
        DAWN_TRY(Deserialize(readHead, &entryType));
        DAWN_TRY(Deserialize(readHead, &binding));

        switch (entryType) {
            case schema::BindGroupLayoutEntryType::BufferBinding: {
                schema::BindGroupLayoutEntryTypeBufferBindingData data;
                DAWN_TRY(Deserialize(readHead, &data));

                entries.push_back({
                    .binding = binding.binding,
                    .visibility = binding.visibility,
                    .bindingArraySize = binding.bindingArraySize,
                    .buffer =
                        {
                            .type = data.type,
                            .hasDynamicOffset = data.hasDynamicOffset,
                            .minBindingSize = data.minBindingSize,
                        },
                });
                break;
            }
            case schema::BindGroupLayoutEntryType::StorageTextureBinding: {
                schema::BindGroupLayoutEntryTypeStorageTextureBindingData data;
                DAWN_TRY(Deserialize(readHead, &data));

                entries.push_back({
                    .binding = binding.binding,
                    .visibility = binding.visibility,
                    .bindingArraySize = binding.bindingArraySize,
                    .storageTexture =
                        {
                            .access = data.access,
                            .format = data.format,
                            .viewDimension = data.viewDimension,
                        },
                });
                break;
            }
            case schema::BindGroupLayoutEntryType::TextureBinding: {
                schema::BindGroupLayoutEntryTypeTextureBindingData data;
                DAWN_TRY(Deserialize(readHead, &data));

                entries.push_back({
                    .binding = binding.binding,
                    .visibility = binding.visibility,
                    .bindingArraySize = binding.bindingArraySize,
                    .texture =
                        {
                            .sampleType = data.sampleType,
                            .viewDimension = data.viewDimension,
                            .multisampled = data.multisampled,
                        },
                });
                break;
            }
            default:
                return DAWN_INTERNAL_ERROR("unhandled bind group layout entry type");
        }
    }

    wgpu::BindGroupLayoutDescriptor desc{
        .label = wgpu::StringView(label),
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroupLayout bindGroupLayout = device.CreateBindGroupLayout(&desc);
    return {bindGroupLayout};
}

ResultOrError<wgpu::Buffer> CreateBuffer(wgpu::Device device,
                                         ReadHead& readHead,
                                         const std::string& label) {
    schema::Buffer buf;
    DAWN_TRY(Deserialize(readHead, &buf));

    wgpu::BufferUsage usage = (buf.usage & wgpu::BufferUsage::MapRead)
                                  ? buf.usage
                                  : (buf.usage | wgpu::BufferUsage::CopySrc);

    // Remap mappable write buffers as CopySrc|CopyDst as we use WriteBuffer to set their contents.
    if (usage == (wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc)) {
        usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
    }

    wgpu::BufferDescriptor desc{
        .label = wgpu::StringView(label),
        .usage = usage,
        .size = buf.size,
    };
    wgpu::Buffer buffer = device.CreateBuffer(&desc);
    return {buffer};
}

template <typename F>
ResultOrError<wgpu::ComputePipeline> CreateComputePipeline(const Replay& replay,
                                                           wgpu::Device device,
                                                           ReadHead& readHead,
                                                           const std::string& label,
                                                           F func) {
    schema::ComputePipeline pipeline;
    DAWN_TRY(Deserialize(readHead, &pipeline));

    std::vector<wgpu::ConstantEntry> constants = ToWGPU(pipeline.compute.constants);

    wgpu::ComputePipelineDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::PipelineLayout>(pipeline.layoutId),
        .compute =
            {
                .module = replay.GetObjectById<wgpu::ShaderModule>(pipeline.compute.moduleId),
                .entryPoint = wgpu::StringView(pipeline.compute.entryPoint),
                .constantCount = constants.size(),
                .constants = constants.data(),
            },
    };
    wgpu::ComputePipeline computePipeline = device.CreateComputePipeline(&desc);
    func(computePipeline, pipeline.groupIndexIds);
    return {computePipeline};
}

ResultOrError<wgpu::PipelineLayout> CreatePipelineLayout(const Replay& replay,
                                                         wgpu::Device device,
                                                         ReadHead& readHead,
                                                         const std::string& label) {
    schema::PipelineLayout layout;
    DAWN_TRY(Deserialize(readHead, &layout));

    std::vector<wgpu::BindGroupLayout> bindGroupLayouts;
    for (const auto bindGroupLayoutId : layout.bindGroupLayoutIds) {
        bindGroupLayouts.push_back(replay.GetObjectById<wgpu::BindGroupLayout>(bindGroupLayoutId));
    }

    wgpu::PipelineLayoutDescriptor desc{
        .label = wgpu::StringView(label),
        .bindGroupLayoutCount = bindGroupLayouts.size(),
        .bindGroupLayouts = bindGroupLayouts.data(),
    };
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&desc);
    return {pipelineLayout};
}

template <typename F>
ResultOrError<wgpu::RenderPipeline> CreateRenderPipeline(const Replay& replay,
                                                         wgpu::Device device,
                                                         ReadHead& readHead,
                                                         const std::string& label,
                                                         F func) {
    schema::RenderPipeline pipeline;
    DAWN_TRY(Deserialize(readHead, &pipeline));

    std::vector<wgpu::ConstantEntry> vertexConstants = ToWGPU(pipeline.vertex.program.constants);
    std::vector<wgpu::ConstantEntry> fragmentConstants =
        ToWGPU(pipeline.fragment.program.constants);
    std::vector<wgpu::ColorTargetState> colorTargets;
    std::vector<wgpu::BlendState> blendStates(pipeline.fragment.targets.size());
    std::vector<wgpu::VertexBufferLayout> buffers;

    std::vector<wgpu::VertexAttribute> attributes(kMaxVertexAttributes);
    uint32_t attributeCount = 0;

    for (const auto& buffer : pipeline.vertex.buffers) {
        const auto attributesForBuffer = &attributes[attributeCount];
        for (const auto& attrib : buffer.attributes) {
            auto& attr = attributes[attributeCount++];
            attr.format = attrib.format;
            attr.offset = attrib.offset;
            attr.shaderLocation = attrib.shaderLocation;
        }
        buffers.push_back({
            .stepMode = buffer.stepMode,
            .arrayStride = buffer.arrayStride,
            .attributeCount = buffer.attributes.size(),
            .attributes = attributesForBuffer,
        });
    }

    wgpu::FragmentState* fragment = nullptr;
    wgpu::FragmentState fragmentState;
    if (pipeline.fragment.program.moduleId) {
        fragment = &fragmentState;
        fragmentState.module =
            replay.GetObjectById<wgpu::ShaderModule>(pipeline.fragment.program.moduleId);
        fragmentState.entryPoint = wgpu::StringView(pipeline.fragment.program.entryPoint);
        fragmentState.constantCount = fragmentConstants.size();
        fragmentState.constants = fragmentConstants.data();
        for (const auto& target : pipeline.fragment.targets) {
            wgpu::BlendState& blend = blendStates[colorTargets.size()];
            blend = ToWGPU(target.blend);
            colorTargets.push_back({
                .format = target.format,
                .blend = IsBlendEnabled(blend) ? &blend : nullptr,
                .writeMask = target.writeMask,
            });
        }
        fragmentState.targetCount = colorTargets.size();
        fragmentState.targets = colorTargets.data();
    }

    wgpu::DepthStencilState* depthStencil = nullptr;
    wgpu::DepthStencilState depthStencilState;
    if (pipeline.depthStencil.format != wgpu::TextureFormat::Undefined) {
        depthStencil = &depthStencilState;
        depthStencilState.format = pipeline.depthStencil.format;
        depthStencilState.depthWriteEnabled = pipeline.depthStencil.depthWriteEnabled;
        depthStencilState.depthCompare = pipeline.depthStencil.depthCompare;
        depthStencilState.stencilFront = ToWGPU(pipeline.depthStencil.stencilFront);
        depthStencilState.stencilBack = ToWGPU(pipeline.depthStencil.stencilBack);
        depthStencilState.stencilReadMask = pipeline.depthStencil.stencilReadMask;
        depthStencilState.stencilWriteMask = pipeline.depthStencil.stencilWriteMask;
        depthStencilState.depthBias = pipeline.depthStencil.depthBias;
        depthStencilState.depthBiasSlopeScale = pipeline.depthStencil.depthBiasSlopeScale;
        depthStencilState.depthBiasClamp = pipeline.depthStencil.depthBiasClamp;
    }

    wgpu::RenderPipelineDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::PipelineLayout>(pipeline.layoutId),
        .vertex =
            {
                .module =
                    replay.GetObjectById<wgpu::ShaderModule>(pipeline.vertex.program.moduleId),
                .entryPoint = wgpu::StringView(pipeline.vertex.program.entryPoint),
                .constantCount = vertexConstants.size(),
                .constants = vertexConstants.data(),
                .bufferCount = buffers.size(),
                .buffers = buffers.data(),
            },
        .primitive =
            {
                .topology = pipeline.primitive.topology,
                .stripIndexFormat = pipeline.primitive.stripIndexFormat,
                .frontFace = pipeline.primitive.frontFace,
                .cullMode = pipeline.primitive.cullMode,
                .unclippedDepth = pipeline.primitive.unclippedDepth,
            },
        .depthStencil = depthStencil,
        .multisample =
            {
                .count = pipeline.multisample.count,
                .mask = pipeline.multisample.mask,
                .alphaToCoverageEnabled = pipeline.multisample.alphaToCoverageEnabled,
            },
        .fragment = fragment,
    };
    wgpu::RenderPipeline renderPipeline = device.CreateRenderPipeline(&desc);
    func(renderPipeline, pipeline.groupIndexIds);
    return {renderPipeline};
}

ResultOrError<wgpu::ShaderModule> CreateShaderModule(wgpu::Device device,
                                                     ReadHead& readHead,
                                                     const std::string& label) {
    schema::ShaderModule mod;
    DAWN_TRY(Deserialize(readHead, &mod));

    // TODO(452840621): Make this use a chain instead of hard coded to WGSL only and handle other
    // chained structs.
    wgpu::ShaderSourceWGSL source({
        .nextInChain = nullptr,
        .code = wgpu::StringView(mod.code),
    });
    wgpu::ShaderModuleDescriptor desc{
        .nextInChain = &source,
        .label = wgpu::StringView(label),
    };
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&desc);
    return {shaderModule};
}

ResultOrError<wgpu::Texture> CreateTexture(wgpu::Device device,
                                           ReadHead& readHead,
                                           const std::string& label) {
    schema::Texture tex;
    DAWN_TRY(Deserialize(readHead, &tex));

    wgpu::TextureDescriptor desc{
        .label = wgpu::StringView(label),
        .usage = tex.usage | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst,
        .dimension = tex.dimension,
        .size = ToWGPU(tex.size),
        .format = tex.format,
        .mipLevelCount = tex.mipLevelCount,
        .sampleCount = tex.sampleCount,
        .viewFormatCount = tex.viewFormats.size(),
        .viewFormats = tex.viewFormats.data(),
    };
    wgpu::Texture texture = device.CreateTexture(&desc);
    return {texture};
}

ResultOrError<wgpu::TextureView> CreateTextureView(const Replay& replay,
                                                   ReadHead& readHead,
                                                   const std::string& label) {
    schema::TextureView view;
    DAWN_TRY(Deserialize(readHead, &view));

    wgpu::TextureViewDescriptor desc{
        .label = wgpu::StringView(label),
        .format = view.format,
        .dimension = view.dimension,
        .baseMipLevel = view.baseMipLevel,
        .mipLevelCount = view.mipLevelCount,
        .baseArrayLayer = view.baseArrayLayer,
        .arrayLayerCount = view.arrayLayerCount,
        .aspect = view.aspect,
        .usage = view.usage,
    };
    wgpu::Texture texture = replay.GetObjectById<wgpu::Texture>(view.textureId);
    wgpu::TextureView textureView = texture.CreateView(&desc);
    return {textureView};
}

MaybeError ProcessComputePassCommands(const Replay& replay,
                                      ReadHead& readHead,
                                      wgpu::Device device,
                                      wgpu::ComputePassEncoder pass) {
    schema::ComputePassCommand cmd;

    while (!readHead.IsDone()) {
        DAWN_TRY(Deserialize(readHead, &cmd));
        switch (cmd) {
            case schema::ComputePassCommand::End: {
                pass.End();
                return {};
            }
            case schema::ComputePassCommand::SetComputePipeline: {
                schema::ComputePassCommandSetComputePipelineCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.SetPipeline(replay.GetObjectById<wgpu::ComputePipeline>(data.pipelineId));
                break;
            }
            case schema::ComputePassCommand::SetBindGroup: {
                schema::ComputePassCommandSetBindGroupCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.SetBindGroup(data.index,
                                  replay.GetObjectById<wgpu::BindGroup>(data.bindGroupId),
                                  data.dynamicOffsets.size(), data.dynamicOffsets.data());
                break;
            }
            case schema::ComputePassCommand::Dispatch: {
                schema::ComputePassCommandDispatchCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.DispatchWorkgroups(data.x, data.y, data.z);
                break;
            }
            default:
                return DAWN_INTERNAL_ERROR("Compute Pass Command not implemented");
        }
    }
    return DAWN_INTERNAL_ERROR("Missing ComputePass End command");
}

MaybeError ProcessRenderPassCommands(const Replay& replay,
                                     ReadHead& readHead,
                                     wgpu::Device device,
                                     wgpu::RenderPassEncoder pass) {
    schema::RenderPassCommand cmd;

    while (!readHead.IsDone()) {
        DAWN_TRY(Deserialize(readHead, &cmd));
        switch (cmd) {
            case schema::RenderPassCommand::End: {
                pass.End();
                return {};
            }
            case schema::RenderPassCommand::SetPipeline: {
                schema::RenderPassCommandSetPipelineCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.SetPipeline(replay.GetObjectById<wgpu::RenderPipeline>(data.pipelineId));
                break;
            }
            case schema::RenderPassCommand::SetBindGroup: {
                schema::RenderPassCommandSetBindGroupCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.SetBindGroup(data.index,
                                  replay.GetObjectById<wgpu::BindGroup>(data.bindGroupId),
                                  data.dynamicOffsets.size(), data.dynamicOffsets.data());
                break;
            }
            case schema::RenderPassCommand::SetVertexBuffer: {
                schema::RenderPassCommandSetVertexBufferCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.SetVertexBuffer(data.slot, replay.GetObjectById<wgpu::Buffer>(data.bufferId),
                                     data.offset, data.size);
                break;
            }
            case schema::RenderPassCommand::Draw: {
                schema::RenderPassCommandDrawCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                pass.Draw(data.vertexCount, data.instanceCount, data.firstVertex,
                          data.firstInstance);
                break;
            }
            default:
                return DAWN_INTERNAL_ERROR("Render Pass Command not implemented");
        }
    }
    return DAWN_INTERNAL_ERROR("Missing RenderPass End command");
}

MaybeError ProcessEncoderCommands(const Replay& replay,
                                  ReadHead& readHead,
                                  wgpu::Device device,
                                  wgpu::CommandEncoder encoder) {
    schema::EncoderCommand cmd;

    while (!readHead.IsDone()) {
        DAWN_TRY(Deserialize(readHead, &cmd));
        switch (cmd) {
            case schema::EncoderCommand::BeginComputePass: {
                schema::EncoderCommandBeginComputePassCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::PassTimestampWrites timestampWrites = ToWGPU(replay, data.timestampWrites);
                wgpu::ComputePassDescriptor desc{
                    .label = wgpu::StringView(data.label),
                    .timestampWrites = timestampWrites.querySet ? &timestampWrites : nullptr,
                };
                wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
                DAWN_TRY(ProcessComputePassCommands(replay, readHead, device, pass));
                break;
            }
            case schema::EncoderCommand::BeginRenderPass: {
                schema::EncoderCommandBeginRenderPassCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::PassTimestampWrites timestampWrites = ToWGPU(replay, data.timestampWrites);
                std::vector<wgpu::RenderPassColorAttachment> colorAttachments;

                for (const auto& attachment : data.colorAttachments) {
                    colorAttachments.push_back(wgpu::RenderPassColorAttachment{
                        .nextInChain = nullptr,
                        .view = replay.GetObjectById<wgpu::TextureView>(attachment.viewId),
                        .depthSlice = attachment.depthSlice,
                        .resolveTarget =
                            replay.GetObjectById<wgpu::TextureView>(attachment.resolveTargetId),
                        .loadOp = attachment.loadOp,
                        .storeOp = attachment.storeOp,
                        .clearValue = ToWGPU(attachment.clearValue),
                    });
                }

                wgpu::RenderPassDepthStencilAttachment depthStencilAttachment{
                    .view =
                        replay.GetObjectById<wgpu::TextureView>(data.depthStencilAttachment.viewId),
                    .depthLoadOp = data.depthStencilAttachment.depthLoadOp,
                    .depthStoreOp = data.depthStencilAttachment.depthStoreOp,
                    .depthClearValue = data.depthStencilAttachment.depthClearValue,
                    .depthReadOnly = data.depthStencilAttachment.depthReadOnly,
                    .stencilLoadOp = data.depthStencilAttachment.stencilLoadOp,
                    .stencilStoreOp = data.depthStencilAttachment.stencilStoreOp,
                    .stencilClearValue = data.depthStencilAttachment.stencilClearValue,
                    .stencilReadOnly = data.depthStencilAttachment.stencilReadOnly,
                };

                wgpu::RenderPassDescriptor desc{
                    .label = wgpu::StringView(data.label),
                    .colorAttachmentCount = colorAttachments.size(),
                    .colorAttachments = colorAttachments.data(),
                    .depthStencilAttachment =
                        depthStencilAttachment.view != nullptr ? &depthStencilAttachment : nullptr,
                    .occlusionQuerySet =
                        replay.GetObjectById<wgpu::QuerySet>(data.occlusionQuerySetId),
                    .timestampWrites = timestampWrites.querySet ? &timestampWrites : nullptr,
                };
                wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
                DAWN_TRY(ProcessRenderPassCommands(replay, readHead, device, pass));
                break;
            }
            case schema::EncoderCommand::CopyBufferToBuffer: {
                schema::EncoderCommandCopyBufferToBufferCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                encoder.CopyBufferToBuffer(replay.GetObjectById<wgpu::Buffer>(data.srcBufferId),
                                           data.srcOffset,
                                           replay.GetObjectById<wgpu::Buffer>(data.dstBufferId),
                                           data.dstOffset, data.size);
                break;
            }
            case schema::EncoderCommand::CopyBufferToTexture: {
                schema::EncoderCommandCopyBufferToTextureCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::TexelCopyBufferInfo src = ToWGPU(replay, data.source);
                wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, data.destination);
                wgpu::Extent3D copySize = ToWGPU(data.copySize);
                encoder.CopyBufferToTexture(&src, &dst, &copySize);
                break;
            }
            case schema::EncoderCommand::CopyTextureToBuffer: {
                schema::EncoderCommandCopyTextureToBufferCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::TexelCopyTextureInfo src = ToWGPU(replay, data.source);
                wgpu::TexelCopyBufferInfo dst = ToWGPU(replay, data.destination);
                wgpu::Extent3D copySize = ToWGPU(data.copySize);
                encoder.CopyTextureToBuffer(&src, &dst, &copySize);
                break;
            }
            case schema::EncoderCommand::CopyTextureToTexture: {
                schema::EncoderCommandCopyTextureToTextureCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::TexelCopyTextureInfo src = ToWGPU(replay, data.source);
                wgpu::TexelCopyTextureInfo dst = ToWGPU(replay, data.destination);
                wgpu::Extent3D copySize = ToWGPU(data.copySize);
                encoder.CopyTextureToTexture(&src, &dst, &copySize);
                break;
            }
            case schema::EncoderCommand::End: {
                return {};
            }
            default:
                return DAWN_INTERNAL_ERROR("Encoder Command not implemented");
        }
    }
    return DAWN_INTERNAL_ERROR("Missing End command");
}

ResultOrError<wgpu::CommandBuffer> CreateCommandBuffer(const Replay& replay,
                                                       wgpu::Device device,
                                                       ReadHead& readHead,
                                                       const std::string& label) {
    wgpu::CommandEncoderDescriptor desc{
        .label = wgpu::StringView(label),
    };
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&desc);
    DAWN_TRY(ProcessEncoderCommands(replay, readHead, device, encoder));
    return {encoder.Finish()};
}

}  // anonymous namespace

std::unique_ptr<Replay> Replay::Create(wgpu::Device device, const Capture* capture) {
    return std::unique_ptr<Replay>(new Replay(device, capture));
}

Replay::Replay(wgpu::Device device, const Capture* capture) : mDevice(device), mCapture(capture) {}

MaybeError Replay::CreateResource(wgpu::Device device, ReadHead& readHead) {
    schema::LabeledResource resource;
    DAWN_TRY(Deserialize(readHead, &resource));

    switch (resource.type) {
        case schema::ObjectType::BindGroup: {
            wgpu::BindGroup bindGroup;
            DAWN_TRY_ASSIGN(bindGroup, CreateBindGroup(*this, device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, bindGroup}});
            return {};
        }

        case schema::ObjectType::BindGroupLayout: {
            wgpu::BindGroupLayout bindGroupLayout;
            DAWN_TRY_ASSIGN(bindGroupLayout,
                            CreateBindGroupLayout(*this, device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, bindGroupLayout}});
            return {};
        }

        case schema::ObjectType::Buffer: {
            wgpu::Buffer buffer;
            DAWN_TRY_ASSIGN(buffer, CreateBuffer(device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, buffer}});
            return {};
        }

        case schema::ObjectType::CommandBuffer: {
            // Command buffers are special and don't have any resources to create.
            // They are just a sequence of commands.
            wgpu::CommandBuffer commandBuffer;
            DAWN_TRY_ASSIGN(commandBuffer,
                            CreateCommandBuffer(*this, device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, commandBuffer}});
            return {};
        }

        case schema::ObjectType::ComputePipeline: {
            wgpu::ComputePipeline computePipeline;
            DAWN_TRY_ASSIGN(
                computePipeline,
                CreateComputePipeline(
                    *this, device, readHead, resource.label,
                    [this](wgpu::ComputePipeline& computePipeline,
                           const std::vector<schema::BindGroupLayoutIndexIdPair>& groupIndexIds) {
                        // Register any implicit bindgroups.
                        for (const auto& groupIndexId : groupIndexIds) {
                            wgpu::BindGroupLayout bgl =
                                computePipeline.GetBindGroupLayout(groupIndexId.groupIndex);
                            mResources.insert({groupIndexId.bindGroupLayoutId, {"", bgl}});
                        }
                    }));
            mResources.insert({resource.id, {resource.label, computePipeline}});
            return {};
        }

        case schema::ObjectType::PipelineLayout: {
            wgpu::PipelineLayout pipelineLayout;
            DAWN_TRY_ASSIGN(pipelineLayout,
                            CreatePipelineLayout(*this, device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, pipelineLayout}});
            return {};
        }

        case schema::ObjectType::RenderPipeline: {
            wgpu::RenderPipeline renderPipeline;
            DAWN_TRY_ASSIGN(
                renderPipeline,
                CreateRenderPipeline(
                    *this, device, readHead, resource.label,
                    [this](wgpu::RenderPipeline& renderPipeline,
                           const std::vector<schema::BindGroupLayoutIndexIdPair>& groupIndexIds) {
                        // Register any implicit bindgroups.
                        for (const auto& groupIndexId : groupIndexIds) {
                            wgpu::BindGroupLayout bgl =
                                renderPipeline.GetBindGroupLayout(groupIndexId.groupIndex);
                            mResources.insert({groupIndexId.bindGroupLayoutId, {"", bgl}});
                        }
                    }));
            mResources.insert({resource.id, {resource.label, renderPipeline}});
            return {};
        }

        case schema::ObjectType::ShaderModule: {
            wgpu::ShaderModule shaderModule;
            DAWN_TRY_ASSIGN(shaderModule, CreateShaderModule(device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, shaderModule}});
            return {};
        }

        case schema::ObjectType::Texture: {
            wgpu::Texture texture;
            DAWN_TRY_ASSIGN(texture, CreateTexture(device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, texture}});
            return {};
        }

        case schema::ObjectType::TextureView: {
            wgpu::TextureView textureView;
            DAWN_TRY_ASSIGN(textureView, CreateTextureView(*this, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, textureView}});
            return {};
        }

        default:
            return DAWN_INTERNAL_ERROR("unhandled resource type");
    }
}

MaybeError Replay::Play() {
    auto readHead = mCapture->GetCommandReadHead();
    auto contentReadHead = mCapture->GetContentReadHead();
    schema::RootCommand cmd;

    while (!readHead.IsDone()) {
        DAWN_TRY(Deserialize(readHead, &cmd));
        switch (cmd) {
            case schema::RootCommand::CreateResource: {
                DAWN_TRY(CreateResource(mDevice, readHead));
                break;
            }
            case schema::RootCommand::WriteBuffer: {
                schema::RootCommandWriteBufferCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::Buffer buffer = GetObjectById<wgpu::Buffer>(data.bufferId);
                DAWN_TRY(ReadContentIntoBuffer(contentReadHead, mDevice, buffer, data.bufferOffset,
                                               data.size));
                break;
            }
            case schema::RootCommand::WriteTexture: {
                // TODO(451460573): Support textures with multiple subresources
                schema::RootCommandWriteTextureCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                DAWN_TRY(ReadContentIntoTexture(*this, contentReadHead, mDevice, data));
                break;
            }
            case schema::RootCommand::QueueSubmit: {
                schema::RootCommandQueueSubmitCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));

                std::vector<wgpu::CommandBuffer> commandBuffers;
                std::transform(data.commandBuffers.begin(), data.commandBuffers.end(),
                               std::back_inserter(commandBuffers), [&](const auto id) {
                                   return GetObjectById<wgpu::CommandBuffer>(id);
                               });

                mDevice.GetQueue().Submit(commandBuffers.size(), commandBuffers.data());
                break;
            }
            case schema::RootCommand::UnmapBuffer: {
                schema::RootCommandUnmapBufferCmdData data;
                DAWN_TRY(Deserialize(readHead, &data));
                wgpu::Buffer buffer = GetObjectById<wgpu::Buffer>(data.bufferId);
                DAWN_TRY(MapContentIntoBuffer(contentReadHead, mDevice, buffer, data.bufferOffset,
                                              data.size));
                break;
            }
            default: {
                return DAWN_INTERNAL_ERROR("unimplemented root command");
            }
        }
    }

    return {};
}

}  // namespace dawn::replay
