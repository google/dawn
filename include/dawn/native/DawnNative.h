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

#ifndef INCLUDE_DAWN_NATIVE_DAWNNATIVE_H_
#define INCLUDE_DAWN_NATIVE_DAWNNATIVE_H_

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <string_view>
#include <vector>

#include "dawn/dawn_proc_table.h"
#include "dawn/native/dawn_native_export.h"

namespace dawn::platform {
class Platform;
}  // namespace dawn::platform

namespace dawn::native {

class InstanceBase;
class AdapterBase;

// Each toggle is assigned with a TogglesStage, indicating the validation and earliest usage
// time of the toggle.
enum class ToggleStage { Instance, Adapter, Device };

// A struct to record the information of a toggle. A toggle is a code path in Dawn device that
// can be manually configured to run or not outside Dawn, including workarounds, special
// features and optimizations.
struct ToggleInfo {
    const char* name;
    const char* description;
    const char* url;
    ToggleStage stage;
};

// A struct to record the information of a feature. A feature is a GPU feature that is not
// required to be supported by all Dawn backends and can only be used when it is enabled on the
// creation of device.
struct FeatureInfo {
    const char* name;
    const char* description;
    const char* url;
    // The enum of feature state, could be stable or experimental. Using an experimental feature
    // requires the AllowUnsafeAPIs toggle to be enabled.
    enum class FeatureState { Stable = 0, Experimental };
    FeatureState featureState;
};

// An adapter is an object that represent on possibility of creating devices in the system.
// Most of the time it will represent a combination of a physical GPU and an API. Not that the
// same GPU can be represented by multiple adapters but on different APIs.
//
// The underlying Dawn adapter is owned by the Dawn instance so this class is not RAII but just
// a reference to an underlying adapter.
class DAWN_NATIVE_EXPORT Adapter {
  public:
    Adapter();
    // NOLINTNEXTLINE(runtime/explicit)
    Adapter(AdapterBase* impl);
    ~Adapter();

    Adapter(const Adapter& other);
    Adapter& operator=(const Adapter& other);

    // TODO(crbug.com/347047627): These methods are historical duplicates of
    // those in webgpu_cpp.h. Update uses of these methods and remove them.
    wgpu::Status GetInfo(wgpu::AdapterInfo* info) const;
    wgpu::Status GetInfo(WGPUAdapterInfo* info) const;
    std::vector<const char*> GetSupportedFeatures() const;
    wgpu::ConvertibleStatus GetLimits(WGPUSupportedLimits* limits) const;

    void SetUseTieredLimits(bool useTieredLimits);

    // Check that the Adapter is able to support importing external images. This is necessary
    // to implement the swapchain and interop APIs in Chromium.
    bool SupportsExternalImages() const;

    explicit operator bool() const;

    // Create a device on this adapter. On an error, nullptr is returned.
    WGPUDevice CreateDevice(const wgpu::DeviceDescriptor* deviceDescriptor);
    WGPUDevice CreateDevice(const WGPUDeviceDescriptor* deviceDescriptor = nullptr);

    void RequestDevice(const wgpu::DeviceDescriptor* descriptor,
                       WGPURequestDeviceCallback callback,
                       void* userdata);
    void RequestDevice(const WGPUDeviceDescriptor* descriptor,
                       WGPURequestDeviceCallback callback,
                       void* userdata);
    void RequestDevice(std::nullptr_t descriptor,
                       WGPURequestDeviceCallback callback,
                       void* userdata) {
        RequestDevice(static_cast<const wgpu::DeviceDescriptor*>(descriptor), callback, userdata);
    }

    // Returns the underlying WGPUAdapter object.
    WGPUAdapter Get() const;

    // Reset the backend device object for testing purposes.
    void ResetInternalDeviceForTesting();

  private:
    AdapterBase* mImpl = nullptr;
};

enum BackendValidationLevel { Full, Partial, Disabled };

// Can be chained in InstanceDescriptor
struct DAWN_NATIVE_EXPORT DawnInstanceDescriptor : wgpu::ChainedStruct {
    DawnInstanceDescriptor();
    uint32_t additionalRuntimeSearchPathsCount = 0;
    const char* const* additionalRuntimeSearchPaths;
    dawn::platform::Platform* platform = nullptr;

    BackendValidationLevel backendValidationLevel = BackendValidationLevel::Disabled;
    bool beginCaptureOnStartup = false;

    WGPULoggingCallback loggingCallback = nullptr;
    void* loggingCallbackUserdata = nullptr;

    // Equality operators, mostly for testing. Note that this tests
    // strict pointer-pointer equality if the struct contains member pointers.
    bool operator==(const DawnInstanceDescriptor& rhs) const;
};

// Represents a connection to dawn_native and is used for dependency injection, discovering
// system adapters and injecting custom adapters (like a Swiftshader Vulkan adapter).
//
// This is an RAII class for Dawn instances and also controls the lifetime of all adapters
// for this instance.
class DAWN_NATIVE_EXPORT Instance {
  public:
    explicit Instance(const WGPUInstanceDescriptor* desc = nullptr);
    explicit Instance(InstanceBase* impl);
    ~Instance();

    Instance(const Instance& other) = delete;
    Instance& operator=(const Instance& other) = delete;

    // Discovers and returns a vector of adapters.
    // All systems adapters that can be found are returned if no options are passed.
    // Otherwise, returns adapters based on the `options`. Adapter toggles descriptor can chained
    // after options.
    std::vector<Adapter> EnumerateAdapters(const WGPURequestAdapterOptions* options) const;
    std::vector<Adapter> EnumerateAdapters(
        const wgpu::RequestAdapterOptions* options = nullptr) const;

    const ToggleInfo* GetToggleInfo(const char* toggleName);

    // Enables backend validation layers
    void SetBackendValidationLevel(BackendValidationLevel validationLevel);

    uint64_t GetDeviceCountForTesting() const;
    // Backdoor to get the number of deprecation warnings for testing
    uint64_t GetDeprecationWarningCountForTesting() const;

    // Returns the underlying WGPUInstance object.
    WGPUInstance Get() const;

    // Make mImpl->mPlatform point to an object inside Dawn in case it becomes a dangling pointer
    void DisconnectDawnPlatform();

  private:
    InstanceBase* mImpl = nullptr;
};

// Backend-agnostic API for dawn_native
DAWN_NATIVE_EXPORT const DawnProcTable& GetProcs();

// Query the names of all the toggles that are enabled in device
DAWN_NATIVE_EXPORT std::vector<const char*> GetTogglesUsed(WGPUDevice device);

// Backdoor to get the number of lazy clears for testing
DAWN_NATIVE_EXPORT size_t GetLazyClearCountForTesting(WGPUDevice device);

//  Query if texture has been initialized
DAWN_NATIVE_EXPORT bool IsTextureSubresourceInitialized(
    WGPUTexture texture,
    uint32_t baseMipLevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount,
    WGPUTextureAspect aspect = WGPUTextureAspect_All);

// Backdoor to get the order of the ProcMap for testing
DAWN_NATIVE_EXPORT std::vector<std::string_view> GetProcMapNamesForTesting();

DAWN_NATIVE_EXPORT bool DeviceTick(WGPUDevice device);

DAWN_NATIVE_EXPORT bool InstanceProcessEvents(WGPUInstance instance);

// ErrorInjector functions used for testing only. Defined in dawn_native/ErrorInjector.cpp
DAWN_NATIVE_EXPORT void EnableErrorInjector();
DAWN_NATIVE_EXPORT void DisableErrorInjector();
DAWN_NATIVE_EXPORT void ClearErrorInjector();
DAWN_NATIVE_EXPORT uint64_t AcquireErrorInjectorCallCount();
DAWN_NATIVE_EXPORT void InjectErrorAt(uint64_t index);

// The different types of external images
enum ExternalImageType {
    OpaqueFD,
    DmaBuf,
    IOSurface,
    EGLImage,
    GLTexture,
    AHardwareBuffer,
    Last = AHardwareBuffer,
};

// Common properties of external images
struct DAWN_NATIVE_EXPORT ExternalImageDescriptor {
  public:
    const WGPUTextureDescriptor* cTextureDescriptor;  // Must match image creation params
    bool isInitialized;  // Whether the texture is initialized on import
    ExternalImageType GetType() const;

  protected:
    explicit ExternalImageDescriptor(ExternalImageType type);

  private:
    ExternalImageType mType;
};

struct DAWN_NATIVE_EXPORT ExternalImageExportInfo {
  public:
    bool isInitialized = false;  // Whether the texture is initialized after export
    ExternalImageType GetType() const;

  protected:
    explicit ExternalImageExportInfo(ExternalImageType type);

  private:
    ExternalImageType mType;
};

DAWN_NATIVE_EXPORT bool CheckIsErrorForTesting(void* objectHandle);

DAWN_NATIVE_EXPORT const char* GetObjectLabelForTesting(void* objectHandle);

DAWN_NATIVE_EXPORT uint64_t GetAllocatedSizeForTesting(WGPUBuffer buffer);

DAWN_NATIVE_EXPORT std::vector<const ToggleInfo*> AllToggleInfos();

// Used to query the details of an feature. Return nullptr if featureName is not a valid
// name of an feature supported in Dawn.
DAWN_NATIVE_EXPORT const FeatureInfo* GetFeatureInfo(wgpu::FeatureName feature);

class DAWN_NATIVE_EXPORT MemoryDump {
  public:
    // Standard attribute |name|s for the AddScalar() and AddString() methods.
    // These match the expected names in Chromium memory-infra instrumentation.
    static const char kNameSize[];         // To represent allocated space.
    static const char kNameObjectCount[];  // To represent number of objects.

    // Standard attribute |unit|s for the AddScalar() and AddString() methods.
    // These match the expected names in Chromium memory-infra instrumentation.
    static const char kUnitsBytes[];    // Unit name to represent bytes.
    static const char kUnitsObjects[];  // Unit name to represent #objects.

    MemoryDump() = default;

    virtual void AddScalar(const char* name,
                           const char* key,
                           const char* units,
                           uint64_t value) = 0;

    virtual void AddString(const char* name, const char* key, const std::string& value) = 0;

    MemoryDump(const MemoryDump&) = delete;
    MemoryDump& operator=(const MemoryDump&) = delete;

  protected:
    virtual ~MemoryDump() = default;
};
DAWN_NATIVE_EXPORT void DumpMemoryStatistics(WGPUDevice device, MemoryDump* dump);

// Unlike memory dumps which include detailed information about allocations, this only returns the
// total estimated memory usage, and is intended for background tracing for UMA.
DAWN_NATIVE_EXPORT uint64_t ComputeEstimatedMemoryUsage(WGPUDevice device);

// Free any unused GPU memory like staging buffers, cached resources, etc.
DAWN_NATIVE_EXPORT void ReduceMemoryUsage(WGPUDevice device);

}  // namespace dawn::native

#endif  // INCLUDE_DAWN_NATIVE_DAWNNATIVE_H_
