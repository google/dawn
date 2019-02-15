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

#ifndef DAWNNATIVE_COMMANDENCODER_H_
#define DAWNNATIVE_COMMANDENCODER_H_

#include "dawn_native/dawn_platform.h"

#include "dawn_native/Error.h"
#include "dawn_native/ObjectBase.h"

#include <string>

namespace dawn_native {

    class CommandBufferBuilder;

    // CommandEncoder is temporarily a wrapper around CommandBufferBuilder so the two can coexist
    // while code is migrated to the new shiny CommandEncoder interface. It captures any command
    // buffer builder error and defers to trigger a device error when "Finish" is called.
    class CommandEncoderBase : public ObjectBase {
      public:
        CommandEncoderBase(DeviceBase* device);
        ~CommandEncoderBase();

        // Dawn API
        ComputePassEncoderBase* BeginComputePass();
        RenderPassEncoderBase* BeginRenderPass(RenderPassDescriptorBase* info);
        void CopyBufferToBuffer(BufferBase* source,
                                uint32_t sourceOffset,
                                BufferBase* destination,
                                uint32_t destinationOffset,
                                uint32_t size);
        void CopyBufferToTexture(const BufferCopyView* source,
                                 const TextureCopyView* destination,
                                 const Extent3D* copySize);
        void CopyTextureToBuffer(const TextureCopyView* source,
                                 const BufferCopyView* destination,
                                 const Extent3D* copySize);
        CommandBufferBase* Finish();

      private:
        MaybeError ValidateFinish();
        static void HandleBuilderError(dawnBuilderErrorStatus status,
                                       const char* message,
                                       dawnCallbackUserdata userdata1,
                                       dawnCallbackUserdata userdata2);

        CommandBufferBuilder* mBuilder = nullptr;
        bool mGotError = false;
        std::string mErrorMessage;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMMANDENCODER_H_
