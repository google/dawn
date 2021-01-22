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

#include "dawn_native/d3d12/TextureD3D12.h"

#include "common/Constants.h"
#include "common/Math.h"
#include "dawn_native/DynamicUploader.h"
#include "dawn_native/EnumMaskIterator.h"
#include "dawn_native/Error.h"
#include "dawn_native/d3d12/BufferD3D12.h"
#include "dawn_native/d3d12/CommandRecordingContext.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/HeapD3D12.h"
#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"
#include "dawn_native/d3d12/StagingBufferD3D12.h"
#include "dawn_native/d3d12/StagingDescriptorAllocatorD3D12.h"
#include "dawn_native/d3d12/TextureCopySplitter.h"
#include "dawn_native/d3d12/UtilsD3D12.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_RESOURCE_STATES D3D12TextureUsage(wgpu::TextureUsage usage, const Format& format) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            if (usage & kPresentTextureUsage) {
                // The present usage is only used internally by the swapchain and is never used in
                // combination with other usages.
                ASSERT(usage == kPresentTextureUsage);
                return D3D12_RESOURCE_STATE_PRESENT;
            }

            if (usage & wgpu::TextureUsage::CopySrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & wgpu::TextureUsage::CopyDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & (wgpu::TextureUsage::Sampled | kReadOnlyStorageTexture)) {
                resourceState |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                                  D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
            }
            if (usage & wgpu::TextureUsage::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & wgpu::TextureUsage::RenderAttachment) {
                if (format.HasDepthOrStencil()) {
                    resourceState |= D3D12_RESOURCE_STATE_DEPTH_WRITE;
                } else {
                    resourceState |= D3D12_RESOURCE_STATE_RENDER_TARGET;
                }
            }

            return resourceState;
        }

        D3D12_RESOURCE_FLAGS D3D12ResourceFlags(wgpu::TextureUsage usage,
                                                const Format& format,
                                                bool isMultisampledTexture) {
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

            if (usage & wgpu::TextureUsage::Storage) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            // A multisampled resource must have either D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET or
            // D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL set in D3D12_RESOURCE_DESC::Flags.
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_resource_desc
            if ((usage & wgpu::TextureUsage::RenderAttachment) != 0 || isMultisampledTexture) {
                if (format.HasDepthOrStencil()) {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
                } else {
                    flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
                }
            }

            ASSERT(!(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ||
                   flags == D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            return flags;
        }

        D3D12_RESOURCE_DIMENSION D3D12TextureDimension(wgpu::TextureDimension dimension) {
            switch (dimension) {
                case wgpu::TextureDimension::e2D:
                    return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

                case wgpu::TextureDimension::e1D:
                case wgpu::TextureDimension::e3D:
                    UNREACHABLE();
            }
        }

        DXGI_FORMAT D3D12TypelessTextureFormat(wgpu::TextureFormat format) {
            switch (format) {
                case wgpu::TextureFormat::R8Unorm:
                case wgpu::TextureFormat::R8Snorm:
                case wgpu::TextureFormat::R8Uint:
                case wgpu::TextureFormat::R8Sint:
                    return DXGI_FORMAT_R8_TYPELESS;

                case wgpu::TextureFormat::R16Uint:
                case wgpu::TextureFormat::R16Sint:
                case wgpu::TextureFormat::R16Float:
                    return DXGI_FORMAT_R16_TYPELESS;

                case wgpu::TextureFormat::RG8Unorm:
                case wgpu::TextureFormat::RG8Snorm:
                case wgpu::TextureFormat::RG8Uint:
                case wgpu::TextureFormat::RG8Sint:
                    return DXGI_FORMAT_R8G8_TYPELESS;

                case wgpu::TextureFormat::R32Uint:
                case wgpu::TextureFormat::R32Sint:
                case wgpu::TextureFormat::R32Float:
                    return DXGI_FORMAT_R32_TYPELESS;

                case wgpu::TextureFormat::RG16Uint:
                case wgpu::TextureFormat::RG16Sint:
                case wgpu::TextureFormat::RG16Float:
                    return DXGI_FORMAT_R16G16_TYPELESS;

                case wgpu::TextureFormat::RGBA8Unorm:
                case wgpu::TextureFormat::RGBA8UnormSrgb:
                case wgpu::TextureFormat::RGBA8Snorm:
                case wgpu::TextureFormat::RGBA8Uint:
                case wgpu::TextureFormat::RGBA8Sint:
                    return DXGI_FORMAT_R8G8B8A8_TYPELESS;

                case wgpu::TextureFormat::BGRA8Unorm:
                case wgpu::TextureFormat::BGRA8UnormSrgb:
                    return DXGI_FORMAT_B8G8R8A8_TYPELESS;

                case wgpu::TextureFormat::RGB10A2Unorm:
                    return DXGI_FORMAT_R10G10B10A2_TYPELESS;

                case wgpu::TextureFormat::RG11B10Ufloat:
                    return DXGI_FORMAT_R11G11B10_FLOAT;
                case wgpu::TextureFormat::RGB9E5Ufloat:
                    return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

                case wgpu::TextureFormat::RG32Uint:
                case wgpu::TextureFormat::RG32Sint:
                case wgpu::TextureFormat::RG32Float:
                    return DXGI_FORMAT_R32G32_TYPELESS;

                case wgpu::TextureFormat::RGBA16Uint:
                case wgpu::TextureFormat::RGBA16Sint:
                case wgpu::TextureFormat::RGBA16Float:
                    return DXGI_FORMAT_R16G16B16A16_TYPELESS;

                case wgpu::TextureFormat::RGBA32Uint:
                case wgpu::TextureFormat::RGBA32Sint:
                case wgpu::TextureFormat::RGBA32Float:
                    return DXGI_FORMAT_R32G32B32A32_TYPELESS;

                case wgpu::TextureFormat::Depth32Float:
                case wgpu::TextureFormat::Depth24Plus:
                    return DXGI_FORMAT_R32_TYPELESS;

                case wgpu::TextureFormat::Depth24PlusStencil8:
                    return DXGI_FORMAT_R32G8X24_TYPELESS;

                case wgpu::TextureFormat::BC1RGBAUnorm:
                case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                    return DXGI_FORMAT_BC1_TYPELESS;

                case wgpu::TextureFormat::BC2RGBAUnorm:
                case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                    return DXGI_FORMAT_BC2_TYPELESS;

                case wgpu::TextureFormat::BC3RGBAUnorm:
                case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                    return DXGI_FORMAT_BC3_TYPELESS;

                case wgpu::TextureFormat::BC4RSnorm:
                case wgpu::TextureFormat::BC4RUnorm:
                    return DXGI_FORMAT_BC4_TYPELESS;

                case wgpu::TextureFormat::BC5RGSnorm:
                case wgpu::TextureFormat::BC5RGUnorm:
                    return DXGI_FORMAT_BC5_TYPELESS;

                case wgpu::TextureFormat::BC6HRGBFloat:
                case wgpu::TextureFormat::BC6HRGBUfloat:
                    return DXGI_FORMAT_BC6H_TYPELESS;

                case wgpu::TextureFormat::BC7RGBAUnorm:
                case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                    return DXGI_FORMAT_BC7_TYPELESS;

                case wgpu::TextureFormat::Undefined:
                    UNREACHABLE();
            }
        }

    }  // namespace

    DXGI_FORMAT D3D12TextureFormat(wgpu::TextureFormat format) {
        switch (format) {
            case wgpu::TextureFormat::R8Unorm:
                return DXGI_FORMAT_R8_UNORM;
            case wgpu::TextureFormat::R8Snorm:
                return DXGI_FORMAT_R8_SNORM;
            case wgpu::TextureFormat::R8Uint:
                return DXGI_FORMAT_R8_UINT;
            case wgpu::TextureFormat::R8Sint:
                return DXGI_FORMAT_R8_SINT;

            case wgpu::TextureFormat::R16Uint:
                return DXGI_FORMAT_R16_UINT;
            case wgpu::TextureFormat::R16Sint:
                return DXGI_FORMAT_R16_SINT;
            case wgpu::TextureFormat::R16Float:
                return DXGI_FORMAT_R16_FLOAT;
            case wgpu::TextureFormat::RG8Unorm:
                return DXGI_FORMAT_R8G8_UNORM;
            case wgpu::TextureFormat::RG8Snorm:
                return DXGI_FORMAT_R8G8_SNORM;
            case wgpu::TextureFormat::RG8Uint:
                return DXGI_FORMAT_R8G8_UINT;
            case wgpu::TextureFormat::RG8Sint:
                return DXGI_FORMAT_R8G8_SINT;

            case wgpu::TextureFormat::R32Uint:
                return DXGI_FORMAT_R32_UINT;
            case wgpu::TextureFormat::R32Sint:
                return DXGI_FORMAT_R32_SINT;
            case wgpu::TextureFormat::R32Float:
                return DXGI_FORMAT_R32_FLOAT;
            case wgpu::TextureFormat::RG16Uint:
                return DXGI_FORMAT_R16G16_UINT;
            case wgpu::TextureFormat::RG16Sint:
                return DXGI_FORMAT_R16G16_SINT;
            case wgpu::TextureFormat::RG16Float:
                return DXGI_FORMAT_R16G16_FLOAT;
            case wgpu::TextureFormat::RGBA8Unorm:
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            case wgpu::TextureFormat::RGBA8UnormSrgb:
                return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            case wgpu::TextureFormat::RGBA8Snorm:
                return DXGI_FORMAT_R8G8B8A8_SNORM;
            case wgpu::TextureFormat::RGBA8Uint:
                return DXGI_FORMAT_R8G8B8A8_UINT;
            case wgpu::TextureFormat::RGBA8Sint:
                return DXGI_FORMAT_R8G8B8A8_SINT;
            case wgpu::TextureFormat::BGRA8Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case wgpu::TextureFormat::BGRA8UnormSrgb:
                return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            case wgpu::TextureFormat::RGB10A2Unorm:
                return DXGI_FORMAT_R10G10B10A2_UNORM;
            case wgpu::TextureFormat::RG11B10Ufloat:
                return DXGI_FORMAT_R11G11B10_FLOAT;
            case wgpu::TextureFormat::RGB9E5Ufloat:
                return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;

            case wgpu::TextureFormat::RG32Uint:
                return DXGI_FORMAT_R32G32_UINT;
            case wgpu::TextureFormat::RG32Sint:
                return DXGI_FORMAT_R32G32_SINT;
            case wgpu::TextureFormat::RG32Float:
                return DXGI_FORMAT_R32G32_FLOAT;
            case wgpu::TextureFormat::RGBA16Uint:
                return DXGI_FORMAT_R16G16B16A16_UINT;
            case wgpu::TextureFormat::RGBA16Sint:
                return DXGI_FORMAT_R16G16B16A16_SINT;
            case wgpu::TextureFormat::RGBA16Float:
                return DXGI_FORMAT_R16G16B16A16_FLOAT;

            case wgpu::TextureFormat::RGBA32Uint:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            case wgpu::TextureFormat::RGBA32Sint:
                return DXGI_FORMAT_R32G32B32A32_SINT;
            case wgpu::TextureFormat::RGBA32Float:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;

            case wgpu::TextureFormat::Depth32Float:
                return DXGI_FORMAT_D32_FLOAT;
            case wgpu::TextureFormat::Depth24Plus:
                return DXGI_FORMAT_D32_FLOAT;
            case wgpu::TextureFormat::Depth24PlusStencil8:
                return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

            case wgpu::TextureFormat::BC1RGBAUnorm:
                return DXGI_FORMAT_BC1_UNORM;
            case wgpu::TextureFormat::BC1RGBAUnormSrgb:
                return DXGI_FORMAT_BC1_UNORM_SRGB;
            case wgpu::TextureFormat::BC2RGBAUnorm:
                return DXGI_FORMAT_BC2_UNORM;
            case wgpu::TextureFormat::BC2RGBAUnormSrgb:
                return DXGI_FORMAT_BC2_UNORM_SRGB;
            case wgpu::TextureFormat::BC3RGBAUnorm:
                return DXGI_FORMAT_BC3_UNORM;
            case wgpu::TextureFormat::BC3RGBAUnormSrgb:
                return DXGI_FORMAT_BC3_UNORM_SRGB;
            case wgpu::TextureFormat::BC4RSnorm:
                return DXGI_FORMAT_BC4_SNORM;
            case wgpu::TextureFormat::BC4RUnorm:
                return DXGI_FORMAT_BC4_UNORM;
            case wgpu::TextureFormat::BC5RGSnorm:
                return DXGI_FORMAT_BC5_SNORM;
            case wgpu::TextureFormat::BC5RGUnorm:
                return DXGI_FORMAT_BC5_UNORM;
            case wgpu::TextureFormat::BC6HRGBFloat:
                return DXGI_FORMAT_BC6H_SF16;
            case wgpu::TextureFormat::BC6HRGBUfloat:
                return DXGI_FORMAT_BC6H_UF16;
            case wgpu::TextureFormat::BC7RGBAUnorm:
                return DXGI_FORMAT_BC7_UNORM;
            case wgpu::TextureFormat::BC7RGBAUnormSrgb:
                return DXGI_FORMAT_BC7_UNORM_SRGB;

            case wgpu::TextureFormat::Undefined:
                UNREACHABLE();
        }
    }

    MaybeError ValidateTextureDescriptorCanBeWrapped(const TextureDescriptor* descriptor) {
        if (descriptor->dimension != wgpu::TextureDimension::e2D) {
            return DAWN_VALIDATION_ERROR("Texture must be 2D");
        }

        if (descriptor->mipLevelCount != 1) {
            return DAWN_VALIDATION_ERROR("Mip level count must be 1");
        }

        if (descriptor->size.depth != 1) {
            return DAWN_VALIDATION_ERROR("Depth must be 1");
        }

        if (descriptor->sampleCount != 1) {
            return DAWN_VALIDATION_ERROR("Sample count must be 1");
        }

        return {};
    }

    MaybeError ValidateD3D12TextureCanBeWrapped(ID3D12Resource* d3d12Resource,
                                                const TextureDescriptor* dawnDescriptor) {
        const D3D12_RESOURCE_DESC d3dDescriptor = d3d12Resource->GetDesc();
        if ((dawnDescriptor->size.width != d3dDescriptor.Width) ||
            (dawnDescriptor->size.height != d3dDescriptor.Height) ||
            (dawnDescriptor->size.depth != 1)) {
            return DAWN_VALIDATION_ERROR("D3D12 texture size doesn't match descriptor");
        }

        const DXGI_FORMAT dxgiFormatFromDescriptor = D3D12TextureFormat(dawnDescriptor->format);
        if (dxgiFormatFromDescriptor != d3dDescriptor.Format) {
            return DAWN_VALIDATION_ERROR(
                "D3D12 texture format must be compatible with descriptor format.");
        }

        if (d3dDescriptor.MipLevels != 1) {
            return DAWN_VALIDATION_ERROR("D3D12 texture number of miplevels must be 1.");
        }

        if (d3dDescriptor.DepthOrArraySize != 1) {
            return DAWN_VALIDATION_ERROR("D3D12 texture array size must be 1.");
        }

        // Shared textures cannot be multi-sample so no need to check those.
        ASSERT(d3dDescriptor.SampleDesc.Count == 1);
        ASSERT(d3dDescriptor.SampleDesc.Quality == 0);

        return {};
    }

    // static
    ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                                const TextureDescriptor* descriptor) {
        Ref<Texture> dawnTexture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedInternal));
        DAWN_TRY(dawnTexture->InitializeAsInternalTexture());
        return std::move(dawnTexture);
    }

    // static
    ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                                const ExternalImageDescriptor* descriptor,
                                                HANDLE sharedHandle,
                                                ExternalMutexSerial acquireMutexKey,
                                                bool isSwapChainTexture) {
        const TextureDescriptor* textureDescriptor =
            reinterpret_cast<const TextureDescriptor*>(descriptor->cTextureDescriptor);

        Ref<Texture> dawnTexture =
            AcquireRef(new Texture(device, textureDescriptor, TextureState::OwnedExternal));
        DAWN_TRY(dawnTexture->InitializeAsExternalTexture(textureDescriptor, sharedHandle,
                                                          acquireMutexKey, isSwapChainTexture));
        dawnTexture->SetIsSubresourceContentInitialized(descriptor->isInitialized,
                                                        dawnTexture->GetAllSubresources());
        return std::move(dawnTexture);
    }

    // static
    ResultOrError<Ref<Texture>> Texture::Create(Device* device,
                                                const TextureDescriptor* descriptor,
                                                ComPtr<ID3D12Resource> d3d12Texture) {
        Ref<Texture> dawnTexture =
            AcquireRef(new Texture(device, descriptor, TextureState::OwnedExternal));
        DAWN_TRY(dawnTexture->InitializeAsSwapChainTexture(std::move(d3d12Texture)));
        return std::move(dawnTexture);
    }

    MaybeError Texture::InitializeAsExternalTexture(const TextureDescriptor* descriptor,
                                                    HANDLE sharedHandle,
                                                    ExternalMutexSerial acquireMutexKey,
                                                    bool isSwapChainTexture) {
        Device* dawnDevice = ToBackend(GetDevice());
        DAWN_TRY(ValidateTextureDescriptor(dawnDevice, descriptor));
        DAWN_TRY(ValidateTextureDescriptorCanBeWrapped(descriptor));

        ComPtr<ID3D12Resource> d3d12Resource;
        DAWN_TRY(CheckHRESULT(dawnDevice->GetD3D12Device()->OpenSharedHandle(
                                  sharedHandle, IID_PPV_ARGS(&d3d12Resource)),
                              "D3D12 opening shared handle"));

        DAWN_TRY(ValidateD3D12TextureCanBeWrapped(d3d12Resource.Get(), descriptor));

        ComPtr<IDXGIKeyedMutex> dxgiKeyedMutex;
        DAWN_TRY_ASSIGN(dxgiKeyedMutex,
                        dawnDevice->CreateKeyedMutexForTexture(d3d12Resource.Get()));

        DAWN_TRY(CheckHRESULT(dxgiKeyedMutex->AcquireSync(uint64_t(acquireMutexKey), INFINITE),
                              "D3D12 acquiring shared mutex"));

        mAcquireMutexKey = acquireMutexKey;
        mDxgiKeyedMutex = std::move(dxgiKeyedMutex);
        mSwapChainTexture = isSwapChainTexture;

        AllocationInfo info;
        info.mMethod = AllocationMethod::kExternal;
        // When creating the ResourceHeapAllocation, the resource heap is set to nullptr because the
        // texture is owned externally. The texture's owning entity must remain responsible for
        // memory management.
        mResourceAllocation = {info, 0, std::move(d3d12Resource), nullptr};

        return {};
    }

    MaybeError Texture::InitializeAsInternalTexture() {
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12TextureDimension(GetDimension());
        resourceDescriptor.Alignment = 0;

        const Extent3D& size = GetSize();
        resourceDescriptor.Width = size.width;
        resourceDescriptor.Height = size.height;
        resourceDescriptor.DepthOrArraySize = size.depth;

        // This will need to be much more nuanced when WebGPU has
        // texture view compatibility rules.
        const bool needsTypelessFormat =
            GetFormat().HasDepthOrStencil() && (GetUsage() & wgpu::TextureUsage::Sampled) != 0;

        DXGI_FORMAT dxgiFormat = needsTypelessFormat
                                     ? D3D12TypelessTextureFormat(GetFormat().format)
                                     : D3D12TextureFormat(GetFormat().format);

        resourceDescriptor.MipLevels = static_cast<UINT16>(GetNumMipLevels());
        resourceDescriptor.Format = dxgiFormat;
        resourceDescriptor.SampleDesc.Count = GetSampleCount();
        // TODO(bryan.bernhart@intel.com): investigate how to specify standard MSAA sample pattern.
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resourceDescriptor.Flags =
            D3D12ResourceFlags(GetUsage(), GetFormat(), IsMultisampledTexture());

        DAWN_TRY_ASSIGN(mResourceAllocation,
                        ToBackend(GetDevice())
                            ->AllocateMemory(D3D12_HEAP_TYPE_DEFAULT, resourceDescriptor,
                                             D3D12_RESOURCE_STATE_COMMON));

        Device* device = ToBackend(GetDevice());

        if (device->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            CommandRecordingContext* commandContext;
            DAWN_TRY_ASSIGN(commandContext, device->GetPendingCommandContext());

            DAWN_TRY(ClearTexture(commandContext, GetAllSubresources(),
                                  TextureBase::ClearValue::NonZero));
        }

        return {};
    }

    MaybeError Texture::InitializeAsSwapChainTexture(ComPtr<ID3D12Resource> d3d12Texture) {
        AllocationInfo info;
        info.mMethod = AllocationMethod::kExternal;
        // When creating the ResourceHeapAllocation, the resource heap is set to nullptr because the
        // texture is owned externally. The texture's owning entity must remain responsible for
        // memory management.
        mResourceAllocation = { info, 0, std::move(d3d12Texture), nullptr };

        SetIsSubresourceContentInitialized(true, GetAllSubresources());

        return {};
    }

    Texture::Texture(Device* device, const TextureDescriptor* descriptor, TextureState state)
        : TextureBase(device, descriptor, state),
          mSubresourceStateAndDecay(
              GetFormat().aspects,
              GetArrayLayers(),
              GetNumMipLevels(),
              {D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, kMaxExecutionSerial, false}) {
    }

    Texture::~Texture() {
        DestroyInternal();
    }

    void Texture::DestroyImpl() {
        Device* device = ToBackend(GetDevice());

        // In PIX's D3D12-only mode, there is no way to determine frame boundaries
        // for WebGPU since Dawn does not manage DXGI swap chains. Without assistance,
        // PIX will wait forever for a present that never happens.
        // If we know we're dealing with a swapbuffer texture, inform PIX we've
        // "presented" the texture so it can determine frame boundaries and use its
        // contents for the UI.
        if (mSwapChainTexture) {
            ID3D12SharingContract* d3dSharingContract = device->GetSharingContract();
            if (d3dSharingContract != nullptr) {
                d3dSharingContract->Present(mResourceAllocation.GetD3D12Resource(), 0, 0);
            }
        }

        device->DeallocateMemory(mResourceAllocation);

        if (mDxgiKeyedMutex != nullptr) {
            mDxgiKeyedMutex->ReleaseSync(uint64_t(mAcquireMutexKey) + 1);
            device->ReleaseKeyedMutexForTexture(std::move(mDxgiKeyedMutex));
        }
    }

    DXGI_FORMAT Texture::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat().format);
    }

    ID3D12Resource* Texture::GetD3D12Resource() const {
        return mResourceAllocation.GetD3D12Resource();
    }

    DXGI_FORMAT Texture::GetD3D12CopyableSubresourceFormat(Aspect aspect) const {
        ASSERT(GetFormat().aspects & aspect);

        switch (GetFormat().format) {
            case wgpu::TextureFormat::Depth24PlusStencil8:
                switch (aspect) {
                    case Aspect::Depth:
                        return DXGI_FORMAT_R32_FLOAT;
                    case Aspect::Stencil:
                        return DXGI_FORMAT_R8_UINT;
                    default:
                        UNREACHABLE();
                }
            default:
                ASSERT(HasOneBit(GetFormat().aspects));
                return GetD3D12Format();
        }
    }

    void Texture::TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                             wgpu::TextureUsage usage,
                                             const SubresourceRange& range) {
        TrackUsageAndTransitionNow(commandContext, D3D12TextureUsage(usage, GetFormat()), range);
    }

    void Texture::TrackAllUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                                wgpu::TextureUsage usage) {
        TrackUsageAndTransitionNow(commandContext, D3D12TextureUsage(usage, GetFormat()),
                                   GetAllSubresources());
    }

    void Texture::TrackAllUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                                D3D12_RESOURCE_STATES newState) {
        TrackUsageAndTransitionNow(commandContext, newState, GetAllSubresources());
    }

    void Texture::TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                             D3D12_RESOURCE_STATES newState,
                                             const SubresourceRange& range) {
        if (mResourceAllocation.GetInfo().mMethod != AllocationMethod::kExternal) {
            // Track the underlying heap to ensure residency.
            Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
            commandContext->TrackHeapUsage(heap, GetDevice()->GetPendingCommandSerial());
        }

        std::vector<D3D12_RESOURCE_BARRIER> barriers;

        // TODO(enga): Consider adding a Count helper.
        uint32_t aspectCount = 0;
        for (Aspect aspect : IterateEnumMask(range.aspects)) {
            aspectCount++;
            DAWN_UNUSED(aspect);
        }

        barriers.reserve(range.levelCount * range.layerCount * aspectCount);

        TransitionUsageAndGetResourceBarrier(commandContext, &barriers, newState, range);
        if (barriers.size()) {
            commandContext->GetCommandList()->ResourceBarrier(barriers.size(), barriers.data());
        }
    }

    void Texture::TransitionSubresourceRange(std::vector<D3D12_RESOURCE_BARRIER>* barriers,
                                             const SubresourceRange& range,
                                             StateAndDecay* state,
                                             D3D12_RESOURCE_STATES newState,
                                             ExecutionSerial pendingCommandSerial) const {
        // Reuse the subresource(s) directly and avoid transition when it isn't needed, and
        // return false.
        // TODO(cwallez@chromium.org): Need some form of UAV barriers at some point.
        if (state->lastState == newState) {
            return;
        }

        D3D12_RESOURCE_STATES lastState = state->lastState;

        // The COMMON state represents a state where no write operations can be pending, and
        // where all pixels are uncompressed. This makes it possible to transition to and
        // from some states without synchronization (i.e. without an explicit
        // ResourceBarrier call). Textures can be implicitly promoted to 1) a single write
        // state, or 2) multiple read states. Textures will implicitly decay to the COMMON
        // state when all of the following are true: 1) the texture is accessed on a command
        // list, 2) the ExecuteCommandLists call that uses that command list has ended, and
        // 3) the texture was promoted implicitly to a read-only state and is still in that
        // state.
        // https://docs.microsoft.com/en-us/windows/desktop/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions

        // To track implicit decays, we must record the pending serial on which that
        // transition will occur. When that texture is used again, the previously recorded
        // serial must be compared to the last completed serial to determine if the texture
        // has implicity decayed to the common state.
        if (state->isValidToDecay && pendingCommandSerial > state->lastDecaySerial) {
            lastState = D3D12_RESOURCE_STATE_COMMON;
        }

        // Update the tracked state.
        state->lastState = newState;

        // Destination states that qualify for an implicit promotion for a
        // non-simultaneous-access texture: NON_PIXEL_SHADER_RESOURCE,
        // PIXEL_SHADER_RESOURCE, COPY_SRC, COPY_DEST.
        {
            static constexpr D3D12_RESOURCE_STATES kD3D12PromotableReadOnlyStates =
                D3D12_RESOURCE_STATE_COPY_SOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

            if (lastState == D3D12_RESOURCE_STATE_COMMON) {
                if (IsSubset(newState, kD3D12PromotableReadOnlyStates)) {
                    // Implicit texture state decays can only occur when the texture was implicitly
                    // transitioned to a read-only state. isValidToDecay is needed to differentiate
                    // between resources that were implictly or explicitly transitioned to a
                    // read-only state.
                    state->isValidToDecay = true;
                    state->lastDecaySerial = pendingCommandSerial;
                    return;
                } else if (newState == D3D12_RESOURCE_STATE_COPY_DEST) {
                    state->isValidToDecay = false;
                    return;
                }
            }
        }

        D3D12_RESOURCE_BARRIER barrier;
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = GetD3D12Resource();
        barrier.Transition.StateBefore = lastState;
        barrier.Transition.StateAfter = newState;

        bool isFullRange = range.baseArrayLayer == 0 && range.baseMipLevel == 0 &&
                           range.layerCount == GetArrayLayers() &&
                           range.levelCount == GetNumMipLevels() &&
                           range.aspects == GetFormat().aspects;

        // Use a single transition for all subresources if possible.
        if (isFullRange) {
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barriers->push_back(barrier);
        } else {
            for (Aspect aspect : IterateEnumMask(range.aspects)) {
                for (uint32_t arrayLayer = 0; arrayLayer < range.layerCount; ++arrayLayer) {
                    for (uint32_t mipLevel = 0; mipLevel < range.levelCount; ++mipLevel) {
                        barrier.Transition.Subresource =
                            GetSubresourceIndex(range.baseMipLevel + mipLevel,
                                                range.baseArrayLayer + arrayLayer, aspect);
                        barriers->push_back(barrier);
                    }
                }
            }
        }

        state->isValidToDecay = false;
    }

    void Texture::HandleTransitionSpecialCases(CommandRecordingContext* commandContext) {
        // Textures with keyed mutexes can be written from other graphics queues. Hence, they
        // must be acquired before command list submission to ensure work from the other queues
        // has finished. See Device::ExecuteCommandContext.
        if (mDxgiKeyedMutex != nullptr) {
            commandContext->AddToSharedTextureList(this);
        }
    }

    void Texture::TransitionUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                                       std::vector<D3D12_RESOURCE_BARRIER>* barrier,
                                                       wgpu::TextureUsage usage,
                                                       const SubresourceRange& range) {
        TransitionUsageAndGetResourceBarrier(commandContext, barrier,
                                             D3D12TextureUsage(usage, GetFormat()), range);
    }

    void Texture::TransitionUsageAndGetResourceBarrier(
        CommandRecordingContext* commandContext,
        std::vector<D3D12_RESOURCE_BARRIER>* barriers,
        D3D12_RESOURCE_STATES newState,
        const SubresourceRange& range) {
        HandleTransitionSpecialCases(commandContext);

        const ExecutionSerial pendingCommandSerial =
            ToBackend(GetDevice())->GetPendingCommandSerial();

        // This transitions assume it is a 2D texture
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);

        mSubresourceStateAndDecay.Update(
            range, [&](const SubresourceRange& updateRange, StateAndDecay* state) {
                TransitionSubresourceRange(barriers, updateRange, state, newState,
                                           pendingCommandSerial);
            });
    }

    void Texture::TrackUsageAndGetResourceBarrierForPass(
        CommandRecordingContext* commandContext,
        std::vector<D3D12_RESOURCE_BARRIER>* barriers,
        const PassTextureUsage& textureUsages) {
        if (mResourceAllocation.GetInfo().mMethod != AllocationMethod::kExternal) {
            // Track the underlying heap to ensure residency.
            Heap* heap = ToBackend(mResourceAllocation.GetResourceHeap());
            commandContext->TrackHeapUsage(heap, GetDevice()->GetPendingCommandSerial());
        }

        HandleTransitionSpecialCases(commandContext);

        const ExecutionSerial pendingCommandSerial =
            ToBackend(GetDevice())->GetPendingCommandSerial();
        // This transitions assume it is a 2D texture
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);

        mSubresourceStateAndDecay.Merge(
            textureUsages, [&](const SubresourceRange& mergeRange, StateAndDecay* state,
                               wgpu::TextureUsage usage) {
                // Skip if this subresource is not used during the current pass
                if (usage == wgpu::TextureUsage::None) {
                    return;
                }

                D3D12_RESOURCE_STATES newState = D3D12TextureUsage(usage, GetFormat());
                TransitionSubresourceRange(barriers, mergeRange, state, newState,
                                           pendingCommandSerial);
            });
    }

    D3D12_RENDER_TARGET_VIEW_DESC Texture::GetRTVDescriptor(uint32_t mipLevel,
                                                            uint32_t baseArrayLayer,
                                                            uint32_t layerCount) const {
        ASSERT(GetDimension() == wgpu::TextureDimension::e2D);
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = GetD3D12Format();
        if (IsMultisampledTexture()) {
            ASSERT(GetNumMipLevels() == 1);
            ASSERT(layerCount == 1);
            ASSERT(baseArrayLayer == 0);
            ASSERT(mipLevel == 0);
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else {
            // Currently we always use D3D12_TEX2D_ARRAY_RTV because we cannot specify base array
            // layer and layer count in D3D12_TEX2D_RTV. For 2D texture views, we treat them as
            // 1-layer 2D array textures. (Just like how we treat SRVs)
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_rtv
            // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_array
            // _rtv
            rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.FirstArraySlice = baseArrayLayer;
            rtvDesc.Texture2DArray.ArraySize = layerCount;
            rtvDesc.Texture2DArray.MipSlice = mipLevel;
            rtvDesc.Texture2DArray.PlaneSlice = 0;
        }
        return rtvDesc;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC Texture::GetDSVDescriptor(uint32_t mipLevel,
                                                            uint32_t baseArrayLayer,
                                                            uint32_t layerCount) const {
        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = GetD3D12Format();
        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

        if (IsMultisampledTexture()) {
            ASSERT(GetNumMipLevels() == 1);
            ASSERT(layerCount == 1);
            ASSERT(baseArrayLayer == 0);
            ASSERT(mipLevel == 0);
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.FirstArraySlice = baseArrayLayer;
            dsvDesc.Texture2DArray.ArraySize = layerCount;
            dsvDesc.Texture2DArray.MipSlice = mipLevel;
        }

        return dsvDesc;
    }

    MaybeError Texture::ClearTexture(CommandRecordingContext* commandContext,
                                     const SubresourceRange& range,
                                     TextureBase::ClearValue clearValue) {

        ID3D12GraphicsCommandList* commandList = commandContext->GetCommandList();

        Device* device = ToBackend(GetDevice());

        uint8_t clearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0 : 1;
        float fClearColor = (clearValue == TextureBase::ClearValue::Zero) ? 0.f : 1.f;

        if ((GetUsage() & wgpu::TextureUsage::RenderAttachment) != 0) {
            if (GetFormat().HasDepthOrStencil()) {
                TrackUsageAndTransitionNow(commandContext, D3D12_RESOURCE_STATE_DEPTH_WRITE, range);

                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    for (uint32_t layer = range.baseArrayLayer;
                         layer < range.baseArrayLayer + range.layerCount; ++layer) {
                        // Iterate the aspects individually to determine which clear flags to use.
                        D3D12_CLEAR_FLAGS clearFlags = {};
                        for (Aspect aspect : IterateEnumMask(range.aspects)) {
                            if (clearValue == TextureBase::ClearValue::Zero &&
                                IsSubresourceContentInitialized(
                                    SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                                // Skip lazy clears if already initialized.
                                continue;
                            }

                            switch (aspect) {
                                case Aspect::Depth:
                                    clearFlags |= D3D12_CLEAR_FLAG_DEPTH;
                                    break;
                                case Aspect::Stencil:
                                    clearFlags |= D3D12_CLEAR_FLAG_STENCIL;
                                    break;
                                default:
                                    UNREACHABLE();
                            }
                        }

                        if (clearFlags == 0) {
                            continue;
                        }

                        CPUDescriptorHeapAllocation dsvHandle;
                        DAWN_TRY_ASSIGN(dsvHandle, device->GetDepthStencilViewAllocator()
                                                       ->AllocateTransientCPUDescriptors());
                        const D3D12_CPU_DESCRIPTOR_HANDLE baseDescriptor =
                            dsvHandle.GetBaseDescriptor();
                        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = GetDSVDescriptor(level, layer, 1);
                        device->GetD3D12Device()->CreateDepthStencilView(GetD3D12Resource(),
                                                                         &dsvDesc, baseDescriptor);

                        commandList->ClearDepthStencilView(baseDescriptor, clearFlags, fClearColor,
                                                           clearColor, 0, nullptr);
                    }
                }
            } else {
                TrackUsageAndTransitionNow(commandContext, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                           range);

                const float clearColorRGBA[4] = {fClearColor, fClearColor, fClearColor,
                                                 fClearColor};

                ASSERT(range.aspects == Aspect::Color);
                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    for (uint32_t layer = range.baseArrayLayer;
                         layer < range.baseArrayLayer + range.layerCount; ++layer) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, Aspect::Color))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        CPUDescriptorHeapAllocation rtvHeap;
                        DAWN_TRY_ASSIGN(rtvHeap, device->GetRenderTargetViewAllocator()
                                                     ->AllocateTransientCPUDescriptors());
                        const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap.GetBaseDescriptor();

                        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = GetRTVDescriptor(level, layer, 1);
                        device->GetD3D12Device()->CreateRenderTargetView(GetD3D12Resource(),
                                                                         &rtvDesc, rtvHandle);
                        commandList->ClearRenderTargetView(rtvHandle, clearColorRGBA, 0, nullptr);
                    }
                }
            }
        } else {
            // create temp buffer with clear color to copy to the texture image
            TrackUsageAndTransitionNow(commandContext, D3D12_RESOURCE_STATE_COPY_DEST, range);

            for (Aspect aspect : IterateEnumMask(range.aspects)) {
                const TexelBlockInfo& blockInfo = GetFormat().GetAspectInfo(aspect).block;

                uint32_t bytesPerRow = Align((GetWidth() / blockInfo.width) * blockInfo.byteSize,
                                             kTextureBytesPerRowAlignment);
                uint64_t bufferSize = bytesPerRow * (GetHeight() / blockInfo.height);
                DynamicUploader* uploader = device->GetDynamicUploader();
                UploadHandle uploadHandle;
                DAWN_TRY_ASSIGN(uploadHandle,
                                uploader->Allocate(bufferSize, device->GetPendingCommandSerial(),
                                                   blockInfo.byteSize));
                memset(uploadHandle.mappedBuffer, clearColor, bufferSize);

                for (uint32_t level = range.baseMipLevel;
                     level < range.baseMipLevel + range.levelCount; ++level) {
                    // compute d3d12 texture copy locations for texture and buffer
                    Extent3D copySize = GetMipLevelPhysicalSize(level);

                    uint32_t rowsPerImage = GetHeight() / blockInfo.height;
                    Texture2DCopySplit copySplit = ComputeTextureCopySplit(
                        {0, 0, 0}, copySize, blockInfo, uploadHandle.startOffset, bytesPerRow,
                        rowsPerImage);

                    for (uint32_t layer = range.baseArrayLayer;
                         layer < range.baseArrayLayer + range.layerCount; ++layer) {
                        if (clearValue == TextureBase::ClearValue::Zero &&
                            IsSubresourceContentInitialized(
                                SubresourceRange::SingleMipAndLayer(level, layer, aspect))) {
                            // Skip lazy clears if already initialized.
                            continue;
                        }

                        D3D12_TEXTURE_COPY_LOCATION textureLocation =
                            ComputeTextureCopyLocationForTexture(this, level, layer, aspect);
                        for (uint32_t i = 0; i < copySplit.count; ++i) {
                            Texture2DCopySplit::CopyInfo& info = copySplit.copies[i];

                            D3D12_TEXTURE_COPY_LOCATION bufferLocation =
                                ComputeBufferLocationForCopyTextureRegion(
                                    this, ToBackend(uploadHandle.stagingBuffer)->GetResource(),
                                    info.bufferSize, copySplit.offset, bytesPerRow, aspect);
                            D3D12_BOX sourceRegion =
                                ComputeD3D12BoxFromOffsetAndSize(info.bufferOffset, info.copySize);

                            // copy the buffer filled with clear color to the texture
                            commandList->CopyTextureRegion(
                                &textureLocation, info.textureOffset.x, info.textureOffset.y,
                                info.textureOffset.z, &bufferLocation, &sourceRegion);
                        }
                    }
                }
            }
        }
        if (clearValue == TextureBase::ClearValue::Zero) {
            SetIsSubresourceContentInitialized(true, range);
            GetDevice()->IncrementLazyClearCountForTesting();
        }
        return {};
    }

    void Texture::EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                      const SubresourceRange& range) {
        if (!ToBackend(GetDevice())->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            return;
        }
        if (!IsSubresourceContentInitialized(range)) {
            // If subresource has not been initialized, clear it to black as it could contain
            // dirty bits from recycled memory
            GetDevice()->ConsumedError(
                ClearTexture(commandContext, range, TextureBase::ClearValue::Zero));
        }
    }

    bool Texture::StateAndDecay::operator==(const Texture::StateAndDecay& other) const {
        return lastState == other.lastState && lastDecaySerial == other.lastDecaySerial &&
               isValidToDecay == other.isValidToDecay;
    }

    TextureView::TextureView(TextureBase* texture, const TextureViewDescriptor* descriptor)
        : TextureViewBase(texture, descriptor) {
        mSrvDesc.Format = D3D12TextureFormat(descriptor->format);
        mSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        // TODO(enga): This will need to be much more nuanced when WebGPU has
        // texture view compatibility rules.
        UINT planeSlice = 0;
        if (GetFormat().HasDepthOrStencil()) {
            // Configure the SRV descriptor to reinterpret the texture allocated as
            // TYPELESS as a single-plane shader-accessible view.
            switch (descriptor->format) {
                case wgpu::TextureFormat::Depth32Float:
                case wgpu::TextureFormat::Depth24Plus:
                    mSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
                    break;
                case wgpu::TextureFormat::Depth24PlusStencil8:
                    switch (descriptor->aspect) {
                        case wgpu::TextureAspect::DepthOnly:
                            planeSlice = 0;
                            mSrvDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
                            break;
                        case wgpu::TextureAspect::StencilOnly:
                            planeSlice = 1;
                            mSrvDesc.Format = DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
                            // Stencil is accessed using the .g component in the shader.
                            // Map it to the zeroth component to match other APIs.
                            mSrvDesc.Shader4ComponentMapping =
                                D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
                                    D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
                                    D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0,
                                    D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0,
                                    D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1);
                            break;
                        case wgpu::TextureAspect::All:
                            // A single aspect is not selected. The texture view must not be
                            // sampled.
                            mSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
                            break;
                    }
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
        }

        // Currently we always use D3D12_TEX2D_ARRAY_SRV because we cannot specify base array layer
        // and layer count in D3D12_TEX2D_SRV. For 2D texture views, we treat them as 1-layer 2D
        // array textures.
        // Multisampled textures may only be one array layer, so we use
        // D3D12_SRV_DIMENSION_TEXTURE2DMS.
        // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_srv
        // https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ns-d3d12-d3d12_tex2d_array_srv
        // TODO(jiawei.shao@intel.com): support more texture view dimensions.
        if (GetTexture()->IsMultisampledTexture()) {
            switch (descriptor->dimension) {
                case wgpu::TextureViewDimension::e2DArray:
                    ASSERT(texture->GetArrayLayers() == 1);
                    DAWN_FALLTHROUGH;
                case wgpu::TextureViewDimension::e2D:
                    ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
                    mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
                    break;

                default:
                    UNREACHABLE();
            }
        } else {
            switch (descriptor->dimension) {
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e2DArray:
                    ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
                    mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                    mSrvDesc.Texture2DArray.ArraySize = descriptor->arrayLayerCount;
                    mSrvDesc.Texture2DArray.FirstArraySlice = descriptor->baseArrayLayer;
                    mSrvDesc.Texture2DArray.MipLevels = descriptor->mipLevelCount;
                    mSrvDesc.Texture2DArray.MostDetailedMip = descriptor->baseMipLevel;
                    mSrvDesc.Texture2DArray.PlaneSlice = planeSlice;
                    mSrvDesc.Texture2DArray.ResourceMinLODClamp = 0;
                    break;
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    ASSERT(texture->GetDimension() == wgpu::TextureDimension::e2D);
                    ASSERT(descriptor->arrayLayerCount % 6 == 0);
                    mSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
                    mSrvDesc.TextureCubeArray.First2DArrayFace = descriptor->baseArrayLayer;
                    mSrvDesc.TextureCubeArray.NumCubes = descriptor->arrayLayerCount / 6;
                    mSrvDesc.TextureCubeArray.MostDetailedMip = descriptor->baseMipLevel;
                    mSrvDesc.TextureCubeArray.MipLevels = descriptor->mipLevelCount;
                    mSrvDesc.TextureCubeArray.ResourceMinLODClamp = 0;
                    break;

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e3D:
                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
            }
        }
    }

    DXGI_FORMAT TextureView::GetD3D12Format() const {
        return D3D12TextureFormat(GetFormat().format);
    }

    const D3D12_SHADER_RESOURCE_VIEW_DESC& TextureView::GetSRVDescriptor() const {
        ASSERT(mSrvDesc.Format != DXGI_FORMAT_UNKNOWN);
        return mSrvDesc;
    }

    D3D12_RENDER_TARGET_VIEW_DESC TextureView::GetRTVDescriptor() const {
        return ToBackend(GetTexture())
            ->GetRTVDescriptor(GetBaseMipLevel(), GetBaseArrayLayer(), GetLayerCount());
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC TextureView::GetDSVDescriptor() const {
        ASSERT(GetLevelCount() == 1);
        return ToBackend(GetTexture())
            ->GetDSVDescriptor(GetBaseMipLevel(), GetBaseArrayLayer(), GetLayerCount());
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC TextureView::GetUAVDescriptor() const {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = GetD3D12Format();

        ASSERT(!GetTexture()->IsMultisampledTexture());
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        uavDesc.Texture2DArray.FirstArraySlice = GetBaseArrayLayer();
        uavDesc.Texture2DArray.ArraySize = GetLayerCount();
        uavDesc.Texture2DArray.MipSlice = GetBaseMipLevel();
        uavDesc.Texture2DArray.PlaneSlice = 0;
        return uavDesc;
    }

}}  // namespace dawn_native::d3d12
