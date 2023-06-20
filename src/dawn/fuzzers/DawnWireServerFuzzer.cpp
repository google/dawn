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

#include "dawn/fuzzers/DawnWireServerFuzzer.h"

#include <fstream>
#include <memory>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/DynamicLib.h"
#include "dawn/common/Log.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/SystemUtils.h"
#include "dawn/webgpu_cpp.h"
#include "dawn/wire/WireServer.h"

namespace {

class DevNull : public dawn::wire::CommandSerializer {
  public:
    size_t GetMaximumAllocationSize() const override {
        // Some fuzzer bots have a 2GB allocation limit. Pick a value reasonably below that.
        return 1024 * 1024 * 1024;
    }
    void* GetCmdSpace(size_t size) override {
        if (size > buf.size()) {
            buf.resize(size);
        }
        return buf.data();
    }
    bool Flush() override { return true; }

  private:
    std::vector<char> buf;
};

std::unique_ptr<dawn::native::Instance> sInstance;
static bool (*sAdapterSupported)(const dawn::native::Adapter&) = nullptr;
#if DAWN_PLATFORM_IS(WINDOWS) && defined(ADDRESS_SANITIZER)
static dawn::DynamicLib sVulkanLoader;
#endif

}  // namespace

int DawnWireServerFuzzer::Initialize(int* argc, char*** argv) {
    // TODO(crbug.com/1038952): The Instance must be static because destructing the vkInstance with
    // Swiftshader crashes libFuzzer. When this is fixed, move this into Run so that error injection
    // for adapter discovery can be fuzzed.
    sInstance = std::make_unique<dawn::native::Instance>();

    // TODO(crbug.com/1038952): Although we keep a static instance, when discovering default Vulkan
    // adapters, if no adapter is found, the vulkan loader DLL will be loaded and then unloaded,
    // resulting in ASAN false positives. We work around this by explicitly loading the loader
    // without unloading it here.
#if DAWN_PLATFORM_IS(WINDOWS) && defined(ADDRESS_SANITIZER)
    sVulkanLoader.Open(dawn::GetExecutableDirectory().value_or("") + "vulkan-1.dll");
#endif

    sInstance->DiscoverDefaultPhysicalDevices();

    return 0;
}

int DawnWireServerFuzzer::Run(const uint8_t* data,
                              size_t size,
                              bool (*AdapterSupported)(const dawn::native::Adapter&),
                              bool supportsErrorInjection) {
    // We require at least the injected error index.
    if (size < sizeof(uint64_t)) {
        return 0;
    }

    // Get and consume the injected error index.
    uint64_t injectedErrorIndex = *reinterpret_cast<const uint64_t*>(data);
    data += sizeof(uint64_t);
    size -= sizeof(uint64_t);

    if (supportsErrorInjection) {
        dawn::native::EnableErrorInjector();

        // Clear the error injector since it has the previous run's call counts.
        dawn::native::ClearErrorInjector();

        dawn::native::InjectErrorAt(injectedErrorIndex);
    }

    sAdapterSupported = AdapterSupported;

    DawnProcTable procs = dawn::native::GetProcs();

    // Override requestAdapter to find an adapter that the fuzzer supports.
    procs.instanceRequestAdapter = [](WGPUInstance cInstance,
                                      const WGPURequestAdapterOptions* options,
                                      WGPURequestAdapterCallback callback, void* userdata) {
        std::vector<dawn::native::Adapter> adapters = sInstance->EnumerateAdapters();
        for (dawn::native::Adapter adapter : adapters) {
            if (sAdapterSupported(adapter)) {
                WGPUAdapter cAdapter = adapter.Get();
                dawn::native::GetProcs().adapterReference(cAdapter);
                callback(WGPURequestAdapterStatus_Success, cAdapter, nullptr, userdata);
                return;
            }
        }
        callback(WGPURequestAdapterStatus_Unavailable, nullptr, "No supported adapter.", userdata);
    };

    dawnProcSetProcs(&procs);

    DevNull devNull;
    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &procs;
    serverDesc.serializer = &devNull;

    std::unique_ptr<dawn::wire::WireServer> wireServer(new dawn_wire::WireServer(serverDesc));
    wireServer->InjectInstance(sInstance->Get(), 1, 0);
    wireServer->HandleCommands(reinterpret_cast<const char*>(data), size);

    // Flush remaining callbacks to avoid memory leaks.
    // TODO(crbug.com/dawn/1712): DeviceNull's APITick() will always return true so cannot
    // do a polling loop here.
    dawn::native::InstanceProcessEvents(sInstance->Get());

    // Note: Deleting the server will release all created objects.
    // Deleted devices will wait for idle on destruction.
    wireServer = nullptr;
    return 0;
}
