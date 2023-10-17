// Copyright 2017 The Dawn & Tint Authors
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

#include <gmock/gmock.h>

#include "dawn/native/CommandEncoder.h"

#include "dawn/tests/unittests/validation/ValidationTest.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using ::testing::HasSubstr;

class CommandBufferValidationTest : public ValidationTest {};

// Test for an empty command buffer
TEST_F(CommandBufferValidationTest, Empty) {
    device.CreateCommandEncoder().Finish();
}

// Test that a command buffer cannot be ended mid render pass
TEST_F(CommandBufferValidationTest, EndedMidRenderPass) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        encoder.Finish();
    }

    // Error case, command buffer ended mid-pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [RenderPassEncoder] was ended."));
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after Finish
    // should fail too.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [RenderPassEncoder] was ended."));
        ASSERT_DEVICE_ERROR(
            pass.End(), HasSubstr("Parent encoder of [RenderPassEncoder] is already finished."));
    }
}

// Test that a command buffer cannot be ended mid compute pass
TEST_F(CommandBufferValidationTest, EndedMidComputePass) {
    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.End();
        encoder.Finish();
    }

    // Error case, command buffer ended mid-pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [ComputePassEncoder] was ended."));
    }

    // Error case, command buffer ended mid-pass. Trying to use encoders after Finish
    // should fail too.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [ComputePassEncoder] was ended."));
        ASSERT_DEVICE_ERROR(
            pass.End(), HasSubstr("Parent encoder of [ComputePassEncoder] is already finished."));
    }
}

// Test that a render pass cannot be ended twice
TEST_F(CommandBufferValidationTest, RenderPassEndedTwice) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Control case, pass is ended once
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        encoder.Finish();
    }

    // Error case, pass ended twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(pass.End(), HasSubstr("[RenderPassEncoder] was already ended."));
        encoder.Finish();
    }
}

// Test that a compute pass cannot be ended twice
TEST_F(CommandBufferValidationTest, ComputePassEndedTwice) {
    // Control case, pass is ended once.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.End();
        encoder.Finish();
    }

    // Error case, pass ended twice
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.End();
        ASSERT_DEVICE_ERROR(pass.End(), HasSubstr("[ComputePassEncoder] was already ended."));
        encoder.Finish();
    }
}

// Test that a pass cannot be ended again in another pass
TEST_F(CommandBufferValidationTest, PassEndedAgainMidAnotherPass) {
    // Error case, render pass ended again in another render pass
    {
        PlaceholderRenderPass placeholderRenderPass(device);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass0 = encoder.BeginRenderPass(&placeholderRenderPass);
        pass0.End();
        wgpu::RenderPassEncoder pass1 = encoder.BeginRenderPass(&placeholderRenderPass);
        ASSERT_DEVICE_ERROR(pass0.End(), HasSubstr("[RenderPassEncoder] was already ended."));
        pass1.End();
        encoder.Finish();
    }

    // Error case, compute pass ended again in another compute pass
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass0 = encoder.BeginComputePass();
        pass0.End();
        wgpu::ComputePassEncoder pass1 = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(pass0.End(), HasSubstr("[ComputePassEncoder] was already ended."));
        pass1.End();
        encoder.Finish();
    }
}

// Test that beginning a compute pass before ending the previous pass causes an error.
TEST_F(CommandBufferValidationTest, BeginComputePassBeforeEndPreviousPass) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Beginning a compute pass before ending a render pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&placeholderRenderPass);
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        computePass.End();
        renderPass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Beginning a compute pass before ending a compute pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass1 = encoder.BeginComputePass();
        wgpu::ComputePassEncoder computePass2 = encoder.BeginComputePass();
        computePass2.End();
        computePass1.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that beginning a render pass before ending the previous pass causes an error.
TEST_F(CommandBufferValidationTest, BeginRenderPassBeforeEndPreviousPass) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Beginning a render pass before ending the render pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPass1 = encoder.BeginRenderPass(&placeholderRenderPass);
        wgpu::RenderPassEncoder renderPass2 = encoder.BeginRenderPass(&placeholderRenderPass);
        renderPass2.End();
        renderPass1.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }

    // Beginning a compute pass before ending a compute pass causes an error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePass = encoder.BeginComputePass();
        wgpu::RenderPassEncoder renderPass = encoder.BeginRenderPass(&placeholderRenderPass);
        renderPass.End();
        computePass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish());
    }
}

// Test that encoding command after a successful finish produces an error
TEST_F(CommandBufferValidationTest, CallsAfterASuccessfulFinish) {
    // A buffer that can be used in CopyBufferToBuffer
    wgpu::BufferDescriptor copyBufferDesc;
    copyBufferDesc.size = 16;
    copyBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer copyBuffer = device.CreateBuffer(&copyBufferDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.Finish();

    ASSERT_DEVICE_ERROR(encoder.CopyBufferToBuffer(copyBuffer, 0, copyBuffer, 0, 0),
                        HasSubstr("[CommandEncoder] is already finished."));
}

// Test that encoding command after a failed finish produces an error
TEST_F(CommandBufferValidationTest, CallsAfterAFailedFinish) {
    // A buffer that can be used in CopyBufferToBuffer
    wgpu::BufferDescriptor copyBufferDesc;
    copyBufferDesc.size = 16;
    copyBufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer copyBuffer = device.CreateBuffer(&copyBufferDesc);

    // A buffer that can't be used in CopyBufferToBuffer
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = 16;
    bufferDesc.usage = wgpu::BufferUsage::Uniform;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(buffer, 0, buffer, 0, 0);
    ASSERT_DEVICE_ERROR(encoder.Finish());

    ASSERT_DEVICE_ERROR(encoder.CopyBufferToBuffer(copyBuffer, 0, copyBuffer, 0, 0),
                        HasSubstr("[CommandEncoder] is already finished."));
}

// Test that passes which are de-referenced prior to ending still allow the correct errors to be
// produced.
TEST_F(CommandBufferValidationTest, PassDereferenced) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        encoder.Finish();
    }

    // Error case, no reference is kept to a render pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginRenderPass(&placeholderRenderPass);
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [RenderPassEncoder] was ended."));
    }

    // Error case, no reference is kept to a compute pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [ComputePassEncoder] was ended."));
    }

    // Error case, beginning a new pass after failing to end a de-referenced pass.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginRenderPass(&placeholderRenderPass);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.End();
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [RenderPassEncoder] was ended."));
    }

    // Error case, deleting the pass after finishing the command encoder shouldn't generate an
    // uncaptured error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        ASSERT_DEVICE_ERROR(
            encoder.Finish(),
            HasSubstr("Command buffer recording ended before [ComputePassEncoder] was ended."));

        pass = nullptr;
    }

    // Valid case, command encoder is never finished so the de-referenced pass shouldn't
    // generate an uncaptured error.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.BeginComputePass();
    }
}

// Test that calling inject validation error produces an error.
TEST_F(CommandBufferValidationTest, InjectValidationError) {
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.InjectValidationError("my error");
    ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("my error"));
}

TEST_F(CommandBufferValidationTest, DestroyEncoder) {
    // Skip these tests if we are using wire because the destroy functionality is not exposed
    // and needs to use a cast to call manually. We cannot test this in the wire case since the
    // only way to trigger the destroy call is by losing all references which means we cannot
    // call finish.
    DAWN_SKIP_TEST_IF(UsesWire());
    PlaceholderRenderPass placeholderRenderPass(device);

    // Control case, command buffer ended after the pass is ended.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        encoder.Finish();
    }

    // Destroyed encoder with encoded commands should emit error on finish.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        native::FromAPI(encoder.Get())->Destroy();
        ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("Destroyed encoder cannot be finished."));
    }

    // Destroyed encoder with encoded commands shouldn't emit an error if never finished.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        native::FromAPI(encoder.Get())->Destroy();
    }

    // Destroyed encoder should allow encoding, and emit error on finish.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        native::FromAPI(encoder.Get())->Destroy();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("Destroyed encoder cannot be finished."));
    }

    // Destroyed encoder should allow encoding and shouldn't emit an error if never finished.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        native::FromAPI(encoder.Get())->Destroy();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
    }

    // Destroying a finished encoder should not emit any errors.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        encoder.Finish();
        native::FromAPI(encoder.Get())->Destroy();
    }

    // Destroying an encoder twice should not emit any errors.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        native::FromAPI(encoder.Get())->Destroy();
        native::FromAPI(encoder.Get())->Destroy();
    }

    // Destroying an encoder twice and then calling finish should fail.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        native::FromAPI(encoder.Get())->Destroy();
        native::FromAPI(encoder.Get())->Destroy();
        ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("Destroyed encoder cannot be finished."));
    }
}

TEST_F(CommandBufferValidationTest, EncodeAfterDeviceDestroyed) {
    PlaceholderRenderPass placeholderRenderPass(device);

    // Device destroyed before encoding.
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        ExpectDeviceDestruction();
        device.Destroy();
        // The encoder should not accessing any device info if device is destroyed when try
        // encoding.
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("Destroyed encoder cannot be finished."));
    }

    // Device destroyed after encoding.
    {
        ExpectDeviceDestruction();
        device.Destroy();
        ASSERT_DEVICE_ERROR(wgpu::CommandEncoder encoder = device.CreateCommandEncoder(),
                            HasSubstr("[Device] is lost"));
        // The encoder should not accessing any device info if device is destroyed when try
        // encoding.
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&placeholderRenderPass);
        pass.End();
        ASSERT_DEVICE_ERROR(encoder.Finish(), HasSubstr("[Invalid CommandEncoder] is invalid."));
    }
}

}  // anonymous namespace
}  // namespace dawn
