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

#ifndef SRC_DAWN_NATIVE_OPENGL_BUFFERGL_H_
#define SRC_DAWN_NATIVE_OPENGL_BUFFERGL_H_

#include "dawn/native/Buffer.h"

#include "dawn/native/opengl/opengl_platform.h"

namespace dawn::native::opengl {

class Device;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> CreateInternalBuffer(Device* device,
                                                           const BufferDescriptor* descriptor,
                                                           bool shouldLazyClear);

    Buffer(Device* device, const BufferDescriptor* descriptor);

    GLuint GetHandle() const;

    bool EnsureDataInitialized();
    bool EnsureDataInitializedAsDestination(uint64_t offset, uint64_t size);
    bool EnsureDataInitializedAsDestination(const CopyTextureToBufferCmd* copy);

    void TrackUsage() { MarkUsedInPendingCommands(); }

  private:
    Buffer(Device* device, const BufferDescriptor* descriptor, bool shouldLazyClear);
    ~Buffer() override;
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    void InitializeToZero();

    GLuint mBuffer = 0;
    void* mMappedData = nullptr;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_BUFFERGL_H_
