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

#ifndef SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_
#define SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/glsl/writer/printer/printer.h"
#include "src/tint/lang/glsl/writer/raise/raise.h"

namespace tint::glsl::writer {

/// Base helper class for testing the GLSL generator implementation.
template <typename BASE>
class GlslPrinterTestHelperBase : public BASE {
  public:
    GlslPrinterTestHelperBase() : writer_(mod) {}

    /// The test module.
    core::ir::Module mod;
    /// The test builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};

  protected:
    /// The GLSL writer.
    Printer writer_;

    /// Validation errors
    std::string err_;

    /// Generated GLSL
    std::string output_;

    /// Run the writer on the IR module and validate the result.
    /// @returns true if generation and validation succeeded
    bool Generate() {
        auto raised = raise::Raise(mod);
        if (!raised) {
            err_ = raised.Failure().reason.str();
            return false;
        }

        auto result = writer_.Generate();
        if (!result) {
            err_ = result.Failure().reason.str();
            return false;
        }
        output_ = writer_.Result();

        return true;
    }
};

using GlslPrinterTest = GlslPrinterTestHelperBase<testing::Test>;

template <typename T>
using GlslPrinterTestWithParam = GlslPrinterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_
