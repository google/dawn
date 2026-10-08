// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_
#define SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/common/ityp_vector.h"
#include "src/dawn/native/IntegerTypes.h"
#include "src/dawn/native/PassResourceUsage.h"
#include "src/dawn/native/dawn_platform.h"

namespace dawn::native {

class BindGroupBase;
class BufferBase;
class ExternalTextureBase;
class QuerySetBase;
class TextureBase;

using QueryAvailabilityMap = absl::flat_hash_map<QuerySetBase*, ityp::vector<QueryIndex, bool>>;

// Helper class to build SyncScopeResourceUsages
class SyncScopeUsageTracker {
  public:
    SyncScopeUsageTracker();
    SyncScopeUsageTracker(SyncScopeUsageTracker&&);
    ~SyncScopeUsageTracker();

    SyncScopeUsageTracker& operator=(SyncScopeUsageTracker&&);

    void BufferUsedAs(BufferBase* buffer,
                      wgpu::BufferUsage usage,
                      wgpu::ShaderStage shaderStages = wgpu::ShaderStage::None);
    void TextureViewUsedAs(TextureViewBase* view,
                           wgpu::TextureUsage usage,
                           wgpu::ShaderStage shaderStages = wgpu::ShaderStage::None);
    void TextureRangeUsedAs(TextureBase* texture,
                            const SubresourceRange& range,
                            wgpu::TextureUsage usage,
                            wgpu::ShaderStage shaderStages = wgpu::ShaderStage::None);

    // Add all usages referenced to this tracker.
    void MergeResourceUsages(const SyncScopeResourceUsage& usages);

    void SetUsedResourceTable(ResourceTableBase* table);

    // Returns the per-pass usage for use by backends for APIs with explicit barriers.
    SyncScopeResourceUsage AcquireSyncScopeUsage();

  protected:
    // Walks the bind group and tracks all its resources. Subclasses expose this through their own
    // AddBindGroup() so that callers can't bypass deduplication where it is expected.
    void AddBindGroupUsages(BindGroupBase* group);

  private:
    void MergeTextureUsage(TextureBase* texture, const TextureSubresourceSyncInfo& textureSyncInfo);

    RAW_PTR_EXCLUSION absl::flat_hash_map<BufferBase*, BufferSyncInfo> mBufferSyncInfos;
    RAW_PTR_EXCLUSION absl::flat_hash_map<TextureBase*, TextureSubresourceSyncInfo>
        mTextureSyncInfos;
    RAW_PTR_EXCLUSION absl::flat_hash_set<ExternalTextureBase*> mExternalTextureUsages;
    raw_ptr<ResourceTableBase> mUsedResourceTable = nullptr;
};

// Helper class to build the SyncScopeResourceUsage of a single dispatch in a compute pass.
class DispatchResourceUsageTracker : public SyncScopeUsageTracker {
  public:
    // Walks the bind group and tracks all its resources.
    void AddBindGroup(BindGroupBase* group);
};

// Helper class to build ComputePassResourceUsages
class ComputePassResourceUsageTracker {
  public:
    explicit ComputePassResourceUsageTracker(PassTrackerID passTrackerID);
    ComputePassResourceUsageTracker(ComputePassResourceUsageTracker&&);
    ~ComputePassResourceUsageTracker();

    ComputePassResourceUsageTracker& operator=(ComputePassResourceUsageTracker&&);

    void AddDispatch(SyncScopeResourceUsage scope);
    void AddReferencedBuffer(BufferBase* buffer);
    void AddResourcesReferencedByBindGroup(BindGroupBase* group);
    void AddReferencedResourceTable(ResourceTableBase* table);

    ComputePassResourceUsage AcquireResourceUsage();

  private:
    ComputePassResourceUsage mUsage;

    // Unique identifier for this compute pass used by AddResourcesReferencedByBindGroup() to
    // skip bind groups already referenced in this pass.
    PassTrackerID mPassTrackerID = kInvalidPassTrackerID;
};

// Helper class to build RenderPassResourceUsages
class RenderPassResourceUsageTracker : public SyncScopeUsageTracker {
  public:
    explicit RenderPassResourceUsageTracker(PassTrackerID passTrackerID);
    RenderPassResourceUsageTracker(RenderPassResourceUsageTracker&&);
    ~RenderPassResourceUsageTracker();

    RenderPassResourceUsageTracker& operator=(RenderPassResourceUsageTracker&&);

    // Walks the bind group and tracks all its resources.
    void AddBindGroup(BindGroupBase* group);

    void TrackQueryAvailability(QuerySetBase* querySet, QueryIndex queryIndex);
    const QueryAvailabilityMap& GetQueryAvailabilityMap() const;

    void MarkFramebufferFetchUsed();

    RenderPassResourceUsage AcquireResourceUsage();

  private:
    // Hide AcquireSyncScopeUsage since users of this class should use AcquireResourceUsage
    // instead.
    using SyncScopeUsageTracker::AcquireSyncScopeUsage;

    // Tracks queries used in the render pass to validate that they aren't written twice.
    QueryAvailabilityMap mQueryAvailabilities;

    // Unique identifier for this render pass tracker used by AddBindGroup() to intrusively
    // deduplicate BindGroup usage tracking without a hash table lookup.
    PassTrackerID mPassTrackerID = kInvalidPassTrackerID;

    bool mFramebufferFetchUsed = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_
