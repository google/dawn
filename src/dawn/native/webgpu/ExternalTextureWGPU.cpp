// Copyright 2026 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/ExternalTextureWGPU.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/native/webgpu/DeviceWGPU.h"
#include "dawn/native/webgpu/TextureWGPU.h"
#include "dawn/native/webgpu/ToWGPU.h"

namespace dawn::native::webgpu {

// static
ResultOrError<Ref<ExternalTextureBase>> ExternalTexture::Create(
    Device* device,
    const ExternalTextureDescriptor* descriptor) {
    Ref<ExternalTexture> externalTexture = AcquireRef(new ExternalTexture(device, descriptor));
    DAWN_TRY(externalTexture->Initialize(device, descriptor));
    return externalTexture;
}

ExternalTexture::ExternalTexture(Device* device, const ExternalTextureDescriptor* descriptor)
    : ExternalTextureBase(device, descriptor),
      RecordableObject(schema::ObjectType::ExternalTexture),
      ObjectWGPU(device->wgpu.externalTextureRelease) {
    WGPUExternalTextureDescriptor desc = {
        .nextInChain = nullptr,
        .label = ToOutputStringView(GetLabel()),
        .plane0 = ToBackend(descriptor->plane0)->GetInnerHandle(),
        .plane1 = descriptor->plane1 ? ToBackend(descriptor->plane1)->GetInnerHandle() : nullptr,
        .cropOrigin = ToWGPU(descriptor->cropOrigin),
        .cropSize = ToWGPU(descriptor->cropSize),
        .apparentSize = ToWGPU(descriptor->apparentSize),
        .doYuvToRgbConversionOnly = descriptor->doYuvToRgbConversionOnly,
        .yuvToRgbConversionMatrix = descriptor->yuvToRgbConversionMatrix,
        .srcTransferFunctionParameters = descriptor->srcTransferFunctionParameters,
        .dstTransferFunctionParameters = descriptor->dstTransferFunctionParameters,
        .gamutConversionMatrix = descriptor->gamutConversionMatrix,
        .mirrored = descriptor->mirrored,
        .rotation = ToAPI(descriptor->rotation),
    };

    mInnerHandle = device->wgpu.deviceCreateExternalTexture(device->GetInnerHandle(), &desc);
    DAWN_ASSERT(mInnerHandle);
}

ExternalTexture::~ExternalTexture() = default;

void ExternalTexture::DestroyImpl(DestroyReason reason) {
    ExternalTextureBase::DestroyImpl(reason);
    auto& wgpu = ToBackend(GetDevice())->wgpu;
    wgpu.externalTextureDestroy(mInnerHandle);
}

void ExternalTexture::SetLabelImpl() {
    ToBackend(GetDevice())->CaptureSetLabel(this, GetLabel());
}

MaybeError ExternalTexture::AddReferenced(CaptureContext& captureContext) {
    // TODO(crbug.com/465184041): Capture ExternalTexture
    DAWN_UNREACHABLE();
}

MaybeError ExternalTexture::CaptureCreationParameters(CaptureContext& context) {
    // TODO(crbug.com/465184041): Capture ExternalTexture
    DAWN_UNREACHABLE();
}

}  // namespace dawn::native::webgpu
