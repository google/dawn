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

#include "dawn/native/SwapChain.h"

#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

class ErrorSwapChain final : public SwapChainBase {
  public:
    explicit ErrorSwapChain(DeviceBase* device, const SurfaceConfiguration* config)
        : SwapChainBase(device, config, ObjectBase::kError) {}

  private:
    ResultOrError<SwapChainTextureInfo> GetCurrentTextureImpl() override { DAWN_UNREACHABLE(); }
    MaybeError PresentImpl() override { DAWN_UNREACHABLE(); }
    void DetachFromSurfaceImpl() override { DAWN_UNREACHABLE(); }
};

}  // anonymous namespace

MaybeError ValidateSwapChainDescriptor(const DeviceBase* device,
                                       const Surface* surface,
                                       const SwapChainDescriptor* descriptor) {
    DAWN_INVALID_IF(surface->IsError(), "[Surface] is invalid.");

    DAWN_TRY(ValidatePresentMode(descriptor->presentMode));

// TODO(crbug.com/dawn/160): Lift this restriction once wgpu::Instance::GetPreferredSurfaceFormat is
// implemented.
// TODO(dawn:286):
#if DAWN_PLATFORM_IS(ANDROID)
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::RGBA8Unorm;
#else
    constexpr wgpu::TextureFormat kRequireSwapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
#endif  // !DAWN_PLATFORM_IS(ANDROID)
    DAWN_INVALID_IF(descriptor->format != kRequireSwapChainFormat,
                    "Format (%s) is not %s, which is (currently) the only accepted format.",
                    descriptor->format, kRequireSwapChainFormat);

    if (device->HasFeature(Feature::SurfaceCapabilities)) {
        wgpu::TextureUsage validUsage;
        DAWN_TRY_ASSIGN(validUsage, device->GetSupportedSurfaceUsage(surface));
        DAWN_INVALID_IF(
            (descriptor->usage | validUsage) != validUsage,
            "Usage (%s) is not supported, %s are (currently) the only accepted usage flags.",
            descriptor->usage, validUsage);
    } else {
        DAWN_INVALID_IF(descriptor->usage != wgpu::TextureUsage::RenderAttachment,
                        "Usage (%s) is not %s, which is (currently) the only accepted usage. Other "
                        "usage flags require enabling %s",
                        descriptor->usage, wgpu::TextureUsage::RenderAttachment,
                        wgpu::FeatureName::SurfaceCapabilities);
    }

    DAWN_INVALID_IF(descriptor->width == 0 || descriptor->height == 0,
                    "Swap Chain size (width: %u, height: %u) is empty.", descriptor->width,
                    descriptor->height);

    DAWN_INVALID_IF(
        descriptor->width > device->GetLimits().v1.maxTextureDimension2D ||
            descriptor->height > device->GetLimits().v1.maxTextureDimension2D,
        "Swap Chain size (width: %u, height: %u) is greater than the maximum 2D texture "
        "size (width: %u, height: %u).",
        descriptor->width, descriptor->height, device->GetLimits().v1.maxTextureDimension2D,
        device->GetLimits().v1.maxTextureDimension2D);

    return {};
}

TextureDescriptor GetSwapChainBaseTextureDescriptor(SwapChainBase* swapChain) {
    TextureDescriptor desc;
    desc.usage = swapChain->GetUsage();
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {swapChain->GetWidth(), swapChain->GetHeight(), 1};
    desc.format = swapChain->GetFormat();
    desc.viewFormatCount = swapChain->GetViewFormats().size();
    desc.viewFormats = swapChain->GetViewFormats().data();
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    return desc;
}

SwapChainBase::SwapChainBase(DeviceBase* device,
                             Surface* surface,
                             const SurfaceConfiguration* config)
    : ApiObjectBase(device, kLabelNotImplemented),
      mWidth(config->width),
      mHeight(config->height),
      mFormat(config->format),
      mUsage(config->usage),
      mPresentMode(config->presentMode),
      mAlphaMode(config->alphaMode),
      mSurface(surface) {
    GetObjectTrackingList()->Track(this);
    for (uint32_t i = 0; i < config->viewFormatCount; ++i) {
        if (config->viewFormats[i] == config->format) {
            // Skip our own format, like texture creations does.
            continue;
        }
        mViewFormats.push_back(config->viewFormats[i]);
    }
}

FormatSet SwapChainBase::ComputeViewFormatSet() const {
    FormatSet viewFormatSet;
    for (wgpu::TextureFormat format : mViewFormats) {
        viewFormatSet[GetDevice()->GetValidInternalFormat(format)] = true;
    }
    return viewFormatSet;
}

SwapChainBase::~SwapChainBase() {
    if (mCurrentTextureInfo.texture != nullptr) {
        DAWN_ASSERT(mCurrentTextureInfo.texture->IsDestroyed());
    }

    DAWN_ASSERT(!mAttached);
}

SwapChainBase::SwapChainBase(DeviceBase* device,
                             const SurfaceConfiguration* config,
                             ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag),
      mWidth(config->width),
      mHeight(config->height),
      mFormat(config->format),
      mUsage(config->usage),
      mPresentMode(config->presentMode),
      mAlphaMode(config->alphaMode) {}

// static
Ref<SwapChainBase> SwapChainBase::MakeError(DeviceBase* device,
                                            const SurfaceConfiguration* config) {
    return AcquireRef(new ErrorSwapChain(device, config));
}

void SwapChainBase::DestroyImpl() {}

ObjectType SwapChainBase::GetType() const {
    return ObjectType::SwapChain;
}

void SwapChainBase::DetachFromSurface() {
    if (mAttached) {
        DetachFromSurfaceImpl();
        mSurface = nullptr;
        mAttached = false;
    }
}

void SwapChainBase::SetIsAttached() {
    mAttached = true;
}

void SwapChainBase::APIConfigure(wgpu::TextureFormat format,
                                 wgpu::TextureUsage allowedUsage,
                                 uint32_t width,
                                 uint32_t height) {
    GetDevice()->HandleError(
        DAWN_VALIDATION_ERROR("Configure is invalid for surface-based swapchains."));
}

TextureBase* SwapChainBase::APIGetCurrentTexture() {
    SurfaceTexture result;
    if (GetDevice()->ConsumedError(GetCurrentTexture(), &result, "calling %s.GetCurrentTexture()",
                                   this)) {
        TextureDescriptor desc = GetSwapChainBaseTextureDescriptor(this);
        Ref<TextureBase> errorTexture = TextureBase::MakeError(GetDevice(), &desc);
        SetChildLabel(errorTexture.Get());
        result.texture = ReturnToAPI(std::move(errorTexture));
    }
    return result.texture;
}

TextureViewBase* SwapChainBase::APIGetCurrentTextureView() {
    Ref<TextureViewBase> result;
    if (GetDevice()->ConsumedError(GetCurrentTextureView(), &result,
                                   "calling %s.GetCurrentTextureView()", this)) {
        result = TextureViewBase::MakeError(GetDevice());
        SetChildLabel(result.Get());
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<SurfaceTexture> SwapChainBase::GetCurrentTexture() {
    DAWN_TRY(ValidateGetCurrentTexture());
    SurfaceTexture surfaceTexture;

    if (mCurrentTextureInfo.texture == nullptr) {
        DAWN_TRY_ASSIGN(mCurrentTextureInfo, GetCurrentTextureImpl());
        SetChildLabel(mCurrentTextureInfo.texture.Get());

        // Check that the return texture matches exactly what was given for this descriptor.
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetFormat().format == mFormat);
        DAWN_ASSERT(IsSubset(mUsage, mCurrentTextureInfo.texture->GetUsage()));
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetDimension() == wgpu::TextureDimension::e2D);
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetWidth(Aspect::Color) == mWidth);
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetHeight(Aspect::Color) == mHeight);
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetNumMipLevels() == 1);
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetArrayLayers() == 1);
        DAWN_ASSERT(mCurrentTextureInfo.texture->GetViewFormats() == ComputeViewFormatSet());
    }

    // Calling GetCurrentTexture always returns a new reference.
    surfaceTexture.texture = Ref<TextureBase>(mCurrentTextureInfo.texture).Detach();
    surfaceTexture.suboptimal = mCurrentTextureInfo.suboptimal;
    surfaceTexture.status = mCurrentTextureInfo.status;
    return surfaceTexture;
}

ResultOrError<Ref<TextureViewBase>> SwapChainBase::GetCurrentTextureView() {
    SurfaceTexture surfaceTexture;
    DAWN_TRY_ASSIGN(surfaceTexture, GetCurrentTexture());
    return surfaceTexture.texture->CreateView();
}

void SwapChainBase::APIPresent() {
    if (GetDevice()->ConsumedError(ValidatePresent())) {
        return;
    }

    if (GetDevice()->ConsumedError(PresentImpl())) {
        return;
    }

    DAWN_ASSERT(mCurrentTextureInfo.texture->IsDestroyed());
    mCurrentTextureInfo.texture = nullptr;
}

uint32_t SwapChainBase::GetWidth() const {
    return mWidth;
}

uint32_t SwapChainBase::GetHeight() const {
    return mHeight;
}

wgpu::TextureFormat SwapChainBase::GetFormat() const {
    return mFormat;
}

const std::vector<wgpu::TextureFormat>& SwapChainBase::GetViewFormats() const {
    return mViewFormats;
}

wgpu::TextureUsage SwapChainBase::GetUsage() const {
    return mUsage;
}

wgpu::PresentMode SwapChainBase::GetPresentMode() const {
    return mPresentMode;
}

wgpu::CompositeAlphaMode SwapChainBase::GetAlphaMode() const {
    return mAlphaMode;
}

Surface* SwapChainBase::GetSurface() const {
    return mSurface;
}

bool SwapChainBase::IsAttached() const {
    return mAttached;
}

wgpu::BackendType SwapChainBase::GetBackendType() const {
    return GetDevice()->GetPhysicalDevice()->GetBackendType();
}

MaybeError SwapChainBase::ValidatePresent() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call Present called on detached %s.", this);

    DAWN_INVALID_IF(mCurrentTextureInfo.texture == nullptr,
                    "GetCurrentTexture was not called on %s this frame prior to calling Present.",
                    this);

    return {};
}

MaybeError SwapChainBase::ValidateGetCurrentTexture() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call GetCurrentTexture on detached %s.", this);

    return {};
}

void SwapChainBase::SetChildLabel(ApiObjectBase* child) const {
    child->SetLabel(absl::StrFormat("of %s", this));
}

}  // namespace dawn::native
