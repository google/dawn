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

#ifndef SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_
#define SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_

#include "dawn/native/Error.h"
#include "dawn/native/d3d/SharedTextureMemoryD3D.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;
struct SharedTextureMemoryD3D11Texture2DDescriptor;

class SharedTextureMemory final : public d3d::SharedTextureMemory {
  public:
    static ResultOrError<Ref<SharedTextureMemory>> Create(
        Device* device,
        const char* label,
        const SharedTextureMemoryDXGISharedHandleDescriptor* descriptor);

    static ResultOrError<Ref<SharedTextureMemory>> Create(
        Device* device,
        const char* label,
        const SharedTextureMemoryD3D11Texture2DDescriptor* descriptor);

    ID3D11Resource* GetD3DResource() const;

  private:
    SharedTextureMemory(Device* device,
                        const char* label,
                        SharedTextureMemoryProperties properties,
                        ComPtr<ID3D11Resource> resource);

    void DestroyImpl() override;

    ResultOrError<Ref<TextureBase>> CreateTextureImpl(const TextureDescriptor* descriptor) override;

    ResultOrError<Ref<SharedFenceBase>> CreateFenceImpl(
        const SharedFenceDXGISharedHandleDescriptor* desc) override;

    ComPtr<ID3D11Resource> mResource;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_SHARED_TEXTURE_MEMORY_D3D11_H_
