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

#include "dawn/native/BindGroupLayoutInternal.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectBase.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/PerStage.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

namespace {
MaybeError ValidateStorageTextureFormat(DeviceBase* device,
                                        wgpu::TextureFormat storageTextureFormat) {
    const Format* format = nullptr;
    DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureFormat));

    DAWN_ASSERT(format != nullptr);
    DAWN_INVALID_IF(!format->supportsStorageUsage,
                    "Texture format (%s) does not support storage textures.", storageTextureFormat);

    return {};
}

MaybeError ValidateStorageTextureViewDimension(wgpu::TextureViewDimension dimension) {
    switch (dimension) {
        case wgpu::TextureViewDimension::Cube:
        case wgpu::TextureViewDimension::CubeArray:
            return DAWN_VALIDATION_ERROR("%s texture views cannot be used as storage textures.",
                                         dimension);

        case wgpu::TextureViewDimension::e1D:
        case wgpu::TextureViewDimension::e2D:
        case wgpu::TextureViewDimension::e2DArray:
        case wgpu::TextureViewDimension::e3D:
            return {};

        case wgpu::TextureViewDimension::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

MaybeError ValidateReadWriteStorageTextureAccess(
    DeviceBase* device,
    const StorageTextureBindingLayout& storageTextureBindingLayout) {
    switch (storageTextureBindingLayout.access) {
        case wgpu::StorageTextureAccess::ReadOnly:
        case wgpu::StorageTextureAccess::ReadWrite:
            if (!device->APIHasFeature(
                    wgpu::FeatureName::ChromiumExperimentalReadWriteStorageTexture)) {
                return DAWN_VALIDATION_ERROR(
                    "storage texture access %s cannot be used without feature "
                    "%s",
                    storageTextureBindingLayout.access,
                    wgpu::FeatureName::ChromiumExperimentalReadWriteStorageTexture);
            }
            break;

        case wgpu::StorageTextureAccess::WriteOnly:
            break;
        default:
            DAWN_UNREACHABLE();
    }

    if (storageTextureBindingLayout.access == wgpu::StorageTextureAccess::ReadWrite) {
        const Format* format = nullptr;
        DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureBindingLayout.format));
        DAWN_INVALID_IF(!format->supportsReadWriteStorageUsage,
                        "Texture format %s does not support storage texture access %s",
                        storageTextureBindingLayout.format, wgpu::StorageTextureAccess::ReadWrite);
    }

    return {};
}

MaybeError ValidateBindGroupLayoutEntry(DeviceBase* device,
                                        const BindGroupLayoutEntry& entry,
                                        bool allowInternalBinding) {
    DAWN_TRY(ValidateShaderStage(entry.visibility));

    int bindingMemberCount = 0;

    if (entry.buffer.type != wgpu::BufferBindingType::Undefined) {
        bindingMemberCount++;
        const BufferBindingLayout& buffer = entry.buffer;

        // The kInternalStorageBufferBinding is used internally and not a value
        // in wgpu::BufferBindingType.
        if (buffer.type == kInternalStorageBufferBinding) {
            DAWN_INVALID_IF(!allowInternalBinding, "Internal binding types are disallowed");
        } else {
            DAWN_TRY(ValidateBufferBindingType(buffer.type));
        }

        if (buffer.type == wgpu::BufferBindingType::Storage ||
            buffer.type == kInternalStorageBufferBinding) {
            DAWN_INVALID_IF(
                entry.visibility & wgpu::ShaderStage::Vertex,
                "Read-write storage buffer binding is used with a visibility (%s) that contains %s "
                "(note that read-only storage buffer bindings are allowed).",
                entry.visibility, wgpu::ShaderStage::Vertex);
        }
    }

    if (entry.sampler.type != wgpu::SamplerBindingType::Undefined) {
        bindingMemberCount++;
        DAWN_TRY(ValidateSamplerBindingType(entry.sampler.type));
    }

    if (entry.texture.sampleType != wgpu::TextureSampleType::Undefined) {
        bindingMemberCount++;
        const TextureBindingLayout& texture = entry.texture;
        // The kInternalResolveAttachmentSampleType is used internally and not a value
        // in wgpu::TextureSampleType.
        switch (texture.sampleType) {
            case kInternalResolveAttachmentSampleType:
                if (allowInternalBinding) {
                    break;
                }
                // should return validation error.
                [[fallthrough]];
            default:
                DAWN_TRY(ValidateTextureSampleType(texture.sampleType));
                break;
        }

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
        if (texture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            DAWN_TRY(ValidateTextureViewDimension(texture.viewDimension));
            viewDimension = texture.viewDimension;
        }

        DAWN_INVALID_IF(texture.multisampled && viewDimension != wgpu::TextureViewDimension::e2D,
                        "View dimension (%s) for a multisampled texture bindings was not %s.",
                        viewDimension, wgpu::TextureViewDimension::e2D);

        DAWN_INVALID_IF(
            texture.multisampled && texture.sampleType == wgpu::TextureSampleType::Float,
            "Sample type for multisampled texture binding was %s.", wgpu::TextureSampleType::Float);
    }

    if (entry.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
        bindingMemberCount++;
        const StorageTextureBindingLayout& storageTexture = entry.storageTexture;
        DAWN_TRY(ValidateStorageTextureAccess(storageTexture.access));
        DAWN_TRY(ValidateStorageTextureFormat(device, storageTexture.format));

        // viewDimension defaults to 2D if left undefined, needs validation otherwise.
        if (storageTexture.viewDimension != wgpu::TextureViewDimension::Undefined) {
            DAWN_TRY(ValidateTextureViewDimension(storageTexture.viewDimension));
            DAWN_TRY(ValidateStorageTextureViewDimension(storageTexture.viewDimension));
        }

        DAWN_TRY(ValidateReadWriteStorageTextureAccess(device, storageTexture));

        switch (storageTexture.access) {
            case wgpu::StorageTextureAccess::ReadOnly:
                break;
            case wgpu::StorageTextureAccess::ReadWrite:
            case wgpu::StorageTextureAccess::WriteOnly:
                DAWN_INVALID_IF(entry.visibility & wgpu::ShaderStage::Vertex,
                                "Storage texture binding with %s is used with a visibility (%s) "
                                "that contains %s.",
                                storageTexture.access, entry.visibility, wgpu::ShaderStage::Vertex);
                break;
            default:
                DAWN_UNREACHABLE();
        }
    }

    const ExternalTextureBindingLayout* externalTextureBindingLayout = nullptr;
    FindInChain(entry.nextInChain, &externalTextureBindingLayout);
    if (externalTextureBindingLayout != nullptr) {
        bindingMemberCount++;
    }

    DAWN_INVALID_IF(bindingMemberCount == 0,
                    "BindGroupLayoutEntry had none of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    DAWN_INVALID_IF(bindingMemberCount != 1,
                    "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                    "storageTexture, or externalTexture set");

    return {};
}

BindGroupLayoutEntry CreateSampledTextureBindingForExternalTexture(uint32_t binding,
                                                                   wgpu::ShaderStage visibility) {
    BindGroupLayoutEntry entry;
    entry.binding = binding;
    entry.visibility = visibility;
    entry.texture.viewDimension = wgpu::TextureViewDimension::e2D;
    entry.texture.multisampled = false;
    entry.texture.sampleType = wgpu::TextureSampleType::Float;
    return entry;
}

BindGroupLayoutEntry CreateUniformBindingForExternalTexture(uint32_t binding,
                                                            wgpu::ShaderStage visibility) {
    BindGroupLayoutEntry entry;
    entry.binding = binding;
    entry.visibility = visibility;
    entry.buffer.hasDynamicOffset = false;
    entry.buffer.type = wgpu::BufferBindingType::Uniform;
    return entry;
}

std::vector<BindGroupLayoutEntry> ExtractAndExpandBglEntries(
    const BindGroupLayoutDescriptor* descriptor,
    BindingCounts* bindingCounts,
    ExternalTextureBindingExpansionMap* externalTextureBindingExpansions) {
    std::vector<BindGroupLayoutEntry> expandedOutput;

    // When new bgl entries are created, we use binding numbers larger than
    // kMaxBindingsPerBindGroup to ensure there are no collisions.
    uint32_t nextOpenBindingNumberForNewEntry = kMaxBindingsPerBindGroup;
    for (uint32_t i = 0; i < descriptor->entryCount; i++) {
        const BindGroupLayoutEntry& entry = descriptor->entries[i];
        const ExternalTextureBindingLayout* externalTextureBindingLayout = nullptr;
        FindInChain(entry.nextInChain, &externalTextureBindingLayout);
        // External textures are expanded from a texture_external into two sampled texture
        // bindings and one uniform buffer binding. The original binding number is used
        // for the first sampled texture.
        if (externalTextureBindingLayout != nullptr) {
            for (SingleShaderStage stage : IterateStages(entry.visibility)) {
                // External textures are not fully implemented, which means that expanding
                // the external texture at this time will not occupy the same number of
                // binding slots as defined in the WebGPU specification. Here we prematurely
                // increment the binding counts for an additional sampled textures and a
                // sampler so that an external texture will occupy the correct number of
                // slots for correct validation of shader binding limits.
                // TODO(dawn:1082): Consider removing this and instead making a change to
                // the validation.
                constexpr uint32_t kUnimplementedSampledTexturesPerExternalTexture = 2;
                constexpr uint32_t kUnimplementedSamplersPerExternalTexture = 1;
                bindingCounts->perStage[stage].sampledTextureCount +=
                    kUnimplementedSampledTexturesPerExternalTexture;
                bindingCounts->perStage[stage].samplerCount +=
                    kUnimplementedSamplersPerExternalTexture;
            }

            dawn::native::ExternalTextureBindingExpansion bindingExpansion;

            BindGroupLayoutEntry plane0Entry =
                CreateSampledTextureBindingForExternalTexture(entry.binding, entry.visibility);
            bindingExpansion.plane0 = BindingNumber(plane0Entry.binding);
            expandedOutput.push_back(plane0Entry);

            BindGroupLayoutEntry plane1Entry = CreateSampledTextureBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry.visibility);
            bindingExpansion.plane1 = BindingNumber(plane1Entry.binding);
            expandedOutput.push_back(plane1Entry);

            BindGroupLayoutEntry paramsEntry = CreateUniformBindingForExternalTexture(
                nextOpenBindingNumberForNewEntry++, entry.visibility);
            bindingExpansion.params = BindingNumber(paramsEntry.binding);
            expandedOutput.push_back(paramsEntry);

            externalTextureBindingExpansions->insert(
                {BindingNumber(entry.binding), bindingExpansion});
        } else {
            expandedOutput.push_back(entry);
        }
    }

    return expandedOutput;
}
}  // anonymous namespace

MaybeError ValidateBindGroupLayoutDescriptor(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             bool allowInternalBinding) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr");

    std::set<BindingNumber> bindingsSet;
    BindingCounts bindingCounts = {};

    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupLayoutEntry& entry = descriptor->entries[i];
        BindingNumber bindingNumber = BindingNumber(entry.binding);

        DAWN_INVALID_IF(bindingNumber >= kMaxBindingsPerBindGroupTyped,
                        "Binding number (%u) exceeds the maxBindingsPerBindGroup limit (%u).",
                        uint32_t(bindingNumber), kMaxBindingsPerBindGroup);
        DAWN_INVALID_IF(bindingsSet.count(bindingNumber) != 0,
                        "On entries[%u]: binding index (%u) was specified by a previous entry.", i,
                        entry.binding);

        DAWN_TRY_CONTEXT(ValidateBindGroupLayoutEntry(device, entry, allowInternalBinding),
                         "validating entries[%u]", i);

        IncrementBindingCounts(&bindingCounts, entry);

        bindingsSet.insert(bindingNumber);
    }

    DAWN_TRY_CONTEXT(ValidateBindingCounts(device->GetLimits(), bindingCounts),
                     "validating binding counts");

    return {};
}

namespace {

bool operator!=(const BindingInfo& a, const BindingInfo& b) {
    if (a.visibility != b.visibility || a.bindingType != b.bindingType) {
        return true;
    }

    switch (a.bindingType) {
        case BindingInfoType::Buffer:
            return a.buffer.type != b.buffer.type ||
                   a.buffer.hasDynamicOffset != b.buffer.hasDynamicOffset ||
                   a.buffer.minBindingSize != b.buffer.minBindingSize;
        case BindingInfoType::Sampler:
            return a.sampler.type != b.sampler.type;
        case BindingInfoType::Texture:
            return a.texture.sampleType != b.texture.sampleType ||
                   a.texture.viewDimension != b.texture.viewDimension ||
                   a.texture.multisampled != b.texture.multisampled;
        case BindingInfoType::StorageTexture:
            return a.storageTexture.access != b.storageTexture.access ||
                   a.storageTexture.viewDimension != b.storageTexture.viewDimension ||
                   a.storageTexture.format != b.storageTexture.format;
        case BindingInfoType::ExternalTexture:
            return false;
    }
    DAWN_UNREACHABLE();
}

bool IsBufferBinding(const BindGroupLayoutEntry& binding) {
    return binding.buffer.type != wgpu::BufferBindingType::Undefined;
}

bool BindingHasDynamicOffset(const BindGroupLayoutEntry& binding) {
    if (binding.buffer.type != wgpu::BufferBindingType::Undefined) {
        return binding.buffer.hasDynamicOffset;
    }
    return false;
}

BindingInfo CreateBindGroupLayoutInfo(const BindGroupLayoutEntry& binding) {
    BindingInfo bindingInfo;
    bindingInfo.binding = BindingNumber(binding.binding);
    bindingInfo.visibility = binding.visibility;

    if (binding.buffer.type != wgpu::BufferBindingType::Undefined) {
        bindingInfo.bindingType = BindingInfoType::Buffer;
        bindingInfo.buffer = binding.buffer;
    } else if (binding.sampler.type != wgpu::SamplerBindingType::Undefined) {
        bindingInfo.bindingType = BindingInfoType::Sampler;
        bindingInfo.sampler = binding.sampler;
    } else if (binding.texture.sampleType != wgpu::TextureSampleType::Undefined) {
        bindingInfo.bindingType = BindingInfoType::Texture;
        bindingInfo.texture = binding.texture;

        if (binding.texture.viewDimension == wgpu::TextureViewDimension::Undefined) {
            bindingInfo.texture.viewDimension = wgpu::TextureViewDimension::e2D;
        }
    } else if (binding.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
        bindingInfo.bindingType = BindingInfoType::StorageTexture;
        bindingInfo.storageTexture = binding.storageTexture;

        if (binding.storageTexture.viewDimension == wgpu::TextureViewDimension::Undefined) {
            bindingInfo.storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
        }
    } else {
        const ExternalTextureBindingLayout* externalTextureBindingLayout = nullptr;
        FindInChain(binding.nextInChain, &externalTextureBindingLayout);
        if (externalTextureBindingLayout != nullptr) {
            bindingInfo.bindingType = BindingInfoType::ExternalTexture;
        }
    }

    return bindingInfo;
}

bool SortBindingsCompare(const BindGroupLayoutEntry& a, const BindGroupLayoutEntry& b) {
    if (&a == &b) {
        return false;
    }

    const bool aIsBuffer = IsBufferBinding(a);
    const bool bIsBuffer = IsBufferBinding(b);
    if (aIsBuffer != bIsBuffer) {
        // Always place buffers first.
        return aIsBuffer;
    }

    if (aIsBuffer) {
        bool aHasDynamicOffset = BindingHasDynamicOffset(a);
        bool bHasDynamicOffset = BindingHasDynamicOffset(b);
        DAWN_ASSERT(bIsBuffer);
        if (aHasDynamicOffset != bHasDynamicOffset) {
            // Buffers with dynamic offsets should come before those without.
            // This makes it easy to iterate over the dynamic buffer bindings
            // [0, dynamicBufferCount) during validation.
            return aHasDynamicOffset;
        }
        if (aHasDynamicOffset) {
            DAWN_ASSERT(bHasDynamicOffset);
            DAWN_ASSERT(a.binding != b.binding);
            // Above, we ensured that dynamic buffers are first. Now, ensure that
            // dynamic buffer bindings are in increasing order. This is because dynamic
            // buffer offsets are applied in increasing order of binding number.
            return a.binding < b.binding;
        }
    }

    // This applies some defaults and gives us a single value to check for the binding type.
    BindingInfo aInfo = CreateBindGroupLayoutInfo(a);
    BindingInfo bInfo = CreateBindGroupLayoutInfo(b);

    // Sort by type.
    if (aInfo.bindingType != bInfo.bindingType) {
        return aInfo.bindingType < bInfo.bindingType;
    }

    if (a.visibility != b.visibility) {
        return a.visibility < b.visibility;
    }

    switch (aInfo.bindingType) {
        case BindingInfoType::Buffer:
            if (aInfo.buffer.minBindingSize != bInfo.buffer.minBindingSize) {
                return aInfo.buffer.minBindingSize < bInfo.buffer.minBindingSize;
            }
            break;
        case BindingInfoType::Sampler:
            if (aInfo.sampler.type != bInfo.sampler.type) {
                return aInfo.sampler.type < bInfo.sampler.type;
            }
            break;
        case BindingInfoType::Texture:
            if (aInfo.texture.multisampled != bInfo.texture.multisampled) {
                return aInfo.texture.multisampled < bInfo.texture.multisampled;
            }
            if (aInfo.texture.viewDimension != bInfo.texture.viewDimension) {
                return aInfo.texture.viewDimension < bInfo.texture.viewDimension;
            }
            if (aInfo.texture.sampleType != bInfo.texture.sampleType) {
                return aInfo.texture.sampleType < bInfo.texture.sampleType;
            }
            break;
        case BindingInfoType::StorageTexture:
            if (aInfo.storageTexture.access != bInfo.storageTexture.access) {
                return aInfo.storageTexture.access < bInfo.storageTexture.access;
            }
            if (aInfo.storageTexture.viewDimension != bInfo.storageTexture.viewDimension) {
                return aInfo.storageTexture.viewDimension < bInfo.storageTexture.viewDimension;
            }
            if (aInfo.storageTexture.format != bInfo.storageTexture.format) {
                return aInfo.storageTexture.format < bInfo.storageTexture.format;
            }
            break;
        case BindingInfoType::ExternalTexture:
            break;
    }
    return a.binding < b.binding;
}

// This is a utility function to help DAWN_ASSERT that the BGL-binding comparator places buffers
// first.
bool CheckBufferBindingsFirst(ityp::span<BindingIndex, const BindingInfo> bindings) {
    BindingIndex lastBufferIndex{0};
    BindingIndex firstNonBufferIndex = std::numeric_limits<BindingIndex>::max();
    for (BindingIndex i{0}; i < bindings.size(); ++i) {
        if (bindings[i].bindingType == BindingInfoType::Buffer) {
            lastBufferIndex = std::max(i, lastBufferIndex);
        } else {
            firstNonBufferIndex = std::min(i, firstNonBufferIndex);
        }
    }

    // If there are no buffers, then |lastBufferIndex| is initialized to 0 and
    // |firstNonBufferIndex| gets set to 0.
    return firstNonBufferIndex >= lastBufferIndex;
}

}  // namespace

// BindGroupLayoutInternalBase

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptor,
    ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label), mUnexpandedBindingCount(descriptor->entryCount) {
    std::vector<BindGroupLayoutEntry> sortedBindings = ExtractAndExpandBglEntries(
        descriptor, &mBindingCounts, &mExternalTextureBindingExpansionMap);

    std::sort(sortedBindings.begin(), sortedBindings.end(), SortBindingsCompare);

    for (uint32_t i = 0; i < sortedBindings.size(); ++i) {
        const BindGroupLayoutEntry& binding = sortedBindings[static_cast<uint32_t>(i)];

        mBindingInfo.push_back(CreateBindGroupLayoutInfo(binding));

        if (IsBufferBinding(binding)) {
            // Buffers must be contiguously packed at the start of the binding info.
            DAWN_ASSERT(GetBufferCount() == BindingIndex(i));
        }
        IncrementBindingCounts(&mBindingCounts, binding);

        const auto& [_, inserted] = mBindingMap.emplace(BindingNumber(binding.binding), i);
        DAWN_ASSERT(inserted);
    }
    DAWN_ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));
    DAWN_ASSERT(mBindingInfo.size() <= kMaxBindingsPerPipelineLayoutTyped);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(
    DeviceBase* device,
    const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutInternalBase::BindGroupLayoutInternalBase(DeviceBase* device,
                                                         ObjectBase::ErrorTag tag,
                                                         const char* label)
    : ApiObjectBase(device, tag, label) {}

BindGroupLayoutInternalBase::~BindGroupLayoutInternalBase() = default;

void BindGroupLayoutInternalBase::DestroyImpl() {
    Uncache();
}

ObjectType BindGroupLayoutInternalBase::GetType() const {
    return ObjectType::BindGroupLayout;
}

const BindingInfo& BindGroupLayoutInternalBase::GetBindingInfo(BindingIndex bindingIndex) const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(bindingIndex < mBindingInfo.size());
    return mBindingInfo[bindingIndex];
}

const BindGroupLayoutInternalBase::BindingMap& BindGroupLayoutInternalBase::GetBindingMap() const {
    DAWN_ASSERT(!IsError());
    return mBindingMap;
}

bool BindGroupLayoutInternalBase::HasBinding(BindingNumber bindingNumber) const {
    return mBindingMap.count(bindingNumber) != 0;
}

BindingIndex BindGroupLayoutInternalBase::GetBindingIndex(BindingNumber bindingNumber) const {
    DAWN_ASSERT(!IsError());
    const auto& it = mBindingMap.find(bindingNumber);
    DAWN_ASSERT(it != mBindingMap.end());
    return it->second;
}

size_t BindGroupLayoutInternalBase::ComputeContentHash() {
    ObjectContentHasher recorder;

    // std::map is sorted by key, so two BGLs constructed in different orders
    // will still record the same.
    for (const auto [id, index] : mBindingMap) {
        recorder.Record(id, index);

        const BindingInfo& info = mBindingInfo[index];
        recorder.Record(info.buffer.hasDynamicOffset, info.visibility, info.bindingType,
                        info.buffer.type, info.buffer.minBindingSize, info.sampler.type,
                        info.texture.sampleType, info.texture.viewDimension,
                        info.texture.multisampled, info.storageTexture.access,
                        info.storageTexture.format, info.storageTexture.viewDimension);
    }

    return recorder.GetContentHash();
}

bool BindGroupLayoutInternalBase::EqualityFunc::operator()(
    const BindGroupLayoutInternalBase* a,
    const BindGroupLayoutInternalBase* b) const {
    return a->IsLayoutEqual(b);
}

BindingIndex BindGroupLayoutInternalBase::GetBindingCount() const {
    return mBindingInfo.size();
}

BindingIndex BindGroupLayoutInternalBase::GetBufferCount() const {
    return BindingIndex(mBindingCounts.bufferCount);
}

BindingIndex BindGroupLayoutInternalBase::GetDynamicBufferCount() const {
    // This is a binding index because dynamic buffers are packed at the front of the binding
    // info.
    return static_cast<BindingIndex>(mBindingCounts.dynamicStorageBufferCount +
                                     mBindingCounts.dynamicUniformBufferCount);
}

uint32_t BindGroupLayoutInternalBase::GetUnverifiedBufferCount() const {
    return mBindingCounts.unverifiedBufferCount;
}

uint32_t BindGroupLayoutInternalBase::GetExternalTextureBindingCount() const {
    return mExternalTextureBindingExpansionMap.size();
}

const BindingCounts& BindGroupLayoutInternalBase::GetBindingCountInfo() const {
    return mBindingCounts;
}

const ExternalTextureBindingExpansionMap&
BindGroupLayoutInternalBase::GetExternalTextureBindingExpansionMap() const {
    return mExternalTextureBindingExpansionMap;
}

uint32_t BindGroupLayoutInternalBase::GetUnexpandedBindingCount() const {
    return mUnexpandedBindingCount;
}

bool BindGroupLayoutInternalBase::IsLayoutEqual(const BindGroupLayoutInternalBase* other) const {
    if (GetBindingCount() != other->GetBindingCount()) {
        return false;
    }
    for (BindingIndex i{0}; i < GetBindingCount(); ++i) {
        if (mBindingInfo[i] != other->mBindingInfo[i]) {
            return false;
        }
    }
    return mBindingMap == other->mBindingMap;
}

size_t BindGroupLayoutInternalBase::GetBindingDataSize() const {
    // | ------ buffer-specific ----------| ------------ object pointers -------------|
    // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
    // Followed by:
    // |---------buffer size array--------|
    // |-uint64_t[mUnverifiedBufferCount]-|
    size_t objectPointerStart = mBindingCounts.bufferCount * sizeof(BufferBindingData);
    DAWN_ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
    size_t bufferSizeArrayStart = Align(
        objectPointerStart + mBindingCounts.totalCount * sizeof(Ref<ObjectBase>), sizeof(uint64_t));
    DAWN_ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
    return bufferSizeArrayStart + mBindingCounts.unverifiedBufferCount * sizeof(uint64_t);
}

BindGroupLayoutInternalBase::BindingDataPointers
BindGroupLayoutInternalBase::ComputeBindingDataPointers(void* dataStart) const {
    BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
    auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + mBindingCounts.bufferCount);
    uint64_t* unverifiedBufferSizes = AlignPtr(
        reinterpret_cast<uint64_t*>(bindings + mBindingCounts.totalCount), sizeof(uint64_t));

    DAWN_ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
    DAWN_ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
    DAWN_ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

    return {{bufferData, GetBufferCount()},
            {bindings, GetBindingCount()},
            {unverifiedBufferSizes, mBindingCounts.unverifiedBufferCount}};
}

bool BindGroupLayoutInternalBase::IsStorageBufferBinding(BindingIndex bindingIndex) const {
    DAWN_ASSERT(bindingIndex < GetBufferCount());
    switch (GetBindingInfo(bindingIndex).buffer.type) {
        case wgpu::BufferBindingType::Uniform:
            return false;
        case kInternalStorageBufferBinding:
        case wgpu::BufferBindingType::Storage:
        case wgpu::BufferBindingType::ReadOnlyStorage:
            return true;
        case wgpu::BufferBindingType::Undefined:
            break;
    }
    DAWN_UNREACHABLE();
}

std::string BindGroupLayoutInternalBase::EntriesToString() const {
    std::string entries = "[";
    std::string sep = "";
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = GetBindingMap();
    for (const auto [bindingNumber, bindingIndex] : bindingMap) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        entries += absl::StrFormat("%s%s", sep, bindingInfo);
        sep = ", ";
    }
    entries += "]";
    return entries;
}

}  // namespace dawn::native
