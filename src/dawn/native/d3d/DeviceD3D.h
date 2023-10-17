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

#ifndef SRC_DAWN_NATIVE_D3D_DEVICED3D_H_
#define SRC_DAWN_NATIVE_D3D_DEVICED3D_H_

#include <memory>
#include <vector>

#include "dawn/native/Device.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

struct ExternalImageDescriptorDXGISharedHandle;
struct ExternalImageDXGIFenceDescriptor;
class ExternalImageDXGIImpl;
class Fence;
class PlatformFunctions;

class Device : public DeviceBase {
  public:
    Device(AdapterBase* adapter,
           const DeviceDescriptor* descriptor,
           const TogglesState& deviceToggles);
    ~Device() override;

    ResultOrError<wgpu::TextureUsage> GetSupportedSurfaceUsageImpl(
        const Surface* surface) const override;

    const PlatformFunctions* GetFunctions() const;
    ComPtr<IDXGIFactory4> GetFactory() const;

    HANDLE GetFenceHandle() const;

    std::unique_ptr<ExternalImageDXGIImpl> CreateExternalImageDXGIImpl(
        const ExternalImageDescriptor* descriptor);

    virtual ResultOrError<Ref<Fence>> CreateFence(
        const ExternalImageDXGIFenceDescriptor* descriptor) = 0;
    virtual Ref<TextureBase> CreateD3DExternalTexture(const TextureDescriptor* descriptor,
                                                      ComPtr<IUnknown> d3dTexture,
                                                      std::vector<Ref<Fence>> waitFences,
                                                      bool isSwapChainTexture,
                                                      bool isInitialized) = 0;

  protected:
    void DestroyImpl() override;

    virtual ResultOrError<std::unique_ptr<ExternalImageDXGIImpl>> CreateExternalImageDXGIImplImpl(
        const ExternalImageDescriptor* descriptor) = 0;

    HANDLE mFenceHandle = nullptr;

  private:
    // List of external image resources opened using this device.
    LinkedList<d3d::ExternalImageDXGIImpl> mExternalImageList;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_DEVICED3D_H_
