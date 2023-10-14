// Copyright 2022 The Dawn Authors
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

#include <memory>
#include <utility>
#include <variant>
#include <vector>

#include "VideoViewsTests.h"
#include "dawn/common/Assert.h"
#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/IOSurfaceUtils.h"
#include "dawn/native/MetalBackend.h"

namespace dawn {
namespace {

class PlatformTextureIOSurface : public VideoViewsTestBackend::PlatformTexture {
  public:
    PlatformTextureIOSurface(wgpu::Texture&& texture, IOSurfaceRef iosurface)
        : PlatformTexture(std::move(texture)) {
        mIOSurface = AcquireCFRef<IOSurfaceRef>(iosurface);
    }
    ~PlatformTextureIOSurface() override { mIOSurface = nullptr; }

    bool CanWrapAsWGPUTexture() override { return true; }

  private:
    CFRef<IOSurfaceRef> mIOSurface = nullptr;
};

class VideoViewsTestBackendIOSurface : public VideoViewsTestBackend {
  public:
    void OnSetUp(WGPUDevice device) override { mWGPUDevice = device; }

  private:
    std::unique_ptr<VideoViewsTestBackend::PlatformTexture> CreateVideoTextureForTest(
        wgpu::TextureFormat format,
        wgpu::TextureUsage usage,
        bool isCheckerboard,
        bool initialized) override {
        IOSurfaceRef surface =
            CreateMultiPlanarIOSurface(format, VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                                       VideoViewsTestsBase::kYUVImageDataHeightInTexels);

        if (initialized) {
            const size_t numPlanes = VideoViewsTestsBase::NumPlanes(format);

            IOSurfaceLock(surface, 0, nullptr);

            for (size_t plane = 0; plane < numPlanes; ++plane) {
                void* pointer = IOSurfaceGetBaseAddressOfPlane(surface, plane);
                if (format == wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm) {
                    std::vector<uint16_t> data =
                        VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint16_t>(
                            plane, IOSurfaceGetBytesPerRowOfPlane(surface, plane),
                            IOSurfaceGetHeightOfPlane(surface, plane), isCheckerboard);
                    memcpy(pointer, data.data(), data.size() * 2);
                } else {
                    std::vector<uint8_t> data =
                        VideoViewsTestsBase::GetTestTextureDataWithPlaneIndex<uint8_t>(
                            plane, IOSurfaceGetBytesPerRowOfPlane(surface, plane),
                            IOSurfaceGetHeightOfPlane(surface, plane), isCheckerboard);
                    memcpy(pointer, data.data(), data.size());
                }
            }
            IOSurfaceUnlock(surface, 0, nullptr);
        }

        wgpu::TextureDescriptor textureDesc;
        textureDesc.format = format;
        textureDesc.dimension = wgpu::TextureDimension::e2D;
        textureDesc.usage = usage;
        textureDesc.size = {VideoViewsTestsBase::kYUVImageDataWidthInTexels,
                            VideoViewsTestsBase::kYUVImageDataHeightInTexels, 1};

        wgpu::DawnTextureInternalUsageDescriptor internalDesc;
        internalDesc.internalUsage = wgpu::TextureUsage::CopySrc;
        textureDesc.nextInChain = &internalDesc;

        native::metal::ExternalImageDescriptorIOSurface descriptor = {};
        descriptor.cTextureDescriptor =
            reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
        descriptor.isInitialized = initialized;
        descriptor.ioSurface = surface;

        return std::make_unique<PlatformTextureIOSurface>(
            wgpu::Texture::Acquire(native::metal::WrapIOSurface(mWGPUDevice, &descriptor)),
            surface);
    }

    void DestroyVideoTextureForTest(
        std::unique_ptr<VideoViewsTestBackend::PlatformTexture>&& platformTexture) override {}

    WGPUDevice mWGPUDevice = nullptr;
};

}  // anonymous namespace

// static
std::vector<BackendTestConfig> VideoViewsTestBackend::Backends() {
    return {MetalBackend()};
}

// static
std::vector<Format> VideoViewsTestBackend::Formats() {
    return {wgpu::TextureFormat::R8BG8Biplanar420Unorm,
            wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm};
}

// static
std::unique_ptr<VideoViewsTestBackend> VideoViewsTestBackend::Create() {
    return std::make_unique<VideoViewsTestBackendIOSurface>();
}

}  // namespace dawn
