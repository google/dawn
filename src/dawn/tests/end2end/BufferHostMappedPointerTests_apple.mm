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

#include <mach/mach.h>

#include "dawn/common/Log.h"
#include "dawn/tests/MockCallback.h"
#include "dawn/tests/end2end/BufferHostMappedPointerTests.h"

namespace dawn {
namespace {

class VMBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static VMBackend backend;
        return &backend;
    }

    const char* Name() const override { return "vm_allocate"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        vm_address_t addr = 0;
        EXPECT_EQ(vm_allocate(mach_task_self(), &addr, size, /* anywhere */ true), KERN_SUCCESS);

        void* ptr = reinterpret_cast<void*>(addr);
        Populate(ptr);

        auto DeallocMemory = [=]() { vm_deallocate(mach_task_self(), addr, size); };

        wgpu::BufferHostMappedPointer hostMappedDesc;
        hostMappedDesc.pointer = ptr;
        hostMappedDesc.disposeCallback = mDisposeCallback.Callback();
        hostMappedDesc.userdata = mDisposeCallback.MakeUserdata(ptr);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.usage = usage;
        bufferDesc.size = size;
        bufferDesc.nextInChain = &hostMappedDesc;

        wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
        if (dawn::native::CheckIsErrorForTesting(buffer.Get())) {
            DeallocMemory();
        } else {
            EXPECT_CALL(mDisposeCallback, Call(ptr))
                .WillOnce(testing::InvokeWithoutArgs(DeallocMemory));
        }

        return std::make_pair(std::move(buffer), hostMappedDesc.pointer);
    }

  private:
    testing::MockCallback<WGPUCallback> mDisposeCallback;
};

DAWN_INSTANTIATE_PREFIXED_TEST_P(Apple,
                                 BufferHostMappedPointerTests,
                                 {MetalBackend()},
                                 {VMBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
