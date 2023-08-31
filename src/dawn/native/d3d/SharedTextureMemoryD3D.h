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
