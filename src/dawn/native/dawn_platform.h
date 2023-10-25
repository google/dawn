// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_DAWN_PLATFORM_H_
#define SRC_DAWN_NATIVE_DAWN_PLATFORM_H_

// Use webgpu_cpp to have the enum and bitfield definitions
#include "dawn/webgpu_cpp.h"

#include "dawn/native/dawn_platform_autogen.h"

namespace dawn::native {

// kEnumCount is a constant specifying the number of enums in a WebGPU enum type,
// if the enums are contiguous, making it suitable for iteration.
// It is defined in dawn_platform_autogen.h
template <typename T>
constexpr uint32_t kEnumCount = EnumCount<T>::value;

// Extra buffer usages
// Add an extra buffer usage and an extra binding type for binding the buffers with QueryResolve
// usage as storage buffer in the internal pipeline.
static constexpr wgpu::BufferUsage kInternalStorageBuffer =
    static_cast<wgpu::BufferUsage>(1u << 31);

// Add an extra buffer usage (readonly storage buffer usage) for render pass resource tracking
static constexpr wgpu::BufferUsage kReadOnlyStorageBuffer =
    static_cast<wgpu::BufferUsage>(1u << 30);

static constexpr wgpu::BufferUsage kAllInternalBufferUsages =
    kInternalStorageBuffer | kReadOnlyStorageBuffer;

// Extra texture usages
// Usage to denote an extra tag value used in system specific ways.
//  - Used to store that attachments are used more than once in PassResourceUsageTracker.
//  - Used to store mixed read-only vs. not depth-stencil layouts in Vulkan.
static constexpr wgpu::TextureUsage kReservedTextureUsage =
    static_cast<wgpu::TextureUsage>(1u << 31);

// Add an extra texture usage for textures that will be presented, for use in backends
// that needs to transition to present usage.
static constexpr wgpu::TextureUsage kPresentTextureUsage =
    static_cast<wgpu::TextureUsage>(1u << 30);

// Add an extra texture usage (readonly render attachment usage) for render pass resource
// tracking
static constexpr wgpu::TextureUsage kReadOnlyRenderAttachment =
    static_cast<wgpu::TextureUsage>(1u << 29);

// Add an extra texture usage (readonly storage texture usage) for resource tracking
static constexpr wgpu::TextureUsage kReadOnlyStorageTexture =
    static_cast<wgpu::TextureUsage>(1u << 28);

// Add an extra texture usage (writeonly storage texture usage) for resource tracking
static constexpr wgpu::TextureUsage kWriteOnlyStorageTexture =
    static_cast<wgpu::TextureUsage>(1u << 27);

// Add an extra texture usage (load resolve texture to MSAA) for render pass resource tracking
static constexpr wgpu::TextureUsage kResolveAttachmentLoadingUsage =
    static_cast<wgpu::TextureUsage>(1u << 26);

// Extra BufferBindingType for internal storage buffer binding.
static constexpr wgpu::BufferBindingType kInternalStorageBufferBinding =
    static_cast<wgpu::BufferBindingType>(~0u);

// Extra TextureSampleType for sampling from a resolve attachment.
static constexpr wgpu::TextureSampleType kInternalResolveAttachmentSampleType =
    static_cast<wgpu::TextureSampleType>(~0u);
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_DAWN_PLATFORM_H_
