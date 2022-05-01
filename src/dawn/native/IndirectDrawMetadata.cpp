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

#include "dawn/native/IndirectDrawMetadata.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Constants.h"
#include "dawn/common/RefCounted.h"
#include "dawn/native/IndirectDrawValidationEncoder.h"
#include "dawn/native/Limits.h"
#include "dawn/native/RenderBundle.h"

namespace dawn::native {

uint64_t ComputeMaxIndirectValidationBatchOffsetRange(const CombinedLimits& limits) {
    return limits.v1.maxStorageBufferBindingSize - limits.v1.minStorageBufferOffsetAlignment -
           kDrawIndexedIndirectSize;
}

IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::IndexedIndirectBufferValidationInfo(
    BufferBase* indirectBuffer)
    : mIndirectBuffer(indirectBuffer) {}

void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddIndirectDraw(
    uint32_t maxDrawCallsPerIndirectValidationBatch,
    uint64_t maxBatchOffsetRange,
    IndirectDraw draw) {
    const uint64_t newOffset = draw.inputBufferOffset;
    auto it = mBatches.begin();
    while (it != mBatches.end()) {
        IndirectValidationBatch& batch = *it;
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

    IndirectValidationBatch newBatch;
    newBatch.minOffset = newOffset;
    newBatch.maxOffset = newOffset;
    newBatch.draws.push_back(std::move(draw));

    mBatches.insert(it, std::move(newBatch));
}

void IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::AddBatch(
    uint32_t maxDrawCallsPerIndirectValidationBatch,
    uint64_t maxBatchOffsetRange,
    const IndirectValidationBatch& newBatch) {
    auto it = mBatches.begin();
    while (it != mBatches.end()) {
        IndirectValidationBatch& batch = *it;
        uint64_t min = std::min(newBatch.minOffset, batch.minOffset);
        uint64_t max = std::max(newBatch.maxOffset, batch.maxOffset);
        if (max - min <= maxBatchOffsetRange &&
            batch.draws.size() + newBatch.draws.size() <= maxDrawCallsPerIndirectValidationBatch) {
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

const std::vector<IndirectDrawMetadata::IndirectValidationBatch>&
IndirectDrawMetadata::IndexedIndirectBufferValidationInfo::GetBatches() const {
    return mBatches;
}

IndirectDrawMetadata::IndirectDrawMetadata(const CombinedLimits& limits)
    : mMaxBatchOffsetRange(ComputeMaxIndirectValidationBatchOffsetRange(limits)),
      mMaxDrawCallsPerBatch(ComputeMaxDrawCallsPerIndirectValidationBatch(limits)) {}

IndirectDrawMetadata::~IndirectDrawMetadata() = default;

IndirectDrawMetadata::IndirectDrawMetadata(IndirectDrawMetadata&&) = default;

IndirectDrawMetadata& IndirectDrawMetadata::operator=(IndirectDrawMetadata&&) = default;

IndirectDrawMetadata::IndexedIndirectBufferValidationInfoMap*
IndirectDrawMetadata::GetIndexedIndirectBufferValidationInfo() {
    return &mIndexedIndirectBufferValidationInfo;
}

void IndirectDrawMetadata::AddBundle(RenderBundleBase* bundle) {
    auto [_, inserted] = mAddedBundles.insert(bundle);
    if (!inserted) {
        return;
    }

    for (const auto& [config, validationInfo] :
         bundle->GetIndirectDrawMetadata().mIndexedIndirectBufferValidationInfo) {
        auto it = mIndexedIndirectBufferValidationInfo.lower_bound(config);
        if (it != mIndexedIndirectBufferValidationInfo.end() && it->first == config) {
            // We already have batches for the same config. Merge the new ones in.
            for (const IndirectValidationBatch& batch : validationInfo.GetBatches()) {
                it->second.AddBatch(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, batch);
            }
        } else {
            mIndexedIndirectBufferValidationInfo.emplace_hint(it, config, validationInfo);
        }
    }
}

void IndirectDrawMetadata::AddIndexedIndirectDraw(wgpu::IndexFormat indexFormat,
                                                  uint64_t indexBufferSize,
                                                  BufferBase* indirectBuffer,
                                                  uint64_t indirectOffset,
                                                  bool duplicateBaseVertexInstance,
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

    const IndexedIndirectConfig config = {indirectBuffer, numIndexBufferElements,
                                          duplicateBaseVertexInstance, DrawType::Indexed};
    auto it = mIndexedIndirectBufferValidationInfo.find(config);
    if (it == mIndexedIndirectBufferValidationInfo.end()) {
        auto result = mIndexedIndirectBufferValidationInfo.emplace(
            config, IndexedIndirectBufferValidationInfo(indirectBuffer));
        it = result.first;
    }

    IndirectDraw draw{};
    draw.inputBufferOffset = indirectOffset;
    draw.cmd = cmd;
    it->second.AddIndirectDraw(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, draw);
}

void IndirectDrawMetadata::AddIndirectDraw(BufferBase* indirectBuffer,
                                           uint64_t indirectOffset,
                                           bool duplicateBaseVertexInstance,
                                           DrawIndirectCmd* cmd) {
    const IndexedIndirectConfig config = {indirectBuffer, 0, duplicateBaseVertexInstance,
                                          DrawType::NonIndexed};
    auto it = mIndexedIndirectBufferValidationInfo.find(config);
    if (it == mIndexedIndirectBufferValidationInfo.end()) {
        auto result = mIndexedIndirectBufferValidationInfo.emplace(
            config, IndexedIndirectBufferValidationInfo(indirectBuffer));
        it = result.first;
    }

    IndirectDraw draw{};
    draw.inputBufferOffset = indirectOffset;
    draw.cmd = cmd;
    it->second.AddIndirectDraw(mMaxDrawCallsPerBatch, mMaxBatchOffsetRange, draw);
}

bool IndirectDrawMetadata::IndexedIndirectConfig::operator<(
    const IndexedIndirectConfig& other) const {
    return std::tie(inputIndirectBuffer, numIndexBufferElements, duplicateBaseVertexInstance,
                    drawType) < std::tie(other.inputIndirectBuffer, other.numIndexBufferElements,
                                         other.duplicateBaseVertexInstance, other.drawType);
}

bool IndirectDrawMetadata::IndexedIndirectConfig::operator==(
    const IndexedIndirectConfig& other) const {
    return std::tie(inputIndirectBuffer, numIndexBufferElements, duplicateBaseVertexInstance,
                    drawType) == std::tie(other.inputIndirectBuffer, other.numIndexBufferElements,
                                          other.duplicateBaseVertexInstance, other.drawType);
}

}  // namespace dawn::native
