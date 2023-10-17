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

#ifndef SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_
#define SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_

#include <IOSurface/IOSurfaceRef.h>
#import <Metal/Metal.h>
#include <vector>

#include "dawn/native/Texture.h"

#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/NSRef.h"
#include "dawn/common/StackContainer.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/MetalBackend.h"

namespace dawn::native::metal {

class CommandRecordingContext;
class Device;
struct MTLSharedEventAndSignalValue;
class SharedTextureMemory;

MTLPixelFormat MetalPixelFormat(const DeviceBase* device, wgpu::TextureFormat format);
MaybeError ValidateIOSurfaceCanBeWrapped(const DeviceBase* device,
                                         const TextureDescriptor* descriptor,
                                         IOSurfaceRef ioSurface);

class Texture final : public TextureBase {
  public:
    static ResultOrError<Ref<Texture>> Create(Device* device, const TextureDescriptor* descriptor);
    static ResultOrError<Ref<Texture>> CreateFromIOSurface(
        Device* device,
        const ExternalImageDescriptor* descriptor,
        IOSurfaceRef ioSurface,
        std::vector<MTLSharedEventAndSignalValue> waitEvents);
    static ResultOrError<Ref<Texture>> CreateFromSharedTextureMemory(
        SharedTextureMemory* memory,
        const TextureDescriptor* descriptor);
    static Ref<Texture> CreateWrapping(Device* device,
                                       const TextureDescriptor* descriptor,
                                       NSPRef<id<MTLTexture>> wrapped);

    Texture(DeviceBase* device, const TextureDescriptor* descriptor);

    id<MTLTexture> GetMTLTexture(Aspect aspect) const;
    IOSurfaceRef GetIOSurface();
    NSPRef<id<MTLTexture>> CreateFormatView(wgpu::TextureFormat format);

    bool ShouldKeepInitialized() const;

    MTLBlitOption ComputeMTLBlitOption(Aspect aspect) const;
    MaybeError EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
                                                   const SubresourceRange& range);

    void SynchronizeTextureBeforeUse(CommandRecordingContext* commandContext);
    void IOSurfaceEndAccess(ExternalImageIOSurfaceEndAccessDescriptor* descriptor);

  private:
    using TextureBase::TextureBase;
    ~Texture() override;

    NSRef<MTLTextureDescriptor> CreateMetalTextureDescriptor() const;

    MaybeError InitializeAsInternalTexture(const TextureDescriptor* descriptor);
    MaybeError InitializeFromIOSurface(const ExternalImageDescriptor* descriptor,
                                       const TextureDescriptor* textureDescriptor,
                                       IOSurfaceRef ioSurface,
                                       std::vector<MTLSharedEventAndSignalValue> waitEvents);
    void InitializeAsWrapping(const TextureDescriptor* descriptor, NSPRef<id<MTLTexture>> wrapped);

    void DestroyImpl() override;
    void SetLabelImpl() override;

    MaybeError ClearTexture(CommandRecordingContext* commandContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue clearValue);

    StackVector<NSPRef<id<MTLTexture>>, kMaxPlanesPerFormat> mMtlPlaneTextures;
    MTLPixelFormat mMtlFormat = MTLPixelFormatInvalid;

    MTLTextureUsage mMtlUsage;
    CFRef<IOSurfaceRef> mIOSurface = nullptr;
    std::vector<MTLSharedEventAndSignalValue> mWaitEvents;
};

class TextureView final : public TextureViewBase {
  public:
    static ResultOrError<Ref<TextureView>> Create(TextureBase* texture,
                                                  const TextureViewDescriptor* descriptor);

    id<MTLTexture> GetMTLTexture() const;

    struct AttachmentInfo {
        NSPRef<id<MTLTexture>> texture;
        uint32_t baseMipLevel;
        uint32_t baseArrayLayer;
    };
    AttachmentInfo GetAttachmentInfo() const;

  private:
    using TextureViewBase::TextureViewBase;
    MaybeError Initialize(const TextureViewDescriptor* descriptor);
    void DestroyImpl() override;
    void SetLabelImpl() override;

    NSPRef<id<MTLTexture>> mMtlTextureView;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_
