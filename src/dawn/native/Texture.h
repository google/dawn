// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_TEXTURE_H_
#define SRC_DAWN_NATIVE_TEXTURE_H_

#include <vector>

#include "dawn/common/WeakRef.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/SharedTextureMemory.h"
#include "dawn/native/Subresource.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

enum class AllowMultiPlanarTextureFormat {
    No,
    Yes,
};

MaybeError ValidateTextureDescriptor(
    const DeviceBase* device,
    const TextureDescriptor* descriptor,
    AllowMultiPlanarTextureFormat allowMultiPlanar = AllowMultiPlanarTextureFormat::No,
    std::optional<wgpu::TextureUsage> allowedSharedTextureMemoryUsage = std::nullopt);
MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                         const TextureBase* texture,
                                         const TextureViewDescriptor* descriptor);
ResultOrError<TextureViewDescriptor> GetTextureViewDescriptorWithDefaults(
    const TextureBase* texture,
    const TextureViewDescriptor* descriptor);

bool IsValidSampleCount(uint32_t sampleCount);

static constexpr wgpu::TextureUsage kReadOnlyTextureUsages =
    wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding | kReadOnlyRenderAttachment |
    kReadOnlyStorageTexture;

// Valid texture usages for a resolve texture that are loaded from at the beginning of a render
// pass.
static constexpr wgpu::TextureUsage kResolveTextureLoadAndStoreUsages =
    kResolveAttachmentLoadingUsage | wgpu::TextureUsage::RenderAttachment;

class TextureBase : public ApiObjectBase {
  public:
    enum class ClearValue { Zero, NonZero };

    static TextureBase* MakeError(DeviceBase* device, const TextureDescriptor* descriptor);

    ObjectType GetType() const override;

    wgpu::TextureDimension GetDimension() const;
    const Format& GetFormat() const;
    const FormatSet& GetViewFormats() const;

    // For multiplanar textures, base size is the size of plane 0. For other types of textures,
    // base size is the original size passed via TextureDescriptor.
    const Extent3D& GetBaseSize() const;
    Extent3D GetSize(Aspect aspect) const;
    Extent3D GetSize(wgpu::TextureAspect aspect) const;
    uint32_t GetWidth(Aspect aspect) const;
    uint32_t GetHeight(Aspect aspect) const;
    uint32_t GetDepth(Aspect aspect) const;
    uint32_t GetArrayLayers() const;
    uint32_t GetNumMipLevels() const;
    SubresourceRange GetAllSubresources() const;
    uint32_t GetSampleCount() const;
    uint32_t GetSubresourceCount() const;

    // |GetUsage| returns the usage with which the texture was created using the base WebGPU
    // API. The dawn-internal-usages extension may add additional usages. |GetInternalUsage|
    // returns the union of base usage and the usages added by the extension.
    wgpu::TextureUsage GetUsage() const;
    wgpu::TextureUsage GetInternalUsage() const;

    bool IsDestroyed() const;
    void SetHasAccess(bool hasAccess);
    uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arraySlice, Aspect aspect) const;
    bool IsSubresourceContentInitialized(const SubresourceRange& range) const;
    void SetIsSubresourceContentInitialized(bool isInitialized, const SubresourceRange& range);

    MaybeError ValidateCanUseInSubmitNow() const;

    bool IsMultisampledTexture() const;

    // Returns true if the size covers the whole subresource.
    bool CoversFullSubresource(uint32_t mipLevel, Aspect aspect, const Extent3D& size) const;

    // For a texture with non-block-compressed texture format, its physical size is always equal
    // to its virtual size. For a texture with block compressed texture format, the physical
    // size is the one with paddings if necessary, which is always a multiple of the block size
    // and used in texture copying. The virtual size is the one without paddings, which is not
    // required to be a multiple of the block size and used in texture sampling.
    Extent3D GetMipLevelSingleSubresourcePhysicalSize(uint32_t level, Aspect aspect) const;
    Extent3D GetMipLevelSingleSubresourceVirtualSize(uint32_t level, Aspect aspect) const;
    Extent3D ClampToMipLevelVirtualSize(uint32_t level,
                                        Aspect aspect,
                                        const Origin3D& origin,
                                        const Extent3D& extent) const;
    // For 2d-array textures, this keeps the array layers in contrast to
    // GetMipLevelSingleSubresourceVirtualSize.
    Extent3D GetMipLevelSubresourceVirtualSize(uint32_t level, Aspect aspect) const;

    ResultOrError<Ref<TextureViewBase>> CreateView(
        const TextureViewDescriptor* descriptor = nullptr);
    ApiObjectList* GetViewTrackingList();

    bool IsImplicitMSAARenderTextureViewSupported() const;

    SharedTextureMemoryContents* GetSharedTextureMemoryContents() const;

    // Dawn API
    TextureViewBase* APICreateView(const TextureViewDescriptor* descriptor = nullptr);
    void APIDestroy();
    uint32_t APIGetWidth() const;
    uint32_t APIGetHeight() const;
    uint32_t APIGetDepthOrArrayLayers() const;
    uint32_t APIGetMipLevelCount() const;
    uint32_t APIGetSampleCount() const;
    wgpu::TextureDimension APIGetDimension() const;
    wgpu::TextureFormat APIGetFormat() const;
    wgpu::TextureUsage APIGetUsage() const;

  protected:
    TextureBase(DeviceBase* device, const TextureDescriptor* descriptor);
    ~TextureBase() override;

    void DestroyImpl() override;
    void AddInternalUsage(wgpu::TextureUsage usage);

    // The shared texture memory state the texture was created from. May be null.
    Ref<SharedTextureMemoryContents> mSharedTextureMemoryContents;

  private:
    struct TextureState {
        TextureState();

        // Indicates whether the texture may access by the GPU in a queue submit.
        bool hasAccess : 1;
        // Indicates whether the texture has been destroyed.
        bool destroyed : 1;
    };

    TextureBase(DeviceBase* device, const TextureDescriptor* descriptor, ObjectBase::ErrorTag tag);

    wgpu::TextureDimension mDimension;
    const Format& mFormat;
    FormatSet mViewFormats;
    Extent3D mBaseSize;
    uint32_t mMipLevelCount;
    uint32_t mSampleCount;
    wgpu::TextureUsage mUsage = wgpu::TextureUsage::None;
    wgpu::TextureUsage mInternalUsage = wgpu::TextureUsage::None;
    TextureState mState;
    wgpu::TextureFormat mFormatEnumForReflection;

    // Textures track texture views created from them so that they can be destroyed when the texture
    // is destroyed.
    ApiObjectList mTextureViews;

    // TODO(crbug.com/dawn/845): Use a more optimized data structure to save space
    std::vector<bool> mIsSubresourceContentInitializedAtIndex;
};

class TextureViewBase : public ApiObjectBase {
  public:
    TextureViewBase(TextureBase* texture, const TextureViewDescriptor* descriptor);
    ~TextureViewBase() override;

    static TextureViewBase* MakeError(DeviceBase* device, const char* label = nullptr);

    ObjectType GetType() const override;
    void FormatLabel(absl::FormatSink* s) const override;

    const TextureBase* GetTexture() const;
    TextureBase* GetTexture();

    Aspect GetAspects() const;
    const Format& GetFormat() const;
    wgpu::TextureViewDimension GetDimension() const;
    uint32_t GetBaseMipLevel() const;
    uint32_t GetLevelCount() const;
    uint32_t GetBaseArrayLayer() const;
    uint32_t GetLayerCount() const;
    const SubresourceRange& GetSubresourceRange() const;

    // Returns the size of the texture's subresource at this view's base mip level and aspect.
    Extent3D GetSingleSubresourceVirtualSize() const;

  protected:
    void DestroyImpl() override;

  private:
    TextureViewBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label);

    ApiObjectList* GetObjectTrackingList() override;

    Ref<TextureBase> mTexture;

    const Format& mFormat;
    wgpu::TextureViewDimension mDimension;
    SubresourceRange mRange;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TEXTURE_H_
