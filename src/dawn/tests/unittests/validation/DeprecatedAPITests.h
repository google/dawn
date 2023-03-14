// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_UNITTESTS_VALIDATION_DEPRECATEDAPITESTS_H_
#define SRC_DAWN_TESTS_UNITTESTS_VALIDATION_DEPRECATEDAPITESTS_H_

#include "dawn/tests/unittests/validation/ValidationTest.h"

// This test header should be included when testing for deprecated parts of Dawn's API while
// following WebGPU's evolution. Tests in this suite test that a deprecation warning is emitted when
// the "old" behavior is used, and tests that an error is emitted when both the old and the new
// behavior are used (when applicable). Note that implementations of tests in this suite may be
// scattered in other files as well for organizational purposes so that similar tests can live
// together.

static constexpr char kAllowDeprecatedAPIsToggleName[] = "allow_deprecated_apis";

#define EXPECT_DEPRECATION_ERROR_OR_WARNING(statement)       \
    if (!HasToggleEnabled(kAllowDeprecatedAPIsToggleName)) { \
        ASSERT_DEVICE_ERROR(statement);                      \
    } else {                                                 \
        EXPECT_DEPRECATION_WARNING(statement);               \
    }                                                        \
    for (;;)                                                 \
    break

#define EXPECT_DEPRECATION_WARNING_ONLY(statement)          \
    if (HasToggleEnabled(kAllowDeprecatedAPIsToggleName)) { \
        EXPECT_DEPRECATION_WARNING(statement);              \
    } else {                                                \
        statement;                                          \
    }                                                       \
    for (;;)                                                \
    break

#define EXPECT_DEPRECATION_ERROR_ONLY(statement)             \
    if (!HasToggleEnabled(kAllowDeprecatedAPIsToggleName)) { \
        ASSERT_DEVICE_ERROR(statement);                      \
    } else {                                                 \
        statement;                                           \
    }                                                        \
    for (;;)                                                 \
    break

// Parameter is a single bool. When true, deprecated APIs are strictly disallowed (i.e. generate
// errors). Otherwise, deprecated APIs only generate a warning message.
class DeprecationTests : public ValidationTest, public testing::WithParamInterface<bool> {
  protected:
    WGPUDevice CreateTestDevice(dawn::native::Adapter dawnAdapter) override;
};

#endif  // SRC_DAWN_TESTS_UNITTESTS_VALIDATION_DEPRECATEDAPITESTS_H_
