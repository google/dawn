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

#ifndef SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_
#define SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_

#include <array>
#include <bitset>
#include <mutex>
#include <unordered_map>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native::vulkan {

class Device;

// This is a key to query the RenderPassCache, it can be sparse meaning that only the
// information for bits set in colorMask or hasDepthStencil need to be provided and the rest can
// be uninintialized.
struct RenderPassCacheQuery {
    // Use these helpers to build the query, they make sure all relevant data is initialized and
    // masks set.
    void SetColor(ColorAttachmentIndex index,
                  wgpu::TextureFormat format,
                  wgpu::LoadOp loadOp,
                  wgpu::StoreOp storeOp,
                  bool hasResolveTarget);
    void SetDepthStencil(wgpu::TextureFormat format,
                         wgpu::LoadOp depthLoadOp,
                         wgpu::StoreOp depthStoreOp,
                         wgpu::LoadOp stencilLoadOp,
                         wgpu::StoreOp stencilStoreOp,
                         bool readOnly);
    void SetSampleCount(uint32_t sampleCount);

    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> colorMask;
    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> resolveTargetMask;
    ityp::array<ColorAttachmentIndex, wgpu::TextureFormat, kMaxColorAttachments> colorFormats;
    ityp::array<ColorAttachmentIndex, wgpu::LoadOp, kMaxColorAttachments> colorLoadOp;
    ityp::array<ColorAttachmentIndex, wgpu::StoreOp, kMaxColorAttachments> colorStoreOp;

    bool hasDepthStencil = false;
    wgpu::TextureFormat depthStencilFormat;
    wgpu::LoadOp depthLoadOp;
    wgpu::StoreOp depthStoreOp;
    wgpu::LoadOp stencilLoadOp;
    wgpu::StoreOp stencilStoreOp;
    bool readOnlyDepthStencil;

    uint32_t sampleCount;
};

// Caches VkRenderPasses so that we don't create duplicate ones for every RenderPipeline or
// render pass. We always arrange the order of attachments in "color-depthstencil-resolve" order
// when creating render pass and framebuffer so that we can always make sure the order of
// attachments in the rendering pipeline matches the one of the framebuffer.
// All the operations on RenderPassCache are guaranteed to be thread-safe.
// TODO(cwallez@chromium.org): Make it an LRU cache somehow?
class RenderPassCache {
  public:
    explicit RenderPassCache(Device* device);
    ~RenderPassCache();

    ResultOrError<VkRenderPass> GetRenderPass(const RenderPassCacheQuery& query);

  private:
    // Does the actual VkRenderPass creation on a cache miss.
    ResultOrError<VkRenderPass> CreateRenderPassForQuery(const RenderPassCacheQuery& query) const;

    // Implements the functors necessary for to use RenderPassCacheQueries as unordered_map
    // keys.
    struct CacheFuncs {
        size_t operator()(const RenderPassCacheQuery& query) const;
        bool operator()(const RenderPassCacheQuery& a, const RenderPassCacheQuery& b) const;
    };
    using Cache = std::unordered_map<RenderPassCacheQuery, VkRenderPass, CacheFuncs, CacheFuncs>;

    Device* mDevice = nullptr;

    std::mutex mMutex;
    Cache mCache;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RENDERPASSCACHE_H_
