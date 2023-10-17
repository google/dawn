// Copyright 2022 The Dawn & Tint Authors
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

#include <utility>

#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class DeviceLifetimeTests : public DawnTest {};

// Test that the device can be dropped before its queue.
TEST_P(DeviceLifetimeTests, DroppedBeforeQueue) {
    wgpu::Queue queue = device.GetQueue();

    device = nullptr;
}

// Test that the device can be dropped while an onSubmittedWorkDone callback is in flight.
TEST_P(DeviceLifetimeTests, DroppedWhileQueueOnSubmittedWorkDone) {
    // Submit some work.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(nullptr);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    // Ask for an onSubmittedWorkDone callback and drop the device.
    queue.OnSubmittedWorkDone(
        [](WGPUQueueWorkDoneStatus status, void*) {
            // There is a bug in DeviceBase::Destroy(). If all submitted work is done when
            // OnSubmittedWorkDone() is being called, the callback will be resolved with
            // DeviceLost, otherwise the callback will be resolved with Success.
            // TODO(dawn:1640): fix DeviceBase::Destroy() to always reslove the callback
            // with success.
            EXPECT_TRUE(status == WGPUQueueWorkDoneStatus_Success ||
                        status == WGPUQueueWorkDoneStatus_DeviceLost);
        },
        nullptr);

    device = nullptr;
}

// Test that the device can be dropped inside an onSubmittedWorkDone callback.
TEST_P(DeviceLifetimeTests, DroppedInsideQueueOnSubmittedWorkDone) {
    // Submit some work.
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder(nullptr);
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    // Ask for an onSubmittedWorkDone callback and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    queue.OnSubmittedWorkDone(
        [](WGPUQueueWorkDoneStatus status, void* userdata) {
            EXPECT_EQ(status, WGPUQueueWorkDoneStatus_Success);
            static_cast<Userdata*>(userdata)->device = nullptr;
            static_cast<Userdata*>(userdata)->done = true;
        },
        &data);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }
}

// Test that the device can be dropped while a popErrorScope callback is in flight.
TEST_P(DeviceLifetimeTests, DroppedWhilePopErrorScope) {
    device.PushErrorScope(wgpu::ErrorFilter::Validation);
    bool wire = UsesWire();
    device.PopErrorScope(
        [](WGPUErrorType type, const char*, void* userdata) {
            const bool wire = *static_cast<bool*>(userdata);
            // On the wire, all callbacks get rejected immediately with once the device is deleted.
            // In native, popErrorScope is called synchronously.
            // TODO(crbug.com/dawn/1122): These callbacks should be made consistent.
            EXPECT_EQ(type, wire ? WGPUErrorType_Unknown : WGPUErrorType_NoError);
        },
        &wire);
    device = nullptr;
}

// Test that the device can be dropped inside an onSubmittedWorkDone callback.
TEST_P(DeviceLifetimeTests, DroppedInsidePopErrorScope) {
    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    // Ask for a popErrorScope callback and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    data.device.PopErrorScope(
        [](WGPUErrorType type, const char*, void* userdata) {
            EXPECT_EQ(type, WGPUErrorType_NoError);
            static_cast<Userdata*>(userdata)->device = nullptr;
            static_cast<Userdata*>(userdata)->done = true;
        },
        &data);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }
}

// Test that the device can be dropped before a buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;
}

// Test that the device can be dropped while a buffer created from it is being mapped.
TEST_P(DeviceLifetimeTests, DroppedWhileMappingBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void*) {
            EXPECT_EQ(status, WGPUBufferMapAsyncStatus_DestroyedBeforeCallback);
        },
        nullptr);

    device = nullptr;
}

// Test that the device can be dropped before a mapped buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeMappedBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    bool done = false;
    buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            EXPECT_EQ(status, WGPUBufferMapAsyncStatus_Success);
            *static_cast<bool*>(userdata) = true;
        },
        &done);

    while (!done) {
        WaitABit();
    }

    device = nullptr;
}

// Test that the device can be dropped before a mapped at creation buffer created from it.
TEST_P(DeviceLifetimeTests, DroppedBeforeMappedAtCreationBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    desc.mappedAtCreation = true;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;
}

// Test that the device can be dropped before a buffer created from it, then mapping the buffer
// fails.
TEST_P(DeviceLifetimeTests, DroppedThenMapBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;

    bool done = false;
    buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            EXPECT_EQ(status, WGPUBufferMapAsyncStatus_DeviceLost);
            *static_cast<bool*>(userdata) = true;
        },
        &done);

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped before a buffer created from it, then mapping the buffer
// twice (one inside callback) will both fail.
TEST_P(DeviceLifetimeTests, Dropped_ThenMapBuffer_ThenMapBufferInCallback) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    device = nullptr;

    struct UserData {
        wgpu::Buffer buffer;
        bool done = false;
    };

    UserData userData;
    userData.buffer = buffer;

    // First mapping.
    buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void* userdataPtr) {
            EXPECT_EQ(status, WGPUBufferMapAsyncStatus_DeviceLost);
            auto userdata = static_cast<UserData*>(userdataPtr);

            // Second mapping.
            userdata->buffer.MapAsync(
                wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
                [](WGPUBufferMapAsyncStatus status, void* userdataPtr) {
                    EXPECT_EQ(status, WGPUBufferMapAsyncStatus_DeviceLost);
                    *static_cast<bool*>(userdataPtr) = true;
                },
                &userdata->done);
        },
        &userData);

    while (!userData.done) {
        WaitABit();
    }
}

// Test that the device can be dropped inside a buffer map callback.
TEST_P(DeviceLifetimeTests, DroppedInsideBufferMapCallback) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    struct Userdata {
        wgpu::Device device;
        wgpu::Buffer buffer;
        bool wire;
        bool done;
    };

    // Ask for a mapAsync callback and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), buffer, UsesWire(), false};
    buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            EXPECT_EQ(status, WGPUBufferMapAsyncStatus_Success);
            auto* data = static_cast<Userdata*>(userdata);
            data->device = nullptr;
            data->done = true;

            // Mapped data should be null since the buffer is implicitly destroyed.
            // TODO(crbug.com/dawn/1424): On the wire client, we don't track device child objects so
            // the mapped data is still available when the device is destroyed.
            if (!data->wire) {
                EXPECT_EQ(data->buffer.GetConstMappedRange(), nullptr);
            }
        },
        &data);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }

    // Mapped data should be null since the buffer is implicitly destroyed.
    // TODO(crbug.com/dawn/1424): On the wire client, we don't track device child objects so the
    // mapped data is still available when the device is destroyed.
    if (!UsesWire()) {
        EXPECT_EQ(buffer.GetConstMappedRange(), nullptr);
    }
}

// Test that the device can be dropped while a write buffer operation is enqueued.
TEST_P(DeviceLifetimeTests, DroppedWhileWriteBuffer) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    uint32_t value = 7;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));
    device = nullptr;
}

// Test that the device can be dropped while a write buffer operation is enqueued and then
// a queue submit occurs. This is slightly different from the former test since it ensures
// that pending work is flushed.
TEST_P(DeviceLifetimeTests, DroppedWhileWriteBufferAndSubmit) {
    wgpu::BufferDescriptor desc = {};
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&desc);

    uint32_t value = 7;
    queue.WriteBuffer(buffer, 0, &value, sizeof(value));
    queue.Submit(0, nullptr);
    device = nullptr;
}

// Test that the device can be dropped while createPipelineAsync is in flight
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char* message,
           void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(cPipeline, nullptr);
            wgpu::ComputePipeline::Acquire(cPipeline);
        },
        nullptr);

    device = nullptr;
}

// Test that the device can be dropped inside a createPipelineAsync callback
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsync) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    // Call CreateComputePipelineAsync and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    data.device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char* message,
           void* userdata) {
            wgpu::ComputePipeline::Acquire(cPipeline);
            EXPECT_EQ(status, WGPUCreatePipelineAsyncStatus_Success);

            static_cast<Userdata*>(userdata)->device = nullptr;
            static_cast<Userdata*>(userdata)->done = true;
        },
        &data);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }
}

// Test that the device can be dropped while createPipelineAsync which will hit the frontend cache
// is in flight
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsyncAlreadyCached) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    // Create a pipeline ahead of time so it's in the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    bool done = false;
    device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char*,
           void* userdata) {
            wgpu::ComputePipeline::Acquire(cPipeline);
            EXPECT_EQ(status, WGPUCreatePipelineAsyncStatus_Success);
            EXPECT_NE(cPipeline, nullptr);

            *static_cast<bool*>(userdata) = true;
        },
        &done);
    device = nullptr;

    while (!done) {
        WaitABit();
    }
}

// Test that the device can be dropped inside a createPipelineAsync callback which will hit the
// frontend cache
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsyncAlreadyCached) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    // Create a pipeline ahead of time so it's in the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    // Call CreateComputePipelineAsync and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    data.device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char* message,
           void* userdata) {
            wgpu::ComputePipeline::Acquire(cPipeline);
            // Success because it hits the frontend cache immediately.
            EXPECT_EQ(status, WGPUCreatePipelineAsyncStatus_Success);
            EXPECT_NE(cPipeline, nullptr);

            static_cast<Userdata*>(userdata)->device = nullptr;
            static_cast<Userdata*>(userdata)->done = true;
        },
        &data);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }
}

// Test that the device can be dropped while createPipelineAsync which will race with a compilation
// to add the same pipeline to the frontend cache
TEST_P(DeviceLifetimeTests, DroppedWhileCreatePipelineAsyncRaceCache) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char* message,
           void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(cPipeline, nullptr);
            wgpu::ComputePipeline::Acquire(cPipeline);
        },
        nullptr);

    // Create the same pipeline synchronously which will get added to the cache.
    wgpu::ComputePipeline p = device.CreateComputePipeline(&desc);

    device = nullptr;
}

// Test that the device can be dropped inside a createPipelineAsync callback which will race
// with a compilation to add the same pipeline to the frontend cache
TEST_P(DeviceLifetimeTests, DroppedInsideCreatePipelineAsyncRaceCache) {
    wgpu::ComputePipelineDescriptor desc;
    desc.compute.module = utils::CreateShaderModule(device, R"(
    @compute @workgroup_size(1) fn main() {
    })");
    desc.compute.entryPoint = "main";

    struct Userdata {
        wgpu::Device device;
        bool done;
    };
    // Call CreateComputePipelineAsync and drop the device inside the callback.
    Userdata data = Userdata{std::move(device), false};
    data.device.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline cPipeline, const char* message,
           void* userdata) {
            EXPECT_EQ(WGPUCreatePipelineAsyncStatus_Success, status);
            EXPECT_NE(cPipeline, nullptr);
            wgpu::ComputePipeline::Acquire(cPipeline);

            static_cast<Userdata*>(userdata)->device = nullptr;
            static_cast<Userdata*>(userdata)->done = true;
        },
        &data);

    // Create the same pipeline synchronously which will get added to the cache.
    wgpu::ComputePipeline p = data.device.CreateComputePipeline(&desc);

    while (!data.done) {
        // WaitABit no longer can call tick since we've moved the device from the fixture into the
        // userdata.
        if (data.device) {
            data.device.Tick();
        }
        WaitABit();
    }
}

// Tests that dropping 2nd device inside 1st device's callback triggered by instance.ProcessEvents
// won't crash.
TEST_P(DeviceLifetimeTests, DropDevice2InProcessEvents) {
    wgpu::Device device2 = CreateDevice();

    struct UserData {
        wgpu::Device device2;
        bool done = false;
    } userdata;

    userdata.device2 = std::move(device2);

    device.PushErrorScope(wgpu::ErrorFilter::Validation);

    // The following callback will drop the 2nd device. It won't be triggered until
    // instance.ProcessEvents() is called.
    device.PopErrorScope(
        [](WGPUErrorType type, const char*, void* userdataPtr) {
            auto userdata = static_cast<UserData*>(userdataPtr);

            userdata->device2 = nullptr;
            userdata->done = true;
        },
        &userdata);

    while (!userdata.done) {
        WaitABit();
    }
}

DAWN_INSTANTIATE_TEST(DeviceLifetimeTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // anonymous namespace
}  // namespace dawn
