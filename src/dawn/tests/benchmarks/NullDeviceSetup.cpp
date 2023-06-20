// Copyright 2023 The Dawn Authors
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

#include "dawn/tests/benchmarks/NullDeviceSetup.h"

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/common/Log.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/DawnNative.h"

namespace {
std::unique_ptr<dawn::native::Instance> nativeInstance = nullptr;
wgpu::Adapter nullBackendAdapter = nullptr;
}  // namespace

void SetupNullBackend(const benchmark::State& state) {
    dawnProcSetProcs(&dawn::native::GetProcs());

    if (!nativeInstance) {
        nativeInstance = std::make_unique<dawn::native::Instance>();
    }

    if (!nullBackendAdapter) {
        wgpu::RequestAdapterOptionsBackendType backendTypeOptions = {};
        backendTypeOptions.backendType = wgpu::BackendType::Null;

        wgpu::RequestAdapterOptions options = {};
        options.nextInChain = &backendTypeOptions;

        auto nativeAdapter = nativeInstance->EnumerateAdapters(&options)[0];
        nullBackendAdapter = wgpu::Adapter(nativeAdapter.Get());
    }
    ASSERT(nullBackendAdapter != nullptr);
}

wgpu::Device CreateNullDevice(const wgpu::DeviceDescriptor& desc) {
    wgpu::Device device;

    nullBackendAdapter.RequestDevice(
        &desc,
        [](WGPURequestDeviceStatus status, WGPUDevice cDevice, char const* message,
           void* userdata) {
            ASSERT(status == WGPURequestDeviceStatus_Success);
            *reinterpret_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    while (!device) {
        wgpuInstanceProcessEvents(nativeInstance->Get());
    }

    device.SetUncapturedErrorCallback(
        [](WGPUErrorType, char const* message, void* userdata) {
            dawn::ErrorLog() << message;
            UNREACHABLE();
        },
        nullptr);

    device.SetDeviceLostCallback(
        [](WGPUDeviceLostReason reason, char const* message, void* userdata) {
            if (reason == WGPUDeviceLostReason_Undefined) {
                dawn::ErrorLog() << message;
                UNREACHABLE();
            }
        },
        nullptr);

    return device;
}
