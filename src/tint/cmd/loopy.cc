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

#include <iostream>

#include "src/tint/cmd/generate_external_texture_bindings.h"
#include "src/tint/cmd/helper.h"
#include "tint/tint.h"

#if TINT_BUILD_IR
#include "src/tint/ir/from_program.h"
#include "src/tint/ir/module.h"
#endif  // TINT_BUILD_IR

namespace {

enum class Format {
    kUnknown,
    kNone,
    kSpirv,
    kWgsl,
    kMsl,
    kHlsl,
    kGlsl,
};

enum class Looper {
    kLoad,
    kIRGenerate,
    kWriter,
};

struct Options {
    bool show_help = false;

    std::string input_filename;
    Format format = Format::kUnknown;

    Looper loop = Looper::kLoad;
    uint32_t loop_count = 100;
};

const char kUsage[] = R"(Usage: tint-loopy [options] <input-file>

 options:
  --format <spirv|wgsl|msl|hlsl|none>  -- Generation format. Default SPIR-V.
  --loop <load,ir-gen,writer>          -- Item to loop
  --loop-count <num>                   -- Number of loops to run, default 100.
)";

Format parse_format(const std::string& fmt) {
    (void)fmt;

#if TINT_BUILD_SPV_WRITER
    if (fmt == "spirv") {
        return Format::kSpirv;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (fmt == "wgsl") {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (fmt == "msl") {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (fmt == "hlsl") {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    if (fmt == "glsl") {
        return Format::kGlsl;
    }
#endif  // TINT_BUILD_GLSL_WRITER

    if (fmt == "none") {
        return Format::kNone;
    }

    return Format::kUnknown;
}

bool ParseArgs(const std::vector<std::string>& args, Options* opts) {
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "--format") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for --format argument." << std::endl;
                return false;
            }
            opts->format = parse_format(args[i]);

            if (opts->format == Format::kUnknown) {
                std::cerr << "Unknown output format: " << args[i] << std::endl;
                return false;
            }
        } else if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (arg == "--loop") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for --loop argument." << std::endl;
                return false;
            }
            if (args[i] == "load") {
                opts->loop = Looper::kLoad;
            } else if (args[i] == "ir-gen") {
                opts->loop = Looper::kIRGenerate;
            } else if (args[i] == "writer") {
                opts->loop = Looper::kWriter;
            } else {
                std::cerr << "Invalid loop value" << std::endl;
                return false;
            }
        } else if (arg == "--loop-count") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for --loop-count argument." << std::endl;
                return false;
            }
            int32_t val = atoi(args[i].c_str());
            if (val <= 0) {
                std::cerr << "Loop count must be greater then 0" << std::endl;
                return false;
            }
            opts->loop_count = static_cast<uint32_t>(val);
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

/// Generate SPIR-V code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateSpirv(const tint::Program* program) {
#if TINT_BUILD_SPV_WRITER
    tint::writer::spirv::Options gen_options;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
    auto result = tint::writer::spirv::Generate(program, gen_options);
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }
    return true;
#else
    (void)program;
    std::cerr << "SPIR-V writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_SPV_WRITER
}

/// Generate WGSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateWgsl(const tint::Program* program) {
#if TINT_BUILD_WGSL_WRITER
    tint::writer::wgsl::Options gen_options;
    auto result = tint::writer::wgsl::Generate(program, gen_options);
    if (!result.success) {
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "WGSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_WGSL_WRITER
}

/// Generate MSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateMsl(const tint::Program* program) {
#if TINT_BUILD_MSL_WRITER
    // Remap resource numbers to a flat namespace.
    // TODO(crbug.com/tint/1501): Do this via Options::BindingMap.
    const tint::Program* input_program = program;
    auto flattened = tint::writer::FlattenBindings(program);
    if (flattened) {
        input_program = &*flattened;
    }

    tint::writer::msl::Options gen_options;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(input_program);
    gen_options.array_length_from_uniform.ubo_binding = tint::writer::BindingPoint{0, 30};
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(
        tint::writer::BindingPoint{0, 0}, 0);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(
        tint::writer::BindingPoint{0, 1}, 1);
    auto result = tint::writer::msl::Generate(input_program, gen_options);
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "MSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_MSL_WRITER
}

/// Generate HLSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateHlsl(const tint::Program* program) {
#if TINT_BUILD_HLSL_WRITER
    tint::writer::hlsl::Options gen_options;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
    auto result = tint::writer::hlsl::Generate(program, gen_options);
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    return true;
#else
    (void)program;
    std::cerr << "HLSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_HLSL_WRITER
}

/// Generate GLSL code for a program.
/// @param program the program to generate
/// @returns true on success
bool GenerateGlsl(const tint::Program* program) {
#if TINT_BUILD_GLSL_WRITER
    tint::writer::glsl::Options gen_options;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
    auto result = tint::writer::glsl::Generate(program, gen_options, "");
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    return true;

#else
    (void)program;
    std::cerr << "GLSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_GLSL_WRITER
}

}  // namespace

int main(int argc, const char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    Options options;

    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

#if TINT_BUILD_WGSL_WRITER
    tint::Program::printer = [](const tint::Program* program) {
        auto result = tint::writer::wgsl::Generate(program, {});
        if (!result.error.empty()) {
            return "error: " + result.error;
        }
        return result.wgsl;
    };
#endif  // TINT_BUILD_WGSL_WRITER

    if (!ParseArgs(args, &options)) {
        std::cerr << "Failed to parse arguments." << std::endl;
        return 1;
    }

    if (options.show_help) {
        std::cout << kUsage << std::endl;
        return 0;
    }

    // Implement output format defaults.
    if (options.format == Format::kUnknown) {
        options.format = Format::kSpirv;
    }

    auto diag_printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter diag_formatter;

    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    if (options.loop == Looper::kLoad) {
        if (options.input_filename.size() > 5 &&
            options.input_filename.substr(options.input_filename.size() - 5) == ".wgsl") {
#if TINT_BUILD_WGSL_READER
            std::vector<uint8_t> data;
            if (!tint::cmd::ReadFile<uint8_t>(options.input_filename, &data)) {
                exit(1);
            }
            source_file = std::make_unique<tint::Source::File>(
                options.input_filename, std::string(data.begin(), data.end()));

            uint32_t loop_count = options.loop_count;
            for (uint32_t i = 0; i < loop_count; ++i) {
                program =
                    std::make_unique<tint::Program>(tint::reader::wgsl::Parse(source_file.get()));
            }
#else
            std::cerr << "Tint not built with the WGSL reader enabled" << std::endl;
            exit(1);
#endif  // TINT_BUILD_WGSL_READER
        } else {
#if TINT_BUILD_SPV_READER
            std::vector<uint32_t> data;
            if (!tint::cmd::ReadFile<uint32_t>(options.input_filename, &data)) {
                exit(1);
            }

            uint32_t loop_count = options.loop_count;
            for (uint32_t i = 0; i < loop_count; ++i) {
                program = std::make_unique<tint::Program>(tint::reader::spirv::Parse(data, {}));
            }
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            exit(1);
#endif  // TINT_BUILD_SPV_READER
        }
    }

    // Load the program that will actually be used
    {
        tint::cmd::LoadProgramOptions opts;
        opts.filename = options.input_filename;

        auto info = tint::cmd::LoadProgramInfo(opts);
        program = std::move(info.program);
        source_file = std::move(info.source_file);
    }
#if TINT_BUILD_IR
    {
        uint32_t loop_count = 1;
        if (options.loop == Looper::kIRGenerate) {
            loop_count = options.loop_count;
        }
        for (uint32_t i = 0; i < loop_count; ++i) {
            auto result = tint::ir::FromProgram(program.get());
            if (!result) {
                std::cerr << "Failed to build IR from program: " << result.Failure() << std::endl;
            }
        }
    }
#endif  // TINT_BUILD_IR

    bool success = false;
    {
        uint32_t loop_count = 1;
        if (options.loop == Looper::kWriter) {
            loop_count = options.loop_count;
        }

        switch (options.format) {
            case Format::kSpirv:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateSpirv(program.get());
                }
                break;
            case Format::kWgsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateWgsl(program.get());
                }
                break;
            case Format::kMsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateMsl(program.get());
                }
                break;
            case Format::kHlsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateHlsl(program.get());
                }
                break;
            case Format::kGlsl:
                for (uint32_t i = 0; i < loop_count; ++i) {
                    success = GenerateGlsl(program.get());
                }
                break;
            case Format::kNone:
                break;
            default:
                std::cerr << "Unknown output format specified" << std::endl;
                return 1;
        }
    }
    if (!success) {
        return 1;
    }

    return 0;
}
