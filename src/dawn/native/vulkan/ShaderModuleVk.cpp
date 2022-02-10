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

#include "dawn/native/vulkan/ShaderModuleVk.h"

#include "dawn/native/SpirvValidation.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include <tint/tint.h>
#include <spirv-tools/libspirv.hpp>

namespace dawn::native::vulkan {

    ShaderModule::ConcurrentTransformedShaderModuleCache::ConcurrentTransformedShaderModuleCache(
        Device* device)
        : mDevice(device) {
    }

    ShaderModule::ConcurrentTransformedShaderModuleCache::
        ~ConcurrentTransformedShaderModuleCache() {
        std::lock_guard<std::mutex> lock(mMutex);
        for (const auto& [_, module] : mTransformedShaderModuleCache) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(module);
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
        : ShaderModuleBase(device, descriptor),
          mTransformedShaderModuleCache(
              std::make_unique<ConcurrentTransformedShaderModuleCache>(device)) {
    }

    MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult) {
        if (GetDevice()->IsRobustnessEnabled()) {
            ScopedTintICEHandler scopedICEHandler(GetDevice());

            tint::transform::Robustness robustness;
            tint::transform::DataMap transformInputs;

            tint::Program program;
            DAWN_TRY_ASSIGN(program, RunTransforms(&robustness, parseResult->tintProgram.get(),
                                                   transformInputs, nullptr, nullptr));
            // Rather than use a new ParseResult object, we just reuse the original parseResult
            parseResult->tintProgram = std::make_unique<tint::Program>(std::move(program));
        }

        return InitializeBase(parseResult);
    }

    void ShaderModule::DestroyImpl() {
        ShaderModuleBase::DestroyImpl();
        // Remove reference to internal cache to trigger cleanup.
        mTransformedShaderModuleCache = nullptr;
    }

    ShaderModule::~ShaderModule() = default;

    ResultOrError<VkShaderModule> ShaderModule::GetTransformedModuleHandle(
        const char* entryPointName,
        PipelineLayout* layout) {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General,
                     "ShaderModuleVk::GetTransformedModuleHandle");

        // If the shader was destroyed, we should never call this function.
        ASSERT(IsAlive());

        ScopedTintICEHandler scopedICEHandler(GetDevice());

        auto cacheKey = std::make_pair(layout, entryPointName);
        VkShaderModule cachedShaderModule =
            mTransformedShaderModuleCache->FindShaderModule(cacheKey);
        if (cachedShaderModule != VK_NULL_HANDLE) {
            return cachedShaderModule;
        }

        // Creation of VkShaderModule is deferred to this point when using tint generator

        // Remap BindingNumber to BindingIndex in WGSL shader
        using BindingRemapper = tint::transform::BindingRemapper;
        using BindingPoint = tint::transform::BindingPoint;
        BindingRemapper::BindingPoints bindingPoints;
        BindingRemapper::AccessControls accessControls;

        const BindingInfoArray& moduleBindingInfo = GetEntryPoint(entryPointName).bindings;

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

        // Transform external textures into the binding locations specified in the bgl
        // TODO(dawn:1082): Replace this block with ShaderModuleBase::AddExternalTextureTransform.
        tint::transform::MultiplanarExternalTexture::BindingsMap newBindingsMap;
        for (BindGroupIndex i : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(i);

            ExternalTextureBindingExpansionMap expansions =
                bgl->GetExternalTextureBindingExpansionMap();

            std::map<BindingNumber, dawn_native::ExternalTextureBindingExpansion>::iterator it =
                expansions.begin();

            while (it != expansions.end()) {
                newBindingsMap[{static_cast<uint32_t>(i),
                                static_cast<uint32_t>(bgl->GetBindingIndex(it->second.plane0))}] = {
                    {static_cast<uint32_t>(i),
                     static_cast<uint32_t>(bgl->GetBindingIndex(it->second.plane1))},
                    {static_cast<uint32_t>(i),
                     static_cast<uint32_t>(bgl->GetBindingIndex(it->second.params))}};
                it++;
            }
        }

        if (!newBindingsMap.empty()) {
            transformManager.Add<tint::transform::MultiplanarExternalTexture>();
            transformInputs.Add<tint::transform::MultiplanarExternalTexture::NewBindingPoints>(
                newBindingsMap);
        }

        tint::Program program;
        {
            TRACE_EVENT0(GetDevice()->GetPlatform(), General, "RunTransforms");
            DAWN_TRY_ASSIGN(program, RunTransforms(&transformManager, GetTintProgram(),
                                                   transformInputs, nullptr, nullptr));
        }

        tint::writer::spirv::Options options;
        options.emit_vertex_point_size = true;
        options.disable_workgroup_init = GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);

        std::vector<uint32_t> spirv;
        {
            TRACE_EVENT0(GetDevice()->GetPlatform(), General, "tint::writer::spirv::Generate()");
            auto result = tint::writer::spirv::Generate(&program, options);
            DAWN_INVALID_IF(!result.success, "An error occured while generating SPIR-V: %s.",
                            result.error);

            spirv = std::move(result.spirv);
        }

        DAWN_TRY(
            ValidateSpirv(GetDevice(), spirv, GetDevice()->IsToggleEnabled(Toggle::DumpShaders)));

        VkShaderModuleCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.codeSize = spirv.size() * sizeof(uint32_t);
        createInfo.pCode = spirv.data();

        Device* device = ToBackend(GetDevice());

        VkShaderModule newHandle = VK_NULL_HANDLE;
        {
            TRACE_EVENT0(GetDevice()->GetPlatform(), General, "vkCreateShaderModule");
            DAWN_TRY(CheckVkSuccess(device->fn.CreateShaderModule(
                                        device->GetVkDevice(), &createInfo, nullptr, &*newHandle),
                                    "CreateShaderModule"));
        }
        if (newHandle != VK_NULL_HANDLE) {
            newHandle =
                mTransformedShaderModuleCache->AddOrGetCachedShaderModule(cacheKey, newHandle);
        }

        SetDebugName(ToBackend(GetDevice()), VK_OBJECT_TYPE_SHADER_MODULE,
                     reinterpret_cast<uint64_t&>(newHandle), "Dawn_ShaderModule", GetLabel());

        return newHandle;
    }

}  // namespace dawn::native::vulkan
