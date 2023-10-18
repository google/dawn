// Copyright 2019 The Dawn & Tint Authors
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

// MetalBackend.cpp: contains the definition of symbols exported by MetalBackend.h so that they
// can be compiled twice: once export (shared library), once not exported (static library)

#include "dawn/native/MetalBackend.h"

#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/QueueMTL.h"
#include "dawn/native/metal/TextureMTL.h"

namespace dawn::native::metal {

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
    auto deviceLock(backendDevice->GetScopedLock());
    Ref<TextureBase> texture = backendDevice->CreateTextureWrappingIOSurface(
        cDescriptor, cDescriptor->ioSurface, std::move(waitEvents));
    return ToAPI(texture.Detach());
}

void IOSurfaceEndAccess(WGPUTexture cTexture,
                        ExternalImageIOSurfaceEndAccessDescriptor* descriptor) {
    Texture* texture = ToBackend(FromAPI(cTexture));
    auto device = texture->GetDevice();
    auto deviceLock(device->GetScopedLock());
    texture->IOSurfaceEndAccess(descriptor);
}

void WaitForCommandsToBeScheduled(WGPUDevice device) {
    Device* backendDevice = ToBackend(FromAPI(device));
    auto deviceLock(backendDevice->GetScopedLock());
    ToBackend(backendDevice->GetQueue())->WaitForCommandsToBeScheduled();
}

}  // namespace dawn::native::metal
