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
#include <vector>

#include "dawn/common/LinkedList.h"
#include "dawn/dawn_wsi.h"
#include "dawn/native/D3D12Backend.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/FenceD3D12.h"
#include "dawn/webgpu_cpp.h"

struct ID3D12Resource;
struct ID3D12Fence;

namespace dawn::native::d3d12 {

class D3D11on12ResourceCache;
class Device;
struct ExternalImageDXGIBeginAccessDescriptor;
struct ExternalImageDescriptorDXGISharedHandle;

class ExternalImageDXGIImpl : public LinkNode<ExternalImageDXGIImpl> {
  public:
    ExternalImageDXGIImpl(Device* backendDevice,
                          Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource,
                          const TextureDescriptor* textureDescriptor,
                          bool useFenceSynchronization);
    ~ExternalImageDXGIImpl();

    ExternalImageDXGIImpl(const ExternalImageDXGIImpl&) = delete;
    ExternalImageDXGIImpl& operator=(const ExternalImageDXGIImpl&) = delete;

    void Destroy();

    bool IsValid() const;

    WGPUTexture BeginAccess(const ExternalImageDXGIBeginAccessDescriptor* descriptor);

    void EndAccess(WGPUTexture texture, ExternalImageDXGIFenceDescriptor* signalFence);

  private:
    Ref<Device> mBackendDevice;
    Microsoft::WRL::ComPtr<ID3D12Resource> mD3D12Resource;
    const bool mUseFenceSynchronization;

    std::unique_ptr<D3D11on12ResourceCache> mD3D11on12ResourceCache;

    wgpu::TextureUsage mUsage;
    wgpu::TextureUsage mUsageInternal = wgpu::TextureUsage::None;
    wgpu::TextureDimension mDimension;
    Extent3D mSize;
    wgpu::TextureFormat mFormat;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
    std::vector<wgpu::TextureFormat> mViewFormats;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_EXTERNALIMAGEDXGIIMPL_H_
