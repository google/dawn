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

#include <algorithm>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/NullBackend.h"
#include "dawn/tests/ToggleParser.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WireHelper.h"
#include "dawn/webgpu.h"

namespace {

bool gUseWire = false;
// NOLINTNEXTLINE(runtime/string)
std::string gWireTraceDir = "";
std::unique_ptr<ToggleParser> gToggleParser = nullptr;

}  // namespace

void InitDawnValidationTestEnvironment(int argc, char** argv) {
    gToggleParser = std::make_unique<ToggleParser>();

    for (int i = 1; i < argc; ++i) {
        if (strcmp("-w", argv[i]) == 0 || strcmp("--use-wire", argv[i]) == 0) {
            gUseWire = true;
            continue;
        }

        constexpr const char kWireTraceDirArg[] = "--wire-trace-dir=";
        size_t argLen = sizeof(kWireTraceDirArg) - 1;
        if (strncmp(argv[i], kWireTraceDirArg, argLen) == 0) {
            gWireTraceDir = argv[i] + argLen;
            continue;
        }

        if (gToggleParser->ParseEnabledToggles(argv[i])) {
            continue;
        }

        if (gToggleParser->ParseDisabledToggles(argv[i])) {
            continue;
        }

        if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
            dawn::InfoLog()
                << "\n\nUsage: " << argv[0]
                << " [GTEST_FLAGS...] [-w]\n"
                   "    [--enable-toggles=toggles] [--disable-toggles=toggles]\n"
                   "  -w, --use-wire: Run the tests through the wire (defaults to no wire)\n"
                   "  --enable-toggles: Comma-delimited list of Dawn toggles to enable.\n"
                   "    ex.) skip_validation,disable_robustness,turn_off_vsync\n"
                   "  --disable-toggles: Comma-delimited list of Dawn toggles to disable\n";
            continue;
        }

        // Skip over args that look like they're for Googletest.
        constexpr const char kGtestArgPrefix[] = "--gtest_";
        if (strncmp(kGtestArgPrefix, argv[i], sizeof(kGtestArgPrefix) - 1) == 0) {
            continue;
        }

        dawn::WarningLog() << " Unused argument: " << argv[i];
    }
}

ValidationTest::ValidationTest()
    : mWireHelper(utils::CreateWireHelper(gUseWire, gWireTraceDir.c_str())) {}

void ValidationTest::SetUp() {
    instance = std::make_unique<dawn::native::Instance>();
    instance->DiscoverDefaultAdapters();

    std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();

    // Validation tests run against the null backend, find the corresponding adapter
    bool foundNullAdapter = false;
    for (auto& currentAdapter : adapters) {
        wgpu::AdapterProperties adapterProperties;
        currentAdapter.GetProperties(&adapterProperties);

        if (adapterProperties.backendType == wgpu::BackendType::Null) {
            adapter = currentAdapter;
            foundNullAdapter = true;
            break;
        }
    }

    ASSERT(foundNullAdapter);

    std::tie(device, backendDevice) = mWireHelper->RegisterDevice(CreateTestDevice());
    device.SetUncapturedErrorCallback(ValidationTest::OnDeviceError, this);
    device.SetDeviceLostCallback(ValidationTest::OnDeviceLost, this);

    std::string traceName =
        std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
        "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    mWireHelper->BeginWireTrace(traceName.c_str());
}

ValidationTest::~ValidationTest() {
    // We need to destroy Dawn objects before setting the procs to null otherwise the dawn*Release
    // will call a nullptr
    device = wgpu::Device();
    mWireHelper.reset();
}

void ValidationTest::TearDown() {
    FlushWire();
    ASSERT_FALSE(mExpectError);

    if (device) {
        EXPECT_EQ(mLastWarningCount,
                  dawn::native::GetDeprecationWarningCountForTesting(backendDevice));
    }

    // The device will be destroyed soon after, so we want to set the expectation.
    ExpectDeviceDestruction();
}

void ValidationTest::StartExpectDeviceError(testing::Matcher<std::string> errorMatcher) {
    mExpectError = true;
    mError = false;
    mErrorMatcher = errorMatcher;
}

void ValidationTest::StartExpectDeviceError() {
    StartExpectDeviceError(testing::_);
}

bool ValidationTest::EndExpectDeviceError() {
    mExpectError = false;
    mErrorMatcher = testing::_;
    return mError;
}
std::string ValidationTest::GetLastDeviceErrorMessage() const {
    return mDeviceErrorMessage;
}

void ValidationTest::ExpectDeviceDestruction() {
    mExpectDestruction = true;
}

wgpu::Device ValidationTest::RegisterDevice(WGPUDevice backendDevice) {
    return mWireHelper->RegisterDevice(backendDevice).first;
}

bool ValidationTest::UsesWire() const {
    return gUseWire;
}

void ValidationTest::FlushWire() {
    EXPECT_TRUE(mWireHelper->FlushClient());
    EXPECT_TRUE(mWireHelper->FlushServer());
}

void ValidationTest::WaitForAllOperations(const wgpu::Device& device) {
    bool done = false;
    device.GetQueue().OnSubmittedWorkDone(
        0u, [](WGPUQueueWorkDoneStatus, void* userdata) { *static_cast<bool*>(userdata) = true; },
        &done);

    // Force the currently submitted operations to completed.
    while (!done) {
        device.Tick();
        FlushWire();
    }

    // TODO(cwallez@chromium.org): It's not clear why we need this additional tick. Investigate it
    // once WebGPU has defined the ordering of callbacks firing.
    device.Tick();
    FlushWire();
}

bool ValidationTest::HasToggleEnabled(const char* toggle) const {
    auto toggles = dawn::native::GetTogglesUsed(backendDevice);
    return std::find_if(toggles.begin(), toggles.end(), [toggle](const char* name) {
               return strcmp(toggle, name) == 0;
           }) != toggles.end();
}

wgpu::SupportedLimits ValidationTest::GetSupportedLimits() {
    WGPUSupportedLimits supportedLimits;
    supportedLimits.nextInChain = nullptr;
    dawn::native::GetProcs().deviceGetLimits(backendDevice, &supportedLimits);
    return *reinterpret_cast<wgpu::SupportedLimits*>(&supportedLimits);
}

WGPUDevice ValidationTest::CreateTestDevice() {
    // Disabled disallowing unsafe APIs so we can test them.
    std::vector<const char*> forceEnabledToggles;
    std::vector<const char*> forceDisabledToggles = {"disallow_unsafe_apis"};

    for (const std::string& toggle : gToggleParser->GetEnabledToggles()) {
        forceEnabledToggles.push_back(toggle.c_str());
    }

    for (const std::string& toggle : gToggleParser->GetDisabledToggles()) {
        forceDisabledToggles.push_back(toggle.c_str());
    }

    wgpu::DeviceDescriptor deviceDescriptor;
    wgpu::DawnTogglesDeviceDescriptor togglesDesc;
    deviceDescriptor.nextInChain = &togglesDesc;

    togglesDesc.forceEnabledToggles = forceEnabledToggles.data();
    togglesDesc.forceEnabledTogglesCount = forceEnabledToggles.size();
    togglesDesc.forceDisabledToggles = forceDisabledToggles.data();
    togglesDesc.forceDisabledTogglesCount = forceDisabledToggles.size();

    return adapter.CreateDevice(&deviceDescriptor);
}

// static
void ValidationTest::OnDeviceError(WGPUErrorType type, const char* message, void* userdata) {
    ASSERT(type != WGPUErrorType_NoError);
    auto self = static_cast<ValidationTest*>(userdata);
    self->mDeviceErrorMessage = message;

    ASSERT_TRUE(self->mExpectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->mError) << "Got two errors in expect block";
    if (self->mExpectError) {
        ASSERT_THAT(message, self->mErrorMatcher);
    }
    self->mError = true;
}

void ValidationTest::OnDeviceLost(WGPUDeviceLostReason reason,
                                  const char* message,
                                  void* userdata) {
    auto self = static_cast<ValidationTest*>(userdata);
    if (self->mExpectDestruction) {
        EXPECT_EQ(reason, WGPUDeviceLostReason_Destroyed);
        return;
    }
    ADD_FAILURE() << "Device lost during test: " << message;
    ASSERT(false);
}

ValidationTest::PlaceholderRenderPass::PlaceholderRenderPass(const wgpu::Device& device)
    : attachmentFormat(wgpu::TextureFormat::RGBA8Unorm), width(400), height(400) {
    wgpu::TextureDescriptor descriptor;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.size.width = width;
    descriptor.size.height = height;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.sampleCount = 1;
    descriptor.format = attachmentFormat;
    descriptor.mipLevelCount = 1;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    attachment = device.CreateTexture(&descriptor);

    wgpu::TextureView view = attachment.CreateView();
    mColorAttachment.view = view;
    mColorAttachment.resolveTarget = nullptr;
    mColorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
    mColorAttachment.loadOp = wgpu::LoadOp::Clear;
    mColorAttachment.storeOp = wgpu::StoreOp::Store;

    colorAttachmentCount = 1;
    colorAttachments = &mColorAttachment;
    depthStencilAttachment = nullptr;
}
