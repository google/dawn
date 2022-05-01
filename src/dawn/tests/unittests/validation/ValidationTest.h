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

#ifndef SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_
#define SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_

#include <memory>
#include <string>

#include "dawn/common/Log.h"
#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// Argument helpers to allow macro overriding.
#define UNIMPLEMENTED_MACRO(...) UNREACHABLE()
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
    FlushWire();                                                \
    if (!EndExpectDeviceError()) {                              \
        FAIL() << "Expected device error in:\n " << #statement; \
    }                                                           \
    do {                                                        \
    } while (0)

#define ASSERT_DEVICE_ERROR_IMPL_2_(statement, matcher)         \
    StartExpectDeviceError(matcher);                            \
    statement;                                                  \
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

namespace utils {
class WireHelper;
}  // namespace utils

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

    wgpu::Device RegisterDevice(WGPUDevice backendDevice);

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

    bool HasToggleEnabled(const char* toggle) const;

    // TODO(crbug.com/dawn/689): Use limits returned from the wire
    // This is implemented here because tests need to always query
    // the |backendDevice| since limits are not implemented in the wire.
    wgpu::SupportedLimits GetSupportedLimits();

  protected:
    virtual WGPUDevice CreateTestDevice();

    std::unique_ptr<dawn::native::Instance> instance;
    dawn::native::Adapter adapter;
    wgpu::Device device;
    WGPUDevice backendDevice;

    size_t mLastWarningCount = 0;

  private:
    std::unique_ptr<utils::WireHelper> mWireHelper;

    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
    static void OnDeviceLost(WGPUDeviceLostReason reason, const char* message, void* userdata);
    std::string mDeviceErrorMessage;
    bool mExpectError = false;
    bool mError = false;
    testing::Matcher<std::string> mErrorMatcher;
    bool mExpectDestruction = false;
};

#endif  // SRC_DAWN_TESTS_UNITTESTS_VALIDATION_VALIDATIONTEST_H_
