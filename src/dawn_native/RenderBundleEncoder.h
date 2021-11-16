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

#ifndef DAWNNATIVE_RENDERBUNDLEENCODER_H_
#define DAWNNATIVE_RENDERBUNDLEENCODER_H_

#include "dawn_native/EncodingContext.h"
#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RenderBundle.h"
#include "dawn_native/RenderEncoderBase.h"

namespace dawn_native {

    MaybeError ValidateRenderBundleEncoderDescriptor(
        const DeviceBase* device,
        const RenderBundleEncoderDescriptor* descriptor);

    class RenderBundleEncoder final : public RenderEncoderBase {
      public:
        static Ref<RenderBundleEncoder> Create(DeviceBase* device,
                                               const RenderBundleEncoderDescriptor* descriptor);
        static RenderBundleEncoder* MakeError(DeviceBase* device);

        ObjectType GetType() const override;

        RenderBundleBase* APIFinish(const RenderBundleDescriptor* descriptor);

        CommandIterator AcquireCommands();

      private:
        RenderBundleEncoder(DeviceBase* device, const RenderBundleEncoderDescriptor* descriptor);
        RenderBundleEncoder(DeviceBase* device, ErrorTag errorTag);

        void DestroyImpl() override;

        ResultOrError<RenderBundleBase*> FinishImpl(const RenderBundleDescriptor* descriptor);
        MaybeError ValidateFinish(const RenderPassResourceUsage& usages) const;

        EncodingContext mBundleEncodingContext;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERBUNDLEENCODER_H_
