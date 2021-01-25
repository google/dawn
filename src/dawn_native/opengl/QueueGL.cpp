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

#include "dawn_native/opengl/QueueGL.h"

#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/CommandBufferGL.h"
#include "dawn_native/opengl/DeviceGL.h"
#include "dawn_native/opengl/TextureGL.h"
#include "dawn_platform/DawnPlatform.h"
#include "dawn_platform/tracing/TraceEvent.h"

namespace dawn_native { namespace opengl {

    Queue::Queue(Device* device) : QueueBase(device) {
    }

    MaybeError Queue::SubmitImpl(uint32_t commandCount, CommandBufferBase* const* commands) {
        Device* device = ToBackend(GetDevice());

        TRACE_EVENT_BEGIN0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");
        for (uint32_t i = 0; i < commandCount; ++i) {
            DAWN_TRY(ToBackend(commands[i])->Execute());
        }
        TRACE_EVENT_END0(GetDevice()->GetPlatform(), Recording, "CommandBufferGL::Execute");

        device->SubmitFenceSync();
        return {};
    }

    MaybeError Queue::WriteBufferImpl(BufferBase* buffer,
                                      uint64_t bufferOffset,
                                      const void* data,
                                      size_t size) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        ToBackend(buffer)->EnsureDataInitializedAsDestination(bufferOffset, size);

        gl.BindBuffer(GL_ARRAY_BUFFER, ToBackend(buffer)->GetHandle());
        gl.BufferSubData(GL_ARRAY_BUFFER, bufferOffset, size, data);
        return {};
    }

    MaybeError Queue::WriteTextureImpl(const TextureCopyView& destination,
                                       const void* data,
                                       const TextureDataLayout& dataLayout,
                                       const Extent3D& writeSizePixel) {
        const OpenGLFunctions& gl = ToBackend(GetDevice())->gl;

        Texture* texture = ToBackend(destination.texture);
        SubresourceRange range(Aspect::Color, {destination.origin.z, writeSizePixel.depth},
                               {destination.mipLevel, 1});
        if (IsCompleteSubresourceCopiedTo(texture, writeSizePixel, destination.mipLevel)) {
            texture->SetIsSubresourceContentInitialized(true, range);
        } else {
            texture->EnsureSubresourceContentInitialized(range);
        }

        const GLFormat& format = texture->GetGLFormat();
        GLenum target = texture->GetGLTarget();
        data = static_cast<const uint8_t*>(data) + dataLayout.offset;
        gl.BindTexture(target, texture->GetHandle());
        const TexelBlockInfo& blockInfo =
            texture->GetFormat().GetAspectInfo(destination.aspect).block;
        if (dataLayout.bytesPerRow % blockInfo.byteSize == 0) {
            gl.PixelStorei(GL_UNPACK_ROW_LENGTH,
                           dataLayout.bytesPerRow / blockInfo.byteSize * blockInfo.width);
            if (texture->GetArrayLayers() == 1) {
                gl.TexSubImage2D(target, destination.mipLevel, destination.origin.x,
                                 destination.origin.y, writeSizePixel.width, writeSizePixel.height,
                                 format.format, format.type, data);
            } else {
                gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, dataLayout.rowsPerImage * blockInfo.height);
                gl.TexSubImage3D(target, destination.mipLevel, destination.origin.x,
                                 destination.origin.y, destination.origin.z, writeSizePixel.width,
                                 writeSizePixel.height, writeSizePixel.depth, format.format,
                                 format.type, data);
                gl.PixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
            }
            gl.PixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        } else {
            if (texture->GetArrayLayers() == 1) {
                const uint8_t* d = static_cast<const uint8_t*>(data);
                for (uint32_t y = 0; y < writeSizePixel.height; ++y) {
                    gl.TexSubImage2D(target, destination.mipLevel, destination.origin.x,
                                     destination.origin.y + y, writeSizePixel.width, 1,
                                     format.format, format.type, d);
                    d += dataLayout.bytesPerRow;
                }
            } else {
                const uint8_t* slice = static_cast<const uint8_t*>(data);
                for (uint32_t z = 0; z < writeSizePixel.depth; ++z) {
                    const uint8_t* d = slice;
                    for (uint32_t y = 0; y < writeSizePixel.height; ++y) {
                        gl.TexSubImage3D(target, destination.mipLevel, destination.origin.x,
                                         destination.origin.y + y, destination.origin.z + z,
                                         writeSizePixel.width, 1, 1, format.format, format.type, d);
                        d += dataLayout.bytesPerRow;
                    }
                    slice += dataLayout.rowsPerImage * dataLayout.bytesPerRow;
                }
            }
        }
        return {};
    }

}}  // namespace dawn_native::opengl
