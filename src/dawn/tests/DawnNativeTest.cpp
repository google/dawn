// Copyright 2021 The Dawn Authors
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

#include "dawn/tests/DawnNativeTest.h"

#include <vector>

#include "absl/strings/str_cat.h"
#include "dawn/common/Assert.h"
#include "dawn/dawn_proc.h"
#include "dawn/native/ErrorData.h"
#include "dawn/native/Instance.h"
#include "dawn/native/dawn_platform.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::native {

void AddFatalDawnFailure(const char* expression, const ErrorData* error) {
    const auto& backtrace = error->GetBacktrace();
    GTEST_MESSAGE_AT_(backtrace.at(0).file, backtrace.at(0).line,
                      absl::StrCat(expression, " returned error: ", error->GetMessage()).c_str(),
                      ::testing::TestPartResult::kFatalFailure);
}

}  // namespace dawn::native

DawnNativeTest::DawnNativeTest() {
    dawnProcSetProcs(&dawn::native::GetProcs());
}

DawnNativeTest::~DawnNativeTest() {
    device = wgpu::Device();
    dawnProcSetProcs(nullptr);
}

void DawnNativeTest::SetUp() {
    // Create an instance with toggle AllowUnsafeAPIs enabled, which would be inherited to
    // adapter and device toggles and allow us to test unsafe apis (including experimental
    // features).
    const char* allowUnsafeApisToggle = "allow_unsafe_apis";
    wgpu::DawnTogglesDescriptor instanceToggles;
    instanceToggles.enabledTogglesCount = 1;
    instanceToggles.enabledToggles = &allowUnsafeApisToggle;

    platform = CreateTestPlatform();
    wgpu::DawnInstanceDescriptor dawnInstanceDesc;
    dawnInstanceDesc.platform = platform.get();
    dawnInstanceDesc.nextInChain = &instanceToggles;

    wgpu::InstanceDescriptor instanceDesc;
    instanceDesc.nextInChain = &dawnInstanceDesc;

    instance = std::make_unique<dawn::native::Instance>(
        reinterpret_cast<const WGPUInstanceDescriptor*>(&instanceDesc));
    instance->EnableAdapterBlocklist(false);

    wgpu::RequestAdapterOptionsBackendType backendTypeOptions = {};
    backendTypeOptions.backendType = wgpu::BackendType::Null;

    wgpu::RequestAdapterOptions options = {};
    options.nextInChain = &backendTypeOptions;

    adapter = instance->EnumerateAdapters(&options)[0];
    device = wgpu::Device::Acquire(CreateTestDevice());
    device.SetUncapturedErrorCallback(DawnNativeTest::OnDeviceError, nullptr);
}

std::unique_ptr<dawn::platform::Platform> DawnNativeTest::CreateTestPlatform() {
    return nullptr;
}

WGPUDevice DawnNativeTest::CreateTestDevice() {
    return adapter.CreateDevice();
}

// static
void DawnNativeTest::OnDeviceError(WGPUErrorType type, const char* message, void* userdata) {
    ASSERT(type != WGPUErrorType_NoError);
    FAIL() << "Unexpected error: " << message;
}
