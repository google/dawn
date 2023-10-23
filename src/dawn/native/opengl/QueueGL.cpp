// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/opengl/QueueGL.h"

#include "dawn/native/opengl/BufferGL.h"
#include "dawn/native/opengl/CommandBufferGL.h"
#include "dawn/native/opengl/DeviceGL.h"
#include "dawn/native/opengl/TextureGL.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::opengl {

ResultOrError<Ref<Queue>> Queue::Create(Device* device, const QueueDescriptor* descriptor) {
    return AcquireRef(new Queue(device, descriptor));
}

Queue::Queue(Device* device, const QueueDescriptor* descriptor) : QueueBase(device, descriptor) {}

MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
    TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");
    for (uint32_t i = 0; i < commandCount; ++i) {
        DAWN_TRY(ToBackend(commands[i])->Execute());
    }
    TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");

    return {};
}

MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                  uint64_t bufferOffset,
                                  const void* data,
                                  size_t size) {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();

    ToBackend(buffer)->EnsureDataInitializedAsDestination(bufferOffset, size);

    gl.BindBuffer(GL_ARRAY_BUFFER, ToBackend(buffer)->GetHandle());
    gl.BufferSubData(GL_ARRAY_BUFFER, bufferOffset, size, data);
    buffer->MarkUsedInPendingCommands();
    return {};
}

MaybeError Queue::WriteTextureImpl(const ImageCopyTexture& destination,
                                   const void* data,
                                   const TextureDataLayout& dataLayout,
                                   const Extent3D& writeSizePixel) {
    DAWN_INVALID_IF(destination.aspect == wgpu::TextureAspect::StencilOnly,
                    "Writes to stencil textures unsupported on the OpenGL backend.");

    TextureCopy textureCopy;
    textureCopy.texture = destination.texture;
    textureCopy.mipLevel = destination.mipLevel;
    textureCopy.origin = destination.origin;
    textureCopy.aspect = SelectFormatAspects(destination.texture->GetFormat(), destination.aspect);

    SubresourceRange range = GetSubresourcesAffectedByCopy(textureCopy, writeSizePixel);
    if (IsCompleteSubresourceCopiedTo(destination.texture, writeSizePixel, destination.mipLevel,
                                      destination.aspect)) {
        destination.texture->SetIsSubresourceContentInitialized(true, range);
    } else {
        DAWN_TRY(ToBackend(destination.texture)->EnsureSubresourceContentInitialized(range));
    }
    DoTexSubImage(ToBackend(GetDevice())->GetGL(), textureCopy, data, dataLayout, writeSizePixel);
    ToBackend(destination.texture)->Touch();
    return {};
}

void Queue::OnGLUsed() {
    mHasPendingCommands = true;
}

void Queue::SubmitFenceSync() {
    if (!mHasPendingCommands) {
        return;
    }

    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    GLsync sync = gl.FenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    IncrementLastSubmittedCommandSerial();
    mFencesInFlight.emplace(sync, GetLastSubmittedCommandSerial());
    mHasPendingCommands = false;
}

bool Queue::HasPendingCommands() const {
    return mHasPendingCommands;
}

ResultOrError<ExecutionSerial> Queue::CheckAndUpdateCompletedSerials() {
    const Device* device = ToBackend(GetDevice());
    const OpenGLFunctions& gl = device->GetGL();

    ExecutionSerial fenceSerial{0};
    while (!mFencesInFlight.empty()) {
        auto [sync, tentativeSerial] = mFencesInFlight.front();

        // Fence are added in order, so we can stop searching as soon
        // as we see one that's not ready.

        // TODO(crbug.com/dawn/633): Remove this workaround after the deadlock issue is fixed.
        if (device->IsToggleEnabled(Toggle::FlushBeforeClientWaitSync)) {
            gl.Flush();
        }
        GLenum result = gl.ClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        if (result == GL_TIMEOUT_EXPIRED) {
            return fenceSerial;
        }
        // Update fenceSerial since fence is ready.
        fenceSerial = tentativeSerial;

        gl.DeleteSync(sync);

        mFencesInFlight.pop();

        DAWN_ASSERT(fenceSerial > GetCompletedCommandSerial());
    }
    return fenceSerial;
}

void Queue::ForceEventualFlushOfCommands() {
    mHasPendingCommands = true;
}

MaybeError Queue::WaitForIdleForDestruction() {
    const OpenGLFunctions& gl = ToBackend(GetDevice())->GetGL();
    gl.Finish();
    DAWN_TRY(CheckPassedSerials());
    DAWN_ASSERT(mFencesInFlight.empty());
    return {};
}

}  // namespace dawn::native::opengl
