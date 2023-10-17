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

#ifndef SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_
#define SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_

#include <memory>
#include <string>

#include "dawn/common/Log.h"
#include "dawn/native/BindGroupLayout.h"
#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Argument helpers to allow macro overriding.
#define UNIMPLEMENTED_MACRO(...) DAWN_UNREACHABLE()
#define GET_3RD_ARG_HELPER_(_1, _2, NAME, ...) NAME
#define GET_3RD_ARG_(args) GET_3RD_ARG_HELPER_ args

// Overloaded to allow further validation of the error messages given an error is expected.
// Especially useful to verify that the expected errors are occuring, not just any error.
//
// Example usages:
//   1 Argument Case:
//     ASSERT_DEVICE_ERROR(FunctionThatExpectsError());
//
//   2 Argument Case:
//     ASSERT_DEVICE_ERROR(FunctionThatHasLongError(), HasSubstr("partial match"))
//     ASSERT_DEVICE_ERROR(FunctionThatHasShortError(), Eq("exact match"));
#define ASSERT_DEVICE_ERROR(...)                                                         \
    GET_3RD_ARG_((__VA_ARGS__, ASSERT_DEVICE_ERROR_IMPL_2_, ASSERT_DEVICE_ERROR_IMPL_1_, \
                  UNIMPLEMENTED_MACRO))                                                  \
    (__VA_ARGS__)

#define ASSERT_DEVICE_ERROR_IMPL_1_(statement)                  \
    StartExpectDeviceError();                                   \
    statement;                                                  \
    device.Tick();                                              \
    FlushWire();                                                \
    if (!EndExpectDeviceError()) {                              \
        FAIL() << "Expected device error in:\n " << #statement; \
    }                                                           \
    do {                                                        \
    } while (0)

#define ASSERT_DEVICE_ERROR_IMPL_2_(statement, matcher)         \
    StartExpectDeviceError(matcher);                            \
    statement;                                                  \
    device.Tick();                                              \
    FlushWire();                                                \
    if (!EndExpectDeviceError()) {                              \
        FAIL() << "Expected device error in:\n " << #statement; \
    }                                                           \
    do {                                                        \
    } while (0)

// Skip a test when the given condition is satisfied.
#define DAWN_SKIP_TEST_IF(condition)                            \
    do {                                                        \
        if (condition) {                                        \
            dawn::InfoLog() << "Test skipped: " #condition "."; \
            GTEST_SKIP();                                       \
            return;                                             \
        }                                                       \
    } while (0)

#define EXPECT_DEPRECATION_WARNINGS(statement, n)                                                  \
    do {                                                                                           \
        FlushWire();                                                                               \
        size_t warningsBefore = dawn::native::GetDeprecationWarningCountForTesting(backendDevice); \
        EXPECT_EQ(mLastWarningCount, warningsBefore);                                              \
        statement;                                                                                 \
        FlushWire();                                                                               \
        size_t warningsAfter = dawn::native::GetDeprecationWarningCountForTesting(backendDevice);  \
        EXPECT_EQ(warningsAfter, warningsBefore + n);                                              \
        mLastWarningCount = warningsAfter;                                                         \
    } while (0)
#define EXPECT_DEPRECATION_WARNING(statement) EXPECT_DEPRECATION_WARNINGS(statement, 1)

// Gmock matcher helpers that may be used throughout other tests.

// BindGroupLayouts can either be cache equivalent meaning that they may have different
// compatibility tokens but same internal layout, or fully equivalent meaning that they have the
// same token and internal layout. Note that being fully equivalent implies that they are cache
// equivalent.
MATCHER_P(BindGroupLayoutCacheEq, other, "") {
    return dawn::native::FromAPI(arg.Get())->GetInternalBindGroupLayout() ==
           dawn::native::FromAPI(other.Get())->GetInternalBindGroupLayout();
}
MATCHER_P(BindGroupLayoutEq, other, "") {
    return dawn::native::FromAPI(arg.Get())->IsLayoutEqual(dawn::native::FromAPI(other.Get()));
}

namespace dawn::utils {
class WireHelper;
}  // namespace dawn::utils

void InitDawnValidationTestEnvironment(int argc, char** argv);

class ValidationTest : public testing::Test {
  public:
    ValidationTest();
    ~ValidationTest() override;

    void SetUp() override;
    void TearDown() override;

    void StartExpectDeviceError(testing::Matcher<std::string> errorMatcher);
    void StartExpectDeviceError();
    bool EndExpectDeviceError();
    std::string GetLastDeviceErrorMessage() const;

    void ExpectDeviceDestruction();

    bool UsesWire() const;

    void FlushWire();
    void WaitForAllOperations(const wgpu::Device& device);

    // Helper functions to create objects to test validation.

    struct PlaceholderRenderPass : public wgpu::RenderPassDescriptor {
      public:
        explicit PlaceholderRenderPass(const wgpu::Device& device);
        wgpu::Texture attachment;
        wgpu::TextureFormat attachmentFormat;
        uint32_t width;
        uint32_t height;

      private:
        wgpu::RenderPassColorAttachment mColorAttachment;
    };

    const dawn::native::ToggleInfo* GetToggleInfo(const char* name) const;
    bool HasToggleEnabled(const char* toggle) const;
    wgpu::SupportedLimits GetSupportedLimits() const;

  protected:
    dawn::native::Adapter& GetBackendAdapter();
    // Helper function to create testing adapter and store into ValidationTest::adapter during
    // SetUp. Override this function to change the adapter creation behavior.
    virtual void CreateTestAdapter(wgpu::Instance instance, wgpu::RequestAdapterOptions options);
    virtual WGPUDevice CreateTestDevice(dawn::native::Adapter dawnAdapter,
                                        wgpu::DeviceDescriptor descriptor);

    wgpu::Device RequestDeviceSync(const wgpu::DeviceDescriptor& deviceDesc);

    virtual bool UseCompatibilityMode() const;

    wgpu::Device device;
    wgpu::Adapter adapter;
    WGPUDevice backendDevice;

    size_t mLastWarningCount = 0;

  private:
    std::unique_ptr<dawn::native::Instance> mDawnInstance;
    wgpu::Instance mInstance;
    dawn::native::Adapter mBackendAdapter;
    std::unique_ptr<dawn::utils::WireHelper> mWireHelper;
    WGPUDevice mLastCreatedBackendDevice;

    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
    static void OnDeviceLost(WGPUDeviceLostReason reason, const char* message, void* userdata);
    std::string mDeviceErrorMessage;
    bool mExpectError = false;
    bool mError = false;
    testing::Matcher<std::string> mErrorMatcher;
    bool mExpectDestruction = false;
};

#endif  // SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_
