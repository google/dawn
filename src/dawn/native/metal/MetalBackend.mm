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

#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/TextureMTL.h"

namespace dawn::native::metal {

id<MTLDevice> GetMetalDevice(WGPUDevice device) {
    return ToBackend(FromAPI(device))->GetMTLDevice();
}

AdapterDiscoveryOptions::AdapterDiscoveryOptions()
    : AdapterDiscoveryOptionsBase(WGPUBackendType_Metal) {}

ExternalImageDescriptorIOSurface::ExternalImageDescriptorIOSurface()
    : ExternalImageDescriptor(ExternalImageType::IOSurface) {}

WGPUTexture WrapIOSurface(WGPUDevice device, const ExternalImageDescriptorIOSurface* cDescriptor) {
    Device* backendDevice = ToBackend(FromAPI(device));
    Ref<TextureBase> texture =
        backendDevice->CreateTextureWrappingIOSurface(cDescriptor, cDescriptor->ioSurface);
    return ToAPI(texture.Detach());
}

void WaitForCommandsToBeScheduled(WGPUDevice device) {
    ToBackend(FromAPI(device))->WaitForCommandsToBeScheduled();
}

}  // namespace dawn::native::metal
