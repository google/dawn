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

#ifndef TESTS_DAWNNATIVETEST_H_
#define TESTS_DAWNNATIVETEST_H_

#include <gtest/gtest.h>

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "dawn_native/ErrorData.h"

namespace dawn_native {

    // This is similar to DAWN_TRY_ASSIGN but produces a fatal GTest error if EXPR is an error.
#define DAWN_ASSERT_AND_ASSIGN(VAR, EXPR) \
    DAWN_TRY_ASSIGN_WITH_CLEANUP(VAR, EXPR, {}, AddFatalDawnFailure(#EXPR, error.get()))

    void AddFatalDawnFailure(const char* expression, const ErrorData* error);

}  // namespace dawn_native

class DawnNativeTest : public ::testing::Test {
  public:
    DawnNativeTest();
    ~DawnNativeTest() override;

    void SetUp() override;
    void TearDown() override;

    virtual WGPUDevice CreateTestDevice();

  protected:
    std::unique_ptr<dawn_native::Instance> instance;
    dawn_native::Adapter adapter;
    wgpu::Device device;

  private:
    static void OnDeviceError(WGPUErrorType type, const char* message, void* userdata);
};

#endif  // TESTS_DAWNNATIVETEST_H_
