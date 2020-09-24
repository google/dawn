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
#include "common/HashUtils.h"
#include "dawn_native/Device.h"
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
            if (!format->supportsStorageUsage) {
                return DAWN_VALIDATION_ERROR("Texture format does not support storage textures");
            }

            return {};
        }

        MaybeError ValidateStorageTextureViewDimension(wgpu::TextureViewDimension dimension) {
            switch (dimension) {
                case wgpu::TextureViewDimension::Cube:
                case wgpu::TextureViewDimension::CubeArray:
                    return DAWN_VALIDATION_ERROR(
                        "Cube map and cube map texture views cannot be used as storage textures");

                case wgpu::TextureViewDimension::e1D:
                case wgpu::TextureViewDimension::e2D:
                case wgpu::TextureViewDimension::e2DArray:
                case wgpu::TextureViewDimension::e3D:
                    return {};

                case wgpu::TextureViewDimension::Undefined:
                    UNREACHABLE();
            }
        }
    }  // anonymous namespace

    MaybeError ValidateBindGroupLayoutDescriptor(DeviceBase* device,
                                                 const BindGroupLayoutDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }

        std::set<BindingNumber> bindingsSet;
        BindingCounts bindingCounts = {};
        for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
            const BindGroupLayoutEntry& entry = descriptor->entries[i];
            BindingNumber bindingNumber = BindingNumber(entry.binding);

            DAWN_TRY(ValidateShaderStage(entry.visibility));
            DAWN_TRY(ValidateBindingType(entry.type));
            DAWN_TRY(ValidateTextureComponentType(entry.textureComponentType));

            wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
            if (entry.viewDimension != wgpu::TextureViewDimension::Undefined) {
                DAWN_TRY(ValidateTextureViewDimension(entry.viewDimension));
                viewDimension = entry.viewDimension;
            }

            // Fixup multisampled=true to use MultisampledTexture instead.
            // TODO(dawn:527): Remove once the deprecation of multisampled is done.
            wgpu::BindingType type = entry.type;
            if (entry.multisampled) {
                if (type == wgpu::BindingType::MultisampledTexture) {
                    return DAWN_VALIDATION_ERROR(
                        "Cannot use multisampled = true and MultisampledTexture at the same time.");
                } else if (type == wgpu::BindingType::SampledTexture) {
                    device->EmitDeprecationWarning(
                        "BGLEntry::multisampled is deprecated, use "
                        "wgpu::BindingType::MultisampledTexture instead.");
                    type = wgpu::BindingType::MultisampledTexture;
                } else {
                    return DAWN_VALIDATION_ERROR("Binding type cannot be multisampled");
                }
            }

            if (bindingsSet.count(bindingNumber) != 0) {
                return DAWN_VALIDATION_ERROR("some binding index was specified more than once");
            }

            bool canBeDynamic = false;
            wgpu::ShaderStage allowedStages = kAllStages;

            switch (type) {
                case wgpu::BindingType::StorageBuffer:
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                    DAWN_FALLTHROUGH;
                case wgpu::BindingType::UniformBuffer:
                case wgpu::BindingType::ReadonlyStorageBuffer:
                    canBeDynamic = true;
                    break;

                case wgpu::BindingType::SampledTexture:
                    break;

                case wgpu::BindingType::MultisampledTexture:
                    if (viewDimension != wgpu::TextureViewDimension::e2D) {
                        return DAWN_VALIDATION_ERROR("Multisampled binding must be 2D.");
                    }
                    break;

                case wgpu::BindingType::WriteonlyStorageTexture:
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                    DAWN_FALLTHROUGH;
                case wgpu::BindingType::ReadonlyStorageTexture:
                    DAWN_TRY(ValidateStorageTextureFormat(device, entry.storageTextureFormat));
                    DAWN_TRY(ValidateStorageTextureViewDimension(viewDimension));
                    break;

                case wgpu::BindingType::Sampler:
                case wgpu::BindingType::ComparisonSampler:
                    break;
            }

            if (entry.hasDynamicOffset && !canBeDynamic) {
                return DAWN_VALIDATION_ERROR("Binding type cannot be dynamic.");
            }

            if ((entry.visibility & allowedStages) != entry.visibility) {
                return DAWN_VALIDATION_ERROR("Binding type cannot be used with this visibility.");
            }

            IncrementBindingCounts(&bindingCounts, entry);

            bindingsSet.insert(bindingNumber);
        }

        DAWN_TRY(ValidateBindingCounts(bindingCounts));

        return {};
    }

    namespace {

        void HashCombineBindingInfo(size_t* hash, const BindingInfo& info) {
            HashCombine(hash, info.hasDynamicOffset, info.visibility, info.type,
                        info.textureComponentType, info.viewDimension, info.storageTextureFormat,
                        info.minBufferBindingSize);
        }

        bool operator!=(const BindingInfo& a, const BindingInfo& b) {
            return a.hasDynamicOffset != b.hasDynamicOffset ||          //
                   a.visibility != b.visibility ||                      //
                   a.type != b.type ||                                  //
                   a.textureComponentType != b.textureComponentType ||  //
                   a.viewDimension != b.viewDimension ||                //
                   a.storageTextureFormat != b.storageTextureFormat ||  //
                   a.minBufferBindingSize != b.minBufferBindingSize;
        }

        bool IsBufferBinding(wgpu::BindingType bindingType) {
            switch (bindingType) {
                case wgpu::BindingType::UniformBuffer:
                case wgpu::BindingType::StorageBuffer:
                case wgpu::BindingType::ReadonlyStorageBuffer:
                    return true;
                case wgpu::BindingType::SampledTexture:
                case wgpu::BindingType::MultisampledTexture:
                case wgpu::BindingType::Sampler:
                case wgpu::BindingType::ComparisonSampler:
                case wgpu::BindingType::ReadonlyStorageTexture:
                case wgpu::BindingType::WriteonlyStorageTexture:
                    return false;
            }
        }

        bool SortBindingsCompare(const BindGroupLayoutEntry& a, const BindGroupLayoutEntry& b) {
            const bool aIsBuffer = IsBufferBinding(a.type);
            const bool bIsBuffer = IsBufferBinding(b.type);
            if (aIsBuffer != bIsBuffer) {
                // Always place buffers first.
                return aIsBuffer;
            } else {
                if (aIsBuffer) {
                    ASSERT(bIsBuffer);
                    if (a.hasDynamicOffset != b.hasDynamicOffset) {
                        // Buffers with dynamic offsets should come before those without.
                        // This makes it easy to iterate over the dynamic buffer bindings
                        // [0, dynamicBufferCount) during validation.
                        return a.hasDynamicOffset;
                    }
                    if (a.hasDynamicOffset) {
                        ASSERT(b.hasDynamicOffset);
                        ASSERT(a.binding != b.binding);
                        // Above, we ensured that dynamic buffers are first. Now, ensure that
                        // dynamic buffer bindings are in increasing order. This is because dynamic
                        // buffer offsets are applied in increasing order of binding number.
                        return a.binding < b.binding;
                    }
                }
                // Otherwise, sort by type.
                if (a.type != b.type) {
                    return a.type < b.type;
                }
            }
            if (a.visibility != b.visibility) {
                return a.visibility < b.visibility;
            }
            if (a.viewDimension != b.viewDimension) {
                return a.viewDimension < b.viewDimension;
            }
            if (a.textureComponentType != b.textureComponentType) {
                return a.textureComponentType < b.textureComponentType;
            }
            if (a.storageTextureFormat != b.storageTextureFormat) {
                return a.storageTextureFormat < b.storageTextureFormat;
            }
            if (a.minBufferBindingSize != b.minBufferBindingSize) {
                return a.minBufferBindingSize < b.minBufferBindingSize;
            }
            return false;
        }

        // This is a utility function to help ASSERT that the BGL-binding comparator places buffers
        // first.
        bool CheckBufferBindingsFirst(ityp::span<BindingIndex, const BindingInfo> bindings) {
            BindingIndex lastBufferIndex{0};
            BindingIndex firstNonBufferIndex = std::numeric_limits<BindingIndex>::max();
            for (BindingIndex i{0}; i < bindings.size(); ++i) {
                if (IsBufferBinding(bindings[i].type)) {
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
                                             const BindGroupLayoutDescriptor* descriptor)
        : CachedObject(device), mBindingInfo(BindingIndex(descriptor->entryCount)) {
        std::vector<BindGroupLayoutEntry> sortedBindings(
            descriptor->entries, descriptor->entries + descriptor->entryCount);

        // Fixup multisampled=true to use MultisampledTexture instead.
        // TODO(dawn:527): Remove once multisampled=true deprecation is finished.
        for (BindGroupLayoutEntry& entry : sortedBindings) {
            if (entry.multisampled) {
                ASSERT(entry.type == wgpu::BindingType::SampledTexture);
                entry.multisampled = false;
                entry.type = wgpu::BindingType::MultisampledTexture;
            }
        }

        std::sort(sortedBindings.begin(), sortedBindings.end(), SortBindingsCompare);

        for (BindingIndex i{0}; i < mBindingInfo.size(); ++i) {
            const BindGroupLayoutEntry& binding = sortedBindings[static_cast<uint32_t>(i)];
            mBindingInfo[i].binding = BindingNumber(binding.binding);
            mBindingInfo[i].type = binding.type;
            mBindingInfo[i].visibility = binding.visibility;
            mBindingInfo[i].textureComponentType =
                Format::TextureComponentTypeToFormatType(binding.textureComponentType);
            mBindingInfo[i].storageTextureFormat = binding.storageTextureFormat;
            mBindingInfo[i].minBufferBindingSize = binding.minBufferBindingSize;

            if (binding.viewDimension == wgpu::TextureViewDimension::Undefined) {
                mBindingInfo[i].viewDimension = wgpu::TextureViewDimension::e2D;
            } else {
                mBindingInfo[i].viewDimension = binding.viewDimension;
            }

            mBindingInfo[i].hasDynamicOffset = binding.hasDynamicOffset;

            if (IsBufferBinding(binding.type)) {
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

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device, ObjectBase::ErrorTag tag)
        : CachedObject(device, tag) {
    }

    BindGroupLayoutBase::~BindGroupLayoutBase() {
        // Do not uncache the actual cached object if we are a blueprint
        if (IsCachedReference()) {
            GetDevice()->UncacheBindGroupLayout(this);
        }
    }

    // static
    BindGroupLayoutBase* BindGroupLayoutBase::MakeError(DeviceBase* device) {
        return new BindGroupLayoutBase(device, ObjectBase::kError);
    }

    const BindGroupLayoutBase::BindingMap& BindGroupLayoutBase::GetBindingMap() const {
        ASSERT(!IsError());
        return mBindingMap;
    }

    BindingIndex BindGroupLayoutBase::GetBindingIndex(BindingNumber bindingNumber) const {
        ASSERT(!IsError());
        const auto& it = mBindingMap.find(bindingNumber);
        ASSERT(it != mBindingMap.end());
        return it->second;
    }

    size_t BindGroupLayoutBase::HashFunc::operator()(const BindGroupLayoutBase* bgl) const {
        size_t hash = 0;
        // std::map is sorted by key, so two BGLs constructed in different orders
        // will still hash the same.
        for (const auto& it : bgl->mBindingMap) {
            HashCombine(&hash, it.first, it.second);
            HashCombineBindingInfo(&hash, bgl->mBindingInfo[it.second]);
        }
        return hash;
    }

    bool BindGroupLayoutBase::EqualityFunc::operator()(const BindGroupLayoutBase* a,
                                                       const BindGroupLayoutBase* b) const {
        if (a->GetBindingCount() != b->GetBindingCount()) {
            return false;
        }
        for (BindingIndex i{0}; i < a->GetBindingCount(); ++i) {
            if (a->mBindingInfo[i] != b->mBindingInfo[i]) {
                return false;
            }
        }
        return a->mBindingMap == b->mBindingMap;
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

}  // namespace dawn_native
