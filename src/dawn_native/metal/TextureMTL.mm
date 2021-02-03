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
                wgpu::TextureUsage::Storage | wgpu::TextureUsage::Sampled;
            return usage & kUsageNeedsTextureView;
        }

        MTLTextureUsage MetalTextureUsage(const Format& format, wgpu::TextureUsage usage) {
            MTLTextureUsage result = MTLTextureUsageUnknown;  // This is 0

            if (usage & (wgpu::TextureUsage::Storage)) {
                result |= MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
            }

            if (usage & (wgpu::TextureUsage::Sampled)) {
                result |= MTLTextureUsageShaderRead;

                // For sampling stencil aspect of combined depth/stencil. See TextureView
                // constructor.
                if (@available(macOS 10.12, iOS 10.0, *)) {
                    if (IsSubset(Aspect::Depth | Aspect::Stencil, format.aspects)) {
                        result |= MTLTextureUsagePixelFormatView;
                    }
                }
            }

            if (usage & (wgpu::TextureUsage::RenderAttachment)) {
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
                    return DAWN_VALIDATION_ERROR("Unsupported IOSurface format");
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
#endif

            default:
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
        if (plane >= surfacePlaneCount) {
            return DAWN_VALIDATION_ERROR("IOSurface plane doesn't exist");
        }

        if (descriptor->dimension != wgpu::TextureDimension::e2D) {
            return DAWN_VALIDATION_ERROR("IOSurface texture must be 2D");
        }

        if (descriptor->mipLevelCount != 1) {
            return DAWN_VALIDATION_ERROR("IOSurface mip level count must be 1");
        }

        if (descriptor->size.depth != 1) {
            return DAWN_VALIDATION_ERROR("IOSurface array layer count must be 1");
        }

        if (descriptor->sampleCount != 1) {
            return DAWN_VALIDATION_ERROR("IOSurface sample count must be 1");
        }

        if (descriptor->size.width != IOSurfaceGetWidthOfPlane(ioSurface, plane) ||
            descriptor->size.height != IOSurfaceGetHeightOfPlane(ioSurface, plane) ||
            descriptor->size.depth != 1) {
            return DAWN_VALIDATION_ERROR("IOSurface size doesn't match descriptor");
        }

        wgpu::TextureFormat ioSurfaceFormat;
        DAWN_TRY_ASSIGN(ioSurfaceFormat,
                        GetFormatEquivalentToIOSurfaceFormat(IOSurfaceGetPixelFormat(ioSurface)));
        if (descriptor->format != ioSurfaceFormat) {
            return DAWN_VALIDATION_ERROR("IOSurface format doesn't match descriptor");
        }

        return {};
    }

    NSRef<MTLTextureDescriptor> CreateMetalTextureDescriptor(DeviceBase* device,
                                                             const TextureDescriptor* descriptor) {
        NSRef<MTLTextureDescriptor> mtlDescRef = AcquireNSRef([MTLTextureDescriptor new]);
        MTLTextureDescriptor* mtlDesc = mtlDescRef.Get();

        mtlDesc.width = descriptor->size.width;
        mtlDesc.height = descriptor->size.height;
        mtlDesc.sampleCount = descriptor->sampleCount;
        // TODO: add MTLTextureUsagePixelFormatView when needed when we support format
        // reinterpretation.
        mtlDesc.usage = MetalTextureUsage(device->GetValidInternalFormat(descriptor->format),
                                          descriptor->usage);
        mtlDesc.pixelFormat = MetalPixelFormat(descriptor->format);
        mtlDesc.mipmapLevelCount = descriptor->mipLevelCount;
        mtlDesc.storageMode = MTLStorageModePrivate;

        // Choose the correct MTLTextureType and paper over differences in how the array layer count
        // is specified.
        mtlDesc.depth = descriptor->size.depth;
        mtlDesc.arrayLength = 1;
        switch (descriptor->dimension) {
            case wgpu::TextureDimension::e2D:
                if (mtlDesc.depth > 1) {
                    ASSERT(mtlDesc.sampleCount == 1);
                    mtlDesc.textureType = MTLTextureType2DArray;
                    mtlDesc.arrayLength = mtlDesc.depth;
                    mtlDesc.depth = 1;
                } else {
                    if (mtlDesc.sampleCount > 1) {
                        mtlDesc.textureType = MTLTextureType2DMultisample;
                    } else {
                        mtlDesc.textureType = MTLTextureType2D;
                    }
                }
                break;
            case wgpu::TextureDimension::e3D:
                ASSERT(mtlDesc.sampleCount == 1);
                mtlDesc.textureType = MTLTextureType3D;
                break;

            case wgpu::TextureDimension::e1D:
                UNREACHABLE();
        }

        return mtlDescRef;
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor)
        : TextureBase(device, descriptor, TextureState::OwnedInternal) {
        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor(device, descriptor);
        mMtlTexture =
            AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()]);

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            device->ConsumedError(
                ClearTexture(GetAllSubresources(), TextureBase::ClearValue::NonZero));
        }
    }

    Texture::Texture(Device* device,
                     const TextureDescriptor* descriptor,
                     NSPRef<id<MTLTexture>> mtlTexture)
        : TextureBase(device, descriptor, TextureState::OwnedInternal),
          mMtlTexture(std::move(mtlTexture)) {
    }

    Texture::Texture(Device* device,
                     const ExternalImageDescriptor* descriptor,
                     IOSurfaceRef ioSurface,
                     uint32_t plane)
        : TextureBase(device,
                      reinterpret_cast<const TextureDescriptor*>(descriptor->cTextureDescriptor),
                      TextureState::OwnedInternal) {
        NSRef<MTLTextureDescriptor> mtlDesc = CreateMetalTextureDescriptor(
            device, reinterpret_cast<const TextureDescriptor*>(descriptor->cTextureDescriptor));
        [*mtlDesc setStorageMode:kIOSurfaceStorageMode];

        mMtlTexture = AcquireNSPRef([device->GetMTLDevice() newTextureWithDescriptor:mtlDesc.Get()
                                                                           iosurface:ioSurface
                                                                               plane:plane]);

        SetIsSubresourceContentInitialized(descriptor->isInitialized, GetAllSubresources());
    }

    Texture::~Texture() {
        DestroyInternal();
    }

    void Texture::DestroyImpl() {
        mMtlTexture = nullptr;
    }

    id<MTLTexture> Texture::GetMTLTexture() {
        return mMtlTexture.Get();
    }

    MaybeError Texture::ClearTexture(const SubresourceRange& range,
                                     TextureBase::ClearValue clearValue) {
        Device* device = ToBackend(GetDevice());

        CommandRecordingContext* commandContext = device->GetPendingCommandContext();

        const uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
        const double dClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.0 : 1.0;

        if ((GetUsage() & wgpu::TextureUsage::RenderAttachment) != 0) {
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

                    for (uint32_t arrayLayer = range.baseArrayLayer;
                         arrayLayer < range.baseArrayLayer + range.layerCount; arrayLayer++) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(SubresourceRange::SingleMipAndLayer(
                                level, arrayLayer, Aspect::Color))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        if (descriptor == nullptr) {
                            // Note that this creates a descriptor that's autoreleased so we don't
                            // use AcquireNSRef
                            descriptor = [MTLRenderPassDescriptor renderPassDescriptor];
                        }

                        [*descriptor colorAttachments][attachment].texture = GetMTLTexture();
                        [*descriptor colorAttachments][attachment].loadAction = MTLLoadActionClear;
                        [*descriptor colorAttachments][attachment].storeAction =
                            MTLStoreActionStore;
                        [*descriptor colorAttachments][attachment].clearColor =
                            MTLClearColorMake(dClearColor, dClearColor, dClearColor, dClearColor);
                        [*descriptor colorAttachments][attachment].level = level;
                        [*descriptor colorAttachments][attachment].slice = arrayLayer;

                        attachment++;

                        if (attachment == kMaxColorAttachments) {
                            attachment = 0;
                            commandContext->BeginRender(descriptor.Get());
                            commandContext->EndRender();
                            descriptor = nullptr;
                        }
                    }

                    if (descriptor != nullptr) {
                        commandContext->BeginRender(descriptor.Get());
                        commandContext->EndRender();
                    }
                }
            }
        } else {
            // Compute the buffer size big enough to fill the largest mip.
            Extent3D largestMipSize = GetMipLevelVirtualSize(range.baseMipLevel);
            const TexelBlockInfo& blockInfo =
                GetFormat().GetAspectInfo(wgpu::TextureAspect::All).block;

            // Metal validation layers: sourceBytesPerRow must be at least 64.
            uint32_t largestMipBytesPerRow =
                std::max((largestMipSize.width / blockInfo.width) * blockInfo.byteSize, 64u);

            // Metal validation layers: sourceBytesPerImage must be at least 512.
            uint64_t largestMipBytesPerImage =
                std::max(static_cast<uint64_t>(largestMipBytesPerRow) *
                             (largestMipSize.height / blockInfo.height),
                         512llu);

            // TODO(enga): Multiply by largestMipSize.depth and do a larger 3D copy to clear a whole
            // range of subresources when tracking that is improved.
            uint64_t bufferSize = largestMipBytesPerImage * 1;

            if (bufferSize > std::numeric_limits<NSUInteger>::max()) {
                return DAWN_OUT_OF_MEMORY_ERROR("Unable to allocate buffer.");
            }

            DynamicUploader* uploader = device->GetDynamicUploader();
            UploadHandle uploadHandle;
            DAWN_TRY_ASSIGN(uploadHandle,
                            uploader->Allocate(bufferSize, device->GetPendingCommandSerial(),
                                               blockInfo.byteSize));
            memset(uploadHandle.mappedBuffer, clearColor, bufferSize);

            id<MTLBlitCommandEncoder> encoder = commandContext->EnsureBlit();
            id<MTLBuffer> uploadBuffer = ToBackend(uploadHandle.stagingBuffer)->GetBufferHandle();

            // Encode a buffer to texture copy to clear each subresource.
            for (Aspect aspect : IterateEnumMask(range.aspects)) {
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
                        [encoder copyFromBuffer:uploadBuffer
                                   sourceOffset:uploadHandle.startOffset
                              sourceBytesPerRow:largestMipBytesPerRow
                            sourceBytesPerImage:largestMipBytesPerImage
                                     sourceSize:MTLSizeMake(virtualSize.width, virtualSize.height,
                                                            1)
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

    void Texture::EnsureSubresourceContentInitialized(const SubresourceRange& range) {
        if (!GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(range)) {
            // If subresource has not been initialized, clear it to black as it could
            // contain dirty bits from recycled memory
            GetDevice()->ConsumedError(ClearTexture(range, TextureBase::ClearValue::Zero));
        }
    }

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor) {
        id<MTLTexture> mtlTexture = ToBackend(texture)->GetMTLTexture();

        if (!UsageNeedsTextureView(texture->GetUsage())) {
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
        }
    }

    id<MTLTexture> TextureView::GetMTLTexture() {
        ASSERT(mMtlTextureView != nullptr);
        return mMtlTextureView.Get();
    }
}}  // namespace dawn_native::metal
