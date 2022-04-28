// Copyright 2019 The Dawn Authors
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

#include <array>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/native/Toggles.h"

namespace dawn::native {
    namespace {

        struct ToggleEnumAndInfo {
            Toggle toggle;
            ToggleInfo info;
        };

        using ToggleEnumAndInfoList =
            std::array<ToggleEnumAndInfo, static_cast<size_t>(Toggle::EnumCount)>;

        static constexpr ToggleEnumAndInfoList kToggleNameAndInfoList = {{
            {Toggle::EmulateStoreAndMSAAResolve,
             {"emulate_store_and_msaa_resolve",
              "Emulate storing into multisampled color attachments and doing MSAA resolve "
              "simultaneously. This workaround is enabled by default on the Metal drivers that do "
              "not support MTLStoreActionStoreAndMultisampleResolve. To support StoreOp::Store on "
              "those platforms, we should do MSAA resolve in another render pass after ending the "
              "previous one.",
              "https://crbug.com/dawn/56"}},
            {Toggle::NonzeroClearResourcesOnCreationForTesting,
             {"nonzero_clear_resources_on_creation_for_testing",
              "Clears texture to full 1 bits as soon as they are created, but doesn't update "
              "the tracking state of the texture. This way we can test the logic of clearing "
              "textures that use recycled memory.",
              "https://crbug.com/dawn/145"}},
            {Toggle::AlwaysResolveIntoZeroLevelAndLayer,
             {"always_resolve_into_zero_level_and_layer",
              "When the resolve target is a texture view that is created on the non-zero level or "
              "layer of a texture, we first resolve into a temporarily 2D texture with only one "
              "mipmap level and one array layer, and copy the result of MSAA resolve into the "
              "true resolve target. This workaround is enabled by default on the Metal drivers "
              "that have bugs when setting non-zero resolveLevel or resolveSlice.",
              "https://crbug.com/dawn/56"}},
            {Toggle::LazyClearResourceOnFirstUse,
             {"lazy_clear_resource_on_first_use",
              "Clears resource to zero on first usage. This initializes the resource "
              "so that no dirty bits from recycled memory is present in the new resource.",
              "https://crbug.com/dawn/145"}},
            {Toggle::TurnOffVsync,
             {"turn_off_vsync",
              "Turn off vsync when rendering. In order to do performance test or run perf tests, "
              "turn off vsync so that the fps can exeed 60.",
              "https://crbug.com/dawn/237"}},
            {Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy,
             {"use_temporary_buffer_in_texture_to_texture_copy",
              "Split texture-to-texture copy into two copies: copy from source texture into a "
              "temporary buffer, and copy from the temporary buffer into the destination texture "
              "when copying between compressed textures that don't have block-aligned sizes. This "
              "workaround is enabled by default on all Vulkan drivers to solve an issue in the "
              "Vulkan SPEC about the texture-to-texture copies with compressed formats. See #1005 "
              "(https://github.com/KhronosGroup/Vulkan-Docs/issues/1005) for more details.",
              "https://crbug.com/dawn/42"}},
            {Toggle::UseD3D12ResourceHeapTier2,
             {"use_d3d12_resource_heap_tier2",
              "Enable support for resource heap tier 2. Resource heap tier 2 allows mixing of "
              "texture and buffers in the same heap. This allows better heap re-use and reduces "
              "fragmentation.",
              "https://crbug.com/dawn/27"}},
            {Toggle::UseD3D12RenderPass,
             {"use_d3d12_render_pass",
              "Use the D3D12 render pass API introduced in Windows build 1809 by default. On "
              "versions of Windows prior to build 1809, or when this toggle is turned off, Dawn "
              "will emulate a render pass.",
              "https://crbug.com/dawn/36"}},
            {Toggle::UseD3D12ResidencyManagement,
             {"use_d3d12_residency_management",
              "Enable residency management. This allows page-in and page-out of resource heaps in "
              "GPU memory. This component improves overcommitted performance by keeping the most "
              "recently used resources local to the GPU. Turning this component off can cause "
              "allocation failures when application memory exceeds physical device memory.",
              "https://crbug.com/dawn/193"}},
            {Toggle::DisableResourceSuballocation,
             {"disable_resource_suballocation",
              "Force the backends to not perform resource suballocation. This may expose "
              "allocation "
              "patterns which would otherwise only occur with large or specific types of "
              "resources.",
              "https://crbug.com/1313172"}},
            {Toggle::SkipValidation,
             {"skip_validation", "Skip expensive validation of Dawn commands.",
              "https://crbug.com/dawn/271"}},
            {Toggle::VulkanUseD32S8,
             {"vulkan_use_d32s8",
              "Vulkan mandates support of either D32_FLOAT_S8 or D24_UNORM_S8. When available the "
              "backend will use D32S8 (toggle to on) but setting the toggle to off will make it "
              "use the D24S8 format when possible.",
              "https://crbug.com/dawn/286"}},
            {Toggle::VulkanUseS8,
             {"vulkan_use_s8",
              "Vulkan has a pure stencil8 format but it is not universally available. When this "
              "toggle is on, the backend will use S8 for the stencil8 format, otherwise it will "
              "fallback to D32S8 or D24S8.",
              "https://crbug.com/dawn/666"}},
            {Toggle::MetalDisableSamplerCompare,
             {"metal_disable_sampler_compare",
              "Disables the use of sampler compare on Metal. This is unsupported before A9 "
              "processors.",
              "https://crbug.com/dawn/342"}},
            {Toggle::MetalUseSharedModeForCounterSampleBuffer,
             {"metal_use_shared_mode_for_counter_sample_buffer",
              "The query set on Metal need to create MTLCounterSampleBuffer which storage mode "
              "must be either MTLStorageModeShared or MTLStorageModePrivate. But the private mode "
              "does not work properly on Intel platforms. The workaround is use shared mode "
              "instead.",
              "https://crbug.com/dawn/434"}},
            {Toggle::DisableBaseVertex,
             {"disable_base_vertex",
              "Disables the use of non-zero base vertex which is unsupported on some platforms.",
              "https://crbug.com/dawn/343"}},
            {Toggle::DisableBaseInstance,
             {"disable_base_instance",
              "Disables the use of non-zero base instance which is unsupported on some "
              "platforms.",
              "https://crbug.com/dawn/343"}},
            {Toggle::DisableIndexedDrawBuffers,
             {"disable_indexed_draw_buffers",
              "Disables the use of indexed draw buffer state which is unsupported on some "
              "platforms.",
              "https://crbug.com/dawn/582"}},
            {Toggle::DisableSnormRead,
             {"disable_snorm_read",
              "Disables reading from Snorm textures which is unsupported on some platforms.",
              "https://crbug.com/dawn/667"}},
            {Toggle::DisableDepthStencilRead,
             {"disable_depth_stencil_read",
              "Disables reading from depth/stencil textures which is unsupported on some "
              "platforms.",
              "https://crbug.com/dawn/667"}},
            {Toggle::DisableBGRARead,
             {"disable_bgra_read",
              "Disables reading from BGRA textures which is unsupported on some platforms.",
              "https://crbug.com/dawn/1393"}},
            {Toggle::DisableSampleVariables,
             {"disable_sample_variables",
              "Disables gl_SampleMask and related functionality which is unsupported on some "
              "platforms.",
              "https://crbug.com/dawn/673"}},
            {Toggle::UseD3D12SmallShaderVisibleHeapForTesting,
             {"use_d3d12_small_shader_visible_heap",
              "Enable use of a small D3D12 shader visible heap, instead of using a large one by "
              "default. This setting is used to test bindgroup encoding.",
              "https://crbug.com/dawn/155"}},
            {Toggle::UseDXC,
             {"use_dxc",
              "Use DXC instead of FXC for compiling HLSL when both dxcompiler.dll and dxil.dll "
              "is available.",
              "https://crbug.com/dawn/402"}},
            {Toggle::DisableRobustness,
             {"disable_robustness", "Disable robust buffer access", "https://crbug.com/dawn/480"}},
            {Toggle::MetalEnableVertexPulling,
             {"metal_enable_vertex_pulling",
              "Uses vertex pulling to protect out-of-bounds reads on Metal",
              "https://crbug.com/dawn/480"}},
            {Toggle::DisallowUnsafeAPIs,
             {"disallow_unsafe_apis",
              "Produces validation errors on API entry points or parameter combinations that "
              "aren't considered secure yet.",
              "http://crbug.com/1138528"}},
            {Toggle::FlushBeforeClientWaitSync,
             {"flush_before_client_wait_sync",
              "Call glFlush before glClientWaitSync to work around bugs in the latter",
              "https://crbug.com/dawn/633"}},
            {Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
             {"use_temp_buffer_in_small_format_texture_to_texture_copy_from_greater_to_less_mip_"
              "level",
              "Split texture-to-texture copy into two copies: copy from source texture into a "
              "temporary buffer, and copy from the temporary buffer into the destination texture "
              "under specific situations. This workaround is by default enabled on some Intel "
              "GPUs which have a driver bug in the execution of CopyTextureRegion() when we copy "
              "with the formats whose texel block sizes are less than 4 bytes from a greater mip "
              "level to a smaller mip level on D3D12 backends.",
              "https://crbug.com/1161355"}},
            {Toggle::EmitHLSLDebugSymbols,
             {"emit_hlsl_debug_symbols",
              "Sets the D3DCOMPILE_SKIP_OPTIMIZATION and D3DCOMPILE_DEBUG compilation flags when "
              "compiling HLSL code. Enables better shader debugging with external graphics "
              "debugging tools.",
              "https://crbug.com/dawn/776"}},
            {Toggle::DisallowSpirv,
             {"disallow_spirv",
              "Disallow usage of SPIR-V completely so that only WGSL is used for shader modules. "
              "This is useful to prevent a Chromium renderer process from successfully sending "
              "SPIR-V code to be compiled in the GPU process.",
              "https://crbug.com/1214923"}},
            {Toggle::DumpShaders,
             {"dump_shaders",
              "Dump shaders for debugging purposes. Dumped shaders will be log via "
              "EmitLog, thus printed in Chrome console or consumed by user-defined callback "
              "function.",
              "https://crbug.com/dawn/792"}},
            {Toggle::DEPRECATED_DumpTranslatedShaders,
             {"dump_translated_shaders", "Deprecated. Use dump_shaders",
              "https://crbug.com/dawn/792"}},
            {Toggle::ForceWGSLStep,
             {"force_wgsl_step",
              "When ingesting SPIR-V shaders, force a first conversion to WGSL. This allows "
              "testing Tint's SPIRV->WGSL translation on real content to be sure that it will "
              "work when the same translation runs in a WASM module in the page.",
              "https://crbug.com/dawn/960"}},
            {Toggle::DisableWorkgroupInit,
             {"disable_workgroup_init",
              "Disables the workgroup memory zero-initialization for compute shaders.",
              "https://crbug.com/tint/1003"}},
            {Toggle::DisableSymbolRenaming,
             {"disable_symbol_renaming",
              "Disables the WGSL symbol renaming so that names are preserved.",
              "https://crbug.com/dawn/1016"}},
            {Toggle::UseUserDefinedLabelsInBackend,
             {"use_user_defined_labels_in_backend",
              "Enables calls to SetLabel to be forwarded to backend-specific APIs that label "
              "objects.",
              "https://crbug.com/dawn/840"}},
            {Toggle::UsePlaceholderFragmentInVertexOnlyPipeline,
             {"use_placeholder_fragment_in_vertex_only_pipeline",
              "Use a placeholder empty fragment shader in vertex only render pipeline. This toggle "
              "must be enabled for OpenGL ES backend, and serves as a workaround by default "
              "enabled on some Metal devices with Intel GPU to ensure the depth result is correct.",
              "https://crbug.com/dawn/136"}},
            {Toggle::FxcOptimizations,
             {"fxc_optimizations",
              "Enable optimizations when compiling with FXC. Disabled by default because FXC "
              "miscompiles in many cases when optimizations are enabled.",
              "https://crbug.com/dawn/1203"}},
            {Toggle::RecordDetailedTimingInTraceEvents,
             {"record_detailed_timing_in_trace_events",
              "Record detailed timing information in trace events at certain point. Currently the "
              "timing information is recorded right before calling ExecuteCommandLists on a D3D12 "
              "command queue, and the information includes system time, CPU timestamp, GPU "
              "timestamp, and their frequency.",
              "https://crbug.com/dawn/1264"}},
            {Toggle::DisableTimestampQueryConversion,
             {"disable_timestamp_query_conversion",
              "Resolve timestamp queries into ticks instead of nanoseconds.",
              "https://crbug.com/dawn/1305"}},
            {Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension,
             {"use_vulkan_zero_initialize_workgroup_memory_extension",
              "Initialize workgroup memory with OpConstantNull on Vulkan when the Vulkan extension "
              "VK_KHR_zero_initialize_workgroup_memory is supported.",
              "https://crbug.com/dawn/1302"}},
            {Toggle::D3D12SplitBufferTextureCopyForRowsPerImagePaddings,
             {"d3d12_split_buffer_texture_copy_for_rows_per_image_paddings",
              "D3D12 requires more buffer storage than it should when rowsPerImage is greater than "
              "copyHeight, which means there are pure padding row(s) on each image. In this "
              "situation, the buffer used for B2T/T2B copy might be big enough according to "
              "WebGPU's spec but it doesn't meet D3D12's requirement, then we need to workaround "
              "it via split the copy operation into two copies, in order to make B2T/T2B copy "
              "being done correctly on D3D12.",
              "https://crbug.com/dawn/1289"}},
            {Toggle::MetalRenderR8RG8UnormSmallMipToTempTexture,
             {"metal_render_r8_rg8_unorm_small_mip_to_temp_texture",
              "Metal Intel devices have issues with r8unorm and rg8unorm textures where rendering "
              "to small mips (level >= 2) doesn't work correctly. Workaround this issue by "
              "detecting this case and rendering to a temporary texture instead (with copies "
              "before "
              "and after if needed).",
              "https://crbug.com/dawn/1071"}},
            // Comment to separate the }} so it is clearer what to copy-paste to add a toggle.
        }};
    }  // anonymous namespace

    void TogglesSet::Set(Toggle toggle, bool enabled) {
        if (toggle == Toggle::DEPRECATED_DumpTranslatedShaders) {
            Set(Toggle::DumpShaders, enabled);
            return;
        }
        ASSERT(toggle != Toggle::InvalidEnum);
        const size_t toggleIndex = static_cast<size_t>(toggle);
        toggleBitset.set(toggleIndex, enabled);
    }

    bool TogglesSet::Has(Toggle toggle) const {
        if (toggle == Toggle::DEPRECATED_DumpTranslatedShaders) {
            return Has(Toggle::DumpShaders);
        }
        ASSERT(toggle != Toggle::InvalidEnum);
        const size_t toggleIndex = static_cast<size_t>(toggle);
        return toggleBitset.test(toggleIndex);
    }

    std::vector<const char*> TogglesSet::GetContainedToggleNames() const {
        std::vector<const char*> togglesNameInUse(toggleBitset.count());

        uint32_t index = 0;
        for (uint32_t i : IterateBitSet(toggleBitset)) {
            const char* toggleName = ToggleEnumToName(static_cast<Toggle>(i));
            togglesNameInUse[index] = toggleName;
            ++index;
        }

        return togglesNameInUse;
    }

    const char* ToggleEnumToName(Toggle toggle) {
        ASSERT(toggle != Toggle::InvalidEnum);

        const ToggleEnumAndInfo& toggleNameAndInfo =
            kToggleNameAndInfoList[static_cast<size_t>(toggle)];
        ASSERT(toggleNameAndInfo.toggle == toggle);
        return toggleNameAndInfo.info.name;
    }

    const ToggleInfo* TogglesInfo::GetToggleInfo(const char* toggleName) {
        ASSERT(toggleName);

        EnsureToggleNameToEnumMapInitialized();

        const auto& iter = mToggleNameToEnumMap.find(toggleName);
        if (iter != mToggleNameToEnumMap.cend()) {
            return &kToggleNameAndInfoList[static_cast<size_t>(iter->second)].info;
        }
        return nullptr;
    }

    Toggle TogglesInfo::ToggleNameToEnum(const char* toggleName) {
        ASSERT(toggleName);

        EnsureToggleNameToEnumMapInitialized();

        const auto& iter = mToggleNameToEnumMap.find(toggleName);
        if (iter != mToggleNameToEnumMap.cend()) {
            return kToggleNameAndInfoList[static_cast<size_t>(iter->second)].toggle;
        }
        return Toggle::InvalidEnum;
    }

    void TogglesInfo::EnsureToggleNameToEnumMapInitialized() {
        if (mToggleNameToEnumMapInitialized) {
            return;
        }

        for (size_t index = 0; index < kToggleNameAndInfoList.size(); ++index) {
            const ToggleEnumAndInfo& toggleNameAndInfo = kToggleNameAndInfoList[index];
            ASSERT(index == static_cast<size_t>(toggleNameAndInfo.toggle));
            mToggleNameToEnumMap[toggleNameAndInfo.info.name] = toggleNameAndInfo.toggle;
        }

        mToggleNameToEnumMapInitialized = true;
    }

}  // namespace dawn::native
