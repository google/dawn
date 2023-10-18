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

#ifndef INCLUDE_DAWN_NATIVE_D3DBACKEND_H_
#define INCLUDE_DAWN_NATIVE_D3DBACKEND_H_

#include <dxgi1_4.h>
#include <windows.h>
#include <wrl/client.h>

#include <memory>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp_chained_struct.h"

namespace dawn::native::d3d {

class ExternalImageDXGIImpl;

DAWN_NATIVE_EXPORT Microsoft::WRL::ComPtr<IDXGIAdapter> GetDXGIAdapter(WGPUAdapter adapter);

// Can be chained in WGPURequestAdapterOptions
struct DAWN_NATIVE_EXPORT RequestAdapterOptionsLUID : wgpu::ChainedStruct {
    RequestAdapterOptionsLUID();

    ::LUID adapterLUID;
};

struct DAWN_NATIVE_EXPORT ExternalImageDescriptorDXGISharedHandle : ExternalImageDescriptor {
  public:
    ExternalImageDescriptorDXGISharedHandle();

    // Note: SharedHandle must be a handle to a texture object.
    HANDLE sharedHandle = nullptr;
};

struct DAWN_NATIVE_EXPORT ExternalImageDescriptorD3D11Texture : ExternalImageDescriptor {
  public:
    ExternalImageDescriptorD3D11Texture();

    // Texture is used for creating ExternalImageDXGI with d3d11 backend. It must be an
    // ID3D11Texture2D object and created from the same ID3D11Device used in the WGPUDevice.
    Microsoft::WRL::ComPtr<IUnknown> texture;
};

struct DAWN_NATIVE_EXPORT ExternalImageDXGIFenceDescriptor {
    // Shared handle for the fence. This never passes ownership to the callee (when used as an input
    // parameter) or to the caller (when used as a return value or output parameter).
    HANDLE fenceHandle = nullptr;

    // The value that was previously signaled on this fence and should be waited on.
    uint64_t fenceValue = 0;
};

struct DAWN_NATIVE_EXPORT ExternalImageDXGIBeginAccessDescriptor {
    bool isInitialized = false;  // Whether the texture is initialized on import
    WGPUTextureUsageFlags usage = WGPUTextureUsage_None;

    // A list of fences to wait on before accessing the texture.
    std::vector<ExternalImageDXGIFenceDescriptor> waitFences;

    // Whether the texture is for a WebGPU swap chain.
    bool isSwapChainTexture = false;
};

class DAWN_NATIVE_EXPORT ExternalImageDXGI {
  public:
    ~ExternalImageDXGI();

    static std::unique_ptr<ExternalImageDXGI> Create(WGPUDevice device,
                                                     const ExternalImageDescriptor* descriptor);

    // Returns true if the external image resources are still valid, otherwise BeginAccess() is
    // guaranteed to fail e.g. after device destruction.
    bool IsValid() const;

    // Creates WGPUTexture wrapping the DXGI shared handle. The provided wait fences will be
    // synchronized before using the texture in any command lists. Empty fences (nullptr handle) are
    // ignored for convenience (EndAccess can return such fences).
    WGPUTexture BeginAccess(const ExternalImageDXGIBeginAccessDescriptor* descriptor);

    // Returns the signalFence that the client must wait on for correct synchronization. Can return
    // an empty fence (nullptr handle) if the texture wasn't accessed by Dawn.
    // Note that merely calling Destroy() on the WGPUTexture does not ensure synchronization.
    void EndAccess(WGPUTexture texture, ExternalImageDXGIFenceDescriptor* signalFence);

  private:
    explicit ExternalImageDXGI(std::unique_ptr<ExternalImageDXGIImpl> impl);

    std::unique_ptr<ExternalImageDXGIImpl> mImpl;
};

}  // namespace dawn::native::d3d

#endif  // INCLUDE_DAWN_NATIVE_D3DBACKEND_H_
