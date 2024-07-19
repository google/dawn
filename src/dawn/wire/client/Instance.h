// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
#define SRC_DAWN_WIRE_CLIENT_INSTANCE_H_

#include "absl/container/flat_hash_set.h"
#include "dawn/common/RefCountedWithExternalCount.h"
#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Instance final : public RefCountedWithExternalCount<ObjectWithEventsBase> {
  public:
    explicit Instance(const ObjectBaseParams& params);

    ObjectType GetObjectType() const override;

    WireResult Initialize(const WGPUInstanceDescriptor* descriptor);

    void RequestAdapter(const WGPURequestAdapterOptions* options,
                        WGPURequestAdapterCallback callback,
                        void* userdata);
    WGPUFuture RequestAdapterF(const WGPURequestAdapterOptions* options,
                               const WGPURequestAdapterCallbackInfo& callbackInfo);
    WGPUFuture RequestAdapter2(const WGPURequestAdapterOptions* options,
                               const WGPURequestAdapterCallbackInfo2& callbackInfo);

    void ProcessEvents();
    WGPUWaitStatus WaitAny(size_t count, WGPUFutureWaitInfo* infos, uint64_t timeoutNS);

    bool HasWGSLLanguageFeature(WGPUWGSLFeatureName feature) const;
    // Always writes the full list when features is not nullptr.
    // TODO(https://github.com/webgpu-native/webgpu-headers/issues/252): Add a count argument.
    size_t EnumerateWGSLLanguageFeatures(WGPUWGSLFeatureName* features) const;

    WGPUSurface CreateSurface(const WGPUSurfaceDescriptor* desc) const;

  private:
    void WillDropLastExternalRef() override;
    void GatherWGSLFeatures(const WGPUDawnWireWGSLControl* wgslControl,
                            const WGPUDawnWGSLBlocklist* wgslBlocklist);

    absl::flat_hash_set<WGPUWGSLFeatureName> mWGSLFeatures;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_INSTANCE_H_
