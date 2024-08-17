// Copyright 2024 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//
// This file and library_webgpu.js together implement <webgpu/webgpu.h>.
//

#include <webgpu/webgpu.h>

#include <array>
#include <atomic>
#include <cassert>
#include <cstdlib>

// ----------------------------------------------------------------------------
// Declarations for JS emwgpu functions (defined in library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {
void emwgpuDelete(void* id);
}  // extern "C"

// ----------------------------------------------------------------------------
// Implementation details that are not exposed upwards in the API.
// ----------------------------------------------------------------------------

class NonCopyable {
 protected:
  constexpr NonCopyable() = default;
  ~NonCopyable() = default;

  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(NonCopyable&&) = default;

 private:
  NonCopyable(const NonCopyable&) = delete;
  void operator=(const NonCopyable&) = delete;
};

class RefCounted : public NonCopyable {
 public:
  RefCounted() = default;
  virtual ~RefCounted() = default;

  void AddRef() {
    assert(mRefCount.fetch_add(1u, std::memory_order_relaxed) >= 1);
  }

  void Release() {
    if (mRefCount.fetch_sub(1u, std::memory_order_release) == 1u) {
      emwgpuDelete(this);
      delete this;
    }
  }

 private:
  std::atomic<uint64_t> mRefCount = 1;
};

// clang-format off
// X Macro to help generate boilerplate code for all refcounted object types.
#define WGPU_REFCOUNTED_OBJECTS(X) \
  X(Adapter)             \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(Buffer)              \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(Device)              \
  X(Instance)            \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(Queue)               \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(ShaderModule)        \
  X(Surface)             \
  X(SwapChain)           \
  X(Texture)             \
  X(TextureView)

// X Macro to help generate boilerplate code for all passthrough object types.
// Passthrough objects refer to objects that are implemented via JS objects.
#define WGPU_PASSTHROUGH_OBJECTS(X) \
  X(Adapter)             \
  X(BindGroup)           \
  X(BindGroupLayout)     \
  X(Buffer)              \
  X(CommandBuffer)       \
  X(CommandEncoder)      \
  X(ComputePassEncoder)  \
  X(ComputePipeline)     \
  X(PipelineLayout)      \
  X(QuerySet)            \
  X(Queue)               \
  X(RenderBundle)        \
  X(RenderBundleEncoder) \
  X(RenderPassEncoder)   \
  X(RenderPipeline)      \
  X(Sampler)             \
  X(ShaderModule)        \
  X(Surface)             \
  X(SwapChain)           \
  X(Texture)             \
  X(TextureView)
// clang-format on

// Default struct implementations.
#define DEFINE_WGPU_DEFAULT_STRUCT(Name) \
  struct WGPU##Name##Impl : public RefCounted {};
WGPU_PASSTHROUGH_OBJECTS(DEFINE_WGPU_DEFAULT_STRUCT)

// Specialized struct implementations
struct WGPUInstanceImpl : public RefCounted {};

// Device is specially implemented in order to handle refcounting the Queue.
struct WGPUDeviceImpl : public RefCounted {
 public:
  WGPUDeviceImpl(WGPUQueue queue) : mQueue(queue) {
    // TODO(lokokung) Currently we are manually doing the ref counting for
    //                the Queue. We should probably have some RAII helpers.
    mQueue->AddRef();
  }
  ~WGPUDeviceImpl() override { mQueue->Release(); }

  WGPUQueue GetQueue() { return mQueue; }

 private:
  WGPUQueue mQueue;
};

// ----------------------------------------------------------------------------
// Definitions for C++ emwgpu functions (callable from library_webgpu.js)
// ----------------------------------------------------------------------------
extern "C" {

// Object creation helpers that all return a pointer which is used as a key
// in the JS object table in library_webgpu.js.
#define DEFINE_EMWGPU_DEFAULT_CREATE(Name) \
  WGPU##Name emwgpuCreate##Name() {        \
    return new WGPU##Name##Impl();         \
  }
WGPU_PASSTHROUGH_OBJECTS(DEFINE_EMWGPU_DEFAULT_CREATE)

WGPUDevice emwgpuCreateDevice(WGPUQueue queue) {
  return new WGPUDeviceImpl(queue);
}

}  // extern "C"

// ----------------------------------------------------------------------------
// WebGPU function definitions, with methods organized by "class". Note these
// don't need to be extern "C" because they are already declared in webgpu.h.
//
// Also note that the full set of functions declared in webgpu.h are only
// partially implemeted here. The remaining ones are implemented via
// library_webgpu.js.
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Common AddRef/Release APIs are batch generated via X macros for all objects.
// ----------------------------------------------------------------------------

#define DEFINE_WGPU_DEFAULT_ADDREF_RELEASE(Name) \
  void wgpu##Name##AddRef(WGPU##Name o) {        \
    o->AddRef();                                 \
  }                                              \
  void wgpu##Name##Release(WGPU##Name o) {       \
    o->Release();                                \
  }
WGPU_REFCOUNTED_OBJECTS(DEFINE_WGPU_DEFAULT_ADDREF_RELEASE)

// ----------------------------------------------------------------------------
// Standalone (non-method) functions
// ----------------------------------------------------------------------------

void wgpuAdapterInfoFreeMembers(WGPUAdapterInfo value) {
  free(const_cast<char*>(value.vendor));
  free(const_cast<char*>(value.architecture));
  free(const_cast<char*>(value.device));
  free(const_cast<char*>(value.description));
}

WGPUInstance wgpuCreateInstance([[maybe_unused]] const WGPUInstanceDescriptor* descriptor) {
  assert(descriptor == nullptr);  // descriptor not implemented yet
  return new WGPUInstanceImpl();
}

void wgpuSurfaceCapabilitiesFreeMembers(WGPUSurfaceCapabilities) {
  // wgpuSurfaceCapabilities doesn't currently allocate anything.
}

// ----------------------------------------------------------------------------
// Methods of Adapter
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of BindGroup
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of BindGroupLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Buffer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of CommandBuffer
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of CommandEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ComputePipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Device
// ----------------------------------------------------------------------------

WGPUQueue wgpuDeviceGetQueue(WGPUDevice device) {
  device->GetQueue()->AddRef();
  return device->GetQueue();
}

// ----------------------------------------------------------------------------
// Methods of Instance
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of PipelineLayout
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of QuerySet
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Queue
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderBundle
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderBundleEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPassEncoder
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of RenderPipeline
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Sampler
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of ShaderModule
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Surface
// ----------------------------------------------------------------------------

WGPUStatus wgpuSurfaceGetCapabilities(WGPUSurface surface,
                                      WGPUAdapter adapter,
                                      WGPUSurfaceCapabilities* capabilities) {
  assert(capabilities->nextInChain == nullptr); // TODO: Return WGPUStatus_Error

  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsRGBAFirst = {
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  static constexpr std::array<WGPUTextureFormat, 3> kSurfaceFormatsBGRAFirst = {
    WGPUTextureFormat_BGRA8Unorm,
    WGPUTextureFormat_RGBA8Unorm,
    WGPUTextureFormat_RGBA16Float,
  };
  WGPUTextureFormat preferredFormat = wgpuSurfaceGetPreferredFormat(surface, adapter);
  switch (preferredFormat) {
    case WGPUTextureFormat_RGBA8Unorm:
      capabilities->formatCount = kSurfaceFormatsRGBAFirst.size();
      capabilities->formats = kSurfaceFormatsRGBAFirst.data();
      break;
    case WGPUTextureFormat_BGRA8Unorm:
      capabilities->formatCount = kSurfaceFormatsBGRAFirst.size();
      capabilities->formats = kSurfaceFormatsBGRAFirst.data();
      break;
    default:
      assert(false);
      return WGPUStatus_Error;
  }

  {
    static constexpr WGPUPresentMode kPresentMode = WGPUPresentMode_Fifo;
    capabilities->presentModeCount = 1;
    capabilities->presentModes = &kPresentMode;
  }

  {
    static constexpr std::array<WGPUCompositeAlphaMode, 2> kAlphaModes = {
      WGPUCompositeAlphaMode_Opaque,
      WGPUCompositeAlphaMode_Premultiplied,
    };
    capabilities->alphaModeCount = kAlphaModes.size();
    capabilities->alphaModes = kAlphaModes.data();
  }

  return WGPUStatus_Success;
}

// ----------------------------------------------------------------------------
// Methods of SwapChain
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of Texture
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
// Methods of TextureView
// ----------------------------------------------------------------------------
