// Copyright 2018 The Dawn Authors
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

#include "dawn_native/vulkan/ShaderModuleVk.h"

#include "dawn_native/TintUtils.h"
#include "dawn_native/vulkan/BindGroupLayoutVk.h"
#include "dawn_native/vulkan/DeviceVk.h"
#include "dawn_native/vulkan/FencedDeleter.h"
#include "dawn_native/vulkan/PipelineLayoutVk.h"
#include "dawn_native/vulkan/VulkanError.h"

#include <spirv_cross.hpp>

// Tint include must be after spirv_hlsl.hpp, because spirv-cross has its own
// version of spirv_headers. We also need to undef SPV_REVISION because SPIRV-Cross
// is at 3 while spirv-headers is at 4.
#undef SPV_REVISION
#include <tint/tint.h>

namespace dawn_native { namespace vulkan {

    ShaderModule::ConcurrentTransformedShaderModuleCache::ConcurrentTransformedShaderModuleCache(
        Device* device)
        : mDevice(device) {
    }

    ShaderModule::ConcurrentTransformedShaderModuleCache::
        ~ConcurrentTransformedShaderModuleCache() {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& iter : mTransformedShaderModuleCache) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(iter.second);
        }
    }

    VkShaderModule ShaderModule::ConcurrentTransformedShaderModuleCache::FindShaderModule(
        const PipelineLayoutEntryPointPair& key) {
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter != mTransformedShaderModuleCache.end()) {
            auto cached = iter->second;
            return cached;
        }
        return VK_NULL_HANDLE;
    }

    VkShaderModule ShaderModule::ConcurrentTransformedShaderModuleCache::AddOrGetCachedShaderModule(
        const PipelineLayoutEntryPointPair& key,
        VkShaderModule value) {
        ASSERT(value != VK_NULL_HANDLE);
        std::lock_guard<std::mutex> lock(mMutex);
        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter == mTransformedShaderModuleCache.end()) {
            mTransformedShaderModuleCache.emplace(key, value);
            return value;
        } else {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(value);
            return iter->second;
        }
    }

    // static
    ResultOrError<Ref<ShaderModule>> ShaderModule::Create(Device* device,
                                                          const ShaderModuleDescriptor* descriptor,
                                                          ShaderModuleParseResult* parseResult) {
        Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
        DAWN_TRY(module->Initialize(parseResult));
        return module;
    }

    ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
        : ShaderModuleBase(device, descriptor), mTransformedShaderModuleCache(device) {
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        std::vector<uint32_t> spirv;
        const std::vector<uint32_t>* spirvPtr;

        ScopedTintICEHandler scopedICEHandler(GetDevice());

        if (GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator)) {
            std::ostringstream errorStream;
            errorStream << "Tint SPIR-V writer failure:" << std::endl;

            tint::transform::Manager transformManager;
            if (GetDevice()->IsRobustnessEnabled()) {
                transformManager.Add<tint::transform::BoundArrayAccessors>();
            }

            tint::transform::DataMap transformInputs;

            tint::Program program;
            DAWN_TRY_ASSIGN(program,
                            RunTransforms(&transformManager, parseResult->tintProgram.get(),
                                          transformInputs, nullptr, nullptr));

            tint::writer::spirv::Options options;
            options.emit_vertex_point_size = true;
            options.disable_workgroup_init =
                GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
            auto result = tint::writer::spirv::Generate(&program, options);
            if (!result.success) {
                errorStream << "Generator: " << result.error << std::endl;
                return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
            }

            spirv = std::move(result.spirv);
            spirvPtr = &spirv;

            // Rather than use a new ParseResult object, we just reuse the original parseResult
            parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));
            parseResult->spirv = spirv;

            DAWN_TRY(InitializeBase(parseResult));
        } else {
            DAWN_TRY(InitializeBase(parseResult));
            spirvPtr = &GetSpirv();
        }

        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        std::vector<uint32_t> vulkanSource;
        createInfo.codeSize = spirvPtr->size() * sizeof(uint32_t);
        createInfo.pCode = spirvPtr->data();

        Device* device = ToBackend(GetDevice());
        return CheckVkSuccess(
            device->fn.CreateShaderModule(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
            "CreateShaderModule");
    }

    ShaderModule::~ShaderModule() {
        Device* device = ToBackend(GetDevice());

        if (mHandle != VK_NULL_HANDLE) {
            device->GetFencedDeleter()->DeleteWhenUnused(mHandle);
            mHandle = VK_NULL_HANDLE;
        }
    }

    VkShaderModule ShaderModule::GetHandle() const {
        ASSERT(!GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator));
        return mHandle;
    }

    ResultOrError<VkShaderModule> ShaderModule::GetTransformedModuleHandle(
        const char* entryPointName,
        PipelineLayout* layout) {
        ScopedTintICEHandler scopedICEHandler(GetDevice());

        ASSERT(GetDevice()->IsToggleEnabled(Toggle::UseTintGenerator));

        auto cacheKey = std::make_pair(layout, entryPointName);
        VkShaderModule cachedShaderModule =
            mTransformedShaderModuleCache.FindShaderModule(cacheKey);
        if (cachedShaderModule != VK_NULL_HANDLE) {
            return cachedShaderModule;
        }

        // Creation of VkShaderModule is deferred to this point when using tint generator
        std::ostringstream errorStream;
        errorStream << "Tint SPIR-V writer failure:" << std::endl;

        // Remap BindingNumber to BindingIndex in WGSL shader
        using BindingRemapper = tint::transform::BindingRemapper;
        using BindingPoint = tint::transform::BindingPoint;
        BindingRemapper::BindingPoints bindingPoints;
        BindingRemapper::AccessControls accessControls;

        const EntryPointMetadata::BindingInfoArray& moduleBindingInfo =
            GetEntryPoint(entryPointName).bindings;

        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
            const auto& groupBindingInfo = moduleBindingInfo[group];
            for (const auto& it : groupBindingInfo) {
                BindingNumber binding = it.first;
                BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
                BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                             static_cast<uint32_t>(binding)};

                BindingPoint dstBindingPoint{static_cast<uint32_t>(group),
                                             static_cast<uint32_t>(bindingIndex)};
                if (srcBindingPoint != dstBindingPoint) {
                    bindingPoints.emplace(srcBindingPoint, dstBindingPoint);
                }
            }
        }

        tint::transform::Manager transformManager;
        transformManager.append(std::make_unique<tint::transform::BindingRemapper>());
        // Many Vulkan drivers can't handle multi-entrypoint shader modules.
        transformManager.append(std::make_unique<tint::transform::SingleEntryPoint>());

        tint::transform::DataMap transformInputs;
        transformInputs.Add<BindingRemapper::Remappings>(std::move(bindingPoints),
                                                         std::move(accessControls),
                                                         /* mayCollide */ false);
        transformInputs.Add<tint::transform::SingleEntryPoint::Config>(entryPointName);

        tint::Program program;
        DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, GetTintProgram(), transformInputs,
                                               nullptr, nullptr));

        tint::writer::spirv::Options options;
        options.emit_vertex_point_size = true;
        options.disable_workgroup_init = GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
        auto result = tint::writer::spirv::Generate(&program, options);
        if (!result.success) {
            errorStream << "Generator: " << result.error << std::endl;
            return DAWN_VALIDATION_ERROR(errorStream.str().c_str());
        }

        std::vector<uint32_t> spirv = result.spirv;

        // Don't save the transformedParseResult but just create a VkShaderModule
        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        std::vector<uint32_t> vulkanSource;
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        Device* device = ToBackend(GetDevice());

        VkShaderModule newHandle = VK_NULL_HANDLE;

        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateShaderModule(device->GetVkDevice(), &createInfo, nullptr, &*newHandle),
            "CreateShaderModule"));
        if (newHandle != VK_NULL_HANDLE) {
            newHandle =
                mTransformedShaderModuleCache.AddOrGetCachedShaderModule(cacheKey, newHandle);
        }

        return newHandle;
    }

}}  // namespace dawn_native::vulkan
