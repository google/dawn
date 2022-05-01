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

#include "dawn/native/metal/BackendMTL.h"

#include "dawn/common/CoreFoundationRef.h"
#include "dawn/common/GPUInfo.h"
#include "dawn/common/Log.h"
#include "dawn/common/NSRef.h"
#include "dawn/common/Platform.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/native/Instance.h"
#include "dawn/native/MetalBackend.h"
#include "dawn/native/metal/BufferMTL.h"
#include "dawn/native/metal/DeviceMTL.h"

#if defined(DAWN_PLATFORM_MACOS)
#import <IOKit/IOKitLib.h>
#include "dawn/common/IOKitRef.h"
#endif

#include <vector>

namespace dawn::native::metal {

namespace {

struct PCIIDs {
    uint32_t vendorId;
    uint32_t deviceId;
};

struct Vendor {
    const char* trademark;
    uint32_t vendorId;
};

#if defined(DAWN_PLATFORM_MACOS)
const Vendor kVendors[] = {{"AMD", gpu_info::kVendorID_AMD},
                           {"Radeon", gpu_info::kVendorID_AMD},
                           {"Intel", gpu_info::kVendorID_Intel},
                           {"Geforce", gpu_info::kVendorID_Nvidia},
                           {"Quadro", gpu_info::kVendorID_Nvidia}};

// Find vendor ID from MTLDevice name.
MaybeError GetVendorIdFromVendors(id<MTLDevice> device, PCIIDs* ids) {
    uint32_t vendorId = 0;
    const char* deviceName = [device.name UTF8String];
    for (const auto& it : kVendors) {
        if (strstr(deviceName, it.trademark) != nullptr) {
            vendorId = it.vendorId;
            break;
        }
    }

    if (vendorId == 0) {
        return DAWN_INTERNAL_ERROR("Failed to find vendor id with the device");
    }

    // Set vendor id with 0
    *ids = PCIIDs{vendorId, 0};
    return {};
}

// Extracts an integer property from a registry entry.
uint32_t GetEntryProperty(io_registry_entry_t entry, CFStringRef name) {
    uint32_t value = 0;

    // Recursively search registry entry and its parents for property name
    // The data should release with CFRelease
    CFRef<CFDataRef> data = AcquireCFRef(static_cast<CFDataRef>(IORegistryEntrySearchCFProperty(
        entry, kIOServicePlane, name, kCFAllocatorDefault,
        kIORegistryIterateRecursively | kIORegistryIterateParents)));

    if (data == nullptr) {
        return value;
    }

    // CFDataGetBytePtr() is guaranteed to return a read-only pointer
    value = *reinterpret_cast<const uint32_t*>(CFDataGetBytePtr(data.Get()));
    return value;
}

// Queries the IO Registry to find the PCI device and vendor IDs of the MTLDevice.
// The registry entry correponding to [device registryID] doesn't contain the exact PCI ids
// because it corresponds to a driver. However its parent entry corresponds to the device
// itself and has uint32_t "device-id" and "registry-id" keys. For example on a dual-GPU
// MacBook Pro 2017 the IORegistry explorer shows the following tree (simplified here):
//
//  - PCI0@0
//  | - AppleACPIPCI
//  | | - IGPU@2 (type IOPCIDevice)
//  | | | - IntelAccelerator (type IOGraphicsAccelerator2)
//  | | - PEG0@1
//  | | | - IOPP
//  | | | | - GFX0@0 (type IOPCIDevice)
//  | | | | | - AMDRadeonX4000_AMDBaffinGraphicsAccelerator (type IOGraphicsAccelerator2)
//
// [device registryID] is the ID for one of the IOGraphicsAccelerator2 and we can see that
// their parent always is an IOPCIDevice that has properties for the device and vendor IDs.
MaybeError API_AVAILABLE(macos(10.13))
    GetDeviceIORegistryPCIInfo(id<MTLDevice> device, PCIIDs* ids) {
    // Get a matching dictionary for the IOGraphicsAccelerator2
    CFRef<CFMutableDictionaryRef> matchingDict =
        AcquireCFRef(IORegistryEntryIDMatching([device registryID]));
    if (matchingDict == nullptr) {
        return DAWN_INTERNAL_ERROR("Failed to create the matching dict for the device");
    }

    // IOServiceGetMatchingService will consume the reference on the matching dictionary,
    // so we don't need to release the dictionary.
    IORef<io_registry_entry_t> acceleratorEntry =
        AcquireIORef(IOServiceGetMatchingService(kIOMasterPortDefault, matchingDict.Detach()));
    if (acceleratorEntry == IO_OBJECT_NULL) {
        return DAWN_INTERNAL_ERROR("Failed to get the IO registry entry for the accelerator");
    }

    // Get the parent entry that will be the IOPCIDevice
    IORef<io_registry_entry_t> deviceEntry;
    if (IORegistryEntryGetParentEntry(acceleratorEntry.Get(), kIOServicePlane,
                                      deviceEntry.InitializeInto()) != kIOReturnSuccess) {
        return DAWN_INTERNAL_ERROR("Failed to get the IO registry entry for the device");
    }

    ASSERT(deviceEntry != IO_OBJECT_NULL);

    uint32_t vendorId = GetEntryProperty(deviceEntry.Get(), CFSTR("vendor-id"));
    uint32_t deviceId = GetEntryProperty(deviceEntry.Get(), CFSTR("device-id"));

    *ids = PCIIDs{vendorId, deviceId};

    return {};
}

MaybeError GetDevicePCIInfo(id<MTLDevice> device, PCIIDs* ids) {
    // [device registryID] is introduced on macOS 10.13+, otherwise workaround to get vendor
    // id by vendor name on old macOS
    if (@available(macos 10.13, *)) {
        return GetDeviceIORegistryPCIInfo(device, ids);
    } else {
        return GetVendorIdFromVendors(device, ids);
    }
}

bool IsMetalSupported() {
    // Metal was first introduced in macOS 10.11
    // WebGPU is targeted at macOS 10.12+
    // TODO(dawn:1181): Dawn native should allow non-conformant WebGPU on macOS 10.11
    return IsMacOSVersionAtLeast(10, 12);
}
#elif defined(DAWN_PLATFORM_IOS)
MaybeError GetDevicePCIInfo(id<MTLDevice> device, PCIIDs* ids) {
    DAWN_UNUSED(device);
    *ids = PCIIDs{0, 0};
    return {};
}

bool IsMetalSupported() {
    return true;
}
#else
#error "Unsupported Apple platform."
#endif

DAWN_NOINLINE bool IsCounterSamplingBoundarySupport(id<MTLDevice> device)
    API_AVAILABLE(macos(11.0), ios(14.0)) {
    bool isBlitBoundarySupported =
        [device supportsCounterSampling:MTLCounterSamplingPointAtBlitBoundary];
    bool isDispatchBoundarySupported =
        [device supportsCounterSampling:MTLCounterSamplingPointAtDispatchBoundary];
    bool isDrawBoundarySupported =
        [device supportsCounterSampling:MTLCounterSamplingPointAtDrawBoundary];

    return isBlitBoundarySupported && isDispatchBoundarySupported && isDrawBoundarySupported;
}

// This method has seen hard-to-debug crashes. See crbug.com/dawn/1102.
// For now, it is written defensively, with many potentially unnecessary guards until
// we narrow down the cause of the problem.
DAWN_NOINLINE bool IsGPUCounterSupported(id<MTLDevice> device,
                                         MTLCommonCounterSet counterSetName,
                                         std::vector<MTLCommonCounter> counterNames)
    API_AVAILABLE(macos(10.15), ios(14.0)) {
    NSPRef<id<MTLCounterSet>> counterSet = nil;
    if (![device respondsToSelector:@selector(counterSets)]) {
        dawn::ErrorLog() << "MTLDevice does not respond to selector: counterSets.";
        return false;
    }
    NSArray<id<MTLCounterSet>>* counterSets = device.counterSets;
    if (counterSets == nil) {
        // On some systems, [device counterSets] may be null and not an empty array.
        return false;
    }
    // MTLDevice’s counterSets property declares which counter sets it supports. Check
    // whether it's available on the device before requesting a counter set.
    // Note: Don't do for..in loop to avoid potentially crashy interaction with
    // NSFastEnumeration.
    for (NSUInteger i = 0; i < counterSets.count; ++i) {
        id<MTLCounterSet> set = [counterSets objectAtIndex:i];
        if ([set.name caseInsensitiveCompare:counterSetName] == NSOrderedSame) {
            counterSet = set;
            break;
        }
    }

    // The counter set is not supported.
    if (counterSet == nil) {
        return false;
    }

    if (![*counterSet respondsToSelector:@selector(counters)]) {
        dawn::ErrorLog() << "MTLCounterSet does not respond to selector: counters.";
        return false;
    }
    NSArray<id<MTLCounter>>* countersInSet = (*counterSet).counters;
    if (countersInSet == nil) {
        // On some systems, [MTLCounterSet counters] may be null and not an empty array.
        return false;
    }

    // A GPU might support a counter set, but only support a subset of the counters in that
    // set, check if the counter set supports all specific counters we need. Return false
    // if there is a counter unsupported.
    for (MTLCommonCounter counterName : counterNames) {
        bool found = false;
        // Note: Don't do for..in loop to avoid potentially crashy interaction with
        // NSFastEnumeration.
        for (NSUInteger i = 0; i < countersInSet.count; ++i) {
            id<MTLCounter> counter = [countersInSet objectAtIndex:i];
            if ([counter.name caseInsensitiveCompare:counterName] == NSOrderedSame) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    if (@available(macOS 11.0, iOS 14.0, *)) {
        // Check whether it can read GPU counters at the specified command boundary. Apple
        // family GPUs do not support sampling between different Metal commands, because
        // they defer fragment processing until after the GPU processes all the primitives
        // in the render pass.
        if (!IsCounterSamplingBoundarySupport(device)) {
            return false;
        }
    }

    return true;
}

}  // anonymous namespace

// The Metal backend's Adapter.

class Adapter : public AdapterBase {
  public:
    Adapter(InstanceBase* instance, id<MTLDevice> device)
        : AdapterBase(instance, wgpu::BackendType::Metal), mDevice(device) {
        mName = std::string([[*mDevice name] UTF8String]);

        PCIIDs ids;
        if (!instance->ConsumedError(GetDevicePCIInfo(device, &ids))) {
            mVendorId = ids.vendorId;
            mDeviceId = ids.deviceId;
        }

#if defined(DAWN_PLATFORM_IOS)
        mAdapterType = wgpu::AdapterType::IntegratedGPU;
        const char* systemName = "iOS ";
#elif defined(DAWN_PLATFORM_MACOS)
        if ([device isLowPower]) {
            mAdapterType = wgpu::AdapterType::IntegratedGPU;
        } else {
            mAdapterType = wgpu::AdapterType::DiscreteGPU;
        }
        const char* systemName = "macOS ";
#else
#error "Unsupported Apple platform."
#endif

        NSString* osVersion = [[NSProcessInfo processInfo] operatingSystemVersionString];
        mDriverDescription = "Metal driver on " + std::string(systemName) + [osVersion UTF8String];
    }

    // AdapterBase Implementation
    bool SupportsExternalImages() const override {
        // Via dawn::native::metal::WrapIOSurface
        return true;
    }

  private:
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor) override {
        return Device::Create(this, mDevice, descriptor);
    }

    MaybeError InitializeImpl() override { return {}; }

    MaybeError InitializeSupportedFeaturesImpl() override {
        // Check compressed texture format with deprecated MTLFeatureSet way.
#if defined(DAWN_PLATFORM_MACOS)
        if ([*mDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1]) {
            mSupportedFeatures.EnableFeature(Feature::TextureCompressionBC);
        }
#endif
#if defined(DAWN_PLATFORM_IOS)
        if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
            mSupportedFeatures.EnableFeature(Feature::TextureCompressionETC2);
        }
        if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
            mSupportedFeatures.EnableFeature(Feature::TextureCompressionASTC);
        }
#endif

        // Check compressed texture format with MTLGPUFamily
        if (@available(macOS 10.15, iOS 13.0, *)) {
            if ([*mDevice supportsFamily:MTLGPUFamilyMac1]) {
                mSupportedFeatures.EnableFeature(Feature::TextureCompressionBC);
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple2]) {
                mSupportedFeatures.EnableFeature(Feature::TextureCompressionETC2);
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple3]) {
                mSupportedFeatures.EnableFeature(Feature::TextureCompressionASTC);
            }
        }

        if (@available(macOS 10.15, iOS 14.0, *)) {
            if (IsGPUCounterSupported(
                    *mDevice, MTLCommonCounterSetStatistic,
                    {MTLCommonCounterVertexInvocations, MTLCommonCounterClipperInvocations,
                     MTLCommonCounterClipperPrimitivesOut, MTLCommonCounterFragmentInvocations,
                     MTLCommonCounterComputeKernelInvocations})) {
                mSupportedFeatures.EnableFeature(Feature::PipelineStatisticsQuery);
            }

            if (IsGPUCounterSupported(*mDevice, MTLCommonCounterSetTimestamp,
                                      {MTLCommonCounterTimestamp})) {
                bool enableTimestampQuery = true;

#if defined(DAWN_PLATFORM_MACOS)
                // Disable timestamp query on < macOS 11.0 on AMD GPU because WriteTimestamp
                // fails to call without any copy commands on MTLBlitCommandEncoder. This issue
                // has been fixed on macOS 11.0. See crbug.com/dawn/545.
                if (gpu_info::IsAMD(mVendorId) && !IsMacOSVersionAtLeast(11)) {
                    enableTimestampQuery = false;
                }
#endif

                if (enableTimestampQuery) {
                    mSupportedFeatures.EnableFeature(Feature::TimestampQuery);
                }
            }
        }

        if (@available(macOS 10.11, iOS 11.0, *)) {
            mSupportedFeatures.EnableFeature(Feature::DepthClamping);
        }

        if (@available(macOS 10.11, iOS 9.0, *)) {
            mSupportedFeatures.EnableFeature(Feature::Depth32FloatStencil8);
        }

        // Uses newTextureWithDescriptor::iosurface::plane which is available
        // on ios 11.0+ and macOS 11.0+
        if (@available(macOS 10.11, iOS 11.0, *)) {
            mSupportedFeatures.EnableFeature(Feature::MultiPlanarFormats);
        }

#if defined(DAWN_PLATFORM_MACOS)
        // MTLPixelFormatDepth24Unorm_Stencil8 is only available on macOS 10.11+
        if ([*mDevice isDepth24Stencil8PixelFormatSupported]) {
            mSupportedFeatures.EnableFeature(Feature::Depth24UnormStencil8);
        }
#endif

        return {};
    }

    enum class MTLGPUFamily {
        Apple1,
        Apple2,
        Apple3,
        Apple4,
        Apple5,
        Apple6,
        Apple7,
        Mac1,
        Mac2,
    };

    ResultOrError<MTLGPUFamily> GetMTLGPUFamily() const {
        // https://developer.apple.com/documentation/metal/mtldevice/detecting_gpu_features_and_metal_software_versions?language=objc

        if (@available(macOS 10.15, iOS 10.13, *)) {
            if ([*mDevice supportsFamily:MTLGPUFamilyMac2]) {
                return MTLGPUFamily::Mac2;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyMac1]) {
                return MTLGPUFamily::Mac1;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple7]) {
                return MTLGPUFamily::Apple7;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple6]) {
                return MTLGPUFamily::Apple6;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple5]) {
                return MTLGPUFamily::Apple5;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple4]) {
                return MTLGPUFamily::Apple4;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple3]) {
                return MTLGPUFamily::Apple3;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple2]) {
                return MTLGPUFamily::Apple2;
            }
            if ([*mDevice supportsFamily:MTLGPUFamilyApple1]) {
                return MTLGPUFamily::Apple1;
            }
        }

#if TARGET_OS_OSX
        if (@available(macOS 10.14, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily2_v1]) {
                return MTLGPUFamily::Mac2;
            }
        }
        if (@available(macOS 10.11, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_macOS_GPUFamily1_v1]) {
                return MTLGPUFamily::Mac1;
            }
        }
#elif TARGET_OS_IOS
        if (@available(iOS 10.11, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily4_v1]) {
                return MTLGPUFamily::Apple4;
            }
        }
        if (@available(iOS 9.0, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily3_v1]) {
                return MTLGPUFamily::Apple3;
            }
        }
        if (@available(iOS 8.0, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily2_v1]) {
                return MTLGPUFamily::Apple2;
            }
        }
        if (@available(iOS 8.0, *)) {
            if ([*mDevice supportsFeatureSet:MTLFeatureSet_iOS_GPUFamily1_v1]) {
                return MTLGPUFamily::Apple1;
            }
        }
#endif
        return DAWN_INTERNAL_ERROR("Unsupported Metal device");
    }

    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override {
        struct MTLDeviceLimits {
            uint32_t maxVertexAttribsPerDescriptor;
            uint32_t maxBufferArgumentEntriesPerFunc;
            uint32_t maxTextureArgumentEntriesPerFunc;
            uint32_t maxSamplerStateArgumentEntriesPerFunc;
            uint32_t maxThreadsPerThreadgroup;
            uint32_t maxTotalThreadgroupMemory;
            uint32_t maxFragmentInputComponents;
            uint32_t max1DTextureSize;
            uint32_t max2DTextureSize;
            uint32_t max3DTextureSize;
            uint32_t maxTextureArrayLayers;
            uint32_t minBufferOffsetAlignment;
        };

        struct LimitsForFamily {
            uint32_t MTLDeviceLimits::*limit;
            ityp::array<MTLGPUFamily, uint32_t, 9> values;
        };

        // clang-format off
            // https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
            //                                                               Apple                                                      Mac
            //                                                                   1,      2,      3,      4,      5,      6,      7,       1,      2
            constexpr LimitsForFamily kMTLLimits[12] = {
                {&MTLDeviceLimits::maxVertexAttribsPerDescriptor,         {    31u,    31u,    31u,    31u,    31u,    31u,    31u,     31u,    31u }},
                {&MTLDeviceLimits::maxBufferArgumentEntriesPerFunc,       {    31u,    31u,    31u,    31u,    31u,    31u,    31u,     31u,    31u }},
                {&MTLDeviceLimits::maxTextureArgumentEntriesPerFunc,      {    31u,    31u,    31u,    96u,    96u,   128u,   128u,    128u,   128u }},
                {&MTLDeviceLimits::maxSamplerStateArgumentEntriesPerFunc, {    16u,    16u,    16u,    16u,    16u,    16u,    16u,     16u,    16u }},
                {&MTLDeviceLimits::maxThreadsPerThreadgroup,              {   512u,   512u,   512u,  1024u,  1024u,  1024u,  1024u,   1024u,  1024u }},
                {&MTLDeviceLimits::maxTotalThreadgroupMemory,             { 16352u, 16352u, 16384u, 32768u, 32768u, 32768u, 32768u,  32768u, 32768u }},
                {&MTLDeviceLimits::maxFragmentInputComponents,            {    60u,    60u,    60u,   124u,   124u,   124u,   124u,    124u,   124u }},
                {&MTLDeviceLimits::max1DTextureSize,                      {  8192u,  8192u, 16384u, 16384u, 16384u, 16384u, 16384u,  16384u, 16384u }},
                {&MTLDeviceLimits::max2DTextureSize,                      {  8192u,  8192u, 16384u, 16384u, 16384u, 16384u, 16384u,  16384u, 16384u }},
                {&MTLDeviceLimits::max3DTextureSize,                      {  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,   2048u,  2048u }},
                {&MTLDeviceLimits::maxTextureArrayLayers,                 {  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,  2048u,   2048u,  2048u }},
                {&MTLDeviceLimits::minBufferOffsetAlignment,              {     4u,     4u,     4u,     4u,     4u,     4u,     4u,    256u,   256u }},
            };
        // clang-format on

        MTLGPUFamily mtlGPUFamily;
        DAWN_TRY_ASSIGN(mtlGPUFamily, GetMTLGPUFamily());

        MTLDeviceLimits mtlLimits;
        for (const auto& limitsForFamily : kMTLLimits) {
            mtlLimits.*limitsForFamily.limit = limitsForFamily.values[mtlGPUFamily];
        }

        GetDefaultLimits(&limits->v1);

        limits->v1.maxTextureDimension1D = mtlLimits.max1DTextureSize;
        limits->v1.maxTextureDimension2D = mtlLimits.max2DTextureSize;
        limits->v1.maxTextureDimension3D = mtlLimits.max3DTextureSize;
        limits->v1.maxTextureArrayLayers = mtlLimits.maxTextureArrayLayers;

        uint32_t maxBuffersPerStage = mtlLimits.maxBufferArgumentEntriesPerFunc;
        maxBuffersPerStage -= 1;  // One slot is reserved to store buffer lengths.

        uint32_t baseMaxBuffersPerStage = limits->v1.maxStorageBuffersPerShaderStage +
                                          limits->v1.maxUniformBuffersPerShaderStage +
                                          limits->v1.maxVertexBuffers;

        ASSERT(maxBuffersPerStage >= baseMaxBuffersPerStage);
        {
            uint32_t additional = maxBuffersPerStage - baseMaxBuffersPerStage;
            limits->v1.maxStorageBuffersPerShaderStage += additional / 3;
            limits->v1.maxUniformBuffersPerShaderStage += additional / 3;
            limits->v1.maxVertexBuffers += (additional - 2 * (additional / 3));
        }

        uint32_t baseMaxTexturesPerStage = limits->v1.maxSampledTexturesPerShaderStage +
                                           limits->v1.maxStorageTexturesPerShaderStage;

        ASSERT(mtlLimits.maxTextureArgumentEntriesPerFunc >= baseMaxTexturesPerStage);
        {
            uint32_t additional =
                mtlLimits.maxTextureArgumentEntriesPerFunc - baseMaxTexturesPerStage;
            limits->v1.maxSampledTexturesPerShaderStage += additional / 2;
            limits->v1.maxStorageTexturesPerShaderStage += (additional - additional / 2);
        }

        limits->v1.maxSamplersPerShaderStage = mtlLimits.maxSamplerStateArgumentEntriesPerFunc;

        // Metal limits are per-function, so the layout limits are the same as the stage
        // limits. Note: this should likely change if the implementation uses Metal argument
        // buffers. Non-dynamic buffers will probably be bound argument buffers, but dynamic
        // buffers may be set directly.
        //   Mac GPU families with tier 1 argument buffers support 64
        //   buffers, 128 textures, and 16 samplers. Mac GPU families
        //   with tier 2 argument buffers support 500000 buffers and
        //   textures, and 1024 unique samplers
        limits->v1.maxDynamicUniformBuffersPerPipelineLayout =
            limits->v1.maxUniformBuffersPerShaderStage;
        limits->v1.maxDynamicStorageBuffersPerPipelineLayout =
            limits->v1.maxStorageBuffersPerShaderStage;

        // The WebGPU limit is the limit across all vertex buffers, combined.
        limits->v1.maxVertexAttributes =
            limits->v1.maxVertexBuffers * mtlLimits.maxVertexAttribsPerDescriptor;

        limits->v1.maxInterStageShaderComponents = mtlLimits.maxFragmentInputComponents;

        limits->v1.maxComputeWorkgroupStorageSize = mtlLimits.maxTotalThreadgroupMemory;
        limits->v1.maxComputeInvocationsPerWorkgroup = mtlLimits.maxThreadsPerThreadgroup;
        limits->v1.maxComputeWorkgroupSizeX = mtlLimits.maxThreadsPerThreadgroup;
        limits->v1.maxComputeWorkgroupSizeY = mtlLimits.maxThreadsPerThreadgroup;
        limits->v1.maxComputeWorkgroupSizeZ = mtlLimits.maxThreadsPerThreadgroup;

        limits->v1.minUniformBufferOffsetAlignment = mtlLimits.minBufferOffsetAlignment;
        limits->v1.minStorageBufferOffsetAlignment = mtlLimits.minBufferOffsetAlignment;

        uint64_t maxBufferSize = Buffer::QueryMaxBufferLength(*mDevice);

        // Metal has no documented limit on the size of a binding. Use the maximum
        // buffer size.
        limits->v1.maxUniformBufferBindingSize = maxBufferSize;
        limits->v1.maxStorageBufferBindingSize = maxBufferSize;

        // TODO(crbug.com/dawn/685):
        // LIMITS NOT SET:
        // - maxBindGroups
        // - maxVertexBufferArrayStride

        return {};
    }

    NSPRef<id<MTLDevice>> mDevice;
};

// Implementation of the Metal backend's BackendConnection

Backend::Backend(InstanceBase* instance) : BackendConnection(instance, wgpu::BackendType::Metal) {
    if (GetInstance()->IsBackendValidationEnabled()) {
        setenv("METAL_DEVICE_WRAPPER_TYPE", "1", 1);
    }
}

std::vector<Ref<AdapterBase>> Backend::DiscoverDefaultAdapters() {
    AdapterDiscoveryOptions options;
    auto result = DiscoverAdapters(&options);
    if (result.IsError()) {
        GetInstance()->ConsumedError(result.AcquireError());
        return {};
    }
    return result.AcquireSuccess();
}

ResultOrError<std::vector<Ref<AdapterBase>>> Backend::DiscoverAdapters(
    const AdapterDiscoveryOptionsBase* optionsBase) {
    ASSERT(optionsBase->backendType == WGPUBackendType_Metal);

    std::vector<Ref<AdapterBase>> adapters;
    BOOL supportedVersion = NO;
#if defined(DAWN_PLATFORM_MACOS)
    if (@available(macOS 10.11, *)) {
        supportedVersion = YES;

        NSRef<NSArray<id<MTLDevice>>> devices = AcquireNSRef(MTLCopyAllDevices());

        for (id<MTLDevice> device in devices.Get()) {
            Ref<Adapter> adapter = AcquireRef(new Adapter(GetInstance(), device));
            if (!GetInstance()->ConsumedError(adapter->Initialize())) {
                adapters.push_back(std::move(adapter));
            }
        }
    }
#endif

#if defined(DAWN_PLATFORM_IOS)
    if (@available(iOS 8.0, *)) {
        supportedVersion = YES;
        // iOS only has a single device so MTLCopyAllDevices doesn't exist there.
        Ref<Adapter> adapter =
            AcquireRef(new Adapter(GetInstance(), MTLCreateSystemDefaultDevice()));
        if (!GetInstance()->ConsumedError(adapter->Initialize())) {
            adapters.push_back(std::move(adapter));
        }
    }
#endif
    if (!supportedVersion) {
        UNREACHABLE();
    }
    return adapters;
}

BackendConnection* Connect(InstanceBase* instance) {
    if (!IsMetalSupported()) {
        return nullptr;
    }
    return new Backend(instance);
}

}  // namespace dawn::native::metal
