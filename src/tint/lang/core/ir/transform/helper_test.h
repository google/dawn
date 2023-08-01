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

#ifndef SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_
#define SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/transform/transform.h"
#include "src/tint/lang/core/ir/validator.h"

namespace tint::ir::transform {

/// Helper class for testing IR transforms.
template <typename BASE>
class TransformTestBase : public BASE {
  public:
    /// Transforms the module, using the transforms `TRANSFORMS`.
    template <typename... TRANSFORMS>
    void Run() {
        // Validate the input IR.
        {
            auto res = ir::Validate(mod);
            EXPECT_TRUE(res) << res.Failure().str();
            if (!res) {
                return;
            }
        }

        // Run the transforms.
        (TRANSFORMS().Run(&mod), ...);

        // Validate the output IR.
        auto res = ir::Validate(mod);
        EXPECT_TRUE(res) << res.Failure().str();
    }

    /// Transforms the module, using @p transform.
    /// @param transform_func the transform to run
    void Run(std::function<Result<SuccessType, std::string>(Module*)> transform_func) {
        // Run the transform.
        auto result = transform_func(&mod);
        EXPECT_TRUE(result) << result.Failure();
        if (!result) {
            return;
        }

        // Validate the output IR.
        auto valid = ir::Validate(mod);
        EXPECT_TRUE(valid) << valid.Failure().str();
    }

    /// @returns the transformed module as a disassembled string
    std::string str() {
        ir::Disassembler dis(mod);
        return "\n" + dis.Disassemble();
    }

  protected:
    /// The test IR module.
    ir::Module mod;
    /// The test IR builder.
    ir::Builder b{mod};
    /// The type manager.
    type::Manager& ty{mod.Types()};
};

using TransformTest = TransformTestBase<testing::Test>;

template <typename T>
using TransformTestWithParam = TransformTestBase<testing::TestWithParam<T>>;

}  // namespace tint::ir::transform

#endif  // SRC_TINT_LANG_CORE_IR_TRANSFORM_HELPER_TEST_H_
