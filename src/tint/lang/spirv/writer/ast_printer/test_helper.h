// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_TEST_HELPER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_TEST_HELPER_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/spirv/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/spirv/writer/binary_writer.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

namespace tint::spirv::writer {

/// Helper class for testing
template <typename BASE>
class TestHelperBase : public ProgramBuilder, public BASE {
  public:
    /// Builder is an alias to the spirv::writer::Builder (as opposed to the ast::Builder)
    using Builder = writer::Builder;

    TestHelperBase() = default;
    ~TestHelperBase() override = default;

    /// @returns the default generator options for SanitizeAndBuild(), if no explicit options are
    /// provided.
    static Options DefaultOptions() {
        Options opts;
        opts.disable_robustness = true;
        return opts;
    }

    /// Builds and returns a Builder from the program.
    /// @note The Builder is only built once. Multiple calls to Build()
    /// will return the same Builder without rebuilding.
    /// @return the built Builder
    Builder& Build() {
        if (spirv_builder) {
            return *spirv_builder;
        }
        [&] {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n" << Diagnostics().str();
        }();
        program = std::make_unique<Program>(resolver::Resolve(*this));
        [&] { ASSERT_TRUE(program->IsValid()) << program->Diagnostics().str(); }();
        spirv_builder = std::make_unique<Builder>(program.get());
        return *spirv_builder;
    }

    /// Builds the program, runs the program through the transform::Spirv
    /// sanitizer and returns a Builder from the sanitized program.
    /// @param options The SPIR-V generator options.
    /// @note The Builder is only built once. Multiple calls to Build()
    /// will return the same Builder without rebuilding.
    /// @return the built Builder
    Builder& SanitizeAndBuild(const Options& options = DefaultOptions()) {
        if (spirv_builder) {
            return *spirv_builder;
        }
        [&] {
            ASSERT_TRUE(IsValid()) << "Builder program is not valid\n" << Diagnostics().str();
        }();
        program = std::make_unique<Program>(resolver::Resolve(*this));
        [&] { ASSERT_TRUE(program->IsValid()) << program->Diagnostics().str(); }();
        auto result = Sanitize(program.get(), options);
        [&] { ASSERT_TRUE(result.program.IsValid()) << result.program.Diagnostics().str(); }();
        *program = std::move(result.program);
        spirv_builder = std::make_unique<Builder>(program.get());
        return *spirv_builder;
    }

    /// Validate passes the generated SPIR-V of the builder `b` to the SPIR-V
    /// Tools Validator. If the validator finds problems the test will fail.
    /// @param b the Builder containing the built SPIR-V module
    void Validate(Builder& b) {
        BinaryWriter writer;
        writer.WriteHeader(b.Module().IdBound());
        writer.WriteModule(&b.Module());
        auto binary = writer.Result();

        std::string spv_errors;
        auto msg_consumer = [&spv_errors](spv_message_level_t level, const char*,
                                          const spv_position_t& position, const char* message) {
            switch (level) {
                case SPV_MSG_FATAL:
                case SPV_MSG_INTERNAL_ERROR:
                case SPV_MSG_ERROR:
                    spv_errors +=
                        "error: line " + std::to_string(position.index) + ": " + message + "\n";
                    break;
                case SPV_MSG_WARNING:
                    spv_errors +=
                        "warning: line " + std::to_string(position.index) + ": " + message + "\n";
                    break;
                case SPV_MSG_INFO:
                    spv_errors +=
                        "info: line " + std::to_string(position.index) + ": " + message + "\n";
                    break;
                case SPV_MSG_DEBUG:
                    break;
            }
        };

        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_2);
        tools.SetMessageConsumer(msg_consumer);
        ASSERT_TRUE(tools.Validate(binary)) << spv_errors;
    }

    /// The program built with a call to Build()
    std::unique_ptr<Program> program;

  private:
    std::unique_ptr<Builder> spirv_builder;
};
using TestHelper = TestHelperBase<testing::Test>;

template <typename T>
using TestParamHelper = TestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_TEST_HELPER_H_
