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

#include <fcntl.h>
#include <gbm.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "VideoViewsTests.h"
#include "dawn/common/Assert.h"
#include "dawn/native/VulkanBackend.h"

// "linux-chromeos-rel"'s gbm.h is too old to compile, missing this change at least:
// https://chromium-review.googlesource.com/c/chromiumos/platform/minigbm/+/1963001/10/gbm.h#244
#ifndef MINIGBM
#define GBM_BO_USE_TEXTURING (1 << 5)
#define GBM_BO_USE_SW_WRITE_RARELY (1 << 12)
#define GBM_BO_USE_HW_VIDEO_DECODER (1 << 13)
#endif

class PlatformTextureGbm : public VideoViewsTestBackend::PlatformTexture {
  public:
    PlatformTextureGbm(wgpu::Texture&& texture, gbm_bo* gbmBo)
        : PlatformTexture(std::move(texture)), mGbmBo(gbmBo) {}
    ~PlatformTextureGbm() override = default;

    // TODO(chromium:1258986): Add DISJOINT vkImage support for multi-plannar formats.
    bool CanWrapAsWGPUTexture() override {
        ASSERT(mGbmBo != nullptr);
        // Checks if all plane handles of a multi-planar gbm_bo are same.
        gbm_bo_handle plane0Handle = gbm_bo_get_handle_for_plane(mGbmBo, 0);
        for (int plane = 1; plane < gbm_bo_get_plane_count(mGbmBo); ++plane) {
            if (gbm_bo_get_handle_for_plane(mGbmBo, plane).u32 != plane0Handle.u32) {
                return false;
            }
        }
        return true;
    }

    gbm_bo* GetGbmBo() { return mGbmBo; }

  private:
    gbm_bo* mGbmBo = nullptr;
};

class VideoViewsTestBackendGbm : public VideoViewsTestBackend {
  public:
    void OnSetUp(WGPUDevice device) override {
        mWGPUDevice = device;
        mGbmDevice = CreateGbmDevice();
    }

    void OnTearDown() override { gbm_device_destroy(mGbmDevice); }

  private:
    gbm_device* CreateGbmDevice() {
        // Render nodes [1] are the primary interface for communicating with the GPU on
        // devices that support DRM. The actual filename of the render node is
        // implementation-specific, so we must scan through all possible filenames to find
        // one that we can use [2].
        //
        // [1] https://dri.freedesktop.org/docs/drm/gpu/drm-uapi.html#render-nodes
        // [2]
        // https://cs.chromium.org/chromium/src/ui/ozone/platform/wayland/gpu/drm_render_node_path_finder.cc
        const uint32_t kRenderNodeStart = 128;
        const uint32_t kRenderNodeEnd = kRenderNodeStart + 16;
        const std::string kRenderNodeTemplate = "/dev/dri/renderD";

        int renderNodeFd = -1;
        for (uint32_t i = kRenderNodeStart; i < kRenderNodeEnd; i++) {
            std::string renderNode = kRenderNodeTemplate + std::to_string(i);
            renderNodeFd = open(renderNode.c_str(), O_RDWR);
            if (renderNodeFd >= 0)
                break;
        }
        ASSERT(renderNodeFd > 0);

        gbm_device* gbmDevice = gbm_create_device(renderNodeFd);
        ASSERT(gbmDevice != nullptr);
        return gbmDevice;
    }

    static uint32_t GetGbmBoFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return GBM_FORMAT_NV12;
            default:
                UNREACHABLE();
        }
    }

    WGPUTextureFormat ToWGPUTextureFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
                return WGPUTextureFormat_R8BG8Biplanar420Unorm;
            default:
                UNREACHABLE();
        }
    }

    WGPUTextureUsage ToWGPUTextureUsage(wgpu::TextureUsage usage) {
        switch (usage) {
            case wgpu::TextureUsage::TextureBinding:
                return WGPUTextureUsage_TextureBinding;
            default:
                UNREACHABLE();
        }
    }

    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard) override {
        uint32_t flags = GBM_BO_USE_SCANOUT | GBM_BO_USE_TEXTURING | GBM_BO_USE_HW_VIDEO_DECODER |
                         GBM_BO_USE_SW_WRITE_RARELY;
        gbm_bo* gbmBo = gbm_bo_create(mGbmDevice, VideoViewsTests::kYUVImageDataWidthInTexels,
                                      VideoViewsTests::kYUVImageDataHeightInTexels,
                                      GetGbmBoFormat(format), flags);
        if (gbmBo == nullptr) {
            return nullptr;
        }

        void* mapHandle = nullptr;
        uint32_t strideBytes = 0;
        void* addr = gbm_bo_map(gbmBo, 0, 0, VideoViewsTests::kYUVImageDataWidthInTexels,
                                VideoViewsTests::kYUVImageDataHeightInTexels, GBM_BO_TRANSFER_WRITE,
                                &strideBytes, &mapHandle);
        EXPECT_NE(addr, nullptr);
        std::vector<uint8_t> initialData =
            VideoViewsTests::GetTestTextureData(format, isCheckerboard);
        std::memcpy(addr, initialData.data(), initialData.size());

        gbm_bo_unmap(gbmBo, mapHandle);

        wgpu::TextureDescriptor textureDesc;
        textureDesc.format = format;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.usage = usage;
        textureDesc.size = {VideoViewsTests::kYUVImageDataWidthInTexels,
                            VideoViewsTests::kYUVImageDataHeightInTexels, 1};

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        textureDesc.nextInChain = &internalDesc;

        dawn::native::vulkan::ExternalImageDescriptorDmaBuf descriptor = {};
        descriptor.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
        descriptor.isInitialized = true;

        descriptor.memoryFD = gbm_bo_get_fd(gbmBo);
        descriptor.stride = gbm_bo_get_stride(gbmBo);
        descriptor.drmModifier = gbm_bo_get_modifier(gbmBo);
        descriptor.waitFDs = {};

        return std::make_unique<PlatformTextureGbm>(
            wgpu::Texture::Acquire(dawn::native::vulkan::WrapVulkanImage(mWGPUDevice, &descriptor)),
            gbmBo);
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& platformTexture) override {
        // Exports the signal and ignores it.
        dawn::native::vulkan::ExternalImageExportInfoDmaBuf exportInfo;
        dawn::native::vulkan::ExportVulkanImage(platformTexture->wgpuTexture.Get(),
                                                VK_IMAGE_LAYOUT_GENERAL, &exportInfo);
        for (int fd : exportInfo.semaphoreHandles) {
            ASSERT_NE(fd, -1);
            close(fd);
        }
        gbm_bo* gbmBo = static_cast<PlatformTextureGbm*>(platformTexture.get())->GetGbmBo();
        ASSERT_NE(gbmBo, nullptr);
        gbm_bo_destroy(gbmBo);
    }

    WGPUDevice mWGPUDevice = nullptr;
    gbm_device* mGbmDevice = nullptr;
};

// static
BackendTestConfig VideoViewsTestBackend::Backend() {
    return VulkanBackend();
}

// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendGbm>();
}
