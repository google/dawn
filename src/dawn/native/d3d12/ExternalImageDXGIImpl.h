// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_EXTERNALIMAGEDXGIIMPL_H_
#define SRC_DAWN_NATIVE_D3D12_EXTERNALIMAGEDXGIIMPL_H_

#include <wrl/client.h>

#include <memory>

#include "dawn/common/LinkedList.h"
#include "dawn/dawn_wsi.h"
#include "dawn/native/Forward.h"

struct ID3D12Resource;
struct ID3D12Fence;

namespace dawn::native::d3d12 {

class D3D11on12ResourceCache;
class Device;
struct ExternalImageAccessDescriptorDXGISharedHandle;
struct ExternalImageDescriptorDXGISharedHandle;

class ExternalImageDXGIImpl : public LinkNode<ExternalImageDXGIImpl> {
  public:
    ExternalImageDXGIImpl(Device* backendDevice,
                          Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
                          Microsoft::WRL::ComPtr<ID3D12Fence> d3d12Fence,
                          const WGPUTextureDescriptor* descriptor);
    ~ExternalImageDXGIImpl();

    ExternalImageDXGIImpl(const ExternalImageDXGIImpl&) = delete;
    ExternalImageDXGIImpl& operator=(const ExternalImageDXGIImpl&) = delete;

    void Destroy();

    bool IsValid() const;

    WGPUTexture ProduceTexture(const ExternalImageAccessDescriptorDXGISharedHandle* descriptor);

  private:
    Device* mBackendDevice;
    Microsoft::WRL::ComPtr<ID3D12Resource> mD3D12Resource;
    Microsoft::WRL::ComPtr<ID3D12Fence> mD3D12Fence;
    std::unique_ptr<D3D11on12ResourceCache> mD3D11on12ResourceCache;

    // Contents of WGPUTextureDescriptor are stored individually since the descriptor
    // could outlive this image.
    WGPUTextureUsageFlags mUsage;
    WGPUTextureUsageFlags mUsageInternal = WGPUTextureUsage_None;
    WGPUTextureDimension mDimension;
    WGPUExtent3D mSize;
    WGPUTextureFormat mFormat;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_EXTERNALIMAGEDXGIIMPL_H_
