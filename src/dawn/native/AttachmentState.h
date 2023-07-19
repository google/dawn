// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_
#define SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_

#include <array>
#include <bitset>

#include "dawn/common/Constants.h"
#include "dawn/common/ityp_array.h"
#include "dawn/common/ityp_bitset.h"
#include "dawn/native/CachedObject.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

class DeviceBase;

class AttachmentState final : public ObjectBase, public CachedObject {
  public:
    // Note: Descriptors must be validated before the AttachmentState is constructed.
    explicit AttachmentState(DeviceBase* device, const RenderBundleEncoderDescriptor* descriptor);
    explicit AttachmentState(DeviceBase* device, const RenderPipelineDescriptor* descriptor);
    explicit AttachmentState(DeviceBase* device, const RenderPassDescriptor* descriptor);

    // Constructor used to avoid re-parsing descriptors when we already parsed them for cache keys.
    AttachmentState(const AttachmentState& blueprint);

    ~AttachmentState() override;

    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> GetColorAttachmentsMask() const;
    wgpu::TextureFormat GetColorAttachmentFormat(ColorAttachmentIndex index) const;
    bool HasDepthStencilAttachment() const;
    wgpu::TextureFormat GetDepthStencilFormat() const;
    uint32_t GetSampleCount() const;
    bool IsMSAARenderToSingleSampledEnabled() const;

    struct EqualityFunc {
        bool operator()(const AttachmentState* a, const AttachmentState* b) const;
    };
    size_t ComputeContentHash() override;

  private:
    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> mColorAttachmentsSet;
    ityp::array<ColorAttachmentIndex, wgpu::TextureFormat, kMaxColorAttachments> mColorFormats;
    // Default (texture format Undefined) indicates there is no depth stencil attachment.
    wgpu::TextureFormat mDepthStencilFormat = wgpu::TextureFormat::Undefined;
    uint32_t mSampleCount = 0;

    bool mIsMSAARenderToSingleSampledEnabled = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ATTACHMENTSTATE_H_
