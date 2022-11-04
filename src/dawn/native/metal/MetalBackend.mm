// Copyright 2019 The Dawn Authors
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

// MetalBackend.cpp: contains the definition of symbols exported by MetalBackend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/MetalBackend.h"

#include "dawn/native/metal/CommandRecordingContext.h"
#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/TextureMTL.h"

namespace dawn::native::metal {

AdapterDiscoveryOptions::AdapterDiscoveryOptions()
    : AdapterDiscoveryOptionsBase(WGPUBackendType_Metal) {}

ExternalImageDescriptorIOSurface::ExternalImageDescriptorIOSurface()
    : ExternalImageDescriptor(ExternalImageType::IOSurface) {}

ExternalImageDescriptorIOSurface::~ExternalImageDescriptorIOSurface() = default;

WGPUTexture WrapIOSurface(WGPUDevice device, const ExternalImageDescriptorIOSurface* cDescriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));
    std::vector<MTLSharedEventAndSignalValue> waitEvents;
    for (const auto& waitEvent : cDescriptor->waitEvents) {
        waitEvents.push_back(
            {static_cast<id<MTLSharedEvent>>(waitEvent.sharedEvent), waitEvent.signaledValue});
    }
    Ref<TextureBase> texture = backendDevice->CreateTextureWrappingIOSurface(
        cDescriptor, cDescriptor->ioSurface, std::move(waitEvents));
    return ToAPI(texture.Detach());
}

void IOSurfaceEndAccess(WGPUTexture cTexture,
                        ExternalImageIOSurfaceEndAccessDescriptor* descriptor) {
    Texture* texture = ToBackend(FromAPI(cTexture));
    texture->IOSurfaceEndAccess(descriptor);
}

void WaitForCommandsToBeScheduled(WGPUDevice device) {
    ToBackend(FromAPI(device))->WaitForCommandsToBeScheduled();
}

}  // namespace dawn::native::metal
