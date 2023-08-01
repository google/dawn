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

#include <spirv-tools/libspirv.hpp>

#include <map>
#include <string>
#include <vector>

#include "dawn/native/CacheRequest.h"
#include "dawn/native/Serializable.h"
#include "dawn/native/SpirvValidation.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "tint/tint.h"

namespace dawn::native::vulkan {

#define COMPILED_SPIRV_MEMBERS(X)   \
    X(std::vector<uint32_t>, spirv) \
    X(std::string, remappedEntryPoint)

// Represents the result and metadata for a SPIR-V compilation.
DAWN_SERIALIZABLE(struct, CompiledSpirv, COMPILED_SPIRV_MEMBERS){};
#undef COMPILED_SPIRV_MEMBERS

bool TransformedShaderModuleCacheKey::operator==(
    const TransformedShaderModuleCacheKey& other) const {
    if (layout != other.layout || entryPoint != other.entryPoint ||
        constants.size() != other.constants.size()) {
        return false;
    }
    if (!std::equal(constants.begin(), constants.end(), other.constants.begin())) {
        return false;
    }
    return true;
}

size_t TransformedShaderModuleCacheKeyHashFunc::operator()(
    const TransformedShaderModuleCacheKey& key) const {
    size_t hash = 0;
    HashCombine(&hash, key.layout, key.entryPoint);
    for (const auto& entry : key.constants) {
        HashCombine(&hash, entry.first, entry.second);
    }
    return hash;
}

class ShaderModule::ConcurrentTransformedShaderModuleCache {
  public:
    explicit ConcurrentTransformedShaderModuleCache(Device* device) : mDevice(device) {}

    ~ConcurrentTransformedShaderModuleCache() {
        std::lock_guard<std::mutex> lock(mMutex);

        for (const auto& [_, moduleAndSpirv] : mTransformedShaderModuleCache) {
            mDevice->GetFencedDeleter()->DeleteWhenUnused(moduleAndSpirv.vkModule);
        }
    }

    std::optional<ModuleAndSpirv> Find(const TransformedShaderModuleCacheKey& key) {
        std::lock_guard<std::mutex> lock(mMutex);

        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter != mTransformedShaderModuleCache.end()) {
            return iter->second.AsRefs();
        }
        return {};
    }
    ModuleAndSpirv AddOrGet(const TransformedShaderModuleCacheKey& key,
                            VkShaderModule module,
                            CompiledSpirv compilation) {
        ASSERT(module != VK_NULL_HANDLE);
        std::lock_guard<std::mutex> lock(mMutex);

        auto iter = mTransformedShaderModuleCache.find(key);
        if (iter == mTransformedShaderModuleCache.end()) {
            bool added = false;
            std::tie(iter, added) = mTransformedShaderModuleCache.emplace(
                key, Entry{module, std::move(compilation.spirv),
                           std::move(compilation.remappedEntryPoint)});
            ASSERT(added);
        } else {
            // No need to use FencedDeleter since this shader module was just created and does
            // not need to wait for queue operations to complete.
            // Also, use of fenced deleter here is not thread safe.
            mDevice->fn.DestroyShaderModule(mDevice->GetVkDevice(), module, nullptr);
        }
        return iter->second.AsRefs();
    }

  private:
    struct Entry {
        VkShaderModule vkModule;
        std::vector<uint32_t> spirv;
        std::string remappedEntryPoint;

        ModuleAndSpirv AsRefs() const {
            return {
                vkModule,
                spirv.data(),
                spirv.size(),
                remappedEntryPoint.c_str(),
            };
        }
    };

    Device* mDevice;
    std::mutex mMutex;
    std::unordered_map<TransformedShaderModuleCacheKey,
                       Entry,
                       TransformedShaderModuleCacheKeyHashFunc>
        mTransformedShaderModuleCache;
};

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}

ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor),
      mTransformedShaderModuleCache(
          std::make_unique<ConcurrentTransformedShaderModuleCache>(device)) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());
    return InitializeBase(parseResult, compilationMessages);
}

void ShaderModule::DestroyImpl() {
    ShaderModuleBase::DestroyImpl();
    // Remove reference to internal cache to trigger cleanup.
    mTransformedShaderModuleCache = nullptr;
}

ShaderModule::~ShaderModule() = default;

#define SPIRV_COMPILATION_REQUEST_MEMBERS(X)                                                     \
    X(SingleShaderStage, stage)                                                                  \
    X(const tint::Program*, inputProgram)                                                        \
    X(tint::BindingRemapperOptions, bindingRemapper)                                             \
    X(tint::ExternalTextureOptions, externalTextureOptions)                                      \
    X(std::optional<tint::ast::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(LimitsForCompilationRequest, limits)                                                       \
    X(std::string_view, entryPointName)                                                          \
    X(bool, isRobustnessEnabled)                                                                 \
    X(bool, disableWorkgroupInit)                                                                \
    X(bool, disableSymbolRenaming)                                                               \
    X(bool, useZeroInitializeWorkgroupMemoryExtension)                                           \
    X(bool, clampFragDepth)                                                                      \
    X(bool, disableImageRobustness)                                                              \
    X(bool, disableRuntimeSizedArrayIndexClamping)                                               \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, platform)

DAWN_MAKE_CACHE_REQUEST(SpirvCompilationRequest, SPIRV_COMPILATION_REQUEST_MEMBERS);
#undef SPIRV_COMPILATION_REQUEST_MEMBERS

ResultOrError<ShaderModule::ModuleAndSpirv> ShaderModule::GetHandleAndSpirv(
    SingleShaderStage stage,
    const ProgrammableStage& programmableStage,
    const PipelineLayout* layout,
    bool clampFragDepth) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleVk::GetHandleAndSpirv");

    // If the shader was destroyed, we should never call this function.
    ASSERT(IsAlive());

    ScopedTintICEHandler scopedICEHandler(GetDevice());

    // Check to see if we have the handle and spirv cached already.
    auto cacheKey = TransformedShaderModuleCacheKey{layout, programmableStage.entryPoint.c_str(),
                                                    programmableStage.constants};
    auto handleAndSpirv = mTransformedShaderModuleCache->Find(cacheKey);
    if (handleAndSpirv.has_value()) {
        return std::move(*handleAndSpirv);
    }

    // Creation of module and spirv is deferred to this point when using tint generator

    // Remap BindingNumber to BindingIndex in WGSL shader
    using BindingPoint = tint::BindingPoint;

    tint::BindingRemapperOptions bindingRemapper;

    const BindingInfoArray& moduleBindingInfo =
        GetEntryPoint(programmableStage.entryPoint.c_str()).bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
        const auto& groupBindingInfo = moduleBindingInfo[group];
        for (const auto& [binding, _] : groupBindingInfo) {
            BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
            BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                         static_cast<uint32_t>(binding)};

            BindingPoint dstBindingPoint{static_cast<uint32_t>(group),
                                         static_cast<uint32_t>(bindingIndex)};
            if (srcBindingPoint != dstBindingPoint) {
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
            }
        }
    }

    // Transform external textures into the binding locations specified in the bgl
    // TODO(dawn:1082): Replace this block with BuildExternalTextureTransformBindings.
    tint::ExternalTextureOptions externalTextureOptions;
    for (BindGroupIndex i : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayoutBase* bgl = layout->GetBindGroupLayout(i);

        for (const auto& [_, expansion] : bgl->GetExternalTextureBindingExpansionMap()) {
            externalTextureOptions
                .bindings_map[{static_cast<uint32_t>(i),
                               static_cast<uint32_t>(bgl->GetBindingIndex(expansion.plane0))}] = {
                {static_cast<uint32_t>(i),
                 static_cast<uint32_t>(bgl->GetBindingIndex(expansion.plane1))},
                {static_cast<uint32_t>(i),
                 static_cast<uint32_t>(bgl->GetBindingIndex(expansion.params))}};
        }
    }

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

#if TINT_BUILD_SPV_WRITER
    SpirvCompilationRequest req = {};
    req.stage = stage;
    req.inputProgram = GetTintProgram();
    req.bindingRemapper = std::move(bindingRemapper);
    req.externalTextureOptions = std::move(externalTextureOptions);
    req.entryPointName = programmableStage.entryPoint;
    req.isRobustnessEnabled = GetDevice()->IsRobustnessEnabled();
    req.disableWorkgroupInit = GetDevice()->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.disableSymbolRenaming = GetDevice()->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.useZeroInitializeWorkgroupMemoryExtension =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension);
    req.clampFragDepth = clampFragDepth;
    req.disableImageRobustness = GetDevice()->IsToggleEnabled(Toggle::VulkanUseImageRobustAccess2);
    // Currently we can disable index clamping on all runtime-sized arrays in Tint robustness
    // transform as unsized arrays can only be declared on storage address space.
    req.disableRuntimeSizedArrayIndexClamping =
        GetDevice()->IsToggleEnabled(Toggle::VulkanUseBufferRobustAccess2);
    req.platform = UnsafeUnkeyedValue(GetDevice()->GetPlatform());
    req.substituteOverrideConfig = std::move(substituteOverrideConfig);

    const CombinedLimits& limits = GetDevice()->GetLimits();
    req.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<CompiledSpirv> compilation;
    DAWN_TRY_LOAD_OR_RUN(
        compilation, GetDevice(), std::move(req), CompiledSpirv::FromBlob,
        [](SpirvCompilationRequest r) -> ResultOrError<CompiledSpirv> {
            tint::ast::transform::Manager transformManager;
            tint::ast::transform::DataMap transformInputs;

            // Many Vulkan drivers can't handle multi-entrypoint shader modules.
            // Run before the renamer so that the entry point name matches `entryPointName` still.
            transformManager.append(std::make_unique<tint::ast::transform::SingleEntryPoint>());
            transformInputs.Add<tint::ast::transform::SingleEntryPoint::Config>(
                std::string(r.entryPointName));

            // Needs to run before all other transforms so that they can use builtin names safely.
            if (!r.disableSymbolRenaming) {
                transformManager.Add<tint::ast::transform::Renamer>();
            }

            if (r.substituteOverrideConfig) {
                // This needs to run after SingleEntryPoint transform which removes unused overrides
                // for current entry point.
                transformManager.Add<tint::ast::transform::SubstituteOverride>();
                transformInputs.Add<tint::ast::transform::SubstituteOverride::Config>(
                    std::move(r.substituteOverrideConfig).value());
            }

            tint::Program program;
            tint::ast::transform::DataMap transformOutputs;
            {
                TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "RunTransforms");
                DAWN_TRY_ASSIGN(program,
                                RunTransforms(&transformManager, r.inputProgram, transformInputs,
                                              &transformOutputs, nullptr));
            }

            // Get the entry point name after the renamer pass.
            std::string remappedEntryPoint;
            if (r.disableSymbolRenaming) {
                remappedEntryPoint = r.entryPointName;
            } else {
                auto* data = transformOutputs.Get<tint::ast::transform::Renamer::Data>();
                ASSERT(data != nullptr);

                auto it = data->remappings.find(r.entryPointName.data());
                ASSERT(it != data->remappings.end());
                remappedEntryPoint = it->second;
            }
            ASSERT(remappedEntryPoint != "");

            // Validate workgroup size after program runs transforms.
            if (r.stage == SingleShaderStage::Compute) {
                Extent3D _;
                DAWN_TRY_ASSIGN(_, ValidateComputeStageWorkgroupSize(
                                       program, remappedEntryPoint.c_str(), r.limits));
            }

            tint::spirv::writer::Options options;
            options.clamp_frag_depth = r.clampFragDepth;
            options.disable_robustness = !r.isRobustnessEnabled;
            options.emit_vertex_point_size = true;
            options.disable_workgroup_init = r.disableWorkgroupInit;
            options.use_zero_initialize_workgroup_memory_extension =
                r.useZeroInitializeWorkgroupMemoryExtension;
            options.binding_remapper_options = r.bindingRemapper;
            options.external_texture_options = r.externalTextureOptions;
            options.disable_image_robustness = r.disableImageRobustness;
            options.disable_runtime_sized_array_index_clamping =
                r.disableRuntimeSizedArrayIndexClamping;

            TRACE_EVENT0(r.platform.UnsafeGetValue(), General, "tint::spirv::writer::Generate()");
            auto tintResult = tint::spirv::writer::Generate(&program, options);
            DAWN_INVALID_IF(!tintResult, "An error occured while generating SPIR-V: %s.",
                            tintResult.Failure());

            CompiledSpirv result;
            result.spirv = std::move(tintResult.Get().spirv);
            result.remappedEntryPoint = remappedEntryPoint;
            return result;
        },
        "Vulkan.CompileShaderToSPIRV");

#ifdef DAWN_ENABLE_SPIRV_VALIDATION
    DAWN_TRY(ValidateSpirv(GetDevice(), compilation->spirv.data(), compilation->spirv.size(),
                           GetDevice()->IsToggleEnabled(Toggle::DumpShaders)));
#endif

    VkShaderModuleCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.codeSize = compilation->spirv.size() * sizeof(uint32_t);
    createInfo.pCode = compilation->spirv.data();

    Device* device = ToBackend(GetDevice());

    VkShaderModule newHandle = VK_NULL_HANDLE;
    {
        TRACE_EVENT0(GetDevice()->GetPlatform(), General, "vkCreateShaderModule");
        DAWN_TRY(CheckVkSuccess(
            device->fn.CreateShaderModule(device->GetVkDevice(), &createInfo, nullptr, &*newHandle),
            "CreateShaderModule"));
    }

    ModuleAndSpirv moduleAndSpirv;
    if (newHandle != VK_NULL_HANDLE) {
        device->GetBlobCache()->EnsureStored(compilation);

        // Set the label on `newHandle` now, and not on `moduleAndSpirv.module` later
        // since `moduleAndSpirv.module` may be in use by multiple threads.
        SetDebugName(ToBackend(GetDevice()), newHandle, "Dawn_ShaderModule", GetLabel());
        moduleAndSpirv =
            mTransformedShaderModuleCache->AddOrGet(cacheKey, newHandle, compilation.Acquire());
    }

    return std::move(moduleAndSpirv);
#else
    return DAWN_INTERNAL_ERROR("TINT_BUILD_SPV_WRITER is not defined.");
#endif
}

}  // namespace dawn::native::vulkan
