// Copyright 2021 The Dawn Authors
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

#include "tests/DawnTest.h"

class MemoryAllocationStressTests : public DawnTest {};

// Test memory allocation is freed correctly when creating and destroying large buffers.
// It will consume a total of 100G of memory, 1G each time. Expect not to trigger out of memory on
// devices with gpu memory less than 100G.
TEST_P(MemoryAllocationStressTests, LargeBuffer) {
    // TODO(crbug.com/dawn/957): Memory leak on D3D12, the memory of destroyed buffer cannot be
    // released.
    DAWN_TEST_UNSUPPORTED_IF(IsD3D12());

    // TODO(crbug.com/dawn/957): Check whether it can be reproduced on each backend.
    DAWN_TEST_UNSUPPORTED_IF(IsMetal() || IsOpenGL() || IsOpenGLES() || IsVulkan());

    uint32_t count = 100;
    for (uint32_t i = 0; i < count; i++) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 1024 * 1024 * 1024;  // 1G
        descriptor.usage = wgpu::BufferUsage::Storage;
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.Destroy();
    }
}

DAWN_INSTANTIATE_TEST(MemoryAllocationStressTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());
