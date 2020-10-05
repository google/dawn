// Copyright 2020 The Dawn Authors
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

#include "dawn/dawn_thread_dispatch_proc.h"
#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/Instance.h"
#include "dawn_native/null/DeviceNull.h"

#include <gtest/gtest.h>
#include <atomic>
#include <thread>

class PerThreadProcTests : public testing::Test {
  public:
    PerThreadProcTests()
        : mNativeInstance(dawn_native::InstanceBase::Create()),
          mNativeAdapter(mNativeInstance.Get()) {
    }
    ~PerThreadProcTests() override = default;

  protected:
    Ref<dawn_native::InstanceBase> mNativeInstance;
    dawn_native::null::Adapter mNativeAdapter;
};

// Test that procs can be set per thread. This test overrides deviceCreateBuffer with a dummy proc
// for each thread that increments a counter. Because each thread has their own proc and counter,
// there should be no data races. The per-thread procs also check that the current thread id is
// exactly equal to the expected thread id.
TEST_F(PerThreadProcTests, DispatchesPerThread) {
    dawnProcSetProcs(&dawnThreadDispatchProcTable);

    // Threads will block on this atomic to be sure we set procs on both threads before
    // either thread calls the procs.
    std::atomic<bool> ready(false);

    static int threadACounter = 0;
    static int threadBCounter = 0;

    static std::atomic<std::thread::id> threadIdA;
    static std::atomic<std::thread::id> threadIdB;

    constexpr int kThreadATargetCount = 28347;
    constexpr int kThreadBTargetCount = 40420;

    // Note: Acquire doesn't call reference or release.
    wgpu::Device deviceA =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mNativeAdapter.CreateDevice(nullptr)));

    wgpu::Device deviceB =
        wgpu::Device::Acquire(reinterpret_cast<WGPUDevice>(mNativeAdapter.CreateDevice(nullptr)));

    std::thread threadA([&]() {
        DawnProcTable procs = dawn_native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* descriptor) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdA);
            threadACounter++;
            return nullptr;
        };
        dawnProcSetPerThreadProcs(&procs);

        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadATargetCount; ++i) {
            deviceA.CreateBuffer(nullptr);
        }

        deviceA = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    std::thread threadB([&]() {
        DawnProcTable procs = dawn_native::GetProcs();
        procs.deviceCreateBuffer = [](WGPUDevice device,
                                      WGPUBufferDescriptor const* bufferDesc) -> WGPUBuffer {
            EXPECT_EQ(std::this_thread::get_id(), threadIdB);
            threadBCounter++;
            return nullptr;
        };
        dawnProcSetPerThreadProcs(&procs);

        while (!ready) {
        }  // Should be fast, so just spin.

        for (int i = 0; i < kThreadBTargetCount; ++i) {
            deviceB.CreateBuffer(nullptr);
        }

        deviceB = nullptr;
        dawnProcSetPerThreadProcs(nullptr);
    });

    threadIdA = threadA.get_id();
    threadIdB = threadB.get_id();

    ready = true;
    threadA.join();
    threadB.join();

    EXPECT_EQ(threadACounter, kThreadATargetCount);
    EXPECT_EQ(threadBCounter, kThreadBTargetCount);

    dawnProcSetProcs(nullptr);
}
