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

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/binary/decode.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/cli/cli.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/color_mode.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

TINT_BEGIN_DISABLE_PROTOBUF_WARNINGS();
#include "src/tint/utils/protos/ir_fuzz/ir_fuzz.pb.h"
TINT_END_DISABLE_PROTOBUF_WARNINGS();

namespace {

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_filename;

    bool dump_wgsl = false;
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

    auto& output = options.Add<StringOption>(
        "output-filename", "Output file name, if not specified, IR text output will go to STDOUT",
        ShortName{"o"}, Parameter{"name"});
    TINT_DEFER(opts->output_filename = output.value.value_or(""));

    auto& dump_wgsl = options.Add<BoolOption>(
        "dump-wgsl", "Writes the WGSL form of input to stdout, may fail due to validation errors",
        Alias{"emit-wgsl"}, Default{false});
    TINT_DEFER(opts->dump_wgsl = *dump_wgsl.value);

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout << R"(Usage: ir_fuzz_dis [options] <input-file>

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

    auto args = result.Get();
    if (args.Length() > 1) {
        std::cerr << "More than one input arg specified: "
                  << tint::Join(Transform(args, tint::Quote), ", ") << "\n";
        return false;
    }
    if (args.Length() == 1) {
        opts->input_filename = args[0];
    }

    return true;
}

/// @returns a fuzzer test case protobuf for the given file
/// @param options program options that contains the filename to be read, etc.
tint::Result<tint::cmd::fuzz::ir::pb::Root> GenerateFuzzCaseProto(const Options& options) {
    tint::cmd::fuzz::ir::pb::Root fuzz_pb;

    std::fstream input(options.input_filename, std::ios::in | std::ios::binary);
    if (!fuzz_pb.ParseFromIstream(&input)) {
        return tint::Failure{"Unable to parse bytes into test case protobuf"};
    }

    return std::move(fuzz_pb);
}

bool ProcessFile(const Options& options) {
    auto fuzz_pb = GenerateFuzzCaseProto(options);
    if (fuzz_pb != tint::Success) {
        std::cerr << "Failed to read test case protobuf: " << fuzz_pb.Failure() << "\n";
        return false;
    }

    auto module = tint::core::ir::binary::Decode(fuzz_pb.Get().module());
    if (module != tint::Success) {
        std::cerr << "Unable to decode ir protobuf from test case protobuf: " << module.Failure()
                  << "\n";
        return false;
    }

    const auto ir_text = tint::core::ir::Disassembler(module.Get()).Text();
    if (options.output_filename.empty()) {
        options.printer->Print(ir_text);
        options.printer->Print(tint::StyledText{} << "\n");
    } else {
        if (!tint::cmd::WriteFile(options.output_filename, "w", ir_text.Plain())) {
            std::cerr << "Unable to print IR text to file, " << options.output_filename << "\n";
            return false;
        }
    }

    if (options.dump_wgsl) {
        tint::wgsl::writer::ProgramOptions writer_options;
        auto output = tint::wgsl::writer::WgslFromIR(module.Get(), writer_options);
        if (output != tint::Success) {
            std::cerr << "Failed to convert IR to Program: " << output.Failure() << "\n";
            return false;
        }

        if (options.output_filename.empty()) {
            options.printer->Print(tint::StyledText{} << output->wgsl);
            options.printer->Print(tint::StyledText{} << "\n");
        } else {
            if (!tint::cmd::WriteFile(options.output_filename, "w", output->wgsl)) {
                std::cerr << "Unable to print WGSL to file, " << options.output_filename << "\n";
                return false;
            }
        }
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

    if (!ProcessFile(options)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
