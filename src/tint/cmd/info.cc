
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

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "src/tint/ast/module.h"
#include "src/tint/cmd/helper.h"
#include "src/tint/type/struct.h"
#include "src/tint/utils/io/command.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/transform.h"
#include "src/tint/val/val.h"
#include "tint/tint.h"

namespace {

struct Options {
    bool show_help = false;

#if TINT_BUILD_SPV_READER
    tint::reader::spirv::Options spirv_reader_options;
#endif

    std::string input_filename;
};

const char kUsage[] = R"(Usage: tint [options] <input-file>

 options:
   -h                        -- This help text

)";

bool ParseArgs(const std::vector<std::string>& args, Options* opts) {
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (!arg.empty()) {
            if (arg[0] == '-') {
                std::cerr << "Unrecognized option: " << arg << std::endl;
                return false;
            }
            if (!opts->input_filename.empty()) {
                std::cerr << "More than one input file specified: '" << opts->input_filename
                          << "' and '" << arg << "'" << std::endl;
                return false;
            }
            opts->input_filename = arg;
        }
    }
    return true;
}

}  // namespace

int main(int argc, const char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    Options options;

    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

    if (!ParseArgs(args, &options)) {
        std::cerr << "Failed to parse arguments." << std::endl;
        return 1;
    }

    if (options.show_help) {
        std::cout << kUsage << std::endl;
        return 0;
    }

    auto diag_printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter diag_formatter;

    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    {
        tint::cmd::LoadProgramOptions opts;
        opts.filename = options.input_filename;
#if TINT_BUILD_SPV_READER
        opts.spirv_reader_options = options.spirv_reader_options;
#endif

        auto info = tint::cmd::LoadProgramInfo(opts);
        program = std::move(info.program);
        source_file = std::move(info.source_file);
    }

    tint::inspector::Inspector inspector(program.get());

    if (!inspector.GetUsedExtensionNames().empty()) {
        std::cout << "Extensions:" << std::endl;
        for (const auto& name : inspector.GetUsedExtensionNames()) {
            std::cout << "\t" << name << std::endl;
        }
    }
    std::cout << std::endl;

    tint::cmd::PrintInspectorData(inspector);

    bool has_struct = false;
    for (const auto* ty : program->Types()) {
        if (!ty->Is<tint::type::Struct>()) {
            continue;
        }
        has_struct = true;
        break;
    }

    if (has_struct) {
        std::cout << "Structures" << std::endl;
        for (const auto* ty : program->Types()) {
            if (!ty->Is<tint::type::Struct>()) {
                continue;
            }
            const auto* s = ty->As<tint::type::Struct>();
            std::cout << s->Layout(program->Symbols()) << std::endl << std::endl;
        }
    }

    return 0;
}
