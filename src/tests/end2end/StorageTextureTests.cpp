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

#include "tests/DawnTest.h"

class StorageTextureTests : public DawnTest {};

// Test that using read-only storage texture and write-only storage texture in BindGroupLayout is
// valid on all backends. This test is a regression test for chromium:1061156 and passes by not
// asserting or crashing.
TEST_P(StorageTextureTests, BindGroupLayoutWithStorageTextureBindingType) {
    // wgpu::BindingType::ReadonlyStorageTexture is a valid binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutBinding binding = {0, wgpu::ShaderStage::Compute,
                                                wgpu::BindingType::ReadonlyStorageTexture};
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.bindingCount = 1;
        descriptor.bindings = &binding;
        device.CreateBindGroupLayout(&descriptor);
    }

    // wgpu::BindingType::WriteonlyStorageTexture is a valid binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutBinding binding = {0, wgpu::ShaderStage::Compute,
                                                wgpu::BindingType::WriteonlyStorageTexture};
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.bindingCount = 1;
        descriptor.bindings = &binding;
        device.CreateBindGroupLayout(&descriptor);
    }
}

DAWN_INSTANTIATE_TEST(StorageTextureTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());
