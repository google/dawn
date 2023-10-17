// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D_SHARED_TEXTURE_MEMORY_D3D_H_
#define SRC_DAWN_NATIVE_D3D_SHARED_TEXTURE_MEMORY_D3D_H_

#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class Device;

class SharedTextureMemory : public SharedTextureMemoryBase {
  protected:
    SharedTextureMemory(Device* device,
                        const char* label,
                        SharedTextureMemoryProperties properties,
                        IUnknown* resource);

  protected:
    MaybeError BeginAccessImpl(TextureBase* texture,
                               const BeginAccessDescriptor* descriptor) override;
    ResultOrError<FenceAndSignalValue> EndAccessImpl(TextureBase* texture) override;

  private:
    virtual ResultOrError<Ref<SharedFenceBase>> CreateFenceImpl(
        const SharedFenceDXGISharedHandleDescriptor* desc) = 0;

    // If the resource has IDXGIKeyedMutex interface, it will be used for synchronization.
    // TODO(dawn:1906): remove the mDXGIKeyedMutex when it is not used in chrome.
    ComPtr<IDXGIKeyedMutex> mDXGIKeyedMutex;
    // Chrome uses 0 as acquire key.
    static constexpr UINT64 kDXGIKeyedMutexAcquireKey = 0;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_SHARED_TEXTURE_MEMORY_D3D_H_
