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

wgpu::BindGroupEntry ToWGPU(const Replay& replay, const schema::BindGroupEntry& entry) {
    return wgpu::BindGroupEntry{
        .binding = entry.binding,
        .buffer = replay.GetObjectById<wgpu::Buffer>(entry.bufferId),
        .offset = entry.offset,
        .size = entry.size,
        .sampler = replay.GetObjectById<wgpu::Sampler>(entry.samplerId),
        .textureView = replay.GetObjectById<wgpu::TextureView>(entry.textureViewId),
    };
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
    std::transform(bg.entries.begin(), bg.entries.end(), std::back_inserter(entries),
                   [&](const auto& entry) { return ToWGPU(replay, entry); });

    wgpu::BindGroupDescriptor desc{
        .label = wgpu::StringView(label),
        .layout = replay.GetObjectById<wgpu::BindGroupLayout>(bg.layoutId),
        .entryCount = entries.size(),
        .entries = entries.data(),
    };
    wgpu::BindGroup bindGroup = device.CreateBindGroup(&desc);
    return {bindGroup};
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
                // UNIMPLEMENTED();
                break;
            }
        }
    }

    return {};
}

}  // namespace dawn::replay
