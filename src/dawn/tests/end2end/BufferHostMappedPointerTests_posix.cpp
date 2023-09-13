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

#include <sys/mman.h>
#include <unistd.h>
#include <utility>
#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/end2end/BufferHostMappedPointerTests.h"

namespace dawn {
namespace {

class MMapBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static MMapBackend backend;
        return &backend;
    }

    const char* Name() const override { return "mmap"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        // Create a temporary file.
        char filename[] = "tmpXXXXXX";
        int fd = mkstemp(filename);
        EXPECT_GT(fd, -1);

        unlink(filename);

        // Write the initial data.
        std::vector<char> initialData(size);
        Populate(initialData.data());
        EXPECT_EQ(write(fd, initialData.data(), size), (signed)size);

        // Memory map the file.
        void* ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

        auto UnmapMemory = [=]() {
            munmap(ptr, size);
            close(fd);
        };

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
            UnmapMemory();
        } else {
            EXPECT_CALL(mDisposeCallback, Call(ptr))
                .WillOnce(testing::InvokeWithoutArgs(UnmapMemory));
        }

        return std::make_pair(std::move(buffer), hostMappedDesc.pointer);
    }

  private:
    testing::MockCallback<WGPUCallback> mDisposeCallback;
};

DAWN_INSTANTIATE_PREFIXED_TEST_P(Posix,
                                 BufferHostMappedPointerTests,
                                 {MetalBackend(), VulkanBackend()},
                                 {MMapBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
