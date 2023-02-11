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

#ifndef SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_
#define SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_

#include <IOSurface/IOSurfaceRef.h>
#import <Metal/Metal.h>
#include <vector>

#include "dawn/native/Texture.h"

#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/NSRef.h"
#include "dawn/native/DawnNative.h"
#include "dawn/native/MetalBackend.h"

namespace dawn::native::metal {

class CommandRecordingContext;
class Device;
struct MTLSharedEventAndSignalValue;

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
    static Ref<Texture> CreateWrapping(Device* device,
                                       const TextureDescriptor* descriptor,
                                       NSPRef<id<MTLTexture>> wrapped);

    Texture(DeviceBase* device, const TextureDescriptor* descriptor, TextureState state);

    id<MTLTexture> GetMTLTexture() const;
    IOSurfaceRef GetIOSurface();
    NSPRef<id<MTLTexture>> CreateFormatView(wgpu::TextureFormat format);

    bool ShouldKeepInitialized() const;

    MTLBlitOption ComputeMTLBlitOption(Aspect aspect) const;
    void EnsureSubresourceContentInitialized(CommandRecordingContext* commandContext,
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

    MaybeError ClearTexture(CommandRecordingContext* commandContext,
                            const SubresourceRange& range,
                            TextureBase::ClearValue clearValue);

    NSPRef<id<MTLTexture>> mMtlTexture;

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

    NSPRef<id<MTLTexture>> mMtlTextureView;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_TEXTUREMTL_H_
