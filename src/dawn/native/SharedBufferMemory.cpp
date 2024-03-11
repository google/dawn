// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/native/SharedBufferMemory.h"

#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"

namespace dawn::native {

namespace {

class ErrorSharedBufferMemory : public SharedBufferMemoryBase {
  public:
    ErrorSharedBufferMemory(DeviceBase* device, const SharedBufferMemoryDescriptor* descriptor)
        : SharedBufferMemoryBase(device, descriptor, ObjectBase::kError) {}

    ResultOrError<Ref<BufferBase>> CreateBufferImpl(
        const UnpackedPtr<BufferDescriptor>& descriptor) override {
        DAWN_UNREACHABLE();
    }
};

}  // namespace

// static
SharedBufferMemoryBase* SharedBufferMemoryBase::MakeError(
    DeviceBase* device,
    const SharedBufferMemoryDescriptor* descriptor) {
    return new ErrorSharedBufferMemory(device, descriptor);
}

SharedBufferMemoryBase::SharedBufferMemoryBase(DeviceBase* device,
                                               const SharedBufferMemoryDescriptor* descriptor,
                                               ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label),
      mProperties{nullptr, wgpu::BufferUsage::None, 0} {}

SharedBufferMemoryBase::SharedBufferMemoryBase(DeviceBase* device,
                                               const char* label,
                                               const SharedBufferMemoryProperties& properties)
    : ApiObjectBase(device, label), mProperties(properties) {
    GetObjectTrackingList()->Track(this);
}

ObjectType SharedBufferMemoryBase::GetType() const {
    return ObjectType::SharedBufferMemory;
}

void SharedBufferMemoryBase::DestroyImpl() {}

void SharedBufferMemoryBase::Initialize() {
    DAWN_ASSERT(!IsError());
    mContents = CreateContents();
}

void SharedBufferMemoryBase::APIGetProperties(SharedBufferMemoryProperties* properties) const {
    properties->usage = mProperties.usage;
    properties->size = mProperties.size;

    UnpackedPtr<SharedBufferMemoryProperties> unpacked;
    if (GetDevice()->ConsumedError(ValidateAndUnpack(properties), &unpacked,
                                   "calling %s.GetProperties", this)) {
        return;
    }
}

BufferBase* SharedBufferMemoryBase::APICreateBuffer(const BufferDescriptor* descriptor) {
    Ref<BufferBase> result;

    // Provide the defaults if no descriptor is provided.
    BufferDescriptor defaultDescriptor;
    if (descriptor == nullptr) {
        defaultDescriptor = {};
        defaultDescriptor.size = mProperties.size;
        defaultDescriptor.usage = mProperties.usage;
        descriptor = &defaultDescriptor;
    }

    if (GetDevice()->ConsumedError(CreateBuffer(descriptor), &result,
                                   InternalErrorType::OutOfMemory, "calling %s.CreateBuffer(%s).",
                                   this, descriptor)) {
        result = BufferBase::MakeError(GetDevice(), descriptor);
    }
    return ReturnToAPI(std::move(result));
}

ResultOrError<Ref<BufferBase>> SharedBufferMemoryBase::CreateBuffer(
    const BufferDescriptor* rawDescriptor) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));
    // Validate the buffer descriptor.
    UnpackedPtr<BufferDescriptor> descriptor;
    DAWN_TRY_ASSIGN(descriptor, ValidateBufferDescriptor(GetDevice(), rawDescriptor));

    // Ensure the buffer descriptor usage is a subset of the shared buffer memory's usage.
    DAWN_INVALID_IF(!IsSubset(descriptor->usage, mProperties.usage),
                    "The buffer usage (%s) is incompatible with the SharedBufferMemory usage (%s).",
                    descriptor->usage, mProperties.usage);

    // Validate that the buffer size exactly matches the shared buffer memory's size.
    DAWN_INVALID_IF(descriptor->size != mProperties.size,
                    "SharedBufferMemory size (%u) doesn't match descriptor size (%u).",
                    mProperties.size, descriptor->size);

    Ref<BufferBase> buffer;
    DAWN_TRY_ASSIGN(buffer, CreateBufferImpl(descriptor));
    // Access is not allowed until BeginAccess has been called.
    buffer->SetHasAccess(false);
    return buffer;
}

Ref<SharedBufferMemoryContents> SharedBufferMemoryBase::CreateContents() {
    return AcquireRef(new SharedBufferMemoryContents(GetWeakRef(this)));
}

SharedBufferMemoryContents* SharedBufferMemoryBase::GetContents() const {
    return mContents.Get();
}

bool SharedBufferMemoryBase::APIBeginAccess(BufferBase* buffer,
                                            const BeginAccessDescriptor* descriptor) {
    return false;
}

bool SharedBufferMemoryBase::APIEndAccess(BufferBase* buffer, EndAccessState* state) {
    return false;
}

bool SharedBufferMemoryBase::APIIsDeviceLost() {
    return GetDevice()->IsLost();
}

SharedBufferMemoryContents::SharedBufferMemoryContents(
    WeakRef<SharedBufferMemoryBase> sharedBufferMemory)
    : mSharedBufferMemory(std::move(sharedBufferMemory)) {}

const WeakRef<SharedBufferMemoryBase>& SharedBufferMemoryContents::GetSharedBufferMemory() const {
    return mSharedBufferMemory;
}

void APISharedBufferMemoryEndAccessStateFreeMembers(WGPUSharedBufferMemoryEndAccessState cState) {
    auto* state = reinterpret_cast<SharedBufferMemoryBase::EndAccessState*>(&cState);
    for (size_t i = 0; i < state->fenceCount; ++i) {
        state->fences[i]->APIRelease();
    }
    delete[] state->fences;
    delete[] state->signaledValues;
}

}  // namespace dawn::native
