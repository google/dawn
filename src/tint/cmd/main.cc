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

#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#if TINT_BUILD_GLSL_WRITER
#include "StandAlone/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
#endif

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "src/tint/utils/io/command.h"
#include "src/tint/utils/string.h"
#include "src/tint/val/val.h"
#include "tint/tint.h"

namespace {

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::diag::List& diagnostics) {
    auto printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter{}.format(diagnostics, printer.get());
    tint::diag::Style bold_red{tint::diag::Color::kRed, true};
    constexpr const char* please_file_bug = R"(
********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************
)";
    printer->write(please_file_bug, bold_red);
    exit(1);
}

enum class Format {
    kNone = -1,
    kSpirv,
    kSpvAsm,
    kWgsl,
    kMsl,
    kHlsl,
    kGlsl,
};

struct Options {
    bool show_help = false;

    std::string input_filename;
    std::string output_file = "-";  // Default to stdout

    bool parse_only = false;
    bool disable_workgroup_init = false;
    bool validate = false;
    bool demangle = false;
    bool dump_inspector_bindings = false;

    Format format = Format::kNone;

    bool emit_single_entry_point = false;
    std::string ep_name;

    std::vector<std::string> transforms;

    bool use_fxc = false;
    std::string dxc_path;
    std::string xcrun_path;
};

const char kUsage[] = R"(Usage: tint [options] <input-file>

 options:
  --format <spirv|spvasm|wgsl|msl|hlsl>  -- Output format.
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
${transforms}
  --parse-only              -- Stop after parsing the input
  --disable-workgroup-init  -- Disable workgroup memory zero initialization.
  --demangle                -- Preserve original source names. Demangle them.
                               Affects AST dumping, and text-based output languages.
  --dump-inspector-bindings -- Dump reflection data about bindins to stdout.
  -h                        -- This help text
  --validate                -- Validates the generated shader
  --fxc                     -- Ask to validate HLSL output using FXC instead of DXC.
                               When specified, automatically enables --validate
  --dxc                     -- Path to DXC executable, used to validate HLSL output.
                               When specified, automatically enables --validate
  --xcrun                   -- Path to xcrun executable, used to validate MSL output.
                               When specified, automatically enables --validate)";

Format parse_format(const std::string& fmt) {
    (void)fmt;

#if TINT_BUILD_SPV_WRITER
    if (fmt == "spirv")
        return Format::kSpirv;
    if (fmt == "spvasm")
        return Format::kSpvAsm;
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (fmt == "wgsl")
        return Format::kWgsl;
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (fmt == "msl")
        return Format::kMsl;
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (fmt == "hlsl")
        return Format::kHlsl;
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    if (fmt == "glsl")
        return Format::kGlsl;
#endif  // TINT_BUILD_GLSL_WRITER

    return Format::kNone;
}

#if TINT_BUILD_SPV_WRITER || TINT_BUILD_WGSL_WRITER || TINT_BUILD_MSL_WRITER || \
    TINT_BUILD_HLSL_WRITER
/// @param input input string
/// @param suffix potential suffix string
/// @returns true if input ends with the given suffix.
bool ends_with(const std::string& input, const std::string& suffix) {
    const auto input_len = input.size();
    const auto suffix_len = suffix.size();
    // Avoid integer overflow.
    return (input_len >= suffix_len) && (input_len - suffix_len == input.rfind(suffix));
}
#endif

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format infer_format(const std::string& filename) {
    (void)filename;

#if TINT_BUILD_SPV_WRITER
    if (ends_with(filename, ".spv")) {
        return Format::kSpirv;
    }
    if (ends_with(filename, ".spvasm")) {
        return Format::kSpvAsm;
    }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
    if (ends_with(filename, ".wgsl")) {
        return Format::kWgsl;
    }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
    if (ends_with(filename, ".metal")) {
        return Format::kMsl;
    }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
    if (ends_with(filename, ".hlsl")) {
        return Format::kHlsl;
    }
#endif  // TINT_BUILD_HLSL_WRITER

    return Format::kNone;
}

std::vector<std::string> split_transform_names(std::string list) {
    std::vector<std::string> res;

    std::stringstream str(list);
    while (str.good()) {
        std::string substr;
        getline(str, substr, ',');
        res.push_back(substr);
    }
    return res;
}

std::string TextureDimensionToString(tint::inspector::ResourceBinding::TextureDimension dim) {
    switch (dim) {
        case tint::inspector::ResourceBinding::TextureDimension::kNone:
            return "None";
        case tint::inspector::ResourceBinding::TextureDimension::k1d:
            return "1d";
        case tint::inspector::ResourceBinding::TextureDimension::k2d:
            return "2d";
        case tint::inspector::ResourceBinding::TextureDimension::k2dArray:
            return "2dArray";
        case tint::inspector::ResourceBinding::TextureDimension::k3d:
            return "3d";
        case tint::inspector::ResourceBinding::TextureDimension::kCube:
            return "Cube";
        case tint::inspector::ResourceBinding::TextureDimension::kCubeArray:
            return "CubeArray";
    }

    return "Unknown";
}

std::string SampledKindToString(tint::inspector::ResourceBinding::SampledKind kind) {
    switch (kind) {
        case tint::inspector::ResourceBinding::SampledKind::kFloat:
            return "Float";
        case tint::inspector::ResourceBinding::SampledKind::kUInt:
            return "UInt";
        case tint::inspector::ResourceBinding::SampledKind::kSInt:
            return "SInt";
        case tint::inspector::ResourceBinding::SampledKind::kUnknown:
            break;
    }

    return "Unknown";
}

std::string TexelFormatToString(tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return "R32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return "R32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return "R32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return "Rgba8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return "Rgba8Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return "Rgba8Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return "Rgba8Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return "Rg32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return "Rg32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return "Rg32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return "Rgba16Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return "Rgba16Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return "Rgba16Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return "Rgba32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return "Rgba32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return "Rgba32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return "None";
    }
    return "Unknown";
}

std::string ResourceTypeToString(tint::inspector::ResourceBinding::ResourceType type) {
    switch (type) {
        case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
            return "UniformBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
            return "StorageBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
            return "ReadOnlyStorageBuffer";
        case tint::inspector::ResourceBinding::ResourceType::kSampler:
            return "Sampler";
        case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
            return "ComparisonSampler";
        case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
            return "SampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
            return "MultisampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return "WriteOnlyStorageTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
            return "DepthTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return "DepthMultisampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return "ExternalTexture";
    }

    return "Unknown";
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

            if (opts->format == Format::kNone) {
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
        } else if (arg == "--transform") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->transforms = split_transform_names(args[i]);
        } else if (arg == "--parse-only") {
            opts->parse_only = true;
        } else if (arg == "--disable-workgroup-init") {
            opts->disable_workgroup_init = true;
        } else if (arg == "--demangle") {
            opts->demangle = true;
        } else if (arg == "--dump-inspector-bindings") {
            opts->dump_inspector_bindings = true;
        } else if (arg == "--validate") {
            opts->validate = true;
        } else if (arg == "--fxc") {
            opts->validate = true;
            opts->use_fxc = true;
        } else if (arg == "--dxc") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->dxc_path = args[i];
            opts->validate = true;
        } else if (arg == "--xcrun") {
            ++i;
            if (i >= args.size()) {
                std::cerr << "Missing value for " << arg << std::endl;
                return false;
            }
            opts->xcrun_path = args[i];
            opts->validate = true;
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

/// Copies the content from the file named `input_file` to `buffer`,
/// assuming each element in the file is of type `T`.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a `T` object is divisible by its required alignment.
/// @returns true if we successfully read the file.
template <typename T>
bool ReadFile(const std::string& input_file, std::vector<T>* buffer) {
    if (!buffer) {
        std::cerr << "The buffer pointer was null" << std::endl;
        return false;
    }

    FILE* file = nullptr;
#if defined(_MSC_VER)
    fopen_s(&file, input_file.c_str(), "rb");
#else
    file = fopen(input_file.c_str(), "rb");
#endif
    if (!file) {
        std::cerr << "Failed to open " << input_file << std::endl;
        return false;
    }

    fseek(file, 0, SEEK_END);
    const auto file_size = static_cast<size_t>(ftell(file));
    if (0 != (file_size % sizeof(T))) {
        std::cerr << "File " << input_file
                  << " does not contain an integral number of objects: " << file_size
                  << " bytes in the file, require " << sizeof(T) << " bytes per object"
                  << std::endl;
        fclose(file);
        return false;
    }
    fseek(file, 0, SEEK_SET);

    buffer->clear();
    buffer->resize(file_size / sizeof(T));

    size_t bytes_read = fread(buffer->data(), 1, file_size, file);
    fclose(file);
    if (bytes_read != file_size) {
        std::cerr << "Failed to read " << input_file << std::endl;
        return false;
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

/// PrintWGSL writes the WGSL of the program to the provided ostream, if the
/// WGSL writer is enabled, otherwise it does nothing.
/// @param out the output stream to write the WGSL to
/// @param program the program
void PrintWGSL(std::ostream& out, const tint::Program& program) {
#if TINT_BUILD_WGSL_WRITER
    tint::writer::wgsl::Options options;
    auto result = tint::writer::wgsl::Generate(&program, options);
    out << std::endl << result.wgsl << std::endl;
#else
    (void)out;
    (void)program;
#endif
}

/// Generate SPIR-V code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateSpirv(const tint::Program* program, const Options& options) {
#if TINT_BUILD_SPV_WRITER
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::writer::spirv::Options gen_options;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.generate_external_texture_bindings = true;
    auto result = tint::writer::spirv::Generate(program, gen_options);
    if (!result.success) {
        PrintWGSL(std::cerr, *program);
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

    if (options.validate) {
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

    if (options.validate) {
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
    const tint::Program* input_program = program;

    // Remap resource numbers to a flat namespace.
    // TODO(crbug.com/tint/1101): Make this more robust for multiple entry points.
    using BindingPoint = tint::transform::BindingPoint;
    tint::transform::BindingRemapper::BindingPoints binding_points;
    uint32_t next_buffer_idx = 0;
    uint32_t next_sampler_idx = 0;
    uint32_t next_texture_idx = 0;

    tint::inspector::Inspector inspector(program);
    auto entry_points = inspector.GetEntryPoints();
    for (auto& entry_point : entry_points) {
        auto bindings = inspector.GetResourceBindings(entry_point.name);
        for (auto& binding : bindings) {
            BindingPoint src = {binding.bind_group, binding.binding};
            if (binding_points.count(src)) {
                continue;
            }
            switch (binding.resource_type) {
                case tint::inspector::ResourceBinding::ResourceType::kUniformBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kStorageBuffer:
                case tint::inspector::ResourceBinding::ResourceType::kReadOnlyStorageBuffer:
                    binding_points.emplace(src, BindingPoint{0, next_buffer_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampler:
                case tint::inspector::ResourceBinding::ResourceType::kComparisonSampler:
                    binding_points.emplace(src, BindingPoint{0, next_sampler_idx++});
                    break;
                case tint::inspector::ResourceBinding::ResourceType::kSampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
                case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
                case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
                    binding_points.emplace(src, BindingPoint{0, next_texture_idx++});
                    break;
            }
        }
    }

    // Run the binding remapper transform.
    tint::transform::Output transform_output;
    if (!binding_points.empty()) {
        tint::transform::Manager manager;
        tint::transform::DataMap inputs;
        inputs.Add<tint::transform::BindingRemapper::Remappings>(
            std::move(binding_points), tint::transform::BindingRemapper::AccessControls{},
            /* mayCollide */ true);
        manager.Add<tint::transform::BindingRemapper>();
        transform_output = manager.Run(program, inputs);
        input_program = &transform_output.program;
    }

    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::writer::msl::Options gen_options;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.generate_external_texture_bindings = true;
    auto result = tint::writer::msl::Generate(input_program, gen_options);
    if (!result.success) {
        PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result.msl)) {
        return false;
    }

    if (options.validate) {
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
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.generate_external_texture_bindings = true;
    auto result = tint::writer::hlsl::Generate(program, gen_options);
    if (!result.success) {
        PrintWGSL(std::cerr, *program);
        std::cerr << "Failed to generate: " << result.error << std::endl;
        return false;
    }

    if (!WriteFile(options.output_file, "w", result.hlsl)) {
        return false;
    }

    if (options.validate) {
        tint::val::Result res;
        if (options.use_fxc) {
#ifdef _WIN32
            res = tint::val::HlslUsingFXC(result.hlsl, result.entry_points);
#else
            res.failed = true;
            res.output = "FXC can only be used on Windows. Sorry :X";
#endif  // _WIN32
        } else {
            auto dxc =
                tint::utils::Command::LookPath(options.dxc_path.empty() ? "dxc" : options.dxc_path);
            if (dxc.Found()) {
                res = tint::val::HlslUsingDXC(dxc.Path(), result.hlsl, result.entry_points);
            } else {
                res.failed = true;
                res.output = "DXC executable not found. Cannot validate";
            }
        }
        if (res.failed) {
            std::cerr << res.output << std::endl;
            return false;
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
        gen_options.generate_external_texture_bindings = true;
        auto result = tint::writer::glsl::Generate(prg, gen_options, entry_point_name);
        if (!result.success) {
            PrintWGSL(std::cerr, *prg);
            std::cerr << "Failed to generate: " << result.error << std::endl;
            return false;
        }

        if (!WriteFile(options.output_file, "w", result.glsl)) {
            return false;
        }

        if (options.validate) {
            for (auto entry_pt : result.entry_points) {
                EShLanguage lang = pipeline_stage_to_esh_language(entry_pt.second);
                glslang::TShader shader(lang);
                const char* strings[1] = {result.glsl.c_str()};
                int lengths[1] = {static_cast<int>(result.glsl.length())};
                shader.setStringsWithLengths(strings, lengths, 1);
                shader.setEntryPoint("main");
                bool glslang_result = shader.parse(&glslang::DefaultTBuiltInResource, 310,
                                                   EEsProfile, false, false, EShMsgDefault);
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

    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

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
        std::function<void(tint::transform::Manager& manager, tint::transform::DataMap& inputs)>
            make;
    };
    std::vector<TransformFactory> transforms = {
        {"first_index_offset",
         [](tint::transform::Manager& m, tint::transform::DataMap& i) {
             i.Add<tint::transform::FirstIndexOffset::BindingPoint>(0, 0);
             m.Add<tint::transform::FirstIndexOffset>();
         }},
        {"fold_trivial_single_use_lets",
         [](tint::transform::Manager& m, tint::transform::DataMap&) {
             m.Add<tint::transform::FoldTrivialSingleUseLets>();
         }},
        {"renamer", [](tint::transform::Manager& m,
                       tint::transform::DataMap&) { m.Add<tint::transform::Renamer>(); }},
        {"robustness", [](tint::transform::Manager& m,
                          tint::transform::DataMap&) { m.Add<tint::transform::Robustness>(); }},
    };
    auto transform_names = [&] {
        std::stringstream names;
        for (auto& t : transforms) {
            names << "   " << t.name << std::endl;
        }
        return names.str();
    };

    if (options.show_help) {
        std::string usage = tint::utils::ReplaceAll(kUsage, "${transforms}", transform_names());
        std::cout << usage << std::endl;
        return 0;
    }

    // Implement output format defaults.
    if (options.format == Format::kNone) {
        // Try inferring from filename.
        options.format = infer_format(options.output_file);
    }
    if (options.format == Format::kNone) {
        // Ultimately, default to SPIR-V assembly. That's nice for interactive use.
        options.format = Format::kSpvAsm;
    }

    auto diag_printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter diag_formatter;

    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    enum class InputFormat {
        kUnknown,
        kWgsl,
        kSpirvBin,
        kSpirvAsm,
    };
    auto input_format = InputFormat::kUnknown;

    if (options.input_filename.size() > 5 &&
        options.input_filename.substr(options.input_filename.size() - 5) == ".wgsl") {
        input_format = InputFormat::kWgsl;
    } else if (options.input_filename.size() > 4 &&
               options.input_filename.substr(options.input_filename.size() - 4) == ".spv") {
        input_format = InputFormat::kSpirvBin;
    } else if (options.input_filename.size() > 7 &&
               options.input_filename.substr(options.input_filename.size() - 7) == ".spvasm") {
        input_format = InputFormat::kSpirvAsm;
    }

    switch (input_format) {
        case InputFormat::kUnknown: {
            std::cerr << "Unknown input format" << std::endl;
            return 1;
        }
        case InputFormat::kWgsl: {
#if TINT_BUILD_WGSL_READER
            std::vector<uint8_t> data;
            if (!ReadFile<uint8_t>(options.input_filename, &data)) {
                return 1;
            }
            source_file = std::make_unique<tint::Source::File>(
                options.input_filename, std::string(data.begin(), data.end()));
            program = std::make_unique<tint::Program>(tint::reader::wgsl::Parse(source_file.get()));
            break;
#else
            std::cerr << "Tint not built with the WGSL reader enabled" << std::endl;
            return 1;
#endif  // TINT_BUILD_WGSL_READER
        }
        case InputFormat::kSpirvBin: {
#if TINT_BUILD_SPV_READER
            std::vector<uint32_t> data;
            if (!ReadFile<uint32_t>(options.input_filename, &data)) {
                return 1;
            }
            program = std::make_unique<tint::Program>(tint::reader::spirv::Parse(data));
            break;
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            return 1;
#endif  // TINT_BUILD_SPV_READER
        }
        case InputFormat::kSpirvAsm: {
#if TINT_BUILD_SPV_READER
            std::vector<char> text;
            if (!ReadFile<char>(options.input_filename, &text)) {
                return 1;
            }
            // Use Vulkan 1.1, since this is what Tint, internally, is expecting.
            spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
            tools.SetMessageConsumer([](spv_message_level_t, const char*, const spv_position_t& pos,
                                        const char* msg) {
                std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << std::endl;
            });
            std::vector<uint32_t> data;
            if (!tools.Assemble(text.data(), text.size(), &data,
                                SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS)) {
                return 1;
            }
            program = std::make_unique<tint::Program>(tint::reader::spirv::Parse(data));
            break;
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            return 1;
#endif  // TINT_BUILD_SPV_READER
        }
    }

    if (!program) {
        std::cerr << "Failed to parse input file: " << options.input_filename << std::endl;
        return 1;
    }
    if (program->Diagnostics().count() > 0) {
        if (!program->IsValid() && input_format != InputFormat::kWgsl) {
            // Invalid program from a non-wgsl source. Print the WGSL, to help
            // understand the diagnostics.
            PrintWGSL(std::cout, *program);
        }
        diag_formatter.format(program->Diagnostics(), diag_printer.get());
    }

    if (!program->IsValid()) {
        return 1;
    }
    if (options.parse_only) {
        return 1;
    }

    tint::transform::Manager transform_manager;
    tint::transform::DataMap transform_inputs;
    for (const auto& name : options.transforms) {
        // TODO(dsinclair): The vertex pulling transform requires setup code to
        // be run that needs user input. Should we find a way to support that here
        // maybe through a provided file?

        bool found = false;
        for (auto& t : transforms) {
            if (t.name == name) {
                t.make(transform_manager, transform_inputs);
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "Unknown transform: " << name << std::endl;
            std::cerr << "Available transforms: " << std::endl << transform_names();
            return 1;
        }
    }

    if (options.emit_single_entry_point) {
        transform_manager.append(std::make_unique<tint::transform::SingleEntryPoint>());
        transform_inputs.Add<tint::transform::SingleEntryPoint::Config>(options.ep_name);
    }

    switch (options.format) {
        case Format::kMsl: {
#if TINT_BUILD_MSL_WRITER
            transform_inputs.Add<tint::transform::Renamer::Config>(
                tint::transform::Renamer::Target::kMslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::transform::Renamer>();
#endif  // TINT_BUILD_MSL_WRITER
            break;
        }
#if TINT_BUILD_GLSL_WRITER
        case Format::kGlsl: {
            break;
        }
#endif  // TINT_BUILD_GLSL_WRITER
        case Format::kHlsl: {
#if TINT_BUILD_HLSL_WRITER
            transform_inputs.Add<tint::transform::Renamer::Config>(
                tint::transform::Renamer::Target::kHlslKeywords,
                /* preserve_unicode */ false);
            transform_manager.Add<tint::transform::Renamer>();
#endif  // TINT_BUILD_HLSL_WRITER
            break;
        }
        default:
            break;
    }

    auto out = transform_manager.Run(program.get(), std::move(transform_inputs));
    if (!out.program.IsValid()) {
        PrintWGSL(std::cerr, out.program);
        diag_formatter.format(out.program.Diagnostics(), diag_printer.get());
        return 1;
    }

    *program = std::move(out.program);

    if (options.dump_inspector_bindings) {
        std::cout << std::string(80, '-') << std::endl;
        tint::inspector::Inspector inspector(program.get());
        auto entry_points = inspector.GetEntryPoints();
        if (!inspector.error().empty()) {
            std::cerr << "Failed to get entry points from Inspector: " << inspector.error()
                      << std::endl;
            return 1;
        }

        for (auto& entry_point : entry_points) {
            auto bindings = inspector.GetResourceBindings(entry_point.name);
            if (!inspector.error().empty()) {
                std::cerr << "Failed to get bindings from Inspector: " << inspector.error()
                          << std::endl;
                return 1;
            }
            std::cout << "Entry Point = " << entry_point.name << std::endl;
            for (auto& binding : bindings) {
                std::cout << "\t[" << binding.bind_group << "][" << binding.binding
                          << "]:" << std::endl;
                std::cout << "\t\t resource_type = " << ResourceTypeToString(binding.resource_type)
                          << std::endl;
                std::cout << "\t\t dim = " << TextureDimensionToString(binding.dim) << std::endl;
                std::cout << "\t\t sampled_kind = " << SampledKindToString(binding.sampled_kind)
                          << std::endl;
                std::cout << "\t\t image_format = " << TexelFormatToString(binding.image_format)
                          << std::endl;
            }
        }
        std::cout << std::string(80, '-') << std::endl;
    }

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
        default:
            std::cerr << "Unknown output format specified" << std::endl;
            return 1;
    }
    if (!success) {
        return 1;
    }

    return 0;
}
