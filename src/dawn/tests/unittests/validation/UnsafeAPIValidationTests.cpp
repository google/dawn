// Copyright 2020 The Dawn & Tint Authors
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

#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::HasSubstr;

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    // UnsafeAPIValidationTest create the device with the AllowUnsafeAPIs toggle explicitly
    // disabled, which overrides the inheritance.
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        // Disable the AllowUnsafeAPIs toggles in device toggles descriptor to override the
        // inheritance and create a device disallowing unsafe apis.
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        const char* toggle = "allow_unsafe_apis";
        deviceTogglesDesc.disabledToggles = &toggle;
        deviceTogglesDesc.disabledToggleCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Check chromium_disable_uniformity_analysis is an unsafe API.
TEST_F(UnsafeAPIValidationTest, chromium_disable_uniformity_analysis) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_disable_uniformity_analysis;

        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3u
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )"));
}

// Check that separate depth-stencil readonlyness is validated as unsafe for render passes.
TEST_F(UnsafeAPIValidationTest, SeparateRenderPassDepthStencilReadOnlyness) {
    wgpu::TextureDescriptor tDesc;
    tDesc.size = {1, 1};
    tDesc.format = wgpu::TextureFormat::Depth24PlusStencil8;
    tDesc.usage = wgpu::TextureUsage::RenderAttachment;
    wgpu::Texture t = device.CreateTexture(&tDesc);

    // Control case: both readonly is valid.
    {
        wgpu::RenderPassDepthStencilAttachment ds;
        ds.view = t.CreateView();
        ds.depthReadOnly = true;
        ds.stencilReadOnly = true;

        wgpu::RenderPassDescriptor rp;
        rp.colorAttachmentCount = 0;
        rp.depthStencilAttachment = &ds;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp);
        pass.End();
        encoder.Finish();
    }

    // Error case: only one readonly is invalid.
    {
        wgpu::RenderPassDepthStencilAttachment ds;
        ds.view = t.CreateView();
        ds.depthReadOnly = true;
        ds.stencilReadOnly = false;
        ds.stencilLoadOp = wgpu::LoadOp::Load;
        ds.stencilStoreOp = wgpu::StoreOp::Store;

        wgpu::RenderPassDescriptor rp;
        rp.colorAttachmentCount = 0;
        rp.depthStencilAttachment = &ds;

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&rp);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Check that separate depth-stencil readonlyness is validated as unsafe for render bundles.
TEST_F(UnsafeAPIValidationTest, SeparateRenderBundleDepthStencilReadOnlyness) {
    utils::ComboRenderBundleEncoderDescriptor desc = {};
    desc.depthStencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;

    // Control case: both readonly is valid.
    {
        desc.depthReadOnly = true;
        desc.stencilReadOnly = true;
        device.CreateRenderBundleEncoder(&desc);
    }

    // Error case: only one readonly is invalid.
    {
        desc.depthReadOnly = true;
        desc.stencilReadOnly = false;
        ASSERT_DEVICE_ERROR(device.CreateRenderBundleEncoder(&desc));
    }
}

// Check that create 3D texture for the render attachment is validated as unsafe.
TEST_F(UnsafeAPIValidationTest, Create3DTextureForRenderAttachment) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e3D;
    descriptor.size = {1, 1};
    descriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
}

class TimestampQueryUnsafeAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        const char* toggle = "allow_unsafe_apis";
        deviceTogglesDesc.disabledToggles = &toggle;
        deviceTogglesDesc.disabledToggleCount = 1;
        wgpu::FeatureName requiredFeatures[1] = {wgpu::FeatureName::TimestampQuery};
        descriptor.requiredFeatures = requiredFeatures;
        descriptor.requiredFeatureCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Check write timestamp on command encoder is an unsafe API.
TEST_F(TimestampQueryUnsafeAPIValidationTest, WriteTimestampOnCommandEncoder) {
    wgpu::QuerySetDescriptor descriptor;
    descriptor.type = wgpu::QueryType::Timestamp;
    descriptor.count = 2;

    wgpu::QuerySet timestampQuerySet = device.CreateQuerySet(&descriptor);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(timestampQuerySet, 0);
    ASSERT_DEVICE_ERROR(encoder.Finish());
}

}  // anonymous namespace
}  // namespace dawn
