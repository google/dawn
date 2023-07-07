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

#ifndef SRC_DAWN_NATIVE_D3D_EXTERNALIMAGEDXGIIMPL_H_
#define SRC_DAWN_NATIVE_D3D_EXTERNALIMAGEDXGIIMPL_H_

#include <wrl/client.h>

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "dawn/common/LinkedList.h"
#include "dawn/common/Mutex.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/webgpu_cpp.h"

namespace dawn::native::d3d {

class Device;
struct ExternalImageDXGIBeginAccessDescriptor;
struct ExternalImageDXGIFenceDescriptor;
struct ExternalImageDescriptorDXGISharedHandle;

MaybeError ValidateTextureDescriptorCanBeWrapped(const TextureDescriptor* descriptor);

class ExternalImageDXGIImpl : public LinkNode<ExternalImageDXGIImpl> {
  public:
    ExternalImageDXGIImpl(Device* backendDevice,
                          ComPtr<IUnknown> d3dResource,
                          const TextureDescriptor* textureDescriptor);
    ~ExternalImageDXGIImpl();

    ExternalImageDXGIImpl(const ExternalImageDXGIImpl&) = delete;
    ExternalImageDXGIImpl& operator=(const ExternalImageDXGIImpl&) = delete;

    bool IsValid() const;

    WGPUTexture BeginAccess(const ExternalImageDXGIBeginAccessDescriptor* descriptor);
    void EndAccess(WGPUTexture texture, ExternalImageDXGIFenceDescriptor* signalFence);

    // This method should only be called by internal code. Don't call this from D3D12Backend side,
    // or without locking.
    void DestroyInternal();

    [[nodiscard]] Mutex::AutoLock GetScopedDeviceLock() const;

  protected:
    Ref<DeviceBase> mBackendDevice;
    ComPtr<IUnknown> mD3DResource;
    ComPtr<IDXGIKeyedMutex> mDXGIKeyedMutex;
    wgpu::TextureUsage mUsage;
    wgpu::TextureUsage mUsageInternal = wgpu::TextureUsage::None;
    wgpu::TextureDimension mDimension;
    Extent3D mSize;
    wgpu::TextureFormat mFormat;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
    std::vector<wgpu::TextureFormat> mViewFormats;
    uint32_t mAccessCount = 0;

    // Chrome uses 0 as acquire key.
    static constexpr UINT64 kDXGIKeyedMutexAcquireKey = 0;
    class KeyedMutexReleaser : public NonCopyable {
      public:
        explicit KeyedMutexReleaser(ComPtr<IDXGIKeyedMutex> keyedMutex)
            : mDXGIKeyedMutex(std::move(keyedMutex)) {}
        ~KeyedMutexReleaser() { mDXGIKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireKey); }

      private:
        const ComPtr<IDXGIKeyedMutex> mDXGIKeyedMutex;
    };
    std::optional<KeyedMutexReleaser> mDXGIKeyedMutexReleaser;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_EXTERNALIMAGEDXGIIMPL_H_
