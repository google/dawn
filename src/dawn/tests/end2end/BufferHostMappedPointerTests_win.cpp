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

#include <windows.h>
#include <utility>

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

    const char* Name() const override { return "VirtualAlloc"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        EXPECT_NE(ptr, nullptr);

        Populate(ptr);

        auto DeallocMemory = [=]() { VirtualFree(ptr, 0, MEM_RELEASE); };

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

class MMapBackend : public BufferHostMappedPointerTestBackend {
  public:
    static BufferHostMappedPointerTestBackend* GetInstance() {
        static MMapBackend backend;
        return &backend;
    }

    const char* Name() const override { return "FileMapping"; }

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) override {
        // Get the temp path string
        TCHAR tmpFilePath[MAX_PATH];
        DWORD dwRetVal = GetTempPath(MAX_PATH,      // length of the buffer
                                     tmpFilePath);  // buffer for path
        EXPECT_GT(dwRetVal, 0u);
        EXPECT_LE(dwRetVal, static_cast<DWORD>(MAX_PATH));

        TCHAR tmpFileName[MAX_PATH];
        EXPECT_GT(GetTempFileName(tmpFilePath,   // directory for tmp files
                                  TEXT("TMP"),   // temp file name prefix
                                  0,             // create unique name
                                  tmpFileName),  // buffer for name
                  0u);

        // Creates the new file
        HANDLE tmpFileHandle = CreateFile(tmpFileName,                   // file name
                                          GENERIC_READ | GENERIC_WRITE,  // open for read write
                                          0,                             // do not share
                                          NULL,                          // default security
                                          CREATE_ALWAYS,                 // overwrite existing
                                          FILE_ATTRIBUTE_NORMAL,         // normal file
                                          NULL);                         // no template
        EXPECT_NE(tmpFileHandle, INVALID_HANDLE_VALUE);

        LARGE_INTEGER largeSize;
        largeSize.QuadPart = size;
        HANDLE fileMappingHandle = CreateFileMapping(
            tmpFileHandle, nullptr, PAGE_READWRITE, largeSize.HighPart, largeSize.LowPart, nullptr);

        EXPECT_NE(fileMappingHandle, INVALID_HANDLE_VALUE);

        void* ptr = MapViewOfFile(fileMappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        EXPECT_NE(ptr, nullptr);
        Populate(ptr);

        auto DeallocMemory = [=]() {
            // Cleanup mapping, handles, and file.
            EXPECT_TRUE(UnmapViewOfFile(ptr));
            CloseHandle(fileMappingHandle);
            CloseHandle(tmpFileHandle);
            EXPECT_TRUE(DeleteFile(tmpFileName));
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

DAWN_INSTANTIATE_PREFIXED_TEST_P(Win,
                                 BufferHostMappedPointerTests,
                                 {D3D12Backend()},
                                 {VMBackend::GetInstance(), MMapBackend::GetInstance()});

}  // anonymous namespace
}  // namespace dawn
