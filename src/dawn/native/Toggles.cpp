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
#include "dawn/common/Log.h"
#include "dawn/native/Toggles.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/native/stream/Stream.h"

namespace dawn::native {
namespace {

struct ToggleEnumAndInfo {
    Toggle toggle;
    ToggleInfo info;
};

using ToggleEnumAndInfoList = std::array<ToggleEnumAndInfo, static_cast<size_t>(Toggle::EnumCount)>;

static constexpr ToggleEnumAndInfoList kToggleNameAndInfoList = {{
    {Toggle::EmulateStoreAndMSAAResolve,
     {"emulate_store_and_msaa_resolve",
      "Emulate storing into multisampled color attachments and doing MSAA resolve simultaneously. "
      "This workaround is enabled by default on the Metal drivers that do not support "
      "MTLStoreActionStoreAndMultisampleResolve. To support StoreOp::Store on those platforms, we "
      "should do MSAA resolve in another render pass after ending the previous one.",
      "https://crbug.com/dawn/56", ToggleStage::Device}},
    {Toggle::NonzeroClearResourcesOnCreationForTesting,
     {"nonzero_clear_resources_on_creation_for_testing",
      "Clears texture to full 1 bits as soon as they are created, but doesn't update the tracking "
      "state of the texture. This way we can test the logic of clearing textures that use recycled "
      "memory.",
      "https://crbug.com/dawn/145", ToggleStage::Device}},
    {Toggle::AlwaysResolveIntoZeroLevelAndLayer,
     {"always_resolve_into_zero_level_and_layer",
      "When the resolve target is a texture view that is created on the non-zero level or layer of "
      "a texture, we first resolve into a temporarily 2D texture with only one mipmap level and "
      "one array layer, and copy the result of MSAA resolve into the true resolve target. This "
      "workaround is enabled by default on the Metal drivers that have bugs when setting non-zero "
      "resolveLevel or resolveSlice. It is also enabled by default on Qualcomm Vulkan drivers, "
      "which have similar bugs.",
      "https://crbug.com/dawn/56", ToggleStage::Device}},
    {Toggle::LazyClearResourceOnFirstUse,
     {"lazy_clear_resource_on_first_use",
      "Clears resource to zero on first usage. This initializes the resource so that no dirty bits "
      "from recycled memory is present in the new resource.",
      "https://crbug.com/dawn/145", ToggleStage::Device}},
    {Toggle::TurnOffVsync,
     {"turn_off_vsync",
      "Turn off vsync when rendering. In order to do performance test or run perf tests, turn off "
      "vsync so that the fps can exeed 60.",
      "https://crbug.com/dawn/237", ToggleStage::Device}},
    {Toggle::UseTemporaryBufferInCompressedTextureToTextureCopy,
     {"use_temporary_buffer_in_texture_to_texture_copy",
      "Split texture-to-texture copy into two copies: copy from source texture into a temporary "
      "buffer, and copy from the temporary buffer into the destination texture when copying "
      "between compressed textures that don't have block-aligned sizes. This workaround is enabled "
      "by default on all Vulkan drivers to solve an issue in the Vulkan SPEC about the "
      "texture-to-texture copies with compressed formats. See #1005 "
      "(https://github.com/KhronosGroup/Vulkan-Docs/issues/1005) for more details.",
      "https://crbug.com/dawn/42", ToggleStage::Device}},
    {Toggle::UseD3D12ResourceHeapTier2,
     {"use_d3d12_resource_heap_tier2",
      "Enable support for resource heap tier 2. Resource heap tier 2 allows mixing of texture and "
      "buffers in the same heap. This allows better heap re-use and reduces fragmentation.",
      "https://crbug.com/dawn/27", ToggleStage::Device}},
    {Toggle::UseD3D12RenderPass,
     {"use_d3d12_render_pass",
      "Use the D3D12 render pass API introduced in Windows build 1809 by default. On versions of "
      "Windows prior to build 1809, or when this toggle is turned off, Dawn will emulate a render "
      "pass.",
      "https://crbug.com/dawn/36", ToggleStage::Device}},
    {Toggle::UseD3D12ResidencyManagement,
     {"use_d3d12_residency_management",
      "Enable residency management. This allows page-in and page-out of resource heaps in GPU "
      "memory. This component improves overcommitted performance by keeping the most recently used "
      "resources local to the GPU. Turning this component off can cause allocation failures when "
      "application memory exceeds physical device memory.",
      "https://crbug.com/dawn/193", ToggleStage::Device}},
    {Toggle::DisableResourceSuballocation,
     {"disable_resource_suballocation",
      "Force the backends to not perform resource suballocation. This may expose allocation "
      "patterns which would otherwise only occur with large or specific types of resources.",
      "https://crbug.com/1313172", ToggleStage::Device}},
    {Toggle::SkipValidation,
     {"skip_validation", "Skip expensive validation of Dawn commands.",
      "https://crbug.com/dawn/271", ToggleStage::Device}},
    {Toggle::VulkanUseD32S8,
     {"vulkan_use_d32s8",
      "Vulkan mandates support of either D32_FLOAT_S8 or D24_UNORM_S8. When available the backend "
      "will use D32S8 (toggle to on) but setting the toggle to off will make it use the D24S8 "
      "format when possible.",
      "https://crbug.com/dawn/286", ToggleStage::Device}},
    {Toggle::VulkanUseS8,
     {"vulkan_use_s8",
      "Vulkan has a pure stencil8 format but it is not universally available. When this toggle is "
      "on, the backend will use S8 for the stencil8 format, otherwise it will fallback to D32S8 or "
      "D24S8.",
      "https://crbug.com/dawn/666", ToggleStage::Device}},
    {Toggle::MetalDisableSamplerCompare,
     {"metal_disable_sampler_compare",
      "Disables the use of sampler compare on Metal. This is unsupported before A9 processors.",
      "https://crbug.com/dawn/342", ToggleStage::Device}},
    {Toggle::MetalUseSharedModeForCounterSampleBuffer,
     {"metal_use_shared_mode_for_counter_sample_buffer",
      "The query set on Metal need to create MTLCounterSampleBuffer which storage mode must be "
      "either MTLStorageModeShared or MTLStorageModePrivate. But the private mode does not work "
      "properly on Intel platforms. The workaround is use shared mode instead.",
      "https://crbug.com/dawn/434", ToggleStage::Device}},
    {Toggle::DisableBaseVertex,
     {"disable_base_vertex",
      "Disables the use of non-zero base vertex which is unsupported on some platforms.",
      "https://crbug.com/dawn/343", ToggleStage::Device}},
    {Toggle::DisableBaseInstance,
     {"disable_base_instance",
      "Disables the use of non-zero base instance which is unsupported on some platforms.",
      "https://crbug.com/dawn/343", ToggleStage::Device}},
    {Toggle::DisableIndexedDrawBuffers,
     {"disable_indexed_draw_buffers",
      "Disables the use of indexed draw buffer state which is unsupported on some platforms.",
      "https://crbug.com/dawn/582", ToggleStage::Device}},
    {Toggle::DisableDepthRead,
     {"disable_depth_read",
      "Disables reading from depth textures which is unsupported on some platforms.",
      "https://crbug.com/dawn/667", ToggleStage::Device}},
    {Toggle::DisableStencilRead,
     {"disable_stencil_read",
      "Disables reading from stencil textures which is unsupported on some platforms.",
      "https://crbug.com/dawn/667", ToggleStage::Device}},
    {Toggle::DisableDepthStencilRead,
     {"disable_depth_stencil_read",
      "Disables reading from depth/stencil textures which is unsupported on some platforms.",
      "https://crbug.com/dawn/667", ToggleStage::Device}},
    {Toggle::DisableSampleVariables,
     {"disable_sample_variables",
      "Disables gl_SampleMask and related functionality which is unsupported on some platforms.",
      "https://crbug.com/dawn/673", ToggleStage::Device}},
    {Toggle::UseD3D12SmallShaderVisibleHeapForTesting,
     {"use_d3d12_small_shader_visible_heap",
      "Enable use of a small D3D12 shader visible heap, instead of using a large one by default. "
      "This setting is used to test bindgroup encoding.",
      "https://crbug.com/dawn/155", ToggleStage::Device}},
    {Toggle::UseDXC,
     {"use_dxc",
      "Use DXC instead of FXC for compiling HLSL when both dxcompiler.dll and dxil.dll is "
      "available.",
      "https://crbug.com/dawn/402", ToggleStage::Adapter}},
    {Toggle::DisableRobustness,
     {"disable_robustness", "Disable robust buffer access", "https://crbug.com/dawn/480",
      ToggleStage::Device}},
    {Toggle::MetalEnableVertexPulling,
     {"metal_enable_vertex_pulling", "Uses vertex pulling to protect out-of-bounds reads on Metal",
      "https://crbug.com/dawn/480", ToggleStage::Device}},
    {Toggle::AllowUnsafeAPIs,
     {"allow_unsafe_apis",
      "Suppresses validation errors on API entry points or parameter combinations that aren't "
      "considered secure yet.",
      "http://crbug.com/1138528", ToggleStage::Instance}},
    {Toggle::FlushBeforeClientWaitSync,
     {"flush_before_client_wait_sync",
      "Call glFlush before glClientWaitSync to work around bugs in the latter",
      "https://crbug.com/dawn/633", ToggleStage::Device}},
    {Toggle::UseTempBufferInSmallFormatTextureToTextureCopyFromGreaterToLessMipLevel,
     {"use_temp_buffer_in_small_format_texture_to_texture_copy_from_greater_to_less_mip_level",
      "Split texture-to-texture copy into two copies: copy from source texture into a temporary "
      "buffer, and copy from the temporary buffer into the destination texture under specific "
      "situations. This workaround is by default enabled on some Intel GPUs which have a driver "
      "bug in the execution of CopyTextureRegion() when we copy with the formats whose texel "
      "block sizes are less than 4 bytes from a greater mip level to a smaller mip level on D3D12 "
      "backends.",
      "https://crbug.com/1161355", ToggleStage::Device}},
    {Toggle::EmitHLSLDebugSymbols,
     {"emit_hlsl_debug_symbols",
      "Sets the D3DCOMPILE_SKIP_OPTIMIZATION and D3DCOMPILE_DEBUG compilation flags when compiling "
      "HLSL code. Enables better shader debugging with external graphics debugging tools.",
      "https://crbug.com/dawn/776", ToggleStage::Device}},
    {Toggle::DisallowSpirv,
     {"disallow_spirv",
      "Disallow usage of SPIR-V completely so that only WGSL is used for shader modules. This is "
      "useful to prevent a Chromium renderer process from successfully sending SPIR-V code to be "
      "compiled in the GPU process.",
      "https://crbug.com/1214923", ToggleStage::Device}},
    {Toggle::DumpShaders,
     {"dump_shaders",
      "Dump shaders for debugging purposes. Dumped shaders will be log via EmitLog, thus printed "
      "in Chrome console or consumed by user-defined callback function.",
      "https://crbug.com/dawn/792", ToggleStage::Device}},
    {Toggle::ForceWGSLStep,
     {"force_wgsl_step",
      "When ingesting SPIR-V shaders, force a first conversion to WGSL. This allows testing Tint's "
      "SPIRV->WGSL translation on real content to be sure that it will work when the same "
      "translation runs in a WASM module in the page.",
      "https://crbug.com/dawn/960", ToggleStage::Device}},
    {Toggle::DisableWorkgroupInit,
     {"disable_workgroup_init",
      "Disables the workgroup memory zero-initialization for compute shaders.",
      "https://crbug.com/tint/1003", ToggleStage::Device}},
    {Toggle::DisableSymbolRenaming,
     {"disable_symbol_renaming", "Disables the WGSL symbol renaming so that names are preserved.",
      "https://crbug.com/dawn/1016", ToggleStage::Device}},
    {Toggle::UseUserDefinedLabelsInBackend,
     {"use_user_defined_labels_in_backend",
      "Enables calls to SetLabel to be forwarded to backend-specific APIs that label objects.",
      "https://crbug.com/dawn/840", ToggleStage::Device}},
    {Toggle::UsePlaceholderFragmentInVertexOnlyPipeline,
     {"use_placeholder_fragment_in_vertex_only_pipeline",
      "Use a placeholder empty fragment shader in vertex only render pipeline. This toggle must be "
      "enabled for OpenGL ES backend, the Vulkan Backend, and serves as a workaround by default "
      "enabled on some Metal "
      "devices with Intel GPU to ensure the depth result is correct.",
      "https://crbug.com/dawn/136", ToggleStage::Device}},
    {Toggle::FxcOptimizations,
     {"fxc_optimizations",
      "Enable optimizations when compiling with FXC. Disabled by default because FXC miscompiles "
      "in many cases when optimizations are enabled.",
      "https://crbug.com/dawn/1203", ToggleStage::Device}},
    {Toggle::RecordDetailedTimingInTraceEvents,
     {"record_detailed_timing_in_trace_events",
      "Record detailed timing information in trace events at certain point. Currently the timing "
      "information is recorded right before calling ExecuteCommandLists on a D3D12 command queue, "
      "and the information includes system time, CPU timestamp, GPU timestamp, and their "
      "frequency.",
      "https://crbug.com/dawn/1264", ToggleStage::Device}},
    {Toggle::DisableTimestampQueryConversion,
     {"disable_timestamp_query_conversion",
      "Resolve timestamp queries into ticks instead of nanoseconds.", "https://crbug.com/dawn/1305",
      ToggleStage::Device}},
    {Toggle::ClearBufferBeforeResolveQueries,
     {"clear_buffer_before_resolve_queries",
      "clear destination buffer to zero before resolving queries. This toggle is enabled on Intel "
      "Gen12 GPUs due to driver issue.",
      "https://crbug.com/dawn/1823", ToggleStage::Device}},
    {Toggle::VulkanUseZeroInitializeWorkgroupMemoryExtension,
     {"use_vulkan_zero_initialize_workgroup_memory_extension",
      "Initialize workgroup memory with OpConstantNull on Vulkan when the Vulkan extension "
      "VK_KHR_zero_initialize_workgroup_memory is supported.",
      "https://crbug.com/dawn/1302", ToggleStage::Device}},
    {Toggle::D3D12SplitBufferTextureCopyForRowsPerImagePaddings,
     {"d3d12_split_buffer_texture_copy_for_rows_per_image_paddings",
      "D3D12 requires more buffer storage than it should when rowsPerImage is greater than "
      "copyHeight, which means there are pure padding row(s) on each image. In this situation, "
      "the buffer used for B2T/T2B copy might be big enough according to WebGPU's spec but it "
      "doesn't meet D3D12's requirement, then we need to workaround it via split the copy "
      "operation into two copies, in order to make B2T/T2B copy being done correctly on D3D12.",
      "https://crbug.com/dawn/1289", ToggleStage::Device}},
    {Toggle::MetalRenderR8RG8UnormSmallMipToTempTexture,
     {"metal_render_r8_rg8_unorm_small_mip_to_temp_texture",
      "Metal Intel devices have issues with r8unorm and rg8unorm textures where rendering to small "
      "mips (level >= 2) doesn't work correctly. Workaround this issue by detecting this case and "
      "rendering to a temporary texture instead (with copies before and after if needed).",
      "https://crbug.com/dawn/1071", ToggleStage::Device}},
    {Toggle::DisableBlobCache,
     {"disable_blob_cache",
      "Disables usage of the blob cache (backed by the platform cache if set/passed). Prevents any "
      "persistent caching capabilities, i.e. pipeline caching.",
      "https://crbug.com/dawn/549", ToggleStage::Device}},
    {Toggle::D3D12ForceClearCopyableDepthStencilTextureOnCreation,
     {"d3d12_force_clear_copyable_depth_stencil_texture_on_creation",
      "Always clearing copyable depth stencil textures when creating them instead of skipping the "
      "initialization when the entire subresource is the copy destination as a workaround on Intel "
      "D3D12 drivers.",
      "https://crbug.com/dawn/1487", ToggleStage::Device}},
    {Toggle::D3D12DontSetClearValueOnDepthTextureCreation,
     {"d3d12_dont_set_clear_value_on_depth_texture_creation",
      "Don't set D3D12_CLEAR_VALUE when creating depth textures with CreatePlacedResource() or "
      "CreateCommittedResource() as a workaround on Intel Gen12 D3D12 drivers.",
      "https://crbug.com/dawn/1487", ToggleStage::Device}},
    {Toggle::D3D12AlwaysUseTypelessFormatsForCastableTexture,
     {"d3d12_always_use_typeless_formats_for_castable_texture",
      "Always use the typeless DXGI format when we create a texture with valid viewFormat. This "
      "Toggle is enabled by default on the D3D12 platforms where CastingFullyTypedFormatSupported "
      "is false.",
      "https://crbug.com/dawn/1276", ToggleStage::Device}},
    {Toggle::D3D12AllocateExtraMemoryFor2DArrayColorTexture,
     {"d3d12_allocate_extra_memory_for_2d_array_color_texture",
      "Memory allocation for 2D array color texture may be smaller than it should be on D3D12 on "
      "some Intel devices. So texture access can be out-of-bound, which may cause critical "
      "security issue. We can workaround this security issue via allocating extra memory and "
      "limiting its access in itself.",
      "https://crbug.com/dawn/949", ToggleStage::Device}},
    {Toggle::D3D12UseTempBufferInDepthStencilTextureAndBufferCopyWithNonZeroBufferOffset,
     {"d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_copy_with_non_zero_buffer_offset",
      "Split buffer-texture copy into two copies: do first copy with a temporary buffer at offset "
      "0, then copy from the temporary buffer to the destination. Now this toggle must be enabled "
      "on the D3D12 platforms where programmable MSAA is not supported.",
      "https://crbug.com/dawn/727", ToggleStage::Device}},
    {Toggle::D3D12UseTempBufferInTextureToTextureCopyBetweenDifferentDimensions,
     {"d3d12_use_temp_buffer_in_texture_to_texture_copy_between_different_dimensions",
      "Use an intermediate temporary buffer when copying between textures of different dimensions. "
      "Force-enabled on D3D12 when the driver does not have the "
      "TextureCopyBetweenDimensionsSupported feature.",
      "https://crbug.com/dawn/1216", ToggleStage::Device}},
    {Toggle::ApplyClearBigIntegerColorValueWithDraw,
     {"apply_clear_big_integer_color_value_with_draw",
      "Apply the clear value of the color attachment with a draw call when load op is 'clear'. "
      "This toggle is enabled by default on D3D12 backends when we set large integer values "
      "(> 2^24 or < -2^24 for signed integer formats) as the clear value of a color attachment "
      "with 32-bit integer or unsigned integer formats because D3D12 APIs only support using "
      "float numbers as clear values, while a float number cannot always precisely represent an "
      "integer that is greater than 2^24 or smaller than -2^24). This toggle is also enabled on "
      "Intel GPUs on Metal backend due to a driver issue on Intel Metal driver.",
      "https://crbug.com/dawn/537", ToggleStage::Device}},
    {Toggle::MetalUseMockBlitEncoderForWriteTimestamp,
     {"metal_use_mock_blit_encoder_for_write_timestamp",
      "Add mock blit command to blit encoder when encoding writeTimestamp as workaround on Metal."
      "This toggle is enabled by default on Metal backend where GPU counters cannot be stored to"
      "sampleBufferAttachments on empty blit encoder.",
      "https://crbug.com/dawn/1473", ToggleStage::Device}},
    {Toggle::VulkanSplitCommandBufferOnDepthStencilComputeSampleAfterRenderPass,
     {"vulkan_split_command_buffer_on_depth_stencil_compute_sample_after_render_pass",
      "Splits any command buffer that samples a depth/stencil texture in a compute pass after that "
      "texture was used as an attachment for a prior render pass. This toggle is enabled by "
      "default on Qualcomm GPUs, which have been observed experiencing a driver crash in this "
      "situation.",
      "https://crbug.com/dawn/1564", ToggleStage::Device}},
    {Toggle::DisableSubAllocationFor2DTextureWithCopyDstOrRenderAttachment,
     {"disable_sub_allocation_for_2d_texture_with_copy_dst_or_render_attachment",
      "Disable resource sub-allocation for the 2D texture with CopyDst or RenderAttachment usage. "
      "This toggle is enabled by default on D3D12 backends using Intel Gen9.5 and Gen11 GPUs and "
      "on Vulkan backends using Intel Gen12 GPUs due to Intel Mesa Vulkan and D3D12 driver issues.",
      "https://crbug.com/1237175", ToggleStage::Device}},
    {Toggle::MetalUseCombinedDepthStencilFormatForStencil8,
     {"metal_use_combined_depth_stencil_format_for_stencil8",
      "Use a combined depth stencil format instead of stencil8. Works around an issue where the "
      "stencil8 format alone does not work correctly. This toggle also causes depth stencil "
      "attachments using a stencil8 format to also set the depth attachment in the Metal render "
      "pass. This works around another issue where Metal fails to set the stencil attachment "
      "correctly for a combined depth stencil format if the depth attachment is not also set.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::MetalUseBothDepthAndStencilAttachmentsForCombinedDepthStencilFormats,
     {"metal_use_both_depth_and_stencil_attachments_for_combined_depth_stencil_formats",
      "In Metal, depth and stencil attachments are set separately. Setting just one without the "
      "other does not work correctly for combined depth stencil formats on some Metal drivers. "
      "This workarounds ensures that both are set. This situation arises during lazy clears, or "
      "for stencil8 formats if metal_use_combined_depth_stencil_format_for_stencil8 is also "
      "enabled.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::MetalKeepMultisubresourceDepthStencilTexturesInitialized,
     {"metal_keep_multisubresource_depth_stencil_textures_initialized",
      "Some platforms have bugs where the wrong depth stencil subresource is read/written. To "
      "avoid reads of uninitialized data, ensure that depth stencil textures with more than one "
      "subresource are completely initialized, and StoreOp::Discard is always translated as a "
      "Store.",
      "https://crbug.com/dawn/838", ToggleStage::Device}},
    {Toggle::MetalFillEmptyOcclusionQueriesWithZero,
     {"metal_fill_empty_occlusion_queries_with_zero",
      "Apple GPUs leave stale results in the visibility result buffer instead of writing zero if "
      "an occlusion query is empty. Workaround this by explicitly filling it with zero if there "
      "are no draw calls.",
      "https://crbug.com/dawn/1707", ToggleStage::Device}},
    {Toggle::UseBlitForBufferToDepthTextureCopy,
     {"use_blit_for_buffer_to_depth_texture_copy",
      "Use a blit instead of a copy command to copy buffer data to the depth aspect of a "
      "texture. Works around an issue where depth writes by copy commands are not visible "
      "to a render or compute pass.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::UseBlitForBufferToStencilTextureCopy,
     {"use_blit_for_buffer_to_stencil_texture_copy",
      "Use a blit instead of a copy command to copy buffer data to the stencil aspect of a "
      "texture. Works around an issue where stencil writes by copy commands are not visible "
      "to a render or compute pass.",
      "https://crbug.com/dawn/1389", ToggleStage::Device}},
    {Toggle::UseBlitForDepthTextureToTextureCopyToNonzeroSubresource,
     {"use_blit_for_depth_texture_to_texture_copy_to_nonzero_subresource",
      "Use a blit to copy from a depth texture to the nonzero subresource of a depth texture. "
      "Works around an issue where nonzero layers are not written.",
      "https://crbug.com/dawn/1083", ToggleStage::Device}},
    {Toggle::UseBlitForDepth16UnormTextureToBufferCopy,
     {"use_blit_for_depth16unorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy depth aspect of a texture to a buffer."
      "Workaround for OpenGL and OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForDepth32FloatTextureToBufferCopy,
     {"use_blit_for_depth32float_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy depth aspect of a texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForStencilTextureToBufferCopy,
     {"use_blit_for_stencil_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy stencil aspect of a texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1782", ToggleStage::Device}},
    {Toggle::UseBlitForSnormTextureToBufferCopy,
     {"use_blit_for_snorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy snorm texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1781", ToggleStage::Device}},
    {Toggle::UseBlitForBGRA8UnormTextureToBufferCopy,
     {"use_blit_for_bgra8unorm_texture_to_buffer_copy",
      "Use a blit instead of a copy command to copy bgra8unorm texture to a buffer."
      "Workaround for OpenGLES.",
      "https://crbug.com/dawn/1393", ToggleStage::Device}},
    {Toggle::D3D12ReplaceAddWithMinusWhenDstFactorIsZeroAndSrcFactorIsDstAlpha,
     {"d3d12_replace_add_with_minus_when_dst_factor_is_zero_and_src_factor_is_dst_alpha",
      "Replace the blending operation 'Add' with 'Minus' when dstBlendFactor is 'Zero' and "
      "srcBlendFactor is 'DstAlpha'. Works around an Intel D3D12 driver issue about alpha "
      "blending.",
      "https://crbug.com/dawn/1579", ToggleStage::Device}},
    {Toggle::D3D12PolyfillReflectVec2F32,
     {"d3d12_polyfill_reflect_vec2_f32",
      "Polyfill the reflect builtin for vec2<f32> for D3D12. This toggle is enabled by default on "
      "D3D12 backends using FXC on Intel GPUs due to a driver issue on Intel D3D12 driver.",
      "https://crbug.com/tint/1798", ToggleStage::Device}},
    {Toggle::VulkanClearGen12TextureWithCCSAmbiguateOnCreation,
     {"vulkan_clear_gen12_texture_with_ccs_ambiguate_on_creation",
      "Clears some R8-like textures to full 0 bits as soon as they are created. This Toggle is "
      "enabled on Intel Gen12 GPUs due to a mesa driver issue.",
      "https://crbug.com/chromium/1361662", ToggleStage::Device}},
    {Toggle::D3D12UseRootSignatureVersion1_1,
     {"d3d12_use_root_signature_version_1_1",
      "Use D3D12 Root Signature Version 1.1 to make additional guarantees about the descriptors in "
      "a descriptor heap and the data pointed to by the descriptors so that the drivers can make "
      "better optimizations on them.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::VulkanUseImageRobustAccess2,
     {"vulkan_use_image_robust_access_2",
      "Disable Tint robustness transform on textures when VK_EXT_robustness2 is supported and "
      "robustImageAccess2 == VK_TRUE.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::VulkanUseBufferRobustAccess2,
     {"vulkan_use_buffer_robust_access_2",
      "Disable index clamping on the runtime-sized arrays on buffers in Tint robustness transform "
      "when VK_EXT_robustness2 is supported and robustBufferAccess2 == VK_TRUE.",
      "https://crbug.com/tint/1890", ToggleStage::Device}},
    {Toggle::D3D12Use64KBAlignedMSAATexture,
     {"d3d12_use_64kb_alignment_msaa_texture",
      "Create MSAA textures with 64KB (D3D12_SMALL_MSAA_RESOURCE_PLACEMENT_ALIGNMENT) alignment.",
      "https://crbug.com/dawn/282", ToggleStage::Device}},
    {Toggle::ResolveMultipleAttachmentInSeparatePasses,
     {"resolve_multiple_attachments_in_separate_passes",
      "When multiple MSAA attachments are used in a render pass, splits any resolve steps into a "
      "separate render pass per resolve target. "
      "This workaround is enabled by default on ARM Mali drivers.",
      "https://crbug.com/dawn/1550", ToggleStage::Device}},
    {Toggle::D3D12CreateNotZeroedHeap,
     {"d3d12_create_not_zeroed_heap",
      "Create D3D12 heap with D3D12_HEAP_FLAG_CREATE_NOT_ZEROED when it is supported. It is safe "
      "because in Dawn we always clear the resources manually when needed.",
      "https://crbug.com/dawn/484", ToggleStage::Device}},
    {Toggle::NoWorkaroundSampleMaskBecomesZeroForAllButLastColorTarget,
     {"no_workaround_sample_mask_becomes_zero_for_all_but_last_color_target",
      "MacOS 12.0+ Intel has a bug where the sample mask is only applied for the last color "
      "target. If there are multiple color targets, all but the last one will use a sample mask "
      "of zero.",
      "https://crbug.com/dawn/1462", ToggleStage::Device}},
    {Toggle::NoWorkaroundIndirectBaseVertexNotApplied,
     {"no_workaround_indirect_base_vertex_not_applied",
      "MacOS Intel < Gen9 has a bug where indirect base vertex is not applied for "
      "drawIndexedIndirect. Draws are done as if it is always zero.",
      "https://crbug.com/dawn/966", ToggleStage::Device}},
    {Toggle::NoWorkaroundDstAlphaAsSrcBlendFactorForBothColorAndAlphaDoesNotWork,
     {"no_workaround_dst_alpha_as_src_blend_factor_for_both_color_and_alpha_does_not_work",
      "Using D3D12_BLEND_DEST_ALPHA as source blend factor for both color and alpha blending "
      "doesn't work correctly on the D3D12 backend using Intel Gen9 or Gen9.5 GPUs.",
      "https://crbug.com/dawn/1579", ToggleStage::Device}},
    // Comment to separate the }} so it is clearer what to copy-paste to add a toggle.
}};
}  // anonymous namespace

void TogglesSet::Set(Toggle toggle, bool enabled) {
    ASSERT(toggle != Toggle::InvalidEnum);
    const size_t toggleIndex = static_cast<size_t>(toggle);
    bitset.set(toggleIndex, enabled);
}

bool TogglesSet::Has(Toggle toggle) const {
    ASSERT(toggle != Toggle::InvalidEnum);
    const size_t toggleIndex = static_cast<size_t>(toggle);
    return bitset.test(toggleIndex);
}

size_t TogglesSet::Count() const {
    return bitset.count();
}

TogglesSet::Iterator TogglesSet::Iterate() const {
    return IterateBitSet(bitset);
}

TogglesState::TogglesState(ToggleStage stage) : mStage(stage) {}

TogglesState TogglesState::CreateFromTogglesDescriptor(const DawnTogglesDescriptor* togglesDesc,
                                                       ToggleStage requiredStage) {
    TogglesState togglesState(requiredStage);

    if (togglesDesc == nullptr) {
        return togglesState;
    }

    TogglesInfo togglesInfo;
    for (uint32_t i = 0; i < togglesDesc->enabledTogglesCount; ++i) {
        Toggle toggle = togglesInfo.ToggleNameToEnum(togglesDesc->enabledToggles[i]);
        if (toggle != Toggle::InvalidEnum) {
            const ToggleInfo* toggleInfo = togglesInfo.GetToggleInfo(toggle);
            // Accept the required toggles of current and earlier stage to allow override
            // inheritance.
            if (toggleInfo->stage <= requiredStage) {
                togglesState.mTogglesSet.Set(toggle, true);
                togglesState.mEnabledToggles.Set(toggle, true);
            }
        }
    }
    for (uint32_t i = 0; i < togglesDesc->disabledTogglesCount; ++i) {
        Toggle toggle = togglesInfo.ToggleNameToEnum(togglesDesc->disabledToggles[i]);
        if (toggle != Toggle::InvalidEnum) {
            const ToggleInfo* toggleInfo = togglesInfo.GetToggleInfo(toggle);
            // Accept the required toggles of current and earlier stage to allow override
            // inheritance.
            if (toggleInfo->stage <= requiredStage) {
                togglesState.mTogglesSet.Set(toggle, true);
                togglesState.mEnabledToggles.Set(toggle, false);
            }
        }
    }

    return togglesState;
}

TogglesState& TogglesState::InheritFrom(const TogglesState& inheritedToggles) {
    ASSERT(inheritedToggles.GetStage() < mStage);

    // Do inheritance. All toggles that are force-set in the inherited toggles states would
    // be force-set in the result toggles state, and all toggles that are set in the inherited
    // toggles states and not required in current toggles state would be set in the result toggles
    // state.
    for (uint32_t i : inheritedToggles.mTogglesSet.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage < mStage);
        bool isEnabled = inheritedToggles.mEnabledToggles.Has(toggle);
        bool isForced = inheritedToggles.mForcedToggles.Has(toggle);
        // Only inherit a toggle if it is not set by user requirement or is forced in earlier stage.
        // In this way we allow user requirement override the inheritance if not forced.
        if (!mTogglesSet.Has(toggle) || isForced) {
            mTogglesSet.Set(toggle, true);
            mEnabledToggles.Set(toggle, isEnabled);
            mForcedToggles.Set(toggle, isForced);
        }
    }

    return *this;
}

// Set a toggle to given state, if the toggle has not been already set. Do nothing otherwise.
void TogglesState::Default(Toggle toggle, bool enabled) {
    ASSERT(toggle != Toggle::InvalidEnum);
    ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage == mStage);
    if (IsSet(toggle)) {
        return;
    }
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
}

void TogglesState::ForceSet(Toggle toggle, bool enabled) {
    ASSERT(toggle != Toggle::InvalidEnum);
    ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage == mStage);
    // Make sure that each toggle is force-set at most once.
    ASSERT(!mForcedToggles.Has(toggle));
    if (mTogglesSet.Has(toggle) && mEnabledToggles.Has(toggle) != enabled) {
        dawn::WarningLog() << "Forcing toggle \"" << ToggleEnumToName(toggle) << "\" to " << enabled
                           << " when it was " << !enabled;
    }
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
    mForcedToggles.Set(toggle, true);
}

TogglesState& TogglesState::SetForTesting(Toggle toggle, bool enabled, bool forced) {
    ASSERT(toggle != Toggle::InvalidEnum);
    mTogglesSet.Set(toggle, true);
    mEnabledToggles.Set(toggle, enabled);
    mForcedToggles.Set(toggle, forced);

    return *this;
}

bool TogglesState::IsSet(Toggle toggle) const {
    // Ensure that the toggle never used earlier than its stage.
    ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage <= mStage);
    return mTogglesSet.Has(toggle);
}

// Return true if the toggle is provided in enable list, and false otherwise.
bool TogglesState::IsEnabled(Toggle toggle) const {
    // Ensure that the toggle never used earlier than its stage.
    ASSERT(TogglesInfo::GetToggleInfo(toggle)->stage <= mStage);
    return mEnabledToggles.Has(toggle);
}

ToggleStage TogglesState::GetStage() const {
    return mStage;
}

std::vector<const char*> TogglesState::GetEnabledToggleNames() const {
    std::vector<const char*> enabledTogglesName(mEnabledToggles.Count());

    uint32_t index = 0;
    for (uint32_t i : mEnabledToggles.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        // All enabled toggles must be provided.
        ASSERT(mTogglesSet.Has(toggle));
        const char* toggleName = ToggleEnumToName(toggle);
        enabledTogglesName[index] = toggleName;
        ++index;
    }

    return enabledTogglesName;
}

std::vector<const char*> TogglesState::GetDisabledToggleNames() const {
    std::vector<const char*> enabledTogglesName(mTogglesSet.Count() - mEnabledToggles.Count());

    uint32_t index = 0;
    for (uint32_t i : mTogglesSet.Iterate()) {
        const Toggle& toggle = static_cast<Toggle>(i);
        // Disabled toggles are those provided but not enabled.
        if (!mEnabledToggles.Has(toggle)) {
            const char* toggleName = ToggleEnumToName(toggle);
            enabledTogglesName[index] = toggleName;
            ++index;
        }
    }

    return enabledTogglesName;
}

// Allowing TogglesState to be used in cache key.
void StreamIn(stream::Sink* s, const TogglesState& togglesState) {
    StreamIn(s, togglesState.mEnabledToggles.bitset);
}

const char* ToggleEnumToName(Toggle toggle) {
    ASSERT(toggle != Toggle::InvalidEnum);

    const ToggleEnumAndInfo& toggleNameAndInfo =
        kToggleNameAndInfoList[static_cast<size_t>(toggle)];
    ASSERT(toggleNameAndInfo.toggle == toggle);
    return toggleNameAndInfo.info.name;
}

TogglesInfo::TogglesInfo() = default;

TogglesInfo::~TogglesInfo() = default;

const ToggleInfo* TogglesInfo::GetToggleInfo(const char* toggleName) {
    ASSERT(toggleName);

    EnsureToggleNameToEnumMapInitialized();

    const auto& iter = mToggleNameToEnumMap.find(toggleName);
    if (iter != mToggleNameToEnumMap.cend()) {
        return &kToggleNameAndInfoList[static_cast<size_t>(iter->second)].info;
    }
    return nullptr;
}

const ToggleInfo* TogglesInfo::GetToggleInfo(Toggle toggle) {
    ASSERT(toggle != Toggle::InvalidEnum);

    return &kToggleNameAndInfoList[static_cast<size_t>(toggle)].info;
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
