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

#include <charconv>
#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/text/color_mode.h"

#if TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER || TINT_BUILD_SPV_WRITER

#include "src/tint/api/tint.h"
#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/transform/first_index_offset.h"
#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/renamer.h"
#include "src/tint/lang/wgsl/ast/transform/single_entry_point.h"
#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/utils/cli/cli.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/diagnostic/formatter.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/styled_text_printer.h"

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"

#if TINT_BUILD_IR_BINARY
#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"
#endif  // TINT_BUILD_IR_BINARY

#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "src/tint/lang/spirv/writer/helpers/generate_bindings.h"
#include "src/tint/lang/spirv/writer/writer.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/tint/lang/msl/validate/validate.h"
#include "src/tint/lang/msl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/msl/writer/writer.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/tint/lang/hlsl/validate/validate.h"
#include "src/tint/lang/hlsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
#include "src/tint/lang/glsl/writer/helpers/generate_bindings.h"
#include "src/tint/lang/glsl/writer/writer.h"
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_GLSL_VALIDATOR
#include "src/tint/lang/glsl/validate/validate.h"
#endif  // TINT_BUILD_GLSL_VALIDATOR

#if TINT_BUILD_WGSL_READER
#define WGSL_READER_ONLY(x) x
#else
#define WGSL_READER_ONLY(x)
#endif

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
[[maybe_unused]] void PrintHash(uint32_t hash) {
    std::cout << "<<HASH: 0x" << std::hex << hash << ">>\n";
}

enum class Format : uint8_t {
    kUnknown,
    kNone,
    kSpirv,
    kSpvAsm,
    kWgsl,
    kMsl,
    kHlsl,
    kHlslFxc,
    kGlsl,
    kIr,
};

#if TINT_BUILD_HLSL_WRITER
constexpr uint32_t kMinShaderModelForDXC = 60u;
constexpr uint32_t kMaxSupportedShaderModelForDXC = 66u;
constexpr uint32_t kMinShaderModelForDP4aInHLSL = 64u;
constexpr uint32_t kMinShaderModelForPackUnpack4x8InHLSL = 66u;
#endif  // TINT_BUILD_HLSL_WRITER

struct Options {
    std::unique_ptr<tint::StyledTextPrinter> printer;

    std::string input_filename;
    std::string output_file = "-";  // Default to stdout

    std::unordered_set<uint32_t> skip_hash;
    tint::Vector<std::string, 4> transforms;
    tint::Hashmap<std::string, double, 8> overrides;

    std::string ep_name;

    Format format = Format::kUnknown;

    bool verbose = false;
    bool parse_only = false;
    bool disable_workgroup_init = false;
    bool validate = false;
    bool print_hash = false;
    bool dump_inspector_bindings = false;
    bool enable_robustness = false;
    bool emit_single_entry_point = false;

    bool rename_all = false;

    bool dump_ir = false;
    bool use_ir = false;
    bool use_ir_reader = false;

#if TINT_BUILD_SYNTAX_TREE_WRITER
    bool dump_ast = false;
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

#if TINT_BUILD_SPV_READER
    tint::spirv::reader::Options spirv_reader_options;
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_SPV_WRITER
    bool use_storage_input_output_16 = true;
#endif  // TINT_BULD_SPV_WRITER

#if TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER
    std::unordered_map<uint32_t, uint32_t> pixel_local_attachments;
#endif

#if TINT_BUILD_HLSL_WRITER
    std::string fxc_path;
    std::string dxc_path;

    std::optional<tint::BindingPoint> hlsl_root_constant_binding_point;
    uint32_t hlsl_shader_model = kMinShaderModelForDXC;

    tint::hlsl::writer::PixelLocalOptions pixel_local_options;
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    bool glsl_desktop = false;
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_MSL_WRITER
    std::string xcrun_path;
#endif  // TINT_BULD_MSL_WRITER
};

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
Format InferFormat(const std::string& filename) {
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
    HLSL_WRITER_ONLY(format_enum_names.Emplace(Format::kHlslFxc, "hlsl-fxc"));
    GLSL_WRITER_ONLY(format_enum_names.Emplace(Format::kGlsl, "glsl"));
    WGSL_READER_ONLY(format_enum_names.Emplace(Format::kIr, "ir"));

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

    auto& col = options.Add<EnumOption<tint::ColorMode>>(
        "color", "Use colored output",
        tint::Vector{
            EnumName{tint::ColorMode::kPlain, "off"},
            EnumName{tint::ColorMode::kDark, "dark"},
            EnumName{tint::ColorMode::kLight, "light"},
        },
        ShortName{"col"}, Default{tint::ColorModeDefault()});
    TINT_DEFER(opts->printer = CreatePrinter(*col.value));

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

#if TINT_BUILD_HLSL_WRITER
    auto& fxc_path =
        options.Add<StringOption>("fxc", R"(Path to FXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation)",
                                  Parameter{"path"});
    TINT_DEFER(opts->fxc_path = fxc_path.value.value_or(""));

    auto& dxc_path =
        options.Add<StringOption>("dxc", R"(Path to DXC dll, used to validate HLSL output.
When specified, automatically enables HLSL validation)",
                                  Parameter{"path"});
    TINT_DEFER(opts->dxc_path = dxc_path.value.value_or(""));
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
    auto& glsl_desktop = options.Add<BoolOption>(
        "glsl-desktop", "Set the version to the desktop GL instead of ES", Default{false});
    TINT_DEFER(opts->glsl_desktop = *glsl_desktop.value);
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_MSL_WRITER
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
#endif  // TINT_BUILD_MSL_WRITER

    auto& dump_ir = options.Add<BoolOption>("dump-ir", "Writes the IR to stdout", Alias{"emit-ir"},
                                            Default{false});
    TINT_DEFER(opts->dump_ir = *dump_ir.value);

    auto& use_ir = options.Add<BoolOption>(
        "use-ir", "Use the IR for writers and transforms when possible", Default{false});
    TINT_DEFER(opts->use_ir = *use_ir.value);

    auto& use_ir_reader = options.Add<BoolOption>(
        "use-ir-reader", "Use the IR for the SPIR-V reader", Default{false});
    TINT_DEFER(opts->use_ir_reader = *use_ir_reader.value);

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

#if TINT_BUILD_SPV_WRITER
    auto& use_storage_input_output_16 =
        options.Add<BoolOption>("use-storage-input-output-16",
                                "Use the StorageInputOutput16 SPIR-V capability", Default{true});
    TINT_DEFER(opts->use_storage_input_output_16 = *use_storage_input_output_16.value);
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

#if TINT_BUILD_HLSL_WRITER
    auto& hlsl_rc_bp = options.Add<StringOption>("hlsl-root-constant-binding-point",
                                                 R"(Binding point for root constant.
Specify the binding point for generated uniform buffer
used for num_workgroups in HLSL. If not specified, then
default to binding 0 of the largest used group plus 1,
or group 0 if no resource bound)");
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER

    auto& pixel_local_attachments =
        options.Add<StringOption>("pixel_local_attachments",
                                  R"(Pixel local storage attachment bindings, comma-separated
Each binding is of the form MEMBER_INDEX=ATTACHMENT_INDEX,
where MEMBER_INDEX is the pixel-local structure member
index and ATTACHMENT_INDEX is the index of the emitted
attachment.
)");

#endif

#if TINT_BUILD_HLSL_WRITER
    auto& pixel_local_attachment_formats =
        options.Add<StringOption>("pixel_local_attachment_formats",
                                  R"(Pixel local storage attachment formats, comma-separated
Each binding is of the form MEMBER_INDEX=ATTACHMENT_FORMAT,
where MEMBER_INDEX is the pixel-local structure member
index and ATTACHMENT_FORMAT is the format of the emitted
attachment, which can only be one of the below value:
R32Sint, R32Uint, R32Float.
)");

    auto& pixel_local_group_index = options.Add<ValueOption<uint32_t>>(
        "pixel_local_group_index", "The bind group index of the pixel local attachments.",
        Default{0});

    std::stringstream hlslShaderModelStream;
    hlslShaderModelStream << R"(
An integer value to set the HLSL shader model for the generated HLSL
shader, which will only be used with option `--dxc`. Now only integers
in the range [)" << kMinShaderModelForDXC
                          << ", " << kMaxSupportedShaderModelForDXC
                          << "] are accepted. The integer \"6x\" represents shader model 6.x.";
    auto& hlsl_shader_model = options.Add<ValueOption<uint32_t>>(
        "hlsl_shader_model", hlslShaderModelStream.str(), Default{kMinShaderModelForDXC});
#endif  // TINT_BUILD_HLSL_WRITER

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

    if (overrides.value.has_value()) {
        for (const auto& o : tint::Split(*overrides.value, ",")) {
            auto parts = tint::Split(o, "=");
            if (parts.Length() != 2) {
                std::cerr << "override values must be of the form IDENTIFIER=VALUE";
                return false;
            }
            auto value = tint::strconv::ParseNumber<double>(parts[1]);
            if (value != tint::Success) {
                std::cerr << "invalid override value: " << parts[1];
                return false;
            }
            opts->overrides.Add(std::string(parts[0]), value.Get());
        }
    }

#if TINT_BUILD_HLSL_WRITER
    if (hlsl_rc_bp.value.has_value()) {
        auto binding_points = tint::Split(*hlsl_rc_bp.value, ",");
        if (binding_points.Length() != 2) {
            std::cerr << "Invalid binding point for " << hlsl_rc_bp.name << ": "
                      << *hlsl_rc_bp.value << "\n";
            return false;
        }
        auto group = tint::strconv::ParseUint32(binding_points[0]);
        if (group != tint::Success) {
            std::cerr << "Invalid group for " << hlsl_rc_bp.name << ": " << binding_points[0]
                      << "\n";
            return false;
        }
        auto binding = tint::strconv::ParseUint32(binding_points[1]);
        if (binding != tint::Success) {
            std::cerr << "Invalid binding for " << hlsl_rc_bp.name << ": " << binding_points[1]
                      << "\n";
            return false;
        }
        opts->hlsl_root_constant_binding_point = tint::BindingPoint{group.Get(), binding.Get()};
    }

    if (pixel_local_group_index.value.has_value()) {
        opts->pixel_local_options.group_index = *pixel_local_group_index.value;
    }

    if (pixel_local_attachment_formats.value.has_value()) {
        auto binding_formats = tint::Split(*pixel_local_attachment_formats.value, ",");
        for (auto& binding_format : binding_formats) {
            auto values = tint::Split(binding_format, "=");
            if (values.Length() != 2) {
                std::cerr << "Invalid binding format " << pixel_local_attachment_formats.name
                          << ": " << binding_format << "\n";
                return false;
            }
            auto member_index = tint::strconv::ParseUint32(values[0]);
            if (member_index != tint::Success) {
                std::cerr << "Invalid member index for " << pixel_local_attachment_formats.name
                          << ": " << values[0] << "\n";
                return false;
            }
            auto format = values[1];
            tint::hlsl::writer::PixelLocalOptions::TexelFormat texel_format =
                tint::hlsl::writer::PixelLocalOptions::TexelFormat::kUndefined;
            if (format == "R32Sint") {
                texel_format = tint::hlsl::writer::PixelLocalOptions::TexelFormat::kR32Sint;
            } else if (format == "R32Uint") {
                texel_format = tint::hlsl::writer::PixelLocalOptions::TexelFormat::kR32Uint;
            } else if (format == "R32Float") {
                texel_format = tint::hlsl::writer::PixelLocalOptions::TexelFormat::kR32Float;
            } else {
                std::cerr << "Invalid texel format for " << pixel_local_attachments.name << ": "
                          << format << "\n";
                return false;
            }
            opts->pixel_local_options.attachment_formats.emplace(member_index.Get(), texel_format);
        }
    }

    if (hlsl_shader_model.value.has_value()) {
        const uint32_t shader_model = *hlsl_shader_model.value;
        if (shader_model < kMinShaderModelForDXC || shader_model > kMaxSupportedShaderModelForDXC) {
            std::cerr << "Invalid HLSL shader model: " << shader_model << "\n";
            return false;
        }
        opts->hlsl_shader_model = shader_model;
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_HLSL_WRITER || TINT_BUILD_MSL_WRITER
    if (pixel_local_attachments.value.has_value()) {
        auto bindings = tint::Split(*pixel_local_attachments.value, ",");
        for (auto& binding : bindings) {
            auto values = tint::Split(binding, "=");
            if (values.Length() != 2) {
                std::cerr << "Invalid binding " << pixel_local_attachments.name << ": " << binding
                          << "\n";
                return false;
            }
            auto member_index = tint::strconv::ParseUint32(values[0]);
            if (member_index != tint::Success) {
                std::cerr << "Invalid member index for " << pixel_local_attachments.name << ": "
                          << values[0] << "\n";
                return false;
            }
            auto attachment_index = tint::strconv::ParseUint32(values[1]);
            if (attachment_index != tint::Success) {
                std::cerr << "Invalid attachment index for " << pixel_local_attachments.name << ": "
                          << values[1] << "\n";
                return false;
            }
            opts->pixel_local_attachments.emplace(member_index.Get(), attachment_index.Get());
        }
    }
#if TINT_BUILD_HLSL_WRITER
    opts->pixel_local_options.attachments = opts->pixel_local_attachments;
#endif

#endif

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

#if TINT_BUILD_SPV_WRITER
std::string Disassemble(const std::vector<uint32_t>& data) {
    std::string spv_errors;
    spv_target_env target_env = SPV_ENV_VULKAN_1_1;

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
        std::cerr << spv_errors << "\n";
    }
    return result;
}
#endif  // TINT_BUILD_SPV_WRITER

/// Generate SPIR-V code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateSpirv(const tint::Program& program, const Options& options) {
#if TINT_BUILD_SPV_WRITER
    // Convert the AST program to an IR module.
    auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (ir != tint::Success) {
        std::cerr << "Failed to generate IR: " << ir << "\n";
        return false;
    }

    tint::spirv::writer::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.use_storage_input_output_16 = options.use_storage_input_output_16;
    gen_options.bindings = tint::spirv::writer::GenerateBindings(ir.Get());

    // Generate SPIR-V from Tint IR.
    auto result = tint::spirv::writer::Generate(ir.Get(), gen_options);
    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate SPIR-V: " << result.Failure() << "\n";
        return false;
    }

    if (options.format == Format::kSpvAsm) {
        if (!tint::cmd::WriteFile(options.output_file, "w", Disassemble(result.Get().spirv))) {
            return false;
        }
    } else {
        if (!tint::cmd::WriteFile(options.output_file, "wb", result.Get().spirv)) {
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
                std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << "\n";
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
    std::cerr << "SPIR-V writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_SPV_WRITER
}

/// Generate WGSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateWgsl([[maybe_unused]] const tint::Program& program,
                  [[maybe_unused]] const Options& options) {
#if TINT_BUILD_WGSL_WRITER
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::wgsl::writer::Options gen_options;
    auto result = tint::wgsl::writer::Generate(program, gen_options);
    if (result != tint::Success) {
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->wgsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->wgsl.data(), result->wgsl.size());
    if (options.print_hash) {
        PrintHash(hash);
    }

#if TINT_BUILD_WGSL_READER
    if (options.validate && options.skip_hash.count(hash) == 0) {
        // Attempt to re-parse the output program with Tint's WGSL reader.
        tint::wgsl::reader::Options parser_options;
        parser_options.allowed_features = tint::wgsl::AllowedFeatures::Everything();
        auto source = std::make_unique<tint::Source::File>(options.input_filename, result->wgsl);
        auto reparsed_program = tint::wgsl::reader::Parse(source.get(), parser_options);
        if (!reparsed_program.IsValid()) {
            tint::diag::Formatter diag_formatter;
            options.printer->Print(diag_formatter.Format(reparsed_program.Diagnostics()));
            return false;
        }
    }
#endif  // TINT_BUILD_WGSL_READER

    return true;
#else
    std::cerr << "WGSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_WGSL_WRITER
}

/// Generate MSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateMsl([[maybe_unused]] const tint::Program& program,
                 [[maybe_unused]] const Options& options) {
#if !TINT_BUILD_MSL_WRITER
    std::cerr << "MSL writer not enabled in tint build\n";
    return false;
#else
    // Remap resource numbers to a flat namespace.
    // TODO(crbug.com/tint/1501): Do this via Options::BindingMap.
    const tint::Program* input_program = &program;
    auto flattened = tint::wgsl::FlattenBindings(program);
    if (flattened) {
        input_program = &*flattened;
    }

    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::msl::writer::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.pixel_local_attachments = options.pixel_local_attachments;
    gen_options.bindings = tint::msl::writer::GenerateBindings(*input_program);
    gen_options.array_length_from_uniform.ubo_binding = 30;

    // Add array_length_from_uniform entries for all storage buffers with runtime sized arrays.
    std::unordered_set<tint::BindingPoint> storage_bindings;
    for (auto* var : program.AST().GlobalVariables()) {
        auto* sem_var = program.Sem().Get<tint::sem::GlobalVariable>(var);
        if (!sem_var->Type()->UnwrapRef()->HasFixedFootprint()) {
            auto bp = sem_var->Attributes().binding_point.value();
            if (storage_bindings.insert(bp).second) {
                gen_options.array_length_from_uniform.bindpoint_to_size_index.emplace(
                    bp, static_cast<uint32_t>(storage_bindings.size() - 1));
            }
        }
    }

    tint::Result<tint::msl::writer::Output> result;
    if (options.use_ir) {
        // Convert the AST program to an IR module.
        auto ir = tint::wgsl::reader::ProgramToLoweredIR(*input_program);
        if (ir != tint::Success) {
            std::cerr << "Failed to generate IR: " << ir << "\n";
            return false;
        }
        result = tint::msl::writer::Generate(ir.Get(), gen_options);
    } else {
        result = tint::msl::writer::Generate(*input_program, gen_options);
    }

    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->msl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->msl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    // Default to validating against MSL 1.2.
    // If subgroups are used, bump the version to 2.2.
    auto msl_version = tint::msl::validate::MslVersion::kMsl_1_2;
    for (auto* enable : program.AST().Enables()) {
        if (enable->HasExtension(tint::wgsl::Extension::kChromiumExperimentalSubgroups) ||
            enable->HasExtension(tint::wgsl::Extension::kSubgroups)) {
            msl_version = std::max(msl_version, tint::msl::validate::MslVersion::kMsl_2_2);
        }
        if (enable->HasExtension(tint::wgsl::Extension::kChromiumExperimentalPixelLocal) ||
            enable->HasExtension(tint::wgsl::Extension::kChromiumExperimentalFramebufferFetch)) {
            msl_version = std::max(msl_version, tint::msl::validate::MslVersion::kMsl_2_3);
        }
    }

    if (options.validate && options.skip_hash.count(hash) == 0) {
        tint::msl::validate::Result res;
#if TINT_BUILD_IS_MAC
        res = tint::msl::validate::ValidateUsingMetal(result->msl, msl_version);
#else
#ifdef _WIN32
        const char* default_xcrun_exe = "metal.exe";
#else
        const char* default_xcrun_exe = "xcrun";
#endif
        auto xcrun = tint::Command::LookPath(
            options.xcrun_path.empty() ? default_xcrun_exe : std::string(options.xcrun_path));
        if (xcrun.Found()) {
            res = tint::msl::validate::Validate(xcrun.Path(), result->msl, msl_version);
        } else {
            res.output = "xcrun executable not found. Cannot validate.";
            res.failed = true;
        }
#endif  // TINT_BUILD_IS_MAC
        if (res.failed) {
            std::cerr << res.output << "\n";
            return false;
        }
    }

    return true;
#endif  // TINT_BUILD_MSL_WRITER
}

/// Generate HLSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateHlsl(const tint::Program& program, const Options& options) {
#if TINT_BUILD_HLSL_WRITER
    const bool for_fxc = options.format == Format::kHlslFxc;
    // TODO(jrprice): Provide a way for the user to set non-default options.
    tint::hlsl::writer::Options gen_options;
    gen_options.disable_robustness = !options.enable_robustness;
    gen_options.disable_workgroup_init = options.disable_workgroup_init;
    gen_options.bindings = tint::hlsl::writer::GenerateBindings(program);
    gen_options.root_constant_binding_point = options.hlsl_root_constant_binding_point;
    gen_options.pixel_local = options.pixel_local_options;
    gen_options.polyfill_dot_4x8_packed = options.hlsl_shader_model < kMinShaderModelForDP4aInHLSL;
    gen_options.polyfill_pack_unpack_4x8 =
        options.hlsl_shader_model < kMinShaderModelForPackUnpack4x8InHLSL;
    gen_options.compiler = for_fxc ? tint::hlsl::writer::Options::Compiler::kFXC
                                   : tint::hlsl::writer::Options::Compiler::kDXC;

    tint::Result<tint::hlsl::writer::Output> result;
    if (options.use_ir) {
        // Convert the AST program to an IR module.
        auto ir = tint::wgsl::reader::ProgramToLoweredIR(program);
        if (ir != tint::Success) {
            std::cerr << "Failed to generate IR: " << ir << "\n";
            return false;
        }
        result = tint::hlsl::writer::Generate(ir.Get(), gen_options);
    } else {
        result = tint::hlsl::writer::Generate(program, gen_options);
    }

    if (result != tint::Success) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << "Failed to generate: " << result.Failure() << "\n";
        return false;
    }

    if (!tint::cmd::WriteFile(options.output_file, "w", result->hlsl)) {
        return false;
    }

    const auto hash = tint::CRC32(result->hlsl.c_str());
    if (options.print_hash) {
        PrintHash(hash);
    }

    const bool validate =
        (options.validate || !options.fxc_path.empty() || !options.dxc_path.empty()) &&
        (options.skip_hash.count(hash) == 0);

    if (validate && !for_fxc) {
        // DXC validation
        tint::hlsl::validate::Result dxc_res;
        const std::string dxc_path =
            options.dxc_path.empty() ? tint::hlsl::validate::kDxcDLLName : options.dxc_path;
        auto dxc = tint::Command::LookPath(dxc_path);
        if (dxc.Found()) {
            uint32_t hlsl_shader_model = options.hlsl_shader_model;
            auto enable_list = program.AST().Enables();
            bool dxc_require_16bit_types = false;
            for (auto* enable : enable_list) {
                if (enable->HasExtension(tint::wgsl::Extension::kF16)) {
                    dxc_require_16bit_types = true;
                    break;
                }
            }
            if (options.verbose) {
                std::cout << "Validating with DXC: " << dxc.Path() << "\n";
            }
            dxc_res = tint::hlsl::validate::ValidateUsingDXC(
                dxc.Path(), result->hlsl, result->entry_points, dxc_require_16bit_types,
                hlsl_shader_model);
        } else {
            dxc_res.failed = true;
            dxc_res.output = "DXC executable '" + dxc_path + "' not found. Cannot validate.";
        }

        if (dxc_res.failed) {
            std::cerr << "DXC validation failure:\n" << dxc_res.output << "\n";
            return false;
        }
        if (options.verbose) {
            std::cout << "Passed DXC validation. Compiler output:\n" << dxc_res.output << "\n";
        }
    }

    if (validate && for_fxc) {
        // FXC validation
#ifndef _WIN32
        std::cerr << "FXC can only be used on Windows.\n";
        return false;
#else
        tint::hlsl::validate::Result fxc_res;
        auto fxc = tint::Command::LookPath(
            options.fxc_path.empty() ? tint::hlsl::validate::kFxcDLLName : options.fxc_path);
        if (fxc.Found()) {
            if (options.verbose) {
                std::cout << "Validating with FXC: " << fxc.Path() << "\n";
            }
            fxc_res = tint::hlsl::validate::ValidateUsingFXC(fxc.Path(), result->hlsl,
                                                             result->entry_points);
        } else {
            fxc_res.failed = true;
            fxc_res.output = "FXC DLL '" + options.fxc_path + "' not found. Cannot validate.";
        }

        if (fxc_res.failed) {
            std::cerr << "FXC validation failure:\n" << fxc_res.output << "\n";
            return false;
        }
        if (options.verbose) {
            std::cout << "Passed FXC validation. Compiler output:\n" << fxc_res.output << "\n";
        }
#endif  // _WIN32
    }

    return true;
#else
    (void)program;
    (void)options;
    std::cerr << "HLSL writer not enabled in tint build\n";
    return false;
#endif  // TINT_BUILD_HLSL_WRITER
}

/// Generate GLSL code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool GenerateGlsl([[maybe_unused]] const tint::Program& program,
                  [[maybe_unused]] const Options& options) {
#if !TINT_BUILD_GLSL_WRITER
    std::cerr << "GLSL writer not enabled in tint build\n";
    return false;
#else
    tint::inspector::Inspector inspector(program);

    auto generate = [&](const tint::Program& prg, const std::string entry_point_name,
                        [[maybe_unused]] tint::ast::PipelineStage stage) -> bool {
        // The GLSL backend assumes single entry point
        tint::ast::transform::Manager transform_manager;
        tint::ast::transform::DataMap transform_inputs;

        if (options.use_ir && !entry_point_name.empty()) {
            transform_manager.append(std::make_unique<tint::ast::transform::SingleEntryPoint>());
            transform_inputs.Add<tint::ast::transform::SingleEntryPoint::Config>(entry_point_name);
        }

        tint::ast::transform::DataMap outputs;
        auto single_prog = transform_manager.Run(prg, std::move(transform_inputs), outputs);
        if (!single_prog.IsValid()) {
            tint::cmd::PrintWGSL(std::cerr, single_prog);
            std::cerr << single_prog.Diagnostics() << "\n";
            return 1;
        }

        tint::glsl::writer::Options gen_options;

        if (options.glsl_desktop) {
            gen_options.version =
                tint::glsl::writer::Version(tint::glsl::writer::Version::Standard::kDesktop, 4, 6);
        } else {
            gen_options.version = tint::glsl::writer::Version();
        }

        gen_options.disable_robustness = !options.enable_robustness;
        gen_options.bindings = tint::glsl::writer::GenerateBindings(program);

        constexpr uint32_t kMaxBindGroups = 4u;
        gen_options.bindings.texture_builtins_from_uniform.ubo_binding = {kMaxBindGroups, 0u};

        auto textureBuiltinsFromUniformData = inspector.GetTextureQueries(entry_point_name);
        if (!textureBuiltinsFromUniformData.empty()) {
            for (size_t i = 0; i < textureBuiltinsFromUniformData.size(); ++i) {
                const auto& info = textureBuiltinsFromUniformData[i];

                // This is the unmodified binding point from the WGSL shader.
                tint::BindingPoint srcBindingPoint{info.group, info.binding};
                gen_options.bindings.texture_builtins_from_uniform.ubo_bindingpoint_ordering
                    .emplace_back(srcBindingPoint);
            }
        }

        auto entry_point = inspector.GetEntryPoint(entry_point_name);
        uint32_t offset = entry_point.push_constant_size;

        if (entry_point.instance_index_used) {
            // Place the first_instance push constant member after user-defined push constants (if
            // any).
            gen_options.first_instance_offset = offset;
            offset += 4;
        }
        if (entry_point.frag_depth_used) {
            gen_options.depth_range_offsets = {offset + 0, offset + 4};
            offset += 8;
        }

        tint::Result<tint::glsl::writer::Output> result;
        if (options.use_ir) {
            // Convert the AST program to an IR module.
            auto ir = tint::wgsl::reader::ProgramToLoweredIR(single_prog);
            if (ir != tint::Success) {
                std::cerr << "Failed to generate IR: " << ir << "\n";
                return false;
            }
            result = tint::glsl::writer::Generate(ir.Get(), gen_options, "");
        } else {
            result = tint::glsl::writer::Generate(single_prog, gen_options, entry_point_name);
        }
        if (result != tint::Success) {
            tint::cmd::PrintWGSL(std::cerr, single_prog);
            std::cerr << "Failed to generate: " << result.Failure() << "\n";
            return false;
        }

        if (!tint::cmd::WriteFile(options.output_file, "w", result->glsl)) {
            return false;
        }

        const auto hash = tint::CRC32(result->glsl.c_str());
        if (options.print_hash) {
            PrintHash(hash);
        }

        if (options.validate && options.skip_hash.count(hash) == 0) {
#if !TINT_BUILD_GLSL_VALIDATOR
            std::cerr << "GLSL validator not enabled in tint build\n";
            return false;
#else
            // If there is no entry point name there is nothing to validate
            if (entry_point_name != "") {
                auto val = tint::glsl::validate::Validate(result->glsl, stage);
                if (val != tint::Success) {
                    std::cerr << val.Failure();
                    return false;
                }
            }
#endif
        }
        return true;
    };

    if (inspector.GetEntryPoints().empty()) {
        // Pass empty string here so that the GLSL generator will generate
        // code for all functions, reachable or not.
        return generate(program, "", tint::ast::PipelineStage::kCompute);
    }

    bool success = true;
    for (auto& entry_point : inspector.GetEntryPoints()) {
        tint::ast::PipelineStage stage = tint::ast::PipelineStage::kCompute;
        switch (entry_point.stage) {
            case tint::inspector::PipelineStage::kCompute:
                stage = tint::ast::PipelineStage::kCompute;
                break;
            case tint::inspector::PipelineStage::kVertex:
                stage = tint::ast::PipelineStage::kVertex;
                break;
            case tint::inspector::PipelineStage::kFragment:
                stage = tint::ast::PipelineStage::kFragment;
                break;
        }
        success &= generate(program, entry_point.name, stage);
    }
    return success;
#endif  // TINT_BUILD_GLSL_WRITER
}

/// Generate IR code for a program.
/// @param program the program to generate
/// @param options the options that Tint was invoked with
/// @returns true on success
bool DumpIR([[maybe_unused]] const tint::Program& program,
            [[maybe_unused]] const Options& options) {
#if !TINT_BUILD_WGSL_READER
    std::cerr << "WGSL reader not enabled in tint build\n";
    return false;
#else
    auto result = tint::wgsl::reader::ProgramToLoweredIR(program);
    if (result != tint::Success) {
        std::cerr << "Failed to build IR from program: " << result.Failure() << "\n";
        return false;
    }

    options.printer->Print(tint::core::ir::Disassembler(result.Get()).Text());
    options.printer->Print(tint::StyledText{} << "\n");

    return true;
#endif
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

             for (auto& override : options.overrides) {
                 const auto& name = override.key.Value();
                 const auto& value = override.value;
                 if (name.empty()) {
                     std::cerr << "empty override name\n";
                     return false;
                 }
                 if (auto num = tint::strconv::ParseNumber<decltype(tint::OverrideId::value)>(name);
                     num == tint::Success) {
                     tint::OverrideId id{num.Get()};
                     values.emplace(id, value);
                 } else {
                     auto override_names = inspector.GetNamedOverrideIds();
                     auto it = override_names.find(name);
                     if (it == override_names.end()) {
                         std::cerr << "unknown override '" << name << "'\n";
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
            names << "   " << t.name << "\n";
        }
        return names.str();
    };

    if (!ParseArgs(arguments, transform_names(), &options)) {
        return 1;
    }

    // Implement output format defaults.
    if (options.format == Format::kUnknown) {
        // Try inferring from filename.
        options.format = InferFormat(options.output_file);
    }
    if (options.format == Format::kUnknown) {
        // Ultimately, default to SPIR-V assembly. That's nice for interactive use.
        options.format = Format::kSpvAsm;
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
    opts.printer = options.printer.get();
#if TINT_BUILD_SPV_READER
    opts.use_ir = options.use_ir_reader;
    opts.spirv_reader_options = options.spirv_reader_options;
#endif

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.parse_only) {
        return 1;
    }

#if TINT_BUILD_SYNTAX_TREE_WRITER
    if (options.dump_ast) {
        tint::wgsl::writer::Options gen_options;
        gen_options.use_syntax_tree_writer = true;
        auto result = tint::wgsl::writer::Generate(info.program, gen_options);
        if (result != tint::Success) {
            std::cerr << "Failed to dump AST: " << result.Failure() << "\n";
        } else {
            std::cout << result->wgsl << "\n";
        }
    }
#endif  // TINT_BUILD_SYNTAX_TREE_WRITER

#if TINT_BUILD_WGSL_READER
    if (options.dump_ir) {
        DumpIR(info.program, options);
    }
#endif  // TINT_BUILD_WGSL_READER

    tint::inspector::Inspector inspector(info.program);
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
        case Format::kHlsl:
        case Format::kHlslFxc: {
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

        std::cerr << "Unknown transform: " << name << "\n";
        std::cerr << "Available transforms: \n" << transform_names() << "\n";
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
    auto program = transform_manager.Run(info.program, std::move(transform_inputs), outputs);
    if (!program.IsValid()) {
        tint::cmd::PrintWGSL(std::cerr, program);
        std::cerr << program.Diagnostics() << "\n";
        return 1;
    }

    bool success = false;
    switch (options.format) {
        case Format::kSpirv:
        case Format::kSpvAsm:
            success = GenerateSpirv(program, options);
            break;
        case Format::kWgsl:
            success = GenerateWgsl(program, options);
            break;
        case Format::kMsl:
            success = GenerateMsl(program, options);
            break;
        case Format::kHlsl:
        case Format::kHlslFxc:
            success = GenerateHlsl(program, options);
            break;
        case Format::kGlsl:
            success = GenerateGlsl(program, options);
            break;
        case Format::kIr:
            success = DumpIR(program, options);
            break;
        case Format::kNone:
            break;
        default:
            std::cerr << "Unknown output format specified\n";
            return 1;
    }
    if (!success) {
        return 1;
    }

    return 0;
}
