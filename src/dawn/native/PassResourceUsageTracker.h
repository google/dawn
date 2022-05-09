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

#ifndef SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_
#define SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_

#include <map>
#include <set>
#include <vector>

#include "dawn/native/PassResourceUsage.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class BindGroupBase;
class BufferBase;
class ExternalTextureBase;
class QuerySetBase;
class TextureBase;

using QueryAvailabilityMap = std::map<QuerySetBase*, std::vector<bool>>;

// Helper class to build SyncScopeResourceUsages
class SyncScopeUsageTracker {
  public:
    SyncScopeUsageTracker();
    SyncScopeUsageTracker(SyncScopeUsageTracker&&);
    ~SyncScopeUsageTracker();

    SyncScopeUsageTracker& operator=(SyncScopeUsageTracker&&);

    void BufferUsedAs(BufferBase* buffer, wgpu::BufferUsage usage);
    void TextureViewUsedAs(TextureViewBase* texture, wgpu::TextureUsage usage);
    void AddRenderBundleTextureUsage(TextureBase* texture,
                                     const TextureSubresourceUsage& textureUsage);

    // Walks the bind groups and tracks all its resources.
    void AddBindGroup(BindGroupBase* group);

    // Returns the per-pass usage for use by backends for APIs with explicit barriers.
    SyncScopeResourceUsage AcquireSyncScopeUsage();

  private:
    std::map<BufferBase*, wgpu::BufferUsage> mBufferUsages;
    std::map<TextureBase*, TextureSubresourceUsage> mTextureUsages;
    std::set<ExternalTextureBase*> mExternalTextureUsages;
};

// Helper class to build ComputePassResourceUsages
class ComputePassResourceUsageTracker {
  public:
    ComputePassResourceUsageTracker();
    ~ComputePassResourceUsageTracker();

    void AddDispatch(SyncScopeResourceUsage scope);
    void AddReferencedBuffer(BufferBase* buffer);
    void AddResourcesReferencedByBindGroup(BindGroupBase* group);

    ComputePassResourceUsage AcquireResourceUsage();

  private:
    ComputePassResourceUsage mUsage;
};

// Helper class to build RenderPassResourceUsages
class RenderPassResourceUsageTracker : public SyncScopeUsageTracker {
  public:
    RenderPassResourceUsageTracker();
    RenderPassResourceUsageTracker(RenderPassResourceUsageTracker&&);
    ~RenderPassResourceUsageTracker();

    RenderPassResourceUsageTracker& operator=(RenderPassResourceUsageTracker&&);

    void TrackQueryAvailability(QuerySetBase* querySet, uint32_t queryIndex);
    const QueryAvailabilityMap& GetQueryAvailabilityMap() const;

    RenderPassResourceUsage AcquireResourceUsage();

  private:
    // Hide AcquireSyncScopeUsage since users of this class should use AcquireResourceUsage
    // instead.
    using SyncScopeUsageTracker::AcquireSyncScopeUsage;

    // Tracks queries used in the render pass to validate that they aren't written twice.
    QueryAvailabilityMap mQueryAvailabilities;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_PASSRESOURCEUSAGETRACKER_H_
