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
