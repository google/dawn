// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/TexelBufferView.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

TexelBufferViewBase::TexelBufferViewBase(BufferBase* buffer,
                                         const UnpackedPtr<TexelBufferViewDescriptor>& desc)
    : ApiObjectBase(buffer->GetDevice(), desc->label),
      mBuffer(buffer),
      mFormat(desc->format),
      mOffset(desc->offset),
      mSize(desc->size) {
    GetObjectTrackingList()->Track(this);
}

TexelBufferViewBase::TexelBufferViewBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         StringView label)
    : ApiObjectBase(device, tag, label) {}

TexelBufferViewBase::~TexelBufferViewBase() = default;

Ref<TexelBufferViewBase> TexelBufferViewBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new TexelBufferViewBase(device, ObjectBase::kError, label));
}

ObjectType TexelBufferViewBase::GetType() const {
    return ObjectType::TexelBufferView;
}

BufferBase* TexelBufferViewBase::GetBuffer() const {
    DAWN_ASSERT(!IsError());
    return mBuffer.Get();
}

wgpu::TextureFormat TexelBufferViewBase::GetFormat() const {
    DAWN_ASSERT(!IsError());
    return mFormat;
}

uint64_t TexelBufferViewBase::GetOffset() const {
    DAWN_ASSERT(!IsError());
    return mOffset;
}

uint64_t TexelBufferViewBase::GetSize() const {
    DAWN_ASSERT(!IsError());
    return mSize;
}

void TexelBufferViewBase::DestroyImpl() {}

ApiObjectList* TexelBufferViewBase::GetObjectTrackingList() {
    if (mBuffer != nullptr) {
        return mBuffer->GetTexelBufferViewTrackingList();
    }
    return ApiObjectBase::GetObjectTrackingList();
}

}  // namespace dawn::native
