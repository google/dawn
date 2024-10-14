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

#include "dawn/fuzzers/DawnWireServerFuzzer.h"

#include <webgpu/webgpu_cpp.h>

#include <fstream>
#include <memory>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/DynamicLib.h"
#include "dawn/common/Log.h"
#include "dawn/common/StringViewUtils.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"
#include "dawn/utils/SystemUtils.h"
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

// We need this static function pointer to make AdapterSupported accessible in
// instanceRequestAdapter
static bool (*sAdapterSupported)(const dawn::native::Adapter&) = nullptr;
#if DAWN_PLATFORM_IS(WINDOWS) && defined(ADDRESS_SANITIZER)
static dawn::DynamicLib sVulkanLoader;
#endif

}  // namespace

int DawnWireServerFuzzer::Initialize(int* argc, char*** argv) {
    // TODO(crbug.com/1038952): Although we keep a static instance, when discovering Vulkan
    // adapters, if no adapter is found, the vulkan loader DLL will be loaded and then unloaded,
    // resulting in ASAN false positives. We work around this by explicitly loading the loader
    // without unloading it here.
#if DAWN_PLATFORM_IS(WINDOWS) && defined(ADDRESS_SANITIZER)
    sVulkanLoader.Open(dawn::GetExecutableDirectory().value_or("") + "vulkan-1.dll");
#endif
    return 0;
}

int DawnWireServerFuzzer::Run(const uint8_t* data,
                              size_t size,
                              bool (*AdapterSupported)(const dawn::native::Adapter&),
                              bool supportsErrorInjection) {
    std::unique_ptr<dawn::native::Instance> instance = std::make_unique<dawn::native::Instance>();

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
    // TODO: crbug.com/42241461 - Remove overrides once older entry points are deprecated.
    static constexpr auto RequestAdapter = [](WGPUInstance cInstance,
                                              WGPURequestAdapterCallback2 callback, void* userdata1,
                                              void* userdata2) -> WGPUFuture {
        std::vector<dawn::native::Adapter> adapters =
            dawn::native::Instance(reinterpret_cast<dawn::native::InstanceBase*>(cInstance))
                .EnumerateAdapters();
        for (dawn::native::Adapter adapter : adapters) {
            if (sAdapterSupported(adapter)) {
                WGPUAdapter cAdapter = adapter.Get();
                dawn::native::GetProcs().adapterAddRef(cAdapter);
                callback(WGPURequestAdapterStatus_Success, cAdapter, dawn::kEmptyOutputStringView,
                         userdata1, userdata2);
                return {};
            }
        }
        callback(WGPURequestAdapterStatus_Unavailable, nullptr,
                 dawn::ToOutputStringView("No supported adapter."), userdata1, userdata2);
        return {};
    };
    procs.instanceRequestAdapter = [](WGPUInstance cInstance, const WGPURequestAdapterOptions*,
                                      WGPURequestAdapterCallback callback, void* userdata) {
        RequestAdapter(
            cInstance,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message,
               void* callback, void* userdata) {
                auto cb = reinterpret_cast<WGPURequestAdapterCallback>(callback);
                cb(status, adapter, message, userdata);
            },
            reinterpret_cast<void*>(callback), userdata);
    };
    procs.instanceRequestAdapterF = [](WGPUInstance cInstance, const WGPURequestAdapterOptions*,
                                       WGPURequestAdapterCallbackInfo callbackInfo) -> WGPUFuture {
        return RequestAdapter(
            cInstance,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, WGPUStringView message,
               void* callback, void* userdata) {
                auto cb = reinterpret_cast<WGPURequestAdapterCallback>(callback);
                cb(status, adapter, message, userdata);
            },
            reinterpret_cast<void*>(callbackInfo.callback), callbackInfo.userdata);
    };
    procs.instanceRequestAdapter2 = [](WGPUInstance cInstance, const WGPURequestAdapterOptions*,
                                       WGPURequestAdapterCallbackInfo2 callbackInfo) -> WGPUFuture {
        return RequestAdapter(cInstance, callbackInfo.callback, callbackInfo.userdata1,
                              callbackInfo.userdata2);
    };

    dawnProcSetProcs(&procs);

    DevNull devNull;
    dawn::wire::WireServerDescriptor serverDesc = {};
    serverDesc.procs = &procs;
    serverDesc.serializer = &devNull;

    std::unique_ptr<dawn::wire::WireServer> wireServer(new dawn::wire::WireServer(serverDesc));
    wireServer->InjectInstance(instance->Get(), {1, 0});
    wireServer->HandleCommands(reinterpret_cast<const char*>(data), size);
    return 0;
}
