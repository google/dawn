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

namespace dawn {

void NullDeviceBenchmarkFixture::SetUp(const benchmark::State& state) {
    // Static initialization that only happens on the first time that a fixture is created.
    static std::unique_ptr<dawn::native::Instance> nativeInstance = []() {
        dawnProcSetProcs(&dawn::native::GetProcs());
        return std::make_unique<dawn::native::Instance>();
    }();

    if (state.thread_index() == 0) {
        // Only thread 0 is responsible for initializing the device on each iteration.
        {
            std::lock_guard lock(mMutex);

            // Get an adapter to create the device with.
            wgpu::RequestAdapterOptions options = {};
            options.backendType = wgpu::BackendType::Null;
            auto nativeAdapter = nativeInstance->EnumerateAdapters(&options)[0];
            adapter = wgpu::Adapter(nativeAdapter.Get());
            ASSERT(adapter != nullptr);

            // Create the device.
            wgpu::DeviceDescriptor desc = GetDeviceDescriptor();
            adapter.RequestDevice(
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
        }
        mNumDoneThreads = 0;
        mCv.notify_all();
    } else {
        // All other threads should wait to proceed once the device is ready.
        std::unique_lock lock(mMutex);
        mCv.wait(lock, [this] { return device != nullptr; });
    }
}

void NullDeviceBenchmarkFixture::TearDown(const benchmark::State& state) {
    if (state.thread_index() == 0) {
        std::unique_lock lock(mMutex);
        mCv.wait(lock, [this, state] { return mNumDoneThreads == state.threads() - 1; });
        device = nullptr;
    } else {
        bool isDone = false;
        {
            std::lock_guard lock(mMutex);
            mNumDoneThreads += 1;
            isDone = mNumDoneThreads == state.threads() - 1;
        }
        if (isDone) {
            mCv.notify_one();
        }
    }
}

}  // namespace dawn
