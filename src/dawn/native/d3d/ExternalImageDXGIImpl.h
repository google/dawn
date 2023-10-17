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
