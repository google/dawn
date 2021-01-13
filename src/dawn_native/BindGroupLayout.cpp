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
#include "dawn_native/Device.h"
#include "dawn_native/ObjectContentHasher.h"
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

            if (bindingsSet.count(bindingNumber) != 0) {
                return DAWN_VALIDATION_ERROR("some binding index was specified more than once");
            }

            DAWN_TRY(ValidateShaderStage(entry.visibility));

            int bindingMemberCount = 0;
            wgpu::ShaderStage allowedStages = kAllStages;

            if (entry.buffer.type != wgpu::BufferBindingType::Undefined) {
                bindingMemberCount++;
                const BufferBindingLayout& buffer = entry.buffer;
                DAWN_TRY(ValidateBufferBindingType(buffer.type));

                if (buffer.type == wgpu::BufferBindingType::Storage) {
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                }

                // Dynamic storage buffers aren't bounds checked properly in D3D12. Disallow them as
                // unsafe until the bounds checks are implemented.
                if (device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs) &&
                    buffer.hasDynamicOffset &&
                    (buffer.type == wgpu::BufferBindingType::Storage ||
                     buffer.type == wgpu::BufferBindingType::ReadOnlyStorage)) {
                    return DAWN_VALIDATION_ERROR(
                        "Dynamic storage buffers are disallowed because they aren't secure yet. "
                        "See https://crbug.com/dawn/429");
                }
            }
            if (entry.sampler.type != wgpu::SamplerBindingType::Undefined) {
                bindingMemberCount++;
                DAWN_TRY(ValidateSamplerBindingType(entry.sampler.type));
            }
            if (entry.texture.sampleType != wgpu::TextureSampleType::Undefined) {
                bindingMemberCount++;
                const TextureBindingLayout& texture = entry.texture;
                DAWN_TRY(ValidateTextureSampleType(texture.sampleType));

                // viewDimension defaults to 2D if left undefined, needs validation otherwise.
                wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
                if (texture.viewDimension != wgpu::TextureViewDimension::Undefined) {
                    DAWN_TRY(ValidateTextureViewDimension(texture.viewDimension));
                    viewDimension = texture.viewDimension;
                }

                if (texture.multisampled) {
                    if (viewDimension != wgpu::TextureViewDimension::e2D) {
                        return DAWN_VALIDATION_ERROR("Multisampled texture bindings must be 2D.");
                    }
                    // TODO: This check should eventually become obsolete. According to the spec,
                    // depth can be used with both regular and comparison sampling. As such, during
                    // pipeline creation we have to check that if a comparison sampler is used
                    // with a texture, that texture must be both depth and not multisampled.
                    if (texture.sampleType == wgpu::TextureSampleType::Depth) {
                        return DAWN_VALIDATION_ERROR(
                            "Multisampled texture bindings must not be Depth.");
                    }
                }
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

                if (storageTexture.access == wgpu::StorageTextureAccess::WriteOnly) {
                    allowedStages &= ~wgpu::ShaderStage::Vertex;
                }
            }

            if (bindingMemberCount > 1) {
                return DAWN_VALIDATION_ERROR(
                    "Only one of buffer, sampler, texture, or storageTexture may be set for each "
                    "BindGroupLayoutEntry");
            } else if (bindingMemberCount == 1) {
                if (entry.type != wgpu::BindingType::Undefined) {
                    return DAWN_VALIDATION_ERROR(
                        "BindGroupLayoutEntry type must be undefined if any of buffer, sampler, "
                        "texture, or storageTexture are set");
                }
            } else if (bindingMemberCount == 0) {
                // Deprecated validation path
                device->EmitDeprecationWarning(
                    "The format of BindGroupLayoutEntry has changed, and will soon require the "
                    "buffer, sampler, texture, or storageTexture members be set rather than "
                    "setting type, etc. on the entry directly.");

                DAWN_TRY(ValidateBindingType(entry.type));
                DAWN_TRY(ValidateTextureComponentType(entry.textureComponentType));

                wgpu::TextureViewDimension viewDimension = wgpu::TextureViewDimension::e2D;
                if (entry.viewDimension != wgpu::TextureViewDimension::Undefined) {
                    DAWN_TRY(ValidateTextureViewDimension(entry.viewDimension));
                    viewDimension = entry.viewDimension;
                }

                bool canBeDynamic = false;

                switch (entry.type) {
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
                        if (entry.textureComponentType ==
                            wgpu::TextureComponentType::DepthComparison) {
                            return DAWN_VALIDATION_ERROR(
                                "Multisampled binding must not be DepthComparison.");
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

                    case wgpu::BindingType::Undefined:
                        UNREACHABLE();
                }

                if (entry.hasDynamicOffset && !canBeDynamic) {
                    return DAWN_VALIDATION_ERROR("Binding type cannot be dynamic.");
                }

                // Dynamic storage buffers aren't bounds checked properly in D3D12. Disallow them as
                // unsafe until the bounds checks are implemented.
                if (device->IsToggleEnabled(Toggle::DisallowUnsafeAPIs) && entry.hasDynamicOffset &&
                    (entry.type == wgpu::BindingType::StorageBuffer ||
                     entry.type == wgpu::BindingType::ReadonlyStorageBuffer)) {
                    return DAWN_VALIDATION_ERROR(
                        "Dynamic storage buffers are disallowed because they aren't secure yet. "
                        "See https://crbug.com/dawn/429");
                }
            }

            if (!IsSubset(entry.visibility, allowedStages)) {
                return DAWN_VALIDATION_ERROR("Binding type cannot be used with this visibility.");
            }

            IncrementBindingCounts(&bindingCounts, entry);

            bindingsSet.insert(bindingNumber);
        }

        DAWN_TRY(ValidateBindingCounts(bindingCounts));

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
            }
        }

        // TODO(dawn:527): Once the deprecated BindGroupLayoutEntry path has been removed, this can
        // turn into a simple `binding.buffer.type != wgpu::BufferBindingType::Undefined` check.
        bool IsBufferBinding(const BindGroupLayoutEntry& binding) {
            if (binding.buffer.type != wgpu::BufferBindingType::Undefined) {
                return true;
            } else if (binding.sampler.type != wgpu::SamplerBindingType::Undefined) {
                return false;
            } else if (binding.texture.sampleType != wgpu::TextureSampleType::Undefined) {
                return false;
            } else if (binding.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
                return false;
            }

            // Deprecated path
            switch (binding.type) {
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
                case wgpu::BindingType::Undefined:
                    return false;
            }
        }

        bool BindingHasDynamicOffset(const BindGroupLayoutEntry& binding) {
            if (binding.buffer.type != wgpu::BufferBindingType::Undefined) {
                return binding.buffer.hasDynamicOffset;
            } else if (binding.sampler.type != wgpu::SamplerBindingType::Undefined) {
                return false;
            } else if (binding.texture.sampleType != wgpu::TextureSampleType::Undefined) {
                return false;
            } else if (binding.storageTexture.access != wgpu::StorageTextureAccess::Undefined) {
                return false;
            }

            return binding.hasDynamicOffset;
        }

        bool SortBindingsCompare(const BindGroupLayoutEntry& a, const BindGroupLayoutEntry& b) {
            const bool aIsBuffer = IsBufferBinding(a);
            const bool bIsBuffer = IsBufferBinding(b);
            if (aIsBuffer != bIsBuffer) {
                // Always place buffers first.
                return aIsBuffer;
            } else {
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
                // Deprecated entry layout.
                switch (binding.type) {
                    case wgpu::BindingType::UniformBuffer:
                        bindingInfo.bindingType = BindingInfoType::Buffer;
                        bindingInfo.buffer.type = wgpu::BufferBindingType::Uniform;
                        bindingInfo.buffer.hasDynamicOffset = binding.hasDynamicOffset;
                        bindingInfo.buffer.minBindingSize = binding.minBufferBindingSize;
                        break;
                    case wgpu::BindingType::StorageBuffer:
                        bindingInfo.bindingType = BindingInfoType::Buffer;
                        bindingInfo.buffer.type = wgpu::BufferBindingType::Storage;
                        bindingInfo.buffer.hasDynamicOffset = binding.hasDynamicOffset;
                        bindingInfo.buffer.minBindingSize = binding.minBufferBindingSize;
                        break;
                    case wgpu::BindingType::ReadonlyStorageBuffer:
                        bindingInfo.bindingType = BindingInfoType::Buffer;
                        bindingInfo.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                        bindingInfo.buffer.hasDynamicOffset = binding.hasDynamicOffset;
                        bindingInfo.buffer.minBindingSize = binding.minBufferBindingSize;
                        break;

                    case wgpu::BindingType::Sampler:
                        bindingInfo.bindingType = BindingInfoType::Sampler;
                        bindingInfo.sampler.type = wgpu::SamplerBindingType::Filtering;
                        break;
                    case wgpu::BindingType::ComparisonSampler:
                        bindingInfo.bindingType = BindingInfoType::Sampler;
                        bindingInfo.sampler.type = wgpu::SamplerBindingType::Comparison;
                        break;

                    case wgpu::BindingType::MultisampledTexture:
                        bindingInfo.texture.multisampled = true;
                        DAWN_FALLTHROUGH;
                    case wgpu::BindingType::SampledTexture:
                        bindingInfo.bindingType = BindingInfoType::Texture;
                        bindingInfo.texture.viewDimension = binding.viewDimension;
                        if (binding.viewDimension == wgpu::TextureViewDimension::Undefined) {
                            bindingInfo.texture.viewDimension = wgpu::TextureViewDimension::e2D;
                        }

                        switch (binding.textureComponentType) {
                            case wgpu::TextureComponentType::Float:
                                bindingInfo.texture.sampleType = wgpu::TextureSampleType::Float;
                                break;
                            case wgpu::TextureComponentType::Uint:
                                bindingInfo.texture.sampleType = wgpu::TextureSampleType::Uint;
                                break;
                            case wgpu::TextureComponentType::Sint:
                                bindingInfo.texture.sampleType = wgpu::TextureSampleType::Sint;
                                break;
                            case wgpu::TextureComponentType::DepthComparison:
                                bindingInfo.texture.sampleType = wgpu::TextureSampleType::Depth;
                                break;
                        }
                        break;

                    case wgpu::BindingType::ReadonlyStorageTexture:
                        bindingInfo.bindingType = BindingInfoType::StorageTexture;
                        bindingInfo.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
                        bindingInfo.storageTexture.format = binding.storageTextureFormat;
                        bindingInfo.storageTexture.viewDimension = binding.viewDimension;
                        if (binding.viewDimension == wgpu::TextureViewDimension::Undefined) {
                            bindingInfo.storageTexture.viewDimension =
                                wgpu::TextureViewDimension::e2D;
                        }
                        break;
                    case wgpu::BindingType::WriteonlyStorageTexture:
                        bindingInfo.bindingType = BindingInfoType::StorageTexture;
                        bindingInfo.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                        bindingInfo.storageTexture.format = binding.storageTextureFormat;
                        bindingInfo.storageTexture.viewDimension = binding.viewDimension;
                        if (binding.viewDimension == wgpu::TextureViewDimension::Undefined) {
                            bindingInfo.storageTexture.viewDimension =
                                wgpu::TextureViewDimension::e2D;
                        }
                        break;

                    case wgpu::BindingType::Undefined:
                        UNREACHABLE();
                }
            }

            return bindingInfo;
        }

    }  // namespace

    // BindGroupLayoutBase

    BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                             const BindGroupLayoutDescriptor* descriptor)
        : CachedObject(device), mBindingInfo(BindingIndex(descriptor->entryCount)) {
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

    size_t BindGroupLayoutBase::ComputeContentHash() {
        ObjectContentHasher recorder;
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
