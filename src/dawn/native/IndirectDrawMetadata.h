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

#ifndef SRC_DAWN_NATIVE_INDIRECTDRAWMETADATA_H_
#define SRC_DAWN_NATIVE_INDIRECTDRAWMETADATA_H_

#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/Commands.h"

namespace dawn::native {

class RenderBundleBase;
struct CombinedLimits;

// In the unlikely scenario that indirect offsets used over a single buffer span more than
// this length of the buffer, we split the validation work into multiple batches.
uint64_t ComputeMaxIndirectValidationBatchOffsetRange(const CombinedLimits& limits);

// Metadata corresponding to the validation requirements of a single render pass. This metadata
// is accumulated while its corresponding render pass is encoded, and is later used to encode
// validation commands to be inserted into the command buffer just before the render pass's own
// commands.
class IndirectDrawMetadata : public NonCopyable {
  public:
    struct IndirectDraw {
        uint64_t inputBufferOffset;
        // This is a pointer to the command that should be populated with the validated
        // indirect scratch buffer. It is only valid up until the encoded command buffer
        // is submitted.
        DrawIndirectCmd* cmd;
    };

    struct IndirectValidationBatch {
        uint64_t minOffset;
        uint64_t maxOffset;
        std::vector<IndirectDraw> draws;
    };

    // Tracks information about every draw call in this render pass which uses the same indirect
    // buffer and the same-sized index buffer. Calls are grouped by indirect offset ranges so
    // that validation work can be chunked efficiently if necessary.
    class IndexedIndirectBufferValidationInfo {
      public:
        explicit IndexedIndirectBufferValidationInfo(BufferBase* indirectBuffer);

        // Logs a new drawIndexedIndirect call for the render pass. `cmd` is updated with an
        // assigned (and deferred) buffer ref and relative offset before returning.
        void AddIndirectDraw(uint32_t maxDrawCallsPerIndirectValidationBatch,
                             uint64_t maxBatchOffsetRange,
                             IndirectDraw draw);

        // Adds draw calls from an already-computed batch, e.g. from a previously encoded
        // RenderBundle. The added batch is merged into an existing batch if possible, otherwise
        // it's added to mBatch.
        void AddBatch(uint32_t maxDrawCallsPerIndirectValidationBatch,
                      uint64_t maxBatchOffsetRange,
                      const IndirectValidationBatch& batch);

        const std::vector<IndirectValidationBatch>& GetBatches() const;

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
        std::vector<IndirectValidationBatch> mBatches;
    };

    enum class DrawType {
        NonIndexed,
        Indexed,
    };
    struct IndexedIndirectConfig {
        BufferBase* inputIndirectBuffer;
        uint64_t numIndexBufferElements;
        bool duplicateBaseVertexInstance;
        DrawType drawType;

        bool operator<(const IndexedIndirectConfig& other) const;
        bool operator==(const IndexedIndirectConfig& other) const;
    };

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
                                bool duplicateBaseVertexInstance,
                                DrawIndexedIndirectCmd* cmd);

    void AddIndirectDraw(BufferBase* indirectBuffer,
                         uint64_t indirectOffset,
                         bool duplicateBaseVertexInstance,
                         DrawIndirectCmd* cmd);

  private:
    IndexedIndirectBufferValidationInfoMap mIndexedIndirectBufferValidationInfo;
    std::set<RenderBundleBase*> mAddedBundles;

    uint64_t mMaxBatchOffsetRange;
    uint32_t mMaxDrawCallsPerBatch;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_INDIRECTDRAWMETADATA_H_
