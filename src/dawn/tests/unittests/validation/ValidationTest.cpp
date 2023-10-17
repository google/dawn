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

#include <algorithm>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/SystemUtils.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/Adapter.h"
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
static ValidationTest* gCurrentTest = nullptr;

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

ValidationTest::ValidationTest() {
    gCurrentTest = this;

    DawnProcTable procs = dawn::native::GetProcs();

    // Forward to dawn::native instanceRequestAdapter, but save the returned adapter in
    // gCurrentTest->mBackendAdapter.
    procs.instanceRequestAdapter = [](WGPUInstance instance,
                                      const WGPURequestAdapterOptions* options,
                                      WGPURequestAdapterCallback callback, void* userdata) {
        DAWN_ASSERT(gCurrentTest);
        dawn::native::GetProcs().instanceRequestAdapter(
            instance, options,
            [](WGPURequestAdapterStatus status, WGPUAdapter cAdapter, char const* message,
               void* userdata) {
                gCurrentTest->mBackendAdapter = dawn::native::FromAPI(cAdapter);

                auto* callbackAndUserdata =
                    static_cast<std::pair<WGPURequestAdapterCallback, void*>*>(userdata);
                callbackAndUserdata->first(status, cAdapter, message, callbackAndUserdata->second);
                delete callbackAndUserdata;
            },
            new std::pair<WGPURequestAdapterCallback, void*>(callback, userdata));
    };

    procs.adapterRequestDevice = [](WGPUAdapter self, const WGPUDeviceDescriptor* descriptor,
                                    WGPURequestDeviceCallback callback, void* userdata) {
        DAWN_ASSERT(gCurrentTest);
        wgpu::DeviceDescriptor deviceDesc =
            *(reinterpret_cast<const wgpu::DeviceDescriptor*>(descriptor));
        WGPUDevice cDevice = gCurrentTest->CreateTestDevice(
            dawn::native::Adapter(reinterpret_cast<dawn::native::AdapterBase*>(self)), deviceDesc);
        DAWN_ASSERT(cDevice != nullptr);
        gCurrentTest->mLastCreatedBackendDevice = cDevice;
        callback(WGPURequestDeviceStatus_Success, cDevice, nullptr, userdata);
    };

    mWireHelper = dawn::utils::CreateWireHelper(procs, gUseWire, gWireTraceDir.c_str());
}

void ValidationTest::SetUp() {
    // Create an instance with toggle AllowUnsafeAPIs enabled, which would be inherited to
    // adapter and device toggles and allow us to test unsafe apis (including experimental
    // features). To test device with AllowUnsafeAPIs disabled, require it in device toggles
    // descriptor to override the inheritance.
    const char* allowUnsafeApisToggle = "allow_unsafe_apis";
    WGPUDawnTogglesDescriptor instanceToggles = {};
    instanceToggles.chain.sType = WGPUSType::WGPUSType_DawnTogglesDescriptor;
    instanceToggles.enabledToggleCount = 1;
    instanceToggles.enabledToggles = &allowUnsafeApisToggle;

    WGPUInstanceDescriptor instanceDesc = {};
    instanceDesc.nextInChain = &instanceToggles.chain;

    mDawnInstance = std::make_unique<dawn::native::Instance>(&instanceDesc);
    mInstance = mWireHelper->RegisterInstance(mDawnInstance->Get());

    std::string traceName =
        std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) +
        "_" + ::testing::UnitTest::GetInstance()->current_test_info()->name();
    mWireHelper->BeginWireTrace(traceName.c_str());

    wgpu::RequestAdapterOptions options = {};
    options.backendType = wgpu::BackendType::Null;
    options.compatibilityMode = gCurrentTest->UseCompatibilityMode();

    CreateTestAdapter(mInstance, options);
    DAWN_ASSERT(adapter);

    wgpu::DeviceDescriptor deviceDescriptor = {};
    deviceDescriptor.deviceLostCallback = ValidationTest::OnDeviceLost;
    deviceDescriptor.deviceLostUserdata = this;

    device = RequestDeviceSync(deviceDescriptor);
    backendDevice = mLastCreatedBackendDevice;

    device.SetUncapturedErrorCallback(ValidationTest::OnDeviceError, this);
}

ValidationTest::~ValidationTest() {
    // We need to destroy Dawn objects before the wire helper which sets procs to null otherwise the
    // dawn*Release will call a nullptr
    device = nullptr;
    adapter = nullptr;
    mInstance = nullptr;
    mWireHelper.reset();

    // Check that all devices were destructed.
    // Note that if the test is skipped before SetUp is called, mDawnInstance will not get set and
    // remain nullptr.
    if (mDawnInstance) {
        EXPECT_EQ(mDawnInstance->GetDeviceCountForTesting(), 0u);
    }

    gCurrentTest = nullptr;
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

bool ValidationTest::UsesWire() const {
    return gUseWire;
}

void ValidationTest::FlushWire() {
    EXPECT_TRUE(mWireHelper->FlushClient());
    EXPECT_TRUE(mWireHelper->FlushServer());
}

void ValidationTest::WaitForAllOperations(const wgpu::Device& waitDevice) {
    bool done = false;
    waitDevice.GetQueue().OnSubmittedWorkDone(
        [](WGPUQueueWorkDoneStatus, void* userdata) { *static_cast<bool*>(userdata) = true; },
        &done);

    // Force the currently submitted operations to completed.
    while (!done) {
        waitDevice.Tick();
        FlushWire();
    }

    // TODO(cwallez@chromium.org): It's not clear why we need this additional tick. Investigate it
    // once WebGPU has defined the ordering of callbacks firing.
    waitDevice.Tick();
    FlushWire();
}

const dawn::native::ToggleInfo* ValidationTest::GetToggleInfo(const char* name) const {
    return mDawnInstance->GetToggleInfo(name);
}

bool ValidationTest::HasToggleEnabled(const char* toggle) const {
    auto toggles = dawn::native::GetTogglesUsed(backendDevice);
    return std::find_if(toggles.begin(), toggles.end(), [toggle](const char* name) {
               return strcmp(toggle, name) == 0;
           }) != toggles.end();
}

wgpu::SupportedLimits ValidationTest::GetSupportedLimits() const {
    wgpu::SupportedLimits supportedLimits = {};
    device.GetLimits(&supportedLimits);
    return supportedLimits;
}

wgpu::Device ValidationTest::RequestDeviceSync(const wgpu::DeviceDescriptor& deviceDesc) {
    DAWN_ASSERT(adapter);

    wgpu::Device apiDevice;
    adapter.RequestDevice(
        &deviceDesc,
        [](WGPURequestDeviceStatus, WGPUDevice cDevice, const char*, void* userdata) {
            *static_cast<wgpu::Device*>(userdata) = wgpu::Device::Acquire(cDevice);
        },
        &apiDevice);
    FlushWire();

    DAWN_ASSERT(apiDevice);
    return apiDevice;
}

dawn::native::Adapter& ValidationTest::GetBackendAdapter() {
    return mBackendAdapter;
}

void ValidationTest::CreateTestAdapter(wgpu::Instance instance,
                                       wgpu::RequestAdapterOptions options) {
    instance.RequestAdapter(
        &options,
        [](WGPURequestAdapterStatus, WGPUAdapter cAdapter, const char*, void* userdata) {
            *static_cast<wgpu::Adapter*>(userdata) = wgpu::Adapter::Acquire(cAdapter);
        },
        &adapter);
    FlushWire();
}

WGPUDevice ValidationTest::CreateTestDevice(dawn::native::Adapter dawnAdapter,
                                            wgpu::DeviceDescriptor deviceDescriptor) {
    std::vector<const char*> enabledToggles;
    std::vector<const char*> disabledToggles;

    for (const std::string& toggle : gToggleParser->GetEnabledToggles()) {
        enabledToggles.push_back(toggle.c_str());
    }

    for (const std::string& toggle : gToggleParser->GetDisabledToggles()) {
        disabledToggles.push_back(toggle.c_str());
    }

    wgpu::DawnTogglesDescriptor deviceTogglesDesc;
    deviceDescriptor.nextInChain = &deviceTogglesDesc;

    deviceTogglesDesc.enabledToggles = enabledToggles.data();
    deviceTogglesDesc.enabledToggleCount = enabledToggles.size();
    deviceTogglesDesc.disabledToggles = disabledToggles.data();
    deviceTogglesDesc.disabledToggleCount = disabledToggles.size();

    return dawnAdapter.CreateDevice(&deviceDescriptor);
}

bool ValidationTest::UseCompatibilityMode() const {
    return false;
}

// static
void ValidationTest::OnDeviceError(WGPUErrorType type, const char* message, void* userdata) {
    DAWN_ASSERT(type != WGPUErrorType_NoError);
    auto* self = static_cast<ValidationTest*>(userdata);

    ASSERT_TRUE(self->mExpectError) << "Got unexpected device error: " << message;
    ASSERT_FALSE(self->mError) << "Got two errors in expect block, first one is:\n"  //
                               << self->mDeviceErrorMessage                          //
                               << "\nsecond one is:\n"                               //
                               << message;

    self->mDeviceErrorMessage = message;
    if (self->mExpectError) {
        ASSERT_THAT(message, self->mErrorMatcher);
    }
    self->mError = true;
}

void ValidationTest::OnDeviceLost(WGPUDeviceLostReason reason,
                                  const char* message,
                                  void* userdata) {
    auto* self = static_cast<ValidationTest*>(userdata);
    if (self->mExpectDestruction) {
        EXPECT_EQ(reason, WGPUDeviceLostReason_Destroyed);
        return;
    }
    ADD_FAILURE() << "Device lost during test: " << message;
    DAWN_ASSERT(false);
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
