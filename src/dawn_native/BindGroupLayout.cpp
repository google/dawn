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

#include "dawn_native/BindGroupLayout.h"

#include "common/BitSetIterator.h"

#include "dawn_native/ChainUtils_autogen.h"
#include "dawn_native/Device.h"
#include "dawn_native/ObjectBase.h"
#include "dawn_native/ObjectContentHasher.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/PerStage.h"
#include "dawn_native/ValidationUtils_autogen.h"

#include <algorithm>
#include <functional>
#include <set>

namespace dawn_native {

    namespace {
        MaybeError ValidateStorageTextureFormat(DeviceBase* device,
                                                wgpu::TextureFormat storageTextureFormat) {
            const Format* format = nullptr;
            DAWN_TRY_ASSIGN(format, device->GetInternalFormat(storageTextureFormat));

            ASSERT(format != nullptr);
            DAWN_INVALID_IF(!format->supportsStorageUsage,
                            "Texture format (%s) does not support storage textures.",
                            storageTextureFormat);

            return {};
        }

        MaybeError ValidateStorageTextureViewDimension(wgpu::TextureViewDimension dimension) {
            switch (dimension) {
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    return DAWN_FORMAT_VALIDATION_ERROR(
                        "%s texture views cannot be used as storage textures.", dimension);

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e2DArray:
                case wgpu::TextureViewDimension::e3D:
                    return {};

                case wgpu::TextureViewDimension::Undefined:
                    break;
            }
            UNREACHABLE();
        }

        MaybeError ValidateBindGroupLayoutEntry(DeviceBase* device,
                                                const BindGroupLayoutEntry& entry,
                                                bool allowInternalBinding) {
            DAWN_TRY(ValidateShaderStage(entry.visibility));

            int bindingMemberCount = 0;
            BindingInfoType bindingType;
            wgpu::ShaderStage allowedStages = kAllStages;

            if (entry.buffer.type != wgpu::BufferBindingType::Undefined) {
                bindingMemberCount++;
                bindingType = BindingInfoType::Buffer;
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
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                }
            }
            if (entry.sampler.type != wgpu::SamplerBindingType::Undefined) {
                bindingMemberCount++;
                bindingType = BindingInfoType::Sampler;
                DAWN_TRY(ValidateSamplerBindingType(entry.sampler.type));
            }
            if (entry.texture.sampleType != wgpu::TextureSampleType::Undefined) {
                bindingMemberCount++;
                bindingType = BindingInfoType::Texture;
                const TextureBindingLayout& texture = entry.texture;
                DAWN_TRY(ValidateTextureSampleType(texture.sampleType));

                // viewDimension defaults to 2D if left undefined, needs validation otherwise.
                wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
                if (texture.viewDimension != wgpu::TextureViewDimension::Undefined) {
                    DAWN_TRY(ValidateTextureViewDimension(texture.viewDimension));
                    viewDimension = texture.viewDimension;
                }

                DAWN_INVALID_IF(
                    texture.multisampled && viewDimension != wgpu::TextureViewDimension::e2D,
                    "View dimension (%s) for a multisampled texture bindings was not %s.",
                    viewDimension, wgpu::TextureViewDimension::e2D);
            }
            if (entry.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
                bindingMemberCount++;
                bindingType = BindingInfoType::StorageTexture;
                const StorageTextureBindingLayout& storageTexture = entry.storageTexture;
                DAWN_TRY(ValidateStorageTextureAccess(storageTexture.access));
                DAWN_TRY(ValidateStorageTextureFormat(device, storageTexture.format));

                // viewDimension defaults to 2D if left undefined, needs validation otherwise.
                if (storageTexture.viewDimension != wgpu::TextureViewDimension::Undefined) {
                    DAWN_TRY(ValidateTextureViewDimension(storageTexture.viewDimension));
                    DAWN_TRY(ValidateStorageTextureViewDimension(storageTexture.viewDimension));
                }

                if (storageTexture.access == wgpu::StorageTextureAccess::WriteOnly) {
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                }
            }

            const ExternalTextureBindingLayout* externalTextureBindingLayout = nullptr;
            FindInChain(entry.nextInChain, &externalTextureBindingLayout);
            if (externalTextureBindingLayout != nullptr) {
                bindingMemberCount++;
                bindingType = BindingInfoType::ExternalTexture;
            }

            DAWN_INVALID_IF(bindingMemberCount != 1,
                            "BindGroupLayoutEntry had more than one of buffer, sampler, texture, "
                            "storageTexture, or externalTexture set");

            DAWN_INVALID_IF(
                !IsSubset(entry.visibility, allowedStages),
                "%s bindings cannot be used with a visibility of %s. Only %s are allowed.",
                bindingType, entry.visibility, allowedStages);

            return {};
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

            DAWN_INVALID_IF(bindingsSet.count(bindingNumber) != 0,
                            "On entries[%u]: binding index (%u) was specified by a previous entry.",
                            i, entry.binding);

            DAWN_TRY_CONTEXT(ValidateBindGroupLayoutEntry(device, entry, allowInternalBinding),
                             "validating entries[%u]", i);

            IncrementBindingCounts(&bindingCounts, entry);

            bindingsSet.insert(bindingNumber);
        }

        DAWN_TRY_CONTEXT(ValidateBindingCounts(bindingCounts), "validating binding counts");

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
            UNREACHABLE();
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
            const bool aIsBuffer = IsBufferBinding(a);
            const bool bIsBuffer = IsBufferBinding(b);
            if (aIsBuffer != bIsBuffer) {
                // Always place buffers first.
                return aIsBuffer;
            }

            if (aIsBuffer) {
                bool aHasDynamicOffset = BindingHasDynamicOffset(a);
                bool bHasDynamicOffset = BindingHasDynamicOffset(b);
                ASSERT(bIsBuffer);
                if (aHasDynamicOffset != bHasDynamicOffset) {
                    // Buffers with dynamic offsets should come before those without.
                    // This makes it easy to iterate over the dynamic buffer bindings
                    // [0, dynamicBufferCount) during validation.
                    return aHasDynamicOffset;
                }
                if (aHasDynamicOffset) {
                    ASSERT(bHasDynamicOffset);
                    ASSERT(a.binding != b.binding);
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
                        return aInfo.storageTexture.viewDimension <
                               bInfo.storageTexture.viewDimension;
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

        // This is a utility function to help ASSERT that the BGL-binding comparator places buffers
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

    // BindGroupLayoutBase

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             PipelineCompatibilityToken pipelineCompatibilityToken,
                                             ApiObjectBase::UntrackedByDeviceTag tag)
        : ApiObjectBase(device, descriptor->label),
          mBindingInfo(BindingIndex(descriptor->entryCount)),
          mPipelineCompatibilityToken(pipelineCompatibilityToken) {
        std::vector<BindGroupLayoutEntry> sortedBindings(
            descriptor->entries, descriptor->entries + descriptor->entryCount);

        std::sort(sortedBindings.begin(), sortedBindings.end(), SortBindingsCompare);

        for (BindingIndex i{0}; i < mBindingInfo.size(); ++i) {
            const BindGroupLayoutEntry& binding = sortedBindings[static_cast<uint32_t>(i)];

            mBindingInfo[i] = CreateBindGroupLayoutInfo(binding);

            if (IsBufferBinding(binding)) {
                // Buffers must be contiguously packed at the start of the binding info.
                ASSERT(GetBufferCount() == i);
            }
            IncrementBindingCounts(&mBindingCounts, binding);

            const auto& it = mBindingMap.emplace(BindingNumber(binding.binding), i);
            ASSERT(it.second);
        }
        ASSERT(CheckBufferBindingsFirst({mBindingInfo.data(), GetBindingCount()}));
        ASSERT(mBindingInfo.size() <= kMaxBindingsPerPipelineLayoutTyped);
    }

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor,
                                             PipelineCompatibilityToken pipelineCompatibilityToken)
        : BindGroupLayoutBase(device, descriptor, pipelineCompatibilityToken, kUntrackedByDevice) {
        TrackInDevice();
    }

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : ApiObjectBase(device, tag) {
    }

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device)
        : ApiObjectBase(device, kLabelNotImplemented) {
        TrackInDevice();
    }

    BindGroupLayoutBase::~BindGroupLayoutBase() = default;

    void BindGroupLayoutBase::DestroyImpl() {
        if (IsCachedReference()) {
            // Do not uncache the actual cached object if we are a blueprint.
            GetDevice()->UncacheBindGroupLayout(this);
        }
    }

    // static
    BindGroupLayoutBase* BindGroupLayoutBase::MakeError(DeviceBase* device) {
        return new BindGroupLayoutBase(device, ObjectBase::kError);
    }

    ObjectType BindGroupLayoutBase::GetType() const {
        return ObjectType::BindGroupLayout;
    }

    const BindGroupLayoutBase::BindingMap& BindGroupLayoutBase::GetBindingMap() const {
        ASSERT(!IsError());
        return mBindingMap;
    }

    bool BindGroupLayoutBase::HasBinding(BindingNumber bindingNumber) const {
        return mBindingMap.count(bindingNumber) != 0;
    }

    BindingIndex BindGroupLayoutBase::GetBindingIndex(BindingNumber bindingNumber) const {
        ASSERT(!IsError());
        const auto& it = mBindingMap.find(bindingNumber);
        ASSERT(it != mBindingMap.end());
        return it->second;
    }

    size_t BindGroupLayoutBase::ComputeContentHash() {
        ObjectContentHasher recorder;
        recorder.Record(mPipelineCompatibilityToken);

        // std::map is sorted by key, so two BGLs constructed in different orders
        // will still record the same.
        for (const auto& it : mBindingMap) {
            recorder.Record(it.first, it.second);

            const BindingInfo& info = mBindingInfo[it.second];

            recorder.Record(info.buffer.hasDynamicOffset, info.visibility, info.bindingType,
                            info.buffer.type, info.buffer.minBindingSize, info.sampler.type,
                            info.texture.sampleType, info.texture.viewDimension,
                            info.texture.multisampled, info.storageTexture.access,
                            info.storageTexture.format, info.storageTexture.viewDimension);
        }

        return recorder.GetContentHash();
    }

    bool BindGroupLayoutBase::EqualityFunc::operator()(const BindGroupLayoutBase* a,
                                                       const BindGroupLayoutBase* b) const {
        return a->IsLayoutEqual(b);
    }

    BindingIndex BindGroupLayoutBase::GetBindingCount() const {
        return mBindingInfo.size();
    }

    BindingIndex BindGroupLayoutBase::GetBufferCount() const {
        return BindingIndex(mBindingCounts.bufferCount);
    }

    BindingIndex BindGroupLayoutBase::GetDynamicBufferCount() const {
        // This is a binding index because dynamic buffers are packed at the front of the binding
        // info.
        return static_cast<BindingIndex>(mBindingCounts.dynamicStorageBufferCount +
                                         mBindingCounts.dynamicUniformBufferCount);
    }

    uint32_t BindGroupLayoutBase::GetUnverifiedBufferCount() const {
        return mBindingCounts.unverifiedBufferCount;
    }

    const BindingCounts& BindGroupLayoutBase::GetBindingCountInfo() const {
        return mBindingCounts;
    }

    bool BindGroupLayoutBase::IsLayoutEqual(const BindGroupLayoutBase* other,
                                            bool excludePipelineCompatibiltyToken) const {
        if (!excludePipelineCompatibiltyToken &&
            GetPipelineCompatibilityToken() != other->GetPipelineCompatibilityToken()) {
            return false;
        }
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

    PipelineCompatibilityToken BindGroupLayoutBase::GetPipelineCompatibilityToken() const {
        return mPipelineCompatibilityToken;
    }

    size_t BindGroupLayoutBase::GetBindingDataSize() const {
        // | ------ buffer-specific ----------| ------------ object pointers -------------|
        // | --- offsets + sizes -------------| --------------- Ref<ObjectBase> ----------|
        // Followed by:
        // |---------buffer size array--------|
        // |-uint64_t[mUnverifiedBufferCount]-|
        size_t objectPointerStart = mBindingCounts.bufferCount * sizeof(BufferBindingData);
        ASSERT(IsAligned(objectPointerStart, alignof(Ref<ObjectBase>)));
        size_t bufferSizeArrayStart =
            Align(objectPointerStart + mBindingCounts.totalCount * sizeof(Ref<ObjectBase>),
                  sizeof(uint64_t));
        ASSERT(IsAligned(bufferSizeArrayStart, alignof(uint64_t)));
        return bufferSizeArrayStart + mBindingCounts.unverifiedBufferCount * sizeof(uint64_t);
    }

    BindGroupLayoutBase::BindingDataPointers BindGroupLayoutBase::ComputeBindingDataPointers(
        void* dataStart) const {
        BufferBindingData* bufferData = reinterpret_cast<BufferBindingData*>(dataStart);
        auto bindings = reinterpret_cast<Ref<ObjectBase>*>(bufferData + mBindingCounts.bufferCount);
        uint64_t* unverifiedBufferSizes = AlignPtr(
            reinterpret_cast<uint64_t*>(bindings + mBindingCounts.totalCount), sizeof(uint64_t));

        ASSERT(IsPtrAligned(bufferData, alignof(BufferBindingData)));
        ASSERT(IsPtrAligned(bindings, alignof(Ref<ObjectBase>)));
        ASSERT(IsPtrAligned(unverifiedBufferSizes, alignof(uint64_t)));

        return {{bufferData, GetBufferCount()},
                {bindings, GetBindingCount()},
                {unverifiedBufferSizes, mBindingCounts.unverifiedBufferCount}};
    }

    bool BindGroupLayoutBase::IsStorageBufferBinding(BindingIndex bindingIndex) const {
        ASSERT(bindingIndex < GetBufferCount());
        switch (GetBindingInfo(bindingIndex).buffer.type) {
            case wgpu::BufferBindingType::Uniform:
                return false;
            case kInternalStorageBufferBinding:
            case wgpu::BufferBindingType::Storage:
            case wgpu::BufferBindingType::ReadOnlyStorage:
                return true;
            case wgpu::BufferBindingType::Undefined:
                UNREACHABLE();
        }
    }

}  // namespace dawn_native
