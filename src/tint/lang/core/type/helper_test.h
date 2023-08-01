// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_

#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::type {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public BASE, public ProgramBuilder {
  public:
    /// Builds and returns the program. Must only be called once per test
    /// @return the built program
    Program Build() {
        [&] {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n" << Diagnostics().str();
        }();
        return resolver::Resolve(*this);
    }
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::type

/// Helper macro for testing that a type was as expected
#define EXPECT_TYPE(GOT, EXPECT)                                                                \
    do {                                                                                        \
        const type::Type* got = GOT;                                                            \
        const type::Type* expect = EXPECT;                                                      \
        if (got != expect) {                                                                    \
            ADD_FAILURE() << #GOT " != " #EXPECT "\n"                                           \
                          << "  " #GOT ": " << (got ? got->FriendlyName() : "<null>") << "\n"   \
                          << "  " #EXPECT ": " << (expect ? expect->FriendlyName() : "<null>"); \
        }                                                                                       \
    } while (false)

#endif  // SRC_TINT_LANG_CORE_TYPE_HELPER_TEST_H_
