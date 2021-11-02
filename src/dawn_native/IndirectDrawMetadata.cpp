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

#include "dawn_native/IndirectDrawMetadata.h"

#include "common/Constants.h"
#include "common/RefCounted.h"
#include "dawn_native/IndirectDrawValidationEncoder.h"
#include "dawn_native/Limits.h"
#include "dawn_native/RenderBundle.h"

#include <algorithm>
#include <utility>

namespace dawn_native {

    uint32_t ComputeMaxIndirectValidationBatchOffsetRange(const CombinedLimits& limits) {
        return limits.v1.maxStorageBufferBindingSize - limits.v1.minStorageBufferOffsetAlignment -
               kDrawIndexedIndirectSize;
    }

    IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::IndexedIndirectBufferValidationInfo(
        BufferBase* indirectBuffer)
        : mIndirectBuffer(indirectBuffer) {
    }

    void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddIndexedIndirectDraw(
        uint32_t maxDrawCallsPerIndirectValidationBatch,
        uint32_t maxBatchOffsetRange,
        IndexedIndirectDraw draw) {
        const uint64_t newOffset = draw.clientBufferOffset;
        auto it = mBatches.begin();
        while (it != mBatches.end()) {
            IndexedIndirectValidationBatch& batch = *it;
            if (batch.draws.size() >= maxDrawCallsPerIndirectValidationBatch) {
                // This batch is full. If its minOffset is to the right of the new offset, we can
                // just insert a new batch here.
                if (newOffset < batch.minOffset) {
                    break;
                }

                // Otherwise keep looking.
                ++it;
                continue;
            }

            if (newOffset >= batch.minOffset && newOffset <= batch.maxOffset) {
                batch.draws.push_back(std::move(draw));
                return;
            }

            if (newOffset < batch.minOffset && batch.maxOffset - newOffset <= maxBatchOffsetRange) {
                // We can extend this batch to the left in order to fit the new offset.
                batch.minOffset = newOffset;
                batch.draws.push_back(std::move(draw));
                return;
            }

            if (newOffset > batch.maxOffset && newOffset - batch.minOffset <= maxBatchOffsetRange) {
                // We can extend this batch to the right in order to fit the new offset.
                batch.maxOffset = newOffset;
                batch.draws.push_back(std::move(draw));
                return;
            }

            if (newOffset < batch.minOffset) {
                // We want to insert a new batch just before this one.
                break;
            }

            ++it;
        }

        IndexedIndirectValidationBatch newBatch;
        newBatch.minOffset = newOffset;
        newBatch.maxOffset = newOffset;
        newBatch.draws.push_back(std::move(draw));

        mBatches.insert(it, std::move(newBatch));
    }

    void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddBatch(
        uint32_t maxDrawCallsPerIndirectValidationBatch,
        uint32_t maxBatchOffsetRange,
        const IndexedIndirectValidationBatch& newBatch) {
        auto it = mBatches.begin();
        while (it != mBatches.end()) {
            IndexedIndirectValidationBatch& batch = *it;
            uint64_t min = std::min(newBatch.minOffset, batch.minOffset);
            uint64_t max = std::max(newBatch.maxOffset, batch.maxOffset);
            if (max - min <= maxBatchOffsetRange && batch.draws.size() + newBatch.draws.size() <=
                                                        maxDrawCallsPerIndirectValidationBatch) {
                // This batch fits within the limits of an existing batch. Merge it.
                batch.minOffset = min;
                batch.maxOffset = max;
                batch.draws.insert(batch.draws.end(), newBatch.draws.begin(), newBatch.draws.end());
                return;
            }

            if (newBatch.minOffset < batch.minOffset) {
                break;
            }

            ++it;
        }
        mBatches.push_back(newBatch);
    }

    const std::vector<IndirectDrawMetadata::IndexedIndirectValidationBatch>&
    IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::GetBatches() const {
        return mBatches;
    }

    IndirectDrawMetadata::IndirectDrawMetadata(const CombinedLimits& limits)
        : mMaxDrawCallsPerBatch(ComputeMaxDrawCallsPerIndirectValidationBatch(limits)),
          mMaxBatchOffsetRange(ComputeMaxIndirectValidationBatchOffsetRange(limits)) {
    }

    IndirectDrawMetadata::~IndirectDrawMetadata() = default;

    IndirectDrawMetadata::IndirectDrawMetadata(IndirectDrawMetadata&&) = default;

    IndirectDrawMetadata& IndirectDrawMetadata::operator=(IndirectDrawMetadata&&) = default;

    IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap*
    IndirectDrawMetadata::GetIndexedIndirectBufferValidationInfo() {
        return &mIndexedIndirectBufferValidationInfo;
    }

    void IndirectDrawMetadata::AddBundle(RenderBundleBase* bundle) {
        auto result = mAddedBundles.insert(bundle);
        if (!result.second) {
            return;
        }

        for (const auto& entry :
             bundle->GetIndirectDrawMetadata().mIndexedIndirectBufferValidationInfo) {
            const IndexedIndirectConfig& config = entry.first;
            auto it = mIndexedIndirectBufferValidationInfo.lower_bound(config);
            if (it != mIndexedIndirectBufferValidationInfo.end() && it->first == config) {
                // We already have batches for the same config. Merge the new ones in.
                for (const IndexedIndirectValidationBatch& batch : entry.second.GetBatches()) {
                    it->second.AddBatch(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, batch);
                }
            } else {
                mIndexedIndirectBufferValidationInfo.emplace_hint(it, config, entry.second);
            }
        }
    }

    void IndirectDrawMetadata::AddIndexedIndirectDraw(wgpu::IndexFormat indexFormat,
                                                      uint64_t indexBufferSize,
                                                      BufferBase* indirectBuffer,
                                                      uint64_t indirectOffset,
                                                      DrawIndexedIndirectCmd* cmd) {
        uint64_t numIndexBufferElements;
        switch (indexFormat) {
            case wgpu::IndexFormat::Uint16:
                numIndexBufferElements = indexBufferSize / 2;
                break;
            case wgpu::IndexFormat::Uint32:
                numIndexBufferElements = indexBufferSize / 4;
                break;
            case wgpu::IndexFormat::Undefined:
                UNREACHABLE();
        }

        const IndexedIndirectConfig config(indirectBuffer, numIndexBufferElements);
        auto it = mIndexedIndirectBufferValidationInfo.find(config);
        if (it == mIndexedIndirectBufferValidationInfo.end()) {
            auto result = mIndexedIndirectBufferValidationInfo.emplace(
                config, IndexedIndirectBufferValidationInfo(indirectBuffer));
            it = result.first;
        }

        IndexedIndirectDraw draw;
        draw.clientBufferOffset = indirectOffset;
        draw.cmd = cmd;
        it->second.AddIndexedIndirectDraw(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange,
                                          std::move(draw));
    }

}  // namespace dawn_native
