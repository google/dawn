// Copyright 2018 The Dawn & Tint Authors
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

#ifndef INCLUDE_DAWN_NATIVE_METALBACKEND_H_
#define INCLUDE_DAWN_NATIVE_METALBACKEND_H_

#include <vector>

#include "dawn/native/DawnNative.h"

// The specifics of the Metal backend expose types in function signatures that might not be
// available in dependent's minimum supported SDK version. Suppress all availability errors using
// clang's pragmas. Dependents using the types without guarded availability will still get errors
// when using the types.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"

struct __IOSurface;
typedef __IOSurface* IOSurfaceRef;

#ifdef __OBJC__
#import <Metal/Metal.h>
#endif  // __OBJC__

namespace dawn::native::metal {

struct DAWN_NATIVE_EXPORT ExternalImageMTLSharedEventDescriptor {
    // Shared event handle `id<MTLSharedEvent>`.
    // This never passes ownership to the callee (when used as an input
    // parameter) or to the caller (when used as a return value or output parameter).
#ifdef __OBJC__
    id<MTLSharedEvent> sharedEvent = nil;
    static_assert(sizeof(id<MTLSharedEvent>) == sizeof(void*));
    static_assert(alignof(id<MTLSharedEvent>) == alignof(void*));
#else
    void* sharedEvent = nullptr;
#endif

    // The value that was previously signaled on this event and should be waited on.
    uint64_t signaledValue = 0;
};

struct DAWN_NATIVE_EXPORT ExternalImageDescriptorIOSurface : ExternalImageDescriptor {
  public:
    ExternalImageDescriptorIOSurface();
    ~ExternalImageDescriptorIOSurface();

    IOSurfaceRef ioSurface;

    // A list of events to wait on before accessing the texture.
    std::vector<ExternalImageMTLSharedEventDescriptor> waitEvents;
};

struct DAWN_NATIVE_EXPORT ExternalImageIOSurfaceEndAccessDescriptor
    : ExternalImageMTLSharedEventDescriptor {
    bool isInitialized;
};

DAWN_NATIVE_EXPORT WGPUTexture WrapIOSurface(WGPUDevice device,
                                             const ExternalImageDescriptorIOSurface* descriptor);

DAWN_NATIVE_EXPORT void IOSurfaceEndAccess(WGPUTexture texture,
                                           ExternalImageIOSurfaceEndAccessDescriptor* descriptor);

// When making Metal interop with other APIs, we need to be careful that QueueSubmit doesn't
// mean that the operations will be visible to other APIs/Metal devices right away. macOS
// does have a global queue of graphics operations, but the command buffers are inserted there
// when they are "scheduled". Submitting other operations before the command buffer is
// scheduled could lead to races in who gets scheduled first and incorrect rendering.
DAWN_NATIVE_EXPORT void WaitForCommandsToBeScheduled(WGPUDevice device);

}  // namespace dawn::native::metal

#pragma clang diagnostic pop

#endif  // INCLUDE_DAWN_NATIVE_METALBACKEND_H_
