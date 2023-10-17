// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/SharedTextureMemory.h"

#include <utility>

#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/Device.h"
#include "dawn/native/SharedFence.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

namespace {

class ErrorSharedTextureMemory : public SharedTextureMemoryBase {
  public:
    ErrorSharedTextureMemory(DeviceBase* device, const SharedTextureMemoryDescriptor* descriptor)
        : SharedTextureMemoryBase(device, descriptor, ObjectBase::kError) {}

    Ref<SharedTextureMemoryContents> CreateContents() override { DAWN_UNREACHABLE(); }
    ResultOrError<Ref<TextureBase>> CreateTextureImpl(
        const TextureDescriptor* descriptor) override {
        DAWN_UNREACHABLE();
    }
    MaybeError BeginAccessImpl(TextureBase* texture,
                               const BeginAccessDescriptor* descriptor) override {
        DAWN_UNREACHABLE();
    }
    ResultOrError<FenceAndSignalValue> EndAccessImpl(TextureBase* texture) override {
        DAWN_UNREACHABLE();
    }
};

}  // namespace

// static
SharedTextureMemoryBase* SharedTextureMemoryBase::MakeError(
    DeviceBase* device,
    const SharedTextureMemoryDescriptor* descriptor) {
    return new ErrorSharedTextureMemory(device, descriptor);
}

SharedTextureMemoryBase::SharedTextureMemoryBase(DeviceBase* device,
                                                 const SharedTextureMemoryDescriptor* descriptor,
                                                 ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label),
      mProperties{
          nullptr,
          wgpu::TextureUsage::None,
          {0, 0, 0},
          wgpu::TextureFormat::Undefined,
      },
      mContents(new SharedTextureMemoryContents(GetWeakRef(this))) {}

SharedTextureMemoryBase::SharedTextureMemoryBase(DeviceBase* device,
                                                 const char* label,
                                                 const SharedTextureMemoryProperties& properties)
    : ApiObjectBase(device, label), mProperties(properties) {
    const Format& internalFormat = device->GetValidInternalFormat(properties.format);
    if (!internalFormat.supportsStorageUsage) {
        DAWN_ASSERT(!(mProperties.usage & wgpu::TextureUsage::StorageBinding));
    }
    if (!internalFormat.isRenderable) {
        DAWN_ASSERT(!(mProperties.usage & wgpu::TextureUsage::RenderAttachment));
    }
    GetObjectTrackingList()->Track(this);
}

ObjectType SharedTextureMemoryBase::GetType() const {
    return ObjectType::SharedTextureMemory;
}

void SharedTextureMemoryBase::DestroyImpl() {}

void SharedTextureMemoryBase::Initialize() {
    DAWN_ASSERT(!IsError());
    mContents = CreateContents();
}

void SharedTextureMemoryBase::APIGetProperties(SharedTextureMemoryProperties* properties) const {
    properties->usage = mProperties.usage;
    properties->size = mProperties.size;
    properties->format = mProperties.format;

    if (GetDevice()->ConsumedError(ValidateSTypes(properties->nextInChain, {}),
                                   "calling %s.GetProperties", this)) {
        return;
    }
}

TextureBase* SharedTextureMemoryBase::APICreateTexture(const TextureDescriptor* descriptor) {
    Ref<TextureBase> result;

    // Provide the defaults if no descriptor is provided.
    TextureDescriptor defaultDescriptor;
    if (descriptor == nullptr) {
        defaultDescriptor = {};
        defaultDescriptor.format = mProperties.format;
        defaultDescriptor.size = mProperties.size;
        defaultDescriptor.usage = mProperties.usage;
        descriptor = &defaultDescriptor;
    }

    if (GetDevice()->ConsumedError(CreateTexture(descriptor), &result,
                                   InternalErrorType::OutOfMemory, "calling %s.CreateTexture(%s).",
                                   this, descriptor)) {
        return TextureBase::MakeError(GetDevice(), descriptor);
    }
    return result.Detach();
}

Ref<SharedTextureMemoryContents> SharedTextureMemoryBase::CreateContents() {
    return AcquireRef(new SharedTextureMemoryContents(GetWeakRef(this)));
}

ResultOrError<Ref<TextureBase>> SharedTextureMemoryBase::CreateTexture(
    const TextureDescriptor* descriptor) {
    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(this));

    // Validate that there is one 2D, single-sampled subresource
    DAWN_INVALID_IF(descriptor->dimension != wgpu::TextureDimension::e2D,
                    "Texture dimension (%s) is not %s.", descriptor->dimension,
                    wgpu::TextureDimension::e2D);
    DAWN_INVALID_IF(descriptor->mipLevelCount != 1, "Mip level count (%u) is not 1.",
                    descriptor->mipLevelCount);
    DAWN_INVALID_IF(descriptor->size.depthOrArrayLayers != 1, "Array layer count (%u) is not 1.",
                    descriptor->size.depthOrArrayLayers);
    DAWN_INVALID_IF(descriptor->sampleCount != 1, "Sample count (%u) is not 1.",
                    descriptor->sampleCount);

    // Validate that the texture size exactly matches the shared texture memory's size.
    DAWN_INVALID_IF(
        (descriptor->size.width != mProperties.size.width) ||
            (descriptor->size.height != mProperties.size.height) ||
            (descriptor->size.depthOrArrayLayers != mProperties.size.depthOrArrayLayers),
        "SharedTextureMemory size (%s) doesn't match descriptor size (%s).", &mProperties.size,
        &descriptor->size);

    // Validate that the texture format exactly matches the shared texture memory's format.
    DAWN_INVALID_IF(descriptor->format != mProperties.format,
                    "SharedTextureMemory format (%s) doesn't match descriptor format (%s).",
                    mProperties.format, descriptor->format);

    // Validate the rest of the texture descriptor, and require its usage to be a subset of the
    // shared texture memory's usage.
    DAWN_TRY(ValidateTextureDescriptor(GetDevice(), descriptor, AllowMultiPlanarTextureFormat::Yes,
                                       mProperties.usage));

    Ref<TextureBase> texture;
    DAWN_TRY_ASSIGN(texture, CreateTextureImpl(descriptor));
    // Access is started on memory.BeginAccess.
    texture->SetHasAccess(false);
    return texture;
}

SharedTextureMemoryContents* SharedTextureMemoryBase::GetContents() const {
    return mContents.Get();
}

MaybeError SharedTextureMemoryBase::ValidateTextureCreatedFromSelf(TextureBase* texture) {
    auto* contents = texture->GetSharedTextureMemoryContents();
    DAWN_INVALID_IF(contents == nullptr, "%s was not created from %s.", texture, this);

    auto* sharedTextureMemory =
        texture->GetSharedTextureMemoryContents()->GetSharedTextureMemory().Promote().Get();
    DAWN_INVALID_IF(sharedTextureMemory != this, "%s created from %s cannot be used with %s.",
                    texture, sharedTextureMemory, this);
    return {};
}

bool SharedTextureMemoryBase::APIBeginAccess(TextureBase* texture,
                                             const BeginAccessDescriptor* descriptor) {
    bool didBegin = false;
    DAWN_UNUSED(GetDevice()->ConsumedError(
        [&]() -> MaybeError {
            // Validate there is not another ongoing access and then set the current access.
            // This is done first because BeginAccess should acquire access regardless of whether or
            // not the internals generate an error.
            DAWN_INVALID_IF(mCurrentAccess != nullptr,
                            "Cannot begin access with %s on %s which is currently accessed by %s.",
                            texture, this, mCurrentAccess.Get());
            mCurrentAccess = texture;
            didBegin = true;

            return BeginAccess(texture, descriptor);
        }(),
        "calling %s.BeginAccess(%s).", this, texture));
    return didBegin;
}

MaybeError SharedTextureMemoryBase::BeginAccess(TextureBase* texture,
                                                const BeginAccessDescriptor* descriptor) {
    // Append begin fences first. Fences should be tracked regardless of whether later errors occur.
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        mContents->mPendingFences->push_back(
            {descriptor->fences[i], descriptor->signaledValues[i]});
    }

    DAWN_TRY(GetDevice()->ValidateIsAlive());
    DAWN_TRY(GetDevice()->ValidateObject(texture));
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        DAWN_TRY(GetDevice()->ValidateObject(descriptor->fences[i]));
    }

    DAWN_TRY(ValidateTextureCreatedFromSelf(texture));

    DAWN_INVALID_IF(texture->GetFormat().IsMultiPlanar() && !descriptor->initialized,
                    "BeginAccess on %s with multiplanar format (%s) must be initialized.", texture,
                    texture->GetFormat().format);

    DAWN_TRY(BeginAccessImpl(texture, descriptor));
    if (!texture->IsError()) {
        texture->SetHasAccess(true);
        texture->SetIsSubresourceContentInitialized(descriptor->initialized,
                                                    texture->GetAllSubresources());
    }
    return {};
}

bool SharedTextureMemoryBase::APIEndAccess(TextureBase* texture, EndAccessState* state) {
    bool didEnd = false;
    DAWN_UNUSED(GetDevice()->ConsumedError(
        [&]() -> MaybeError {
            DAWN_INVALID_IF(mCurrentAccess != texture,
                            "Cannot end access with %s on %s which is currently accessed by %s.",
                            texture, this, mCurrentAccess.Get());
            mCurrentAccess = nullptr;
            didEnd = true;

            return EndAccess(texture, state);
        }(),
        "calling %s.EndAccess(%s).", this, texture));
    return didEnd;
}

MaybeError SharedTextureMemoryBase::EndAccess(TextureBase* texture, EndAccessState* state) {
    PendingFenceList fenceList;
    mContents->AcquirePendingFences(&fenceList);

    if (!texture->IsError()) {
        texture->SetHasAccess(false);
    }

    // Call the error-generating part of the EndAccess implementation. This is separated out because
    // writing the output state must happen regardless of whether or not EndAccessInternal
    // succeeds.
    MaybeError err;
    {
        ResultOrError<FenceAndSignalValue> result = EndAccessInternal(texture, state);
        if (result.IsSuccess()) {
            fenceList->push_back(result.AcquireSuccess());
        } else {
            err = result.AcquireError();
        }
    }

    // Copy the fences to the output state.
    if (size_t fenceCount = fenceList->size()) {
        auto* fences = new SharedFenceBase*[fenceCount];
        uint64_t* signaledValues = new uint64_t[fenceCount];
        for (size_t i = 0; i < fenceCount; ++i) {
            fences[i] = fenceList[i].object.Detach();
            signaledValues[i] = fenceList[i].signaledValue;
        }

        state->fenceCount = fenceCount;
        state->fences = fences;
        state->signaledValues = signaledValues;
    } else {
        state->fenceCount = 0;
        state->fences = nullptr;
        state->signaledValues = nullptr;
    }
    state->initialized = texture->IsError() ||
                         texture->IsSubresourceContentInitialized(texture->GetAllSubresources());
    return err;
}

ResultOrError<FenceAndSignalValue> SharedTextureMemoryBase::EndAccessInternal(
    TextureBase* texture,
    EndAccessState* state) {
    DAWN_TRY(GetDevice()->ValidateObject(texture));
    DAWN_TRY(ValidateTextureCreatedFromSelf(texture));
    return EndAccessImpl(texture);
}

// SharedTextureMemoryContents

SharedTextureMemoryContents::SharedTextureMemoryContents(
    WeakRef<SharedTextureMemoryBase> sharedTextureMemory)
    : mSharedTextureMemory(std::move(sharedTextureMemory)) {}

const WeakRef<SharedTextureMemoryBase>& SharedTextureMemoryContents::GetSharedTextureMemory()
    const {
    return mSharedTextureMemory;
}

void SharedTextureMemoryContents::AcquirePendingFences(PendingFenceList* fences) {
    *fences = mPendingFences;
    mPendingFences->clear();
}

void SharedTextureMemoryContents::SetLastUsageSerial(ExecutionSerial lastUsageSerial) {
    mLastUsageSerial = lastUsageSerial;
}

ExecutionSerial SharedTextureMemoryContents::GetLastUsageSerial() const {
    return mLastUsageSerial;
}

void APISharedTextureMemoryEndAccessStateFreeMembers(WGPUSharedTextureMemoryEndAccessState cState) {
    auto* state = reinterpret_cast<SharedTextureMemoryBase::EndAccessState*>(&cState);
    for (size_t i = 0; i < state->fenceCount; ++i) {
        state->fences[i]->APIRelease();
    }
    delete[] state->fences;
    delete[] state->signaledValues;
}

}  // namespace dawn::native
