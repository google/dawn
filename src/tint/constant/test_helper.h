// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_CONSTANT_TEST_HELPER_H_
#define SRC_TINT_CONSTANT_TEST_HELPER_H_

#include "gtest/gtest.h"
#include "src/tint/program_builder.h"

namespace tint::constant {

/// Helper base class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {};

/// Helper class for testing that derives from testing::Test.
using TestHelper = TestHelperBase<testing::Test>;

/// Helper class for testing that derives from `T`.
template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::constant

#endif  // SRC_TINT_CONSTANT_TEST_HELPER_H_
