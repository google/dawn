// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_
#define SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_

#include <string>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/lang/glsl/writer/printer/printer.h"
#include "src/tint/lang/glsl/writer/raise/raise.h"

namespace tint::glsl::writer {

/// Base helper class for testing the GLSL generator implementation.
template <typename BASE>
class GlslPrinterTestHelperBase : public BASE {
  public:
    /// The test module.
    core::ir::Module mod;
    /// The test builder.
    core::ir::Builder b{mod};
    /// The type manager.
    core::type::Manager& ty{mod.Types()};
    /// The GLSL version
    Version version{};

  protected:
    /// Validation errors
    std::string err_;

    /// Generated GLSL
    std::string output_;

    /// Run the writer on the IR module and validate the result.
    /// @returns true if generation and validation succeeded
    bool Generate() {
        if (auto raised = raise::Raise(mod); !raised) {
            err_ = raised.Failure().reason.str();
            return false;
        }

        auto result = Print(mod, version);
        if (!result) {
            err_ = result.Failure().reason.str();
            return false;
        }
        output_ = result.Get();

        return true;
    }

    /// @returns the metal header string
    std::string GlslHeader() const {
        std::stringstream ver;
        ver << "#version " << version.major_version << version.minor_version << "0";
        if (version.IsES()) {
            ver << " es";
        }
        ver << "\n";
        return ver.str();
    }
};

using GlslPrinterTest = GlslPrinterTestHelperBase<testing::Test>;

template <typename T>
using GlslPrinterTestWithParam = GlslPrinterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_PRINTER_HELPER_TEST_H_
