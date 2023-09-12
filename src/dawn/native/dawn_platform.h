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
// Internal usage to help tracking when a subresource is used as render attachment usage
// more than once in a render pass.
static constexpr wgpu::TextureUsage kAgainAsAttachment =
    static_cast<wgpu::TextureUsage>((1u << 31) + 1);

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
