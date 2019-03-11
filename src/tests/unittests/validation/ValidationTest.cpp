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

#include "common/Assert.h"
#include "dawn/dawn.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/NullBackend.h"

ValidationTest::ValidationTest() {
    mInstance = std::make_unique<dawn_native::Instance>();
    mInstance->DiscoverDefaultAdapters();

    std::vector<dawn_native::Adapter> adapters = mInstance->GetAdapters();

    // Validation tests run against the null backend, find the corresponding adapter
    bool foundNullAdapter = false;
    dawn_native::Adapter nullAdapter;
    for (auto adapter : adapters) {
        if (adapter.GetBackendType() == dawn_native::BackendType::Null) {
            nullAdapter = adapter;
            foundNullAdapter = true;
            break;
        }
    }

    ASSERT(foundNullAdapter);
    device = dawn::Device::Acquire(nullAdapter.CreateDevice());

    DawnProcTable procs = dawn_native::GetProcs();
    dawnSetProcs(&procs);

    device.SetErrorCallback(ValidationTest::OnDeviceError, static_cast<DawnCallbackUserdata>(reinterpret_cast<uintptr_t>(this)));
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

// static
void ValidationTest::OnDeviceError(const char* message, DawnCallbackUserdata userdata) {
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

// static
void ValidationTest::OnBuilderErrorStatus(DawnBuilderErrorStatus status, const char* message, dawn::CallbackUserdata userdata1, dawn::CallbackUserdata userdata2) {
    auto* self = reinterpret_cast<ValidationTest*>(static_cast<uintptr_t>(userdata1));
    size_t index = static_cast<size_t>(userdata2);

    ASSERT_LT(index, self->mExpectations.size());

    auto& expectation = self->mExpectations[index];
    ASSERT_FALSE(expectation.gotStatus);
    expectation.gotStatus = true;
    expectation.status = status;
    expectation.statusMessage = message;
}

ValidationTest::DummyRenderPass::DummyRenderPass(const dawn::Device& device)
    : attachmentFormat(dawn::TextureFormat::R8G8B8A8Unorm), width(400), height(400) {

    dawn::TextureDescriptor descriptor;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depth = 1;
    descriptor.arrayLayerCount = 1;
    descriptor.sampleCount = 1;
    descriptor.format = attachmentFormat;
    descriptor.mipLevelCount = 1;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment;
    attachment = device.CreateTexture(&descriptor);

    dawn::TextureView view = attachment.CreateDefaultTextureView();
    mColorAttachment.attachment = view;
    mColorAttachment.resolveTarget = nullptr;
    mColorAttachment.clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    mColorAttachment.loadOp = dawn::LoadOp::Clear;
    mColorAttachment.storeOp = dawn::StoreOp::Store;
    mColorAttachments[0] = &mColorAttachment;

    colorAttachmentCount = 1;
    colorAttachments = mColorAttachments;
    depthStencilAttachment = nullptr;
}
