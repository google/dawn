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

#if TINT_BUILD_SYNTAX_TREE_WRITER
#include "src/tint/writer/syntax_tree/generator.h"  // nogncheck

#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

#if TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER

#include "src/tint/ast/module.h"
#include "src/tint/cmd/generate_external_texture_bindings.h"
#include "src/tint/cmd/helper.h"
#include "src/tint/utils/io/command.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/utils/transform.h"
#include "src/tint/val/val.h"
#include "tint/tint.h"

#if TINT_BUILD_IR
#include "src/tint/ir/debug.h"         // nogncheck
#include "src/tint/ir/disassembler.h"  // nogncheck
#include "src/tint/ir/from_program.h"  // nogncheck
#include "src/tint/ir/module.h"        // nogncheck
#endif                                 // TINT_BUILD_IR

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
    bool show_help = false;
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
    tint::reader::spirv::Options spirv_reader_options;
#endif

    std::vector<std::string> transforms;

    std::string fxc_path;
    std::string dxc_path;
    std::string xcrun_path;
    std::unordered_map<std::string, double> overrides;
    std::optional<tint::sem::BindingPoint> hlsl_root_constant_binding_point;

#if TINT_BUILD_IR
    bool dump_ir = false;
    bool dump_ir_graph = false;
    bool use_ir = false;
#endif  // TINT_BUILD_IR

#if TINT_BUILD_SYNTAX_TREE_WRITER
    bool dump_syntax_tree = false;
#endif  // TINB_BUILD_SYNTAX_TREE_WRITER
};

const char kUsage[] = R"(Usage: tint [options] <input-file>

 options:
  --format <spirv|spvasm|wgsl|msl|hlsl|none>  -- Output format.
                               If not provided, will be inferred from output
                               filename extension:
                                   .spvasm -> spvasm
                                   .spv    -> spirv
                                   .wgsl   -> wgsl
                                   .metal  -> msl
                                   .hlsl   -> hlsl
                               If none matches, then default to SPIR-V assembly.
  -ep <name>                -- Output single entry point
  --output-file <name>      -- Output file name.  Use "-" for standard output
  -o <name>                 -- Output file name.  Use "-" for standard output
  --transform <name list>   -- Runs transforms, name list is comma separated
                               Available transforms:
${transforms} --parse-only              -- Stop after parsing the input
  --allow-non-uniform-derivatives  -- When using SPIR-V input, allow non-uniform derivatives by
                               inserting a module-scope directive to suppress any uniformity
                               violations that may be produced.
  --disable-workgroup-init  -- Disable workgroup memory zero initialization.
  --dump-inspector-bindings -- Dump reflection data about bindins to stdout.
  -h                        -- This help text
  --hlsl-root-constant-binding-point <group>,<binding>  -- Binding point for root constant.
                               Specify the binding point for generated uniform buffer
                               used for num_workgroups in HLSL. If not specified, then
                               default to binding 0 of the largest used group plus 1,
                               or group 0 if no resource bound.
  --validate                -- Validates the generated shader with all available validators
  --skip-hash <hash list>   -- Skips validation if the hash of the output is equal to any
                               of the hash codes in the comma separated list of hashes
  --print-hash              -- Emit the hash of the output program
  --fxc                     -- Path to FXC dll, used to validate HLSL output.
                               When specified, automatically enables HLSL validation with FXC
  --dxc                     -- Path to DXC executable, used to validate HLSL output.
                               When specified, automatically enables HLSL validation with DXC
  --xcrun                   -- Path to xcrun executable, used to validate MSL output.
                               When specified, automatically enables MSL validation
  --overrides               -- Override values as IDENTIFIER=VALUE, comma-separated.
  --rename-all              -- Renames all symbols.
)";

Format parse_format(const std::string& fmt) {
    (void)fmt;

#if TINT_BUILD_SPV_WRITER
    if (fmt == "spirv") {
        return Format::kSpirv;
    }
    if (fmt == "spvasm") {
        return Format::kSpvAsm;
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

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format infer_format(const std::string& filename) {
    (void)filename;

#if TINT_BUILD_SPV_WRITER
    if (tint::utils::HasSuffix(filename, ".spv")) {
        return Format::kSpirv;
    }
    if (tint::utils::HasSuffix(filename, ".spvasm")) {
        return Format::kSpvAsm;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (tint::utils::HasSuffix(filename, ".wgsl")) {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (tint::utils::HasSuffix(filename, ".metal")) {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (tint::utils::HasSuffix(filename, ".hlsl")) {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

    return Format::kUnknown;
}

std::vector<std::string> split_on_char(std::string list, char c) {
    std::vector<std::string> res;

    std::istringstream str(list);
    while (str.good()) {
        std::string substr;
        getline(str, substr, c);
        res.push_back(substr);
    }
    return res;
}

std::vector<std::string> split_on_comma(std::string list) {
    return split_on_char(list, ',');
}

std::vector<std::string> split_on_equal(std::string list) {
    return split_on_char(list, '=');
}

std::optional<uint64_t> parse_unsigned_number(std::string number) {
    for (char c : number) {
        if (!std::isdigit(c)) {
            // Found a non-digital char, return nullopt
            return std::nullopt;
        }
    }

    errno = 0;
    char* p_end;
    uint64_t result;
    // std::strtoull will not throw exception.
    result = std::strtoull(number.c_str(), &p_end, 10);
    if ((errno != 0) || (static_cast<size_t>(p_end - number.c_str()) != number.length())) {
        // Unexpected conversion result
        return std::nullopt;
    }

    return result;
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
        } else if (arg == "-ep") {
            if (i + 1 >= args.size()) {
                std::cerr << "Missing value for -ep" << std::endl;
                return false;
            }
            i++;
            opts->ep_name = args[i];
            opts->emit_single_entry_point = true;

        } else if (arg == "-o" || arg == "--output-name") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->output_file = args[i];

        } else if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            opts->verbose = true;
        } else if (arg == "--transform") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->transforms = split_on_comma(args[i]);
        } else if (arg == "--parse-only") {
            opts->parse_only = true;
        } else if (arg == "--allow-non-uniform-derivatives") {
#if TINT_BUILD_SPV_READER
            opts->spirv_reader_options.allow_non_uniform_derivatives = true;
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            return false;
#endif
        } else if (arg == "--disable-workgroup-init") {
            opts->disable_workgroup_init = true;
        } else if (arg == "--dump-inspector-bindings") {
            opts->dump_inspector_bindings = true;
        } else if (arg == "--validate") {
            opts->validate = true;
        } else if (arg == "--skip-hash") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing hash value for " << arg << std::endl;
                return false;
            }
            for (auto hash : split_on_comma(args[i])) {
                uint32_t value = 0;
                int base = 10;
                if (hash.size() > 2 && hash[0] == '0' && (hash[1] == 'x' || hash[1] == 'X')) {
                    hash = hash.substr(2);
                    base = 16;
                }
                std::from_chars(hash.data(), hash.data() + hash.size(), value, base);
                opts->skip_hash.emplace(value);
            }
        } else if (arg == "--print-hash") {
            opts->print_hash = true;
        } else if (arg == "--fxc") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->fxc_path = args[i];
        } else if (arg == "--dxc") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->dxc_path = args[i];
#if TINT_BUILD_IR
        } else if (arg == "--dump-ir") {
            opts->dump_ir = true;
        } else if (arg == "--dump-ir-graph") {
            opts->dump_ir_graph = true;
        } else if (arg == "--use-ir") {
            opts->use_ir = true;
#endif  // TINT_BUILD_IR
#if TINT_BUILD_SYNTAX_TREE_WRITER
        } else if (arg == "--dump-ast") {
            opts->dump_syntax_tree = true;
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER
        } else if (arg == "--xcrun") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->xcrun_path = args[i];
            opts->validate = true;
        } else if (arg == "--overrides") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            for (const auto& o : split_on_comma(args[i])) {
                auto parts = split_on_equal(o);
                opts->overrides.insert({parts[0], std::stod(parts[1])});
            }
        } else if (arg == "--rename-all") {
            ++i;
            opts->rename_all = true;
        } else if (arg == "--hlsl-root-constant-binding-point") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            auto binding_points = split_on_comma(args[i]);
            if (binding_points.size() != 2) {
                std::cerr << "Invalid binding point for " << arg << ": " << args[i] << std::endl;
                return false;
            }
            auto group = parse_unsigned_number(binding_points[0]);
            if ((!group.has_value()) || (group.value() > std::numeric_limits<uint32_t>::max())) {
                std::cerr << "Invalid group for " << arg << ": " << binding_points[0] << std::endl;
                return false;
            }
            auto binding = parse_unsigned_number(binding_points[1]);
            if ((!binding.has_value()) ||
                (binding.value() > std::numeric_limits<uint32_t>::max())) {
                std::cerr << "Invalid binding for " << arg << ": " << binding_points[1]
                          << std::endl;
                return false;
            }
            opts->hlsl_root_constant_binding_point = tint::sem::BindingPoint{
                static_cast<uint32_t>(group.value()), static_cast<uint32_t>(binding.value())};
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
    tint::writer::spirv::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
#if TINT_BUILD_IR
    gen_options.use_tint_ir = options.use_ir;
#endif
    auto result = tint::writer::spirv::Generate(program, gen_options);
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (options.format == Format::kSpvAsm) {
        if (!WriteFile(options.output_file, "w", Disassemble(result.spirv))) {
            return false;
        }
    } else {
        if (!WriteFile(options.output_file, "wb", result.spirv)) {
            return false;
        }
    }

    const auto hash = tint::utils::CRC32(result.spirv.data(), result.spirv.size());
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
        if (!tools.Validate(result.spirv.data(), result.spirv.size(),
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
    tint::writer::wgsl::Options gen_options;
    auto result = tint::writer::wgsl::Generate(program, gen_options);
    if (!result.success) {
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result.wgsl)) {
        return false;
    }

    const auto hash = tint::utils::CRC32(result.wgsl.data(), result.wgsl.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Attempt to re-parse the output program with Tint's WGSL reader.
        auto source = std::make_unique<tint::Source::File>(options.input_filename, result.wgsl);
        auto reparsed_program = tint::reader::wgsl::Parse(source.get());
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
    tint::writer::msl::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
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

    if (!WriteFile(options.output_file, "w", result.msl)) {
        return false;
    }

    const auto hash = tint::utils::CRC32(result.msl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        tint::val::Result res;
#ifdef TINT_ENABLE_MSL_VALIDATION_USING_METAL_API
        res = tint::val::MslUsingMetalAPI(result.msl);
#else
#ifdef _WIN32
        const char* default_xcrun_exe = "metal.exe";
#else
        const char* default_xcrun_exe = "xcrun";
#endif
        auto xcrun = tint::utils::Command::LookPath(
            options.xcrun_path.empty() ? default_xcrun_exe : options.xcrun_path);
        if (xcrun.Found()) {
            res = tint::val::Msl(xcrun.Path(), result.msl);
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
    tint::writer::hlsl::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.external_texture_options.bindings_map =
        tint::cmd::GenerateExternalTextureBindings(program);
    gen_options.root_constant_binding_point = options.hlsl_root_constant_binding_point;
    auto result = tint::writer::hlsl::Generate(program, gen_options);
    if (!result.success) {
        tint::cmd::PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result.hlsl)) {
        return false;
    }

    const auto hash = tint::utils::CRC32(result.hlsl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    // If --fxc or --dxc was passed, then we must explicitly find and validate with that respective
    // compiler.
    const bool must_validate_dxc = !options.dxc_path.empty();
    const bool must_validate_fxc = !options.fxc_path.empty();
    if ((options.validate || must_validate_dxc || must_validate_fxc) &&
        (options.skip_hash.count(hash) == 0)) {
        tint::val::Result dxc_res;
        bool dxc_found = false;
        if (options.validate || must_validate_dxc) {
            auto dxc =
                tint::utils::Command::LookPath(options.dxc_path.empty() ? "dxc" : options.dxc_path);
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

                dxc_res = tint::val::HlslUsingDXC(dxc.Path(), result.hlsl, result.entry_points,
                                                  dxc_require_16bit_types);
            } else if (must_validate_dxc) {
                // DXC was explicitly requested. Error if it could not be found.
                dxc_res.failed = true;
                dxc_res.output =
                    "DXC executable '" + options.dxc_path + "' not found. Cannot validate";
            }
        }

        tint::val::Result fxc_res;
        bool fxc_found = false;
        if (options.validate || must_validate_fxc) {
            auto fxc = tint::utils::Command::LookPath(
                options.fxc_path.empty() ? tint::val::kFxcDLLName : options.fxc_path);

#ifdef _WIN32
            if (fxc.Found()) {
                fxc_found = true;
                fxc_res = tint::val::HlslUsingFXC(fxc.Path(), result.hlsl, result.entry_points);
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
            TINT_ASSERT(AST, false);
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
        tint::writer::glsl::Options gen_options;
        gen_options.disable_robustness = !options.enable_robustness;
        gen_options.external_texture_options.bindings_map =
            tint::cmd::GenerateExternalTextureBindings(prg);
        auto result = tint::writer::glsl::Generate(prg, gen_options, entry_point_name);
        if (!result.success) {
            tint::cmd::PrintWGSL(std::cerr, *prg);
            std::cerr << "Failed to generate: " << result.error << std::endl;
            return false;
        }

        if (!WriteFile(options.output_file, "w", result.glsl)) {
            return false;
        }

        const auto hash = tint::utils::CRC32(result.glsl.c_str());
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

    struct TransformFactory {
        const char* name;
        /// Build and adds the transform to the transform manager.
        /// Parameters:
        ///   inspector - an inspector created from the parsed program
        ///   manager   - the transform manager. Add transforms to this.
        ///   inputs    - the input data to the transform manager. Add inputs to this.
        /// Returns true on success, false on error (program will immediately exit)
        std::function<bool(tint::inspector::Inspector& inspector,
                           tint::transform::Manager& manager,
                           tint::transform::DataMap& inputs)>
            make;
    };
    std::vector<TransformFactory> transforms = {
        {"first_index_offset",
         [](tint::inspector::Inspector&, tint::transform::Manager& m, tint::transform::DataMap& i) {
             i.Add<tint::transform::FirstIndexOffset::BindingPoint>(0, 0);
             m.Add<tint::transform::FirstIndexOffset>();
             return true;
         }},
        {"renamer",
         [](tint::inspector::Inspector&, tint::transform::Manager& m, tint::transform::DataMap&) {
             m.Add<tint::transform::Renamer>();
             return true;
         }},
        {"robustness",
         [&](tint::inspector::Inspector&, tint::transform::Manager&,
             tint::transform::DataMap&) {  // enabled via writer option
             options.enable_robustness = true;
             return true;
         }},
        {"substitute_override",
         [&](tint::inspector::Inspector& inspector, tint::transform::Manager& m,
             tint::transform::DataMap& i) {
             tint::transform::SubstituteOverride::Config cfg;

             std::unordered_map<tint::OverrideId, double> values;
             values.reserve(options.overrides.size());

             for (const auto& [name, value] : options.overrides) {
                 if (name.empty()) {
                     std::cerr << "empty override name";
                     return false;
                 }
                 if (isdigit(name[0])) {
                     tint::OverrideId id{
                         static_cast<decltype(tint::OverrideId::value)>(atoi(name.c_str()))};
                     values.emplace(id, value);
                 } else {
                     auto override_names = inspector.GetNamedOverrideIds();
                     auto it = override_names.find(name);
                     if (it == override_names.end()) {
                         std::cerr << "unknown override '" << name << "'";
                         return false;
                     }
                     values.emplace(it->second, value);
                 }
             }

             cfg.map = std::move(values);

             i.Add<tint::transform::SubstituteOverride::Config>(cfg);
             m.Add<tint::transform::SubstituteOverride>();
             return true;
         }},
    };
    auto transform_names = [&] {
        tint::utils::StringStream names;
        for (auto& t : transforms) {
            names << "   " << t.name << std::endl;
        }
        return names.str();
    };

    if (options.show_help) {
        std::string usage = tint::utils::ReplaceAll(kUsage, "${transforms}", transform_names());
#if TINT_BUILD_IR
        usage +=
            "  --dump-ir                 -- Writes the IR to stdout\n"
            "  --dump-ir-graph           -- Writes the IR graph to 'tint.dot' as a dot graph\n"
            "  --use-ir                  -- Use the IR for writers and transforms when possible\n";
#endif  // TINT_BUILD_IR
#if TINT_BUILD_SYNTAX_TREE_WRITER
        usage += "  --dump-ast                -- Writes the AST to stdout\n";
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

        std::cout << usage << std::endl;
        return 0;
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

    if (options.parse_only) {
        return 1;
    }

#if TINT_BUILD_SYNTAX_TREE_WRITER
    if (options.dump_syntax_tree) {
        tint::writer::syntax_tree::Options gen_options;
        auto result = tint::writer::syntax_tree::Generate(program.get(), gen_options);
        if (!result.success) {
            std::cerr << "Failed to dump AST: " << result.error << std::endl;
        } else {
            std::cout << result.ast << std::endl;
        }
    }
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

#if TINT_BUILD_IR
    if (options.dump_ir || options.dump_ir_graph) {
        auto result = tint::ir::FromProgram(program.get());
        if (!result) {
            std::cerr << "Failed to build IR from program: " << result.Failure() << std::endl;
        } else {
            auto mod = result.Move();
            if (options.dump_ir) {
                tint::ir::Disassembler d(mod);
                std::cout << d.Disassemble() << std::endl;
            }
            if (options.dump_ir_graph) {
                auto graph = tint::ir::Debug::AsDotGraph(&mod);
                WriteFile("tint.dot", "w", graph);
            }
        }
    }
#endif  // TINT_BUILD_IR

    tint::inspector::Inspector inspector(program.get());
    if (options.dump_inspector_bindings) {
        tint::cmd::PrintInspectorBindings(inspector);
    }

    tint::transform::Manager transform_manager;
    tint::transform::DataMap transform_inputs;

    // Renaming must always come first
    switch (options.format) {
        case Format::kMsl: {
#if TINT_BUILD_MSL_WRITER
            transform_inputs.Add<tint::transform::Renamer::Config>(
                options.rename_all ? tint::transform::Renamer::Target::kAll
                                   : tint::transform::Renamer::Target::kMslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::transform::Renamer>();
#endif  // TINT_BUILD_MSL_WRITER
            break;
        }
#if TINT_BUILD_GLSL_WRITER
        case Format::kGlsl: {
            transform_inputs.Add<tint::transform::Renamer::Config>(
                options.rename_all ? tint::transform::Renamer::Target::kAll
                                   : tint::transform::Renamer::Target::kGlslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::transform::Renamer>();
            break;
        }
#endif  // TINT_BUILD_GLSL_WRITER
        case Format::kHlsl: {
#if TINT_BUILD_HLSL_WRITER
            transform_inputs.Add<tint::transform::Renamer::Config>(
                options.rename_all ? tint::transform::Renamer::Target::kAll
                                   : tint::transform::Renamer::Target::kHlslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::transform::Renamer>();
#endif  // TINT_BUILD_HLSL_WRITER
            break;
        }
        default: {
            if (options.rename_all) {
                transform_manager.Add<tint::transform::Renamer>();
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
        std::cerr << "Available transforms: " << std::endl << transform_names();
        return false;
    };

    // If overrides are provided, add the SubstituteOverride transform.
    if (!options.overrides.empty()) {
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
        transform_manager.append(std::make_unique<tint::transform::SingleEntryPoint>());
        transform_inputs.Add<tint::transform::SingleEntryPoint::Config>(options.ep_name);
    }

    auto out = transform_manager.Run(program.get(), std::move(transform_inputs));
    if (!out.program.IsValid()) {
        tint::cmd::PrintWGSL(std::cerr, out.program);
        diag_formatter.format(out.program.Diagnostics(), diag_printer.get());
        return 1;
    }

    *program = std::move(out.program);

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
