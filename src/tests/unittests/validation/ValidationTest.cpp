// Copyright 2017 The Dawn Authors
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

#include "dawn/dawn.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/NullBackend.h"

namespace dawn_native {
    namespace null {
        void Init(dawnProcTable* procs, dawnDevice* device);
    }
}

ValidationTest::ValidationTest() {
    dawnProcTable procs = dawn_native::GetProcs();
    dawnDevice cDevice = dawn_native::null::CreateDevice();

    dawnSetProcs(&procs);
    device = dawn::Device::Acquire(cDevice);

    device.SetErrorCallback(ValidationTest::OnDeviceError, static_cast<dawnCallbackUserdata>(reinterpret_cast<uintptr_t>(this)));
}

ValidationTest::~ValidationTest() {
    // We need to destroy Dawn objects before setting the procs to null otherwise the dawn*Release
    // will call a nullptr
    device = dawn::Device();
    dawnSetProcs(nullptr);
}

void ValidationTest::TearDown() {
    ASSERT_FALSE(mExpectError);

    for (auto& expectation : mExpectations) {
        std::string name = expectation.debugName;
        if (name.empty()) {
            name = "<no debug name set>";
        }

        ASSERT_TRUE(expectation.gotStatus) << "Didn't get a status for " << name;

        ASSERT_NE(DAWN_BUILDER_ERROR_STATUS_UNKNOWN, expectation.status) << "Got unknown status for " << name;

        bool wasSuccess = expectation.status == DAWN_BUILDER_ERROR_STATUS_SUCCESS;
        ASSERT_EQ(expectation.expectSuccess, wasSuccess)
            << "Got wrong status value for " << name
            << ", status was " << expectation.status << " with \"" << expectation.statusMessage << "\"";
    }
}

void ValidationTest::StartExpectDeviceError() {
    mExpectError = true;
    mError = false;
}
bool ValidationTest::EndExpectDeviceError() {
    mExpectError = false;
    return mError;
}
std::string ValidationTest::GetLastDeviceErrorMessage() const {
    return mDeviceErrorMessage;
}

dawn::RenderPassDescriptor ValidationTest::CreateSimpleRenderPass() {
        dawn::TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = 640;
        descriptor.size.height = 480;
        descriptor.size.depth = 1;
        descriptor.arrayLayer = 1;
        descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
        descriptor.mipLevel = 1;
        descriptor.usage = dawn::TextureUsageBit::OutputAttachment;

        auto colorBuffer = device.CreateTexture(&descriptor);
        auto colorView = colorBuffer.CreateDefaultTextureView();

        return device.CreateRenderPassDescriptorBuilder()
            .SetColorAttachment(0, colorView, dawn::LoadOp::Clear)
            .GetResult();
}

void ValidationTest::OnDeviceError(const char* message, dawnCallbackUserdata userdata) {
    auto self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata));
    self->mDeviceErrorMessage = message;

    // Skip this one specific error that is raised when a builder is used after it got an error
    // this is important because we don't want to wrap all creation tests in ASSERT_DEVICE_ERROR.
    // Yes the error message is misleading.
    if (self->mDeviceErrorMessage == "Builder cannot be used after GetResult") {
        return;
    }

    ASSERT_TRUE(self->mExpectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->mError) << "Got two errors in expect block";
    self->mError = true;
}

void ValidationTest::OnBuilderErrorStatus(dawnBuilderErrorStatus status, const char* message, dawn::CallbackUserdata userdata1, dawn::CallbackUserdata userdata2) {
    auto* self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata1));
    size_t index = static_cast<size_t>(userdata2);

    ASSERT_LT(index, self->mExpectations.size());

    auto& expectation = self->mExpectations[index];
    ASSERT_FALSE(expectation.gotStatus);
    expectation.gotStatus = true;
    expectation.status = status;
    expectation.statusMessage = message;
}

ValidationTest::DummyRenderPass ValidationTest::CreateDummyRenderPass() {
    DummyRenderPass dummy;
    dummy.width = 400;
    dummy.height = 400;
    dummy.attachmentFormat = dawn::TextureFormat::R8G8B8A8Unorm;

    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size.width = dummy.width;
    descriptor.size.height = dummy.height;
    descriptor.size.depth = 1;
    descriptor.arrayLayer = 1;
    descriptor.format = dummy.attachmentFormat;
    descriptor.mipLevel = 1;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;
    dummy.attachment = device.CreateTexture(&descriptor);

    dawn::TextureView view = dummy.attachment.CreateDefaultTextureView();

    dummy.renderPass = AssertWillBeSuccess(device.CreateRenderPassDescriptorBuilder())
        .SetColorAttachment(0, view, dawn::LoadOp::Clear)
        .GetResult();

    return dummy;
}
