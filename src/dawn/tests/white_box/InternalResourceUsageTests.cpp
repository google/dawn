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

#include "dawn/tests/DawnTest.h"

#include "dawn/native/dawn_platform.h"

namespace dawn {
namespace {

class InternalResourceUsageTests : public DawnTest {
  protected:
    wgpu::Buffer CreateBuffer(wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = usage;

        return device.CreateBuffer(&descriptor);
    }
};

// Verify it is an error to create a buffer with a buffer usage that should only be used
// internally.
TEST_P(InternalResourceUsageTests, InternalBufferUsage) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    ASSERT_DEVICE_ERROR(CreateBuffer(native::kReadOnlyStorageBuffer));

    ASSERT_DEVICE_ERROR(CreateBuffer(native::kInternalStorageBuffer));
}

DAWN_INSTANTIATE_TEST(InternalResourceUsageTests, NullBackend());

class InternalBindingTypeTests : public DawnTest {};

// Verify it is an error to create a bind group layout with a buffer binding type that should only
// be used internally.
TEST_P(InternalBindingTypeTests, InternalStorageBufferBindingType) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BindGroupLayoutEntry bglEntry;
    bglEntry.binding = 0;
    bglEntry.buffer.type = native::kInternalStorageBufferBinding;
    bglEntry.visibility = wgpu::ShaderStage::Compute;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;
    ASSERT_DEVICE_ERROR(device.CreateBindGroupLayout(&bglDesc));
}

// Verify it is an error to create a bind group layout with a texture sample type that should only
// be used internally.
TEST_P(InternalBindingTypeTests, ErrorUseInternalResolveAttachmentSampleTypeExternally) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    wgpu::BindGroupLayoutEntry bglEntry;
    bglEntry.binding = 0;
    bglEntry.texture.sampleType = native::kInternalResolveAttachmentSampleType;
    bglEntry.visibility = wgpu::ShaderStage::Fragment;

    wgpu::BindGroupLayoutDescriptor bglDesc;
    bglDesc.entryCount = 1;
    bglDesc.entries = &bglEntry;
    ASSERT_DEVICE_ERROR_MSG(device.CreateBindGroupLayout(&bglDesc),
                            testing::HasSubstr("invalid for WGPUTextureSampleType"));
}

DAWN_INSTANTIATE_TEST(InternalBindingTypeTests, NullBackend());

}  // anonymous namespace
}  // namespace dawn
