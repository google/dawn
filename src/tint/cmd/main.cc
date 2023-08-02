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

#include <charconv>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#if TINT_BUILD_GLSL_WRITER
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER

#include "src/tint/cmd/generate_external_texture_bindings.h"
#include "src/tint/cmd/helper.h"
#include "src/tint/lang/hlsl/validate/val.h"
#include "src/tint/lang/msl/validate/val.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/utils/cli/cli.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/diagnostic/printer.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"
#include "tint/tint.h"

#if TINT_BUILD_IR
#include "src/tint/lang/core/ir/disassembler.h"                     // nogncheck
#include "src/tint/lang/core/ir/module.h"                           // nogncheck
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"  // nogncheck
#endif                                                              // TINT_BUILD_IR

#if TINT_BUILD_SPV_WRITER
#define SPV_WRITER_ONLY(x) x
#else
#define SPV_WRITER_ONLY(x)
#endif

#if TINT_BUILD_WGSL_WRITER
#define WGSL_WRITER_ONLY(x) x
#else
#define WGSL_WRITER_ONLY(x)
#endif

#if TINT_BUILD_MSL_WRITER
#define MSL_WRITER_ONLY(x) x
#else
#define MSL_WRITER_ONLY(x)
#endif

#if TINT_BUILD_HLSL_WRITER
#define HLSL_WRITER_ONLY(x) x
#else
#define HLSL_WRITER_ONLY(x)
#endif

#if TINT_BUILD_GLSL_WRITER
#define GLSL_WRITER_ONLY(x) x
#else
#define GLSL_WRITER_ONLY(x)
#endif

namespace {

/// Prints the given hash value in a format string that the end-to-end test runner can parse.
void PrintHash(uint32_t hash) {
    std::cout << "<<HASH: 0x" << std::hex << hash << ">>" << std::endl;
}

enum class Format {
    kUnknown,
    kNone,
    kSpirv,
    kSpvAsm,
    kWgsl,
    kMsl,
    kHlsl,
    kGlsl,
};

struct Options {
    bool verbose = false;

    std::string input_filename;
    std::string output_file = "-";  // Default to stdout

    bool parse_only = false;
    bool disable_workgroup_init = false;
    bool validate = false;
    bool print_hash = false;
    bool dump_inspector_bindings = false;
    bool enable_robustness = false;

    std::unordered_set<uint32_t> skip_hash;

    Format format = Format::kUnknown;

    bool emit_single_entry_point = false;
    std::string ep_name;

    bool rename_all = false;

#if TINT_BUILD_SPV_READER
    tint::spirv::reader::Options spirv_reader_options;
#endif

    tint::Vector<std::string, 4> transforms;

    std::string fxc_path;
    std::string dxc_path;
    std::string xcrun_path;
    tint::Hashmap<std::string, double, 8> overrides;
    std::optional<tint::BindingPoint> hlsl_root_constant_binding_point;

#if TINT_BUILD_IR
    bool dump_ir = false;
    bool use_ir = false;
#endif  // TINT_BUILD_IR

#if TINT_BUILD_SYNTAX_TREE_WRITER
    bool dump_ast = false;
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER
};

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format infer_format(const std::string& filename) {
    (void)filename;

#if TINT_BUILD_SPV_WRITER
    if (tint::HasSuffix(filename, ".spv")) {
        return Format::kSpirv;
    }
    if (tint::HasSuffix(filename, ".spvasm")) {
        return Format::kSpvAsm;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (tint::HasSuffix(filename, ".wgsl")) {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (tint::HasSuffix(filename, ".metal")) {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (tint::HasSuffix(filename, ".hlsl")) {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

    return Format::kUnknown;
}

bool ParseArgs(tint::VectorRef<std::string_view> arguments,
               std::string transform_names,
               Options* opts) {
    using namespace tint::cli;  // NOLINT(build/namespaces)

    tint::Vector<EnumName<Format>, 8> format_enum_names{
        EnumName(Format::kNone, "none"),
    };

    SPV_WRITER_ONLY(format_enum_names.Emplace(Format::kSpirv, "spirv"));
    SPV_WRITER_ONLY(format_enum_names.Emplace(Format::kSpvAsm, "spvasm"));
    WGSL_WRITER_ONLY(format_enum_names.Emplace(Format::kWgsl, "wgsl"));
    MSL_WRITER_ONLY(format_enum_names.Emplace(Format::kMsl, "msl"));
    HLSL_WRITER_ONLY(format_enum_names.Emplace(Format::kHlsl, "hlsl"));
    GLSL_WRITER_ONLY(format_enum_names.Emplace(Format::kGlsl, "glsl"));

    OptionSet options;
    auto& fmt = options.Add<EnumOption<Format>>("format",
                                                R"(Output format.
If not provided, will be inferred from output filename extension:
  .spvasm -> spvasm
  .spv    -> spirv
  .wgsl   -> wgsl
  .metal  -> msl
  .hlsl   -> hlsl)",
                                                format_enum_names, ShortName{"f"});
    TINT_DEFER(opts->format = fmt.value.value_or(Format::kUnknown));

    auto& ep = options.Add<StringOption>("entry-point", "Output single entry point",
                                         ShortName{"ep"}, Parameter{"name"});
    TINT_DEFER({
        if (ep.value.has_value()) {
            opts->ep_name = *ep.value;
            opts->emit_single_entry_point = true;
        }
    });

    auto& output = options.Add<StringOption>("output-name", "Output file name", ShortName{"o"},
                                             Parameter{"name"});
    TINT_DEFER(opts->output_file = output.value.value_or(""));

    auto& fxc_path =
        options.Add<StringOption>("fxc", R"(Path to FXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation with FXC)",
                                  Parameter{"path"});
    TINT_DEFER(opts->fxc_path = fxc_path.value.value_or(""));

    auto& dxc_path =
        options.Add<StringOption>("dxc", R"(Path to DXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation with DXC)",
                                  Parameter{"path"});
    TINT_DEFER(opts->dxc_path = dxc_path.value.value_or(""));

    auto& xcrun =
        options.Add<StringOption>("xcrun", R"(Path to xcrun executable, used to validate MSL output.
When specified, automatically enables MSL validation)",
                                  Parameter{"path"});
    TINT_DEFER({
        if (xcrun.value.has_value()) {
            opts->xcrun_path = *xcrun.value;
            opts->validate = true;
        }
    });

#if TINT_BUILD_IR
    auto& dump_ir = options.Add<BoolOption>("dump-ir", "Writes the IR to stdout", Alias{"emit-ir"},
                                            Default{false});
    TINT_DEFER(opts->dump_ir = *dump_ir.value);

    auto& use_ir = options.Add<BoolOption>(
        "use-ir", "Use the IR for writers and transforms when possible", Default{false});
    TINT_DEFER(opts->use_ir = *use_ir.value);
#endif  // TINT_BUILD_IR

    auto& verbose =
        options.Add<BoolOption>("verbose", "Verbose output", ShortName{"v"}, Default{false});
    TINT_DEFER(opts->verbose = *verbose.value);

    auto& validate = options.Add<BoolOption>(
        "validate", "Validates the generated shader with all available validators", Default{false});
    TINT_DEFER(opts->validate = *validate.value);

    auto& parse_only =
        options.Add<BoolOption>("parse-only", "Stop after parsing the input", Default{false});
    TINT_DEFER(opts->parse_only = *parse_only.value);

#if TINT_BUILD_SPV_READER
    auto& allow_nud =
        options.Add<BoolOption>("allow-non-uniform-derivatives",
                                R"(When using SPIR-V input, allow non-uniform derivatives by
inserting a module-scope directive to suppress any uniformity
violations that may be produced)",
                                Default{false});
    TINT_DEFER({
        if (allow_nud.value.value_or(false)) {
            opts->spirv_reader_options.allow_non_uniform_derivatives = true;
        }
    });
#endif

    auto& disable_wg_init = options.Add<BoolOption>(
        "disable-workgroup-init", "Disable workgroup memory zero initialization", Default{false});
    TINT_DEFER(opts->disable_workgroup_init = *disable_wg_init.value);

    auto& rename_all = options.Add<BoolOption>("rename-all", "Renames all symbols", Default{false});
    TINT_DEFER(opts->rename_all = *rename_all.value);

    auto& dump_inspector_bindings = options.Add<BoolOption>(
        "dump-inspector-bindings", "Dump reflection data about bindings to stdout",
        Alias{"emit-inspector-bindings"}, Default{false});
    TINT_DEFER(opts->dump_inspector_bindings = *dump_inspector_bindings.value);

#if TINT_BUILD_SYNTAX_TREE_WRITER
    auto& dump_ast = options.Add<BoolOption>("dump-ast", "Writes the AST to stdout",
                                             Alias{"emit-ast"}, Default{false});
    TINT_DEFER(opts->dump_ast = *dump_ast.value);
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

    auto& print_hash = options.Add<BoolOption>("print-hash", "Emit the hash of the output program",
                                               Default{false});
    TINT_DEFER(opts->print_hash = *print_hash.value);

    auto& transforms =
        options.Add<StringOption>("transform", R"(Runs transforms, name list is comma separated
Available transforms:
)" + transform_names,
                                  ShortName{"t"});
    TINT_DEFER({
        if (transforms.value.has_value()) {
            for (auto transform : tint::Split(*transforms.value, ",")) {
                opts->transforms.Push(std::string(transform));
            }
        }
    });

    auto& hlsl_rc_bp = options.Add<StringOption>("hlsl-root-constant-binding-point",
                                                 R"(Binding point for root constant.
Specify the binding point for generated uniform buffer
used for num_workgroups in HLSL. If not specified, then
default to binding 0 of the largest used group plus 1,
or group 0 if no resource bound)");

    auto& skip_hash = options.Add<StringOption>(
        "skip-hash", R"(Skips validation if the hash of the output is equal to any
of the hash codes in the comma separated list of hashes)");
    TINT_DEFER({
        if (skip_hash.value.has_value()) {
            for (auto hash : tint::Split(*skip_hash.value, ",")) {
                uint32_t value = 0;
                int base = 10;
                if (hash.size() > 2 && hash[0] == '0' && (hash[1] == 'x' || hash[1] == 'X')) {
                    hash = hash.substr(2);
                    base = 16;
                }
                std::from_chars(hash.data(), hash.data() + hash.size(), value, base);
                opts->skip_hash.emplace(value);
            }
        }
    });

    auto& overrides = options.Add<StringOption>(
        "overrides", "Override values as IDENTIFIER=VALUE, comma-separated");

    auto& help = options.Add<BoolOption>("help", "Show usage", ShortName{"h"});

    auto show_usage = [&] {
        std::cout << R"(Usage: tint [options] <input-file>

Options:
)";
        options.ShowHelp(std::cout);
    };

    auto result = options.Parse(std::cerr, arguments);
    if (!result) {
        std::cerr << std::endl;
        show_usage();
        return false;
    }
    if (help.value.value_or(false)) {
        show_usage();
        return false;
    }

    if (overrides.value.has_value()) {
        for (const auto& o : tint::Split(*overrides.value, ",")) {
            auto parts = tint::Split(o, "=");
            if (parts.Length() != 2) {
                std::cerr << "override values must be of the form IDENTIFIER=VALUE";
                return false;
            }
            auto value = tint::ParseNumber<double>(parts[1]);
            if (!value) {
                std::cerr << "invalid override value: " << parts[1];
                return false;
            }
            opts->overrides.Add(std::string(parts[0]), value.Get());
        }
    }

    if (hlsl_rc_bp.value.has_value()) {
        auto binding_points = tint::Split(*hlsl_rc_bp.value, ",");
        if (binding_points.Length() != 2) {
            std::cerr << "Invalid binding point for " << hlsl_rc_bp.name << ": "
                      << *hlsl_rc_bp.value << std::endl;
            return false;
        }
        auto group = tint::ParseUint32(binding_points[0]);
        if (!group) {
            std::cerr << "Invalid group for " << hlsl_rc_bp.name << ": " << binding_points[0]
                      << std::endl;
            return false;
        }
        auto binding = tint::ParseUint32(binding_points[1]);
        if (!binding) {
            std::cerr << "Invalid binding for " << hlsl_rc_bp.name << ": " << binding_points[1]
                      << std::endl;
            return false;
        }
        opts->hlsl_root_constant_binding_point = tint::BindingPoint{group.Get(), binding.Get()};
    }

    auto files = result.Get();
    if (files.Length() > 1) {
        std::cerr << "More than one input file specified: "
                  << tint::Join(Transform(files, tint::Quote), ", ") << std::endl;
        return false;
    }
    if (files.Length() == 1) {
        opts->input_filename = files[0];
    }

    return true;
}

/// Writes the given `buffer` into the file named as `output_file` using the
/// given `mode`.  If `output_file` is empty or "-", writes to standard
/// output. If any error occurs, returns false and outputs error message to
/// standard error. The ContainerT type must have data() and size() methods,
/// like `std::string` and `std::vector` do.
/// @returns true on success
template <typename ContainerT>
bool WriteFile(const std::string& output_file, const std::string mode, const ContainerT& buffer) {
    const bool use_stdout = output_file.empty() || output_file == "-";
    FILE* file = stdout;

    if (!use_stdout) {
#if defined(_MSC_VER)
        fopen_s(&file, output_file.c_str(), mode.c_str());
#else
        file = fopen(output_file.c_str(), mode.c_str());
#endif
        if (!file) {
            std::cerr << "Could not open file " << output_file << " for writing" << std::endl;
            return false;
        }
    }

    size_t written =
        fwrite(buffer.data(), sizeof(typename ContainerT::value_type), buffer.size(), file);
    if (buffer.size() != written) {
        if (use_stdout) {
            std::cerr << "Could not write all output to standard output" << std::endl;
        } else {
            std::cerr << "Could not write to file " << output_file << std::endl;
            fclose(file);
        }
        return false;
    }
    if (!use_stdout) {
        fclose(file);
    }

    return true;
}

#if TINT_BUILD_SPV_WRITER
std::string Disassemble(const std::vector<uint32_t>& data) {
    std::string spv_errors;
    spv_target_env target_env = SPV_ENV_UNIVERSAL_1_0;

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

    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer(msg_consumer);

    std::string result;
    if (!tools.Disassemble(
            data, &result,
            SPV_BINARY_TO_TEXT_OPTION_INDENT | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES)) {
        std::cerr << spv_errors << std::endl;
    }
    return result;
}
#endif  // TINT_BUILD_SPV_WRITER

/// Generate SPIR-V code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateSpirv(const tint::Program* program, const Options& options) {
#if TINT_BUILD_SPV_WRITER
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::spirv::writer::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
#if TINT_BUILD_IR
    gen_options.use_tint_ir = options.use_ir;
#endif
    auto result = tint::spirv::writer::Generate(program, gen_options);
    if (!result) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.Failure() << std::endl;
        return false;
    }

    if (options.format == Format::kSpvAsm) {
        if (!WriteFile(options.output_file, "w", Disassemble(result.Get().spirv))) {
            return false;
        }
    } else {
        if (!WriteFile(options.output_file, "wb", result.Get().spirv)) {
            return false;
        }
    }

    const auto hash = tint::CRC32(result.Get().spirv.data(), result.Get().spirv.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Use Vulkan 1.1, since this is what Tint, internally, uses.
        spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
        tools.SetMessageConsumer(
            [](spv_message_level_t, const char*, const spv_position_t& pos, const char* msg) {
                std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << std::endl;
            });
        if (!tools.Validate(result.Get().spirv.data(), result.Get().spirv.size(),
                            spvtools::ValidatorOptions())) {
            return false;
        }
    }

    return true;
#else
    (void)program;
    (void)options;
    std::cerr << "SPIR-V writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_SPV_WRITER
}

/// Generate WGSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateWgsl(const tint::Program* program, const Options& options) {
#if TINT_BUILD_WGSL_WRITER
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::wgsl::writer::Options gen_options;
    auto result = tint::wgsl::writer::Generate(program, gen_options);
    if (!result.success) {
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result.wgsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result.wgsl.data(), result.wgsl.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Attempt to re-parse the output program with Tint's WGSL reader.
        auto source = std::make_unique<tint::Source::File>(options.input_filename, result.wgsl);
        auto reparsed_program = tint::wgsl::reader::Parse(source.get());
        if (!reparsed_program.IsValid()) {
            auto diag_printer = tint::diag::Printer::create(stderr, true);
            tint::diag::Formatter diag_formatter;
            diag_formatter.format(reparsed_program.Diagnostics(), diag_printer.get());
            return false;
        }
    }

    return true;
#else
    (void)program;
    (void)options;
    std::cerr << "WGSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_WGSL_WRITER
}

/// Generate MSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateMsl(const tint::Program* program, const Options& options) {
#if TINT_BUILD_MSL_WRITER
    // Remap resource numbers to a flat namespace.
    // TODO(crbug.com/tint/1501): Do this via Options::BindingMap.
    const tint::Program* input_program = program;
    auto flattened = tint::writer::FlattenBindings(program);
    if (flattened) {
        input_program = &*flattened;
    }

    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::msl::writer::Options gen_options;
#if TINT_BUILD_IR
    gen_options.use_tint_ir = options.use_ir;
#endif
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(input_program);
    gen_options.array_length_from_uniform.ubo_binding = tint::BindingPoint{0, 30};
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 0},
                                                                          0);
    gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(tint::BindingPoint{0, 1},
                                                                          1);
    auto result = tint::msl::writer::Generate(input_program, gen_options);
    if (!result) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.Failure() << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result->msl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->msl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        tint::msl::validate::Result res;
#ifdef TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
        res = tint::msl::validate::UsingMetalAPI(result->msl);
#else
#ifdef _WIN32
        const char* default_xcrun_exe = "metal.exe";
#else
        const char* default_xcrun_exe = "xcrun";
#endif
        auto xcrun = tint::Command::LookPath(
            options.xcrun_path.empty() ? default_xcrun_exe : std::string(options.xcrun_path));
        if (xcrun.Found()) {
            res = tint::msl::validate::Msl(xcrun.Path(), result->msl);
        } else {
            res.output = "xcrun executable not found. Cannot validate.";
            res.failed = true;
        }
#endif  // TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
        if (res.failed) {
            std::cerr << res.output << std::endl;
            return false;
        }
    }

    return true;
#else
    (void)program;
    (void)options;
    std::cerr << "MSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_MSL_WRITER
}

/// Generate HLSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateHlsl(const tint::Program* program, const Options& options) {
#if TINT_BUILD_HLSL_WRITER
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::hlsl::writer::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
    gen_options.root_constant_binding_point = options.hlsl_root_constant_binding_point;
    auto result = tint::hlsl::writer::Generate(program, gen_options);
    if (!result) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.Failure() << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result->hlsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->hlsl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    // If --fxc or --dxc was passed, then we must explicitly find and validate with that respective
    // compiler.
    const bool must_validate_dxc = !options.dxc_path.empty();
    const bool must_validate_fxc = !options.fxc_path.empty();
    if ((options.validate || must_validate_dxc || must_validate_fxc) &&
        (options.skip_hash.count(hash) == 0)) {
        tint::hlsl::validate::Result dxc_res;
        bool dxc_found = false;
        if (options.validate || must_validate_dxc) {
            auto dxc = tint::Command::LookPath(
                options.dxc_path.empty() ? "dxc" : std::string(options.dxc_path));
            if (dxc.Found()) {
                dxc_found = true;

                auto enable_list = program->AST().Enables();
                bool dxc_require_16bit_types = false;
                for (auto* enable : enable_list) {
                    if (enable->HasExtension(tint::builtin::Extension::kF16)) {
                        dxc_require_16bit_types = true;
                        break;
                    }
                }

                dxc_res = tint::hlsl::validate::UsingDXC(
                    dxc.Path(), result->hlsl, result->entry_points, dxc_require_16bit_types);
            } else if (must_validate_dxc) {
                // DXC was explicitly requested. Error if it could not be found.
                dxc_res.failed = true;
                dxc_res.output = "DXC executable '" + std::string(options.dxc_path) +
                                 "' not found. Cannot validate";
            }
        }

        tint::hlsl::validate::Result fxc_res;
        bool fxc_found = false;
        if (options.validate || must_validate_fxc) {
            auto fxc =
                tint::Command::LookPath(options.fxc_path.empty() ? tint::hlsl::validate::kFxcDLLName
                                                                 : std::string(options.fxc_path));

#ifdef _WIN32
            if (fxc.Found()) {
                fxc_found = true;
                fxc_res =
                    tint::hlsl::validate::UsingFXC(fxc.Path(), result->hlsl, result->entry_points);
            } else if (must_validate_fxc) {
                // FXC was explicitly requested. Error if it could not be found.
                fxc_res.failed = true;
                fxc_res.output = "FXC DLL '" + options.fxc_path + "' not found. Cannot validate";
            }
#else
            if (must_validate_dxc) {
                fxc_res.failed = true;
                fxc_res.output = "FXC can only be used on Windows.";
            }
#endif  // _WIN32
        }

        if (fxc_res.failed) {
            std::cerr << "FXC validation failure:" << std::endl << fxc_res.output << std::endl;
        }
        if (dxc_res.failed) {
            std::cerr << "DXC validation failure:" << std::endl << dxc_res.output << std::endl;
        }
        if (fxc_res.failed || dxc_res.failed) {
            return false;
        }
        if (!fxc_found && !dxc_found) {
            std::cerr << "Couldn't find FXC or DXC. Cannot validate" << std::endl;
            return false;
        }
        if (options.verbose) {
            if (fxc_found && !fxc_res.failed) {
                std::cout << "Passed FXC validation" << std::endl;
                std::cout << fxc_res.output;
                std::cout << std::endl;
            }
            if (dxc_found && !dxc_res.failed) {
                std::cout << "Passed DXC validation" << std::endl;
                std::cout << dxc_res.output;
                std::cout << std::endl;
            }
        }
    }

    return true;
#else
    (void)program;
    (void)options;
    std::cerr << "HLSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_HLSL_WRITER
}

#if TINT_BUILD_GLSL_WRITER
EShLanguage pipeline_stage_to_esh_language(tint::ast::PipelineStage stage) {
    switch (stage) {
        case tint::ast::PipelineStage::kFragment:
            return EShLangFragment;
        case tint::ast::PipelineStage::kVertex:
            return EShLangVertex;
        case tint::ast::PipelineStage::kCompute:
            return EShLangCompute;
        default:
            TINT_UNREACHABLE();
            return EShLangVertex;
    }
}
#endif

/// Generate GLSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateGlsl(const tint::Program* program, const Options& options) {
#if TINT_BUILD_GLSL_WRITER
    if (options.validate) {
        glslang::InitializeProcess();
    }

    auto generate = [&](const tint::Program* prg, const std::string entry_point_name) -> bool {
        tint::glsl::writer::Options gen_options;
        gen_options.disable_robustness = !options.enable_robustness;
        gen_options.external_texture_options.bindings_map =
            tint::cmd::GenerateExternalTextureBindings(prg);
        auto result = tint::glsl::writer::Generate(prg, gen_options, entry_point_name);
        if (!result.success) {
            tint::cmd::PrintWGSL(std::cerr, *prg);
            std::cerr << "Failed to generate: " << result.error << std::endl;
            return false;
        }

        if (!WriteFile(options.output_file, "w", result.glsl)) {
            return false;
        }

        const auto hash = tint::CRC32(result.glsl.c_str());
        if (options.print_hash) {
            PrintHash(hash);
        }

        if (options.validate && options.skip_hash.count(hash) == 0) {
            for (auto entry_pt : result.entry_points) {
                EShLanguage lang = pipeline_stage_to_esh_language(entry_pt.second);
                glslang::TShader shader(lang);
                const char* strings[1] = {result.glsl.c_str()};
                int lengths[1] = {static_cast<int>(result.glsl.length())};
                shader.setStringsWithLengths(strings, lengths, 1);
                shader.setEntryPoint("main");
                bool glslang_result = shader.parse(GetDefaultResources(), 310, EEsProfile, false,
                                                   false, EShMsgDefault);
                if (!glslang_result) {
                    std::cerr << "Error parsing GLSL shader:\n"
                              << shader.getInfoLog() << "\n"
                              << shader.getInfoDebugLog() << "\n";
                    return false;
                }
            }
        }
        return true;
    };

    tint::inspector::Inspector inspector(program);

    if (inspector.GetEntryPoints().empty()) {
        // Pass empty string here so that the GLSL generator will generate
        // code for all functions, reachable or not.
        return generate(program, "");
    }

    bool success = true;
    for (auto& entry_point : inspector.GetEntryPoints()) {
        success &= generate(program, entry_point.name);
    }
    return success;
#else
    (void)program;
    (void)options;
    std::cerr << "GLSL writer not enabled in tint build" << std::endl;
    return false;
#endif  // TINT_BUILD_GLSL_WRITER
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

    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

#if TINT_BUILD_WGSL_WRITER
    tint::Program::printer = [](const tint::Program* program) {
        auto result = tint::wgsl::writer::Generate(program, {});
        if (!result.error.empty()) {
            return "error: " + result.error;
        }
        return result.wgsl;
    };
#endif  // TINT_BUILD_WGSL_WRITER

    struct TransformFactory {
        const char* name;
        /// Build and adds the transform to the transform manager.
        /// Parameters:
        ///   inspector - an inspector created from the parsed program
        ///   manager   - the transform manager. Add transforms to this.
        ///   inputs    - the input data to the transform manager. Add inputs to this.
        /// Returns true on success, false on error (program will immediately exit)
        std::function<bool(tint::inspector::Inspector& inspector,
                           tint::ast::transform::Manager& manager,
                           tint::ast::transform::DataMap& inputs)>
            make;
    };
    std::vector<TransformFactory> transforms = {
        {"first_index_offset",
         [](tint::inspector::Inspector&, tint::ast::transform::Manager& m,
            tint::ast::transform::DataMap& i) {
             i.Add<tint::ast::transform::FirstIndexOffset::BindingPoint>(0, 0);
             m.Add<tint::ast::transform::FirstIndexOffset>();
             return true;
         }},
        {"renamer",
         [](tint::inspector::Inspector&, tint::ast::transform::Manager& m,
            tint::ast::transform::DataMap&) {
             m.Add<tint::ast::transform::Renamer>();
             return true;
         }},
        {"robustness",
         [&](tint::inspector::Inspector&, tint::ast::transform::Manager&,
             tint::ast::transform::DataMap&) {  // enabled via writer option
             options.enable_robustness = true;
             return true;
         }},
        {"substitute_override",
         [&](tint::inspector::Inspector& inspector, tint::ast::transform::Manager& m,
             tint::ast::transform::DataMap& i) {
             tint::ast::transform::SubstituteOverride::Config cfg;

             std::unordered_map<tint::OverrideId, double> values;
             values.reserve(options.overrides.Count());

             for (auto override : options.overrides) {
                 const auto& name = override.key;
                 const auto& value = override.value;
                 if (name.empty()) {
                     std::cerr << "empty override name" << std::endl;
                     return false;
                 }
                 if (auto num = tint::ParseNumber<decltype(tint::OverrideId::value)>(name)) {
                     tint::OverrideId id{num.Get()};
                     values.emplace(id, value);
                 } else {
                     auto override_names = inspector.GetNamedOverrideIds();
                     auto it = override_names.find(name);
                     if (it == override_names.end()) {
                         std::cerr << "unknown override '" << name << "'" << std::endl;
                         return false;
                     }
                     values.emplace(it->second, value);
                 }
             }

             cfg.map = std::move(values);

             i.Add<tint::ast::transform::SubstituteOverride::Config>(cfg);
             m.Add<tint::ast::transform::SubstituteOverride>();
             return true;
         }},
    };
    auto transform_names = [&] {
        tint::StringStream names;
        for (auto& t : transforms) {
            names << "   " << t.name << std::endl;
        }
        return names.str();
    };

    if (!ParseArgs(arguments, transform_names(), &options)) {
        return 1;
    }

    // Implement output format defaults.
    if (options.format == Format::kUnknown) {
        // Try inferring from filename.
        options.format = infer_format(options.output_file);
    }
    if (options.format == Format::kUnknown) {
        // Ultimately, default to SPIR-V assembly. That's nice for interactive use.
        options.format = Format::kSpvAsm;
    }

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

    if (options.parse_only) {
        return 1;
    }

#if TINT_BUILD_SYNTAX_TREE_WRITER
    if (options.dump_ast) {
        tint::wgsl::writer::Options gen_options;
        gen_options.use_syntax_tree_writer = true;
        auto result = tint::wgsl::writer::Generate(program.get(), gen_options);
        if (!result.success) {
            std::cerr << "Failed to dump AST: " << result.error << std::endl;
        } else {
            std::cout << result.wgsl << std::endl;
        }
    }
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

#if TINT_BUILD_WGSL_READER && TINT_BUILD_IR
    if (options.dump_ir) {
        auto result = tint::wgsl::reader::ProgramToIR(program.get());
        if (!result) {
            std::cerr << "Failed to build IR from program: " << result.Failure() << std::endl;
        } else {
            auto mod = result.Move();
            if (options.dump_ir) {
                tint::ir::Disassembler d(mod);
                std::cout << d.Disassemble() << std::endl;
            }
        }
    }
#endif  // TINT_BUILD_WGSL_READER && TINT_BUILD_IR

    tint::inspector::Inspector inspector(program.get());
    if (options.dump_inspector_bindings) {
        tint::cmd::PrintInspectorBindings(inspector);
    }

    tint::ast::transform::Manager transform_manager;
    tint::ast::transform::DataMap transform_inputs;

    // Renaming must always come first
    switch (options.format) {
        case Format::kMsl: {
#if TINT_BUILD_MSL_WRITER
            transform_inputs.Add<tint::ast::transform::Renamer::Config>(
                options.rename_all ? tint::ast::transform::Renamer::Target::kAll
                                   : tint::ast::transform::Renamer::Target::kMslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::ast::transform::Renamer>();
#endif  // TINT_BUILD_MSL_WRITER
            break;
        }
#if TINT_BUILD_GLSL_WRITER
        case Format::kGlsl: {
            transform_inputs.Add<tint::ast::transform::Renamer::Config>(
                options.rename_all ? tint::ast::transform::Renamer::Target::kAll
                                   : tint::ast::transform::Renamer::Target::kGlslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::ast::transform::Renamer>();
            break;
        }
#endif  // TINT_BUILD_GLSL_WRITER
        case Format::kHlsl: {
#if TINT_BUILD_HLSL_WRITER
            transform_inputs.Add<tint::ast::transform::Renamer::Config>(
                options.rename_all ? tint::ast::transform::Renamer::Target::kAll
                                   : tint::ast::transform::Renamer::Target::kHlslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::ast::transform::Renamer>();
#endif  // TINT_BUILD_HLSL_WRITER
            break;
        }
        default: {
            if (options.rename_all) {
                transform_manager.Add<tint::ast::transform::Renamer>();
            }
            break;
        }
    }

    auto enable_transform = [&](std::string_view name) {
        for (auto& t : transforms) {
            if (t.name == name) {
                return t.make(inspector, transform_manager, transform_inputs);
            }
        }

        std::cerr << "Unknown transform: " << name << std::endl;
        std::cerr << "Available transforms: " << std::endl << transform_names() << std::endl;
        return false;
    };

    // If overrides are provided, add the SubstituteOverride transform.
    if (!options.overrides.IsEmpty()) {
        if (!enable_transform("substitute_override")) {
            return 1;
        }
    }

    for (const auto& name : options.transforms) {
        // TODO(dsinclair): The vertex pulling transform requires setup code to
        // be run that needs user input. Should we find a way to support that here
        // maybe through a provided file?
        if (!enable_transform(name)) {
            return 1;
        }
    }

    if (options.emit_single_entry_point) {
        transform_manager.append(std::make_unique<tint::ast::transform::SingleEntryPoint>());
        transform_inputs.Add<tint::ast::transform::SingleEntryPoint::Config>(options.ep_name);
    }

    tint::ast::transform::DataMap outputs;
    auto out = transform_manager.Run(program.get(), std::move(transform_inputs), outputs);
    if (!out.IsValid()) {
        tint::cmd::PrintWGSL(std::cerr, out);
        std::cerr << out.Diagnostics().str() << std::endl;
        return 1;
    }

    *program = std::move(out);

    bool success = false;
    switch (options.format) {
        case Format::kSpirv:
        case Format::kSpvAsm:
            success = GenerateSpirv(program.get(), options);
            break;
        case Format::kWgsl:
            success = GenerateWgsl(program.get(), options);
            break;
        case Format::kMsl:
            success = GenerateMsl(program.get(), options);
            break;
        case Format::kHlsl:
            success = GenerateHlsl(program.get(), options);
            break;
        case Format::kGlsl:
            success = GenerateGlsl(program.get(), options);
            break;
        case Format::kNone:
            break;
        default:
            std::cerr << "Unknown output format specified" << std::endl;
            return 1;
    }
    if (!success) {
        return 1;
    }

    return 0;
}
