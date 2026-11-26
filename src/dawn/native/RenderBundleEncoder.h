// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_
#define SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_

#include <vector>

#include "absl/container/flat_hash_set.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"
#include "src/dawn/native/EncodingContext.h"
#include "src/dawn/native/Error.h"
#include "src/dawn/native/Forward.h"
#include "src/dawn/native/RenderBundle.h"
#include "src/dawn/native/RenderEncoderBase.h"

namespace dawn::native {

ResultOrValError<UnpackedPtr<RenderBundleEncoderDescriptor>> ValidateRenderBundleEncoderDescriptor(
    DeviceBase* device,
    const RenderBundleEncoderDescriptor* descriptor);

class RenderBundleEncoder final : public RenderEncoderBase {
  public:
    static Ref<RenderBundleEncoder> Create(
        DeviceBase* device,
        const UnpackedPtr<RenderBundleEncoderDescriptor>& descriptor);
    static Ref<RenderBundleEncoder> MakeError(DeviceBase* device, StringView label);

    ~RenderBundleEncoder() override;

    ObjectType GetType() const override;

    void APISetPipeline(RenderPipelineBase* pipeline);
    RenderBundleBase* APIFinish(const RenderBundleDescriptor* descriptor);

    CommandIterator AcquireCommands();

    RenderPassResourceUsage AcquireRenderPassUsages();
    IndirectDrawMetadata AcquireIndirectDrawMetadata();
    std::vector<RenderPipelineBase*> AcquireUsedRenderPipelines();

    bool UsesResourceTable() const;

  private:
    RenderBundleEncoder(DeviceBase* device,
                        const UnpackedPtr<RenderBundleEncoderDescriptor>& descriptor);
    RenderBundleEncoder(DeviceBase* device, ErrorTag errorTag, StringView label);

    void DestroyImpl(DestroyReason reason) override;

    ResultOrUnknownError<Ref<RenderBundleBase>> Finish(const RenderBundleDescriptor* descriptor);
    MaybeValError ValidateFinish(const RenderPassResourceUsage& usages) const;

    EncodingContext mBundleEncodingContext;

    // Temporarily store for validation and will be moved to RenderBundle afterwards.
    RenderPassResourceUsage mUsages;
    // NOTE: populated on every SetPipeline, so intentionally left as raw pointers instead of
    // raw_ptr<T>. Kept alive by Ref<...> in mBundleEncodingContext's commands and cleared before
    // mBundleEncodingContext.Destroy() in DestroyImpl().
    RAW_PTR_EXCLUSION absl::flat_hash_set<RenderPipelineBase*> mUsedRenderPipelines;

    bool mUsesResourceTable = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERBUNDLEENCODER_H_
