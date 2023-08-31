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

#include "dawn/native/d3d/SharedTextureMemoryD3D.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d/Forward.h"

namespace dawn::native::d3d {

SharedTextureMemory::SharedTextureMemory(d3d::Device* device,
                                         const char* label,
                                         SharedTextureMemoryProperties properties,
                                         IUnknown* resource)
    : SharedTextureMemoryBase(device, label, properties) {
    // If the resource has IDXGIKeyedMutex interface, it will be used for synchronization.
    // TODO(dawn:1906): remove the mDXGIKeyedMutex when it is not used in chrome.
    resource->QueryInterface(IID_PPV_ARGS(&mDXGIKeyedMutex));
}

MaybeError SharedTextureMemory::BeginAccessImpl(TextureBase* texture,
                                                const BeginAccessDescriptor* descriptor) {
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        SharedFenceBase* fence = descriptor->fences[i];

        SharedFenceExportInfo exportInfo;
        DAWN_TRY(fence->ExportInfo(&exportInfo));
        switch (exportInfo.type) {
            case wgpu::SharedFenceType::DXGISharedHandle:
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                                "Required feature (%s) for %s is missing.",
                                wgpu::FeatureName::SharedFenceDXGISharedHandle,
                                wgpu::SharedFenceType::DXGISharedHandle);
                break;
            default:
                return DAWN_VALIDATION_ERROR("Unsupported fence type %s.", exportInfo.type);
        }
    }

    if (mDXGIKeyedMutex) {
        DAWN_TRY(CheckHRESULT(mDXGIKeyedMutex->AcquireSync(kDXGIKeyedMutexAcquireKey, INFINITE),
                              "Acquire keyed mutex"));
    }
    return {};
}

ResultOrError<FenceAndSignalValue> SharedTextureMemory::EndAccessImpl(TextureBase* texture) {
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                    "Required feature (%s) is missing.",
                    wgpu::FeatureName::SharedFenceDXGISharedHandle);

    if (mDXGIKeyedMutex) {
        mDXGIKeyedMutex->ReleaseSync(kDXGIKeyedMutexAcquireKey);
    }

    SharedFenceDXGISharedHandleDescriptor desc;
    desc.handle = ToBackend(GetDevice())->GetFenceHandle();

    Ref<SharedFenceBase> fence;
    DAWN_TRY_ASSIGN(fence, CreateFenceImpl(&desc));

    return FenceAndSignalValue{
        std::move(fence),
        static_cast<uint64_t>(texture->GetSharedTextureMemoryState()->GetLastUsageSerial())};
}

}  // namespace dawn::native::d3d
