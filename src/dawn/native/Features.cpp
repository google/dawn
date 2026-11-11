// Copyright 2019 The Dawn & Tint Authors
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

#include "src/dawn/native/Features.h"

#include <array>
#include <utility>

#include "src/dawn/common/ityp_array.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/heap_array.h"

namespace dawn::native {
namespace {

struct ManualFeatureInfo {
    const char* description;
    const char* url;
    FeatureInfo::FeatureSafety featureSafety;
};

struct FeatureEnumAndInfo {
    Feature feature;
    ManualFeatureInfo info;
};

static constexpr auto kFeatureInfo = std::to_array<FeatureEnumAndInfo>({
    {Feature::TextureCompressionBC,
     {"Support Block Compressed (BC) texture formats",
      "https://gpuweb.github.io/gpuweb/#texture-compression-bc", FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureCompressionBCSliced3D,
     {"Allows Block Compressed (BC) texture formats to be used with 3D dimensions",
      "https://gpuweb.github.io/gpuweb/#texture-compression-bc-sliced-3d",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureCompressionETC2,
     {"Support Ericsson Texture Compressed (ETC2/EAC) texture formats",
      "https://gpuweb.github.io/gpuweb/#texture-compression-etc2",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureCompressionASTC,
     {"Support Adaptable Scalable Texture Compressed (ASTC) "
      "texture formats",
      "https://gpuweb.github.io/gpuweb/#texture-compression-astc",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureCompressionASTCSliced3D,
     {"Allows Adaptable Scalable Texture Compressed (ASTC) texture formats to be used with 3D "
      "dimensions",
      "https://gpuweb.github.io/gpuweb/#texture-compression-astc-sliced-3d",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TimestampQuery,
     {"Support Timestamp Query", "https://gpuweb.github.io/gpuweb/#timestamp-query",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::ChromiumExperimentalTimestampQueryInsidePasses,
     {"Support experimental Timestamp Query inside render/compute pass",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "timestamp_query_inside_passes.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::DepthClipControl,
     {"Disable depth clipping of primitives to the clip volume",
      "https://gpuweb.github.io/gpuweb/#depth-clip-control", FeatureInfo::FeatureSafety::Safe}},
    {Feature::Depth32FloatStencil8,
     {"Support depth32float-stencil8 texture format",
      "https://gpuweb.github.io/gpuweb/#depth32float-stencil8", FeatureInfo::FeatureSafety::Safe}},
    {Feature::IndirectFirstInstance,
     {"Support non-zero first instance values on indirect draw calls",
      "https://gpuweb.github.io/gpuweb/#indirect-first-instance",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::ShaderF16,
     {"Supports the \"enable f16;\" directive in WGSL",
      "https://gpuweb.github.io/gpuweb/#shader-f16", FeatureInfo::FeatureSafety::Safe}},
    {Feature::RG11B10UfloatRenderable,
     {"Allows the RENDER_ATTACHMENT usage on textures with format \"rg11b10ufloat\", and also "
      "allows textures of that format to be multisampled.",
      "https://gpuweb.github.io/gpuweb/#rg11b10ufloat-renderable",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::BGRA8UnormStorage,
     {"Allows the STORAGE usage on textures with format \"bgra8unorm\".",
      "https://gpuweb.github.io/gpuweb/#bgra8unorm-storage", FeatureInfo::FeatureSafety::Safe}},
    {Feature::Float32Filterable,
     {"Allows textures with formats \"r32float\" \"rg32float\" and \"rgba32float\" to be "
      "filtered.",
      "https://gpuweb.github.io/gpuweb/#float32-filterable", FeatureInfo::FeatureSafety::Safe}},
    {Feature::Float32Blendable,
     {"Allows textures with formats \"r32float\" \"rg32float\" and \"rgba32float\" to be "
      "blendable.",
      "https://gpuweb.github.io/gpuweb/#float32-blendable", FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnInternalUsages,
     {"Add internal usages to resources to affect how the texture is allocated, but not "
      "frontend validation. Other internal commands may access this usage.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_internal_usages.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnMultiPlanarFormats,
     {"Import and use multi-planar texture formats with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::MultiPlanarFormatExtendedUsages,
     {"Enable creating multi-planar formatted textures directly without importing. Also allows "
      "including CopyDst as texture's usage and per plane copies between a texture and a buffer.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarFormatP010,
     {"Import and use the P010 multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::MultiPlanarFormatNv12a,
     {"Import and use the NV12A multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarFormatNv16,
     {"Import and use the NV16 multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarFormatNv24,
     {"Import and use the NV24 multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarFormatP210,
     {"Import and use the P210 multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarFormatP410,
     {"Import and use the P410 multi-planar texture format with per plane views",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::MultiPlanarRenderTargets,
     {"Import and use multi-planar texture formats as render attachments",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_planar_formats.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnNative,
     {"WebGPU is running on top of dawn_native, granting some additional capabilities.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_native.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::ImplicitDeviceSynchronization,
     {"Public API methods (except encoding) will have implicit device synchronization. So they "
      "will be safe to be used on multiple threads.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "implicit_device_synchronization.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TransientAttachments,
     {"Presence of this feature on an Adapter is a hint indicating that the TransientAttachment "
      "usage will be used by the backend. If it's not present, the usage is allowed, but it is a "
      "no-op. Enabling this feature on a Device doesn't do anything.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "transient_attachments.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::MSAARenderToSingleSampled,
     {"Support multisampled rendering on single-sampled attachments efficiently.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "msaa_render_to_single_sampled.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DualSourceBlending,
     {"Support dual source blending. Enables Src1, OneMinusSrc1, Src1Alpha, and OneMinusSrc1Alpha "
      "blend factors along with @blend_src WGSL output attribute.",
      "https://gpuweb.github.io/gpuweb/#dom-gpufeaturename-dual-source-blending",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::D3D11MultithreadProtected,
     {"Enable ID3D11Multithread protection for interop with external users of the D3D11 device.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "d3d11_multithread_protected.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::ANGLETextureSharing,
     {"Enable ANGLE texture sharing to allow the OpenGL ES backend to share textures by external "
      "OpenGL texture ID.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "angle_texture_sharing.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::PixelLocalStorageCoherent,
     {"Supports passing information between invocation in a render pass that cover the same pixel."
      "This helps more efficiently implement algorithms that would otherwise require ping-ponging"
      "between render targets. The coherent version of this extension means that no barrier calls"
      "are needed to prevent data races between fragment shaders on the same pixel.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "pixel_local_storage.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::PixelLocalStorageNonCoherent,
     {"Supports passing information between invocation in a render pass that cover the same pixel."
      "This helps more efficiently implement algorithms that would otherwise require ping-ponging"
      "between render targets. The non-coherent version of this extension means that barrier calls"
      "are needed to prevent data races between fragment shaders on the same pixels (note that "
      "overlapping fragments from the same draw cannot be made data race free).",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "pixel_local_storage.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::Unorm16TextureFormats,
     {"Supports R/RG/RGBA16 unorm texture formats",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "norm16_texture_formats.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryVkDedicatedAllocation,
     {"Support specifying whether a Vulkan allocation for shared texture memory is a dedicated "
      "memory allocation.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryAHardwareBuffer,
     {"Support importing AHardwareBuffer as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryDmaBuf,
     {"Support importing DmaBuf as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryOpaqueFD,
     {"Support importing OpaqueFD as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryZirconHandle,
     {"Support importing ZirconHandle as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryDXGISharedHandle,
     {"Support importing DXGISharedHandle as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryD3D11Texture2D,
     {"Support importing D3D11Texture2D as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryIOSurface,
     {"Support importing IOSurface as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedTextureMemoryEGLImage,
     {"Support importing EGLImage as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceVkSemaphoreOpaqueFD,
     {"Support for importing and exporting VkSemaphoreOpaqueFD used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceSyncFD,
     {"Support for importing and exporting SyncFD used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceVkSemaphoreZirconHandle,
     {"Support for importing and exporting VkSemaphoreZirconHandle used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceDXGISharedHandle,
     {"Support for importing and exporting DXGISharedHandle used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceMTLSharedEvent,
     {"Support for importing and exporting MTLSharedEvent used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedFenceEGLSync,
     {"Support for importing and exporting EGLSync objects used for GPU synchronization.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_fence.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::HostMappedPointer,
     {"Support creation of buffers from host-mapped pointers.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "host_mapped_pointer.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::FramebufferFetch,
     {"Support loading the current framebuffer value in fragment shaders.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "framebuffer_fetch.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::BufferMapExtendedUsages,
     {"Support creating all kinds of buffers with MapRead and/or MapWrite usage.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "buffer_map_extended_usages.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::BufferMapWriteExtendedUsages,
     {"Support creating buffers with MapWrite and any other usage except MapRead.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "buffer_map_write_extended_usages.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::AdapterPropertiesMemoryHeaps,
     {"Support querying memory heap info from the adapter.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "adapter_properties.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::AdapterPropertiesD3D,
     {"Support querying D3D info from the adapter.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "adapter_properties.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::AdapterPropertiesVk,
     {"Support querying Vulkan info from the adapter.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "adapter_properties.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::AdapterPropertiesDrm,
     {"Support querying DRM info from the adapter.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "adapter_properties.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::SharedBufferMemoryD3D12Resource,
     {"Support importing ID3D12Resource as shared buffer memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/shared_buffer.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnFormatCapabilities,
     {"Supports querying the capabilities of a texture format.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "format_capabilities.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnDrmFormatCapabilities,
     {"Supports querying the DRM-related capabilities of a texture format.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "format_capabilities.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::StaticSamplers,
     {"Support setting samplers statically as part of bind group layout",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "static_samplers.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::YCbCrVulkanSamplers,
     {"Support setting VkSamplerYcbcrConversionCreateInfo as part of static vulkan sampler "
      "descriptor",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "y_cb_cr_vulkan_samplers.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::ShaderModuleCompilationOptions,
     {"Support overriding default shader module compilation options.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shader_module_compilation_options.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnLoadResolveTexture,
     {"Support ExpandResolveTexture as LoadOp for a render pass. This LoadOp will expand the "
      "resolve texture into the MSAA texture as a load operation",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_load_resolve_texture.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnPartialLoadResolveTexture,
     {"Support RenderPassDescriptorResolveRect as chained struct into RenderPassDescriptor "
      "for a render pass. This will expand and resolve the texels within the rect of texture.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_partial_load_resolve_texture.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::Subgroups,
     {"Supports the \"enable subgroups;\" directive in WGSL.",
      "https://github.com/gpuweb/gpuweb/blob/main/proposals/subgroups.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureFormatsTier1,
     {"Supports some new GPUTextureFormats with the RENDER_ATTACHMENT, blendable, multisampling "
      "capabilities and the STORAGE_BINDING capability with the 'read-only' and 'write-only'"
      "GPUStorageTextureAccesses.",
      "https://gpuweb.github.io/gpuweb/#texture-formats-tier1", FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureFormatsTier2,
     {"Supports StorageTextureAccess 'read-write' on several additional formats.",
      "https://gpuweb.github.io/gpuweb/#texture-formats-tier2", FeatureInfo::FeatureSafety::Safe}},
    {Feature::TextureComponentSwizzle,
     {"Allows GPUTextureViews to rearrange or replace the color components from texture's "
      "red/green/blue/alpha channels when used as a TEXTURE_BINDING.",
      "https://gpuweb.github.io/gpuweb/#texture-component-swizzle",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::CoreFeaturesAndLimits,
     {"Lifts all compatibility mode restrictions (features and limits) to core when enabled on a "
      "device.",
      "https://github.com/gpuweb/gpuweb/blob/main/proposals/"
      "compatibility-mode.md#core-features-and-limits-feature",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::MultiDrawIndirect,
     {"Support MultiDrawIndirect and MultiDrawIndexedIndirect. Allows batching multiple indirect "
      "calls with one command",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "multi_draw_indirect.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::ClipDistances,
     {"Support the \"enable clip_distances;\" directive in WGSL.",
      "https://gpuweb.github.io/gpuweb/#dom-gpufeaturename-clip-distances",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnTexelCopyBufferRowAlignment,
     {"Expose the min row alignment in buffer for texel copy operations.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_texel_copy_buffer_row_alignment.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::FlexibleTextureViews,
     {"Remove the texture view restrictions in Compat Mode.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "flexible_texture_views.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::ChromiumExperimentalSubgroupMatrix,
     {"Support the \"enable chromium_experimental_subgroup_matrix;\" directive in WGSL.",
      "https://github.com/gpuweb/gpuweb/issues/4195", FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::DawnDeviceAllocatorControl,
     {"Supports configuring device allocator via DawnDeviceAllocatorControl",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_device_allocator_control.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::PrimitiveIndex,
     {"Supports the \"enable primitive_index;\" directive in WGSL",
      "https://gpuweb.github.io/gpuweb/#dom-gpufeaturename-primitive-index",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::AdapterPropertiesWGPU,
     {"Support querying WebGPU backend info from the adapter.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "adapter_properties.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::SharedBufferMemoryFromWindowsHandle,
     {"Supports importing a shared memory file mapping handle as shared buffer memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_buffer_memory.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::SharedBufferMemoryHostPointer,
     {"Supports importing a host-allocated memory pointer as shared buffer memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_buffer_memory.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::SharedTextureMemoryD3D12Resource,
     {"Support importing ID3D12Resource as shared texture memory.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "shared_texture_memory.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::ChromiumExperimentalSamplingResourceTable,
     {"Experimental support for the bindless sampling resource table",
      "https://github.com/gpuweb/gpuweb/blob/main/proposals/bindless.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::SubgroupSizeControl,
     {"Support the \"enable subgroup_size_control;\" directive in WGSL.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/tint/extensions/"
      "subgroup_size_control.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::AtomicVec2uMinMax,
     {"Support the \"enable atomic_vec2u_min_max;\" directive for 64-bit atomics via vec2<u32> "
      "types",
      "https://github.com/gpuweb/gpuweb/blob/main/proposals/atomic-64-min-max.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::Unorm16FormatsForExternalTexture,
     {"Supports R/RG/RGBA16Unorm formats for ExternalTexture planes even if not all the required "
      "feature support has been enabled.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "unorm16_formats_for_external_texture.md",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::OpaqueYCbCrAndroidForExternalTexture,
     {"Allows creating an ExternalTexture from an imported AHardwareBuffer with an opaque YCbCr "
      "format.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "opaque_ycbcr_android_for_external_texture.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::Unorm16Filterable,
     {"Allows textures with formats \"r16unorm\" \"rg16unorm\" and \"rgba16unorm\" to be "
      "filtered.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "unorm16_filterable.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::RenderPassRenderArea,
     {"Supports specifying render area for render pass.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "render_pass_render_area.md",
      FeatureInfo::FeatureSafety::Unsafe}},
    {Feature::TextureCompressionUnaligned,
     {"Supports creating compressed texture with partial blocks in level 0",
      "https://gpuweb.github.io/gpuweb/#dom-gpufeaturename-texture-compression-unaligned",
      FeatureInfo::FeatureSafety::Safe}},
    {Feature::DawnAllowUndefinedLoadStoreOp,
     {"Allow wgpu::LoadOp::Undefined and wgpu::StoreOp::Undefined to be used for render pass "
      "attachments.",
      "https://dawn.googlesource.com/dawn/+/refs/heads/main/docs/dawn/features/"
      "dawn_allow_undefined_load_store_op.md",
      FeatureInfo::FeatureSafety::Safe}},

    // Comment to separate the } so it is clearer what to copy-paste to add a feature.
});

}  // anonymous namespace

void FeaturesSet::EnableFeature(Feature feature) {
    DAWN_ASSERT(feature != Feature::InvalidEnum);
    featuresBitSet.set(feature);
}

void FeaturesSet::EnableFeature(wgpu::FeatureName feature) {
    EnableFeature(FromCppAPI(feature));
}

bool FeaturesSet::IsEnabled(Feature feature) const {
    DAWN_ASSERT(feature != Feature::InvalidEnum);
    return featuresBitSet[feature];
}

bool FeaturesSet::IsEnabled(wgpu::FeatureName feature) const {
    Feature f = FromCppAPI(feature);
    return f != Feature::InvalidEnum && IsEnabled(f);
}

void FeaturesSet::ToSupportedFeatures(SupportedFeatures* supportedFeatures) const {
    if (!supportedFeatures) {
        return;
    }

    const size_t count = featuresBitSet.count();

    if (count == 0) {
        supportedFeatures->features = {};
        return;
    }

    // This will be freed by wgpuSupportedFeaturesFreeMembers.
    auto features = HeapArray<wgpu::FeatureName>(count);
    uint32_t index = 0;
    for (Feature f : featuresBitSet) {
        features[index++] = ToCppAPI(f);
    }
    DAWN_ASSERT(index == count);

    supportedFeatures->features = std::move(features).MoveToSpan();
}

}  // namespace dawn::native

#include "dawn/native/Features_autogen.inl"
