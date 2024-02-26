// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/fuzzers/tint_common_fuzzer.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#if TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"
#include "src/tint/utils/text/text_style.h"

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/helpers/ast_generate_bindings.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#endif  // TINT_BUILD_MSL_WRITER

namespace tint::fuzzers {

namespace {

// A macro is used to avoid FATAL_ERROR creating its own stack frame. This leads
// to better de-duplication of bug reports, because ClusterFuzz only uses the
// top few stack frames for de-duplication, and a FATAL_ERROR stack frame
// provides no useful information.
#define FATAL_ERROR(diags, msg_string)                          \
    do {                                                        \
        StyledText msg;                                         \
        msg << (style::Error + style::Bold) << msg_string;      \
        auto printer = tint::StyledTextPrinter::Create(stderr); \
        printer->Print(msg);                                    \
        printer->Print(tint::diag::Formatter().Format(diags));  \
        __builtin_trap();                                       \
    } while (false)

[[noreturn]] void TintInternalCompilerErrorReporter(const InternalCompilerError& err) {
    std::cerr << err.Error() << std::endl;
    __builtin_trap();
}

// Wrapping in a macro, so it can be a one-liner in the code, but not
// introduce another level in the stack trace. This will help with de-duping
// ClusterFuzz issues.
#define CHECK_INSPECTOR(program, inspector)                                                 \
    do {                                                                                    \
        if ((inspector).has_error()) {                                                      \
            if (!enforce_validity) {                                                        \
                return;                                                                     \
            }                                                                               \
            FATAL_ERROR(program.Diagnostics(), "Inspector failed: " + (inspector).error()); \
        }                                                                                   \
    } while (false)

// Wrapping in a macro to make code more readable and help with issue de-duping.
#define VALIDITY_ERROR(diags, msg_string) \
    do {                                  \
        if (!enforce_validity) {          \
            return 0;                     \
        }                                 \
        FATAL_ERROR(diags, msg_string);   \
    } while (false)

bool SPIRVToolsValidationCheck(const tint::Program& program, const std::vector<uint32_t>& spirv) {
    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
    const tint::diag::List& diags = program.Diagnostics();
    tools.SetMessageConsumer(
        [diags](spv_message_level_t, const char*, const spv_position_t& pos, const char* msg) {
            StyledText out;
            out << "Unexpected spirv-val error:\n"
                << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg;

            auto printer = tint::StyledTextPrinter::Create(stderr);
            printer->Print(out);
            printer->Print(tint::diag::Formatter().Format(diags));
        });

    return tools.Validate(spirv.data(), spirv.size(), spvtools::ValidatorOptions());
}

}  // namespace

void GenerateSpirvOptions(DataBuilder* b, spirv::writer::Options* options) {
    *options = b->build<spirv::writer::Options>();
}

void GenerateWgslOptions(DataBuilder* b, wgsl::writer::Options* options) {
    *options = b->build<wgsl::writer::Options>();
}

void GenerateHlslOptions(DataBuilder* b, hlsl::writer::Options* options) {
    *options = b->build<hlsl::writer::Options>();
}

void GenerateMslOptions(DataBuilder* b, msl::writer::Options* options) {
    *options = b->build<msl::writer::Options>();
}

CommonFuzzer::CommonFuzzer(InputFormat input, OutputFormat output)
    : input_(input), output_(output) {}

CommonFuzzer::~CommonFuzzer() = default;

int CommonFuzzer::Run(const uint8_t* data, size_t size) {
    tint::Initialize();
    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    Program program;

#if TINT_BUILD_SPV_READER
    std::vector<uint32_t> spirv_input(size / sizeof(uint32_t));

#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER || TINT_BUILD_SPV_READER
    auto dump_input_data = [&](auto& content, const char* extension) {
        size_t hash = Hash(content);
        auto filename = "fuzzer_input_" + std::to_string(hash) + extension;  //
        std::ofstream fout(filename, std::ios::binary);
        fout.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
        std::cout << "Dumped input data to " << filename << std::endl;
    };
#endif

    switch (input_) {
        case InputFormat::kWGSL: {
#if TINT_BUILD_WGSL_READER
            // Clear any existing diagnostics, as these will hold pointers to file_,
            // which we are about to release.
            diagnostics_ = {};
            std::string str(reinterpret_cast<const char*>(data), size);
            file_ = std::make_unique<Source::File>("test.wgsl", str);
            if (dump_input_) {
                dump_input_data(str, ".wgsl");
            }
            program = wgsl::reader::Parse(file_.get());
#endif  // TINT_BUILD_WGSL_READER
            break;
        }

        case InputFormat::kSpv: {
#if TINT_BUILD_SPV_READER
            // `spirv_input` has been initialized with the capacity to store `size /
            // sizeof(uint32_t)` uint32_t values. If `size` is not a multiple of
            // sizeof(uint32_t) then not all of `data` can be copied into
            // `spirv_input`, and any trailing bytes are discarded.
            std::memcpy(spirv_input.data(), data, spirv_input.size() * sizeof(uint32_t));
            if (spirv_input.empty()) {
                return 0;
            }
            if (dump_input_) {
                dump_input_data(spirv_input, ".spv");
            }
            program = spirv::reader::Read(spirv_input);
#endif  // TINT_BUILD_SPV_READER
            break;
        }
    }

    if (!program.IsValid()) {
        diagnostics_ = program.Diagnostics();
        return 0;
    }

    // Helper that returns `true` if the program uses the given extension.
    auto uses_extension = [&program](tint::wgsl::Extension extension) {
        for (auto* enable : program.AST().Enables()) {
            if (enable->HasExtension(extension)) {
                return true;
            }
        }
        return false;
    };

#if TINT_BUILD_SPV_READER
    if (input_ == InputFormat::kSpv && !SPIRVToolsValidationCheck(program, spirv_input)) {
        FATAL_ERROR(program.Diagnostics(),
                    "Fuzzing detected invalid input spirv not being caught by Tint");
    }
#endif  // TINT_BUILD_SPV_READER

    RunInspector(program);
    diagnostics_ = program.Diagnostics();

    auto validate_program = [&](auto& out) {
        if (!out.IsValid()) {
            // Transforms can produce error messages for bad input.
            // Catch ICEs and errors from non transform systems.
            for (const auto& diag : out.Diagnostics()) {
                if (diag.severity > diag::Severity::Error ||
                    diag.system != diag::System::Transform) {
                    VALIDITY_ERROR(program.Diagnostics(),
                                   "Fuzzing detected valid input program being "
                                   "transformed into an invalid output program");
                }
            }
            return 0;
        }

        program = std::move(out);
        RunInspector(program);
        return 1;
    };

    if (transform_manager_) {
        ast::transform::DataMap outputs;
        auto out = transform_manager_->Run(program, *transform_inputs_, outputs);
        if (!validate_program(out)) {  // Will move: program <- out on success
            return 0;
        }
    }

    // Run SubstituteOverride if required
    if (auto transformed = tint::wgsl::ApplySubstituteOverrides(program)) {
        program = std::move(*transformed);
        if (!program.IsValid()) {
            return 0;
        }
    }

    switch (output_) {
        case OutputFormat::kMSL:
#if TINT_BUILD_MSL_WRITER
            options_msl_.bindings = tint::msl::writer::GenerateBindings(program);
#endif  // TINT_BUILD_MSL_WRITER
            break;
        case OutputFormat::kHLSL:
#if TINT_BUILD_HLSL_WRITER
            options_hlsl_.bindings = tint::hlsl::writer::GenerateBindings(program);
#endif  // TINT_BUILD_HLSL_WRITER
            break;
        case OutputFormat::kSpv:
#if TINT_BUILD_SPV_WRITER
            options_spirv_.bindings = tint::spirv::writer::GenerateBindings(program);
#endif  // TINT_BUILD_SPV_WRITER
            break;
        case OutputFormat::kWGSL:
            break;
    }

    switch (output_) {
        case OutputFormat::kWGSL: {
#if TINT_BUILD_WGSL_WRITER
            (void)wgsl::writer::Generate(program, options_wgsl_);
#endif  // TINT_BUILD_WGSL_WRITER
            break;
        }
        case OutputFormat::kSpv: {
#if TINT_BUILD_SPV_WRITER
            // Skip fuzzing the SPIR-V writer when the `clamp_frag_depth` option is used with a
            // module that already contains push constants.
            if (uses_extension(tint::wgsl::Extension::kChromiumExperimentalPushConstant) &&
                options_spirv_.clamp_frag_depth) {
                return 0;
            }

            auto result = spirv::writer::Generate(program, options_spirv_);
            if (result == Success) {
                generated_spirv_ = std::move(result->spirv);

                if (!SPIRVToolsValidationCheck(program, generated_spirv_)) {
                    VALIDITY_ERROR(program.Diagnostics(),
                                   "Fuzzing detected invalid spirv being emitted by Tint");
                }
            }

#endif  // TINT_BUILD_SPV_WRITER
            break;
        }
        case OutputFormat::kHLSL: {
#if TINT_BUILD_HLSL_WRITER
            (void)hlsl::writer::Generate(program, options_hlsl_);
#endif  // TINT_BUILD_HLSL_WRITER
            break;
        }
        case OutputFormat::kMSL: {
#if TINT_BUILD_MSL_WRITER
            // Remap resource numbers to a flat namespace.
            // TODO(crbug.com/tint/1501): Do this via Options::BindingMap.
            if (auto flattened = tint::wgsl::FlattenBindings(program)) {
                program = std::move(*flattened);
            }

            (void)msl::writer::Generate(program, options_msl_);
#endif  // TINT_BUILD_MSL_WRITER
            break;
        }
    }

    return 0;
}

void CommonFuzzer::RunInspector(Program& program) {
    inspector::Inspector inspector(program);
    diagnostics_ = program.Diagnostics();

    if (!program.IsValid()) {
        // It's not safe to use the inspector on invalid programs.
        return;
    }

    auto entry_points = inspector.GetEntryPoints();
    CHECK_INSPECTOR(program, inspector);

    auto override_ids = inspector.GetOverrideDefaultValues();
    CHECK_INSPECTOR(program, inspector);

    auto override_name_to_id = inspector.GetNamedOverrideIds();
    CHECK_INSPECTOR(program, inspector);

    for (auto& ep : entry_points) {
        inspector.GetResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetUniformBufferResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetStorageBufferResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetReadOnlyStorageBufferResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetSamplerResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetComparisonSamplerResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetSampledTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetMultisampledTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetStorageTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetDepthTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetDepthMultisampledTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetExternalTextureResourceBindings(ep.name);
        CHECK_INSPECTOR(program, inspector);

        inspector.GetSamplerTextureUses(ep.name);
        CHECK_INSPECTOR(program, inspector);
    }
}

}  // namespace tint::fuzzers
