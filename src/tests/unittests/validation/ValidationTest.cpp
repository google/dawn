// Copyright 2017 The NXT Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include "nxt/nxt.h"

namespace backend {
    namespace null {
        void Init(nxtProcTable* procs, nxtDevice* device);
    }
}

ValidationTest::ValidationTest() {
    nxtProcTable procs;
    nxtDevice cDevice;
    backend::null::Init(&procs, &cDevice);

    nxtSetProcs(&procs);
    device = nxt::Device::Acquire(cDevice);

    device.SetErrorCallback(ValidationTest::OnDeviceError, static_cast<nxtCallbackUserdata>(reinterpret_cast<uintptr_t>(this)));
}

ValidationTest::~ValidationTest() {
    // We need to destroy NXT objects before setting the procs to null otherwise the nxt*Release
    // will call a nullptr
    device = nxt::Device();
    nxtSetProcs(nullptr);
}

void ValidationTest::TearDown() {
    ASSERT_FALSE(expectError);

    for (auto& expectation : expectations) {
        std::string name = expectation.debugName;
        if (name.empty()) {
            name = "<no debug name set>";
        }

        ASSERT_TRUE(expectation.gotStatus) << "Didn't get a status for " << name;

        ASSERT_NE(NXT_BUILDER_ERROR_STATUS_UNKNOWN, expectation.status) << "Got unknown status for " << name;

        bool wasSuccess = expectation.status == NXT_BUILDER_ERROR_STATUS_SUCCESS;
        ASSERT_EQ(expectation.expectSuccess, wasSuccess)
            << "Got wrong status value for " << name
            << ", status was " << expectation.status << " with \"" << expectation.statusMessage << "\"";
    }
}

void ValidationTest::StartExpectDeviceError() {
    expectError = true;
    error = false;
}
bool ValidationTest::EndExpectDeviceError() {
    expectError = false;
    return error;
}

void ValidationTest::CreateSimpleRenderPassAndFramebuffer(const nxt::Device& device, nxt::RenderPass* renderpass, nxt::Framebuffer* framebuffer) {
        auto colorBuffer = device.CreateTextureBuilder()
            .SetDimension(nxt::TextureDimension::e2D)
            .SetExtent(640, 480, 1)
            .SetFormat(nxt::TextureFormat::R8G8B8A8Unorm)
            .SetMipLevels(1)
            .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
            .GetResult();
        colorBuffer.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);
        auto colorView = colorBuffer.CreateTextureViewBuilder()
            .GetResult();

        *renderpass = device.CreateRenderPassBuilder()
            .SetAttachmentCount(1)
            .AttachmentSetFormat(0, nxt::TextureFormat::R8G8B8A8Unorm)
            .SetSubpassCount(1)
            .SubpassSetColorAttachment(0, 0, 0)
            .GetResult();

        *framebuffer = device.CreateFramebufferBuilder()
            .SetRenderPass(*renderpass)
            .SetDimensions(640, 480)
            .SetAttachment(0, colorView)
            .GetResult();
}

void ValidationTest::OnDeviceError(const char* message, nxtCallbackUserdata userdata) {
    // Skip this one specific error that is raised when a builder is used after it got an error
    // this is important because we don't want to wrap all creation tests in ASSERT_DEVICE_ERROR.
    // Yes the error message is misleading.
    if (std::string(message) == "Builder cannot be used after GetResult") {
        return;
    }

    auto self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata));
    ASSERT_TRUE(self->expectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->error) << "Got two errors in expect block";
    self->error = true;
}

void ValidationTest::OnBuilderErrorStatus(nxtBuilderErrorStatus status, const char* message, nxt::CallbackUserdata userdata1, nxt::CallbackUserdata userdata2) {
    auto* self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata1));
    size_t index = static_cast<size_t>(userdata2);

    ASSERT_LT(index, self->expectations.size());

    auto& expectation = self->expectations[index];
    ASSERT_FALSE(expectation.gotStatus);
    expectation.gotStatus = true;
    expectation.status = status;
    expectation.statusMessage = message;
}

ValidationTest::DummyRenderPass ValidationTest::CreateDummyRenderPass() {
    DummyRenderPass dummy;
    dummy.width = 400;
    dummy.height = 400;
    dummy.attachmentFormat = nxt::TextureFormat::R8G8B8A8Unorm;

    dummy.renderPass = AssertWillBeSuccess(device.CreateRenderPassBuilder())
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, dummy.attachmentFormat)
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .GetResult();

    dummy.attachment = AssertWillBeSuccess(device.CreateTextureBuilder())
        .SetDimension(nxt::TextureDimension::e2D)
        .SetExtent(dummy.width, dummy.height, 1)
        .SetFormat(dummy.attachmentFormat)
        .SetMipLevels(1)
        .SetAllowedUsage(nxt::TextureUsageBit::OutputAttachment)
        .GetResult();
    dummy.attachment.FreezeUsage(nxt::TextureUsageBit::OutputAttachment);

    nxt::TextureView view = AssertWillBeSuccess(dummy.attachment.CreateTextureViewBuilder()).GetResult();

    dummy.framebuffer = AssertWillBeSuccess(device.CreateFramebufferBuilder())
        .SetRenderPass(dummy.renderPass)
        .SetAttachment(0, view)
        .SetDimensions(dummy.width, dummy.height)
        .GetResult();

    return dummy;
}
