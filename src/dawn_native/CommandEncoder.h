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

#include "dawn_native/EncodingContext.h"
#include "dawn_native/Error.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/PassResourceUsage.h"

#include <map>
#include <string>

namespace dawn_native {

    using QueryAvailabilityMap = std::map<QuerySetBase*, std::vector<bool>>;

    class CommandEncoder final : public ObjectBase {
      public:
        CommandEncoder(DeviceBase* device, const CommandEncoderDescriptor* descriptor);

        CommandIterator AcquireCommands();
        CommandBufferResourceUsage AcquireResourceUsages();

        void TrackUsedQuerySet(QuerySetBase* querySet);
        void TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex);
        const QueryAvailabilityMap& GetQueryAvailabilityMap() const;

        // Dawn API
        ComputePassEncoder* APIBeginComputePass(const ComputePassDescriptor* descriptor);
        RenderPassEncoder* APIBeginRenderPass(const RenderPassDescriptor* descriptor);

        void APICopyBufferToBuffer(BufferBase* source,
                                   uint64_t sourceOffset,
                                   BufferBase* destination,
                                   uint64_t destinationOffset,
                                   uint64_t size);
        void APICopyBufferToTexture(const ImageCopyBuffer* source,
                                    const ImageCopyTexture* destination,
                                    const Extent3D* copySize);
        void APICopyTextureToBuffer(const ImageCopyTexture* source,
                                    const ImageCopyBuffer* destination,
                                    const Extent3D* copySize);
        void APICopyTextureToTexture(const ImageCopyTexture* source,
                                     const ImageCopyTexture* destination,
                                     const Extent3D* copySize);

        void APIInjectValidationError(const char* message);
        void APIInsertDebugMarker(const char* groupLabel);
        void APIPopDebugGroup();
        void APIPushDebugGroup(const char* groupLabel);

        void APIResolveQuerySet(QuerySetBase* querySet,
                                uint32_t firstQuery,
                                uint32_t queryCount,
                                BufferBase* destination,
                                uint64_t destinationOffset);
        void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

        CommandBufferBase* APIFinish(const CommandBufferDescriptor* descriptor = nullptr);

      private:
        ResultOrError<Ref<CommandBufferBase>> FinishInternal(
            const CommandBufferDescriptor* descriptor);

        MaybeError ValidateFinish(CommandIterator* commands,
                                  const PerPassUsages& perPassUsages) const;

        EncodingContext mEncodingContext;
        std::set<BufferBase*> mTopLevelBuffers;
        std::set<TextureBase*> mTopLevelTextures;
        std::set<QuerySetBase*> mUsedQuerySets;
        QueryAvailabilityMap mQueryAvailabilityMap;

        uint64_t mDebugGroupStackSize = 0;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_COMMANDENCODER_H_
