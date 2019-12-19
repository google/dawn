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
#include "dawn_native/DawnNative.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    return DawnWireServerFuzzer::Run(data, size, [](dawn_native::Instance* instance) {
        instance->DiscoverDefaultAdapters();

        std::vector<dawn_native::Adapter> adapters = instance->GetAdapters();

        wgpu::Device device;
        for (dawn_native::Adapter adapter : adapters) {
            if (adapter.GetBackendType() == dawn_native::BackendType::Vulkan &&
                adapter.GetDeviceType() == dawn_native::DeviceType::CPU) {
                device = wgpu::Device::Acquire(adapter.CreateDevice());
                break;
            }
        }

        ASSERT(device.Get() != nullptr);
        return device;
    });
}
