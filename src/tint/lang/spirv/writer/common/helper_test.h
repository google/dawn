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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_

#include <string>
#include <utility>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spirv-tools/libspirv.hpp"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/spirv/writer/common/spv_dump.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"

namespace tint::spirv::writer {

using namespace tint::number_suffixes;  // NOLINT

// Helper macro to check whether the SPIR-V output contains an instruction, dumping the full output
// if the instruction was not present.
#define EXPECT_INST(inst) ASSERT_THAT(output_, testing::HasSubstr(inst)) << output_

/// The element type of a test.
enum TestElementType {
    kBool,
    kI32,
    kU32,
    kF32,
    kF16,
};
template <typename STREAM, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, TestElementType type) {
    switch (type) {
        case kBool:
            out << "bool";
            break;
        case kI32:
            out << "i32";
            break;
        case kU32:
            out << "u32";
            break;
        case kF32:
            out << "f32";
            break;
        case kF16:
            out << "f16";
            break;
    }
    return out;
}

/// Base helper class for testing the SPIR-V writer implementation.
template <typename BASE>
class SpirvWriterTestHelperBase : public BASE {
  public:
    SpirvWriterTestHelperBase() : writer_(&mod, false) {}

    /// The test module.
    ir::Module mod;
    /// The test builder.
    ir::Builder b{mod};
    /// The type manager.
    type::Manager& ty{mod.Types()};

  protected:
    /// The SPIR-V writer.
    Printer writer_;

    /// Errors produced during codegen or SPIR-V validation.
    std::string err_;

    /// SPIR-V output.
    std::string output_;

    /// @returns the error string from the validation
    std::string Error() const { return err_; }

    /// Run the specified writer on the IR module and validate the result.
    /// @param writer the writer to use for SPIR-V generation
    /// @returns true if generation and validation succeeded
    bool Generate(Printer& writer) {
        auto raised = raise::Raise(&mod);
        if (!raised) {
            err_ = raised.Failure();
            return false;
        }

        auto spirv = writer.Generate();
        if (!spirv) {
            err_ = spirv.Failure();
            return false;
        }

        output_ = Disassemble(spirv.Get(), SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES |
                                               SPV_BINARY_TO_TEXT_OPTION_INDENT |
                                               SPV_BINARY_TO_TEXT_OPTION_COMMENT);

        if (!Validate(spirv.Get())) {
            return false;
        }

        return true;
    }

    /// Run the writer on the IR module and validate the result.
    /// @returns true if generation and validation succeeded
    bool Generate() { return Generate(writer_); }

    /// Validate the generated SPIR-V using the SPIR-V Tools Validator.
    /// @param binary the SPIR-V binary module to validate
    /// @returns true if validation succeeded, false otherwise
    bool Validate(const std::vector<uint32_t>& binary) {
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

        auto result = tools.Validate(binary);
        err_ = std::move(spv_errors);
        return result;
    }

    /// @returns the disassembled types from the generated module.
    std::string DumpTypes() { return DumpInstructions(writer_.Module().Types()); }

    /// Helper to make a scalar type corresponding to the element type `type`.
    /// @param type the element type
    /// @returns the scalar type
    const type::Type* MakeScalarType(TestElementType type) {
        switch (type) {
            case kBool:
                return ty.bool_();
            case kI32:
                return ty.i32();
            case kU32:
                return ty.u32();
            case kF32:
                return ty.f32();
            case kF16:
                return ty.f16();
        }
        return nullptr;
    }

    /// Helper to make a vector type corresponding to the element type `type`.
    /// @param type the element type
    /// @returns the vector type
    const type::Type* MakeVectorType(TestElementType type) { return ty.vec2(MakeScalarType(type)); }

    /// Helper to make a scalar value with the scalar type `type`.
    /// @param type the element type
    /// @param value the optional value to use
    /// @returns the scalar value
    ir::Constant* MakeScalarValue(TestElementType type, uint32_t value = 1) {
        switch (type) {
            case kBool:
                return b.Constant(true);
            case kI32:
                return b.Constant(i32(value));
            case kU32:
                return b.Constant(u32(value));
            case kF32:
                return b.Constant(f32(value));
            case kF16:
                return b.Constant(f16(value));
        }
        return nullptr;
    }

    /// Helper to make a vector value with an element type of `type`.
    /// @param type the element type
    /// @returns the vector value
    ir::Constant* MakeVectorValue(TestElementType type) {
        switch (type) {
            case kBool:
                return b.Composite(MakeVectorType(type), true, false);
            case kI32:
                return b.Composite(MakeVectorType(type), 42_i, -10_i);
            case kU32:
                return b.Composite(MakeVectorType(type), 42_u, 10_u);
            case kF32:
                return b.Composite(MakeVectorType(type), 42_f, -0.5_f);
            case kF16:
                return b.Composite(MakeVectorType(type), 42_h, -0.5_h);
        }
        return nullptr;
    }
};

using SpirvWriterTest = SpirvWriterTestHelperBase<testing::Test>;

template <typename T>
using SpirvWriterTestWithParam = SpirvWriterTestHelperBase<testing::TestWithParam<T>>;

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_HELPER_TEST_H_
