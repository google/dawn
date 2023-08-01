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

#ifndef SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_
#define SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_

#include <string>

#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::wgsl::writer {

/// Class used for IR to Program tests
class IRToProgramTest : public ir::IRTestHelper {
  public:
    /// The result of Run()
    struct Result {
        /// The resulting WGSL
        std::string wgsl;
        /// The resulting AST
        std::string ast;
        /// The resulting IR
        std::string ir;
        /// The resulting error
        std::string err;
    };
    /// @returns the WGSL generated from the IR
    Result Run();
};

#define EXPECT_WGSL(expected_wgsl)                                       \
    do {                                                                 \
        if (auto got = Run(); got.err.empty()) {                         \
            auto expected = std::string(tint::TrimSpace(expected_wgsl)); \
            if (!expected.empty()) {                                     \
                expected = "\n" + expected + "\n";                       \
            }                                                            \
            EXPECT_EQ(expected, got.wgsl) << "IR: " << got.ir;           \
        } else {                                                         \
            FAIL() << got.err << std::endl                               \
                   << "IR: " << got.ir << std::endl                      \
                   << "AST: " << got.ast << std::endl;                   \
        }                                                                \
    } while (false)

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_IR_TO_PROGRAM_IR_TO_PROGRAM_TEST_H_
