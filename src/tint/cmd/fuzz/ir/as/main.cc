// Copyright 2024 The Dawn & Tint Authors
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

#include <iostream>
#include <memory>
#include <string>
#include "src/tint/lang/wgsl/sem/variable.h"

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/binary/encode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/utils/cli/cli.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/color_mode.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

namespace {

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_file = "-";  // Default to stdout

    bool dump_ir = false;
    bool dump_ir_bin = false;
};

bool ParseArgs(tint::VectorRef<std::string_view> arguments, Options* opts) {
    using namespace tint::cli;  // NOLINT(build/namespaces)

    OptionSet options;
    auto& col = options.Add<EnumOption<tint::ColorMode>>(
        "color", "Use colored output",
        tint::Vector{
            EnumName{tint::ColorMode::kPlain, "off"},
            EnumName{tint::ColorMode::kDark, "dark"},
            EnumName{tint::ColorMode::kLight, "light"},
        },
        ShortName{"col"}, Default{tint::ColorModeDefault()});
    TINT_DEFER(opts->printer = CreatePrinter(*col.value));

    auto& output = options.Add<StringOption>("output-name", "Output file name", ShortName{"o"},
                                             Parameter{"name"});
    TINT_DEFER(opts->output_file = output.value.value_or(""));

    auto& dump_ir = options.Add<BoolOption>("dump-ir", "Writes the IR to stdout", Alias{"emit-ir"},
                                            Default{false});
    TINT_DEFER(opts->dump_ir = *dump_ir.value);

    auto& dump_ir_bin = options.Add<BoolOption>(
        "dump-ir-bin", "Writes the IR as a human readable protobuf to stdout", Alias{"emit-ir-bin"},
        Default{false});
    TINT_DEFER(opts->dump_ir_bin = *dump_ir_bin.value);

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout << R"(Usage: tint [options] <input-file>

Options:
)";
        options.ShowHelp(std::cout);
    };

    auto result = options.Parse(arguments);
    if (result != tint::Success) {
        std::cerr << result.Failure() << "\n";
        show_usage();
        return false;
    }
    if (help.value.value_or(false)) {
        show_usage();
        return false;
    }

    auto files = result.Get();
    if (files.Length() > 1) {
        std::cerr << "More than one input file specified: "
                  << tint::Join(Transform(files, tint::Quote), ", ") << "\n";
        return false;
    }
    if (files.Length() == 1) {
        opts->input_filename = files[0];
    }

    return true;
}

/// Dump IR for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool DumpIR(const tint::Program& program, const Options& options) {
    auto result = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (result != tint::Success) {
        std::cerr << "Failed to build IR from program: " << result.Failure() << "\n";
        return false;
    }

    options.printer->Print(tint::core::ir::Disassembler(result.Get()).Text());
    options.printer->Print(tint::StyledText{} << "\n");

    return true;
}

/// Generate an IR module for a program, performs checking for unsupported
/// enables, and validation.
/// @param program the program to generate
/// @returns generated module on success, tint::failure on failure
tint::Result<tint::core::ir::Module> GenerateIrModule(const tint::Program& program) {
    if (program.AST().Enables().Any(tint::wgsl::reader::IsUnsupportedByIR)) {
        return tint::Failure{"Unsupported enable used in shader"};
    }

    auto transformed = tint::wgsl::ApplySubstituteOverrides(program);
    auto& src = transformed ? transformed.value() : program;
    if (!src.IsValid()) {
        return tint::Failure{src.Diagnostics()};
    }

    auto ir = tint::wgsl::reader::ProgramToLoweredIR(src);
    if (ir != tint::Success) {
        return ir.Failure();
    }

    if (auto val = tint::core::ir::Validate(ir.Get()); val != tint::Success) {
        return val.Failure();
    }

    return ir;
}

/// Generate IR binary protobuf for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateIrProtoBinary(const tint::Program& program, const Options& options) {
    auto module = GenerateIrModule(program);
    if (module != tint::Success) {
        std::cerr << "Failed to generate lowered IR from program: " << module.Failure() << "\n";
        return false;
    }

    auto pb = tint::core::ir::binary::Encode(module.Get());
    if (pb != tint::Success) {
        std::cerr << "Failed to encode IR module to protobuf: " << pb.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "wb", ToStdVector(pb.Get()))) {
        std::cerr << "Failed to write protobuf binary out to file\n";
        return false;
    }

    return true;
}

/// Generate IR human readable protobuf for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateIrProtoDebug(const tint::Program& program, const Options& options) {
    auto module = GenerateIrModule(program);
    if (module != tint::Success) {
        std::cerr << "Failed to generate lowered IR from program: " << module.Failure() << "\n";
        return false;
    }

    auto pb = tint::core::ir::binary::EncodeDebug(module.Get());
    if (pb != tint::Success) {
        std::cerr << "Failed to encode IR module to protobuf: " << pb.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", pb.Get())) {
        std::cerr << "Failed to write protobuf debug text out to file\n";
        return false;
    }

    return true;
}

}  // namespace

int main(int argc, const char** argv) {
    tint::Vector<std::string_view, 8> arguments;
    for (int i = 1; i < argc; i++) {
        std::string_view arg(argv[i]);
        if (!arg.empty()) {
            arguments.Push(argv[i]);
        }
    }

    Options options;

    tint::Initialize();
    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

    if (!ParseArgs(arguments, &options)) {
        return EXIT_FAILURE;
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
    opts.printer = options.printer.get();

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.dump_ir) {
        DumpIR(info.program, options);
    }

    if (options.dump_ir_bin) {
        GenerateIrProtoDebug(info.program, options);
    }

    if (!GenerateIrProtoBinary(info.program, options)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
