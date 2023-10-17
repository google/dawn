// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_HELPER_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/spirv/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/spirv/writer/common/binary_writer.h"
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
        if (!IsValid()) {
            ADD_FAILURE() << "ProgramBuilder is not valid: " << Diagnostics();
        }
        program = std::make_unique<Program>(resolver::Resolve(*this));
        if (!program->IsValid()) {
            ADD_FAILURE() << program->Diagnostics();
        }
        spirv_builder = std::make_unique<Builder>(*program);
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
        if (!IsValid()) {
            ADD_FAILURE() << "ProgramBuilder is not valid: " << Diagnostics();
        }
        program = std::make_unique<Program>(resolver::Resolve(*this));
        if (!program->IsValid()) {
            ADD_FAILURE() << program->Diagnostics();
        }
        auto result = Sanitize(*program, options);
        if (!result.program.IsValid()) {
            ADD_FAILURE() << result.program.Diagnostics();
        }
        *program = std::move(result.program);
        bool zero_initialize_workgroup_memory =
            !options.disable_workgroup_init &&
            options.use_zero_initialize_workgroup_memory_extension;
        spirv_builder =
            std::make_unique<Builder>(*program, zero_initialize_workgroup_memory,
                                      options.experimental_require_subgroup_uniform_control_flow);
        return *spirv_builder;
    }

    /// Validate passes the generated SPIR-V of the builder `b` to the SPIR-V
    /// Tools Validator. If the validator finds problems the test will fail.
    /// @param b the Builder containing the built SPIR-V module
    void Validate(Builder& b) {
        BinaryWriter writer;
        writer.WriteHeader(b.Module().IdBound());
        writer.WriteModule(b.Module());
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

#endif  // SRC_TINT_LANG_SPIRV_WRITER_AST_PRINTER_HELPER_TEST_H_
