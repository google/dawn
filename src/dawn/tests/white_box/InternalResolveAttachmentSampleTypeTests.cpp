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

#include <vector>

#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class InternalResolveAttachmentSampleTypeTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());

        // vertex shader module.
        vsModule = utils::CreateShaderModule(device, R"(
            @vertex fn main() -> @builtin(position) vec4f {
                return vec4f(0.0, 0.0, 0.0, 1.0);
            })");
    }

    wgpu::PipelineLayout CreatePipelineLayout(bool withSampler) {
        // Create binding group layout with internal resolve attachment sample type.
        std::vector<native::BindGroupLayoutEntry> bglEntries(2);
        bglEntries[0].binding = 0;
        bglEntries[0].texture.sampleType = native::kInternalResolveAttachmentSampleType;
        bglEntries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
        bglEntries[0].visibility = wgpu::ShaderStage::Fragment;

        native::BindGroupLayoutDescriptor bglDesc;

        if (withSampler) {
            bglEntries[1].binding = 1;
            bglEntries[1].sampler.type = wgpu::SamplerBindingType::Filtering;
            bglEntries[1].visibility = wgpu::ShaderStage::Fragment;

            bglDesc.entryCount = bglEntries.size();
        } else {
            bglDesc.entryCount = 1;
        }
        bglDesc.entries = bglEntries.data();

        native::DeviceBase* nativeDevice = native::FromAPI(device.Get());

        Ref<native::BindGroupLayoutBase> bglRef =
            nativeDevice->CreateBindGroupLayout(&bglDesc, true).AcquireSuccess();

        auto bindGroupLayout = wgpu::BindGroupLayout::Acquire(native::ToAPI(bglRef.Detach()));

        // Create pipeline layout from the bind group layout.
        wgpu::PipelineLayoutDescriptor descriptor;
        std::vector<wgpu::BindGroupLayout> bindgroupLayouts = {bindGroupLayout};
        descriptor.bindGroupLayoutCount = bindgroupLayouts.size();
        descriptor.bindGroupLayouts = bindgroupLayouts.data();
        return device.CreatePipelineLayout(&descriptor);
    }

    wgpu::ShaderModule vsModule;
};

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is compatible with
// a shader using textureLoad(texture_2d<f32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadF32Compatible) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<f32>;

        @fragment fn main() -> @location(0) vec4f {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    device.CreateRenderPipeline(&pipelineDescriptor);
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is compatible
// with a shader using textureSample(texture_2d<f32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureSampleF32Compatible) {
    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<f32>;
        @group(0) @binding(1) var src_sampler : sampler;

        @fragment fn main() -> @location(0) vec4f {
            return textureSample(src_tex, src_sampler, vec2f(0.0, 0.0));
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/true);

    device.CreateRenderPipeline(&pipelineDescriptor);
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is incompatible
// with a shader using textureLoad(texture_2d<i32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadI32Incompatible) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<i32>;

        @fragment fn main() -> @location(0) vec4i {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    ASSERT_DEVICE_ERROR_MSG(device.CreateRenderPipeline(&pipelineDescriptor),
                            testing::HasSubstr("not compatible"));
}

// Test that using a bind group layout with kInternalResolveAttachmentSampleType is incompatible
// with a shader using textureLoad(texture_2d<u32>) function.
TEST_P(InternalResolveAttachmentSampleTypeTests, TextureLoadU32Incompatible) {
    DAWN_TEST_UNSUPPORTED_IF(HasToggleEnabled("skip_validation"));

    utils::ComboRenderPipelineDescriptor pipelineDescriptor;
    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.cFragment.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var src_tex : texture_2d<u32>;

        @fragment fn main() -> @location(0) vec4u {
            return textureLoad(src_tex, vec2u(0, 0), 0);
        })");

    pipelineDescriptor.layout = CreatePipelineLayout(/*withSampler=*/false);

    ASSERT_DEVICE_ERROR_MSG(device.CreateRenderPipeline(&pipelineDescriptor),
                            testing::HasSubstr("not compatible"));
}

DAWN_INSTANTIATE_TEST(InternalResolveAttachmentSampleTypeTests, NullBackend());

}  // anonymous namespace
}  // namespace dawn
