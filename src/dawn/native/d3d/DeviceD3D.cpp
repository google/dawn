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

#include "dawn/native/d3d/DeviceD3D.h"

#include "dawn/native/d3d/BackendD3D.h"
#include "dawn/native/d3d/ExternalImageDXGIImpl.h"
#include "dawn/native/d3d/Forward.h"
#include "dawn/native/d3d/PhysicalDeviceD3D.h"

namespace dawn::native::d3d {

Device::Device(AdapterBase* adapter,
               const DeviceDescriptor* descriptor,
               const TogglesState& deviceToggles)
    : DeviceBase(adapter, descriptor, deviceToggles) {}

Device::~Device() {
    Destroy();

    // Close the handle here instead of in DestroyImpl. The handle is returned from
    // ExternalImageDXGI, so it needs to live as long as the Device ref does, even if the device
    // state is destroyed.
    if (mFenceHandle != nullptr) {
        ::CloseHandle(mFenceHandle);
        mFenceHandle = nullptr;
    }
}

void Device::DestroyImpl() {
    while (!mExternalImageList.empty()) {
        d3d::ExternalImageDXGIImpl* externalImage = mExternalImageList.head()->value();
        // ExternalImageDXGIImpl::DestroyInternal() calls RemoveFromList().
        externalImage->DestroyInternal();
    }
}

ResultOrError<wgpu::TextureUsage> Device::GetSupportedSurfaceUsageImpl(
    const Surface* surface) const {
    wgpu::TextureUsage usages = wgpu::TextureUsage::RenderAttachment |
                                wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                                wgpu::TextureUsage::CopyDst;
    return usages;
}

const PlatformFunctions* Device::GetFunctions() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetFunctions();
}

ComPtr<IDXGIFactory4> Device::GetFactory() const {
    return ToBackend(GetPhysicalDevice())->GetBackend()->GetFactory();
}

HANDLE Device::GetFenceHandle() const {
    return mFenceHandle;
}

std::unique_ptr<ExternalImageDXGIImpl> Device::CreateExternalImageDXGIImpl(
    const ExternalImageDescriptor* descriptor) {
    std::unique_ptr<ExternalImageDXGIImpl> externalImage;
    if (!ConsumedError(CreateExternalImageDXGIImplImpl(descriptor), &externalImage)) {
        mExternalImageList.Append(externalImage.get());
        return externalImage;
    }
    return {};
}

}  // namespace dawn::native::d3d
