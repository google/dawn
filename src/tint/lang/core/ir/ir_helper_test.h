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

#ifndef SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"

namespace tint::ir {

/// Helper class for testing
template <typename BASE>
class IRTestHelperBase : public BASE {
  public:
    IRTestHelperBase() = default;
    ~IRTestHelperBase() override = default;

    /// The IR module
    Module mod;
    /// The IR builder
    Builder b{mod};
    /// The type manager
    type::Manager& ty{mod.Types()};
};

using IRTestHelper = IRTestHelperBase<testing::Test>;

template <typename T>
using IRTestParamHelper = IRTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_IR_HELPER_TEST_H_
