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
    instance = std::make_unique<dawn::native::Instance>();
    instance->DiscoverDefaultAdapters();

    std::vector<dawn::native::Adapter> adapters = instance->GetAdapters();

    // DawnNative unittests run against the null backend, find the corresponding adapter
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

    device = wgpu::Device(CreateTestDevice());
    device.SetUncapturedErrorCallback(DawnNativeTest::OnDeviceError, nullptr);
}

void DawnNativeTest::TearDown() {}

WGPUDevice DawnNativeTest::CreateTestDevice() {
    // Disabled disallowing unsafe APIs so we can test them.
    wgpu::DeviceDescriptor deviceDescriptor = {};
    wgpu::DawnTogglesDeviceDescriptor togglesDesc = {};
    deviceDescriptor.nextInChain = &togglesDesc;

    const char* toggle = "disallow_unsafe_apis";
    togglesDesc.forceDisabledToggles = &toggle;
    togglesDesc.forceDisabledTogglesCount = 1;

    return adapter.CreateDevice(&deviceDescriptor);
}

// static
void DawnNativeTest::OnDeviceError(WGPUErrorType type, const char* message, void* userdata) {
    ASSERT(type != WGPUErrorType_NoError);
    FAIL() << "Unexpected error: " << message;
}
