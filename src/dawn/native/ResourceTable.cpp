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

#include "dawn/native/ResourceTable.h"

#include <utility>

#include "dawn/common/Enumerator.h"
#include "dawn/common/Range.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"
#include "dawn/native/Queue.h"
#include "tint/api/common/resource_type.h"

namespace dawn::native {

namespace {

// Compute the tint::ResourceType that should be in the metadata buffer for the resource.
tint::ResourceType ComputeTypeId(const TextureViewBase* view) {
    if (view == nullptr) {
        return tint::ResourceType::kEmpty;
    }
    const TextureBase* texture = view->GetTexture();

    // TODO(https://issues.chromium.org/473354065): In the future we should allow the same
    // compatibility rules that exist between TextureView and BGLEntry. This means that a depth
    // texture can be either a texture_depth_2d, or a texture_2d<f32> (unfilterable). We should
    // also find a way to differentiate unfilterable and filterable texture_2d<f32>.

    if (texture->IsMultisampledTexture()) {
        DAWN_ASSERT(view->GetDimension() == wgpu::TextureViewDimension::e2D);

        switch (view->GetAspects()) {
            case Aspect::Color:
                switch (view->GetFormat().GetAspectInfo(Aspect::Color).baseType) {
                    case TextureComponentType::Float:
                        return tint::ResourceType::kTextureMultisampled2d_f32;
                    case TextureComponentType::Uint:
                        return tint::ResourceType::kTextureMultisampled2d_u32;
                    case TextureComponentType::Sint:
                        return tint::ResourceType::kTextureMultisampled2d_i32;
                    default:
                        DAWN_UNREACHABLE();
                }

            case Aspect::Depth:
                return tint::ResourceType::kTextureDepthMultisampled2d;
            default:
                DAWN_UNREACHABLE();
        }
    }

    if (view->GetAspects() == Aspect::Depth) {
        DAWN_ASSERT(!texture->IsMultisampledTexture());

        switch (view->GetDimension()) {
            case wgpu::TextureViewDimension::e2D:
                return tint::ResourceType::kTextureDepth2d;
            case wgpu::TextureViewDimension::e2DArray:
                return tint::ResourceType::kTextureDepth2dArray;
            case wgpu::TextureViewDimension::Cube:
                return tint::ResourceType::kTextureDepthCube;
            case wgpu::TextureViewDimension::CubeArray:
                return tint::ResourceType::kTextureDepthCubeArray;
            default:
                DAWN_UNREACHABLE();
        }
    }

    switch (view->GetFormat().GetAspectInfo(view->GetAspects()).baseType) {
        case TextureComponentType::Float:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_f32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_f32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_f32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_f32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_f32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_f32;
                default:
                    DAWN_UNREACHABLE();
            }
        case TextureComponentType::Uint:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_u32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_u32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_u32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_u32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_u32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_u32;
                default:
                    DAWN_UNREACHABLE();
            }
        case TextureComponentType::Sint:
            switch (view->GetDimension()) {
                case wgpu::TextureViewDimension::e1D:
                    return tint::ResourceType::kTexture1d_i32;
                case wgpu::TextureViewDimension::e2D:
                    return tint::ResourceType::kTexture2d_i32;
                case wgpu::TextureViewDimension::e2DArray:
                    return tint::ResourceType::kTexture2dArray_i32;
                case wgpu::TextureViewDimension::Cube:
                    return tint::ResourceType::kTextureCube_i32;
                case wgpu::TextureViewDimension::CubeArray:
                    return tint::ResourceType::kTextureCubeArray_i32;
                case wgpu::TextureViewDimension::e3D:
                    return tint::ResourceType::kTexture3d_i32;
                default:
                    DAWN_UNREACHABLE();
            }
        default:
            DAWN_UNREACHABLE();
    }
}

MaybeError ValidateBindingResource(const DeviceBase* device, const BindingResource* resource) {
    DAWN_INVALID_IF(resource->nextInChain != nullptr, "nextInChain is not null.");

    uint32_t resourceCount = uint32_t(resource->buffer != nullptr) +
                             uint32_t(resource->textureView != nullptr) +
                             uint32_t(resource->sampler != nullptr);
    DAWN_INVALID_IF(resourceCount != 1,
                    "%i resources are specified (when there must be exactly 1).", resourceCount);

    // TODO(https://issues.chromium.org/473444515): Support buffers in FullResourceTable.
    if (resource->buffer != nullptr) {
        return DAWN_VALIDATION_ERROR("Buffers are not supported.");
    }

    // TODO(https://issues.chromium.org/473354063): Support samplers in SamplingResourceTable.
    if (resource->sampler != nullptr) {
        return DAWN_VALIDATION_ERROR("Samplers are not supported.");
    }

    // TODO(https://issues.chromium.org/473444515): Support texel buffers in FullResourceTable.

    if (resource->textureView != nullptr) {
        TextureViewBase* view = resource->textureView;
        DAWN_TRY(device->ValidateObject(view));

        Aspect aspect = view->GetAspects();
        DAWN_INVALID_IF(!HasOneBit(aspect),
                        "Multiple aspects (%s) selected in %s. Expected only 1.", aspect, view);

        // TODO(https://issues.chromium.org/473444515): Support storage textures in
        // FullResourceTable
        DAWN_INVALID_IF(
            (view->GetUsage() & kTextureViewOnlyUsages) != wgpu::TextureUsage::TextureBinding,
            "%s's usages (%s) are not exactly %s.", view, view->GetUsage() & kTextureViewOnlyUsages,
            wgpu::TextureUsage::TextureBinding);

        DAWN_INVALID_IF(view->IsYCbCr(), "%s is YCbCr.", view);
    }

    return {};
}

}  // anonymous namespace

ityp::span<ResourceTableSlot, const tint::ResourceType> GetDefaultResourceOrder() {
    static constexpr auto kDefaults = std::array{
        tint::ResourceType::kTexture1d_f32,
        tint::ResourceType::kTexture2d_f32,
        tint::ResourceType::kTexture2dArray_f32,
        tint::ResourceType::kTextureCube_f32,
        tint::ResourceType::kTextureCubeArray_f32,
        tint::ResourceType::kTexture3d_f32,

        tint::ResourceType::kTexture1d_u32,
        tint::ResourceType::kTexture2d_u32,
        tint::ResourceType::kTexture2dArray_u32,
        tint::ResourceType::kTextureCube_u32,
        tint::ResourceType::kTextureCubeArray_u32,
        tint::ResourceType::kTexture3d_u32,

        tint::ResourceType::kTexture1d_i32,
        tint::ResourceType::kTexture2d_i32,
        tint::ResourceType::kTexture2dArray_i32,
        tint::ResourceType::kTextureCube_i32,
        tint::ResourceType::kTextureCubeArray_i32,
        tint::ResourceType::kTexture3d_i32,

        tint::ResourceType::kTextureMultisampled2d_f32,
        tint::ResourceType::kTextureMultisampled2d_u32,
        tint::ResourceType::kTextureMultisampled2d_i32,

        tint::ResourceType::kTextureDepth2d,
        tint::ResourceType::kTextureDepth2dArray,
        tint::ResourceType::kTextureDepthCube,
        tint::ResourceType::kTextureDepthCubeArray,
        tint::ResourceType::kTextureDepthMultisampled2d,
    };

    return {kDefaults.data(), ResourceTableSlot(uint32_t(kDefaults.size()))};
}

ResourceTableSlot GetDefaultResourceCount() {
    return GetDefaultResourceOrder().size();
}

MaybeError ValidateResourceTableDescriptor(const DeviceBase* device,
                                           const ResourceTableDescriptor* descriptor) {
    DAWN_ASSERT(descriptor);

    DAWN_INVALID_IF(!device->HasFeature(Feature::ChromiumExperimentalSamplingResourceTable),
                    "Resource table used without the %s feature enabled.",
                    wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable);

    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain is not nullptr.");

    DAWN_INVALID_IF(descriptor->size > device->GetLimits().resourceTableLimits.maxResourceTableSize,
                    "Resource table size (%u) is larger than maxResourceTableSize (%u)",
                    descriptor->size, device->GetLimits().resourceTableLimits.maxResourceTableSize);

    return {};
}

ResourceTableBase::ResourceTableBase(DeviceBase* device, const ResourceTableDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label), mAPISize(ResourceTableSlot(descriptor->size)) {
    mSlots.resize(mAPISize + GetDefaultResourceCount());
    // This checks that the default SlotState constructor used in the resize operation will
    // initialize with the typeId of an empty slot.
    DAWN_ASSERT(ComputeTypeId(nullptr) == SlotState{}.typeId);

    GetObjectTrackingList()->Track(this);
}

ResourceTableBase::ResourceTableBase(DeviceBase* device,
                                     const ResourceTableDescriptor* descriptor,
                                     ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {
    // Create the vector of SlotState even for an error resource table because we need to do state
    // tracking used for the validation of synchronous errors. However skip creating it for tables
    // above the limit because that's a special error case caught on the content-timeline as well.
    if (descriptor->size <= device->GetLimits().resourceTableLimits.maxResourceTableSize) {
        mAPISize = ResourceTableSlot(descriptor->size);
        mSlots.resize(mAPISize);
    } else {
        mDestroyed = true;
    }
}

// static
Ref<ResourceTableBase> ResourceTableBase::MakeError(DeviceBase* device,
                                                    const ResourceTableDescriptor* descriptor) {
    return AcquireRef(new ResourceTableBase(device, descriptor, ObjectBase::kError));
}

ObjectType ResourceTableBase::GetType() const {
    return ObjectType::ResourceTable;
}

ResourceTableSlot ResourceTableBase::GetAPISize() const {
    return mAPISize;
}

ResourceTableSlot ResourceTableBase::GetSizeWithDefaultResources() const {
    return mSlots.size();
}

BufferBase* ResourceTableBase::GetMetadataBuffer() const {
    DAWN_ASSERT(!mDestroyed);
    return mMetadataBuffer.Get();
}

bool ResourceTableBase::IsDestroyed() const {
    return mDestroyed;
}

MaybeError ResourceTableBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(IsDestroyed(), "%s used while destroyed.", this);
    return {};
}

MaybeError ResourceTableBase::InitializeBase() {
    DeviceBase* device = GetDevice();

    // Create a storage buffer that will hold the shader-visible metadata for the dynamic array.
    uint32_t metadataArrayLength = uint32_t(GetSizeWithDefaultResources());
    BufferDescriptor metadataDesc{
        .label = "resource table metadata",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = sizeof(uint32_t) * (metadataArrayLength + 1),
        .mappedAtCreation = true,
    };
    DAWN_TRY_ASSIGN(mMetadataBuffer, device->CreateBuffer(&metadataDesc));

    // Initialize the metadata buffer with the arrayLength and a bunch of zeroes that correspond to
    // empty entries.
    DAWN_ASSERT(uint32_t(tint::ResourceType::kEmpty) == 0);
    // TODO(https://crbug.com/435317394): We could rely on zero initialization if it is enabled, and
    // also apply the initial dirty slots in this mapping instead of on the first use of the
    // resource table.
    uint32_t* data = static_cast<uint32_t*>(mMetadataBuffer->GetMappedRange(0, metadataDesc.size));
    *data = uint32_t(mAPISize);
    memset(data + 1, 0, metadataDesc.size - sizeof(uint32_t));
    DAWN_TRY(mMetadataBuffer->Unmap());

    // Add the default resources at the end of the table.
    ityp::span<ResourceTableSlot, Ref<TextureViewBase>> defaultResources;
    DAWN_TRY_ASSIGN(
        defaultResources,
        device->GetResourceTableDefaultResources()->GetOrCreateSampledTextureDefaults(device));

    for (auto [i, defaultResource] : Enumerate(defaultResources)) {
        BindingResource entryContents = {
            .textureView = defaultResource.Get(),
        };
        Update(mAPISize + i, &entryContents);
    }

    return {};
}

void ResourceTableBase::DestroyImpl(DestroyReason reason) {
    DAWN_ASSERT(!mDestroyed);

    for (auto [i, slot] : Enumerate(mSlots)) {
        if (slot.resource != nullptr) {
            slot.resource->GetTexture()->RemoveResourceTableSlotUse(this, i);
        }
    }

    mSlots.clear();
    mDirtySlots.clear();

    if (mMetadataBuffer != nullptr) {
        mMetadataBuffer->Destroy();
        mMetadataBuffer = nullptr;
    }

    mDestroyed = true;
}

void ResourceTableBase::APIDestroy() {
    // Handle error objects directly because Destroy() will skip calling DestroyImpl for them.
    if (IsError()) {
        mSlots.clear();
        mDirtySlots.clear();
        mDestroyed = true;
    } else {
        Destroy();
    }
}

wgpu::Status ResourceTableBase::APIUpdate(uint32_t slotIn, const BindingResource* resource) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Prevent replacing a slot that may be in use by the GPU.
    if (mSlots[slot].availableAfter > GetDevice()->GetQueue()->GetCompletedCommandSerial()) {
        return wgpu::Status::Error;
    }

    UpdateWithDeviceValidation(slot, resource, "Update");
    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIInsertBinding(const BindingResource* resource) {
    if (IsDestroyed()) {
        return wgpu::kInvalidBinding;
    }

    // TODO(https://crbug.com/435317394): This is O(n) in the number of slots. We could make it
    // O(logN) with a heap of the free slots that's maintained over time.
    ExecutionSerial completedSerial = GetDevice()->GetQueue()->GetCompletedCommandSerial();
    for (ResourceTableSlot slot : Range(mAPISize)) {
        if (mSlots[slot].availableAfter > completedSerial) {
            continue;
        }

        UpdateWithDeviceValidation(slot, resource, "InsertBinding");
        return uint32_t(slot);
    }

    // No slot found, return the invalid binding.
    return wgpu::kInvalidBinding;
}

wgpu::Status ResourceTableBase::APIRemoveBinding(uint32_t slotIn) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Always remove the slot, even if a validation error happens, so that we match client-side
    // validation.
    Remove(slot);

    [[maybe_unused]] bool error = GetDevice()->ConsumedError(
        GetDevice()->ValidateObject(this), "validating %s.RemoveBinding(%u)", this, slot);
    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIGetSize() const {
    return uint32_t(mAPISize);
}

bool ResourceTableBase::IsValidSlot(ResourceTableSlot slot) const {
    // Some validation is required to return a synchronous error. It needs to be able to run even on
    // error ResourceTables because it must act the same way as an implementation running on top of
    // the wire client-side and doesn't know if objects are errors or not.
    return !mDestroyed && slot < mAPISize;
}

void ResourceTableBase::OnPinned(ResourceTableSlot slot, TextureBase* texture) {
    DAWN_ASSERT(!mDestroyed);
    DAWN_ASSERT(mSlots[slot].resource != nullptr);
    DAWN_ASSERT(mSlots[slot].resource->GetTexture() == texture);
    DAWN_ASSERT(!mSlots[slot].pinned);
    mSlots[slot].pinned = true;
    MarkStateDirty(slot);
}

void ResourceTableBase::OnUnpinned(ResourceTableSlot slot, TextureBase* texture) {
    DAWN_ASSERT(!mDestroyed);
    DAWN_ASSERT(mSlots[slot].resource != nullptr);
    DAWN_ASSERT(mSlots[slot].resource->GetTexture() == texture);
    DAWN_ASSERT(mSlots[slot].pinned);
    mSlots[slot].pinned = false;
    MarkStateDirty(slot);
}

void ResourceTableBase::Update(ResourceTableSlot slot, const BindingResource* contents) {
    DAWN_ASSERT(mSlots[slot].availableAfter <=
                GetDevice()->GetQueue()->GetCompletedCommandSerial());
    DAWN_ASSERT(mSlots[slot].typeId == tint::ResourceType::kEmpty);
    mSlots[slot].availableAfter = kMaxExecutionSerial;
    SetEntry(slot, contents);
}

void ResourceTableBase::Remove(ResourceTableSlot slot) {
    // Prevent all accesses to the slot which means it will be possible to update it once all
    // current GPU work is finished.
    mSlots[slot].availableAfter = GetDevice()->GetQueue()->GetLastSubmittedCommandSerial();

    // Set the entry to be empty, which will unlink previously set resources.
    BindingResource nothing = {};
    SetEntry(slot, &nothing);
}

void ResourceTableBase::UpdateWithDeviceValidation(ResourceTableSlot slot,
                                                   const BindingResource* resource,
                                                   std::string_view methodName) {
    // Perform validation that produces a validation error, but unconditionally mark the slot as
    // used since we need to match client-side validation that doesn't perform these checks.
    if (GetDevice()->ConsumedError(  //
            ([&]() -> MaybeError {
                DAWN_TRY(GetDevice()->ValidateObject(this));
                return ValidateBindingResource(GetDevice(), resource);
            })(),
            "validating %s.%s()", this, methodName)) {
        BindingResource nothing = {};
        Update(slot, &nothing);
    } else {
        Update(slot, resource);
    }
}

void ResourceTableBase::SetEntry(ResourceTableSlot slot, const BindingResource* contents) {
    // TODO(https://issues.chromium.org/473354063): Support resources that aren't TextureViews
    DAWN_ASSERT(contents->buffer == nullptr && contents->sampler == nullptr);
    TextureViewBase* view = contents->textureView;

    SlotState& state = mSlots[slot];
    if (state.resource == view) {
        return;
    }

    // Update the slot but also the mapping to the slot that are stored in the textures.
    if (state.resource != nullptr) {
        state.resource->GetTexture()->RemoveResourceTableSlotUse(this, slot);
    }
    if (view != nullptr) {
        view->GetTexture()->AddResourceTableSlotUse(this, slot);
    }
    state.resource = view;
    state.resourceDirty = true;

    // Update the slot with information for the updated resource.
    state.typeId = ComputeTypeId(view);
    state.pinned = view != nullptr && view->GetTexture()->HasPinnedUsage();

    MarkStateDirty(slot);
}

ResourceTableBase::Updates ResourceTableBase::AcquireDirtySlotUpdates() {
    DAWN_ASSERT(!mDestroyed);

    Updates updates;
    for (ResourceTableSlot dirtyIndex : mDirtySlots) {
        SlotState& state = mSlots[dirtyIndex];
        DAWN_ASSERT(state.dirty);
        state.dirty = false;

        tint::ResourceType effectiveType = state.pinned ? state.typeId : tint::ResourceType::kEmpty;

        // Add the update for the metadata buffer.
        size_t offset = sizeof(uint32_t) * (uint32_t(dirtyIndex) + 1);
        updates.metadataUpdates.push_back({
            .offset = uint32_t(offset),
            .data = uint32_t(effectiveType),
        });

        // Compute whether a resource update is needed and skip adding it if unnecessary.
        if (!state.resourceDirty) {
            continue;
        }
        state.resourceDirty = false;

        // Don't add updates for removing resources because the shader-side validation will prevent
        // accesses anyway.
        if (state.resource == nullptr) {
            continue;
        }

        updates.resourceUpdates.push_back({
            .slot = dirtyIndex,
            .textureView = state.resource.Get(),
        });
    }
    mDirtySlots.clear();

    return updates;
}

void ResourceTableBase::MarkStateDirty(ResourceTableSlot slot) {
    if (!mSlots[slot].dirty) {
        mDirtySlots.push_back(slot);
        mSlots[slot].dirty = true;
    }
}

// ResourceTableDefaultResources

ResultOrError<ityp::span<ResourceTableSlot, Ref<TextureViewBase>>>
ResourceTableDefaultResources::GetOrCreateSampledTextureDefaults(DeviceBase* device) {
    if (!mSampledTextureDefaults.empty()) {
        return {{mSampledTextureDefaults.data(), mSampledTextureDefaults.size()}};
    }

    auto AddDefaultResource = [&](TextureBase* texture,
                                  const TextureViewDescriptor* viewDesc = nullptr) -> MaybeError {
        DAWN_TRY(texture->Pin(wgpu::TextureUsage::TextureBinding));

        Ref<TextureViewBase> view;
        DAWN_TRY_ASSIGN(view, device->CreateTextureView(texture, viewDesc));

        // Check that the resource we will have will match the order of default textures that we
        // will give to the shader compilation.
        DAWN_ASSERT(ComputeTypeId(view.Get()) ==
                    GetDefaultResourceOrder()[mSampledTextureDefaults.size()]);
        mSampledTextureDefaults.push_back(view);

        return {};
    };

    // Create the color format single-sampled views.
    for (auto format :
         {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::R8Sint}) {
        // Create the necessary 1/2/3D textures.
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {1},
            .format = format,
        };

        tDesc.size = {1};
        tDesc.dimension = wgpu::TextureDimension::e1D;
        Ref<TextureBase> t1D;
        DAWN_TRY_ASSIGN(t1D, device->CreateTexture(&tDesc));

        tDesc.size = {1, 1, 6};
        tDesc.dimension = wgpu::TextureDimension::e2D;
        Ref<TextureBase> t2D;
        DAWN_TRY_ASSIGN(t2D, device->CreateTexture(&tDesc));

        tDesc.size = {1, 1, 1};
        tDesc.dimension = wgpu::TextureDimension::e3D;
        Ref<TextureBase> t3D;
        DAWN_TRY_ASSIGN(t3D, device->CreateTexture(&tDesc));

        // Create all the default binding view, reusing the 2D texture between
        // 2D/2DArray/Cube/CubeArray.
        DAWN_TRY(AddDefaultResource(t1D.Get()));

        TextureViewDescriptor vDesc{
            .label = "default SampledTexture resource",
        };
        vDesc.arrayLayerCount = 1;
        vDesc.dimension = wgpu::TextureViewDimension::e2D;
        DAWN_TRY(AddDefaultResource(t2D.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::e2DArray;
        DAWN_TRY(AddDefaultResource(t2D.Get(), &vDesc));
        vDesc.arrayLayerCount = 6;
        vDesc.dimension = wgpu::TextureViewDimension::Cube;
        DAWN_TRY(AddDefaultResource(t2D.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::CubeArray;
        DAWN_TRY(AddDefaultResource(t2D.Get(), &vDesc));

        DAWN_TRY(AddDefaultResource(t3D.Get()));
    }

    // Create the color format multi-sampled views.
    for (auto format :
         {wgpu::TextureFormat::R8Unorm, wgpu::TextureFormat::R8Uint, wgpu::TextureFormat::R8Sint}) {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1},
            .format = format,
            .sampleCount = 4,
        };

        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));
        DAWN_TRY(AddDefaultResource(t.Get()));
    }

    // Create the single-sampled depth texture default resource.
    {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1, 6},
            .format = wgpu::TextureFormat::Depth16Unorm,
        };
        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));

        TextureViewDescriptor vDesc{
            .label = "default SampledTexture resource",
        };
        vDesc.arrayLayerCount = 1;
        vDesc.dimension = wgpu::TextureViewDimension::e2D;
        DAWN_TRY(AddDefaultResource(t.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::e2DArray;
        DAWN_TRY(AddDefaultResource(t.Get(), &vDesc));
        vDesc.arrayLayerCount = 6;
        vDesc.dimension = wgpu::TextureViewDimension::Cube;
        DAWN_TRY(AddDefaultResource(t.Get(), &vDesc));
        vDesc.dimension = wgpu::TextureViewDimension::CubeArray;
        DAWN_TRY(AddDefaultResource(t.Get(), &vDesc));
    }

    // Create the multi-sampled depth texture default resource.
    {
        TextureDescriptor tDesc{
            .label = "default SampledTexture resource",
            .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment,
            .dimension = wgpu::TextureDimension::e2D,
            .size = {1, 1},
            .format = wgpu::TextureFormat::Depth16Unorm,
            .sampleCount = 4,
        };

        Ref<TextureBase> t;
        DAWN_TRY_ASSIGN(t, device->CreateTexture(&tDesc));
        DAWN_TRY(AddDefaultResource(t.Get()));
    }

    return {{mSampledTextureDefaults.data(), mSampledTextureDefaults.size()}};
}

}  // namespace dawn::native
