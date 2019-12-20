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

#include "DawnWireServerFuzzer.h"

#include "common/Assert.h"
#include "dawn/dawn_proc.h"
#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn_wire/WireServer.h"

#include <vector>

namespace {

    class DevNull : public dawn_wire::CommandSerializer {
      public:
        void* GetCmdSpace(size_t size) override {
            if (size > buf.size()) {
                buf.resize(size);
            }
            return buf.data();
        }
        bool Flush() override {
            return true;
        }

      private:
        std::vector<char> buf;
    };

    WGPUProcDeviceCreateSwapChain sOriginalDeviceCreateSwapChain = nullptr;

    WGPUSwapChain ErrorDeviceCreateSwapChain(WGPUDevice device, const WGPUSwapChainDescriptor*) {
        WGPUSwapChainDescriptor desc;
        desc.nextInChain = nullptr;
        desc.label = nullptr;
        // A 0 implementation will trigger a swapchain creation error.
        desc.implementation = 0;
        return sOriginalDeviceCreateSwapChain(device, &desc);
    }

}  // namespace

int DawnWireServerFuzzer::Run(const uint8_t* data, size_t size, MakeDeviceFn MakeDevice) {
    DawnProcTable procs = dawn_native::GetProcs();

    // Swapchains receive a pointer to an implementation. The fuzzer will pass garbage in so we
    // intercept calls to create swapchains and make sure they always return error swapchains.
    // This is ok for fuzzing because embedders of dawn_wire would always define their own
    // swapchain handling.
    sOriginalDeviceCreateSwapChain = procs.deviceCreateSwapChain;
    procs.deviceCreateSwapChain = ErrorDeviceCreateSwapChain;

    dawnProcSetProcs(&procs);

    std::unique_ptr<dawn_native::Instance> instance = std::make_unique<dawn_native::Instance>();
    wgpu::Device device = MakeDevice(instance.get());
    ASSERT(device);

    DevNull devNull;
    dawn_wire::WireServerDescriptor serverDesc = {};
    serverDesc.device = device.Get();
    serverDesc.procs = &procs;
    serverDesc.serializer = &devNull;

    std::unique_ptr<dawn_wire::WireServer> wireServer(new dawn_wire::WireServer(serverDesc));

    wireServer->HandleCommands(reinterpret_cast<const char*>(data), size);

    // Fake waiting for all previous commands before destroying the server.
    device.Tick();

    // Destroy the server before the device because it needs to free all objects.
    wireServer = nullptr;
    device = nullptr;
    instance = nullptr;

    return 0;
}
