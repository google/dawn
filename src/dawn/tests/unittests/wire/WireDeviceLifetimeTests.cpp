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

#include <memory>
#include <utility>

#include "dawn/native/Instance.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/webgpu.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

static WGPUDevice lastBackendDevice = nullptr;

class WireDeviceLifetimeTests : public testing::Test {
  public:
    WireDeviceLifetimeTests()
        : nativeProcs(BuildProcs()),
          wireHelper(utils::CreateWireHelper(nativeProcs, /* useWire */ true)) {
        WGPUInstanceDescriptor instanceDesc = {};
        nativeInstance = std::make_unique<native::Instance>(&instanceDesc);
        instance = wireHelper->RegisterInstance(nativeInstance->Get());
    }

  protected:
    void SetUp() override {
        wgpu::RequestAdapterOptions options = {};
        options.backendType = wgpu::BackendType::Null;

        instance.RequestAdapter(
            &options,
            [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char*, void* userdata) {
                ASSERT_EQ(status, WGPURequestAdapterStatus_Success);
                *static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter::Acquire(cAdapter);
            },
            &adapter);
        ASSERT_TRUE(wireHelper->FlushClient());
        ASSERT_TRUE(wireHelper->FlushServer());
        ASSERT_NE(adapter, nullptr);
    }

    const DawnProcTable nativeProcs;
    std::unique_ptr<utils::WireHelper> wireHelper;

    std::unique_ptr<native::Instance> nativeInstance;
    wgpu::Instance instance;
    wgpu::Adapter adapter;

  private:
    // Set up the proc table which will save the latest WGPUDevice created from requestDevice in
    // static variable `lastBackendDevice`. This lets tests control the wire device and the native
    // backend device separately.
    DawnProcTable BuildProcs() {
        DawnProcTable procs = native::GetProcs();
        procs.adapterRequestDevice = [](WGPUAdapter self, const WGPUDeviceDescriptor* desc,
                                        WGPURequestDeviceCallback callback, void* userdata) {
            using WrappedUserdata = std::pair<WGPURequestDeviceCallback, void*>;
            native::GetProcs().adapterRequestDevice(
                self, desc,
                [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message,
                   void* userdata) {
                    lastBackendDevice = device;
                    auto* wrappedUserdata = static_cast<WrappedUserdata*>(userdata);
                    wrappedUserdata->first(status, device, message, wrappedUserdata->second);
                    delete wrappedUserdata;
                },
                new WrappedUserdata(callback, userdata));
        };

        return procs;
    }
};

// Drop the device from the wire, then call its uncaptured error callback.
// This is a regression test for crbug.com/1430221 where these operations caused a use-after-free.
TEST_F(WireDeviceLifetimeTests, DeviceDroppedFromWireThenUncapturedErrorCallback) {
    wgpu::Device device;
    wgpu::DeviceDescriptor deviceDesc = {};
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = 128;
    bufferDesc.usage = wgpu::BufferUsage::Uniform;

    // Destroy the device.
    device.Destroy();

    // Inject an error.
    device.InjectError(wgpu::ErrorType::Validation, "injected error");

    // Drop the device, but keep the server-side device alive.
    // This prevents the callbacks from being flushed yet.
    WGPUDevice oldDevice = lastBackendDevice;
    nativeProcs.deviceReference(oldDevice);
    device = nullptr;

    // Request a new device. This overrides the wire's device-related data.
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    // Finally, release the old retained device which will flush any of its outstanding
    // callbacks.
    nativeProcs.deviceRelease(oldDevice);
}

// Drop the device from the wire, then call its logging callback.
// This is a regression test for crbug.com/1430221 where these operations caused a use-after-free.
TEST_F(WireDeviceLifetimeTests, DeviceDroppedFromWireThenLoggingCallback) {
    wgpu::Device device;
    wgpu::DeviceDescriptor deviceDesc = {};
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    wgpu::ShaderModuleDescriptor shaderModuleDesc = {};
    wgpu::ShaderModuleWGSLDescriptor wgslDesc = {};
    shaderModuleDesc.nextInChain = &wgslDesc;
    wgslDesc.code = "@compute @workgroup_size(64) fn main() {}";

    // Create a shader module so the transformed shaders are dumped.
    device.CreateShaderModule(&shaderModuleDesc);

    // Destroy the device.
    device.Destroy();

    // Drop the device, but keep the server-side device alive.
    // This prevents the callbacks from being flushed yet.
    WGPUDevice oldDevice = lastBackendDevice;
    nativeProcs.deviceReference(oldDevice);
    device = nullptr;

    // Request a new device. This overrides the wire's device-related data.
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    // Finally, release the old retained device which will flush any of its outstanding
    // callbacks.
    nativeProcs.deviceRelease(oldDevice);
}

// Drop the device from the wire, then call its lost callback.
// This is a regression test for crbug.com/1430221 where these operations caused a use-after-free.
TEST_F(WireDeviceLifetimeTests, DeviceDroppedFromWireThenLostCallback) {
    wgpu::Device device;
    wgpu::DeviceDescriptor deviceDesc = {};
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    // Drop the device, but keep the server-side device alive.
    // This prevents the callbacks from being flushed yet.
    WGPUDevice oldDevice = lastBackendDevice;
    nativeProcs.deviceReference(oldDevice);
    device = nullptr;

    // Destroy the device to enqueue calling the lost callback.
    nativeProcs.deviceDestroy(oldDevice);

    // Request a new device. This overrides the wire's device-related data.
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &device);
    ASSERT_TRUE(wireHelper->FlushClient());
    ASSERT_TRUE(wireHelper->FlushServer());
    ASSERT_NE(device, nullptr);

    // Finally, release the old retained device which will flush any of its outstanding
    // callbacks.
    nativeProcs.deviceRelease(oldDevice);
}

}  // anonymous namespace
}  // namespace dawn
