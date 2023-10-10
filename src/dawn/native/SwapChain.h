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

#ifndef SRC_DAWN_NATIVE_SWAPCHAIN_H_
#define SRC_DAWN_NATIVE_SWAPCHAIN_H_

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateSwapChainDescriptor(const DeviceBase* device,
                                       const Surface* surface,
                                       const SwapChainDescriptor* descriptor);

TextureDescriptor GetSwapChainBaseTextureDescriptor(SwapChainBase* swapChain);

class SwapChainBase : public ApiObjectBase {
  public:
    SwapChainBase(DeviceBase* device, Surface* surface, const SwapChainDescriptor* descriptor);

    static SwapChainBase* MakeError(DeviceBase* device, const SwapChainDescriptor* descriptor);
    ObjectType GetType() const override;

    // This is called when the swapchain is detached when one of the following happens:
    //
    //  - The surface it is attached to is being destroyed.
    //  - The swapchain is being replaced by another one on the surface.
    //
    // Note that the surface has a Ref on the last swapchain that was used on it so the
    // SwapChain destructor will only be called after one of the things above happens.
    //
    // The call for the detaching previous swapchain should be called inside the backend
    // implementation of SwapChains. This is to allow them to acquire any resources before
    // calling detach to make a seamless transition from the previous swapchain.
    //
    // Likewise the call for the swapchain being destroyed must be done in the backend's
    // swapchain's destructor since C++ says it is UB to call virtual methods in the base class
    // destructor.
    void DetachFromSurface();

    void SetIsAttached();

    // Dawn API
    void APIConfigure(wgpu::TextureFormat format,
                      wgpu::TextureUsage allowedUsage,
                      uint32_t width,
                      uint32_t height);
    TextureBase* APIGetCurrentTexture();
    TextureViewBase* APIGetCurrentTextureView();
    void APIPresent();

    // TODO(crbug.com/dawn/831):
    // APIRelease() can be called without any synchronization guarantees so we need to use a Release
    // method that will call LockAndDeleteThis() on destruction.
    // This is because losing the last reference to the SwapChain will detach its surface which
    // explicitly destroys the current texture. Explicit destruction of textures is not thread safe
    // yet.
    void APIRelease() { ReleaseAndLockBeforeDestroy(); }

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    wgpu::TextureFormat GetFormat() const;
    wgpu::TextureUsage GetUsage() const;
    wgpu::PresentMode GetPresentMode() const;
    Surface* GetSurface() const;
    bool IsAttached() const;
    wgpu::BackendType GetBackendType() const;

  protected:
    SwapChainBase(DeviceBase* device, const SwapChainDescriptor* desc, ObjectBase::ErrorTag tag);
    ~SwapChainBase() override;
    void DestroyImpl() override;

  private:
    void SetChildLabel(ApiObjectBase* child) const;

    bool mAttached = false;
    uint32_t mWidth;
    uint32_t mHeight;
    wgpu::TextureFormat mFormat;
    wgpu::TextureUsage mUsage;
    wgpu::PresentMode mPresentMode;

    // This is a weak reference to the surface. If the surface is destroyed it will call
    // DetachFromSurface and mSurface will be updated to nullptr.
    Surface* mSurface = nullptr;
    Ref<TextureBase> mCurrentTexture;

    MaybeError ValidatePresent() const;
    MaybeError ValidateGetCurrentTexture() const;

    // GetCurrentTextureImpl and PresentImpl are guaranteed to be called in an interleaved manner,
    // starting with GetCurrentTextureImpl.

    // The returned texture must match the swapchain descriptor exactly.
    ResultOrError<Ref<TextureBase>> GetCurrentTexture();
    virtual ResultOrError<Ref<TextureBase>> GetCurrentTextureImpl() = 0;

    ResultOrError<Ref<TextureViewBase>> GetCurrentTextureView();

    // The call to present must destroy the current texture so further access to it are invalid.
    virtual MaybeError PresentImpl() = 0;

    // Guaranteed to be called exactly once during the lifetime of the SwapChain. After it is
    // called no other virtual method can be called.
    virtual void DetachFromSurfaceImpl() = 0;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_SWAPCHAIN_H_
