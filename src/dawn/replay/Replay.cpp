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

#include "dawn/replay/Deserialization.h"

namespace dawn::replay {

namespace {

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

}  // anonymous namespace

std::unique_ptr<Replay> Replay::Create(wgpu::Device device, const Capture* capture) {
    return std::unique_ptr<Replay>(new Replay(device, capture));
}

Replay::Replay(wgpu::Device device, const Capture* capture) : mDevice(device), mCapture(capture) {}

MaybeError Replay::CreateResource(wgpu::Device device, ReadHead& readHead) {
    schema::LabeledResource resource;
    DAWN_TRY(Deserialize(readHead, &resource));

    switch (resource.type) {
        case schema::ObjectType::Buffer: {
            wgpu::Buffer buffer;
            DAWN_TRY_ASSIGN(buffer, CreateBuffer(device, readHead, resource.label));
            mResources.insert({resource.id, {resource.label, buffer}});
            return {};
        }

        default:
            // UNIMPLEMENTED();
            break;
    }
    return {};
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
