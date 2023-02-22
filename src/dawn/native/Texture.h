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

#ifndef SRC_DAWN_NATIVE_TEXTURE_H_
#define SRC_DAWN_NATIVE_TEXTURE_H_

#include <vector>

#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/Error.h"
#include "dawn/native/Format.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/Subresource.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateTextureDescriptor(const DeviceBase* device, const TextureDescriptor* descriptor);
MaybeError ValidateTextureViewDescriptor(const DeviceBase* device,
                                         const TextureBase* texture,
                                         const TextureViewDescriptor* descriptor);
ResultOrError<TextureViewDescriptor> GetTextureViewDescriptorWithDefaults(
    const TextureBase* texture,
    const TextureViewDescriptor* descriptor);

bool IsValidSampleCount(uint32_t sampleCount);

static constexpr wgpu::TextureUsage kReadOnlyTextureUsages =
    wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding | kReadOnlyRenderAttachment;

class TextureBase : public ApiObjectBase {
  public:
    enum class TextureState { OwnedInternal, OwnedExternal, Destroyed };
    enum class ClearValue { Zero, NonZero };

    static TextureBase* MakeError(DeviceBase* device, const TextureDescriptor* descriptor);

    ObjectType GetType() const override;

    wgpu::TextureDimension GetDimension() const;
    const Format& GetFormat() const;
    const FormatSet& GetViewFormats() const;
    const Extent3D& GetSize() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    uint32_t GetDepth() const;
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

    TextureState GetTextureState() const;
    uint32_t GetSubresourceIndex(uint32_t mipLevel, uint32_t arraySlice, Aspect aspect) const;
    bool IsSubresourceContentInitialized(const SubresourceRange& range) const;
    void SetIsSubresourceContentInitialized(bool isInitialized, const SubresourceRange& range);

    MaybeError ValidateCanUseInSubmitNow() const;

    bool IsMultisampledTexture() const;

    // For a texture with non-block-compressed texture format, its physical size is always equal
    // to its virtual size. For a texture with block compressed texture format, the physical
    // size is the one with paddings if necessary, which is always a multiple of the block size
    // and used in texture copying. The virtual size is the one without paddings, which is not
    // required to be a multiple of the block size and used in texture sampling.
    Extent3D GetMipLevelSingleSubresourcePhysicalSize(uint32_t level) const;
    Extent3D GetMipLevelSingleSubresourceVirtualSize(uint32_t level) const;
    Extent3D ClampToMipLevelVirtualSize(uint32_t level,
                                        const Origin3D& origin,
                                        const Extent3D& extent) const;

    ResultOrError<Ref<TextureViewBase>> CreateView(
        const TextureViewDescriptor* descriptor = nullptr);
    ApiObjectList* GetViewTrackingList();

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
    TextureBase(DeviceBase* device, const TextureDescriptor* descriptor, TextureState state);
    ~TextureBase() override;

    void DestroyImpl() override;
    void AddInternalUsage(wgpu::TextureUsage usage);

  private:
    TextureBase(DeviceBase* device, const TextureDescriptor* descriptor, ObjectBase::ErrorTag tag);

    wgpu::TextureDimension mDimension;
    const Format& mFormat;
    FormatSet mViewFormats;
    Extent3D mSize;
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

    static TextureViewBase* MakeError(DeviceBase* device);

    ObjectType GetType() const override;

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

  protected:
    void DestroyImpl() override;

  private:
    TextureViewBase(DeviceBase* device, ObjectBase::ErrorTag tag);

    ApiObjectList* GetObjectTrackingList() override;

    Ref<TextureBase> mTexture;

    const Format& mFormat;
    wgpu::TextureViewDimension mDimension;
    SubresourceRange mRange;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_TEXTURE_H_
