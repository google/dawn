// Copyright 2023 The Dawn Authors
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

#include "dawn/native/d3d11/QueueD3D11.h"

#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandBufferD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/TextureD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

Ref<Queue> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    Device* device = ToBackend(GetDevice());

    // CommandBuffer::Execute() will modify the state of the global immediate device context, it may
    // affect following usage of it.
    // TODO(dawn:1770): figure how if we need to track and restore the state of the immediate device
    // context.
    TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D11::Execute");
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->Execute());
    }
    DAWN_TRY(device->ExecutePendingCommandContext());
    TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferD3D11::Execute");

    DAWN_TRY(device->NextSerial());

    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    if (size == 0) {
        // skip the empty write
        return {};
    }

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    return ToBackend(buffer)->Write(commandContext, bufferOffset, data, size);
}

MaybeError Queue::WriteTextureImpl(const ImageCopyTexture& destination,
                                   const void* data,
                                   const TextureDataLayout& dataLayout,
                                   const Extent3D& writeSizePixel) {
    if (writeSizePixel.width == 0 || writeSizePixel.height == 0 ||
        writeSizePixel.depthOrArrayLayers == 0) {
        return {};
    }

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();

    TextureCopy textureCopy;
    textureCopy.texture = destination.texture;
    textureCopy.mipLevel = destination.mipLevel;
    textureCopy.origin = destination.origin;
    textureCopy.aspect = SelectFormatAspects(destination.texture->GetFormat(), destination.aspect);

    SubresourceRange subresources = GetSubresourcesAffectedByCopy(textureCopy, writeSizePixel);

    Texture* texture = ToBackend(destination.texture);

    return texture->Write(commandContext, subresources, destination.origin, writeSizePixel,
                          static_cast<const uint8_t*>(data) + dataLayout.offset,
                          dataLayout.bytesPerRow, dataLayout.rowsPerImage);
}

bool Queue::HasPendingCommands() const {
    return ToBackend(GetDevice())->HasPendingCommands();
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    return ToBackend(GetDevice())->CheckAndUpdateCompletedSerials();
}

void Queue::ForceEventualFlushOfCommands() {
    return ToBackend(GetDevice())->ForceEventualFlushOfCommands();
}

MaybeError Queue::WaitForIdleForDestruction() {
    return ToBackend(GetDevice())->WaitForIdleForDestruction();
}

}  // namespace dawn::native::d3d11
