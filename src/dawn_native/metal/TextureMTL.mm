// Copyright 2017 The Dawn Authors
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

#include "dawn_native/metal/TextureMTL.h"

#include "common/Constants.h"
#include "common/Math.h"
#include "common/Platform.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/EnumMaskIterator.h"
#include "dawn_native/metal/DeviceMTL.h"
#include "dawn_native/metal/StagingBufferMTL.h"
#include "dawn_native/metal/UtilsMetal.h"

#include <CoreVideo/CVPixelBuffer.h>

namespace dawn_native { namespace metal {

    namespace {
        bool UsageNeedsTextureView(wgpu::TextureUsage usage) {
            constexpr wgpu::TextureUsage kUsageNeedsTextureView =
                wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
            return usage & kUsageNeedsTextureView;
        }

        MTLTextureUsage MetalTextureUsage(const Format& format,
                                          wgpu::TextureUsage usage,
                                          uint32_t sampleCount) {
            MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

            if (usage & (wgpu::TextureUsage::StorageBinding)) {
                result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            }

            if (usage & (wgpu::TextureUsage::TextureBinding)) {
                result |= MTLTextureUsageShaderRead;

                // For sampling stencil aspect of combined depth/stencil. See TextureView
                // constructor.
                if (@available(macOS 10.12, iOS 10.0, *)) {
                    if (IsSubset(Aspect::Depth | Aspect::Stencil, format.aspects)) {
                        result |= MTLTextureUsagePixelFormatView;
                    }
                }
            }

            // MTLTextureUsageRenderTarget is needed to clear multisample textures.
            if (usage & (wgpu::TextureUsage::RenderAttachment) || sampleCount > 1) {
                result |= MTLTextureUsageRenderTarget;
            }

            return result;
        }

        MTLTextureType MetalTextureViewType(wgpu::TextureViewDimension dimension,
                                            unsigned int sampleCount) {
            switch (dimension) {
                case wgpu::TextureViewDimension::e2D:
                    return (sampleCount > 1) ? MTLTextureType2DMultisample : MTLTextureType2D;
                case wgpu::TextureViewDimension::e2DArray:
                    return MTLTextureType2DArray;
                case wgpu::TextureViewDimension::Cube:
                    return MTLTextureTypeCube;
                case wgpu::TextureViewDimension::CubeArray:
                    return MTLTextureTypeCubeArray;
                case wgpu::TextureViewDimension::e3D:
                    return MTLTextureType3D;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
            }
        }

        bool RequiresCreatingNewTextureView(const TextureBase* texture,
                                            const TextureViewDescriptor* textureViewDescriptor) {
            if (texture->GetFormat().format != textureViewDescriptor->format) {
                return true;
            }

            if (texture->GetArrayLayers() != textureViewDescriptor->arrayLayerCount) {
                return true;
            }

            if (texture->GetNumMipLevels() != textureViewDescriptor->mipLevelCount) {
                return true;
            }

            if (IsSubset(Aspect::Depth | Aspect::Stencil, texture->GetFormat().aspects) &&
                textureViewDescriptor->aspect == wgpu::TextureAspect::StencilOnly) {
                return true;
            }

            switch (textureViewDescriptor->dimension) {
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    return true;
                default:
                    break;
            }

            return false;
        }

        ResultOrError<wgpu::TextureFormat> GetFormatEquivalentToIOSurfaceFormat(uint32_t format) {
            switch (format) {
                case kCVPixelFormatType_32RGBA:
                    return wgpu::TextureFormat::RGBA8Unorm;
                case kCVPixelFormatType_32BGRA:
                    return wgpu::TextureFormat::BGRA8Unorm;
                case kCVPixelFormatType_TwoComponent8:
                    return wgpu::TextureFormat::RG8Unorm;
                case kCVPixelFormatType_OneComponent8:
                    return wgpu::TextureFormat::R8Unorm;
                default:
                    return DAWN_FORMAT_VALIDATION_ERROR("Unsupported IOSurface format (%x).",
                                                        format);
            }
        }

#if defined(DAWN_PLATFORM_MACOS)
        MTLStorageMode kIOSurfaceStorageMode = MTLStorageModeManaged;
#elif defined(DAWN_PLATFORM_IOS)
        MTLStorageMode kIOSurfaceStorageMode = MTLStorageModePrivate;
#else
#    error "Unsupported Apple platform."
#endif
    }

    MTLPixelFormat MetalPixelFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8Unorm:
                return MTLPixelFormatR8Unorm;
            case wgpu::TextureFormat::R8Snorm:
                return MTLPixelFormatR8Snorm;
            case wgpu::TextureFormat::R8Uint:
                return MTLPixelFormatR8Uint;
            case wgpu::TextureFormat::R8Sint:
                return MTLPixelFormatR8Sint;

            case wgpu::TextureFormat::R16Uint:
                return MTLPixelFormatR16Uint;
            case wgpu::TextureFormat::R16Sint:
                return MTLPixelFormatR16Sint;
            case wgpu::TextureFormat::R16Float:
                return MTLPixelFormatR16Float;
            case wgpu::TextureFormat::RG8Unorm:
                return MTLPixelFormatRG8Unorm;
            case wgpu::TextureFormat::RG8Snorm:
                return MTLPixelFormatRG8Snorm;
            case wgpu::TextureFormat::RG8Uint:
                return MTLPixelFormatRG8Uint;
            case wgpu::TextureFormat::RG8Sint:
                return MTLPixelFormatRG8Sint;

            case wgpu::TextureFormat::R32Uint:
                return MTLPixelFormatR32Uint;
            case wgpu::TextureFormat::R32Sint:
                return MTLPixelFormatR32Sint;
            case wgpu::TextureFormat::R32Float:
                return MTLPixelFormatR32Float;
            case wgpu::TextureFormat::RG16Uint:
                return MTLPixelFormatRG16Uint;
            case wgpu::TextureFormat::RG16Sint:
                return MTLPixelFormatRG16Sint;
            case wgpu::TextureFormat::RG16Float:
                return MTLPixelFormatRG16Float;
            case wgpu::TextureFormat::RGBA8Unorm:
                return MTLPixelFormatRGBA8Unorm;
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return MTLPixelFormatRGBA8Unorm_sRGB;
            case wgpu::TextureFormat::RGBA8Snorm:
                return MTLPixelFormatRGBA8Snorm;
            case wgpu::TextureFormat::RGBA8Uint:
                return MTLPixelFormatRGBA8Uint;
            case wgpu::TextureFormat::RGBA8Sint:
                return MTLPixelFormatRGBA8Sint;
            case wgpu::TextureFormat::BGRA8Unorm:
                return MTLPixelFormatBGRA8Unorm;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return MTLPixelFormatBGRA8Unorm_sRGB;
            case wgpu::TextureFormat::RGB10A2Unorm:
                return MTLPixelFormatRGB10A2Unorm;
            case wgpu::TextureFormat::RG11B10Ufloat:
                return MTLPixelFormatRG11B10Float;
            case wgpu::TextureFormat::RGB9E5Ufloat:
                return MTLPixelFormatRGB9E5Float;

            case wgpu::TextureFormat::RG32Uint:
                return MTLPixelFormatRG32Uint;
            case wgpu::TextureFormat::RG32Sint:
                return MTLPixelFormatRG32Sint;
            case wgpu::TextureFormat::RG32Float:
                return MTLPixelFormatRG32Float;
            case wgpu::TextureFormat::RGBA16Uint:
                return MTLPixelFormatRGBA16Uint;
            case wgpu::TextureFormat::RGBA16Sint:
                return MTLPixelFormatRGBA16Sint;
            case wgpu::TextureFormat::RGBA16Float:
                return MTLPixelFormatRGBA16Float;

            case wgpu::TextureFormat::RGBA32Uint:
                return MTLPixelFormatRGBA32Uint;
            case wgpu::TextureFormat::RGBA32Sint:
                return MTLPixelFormatRGBA32Sint;
            case wgpu::TextureFormat::RGBA32Float:
                return MTLPixelFormatRGBA32Float;

            case wgpu::TextureFormat::Depth32Float:
                return MTLPixelFormatDepth32Float;
            case wgpu::TextureFormat::Depth24Plus:
                return MTLPixelFormatDepth32Float;
            case wgpu::TextureFormat::Depth24PlusStencil8:
                return MTLPixelFormatDepth32Float_Stencil8;
            case wgpu::TextureFormat::Depth16Unorm:
                if (@available(macOS 10.12, iOS 13.0, *)) {
                    return MTLPixelFormatDepth16Unorm;
                } else {
                    // TODO (dawn:1181): Allow non-conformant implementation on macOS 10.11
                    UNREACHABLE();
                }

#if defined(DAWN_PLATFORM_MACOS)
            case wgpu::TextureFormat::BC1RGBAUnorm:
                return MTLPixelFormatBC1_RGBA;
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                return MTLPixelFormatBC1_RGBA_sRGB;
            case wgpu::TextureFormat::BC2RGBAUnorm:
                return MTLPixelFormatBC2_RGBA;
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                return MTLPixelFormatBC2_RGBA_sRGB;
            case wgpu::TextureFormat::BC3RGBAUnorm:
                return MTLPixelFormatBC3_RGBA;
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                return MTLPixelFormatBC3_RGBA_sRGB;
            case wgpu::TextureFormat::BC4RSnorm:
                return MTLPixelFormatBC4_RSnorm;
            case wgpu::TextureFormat::BC4RUnorm:
                return MTLPixelFormatBC4_RUnorm;
            case wgpu::TextureFormat::BC5RGSnorm:
                return MTLPixelFormatBC5_RGSnorm;
            case wgpu::TextureFormat::BC5RGUnorm:
                return MTLPixelFormatBC5_RGUnorm;
            case wgpu::TextureFormat::BC6HRGBFloat:
                return MTLPixelFormatBC6H_RGBFloat;
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return MTLPixelFormatBC6H_RGBUfloat;
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return MTLPixelFormatBC7_RGBAUnorm;
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return MTLPixelFormatBC7_RGBAUnorm_sRGB;
#else
            case wgpu::TextureFormat::BC1RGBAUnorm:
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
            case wgpu::TextureFormat::BC2RGBAUnorm:
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
            case wgpu::TextureFormat::BC3RGBAUnorm:
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
            case wgpu::TextureFormat::BC4RSnorm:
            case wgpu::TextureFormat::BC4RUnorm:
            case wgpu::TextureFormat::BC5RGSnorm:
            case wgpu::TextureFormat::BC5RGUnorm:
            case wgpu::TextureFormat::BC6HRGBFloat:
            case wgpu::TextureFormat::BC6HRGBUfloat:
            case wgpu::TextureFormat::BC7RGBAUnorm:
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
#endif

            case wgpu::TextureFormat::R8BG8Biplanar420Unorm:

            case wgpu::TextureFormat::ETC2RGB8Unorm:
            case wgpu::TextureFormat::ETC2RGB8UnormSrgb:
            case wgpu::TextureFormat::ETC2RGB8A1Unorm:
            case wgpu::TextureFormat::ETC2RGB8A1UnormSrgb:
            case wgpu::TextureFormat::ETC2RGBA8Unorm:
            case wgpu::TextureFormat::ETC2RGBA8UnormSrgb:
            case wgpu::TextureFormat::EACR11Unorm:
            case wgpu::TextureFormat::EACR11Snorm:
            case wgpu::TextureFormat::EACRG11Unorm:
            case wgpu::TextureFormat::EACRG11Snorm:

            case wgpu::TextureFormat::ASTC4x4Unorm:
            case wgpu::TextureFormat::ASTC4x4UnormSrgb:
            case wgpu::TextureFormat::ASTC5x4Unorm:
            case wgpu::TextureFormat::ASTC5x4UnormSrgb:
            case wgpu::TextureFormat::ASTC5x5Unorm:
            case wgpu::TextureFormat::ASTC5x5UnormSrgb:
            case wgpu::TextureFormat::ASTC6x5Unorm:
            case wgpu::TextureFormat::ASTC6x5UnormSrgb:
            case wgpu::TextureFormat::ASTC6x6Unorm:
            case wgpu::TextureFormat::ASTC6x6UnormSrgb:
            case wgpu::TextureFormat::ASTC8x5Unorm:
            case wgpu::TextureFormat::ASTC8x5UnormSrgb:
            case wgpu::TextureFormat::ASTC8x6Unorm:
            case wgpu::TextureFormat::ASTC8x6UnormSrgb:
            case wgpu::TextureFormat::ASTC8x8Unorm:
            case wgpu::TextureFormat::ASTC8x8UnormSrgb:
            case wgpu::TextureFormat::ASTC10x5Unorm:
            case wgpu::TextureFormat::ASTC10x5UnormSrgb:
            case wgpu::TextureFormat::ASTC10x6Unorm:
            case wgpu::TextureFormat::ASTC10x6UnormSrgb:
            case wgpu::TextureFormat::ASTC10x8Unorm:
            case wgpu::TextureFormat::ASTC10x8UnormSrgb:
            case wgpu::TextureFormat::ASTC10x10Unorm:
            case wgpu::TextureFormat::ASTC10x10UnormSrgb:
            case wgpu::TextureFormat::ASTC12x10Unorm:
            case wgpu::TextureFormat::ASTC12x10UnormSrgb:
            case wgpu::TextureFormat::ASTC12x12Unorm:
            case wgpu::TextureFormat::ASTC12x12UnormSrgb:

            // TODO(dawn:666): implement stencil8
            case wgpu::TextureFormat::Stencil8:
            // TODO(dawn:690): implement depth24unorm-stencil8
            case wgpu::TextureFormat::Depth24UnormStencil8:
            // TODO(dawn:690): implement depth32float-stencil8
            case wgpu::TextureFormat::Depth32FloatStencil8:
            case wgpu::TextureFormat::Undefined:
                UNREACHABLE();
        }
    }

    MaybeError ValidateIOSurfaceCanBeWrapped(const DeviceBase*,
                                             const TextureDescriptor* descriptor,
                                             IOSurfaceRef ioSurface,
                                             uint32_t plane) {
        // IOSurfaceGetPlaneCount can return 0 for non-planar IOSurfaces but we will treat
        // non-planar like it is a single plane.
        size_t surfacePlaneCount = std::max(size_t(1), IOSurfaceGetPlaneCount(ioSurface));
        DAWN_INVALID_IF(plane >= surfacePlaneCount,
                        "IOSurface plane (%u) exceeds the surface's plane count (%u).", plane,
                        surfacePlaneCount);

        DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                        "Texture dimension (%s) is not %s.", descriptor->dimension,
                        wgpu::TextureDimension::e2D);

        DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                        descriptor->mipLevelCount);

        DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1,
                        "Array layer count (%u) is not 1.", descriptor->size.depthOrArrayLayers);

        DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                        descriptor->sampleCount);

        uint32_t surfaceWidth = IOSurfaceGetWidthOfPlane(ioSurface, plane);
        uint32_t surfaceHeight = IOSurfaceGetHeightOfPlane(ioSurface, plane);

        DAWN_INVALID_IF(
            descriptor->size.width != surfaceWidth || descriptor->size.height != surfaceHeight ||
                descriptor->size.depthOrArrayLayers != 1,
            "IOSurface size (width: %u, height %u, depth: 1) doesn't match descriptor size %s.",
            surfaceWidth, surfaceHeight, &descriptor->size);

        wgpu::TextureFormat ioSurfaceFormat;
        DAWN_TRY_ASSIGN(ioSurfaceFormat,
                        GetFormatEquivalentToIOSurfaceFormat(IOSurfaceGetPixelFormat(ioSurface)));
        DAWN_INVALID_IF(descriptor->format != ioSurfaceFormat,
                        "IOSurface format (%s) doesn't match the descriptor format (%s).",
                        ioSurfaceFormat, descriptor->format);

        return {};
    }

    NSRef<MTLTextureDescriptor> Texture::CreateMetalTextureDescriptor() const {
        NSRef<MTLTextureDescriptor> mtlDescRef = AcquireNSRef([MTLTextureDescriptor new]);
        MTLTextureDescriptor* mtlDesc = mtlDescRef.Get();

        mtlDesc.width = GetWidth();
        mtlDesc.height = GetHeight();
        mtlDesc.sampleCount = GetSampleCount();
        // TODO: add MTLTextureUsagePixelFormatView when needed when we support format
        // reinterpretation.
        mtlDesc.usage = MetalTextureUsage(GetFormat(), GetInternalUsage(), GetSampleCount());
        mtlDesc.pixelFormat = MetalPixelFormat(GetFormat().format);
        mtlDesc.mipmapLevelCount = GetNumMipLevels();
        mtlDesc.storageMode = MTLStorageModePrivate;

        // Choose the correct MTLTextureType and paper over differences in how the array layer count
        // is specified.
        switch (GetDimension()) {
            case wgpu::TextureDimension::e2D:
                mtlDesc.depth = 1;
                mtlDesc.arrayLength = GetArrayLayers();
                if (mtlDesc.arrayLength > 1) {
                    ASSERT(mtlDesc.sampleCount == 1);
                    mtlDesc.textureType = MTLTextureType2DArray;
                } else if (mtlDesc.sampleCount > 1) {
                    mtlDesc.textureType = MTLTextureType2DMultisample;
                } else {
                    mtlDesc.textureType = MTLTextureType2D;
                }
                break;
            case wgpu::TextureDimension::e3D:
                mtlDesc.depth = GetDepth();
                mtlDesc.arrayLength = 1;
                ASSERT(mtlDesc.sampleCount == 1);
                mtlDesc.textureType = MTLTextureType3D;
                break;

            case wgpu::TextureDimension::e1D:
                UNREACHABLE();
        }

        return mtlDescRef;
    }

    // static
    ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                                const TextureDescriptor* descriptor) {
        Ref<Texture> texture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
        DAWN_TRY(texture->InitializeAsInternalTexture(descriptor));
        return texture;
    }

    // static
    ResultOrError<Ref<Texture>> Texture::CreateFromIOSurface(
        Device* device,
        const ExternalImageDescriptor* descriptor,
        IOSurfaceRef ioSurface,
        uint32_t plane) {
        const TextureDescriptor* textureDescriptor = FromAPI(descriptor->cTextureDescriptor);

        Ref<Texture> texture =
            AcquireRef(new Texture(device, textureDescriptor, TextureState::OwnedInternal));
        DAWN_TRY(texture->InitializeFromIOSurface(descriptor, textureDescriptor, ioSurface, plane));
        return texture;
    }

    // static
    Ref<Texture> Texture::CreateWrapping(Device* device,
                                         const TextureDescriptor* descriptor,
                                         NSPRef<id<MTLTexture>> wrapped) {
        Ref<Texture> texture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
        texture->InitializeAsWrapping(descriptor, std::move(wrapped));
        return texture;
    }

    MaybeError Texture::InitializeAsInternalTexture(const TextureDescriptor* descriptor) {
        Device* device = ToBackend(GetDevice());

        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
        mMtlUsage = [*mtlDesc usage];
        mMtlTexture =
            AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()]);

        if (mMtlTexture == nil) {
            return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate texture.");
        }

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            DAWN_TRY(ClearTexture(device->GetPendingCommandContext(), GetAllSubresources(),
                                  TextureBase::ClearValue::NonZero));
        }

        return {};
    }

    void Texture::InitializeAsWrapping(const TextureDescriptor* descriptor,
                                       NSPRef<id<MTLTexture>> wrapped) {
        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
        mMtlUsage = [*mtlDesc usage];
        mMtlTexture = std::move(wrapped);
    }

    MaybeError Texture::InitializeFromIOSurface(const ExternalImageDescriptor* descriptor,
                                                const TextureDescriptor* textureDescriptor,
                                                IOSurfaceRef ioSurface,
                                                uint32_t plane) {
        Device* device = ToBackend(GetDevice());

        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor();
        [*mtlDesc setStorageMode:kIOSurfaceStorageMode];

        mMtlUsage = [*mtlDesc usage];
        mMtlTexture = AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()
                                                                           iosurface:ioSurface
                                                                               plane:plane]);

        SetIsSubresourceContentInitialized(descriptor->isInitialized, GetAllSubresources());

        return {};
    }

    Texture::~Texture() {
    }

    void Texture::DestroyImpl() {
        TextureBase::DestroyImpl();
        mMtlTexture = nullptr;
    }

    id<MTLTexture> Texture::GetMTLTexture() {
        return mMtlTexture.Get();
    }

    MaybeError Texture::ClearTexture(CommandRecordingContext* commandContext,
                                     const SubresourceRange& range,
                                     TextureBase::ClearValue clearValue) {
        Device* device = ToBackend(GetDevice());

        const uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
        const double dClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.0 : 1.0;

        if ((mMtlUsage & MTLTextureUsageRenderTarget) != 0) {
            ASSERT(GetFormat().isRenderable);

            // End the blit encoder if it is open.
            commandContext->EndBlit();

            if (GetFormat().HasDepthOrStencil()) {
                // Create a render pass to clear each subresource.
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    for (uint32_t arrayLayer = range.baseArrayLayer;
                         arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                                level, arrayLayer, range.aspects))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        // Note that this creates a descriptor that's autoreleased so we don't use
                        // AcquireNSRef
                        NSRef<MTLRenderPassDescriptor> descriptorRef =
                            [MTLRenderPassDescriptor renderPassDescriptor];
                        MTLRenderPassDescriptor* descriptor = descriptorRef.Get();

                        // At least one aspect needs clearing. Iterate the aspects individually to
                        // determine which to clear.
                        for (Aspect aspect : IterateEnumMask(range.aspects)) {
                            if (clearValue == TextureBase::ClearValue::Zero &&
                                IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                                    level, arrayLayer, aspect))) {
                                // Skip lazy clears if already initialized.
                                continue;
                            }

                            ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
                            switch (aspect) {
                                case Aspect::Depth:
                                    descriptor.depthAttachment.texture = GetMTLTexture();
                                    descriptor.depthAttachment.level = level;
                                    descriptor.depthAttachment.slice = arrayLayer;
                                    descriptor.depthAttachment.loadAction = MTLLoadActionClear;
                                    descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                                    descriptor.depthAttachment.clearDepth = dClearColor;
                                    break;
                                case Aspect::Stencil:
                                    descriptor.stencilAttachment.texture = GetMTLTexture();
                                    descriptor.stencilAttachment.level = level;
                                    descriptor.stencilAttachment.slice = arrayLayer;
                                    descriptor.stencilAttachment.loadAction = MTLLoadActionClear;
                                    descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                                    descriptor.stencilAttachment.clearStencil =
                                        static_cast<uint32_t>(clearColor);
                                    break;
                                default:
                                    UNREACHABLE();
                            }
                        }

                        commandContext->BeginRender(descriptor);
                        commandContext->EndRender();
                    }
                }
            } else {
                ASSERT(GetFormat().IsColor());
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    // Create multiple render passes with each subresource as a color attachment to
                    // clear them all. Only do this for array layers to ensure all attachments have
                    // the same size.
                    NSRef<MTLRenderPassDescriptor> descriptor;
                    uint32_t attachment = 0;

                    uint32_t numZSlices = GetMipLevelVirtualSize(level).depthOrArrayLayers;

                    for (uint32_t arrayLayer = range.baseArrayLayer;
                         arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                                level, arrayLayer, Aspect::Color))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        for (uint32_t z = 0; z < numZSlices; ++z) {
                            if (descriptor == nullptr) {
                                // Note that this creates a descriptor that's autoreleased so we
                                // don't use AcquireNSRef
                                descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
                            }

                            [*descriptor colorAttachments][attachment].texture = GetMTLTexture();
                            [*descriptor colorAttachments][attachment].loadAction =
                                MTLLoadActionClear;
                            [*descriptor colorAttachments][attachment].storeAction =
                                MTLStoreActionStore;
                            [*descriptor colorAttachments][attachment].clearColor =
                                MTLClearColorMake(dClearColor, dClearColor, dClearColor,
                                                  dClearColor);
                            [*descriptor colorAttachments][attachment].level = level;
                            [*descriptor colorAttachments][attachment].slice = arrayLayer;
                            [*descriptor colorAttachments][attachment].depthPlane = z;

                            attachment++;

                            if (attachment == kMaxColorAttachments) {
                                attachment = 0;
                                commandContext->BeginRender(descriptor.Get());
                                commandContext->EndRender();
                                descriptor = nullptr;
                            }
                        }
                    }

                    if (descriptor != nullptr) {
                        commandContext->BeginRender(descriptor.Get());
                        commandContext->EndRender();
                    }
                }
            }
        } else {
            Extent3D largestMipSize = GetMipLevelVirtualSize(range.baseMipLevel);

            // Encode a buffer to texture copy to clear each subresource.
            for (Aspect aspect : IterateEnumMask(range.aspects)) {
                // Compute the buffer size big enough to fill the largest mip.
                const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(aspect).block;

                // Metal validation layers: sourceBytesPerRow must be at least 64.
                uint32_t largestMipBytesPerRow =
                    std::max((largestMipSize.width / blockInfo.width) * blockInfo.byteSize, 64u);

                // Metal validation layers: sourceBytesPerImage must be at least 512.
                uint64_t largestMipBytesPerImage =
                    std::max(static_cast<uint64_t>(largestMipBytesPerRow) *
                                 (largestMipSize.height / blockInfo.height),
                             512llu);

                uint64_t bufferSize = largestMipBytesPerImage * largestMipSize.depthOrArrayLayers;

                if (bufferSize > std::numeric_limits<NSUInteger>::max()) {
                    return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
                }

                DynamicUploader* uploader = device->GetDynamicUploader();
                UploadHandle uploadHandle;
                DAWN_TRY_ASSIGN(uploadHandle,
                                uploader->Allocate(bufferSize, device->GetPendingCommandSerial(),
                                                   blockInfo.byteSize));
                memset(uploadHandle.mappedBuffer, clearColor, bufferSize);

                id<MTLBuffer> uploadBuffer =
                    ToBackend(uploadHandle.stagingBuffer)->GetBufferHandle();

                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    Extent3D virtualSize = GetMipLevelVirtualSize(level);

                    for (uint32_t arrayLayer = range.baseArrayLayer;
                         arrayLayer < range.baseArrayLayer + range.layerCount; ++arrayLayer) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, arrayLayer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        MTLBlitOption blitOption = ComputeMTLBlitOption(GetFormat(), aspect);
                        [commandContext->EnsureBlit()
                                 copyFromBuffer:uploadBuffer
                                   sourceOffset:uploadHandle.startOffset
                              sourceBytesPerRow:largestMipBytesPerRow
                            sourceBytesPerImage:largestMipBytesPerImage
                                     sourceSize:MTLSizeMake(virtualSize.width, virtualSize.height,
                                                            virtualSize.depthOrArrayLayers)
                                      toTexture:GetMTLTexture()
                               destinationSlice:arrayLayer
                               destinationLevel:level
                              destinationOrigin:MTLOriginMake(0, 0, 0)
                                        options:blitOption];
                    }
                }
            }
        }

        if (clearValue == TextureBase::ClearValue::Zero) {
            SetIsSubresourceContentInitialized(true, range);
            device->IncrementLazyClearCountForTesting();
        }
        return {};
    }

    void Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                      const SubresourceRange& range) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(range)) {
            // If subresource has not been initialized, clear it to black as it could
            // contain dirty bits from recycled memory
            GetDevice()->ConsumedError(
                ClearTexture(commandContext, range, TextureBase::ClearValue::Zero));
        }
    }

    // static
    ResultOrError<Ref<TextureView>> TextureView::Create(TextureBase* texture,
                                                        const TextureViewDescriptor* descriptor) {
        Ref<TextureView> view = AcquireRef(new TextureView(texture, descriptor));
        DAWN_TRY(view->Initialize(descriptor));
        return view;
    }

    MaybeError TextureView::Initialize(const TextureViewDescriptor* descriptor) {
        Texture* texture = ToBackend(GetTexture());

        // Texture could be destroyed by the time we make a view.
        if (GetTexture()->GetTextureState() == Texture::TextureState::Destroyed) {
            return {};
        }

        id<MTLTexture> mtlTexture = texture->GetMTLTexture();

        if (!UsageNeedsTextureView(texture->GetInternalUsage())) {
            mMtlTextureView = nullptr;
        } else if (!RequiresCreatingNewTextureView(texture, descriptor)) {
            mMtlTextureView = mtlTexture;
        } else {
            MTLPixelFormat format = MetalPixelFormat(descriptor->format);
            if (descriptor->aspect == wgpu::TextureAspect::StencilOnly) {
                if (@available(macOS 10.12, iOS 10.0, *)) {
                    ASSERT(format == MTLPixelFormatDepth32Float_Stencil8);
                    format = MTLPixelFormatX32_Stencil8;
                } else {
                    // TODO(enga): Add a workaround to back combined depth/stencil textures
                    // with Sampled usage using two separate textures.
                    // Or, consider always using the workaround for D32S8.
                    GetDevice()->ConsumedError(
                        DAWN_DEVICE_LOST_ERROR("Cannot create stencil-only texture view of "
                                               "combined depth/stencil format."));
                }
            }

            MTLTextureType textureViewType =
                MetalTextureViewType(descriptor->dimension, texture->GetSampleCount());
            auto mipLevelRange = NSMakeRange(descriptor->baseMipLevel, descriptor->mipLevelCount);
            auto arrayLayerRange =
                NSMakeRange(descriptor->baseArrayLayer, descriptor->arrayLayerCount);

            mMtlTextureView =
                AcquireNSPRef([mtlTexture newTextureViewWithPixelFormat:format
                                                            textureType:textureViewType
                                                                 levels:mipLevelRange
                                                                 slices:arrayLayerRange]);
            if (mMtlTextureView == nil) {
                return DAWN_INTERNAL_ERROR("Failed to create MTLTexture view.");
            }
        }

        return {};
    }

    id<MTLTexture> TextureView::GetMTLTexture() {
        ASSERT(mMtlTextureView != nullptr);
        return mMtlTextureView.Get();
    }
}}  // namespace dawn_native::metal
