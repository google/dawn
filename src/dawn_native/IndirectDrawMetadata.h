// Copyright 2021 The Dawn Authors
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

#ifndef DAWNNATIVE_INDIRECTDRAWMETADATA_H_
#define DAWNNATIVE_INDIRECTDRAWMETADATA_H_

#include "common/NonCopyable.h"
#include "common/RefCounted.h"
#include "dawn_native/Buffer.h"
#include "dawn_native/CommandBufferStateTracker.h"
#include "dawn_native/Commands.h"

#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace dawn_native {

    class RenderBundleBase;
    struct CombinedLimits;

    // In the unlikely scenario that indirect offsets used over a single buffer span more than
    // this length of the buffer, we split the validation work into multiple batches.
    uint32_t ComputeMaxIndirectValidationBatchOffsetRange(const CombinedLimits& limits);

    // Metadata corresponding to the validation requirements of a single render pass. This metadata
    // is accumulated while its corresponding render pass is encoded, and is later used to encode
    // validation commands to be inserted into the command buffer just before the render pass's own
    // commands.
    class IndirectDrawMetadata : public NonCopyable {
      public:
        struct IndexedIndirectDraw {
            uint64_t clientBufferOffset;
            // This is a pointer to the command that should be populated with the validated
            // indirect scratch buffer. It is only valid up until the encoded command buffer
            // is submitted.
            DrawIndexedIndirectCmd* cmd;
        };

        struct IndexedIndirectValidationBatch {
            uint64_t minOffset;
            uint64_t maxOffset;
            std::vector<IndexedIndirectDraw> draws;
        };

        // Tracks information about every draw call in this render pass which uses the same indirect
        // buffer and the same-sized index buffer. Calls are grouped by indirect offset ranges so
        // that validation work can be chunked efficiently if necessary.
        class IndexedIndirectBufferValidationInfo {
          public:
            explicit IndexedIndirectBufferValidationInfo(BufferBase* indirectBuffer);

            // Logs a new drawIndexedIndirect call for the render pass. `cmd` is updated with an
            // assigned (and deferred) buffer ref and relative offset before returning.
            void AddIndexedIndirectDraw(uint32_t maxDrawCallsPerIndirectValidationBatch,
                                        uint32_t maxBatchOffsetRange,
                                        IndexedIndirectDraw draw);

            // Adds draw calls from an already-computed batch, e.g. from a previously encoded
            // RenderBundle. The added batch is merged into an existing batch if possible, otherwise
            // it's added to mBatch.
            void AddBatch(uint32_t maxDrawCallsPerIndirectValidationBatch,
                          uint32_t maxBatchOffsetRange,
                          const IndexedIndirectValidationBatch& batch);

            const std::vector<IndexedIndirectValidationBatch>& GetBatches() const;

          private:
            Ref<BufferBase> mIndirectBuffer;

            // A list of information about validation batches that will need to be executed for the
            // corresponding indirect buffer prior to a single render pass. These are kept sorted by
            // minOffset and may overlap iff the number of offsets in one batch would otherwise
            // exceed some large upper bound (roughly ~33M draw calls).
            //
            // Since the most common expected cases will overwhelmingly require only a single
            // validation pass per render pass, this is optimized for efficient updates to a single
            // batch rather than for efficient manipulation of a large number of batches.
            std::vector<IndexedIndirectValidationBatch> mBatches;
        };

        // Combination of an indirect buffer reference, and the number of addressable index buffer
        // elements at the time of a draw call.
        using IndexedIndirectConfig = std::pair<BufferBase*, uint64_t>;
        using IndexedIndirectBufferValidationInfoMap =
            std::map<IndexedIndirectConfig, IndexedIndirectBufferValidationInfo>;

        explicit IndirectDrawMetadata(const CombinedLimits& limits);
        ~IndirectDrawMetadata();

        IndirectDrawMetadata(IndirectDrawMetadata&&);
        IndirectDrawMetadata& operator=(IndirectDrawMetadata&&);

        IndexedIndirectBufferValidationInfoMap* GetIndexedIndirectBufferValidationInfo();

        void AddBundle(RenderBundleBase* bundle);
        void AddIndexedIndirectDraw(wgpu::IndexFormat indexFormat,
                                    uint64_t indexBufferSize,
                                    BufferBase* indirectBuffer,
                                    uint64_t indirectOffset,
                                    DrawIndexedIndirectCmd* cmd);

      private:
        IndexedIndirectBufferValidationInfoMap mIndexedIndirectBufferValidationInfo;
        std::set<RenderBundleBase*> mAddedBundles;

        uint32_t mMaxDrawCallsPerBatch;
        uint32_t mMaxBatchOffsetRange;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_INDIRECTDRAWMETADATA_H_
