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

#include "dawn/native/SwapChain.h"

#include "dawn/common/Constants.h"
#include "dawn/native/Adapter.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/Surface.h"
#include "dawn/native/Texture.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {

class ErrorSwapChain final : public SwapChainBase {
  public:
    explicit ErrorSwapChain(DeviceBase* device) : SwapChainBase(device, ObjectBase::kError) {}

  private:
    void APIConfigure(wgpu::TextureFormat format,
                      wgpu::TextureUsage allowedUsage,
                      uint32_t width,
                      uint32_t height) override {
        GetDevice()->ConsumedError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
    }

    TextureViewBase* APIGetCurrentTextureView() override {
        GetDevice()->ConsumedError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
        return TextureViewBase::MakeError(GetDevice());
    }

    void APIPresent() override {
        GetDevice()->ConsumedError(DAWN_VALIDATION_ERROR("%s is an error swapchain.", this));
    }
};

}  // anonymous namespace

MaybeError ValidateSwapChainDescriptor(const DeviceBase* device,
                                       const Surface* surface,
                                       const SwapChainDescriptor* descriptor) {
    if (descriptor->implementation != 0) {
        DAWN_INVALID_IF(surface != nullptr, "Exactly one of surface or implementation must be set");

        DawnSwapChainImplementation* impl =
            reinterpret_cast<DawnSwapChainImplementation*>(descriptor->implementation);

        DAWN_INVALID_IF(!impl->Init || !impl->Destroy || !impl->Configure ||
                            !impl->GetNextTexture || !impl->Present,
                        "Implementation is incomplete");

    } else {
        DAWN_INVALID_IF(surface == nullptr,
                        "At least one of surface or implementation must be set");
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

        DAWN_INVALID_IF(descriptor->usage != wgpu::TextureUsage::RenderAttachment,
                        "Usage (%s) is not %s, which is (currently) the only accepted usage.",
                        descriptor->usage, wgpu::TextureUsage::RenderAttachment);

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
    }

    return {};
}

TextureDescriptor GetSwapChainBaseTextureDescriptor(NewSwapChainBase* swapChain) {
    TextureDescriptor desc;
    desc.usage = swapChain->GetUsage();
    desc.dimension = wgpu::TextureDimension::e2D;
    desc.size = {swapChain->GetWidth(), swapChain->GetHeight(), 1};
    desc.format = swapChain->GetFormat();
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;

    return desc;
}

// SwapChainBase

SwapChainBase::SwapChainBase(DeviceBase* device) : ApiObjectBase(device, kLabelNotImplemented) {
    GetObjectTrackingList()->Track(this);
}

SwapChainBase::SwapChainBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

SwapChainBase::~SwapChainBase() {}

void SwapChainBase::DestroyImpl() {}

// static
SwapChainBase* SwapChainBase::MakeError(DeviceBase* device) {
    return new ErrorSwapChain(device);
}

ObjectType SwapChainBase::GetType() const {
    return ObjectType::SwapChain;
}

// OldSwapChainBase

OldSwapChainBase::OldSwapChainBase(DeviceBase* device, const SwapChainDescriptor* descriptor)
    : SwapChainBase(device),
      mImplementation(*reinterpret_cast<DawnSwapChainImplementation*>(descriptor->implementation)) {
}

OldSwapChainBase::~OldSwapChainBase() {
    if (!IsError()) {
        const auto& im = GetImplementation();
        im.Destroy(im.userData);
    }
}

void OldSwapChainBase::APIConfigure(wgpu::TextureFormat format,
                                    wgpu::TextureUsage allowedUsage,
                                    uint32_t width,
                                    uint32_t height) {
    if (GetDevice()->ConsumedError(ValidateConfigure(format, allowedUsage, width, height))) {
        return;
    }
    ASSERT(!IsError());

    allowedUsage |= wgpu::TextureUsage::Present;

    mFormat = format;
    mAllowedUsage = allowedUsage;
    mWidth = width;
    mHeight = height;
    mImplementation.Configure(mImplementation.userData, static_cast<WGPUTextureFormat>(format),
                              static_cast<WGPUTextureUsage>(allowedUsage), width, height);
}

TextureViewBase* OldSwapChainBase::APIGetCurrentTextureView() {
    if (GetDevice()->ConsumedError(ValidateGetCurrentTextureView())) {
        return TextureViewBase::MakeError(GetDevice());
    }
    ASSERT(!IsError());

    // Return the same current texture view until Present is called.
    if (mCurrentTextureView != nullptr) {
        // Calling GetCurrentTextureView always returns a new reference so add it even when
        // reuse the existing texture view.
        mCurrentTextureView->Reference();
        return mCurrentTextureView.Get();
    }

    // Create the backing texture and the view.
    TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = mWidth;
    descriptor.size.height = mHeight;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = mFormat;
    descriptor.mipLevelCount = 1;
    descriptor.usage = mAllowedUsage;

    // Get the texture but remove the external refcount because it is never passed outside
    // of dawn_native
    mCurrentTexture = AcquireRef(GetNextTextureImpl(&descriptor));

    mCurrentTextureView = mCurrentTexture->APICreateView();
    return mCurrentTextureView.Get();
}

void OldSwapChainBase::APIPresent() {
    if (GetDevice()->ConsumedError(ValidatePresent())) {
        return;
    }
    ASSERT(!IsError());

    if (GetDevice()->ConsumedError(OnBeforePresent(mCurrentTextureView.Get()))) {
        return;
    }

    mImplementation.Present(mImplementation.userData);

    mCurrentTexture = nullptr;
    mCurrentTextureView = nullptr;
}

const DawnSwapChainImplementation& OldSwapChainBase::GetImplementation() {
    ASSERT(!IsError());
    return mImplementation;
}

MaybeError OldSwapChainBase::ValidateConfigure(wgpu::TextureFormat format,
                                               wgpu::TextureUsage allowedUsage,
                                               uint32_t width,
                                               uint32_t height) const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_TRY(ValidateTextureUsage(allowedUsage));
    DAWN_TRY(ValidateTextureFormat(format));

    DAWN_INVALID_IF(width == 0 || height == 0,
                    "Configuration size (width: %u, height: %u) for %s is empty.", width, height,
                    this);

    return {};
}

MaybeError OldSwapChainBase::ValidateGetCurrentTextureView() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    // If width is 0, it implies swap chain has never been configured
    DAWN_INVALID_IF(mWidth == 0, "%s was not configured prior to calling GetNextTexture.", this);

    return {};
}

MaybeError OldSwapChainBase::ValidatePresent() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(
        mCurrentTextureView == nullptr,
        "GetCurrentTextureView was not called on %s this frame prior to calling Present.", this);

    return {};
}

// Implementation of NewSwapChainBase

NewSwapChainBase::NewSwapChainBase(DeviceBase* device,
                                   Surface* surface,
                                   const SwapChainDescriptor* descriptor)
    : SwapChainBase(device),
      mAttached(false),
      mWidth(descriptor->width),
      mHeight(descriptor->height),
      mFormat(descriptor->format),
      mUsage(descriptor->usage),
      mPresentMode(descriptor->presentMode),
      mSurface(surface) {}

NewSwapChainBase::~NewSwapChainBase() {
    if (mCurrentTextureView != nullptr) {
        ASSERT(mCurrentTextureView->GetTexture()->GetTextureState() ==
               TextureBase::TextureState::Destroyed);
    }

    ASSERT(!mAttached);
}

void NewSwapChainBase::DetachFromSurface() {
    if (mAttached) {
        DetachFromSurfaceImpl();
        mSurface = nullptr;
        mAttached = false;
    }
}

void NewSwapChainBase::SetIsAttached() {
    mAttached = true;
}

void NewSwapChainBase::APIConfigure(wgpu::TextureFormat format,
                                    wgpu::TextureUsage allowedUsage,
                                    uint32_t width,
                                    uint32_t height) {
    GetDevice()->ConsumedError(
        DAWN_VALIDATION_ERROR("Configure is invalid for surface-based swapchains."));
}

TextureViewBase* NewSwapChainBase::APIGetCurrentTextureView() {
    Ref<TextureViewBase> result;
    if (GetDevice()->ConsumedError(GetCurrentTextureView(), &result,
                                   "calling %s.GetCurrentTextureView()", this)) {
        return TextureViewBase::MakeError(GetDevice());
    }
    return result.Detach();
}

ResultOrError<Ref<TextureViewBase>> NewSwapChainBase::GetCurrentTextureView() {
    DAWN_TRY(ValidateGetCurrentTextureView());

    if (mCurrentTextureView != nullptr) {
        // Calling GetCurrentTextureView always returns a new reference.
        return mCurrentTextureView;
    }

    DAWN_TRY_ASSIGN(mCurrentTextureView, GetCurrentTextureViewImpl());

    // Check that the return texture view matches exactly what was given for this descriptor.
    ASSERT(mCurrentTextureView->GetTexture()->GetFormat().format == mFormat);
    ASSERT(IsSubset(mUsage, mCurrentTextureView->GetTexture()->GetUsage()));
    ASSERT(mCurrentTextureView->GetLevelCount() == 1);
    ASSERT(mCurrentTextureView->GetLayerCount() == 1);
    ASSERT(mCurrentTextureView->GetDimension() == wgpu::TextureViewDimension::e2D);
    ASSERT(mCurrentTextureView->GetTexture()
               ->GetMipLevelSingleSubresourceVirtualSize(mCurrentTextureView->GetBaseMipLevel())
               .width == mWidth);
    ASSERT(mCurrentTextureView->GetTexture()
               ->GetMipLevelSingleSubresourceVirtualSize(mCurrentTextureView->GetBaseMipLevel())
               .height == mHeight);

    return mCurrentTextureView;
}

void NewSwapChainBase::APIPresent() {
    if (GetDevice()->ConsumedError(ValidatePresent())) {
        return;
    }

    if (GetDevice()->ConsumedError(PresentImpl())) {
        return;
    }

    ASSERT(mCurrentTextureView->GetTexture()->GetTextureState() ==
           TextureBase::TextureState::Destroyed);
    mCurrentTextureView = nullptr;
}

uint32_t NewSwapChainBase::GetWidth() const {
    return mWidth;
}

uint32_t NewSwapChainBase::GetHeight() const {
    return mHeight;
}

wgpu::TextureFormat NewSwapChainBase::GetFormat() const {
    return mFormat;
}

wgpu::TextureUsage NewSwapChainBase::GetUsage() const {
    return mUsage;
}

wgpu::PresentMode NewSwapChainBase::GetPresentMode() const {
    return mPresentMode;
}

Surface* NewSwapChainBase::GetSurface() const {
    return mSurface;
}

bool NewSwapChainBase::IsAttached() const {
    return mAttached;
}

wgpu::BackendType NewSwapChainBase::GetBackendType() const {
    return GetDevice()->GetAdapter()->GetBackendType();
}

MaybeError NewSwapChainBase::ValidatePresent() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call Present called on detached %s.", this);

    DAWN_INVALID_IF(
        mCurrentTextureView == nullptr,
        "GetCurrentTextureView was not called on %s this frame prior to calling Present.", this);

    return {};
}

MaybeError NewSwapChainBase::ValidateGetCurrentTextureView() const {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    DAWN_INVALID_IF(!mAttached, "Cannot call GetCurrentTextureView on detached %s.", this);

    return {};
}

}  // namespace dawn::native
