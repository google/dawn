// Copyright 2018 The Dawn Authors
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

#ifndef DAWNNATIVE_RENDERPASSENCODER_H_
#define DAWNNATIVE_RENDERPASSENCODER_H_

#include "dawn_native/Error.h"
#include "dawn_native/Forward.h"
#include "dawn_native/RenderEncoderBase.h"

namespace dawn_native {

    class RenderBundleBase;

    class RenderPassEncoder final : public RenderEncoderBase {
      public:
        RenderPassEncoder(DeviceBase* device,
                          const RenderPassDescriptor* descriptor,
                          CommandEncoder* commandEncoder,
                          EncodingContext* encodingContext,
                          RenderPassResourceUsageTracker usageTracker,
                          Ref<AttachmentState> attachmentState,
                          QuerySetBase* occlusionQuerySet,
                          uint32_t renderTargetWidth,
                          uint32_t renderTargetHeight,
                          bool depthReadOnly,
                          bool stencilReadOnly);

        static RenderPassEncoder* MakeError(DeviceBase* device,
                                            CommandEncoder* commandEncoder,
                                            EncodingContext* encodingContext);

        ObjectType GetType() const override;

        void APIEndPass();

        void APISetStencilReference(uint32_t reference);
        void APISetBlendConstant(const Color* color);
        void APISetViewport(float x,
                            float y,
                            float width,
                            float height,
                            float minDepth,
                            float maxDepth);
        void APISetScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
        void APIExecuteBundles(uint32_t count, RenderBundleBase* const* renderBundles);

        void APIBeginOcclusionQuery(uint32_t queryIndex);
        void APIEndOcclusionQuery();

        void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

      protected:
        RenderPassEncoder(DeviceBase* device,
                          CommandEncoder* commandEncoder,
                          EncodingContext* encodingContext,
                          ErrorTag errorTag);

      private:
        void DestroyImpl() override;

        void TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex);

        // For render and compute passes, the encoding context is borrowed from the command encoder.
        // Keep a reference to the encoder to make sure the context isn't freed.
        Ref<CommandEncoder> mCommandEncoder;

        uint32_t mRenderTargetWidth;
        uint32_t mRenderTargetHeight;

        // The resources for occlusion query
        Ref<QuerySetBase> mOcclusionQuerySet;
        uint32_t mCurrentOcclusionQueryIndex = 0;
        bool mOcclusionQueryActive = false;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_RENDERPASSENCODER_H_
