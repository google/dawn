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

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "tint/tint.h"

namespace {

[[noreturn]] void TintInternalCompilerErrorReporter(
    const tint::diag::List& diagnostics) {
  auto printer = tint::diag::Printer::create(stderr, true);
  tint::diag::Formatter{}.format(diagnostics, printer.get());
  exit(1);
}

enum class Format {
  kNone = -1,
  kSpirv,
  kSpvAsm,
  kWgsl,
  kMsl,
  kHlsl,
};

struct Options {
  bool show_help = false;

  std::string input_filename;
  std::string output_file = "-";  // Default to stdout

  bool parse_only = false;
  bool dump_ast = false;
  bool dawn_validation = false;
  bool demangle = false;
  bool dump_inspector_bindings = false;

  Format format = Format::kNone;

  bool emit_single_entry_point = false;
  std::string ep_name;

  std::vector<std::string> transforms;
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
                                bound_array_accessors
                                first_index_offset
                                renamer
  --parse-only              -- Stop after parsing the input
  --dump-ast                -- Dump the generated AST to stdout
  --dawn-validation         -- SPIRV outputs are validated with the same flags
                               as Dawn does. Has no effect on non-SPIRV outputs.
  --demangle                -- Preserve original source names. Demangle them.
                               Affects AST dumping, and text-based output languages.
  --dump-inspector-bindings -- Dump reflection data about bindins to stdout.
  -h                        -- This help text)";

#ifdef _MSC_VER
#pragma warning(disable : 4068; suppress : 4100)
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
Format parse_format(const std::string& fmt) {
#pragma clang diagnostic pop
#ifdef _MSC_VER
#pragma warning(default : 4068)
#endif

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

  return Format::kNone;
}

/// @param input input string
/// @param suffix potential suffix string
/// @returns true if input ends with the given suffix.
bool ends_with(const std::string& input, const std::string& suffix) {
  const auto input_len = input.size();
  const auto suffix_len = suffix.size();
  // Avoid integer overflow.
  return (input_len >= suffix_len) &&
         (input_len - suffix_len == input.rfind(suffix));
}

/// @param filename the filename to inspect
/// @returns the inferred format for the filename suffix
#ifdef _MSC_VER
#pragma warning(disable : 4068; suppress : 4100)
#endif
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
Format infer_format(const std::string& filename) {
#pragma clang diagnostic pop
#ifdef _MSC_VER
#pragma warning(default : 4068)
#endif

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

std::string TextureDimensionToString(
    tint::inspector::ResourceBinding::TextureDimension dim) {
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

std::string SampledKindToString(
    tint::inspector::ResourceBinding::SampledKind kind) {
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

std::string ImageFormatToString(
    tint::inspector::ResourceBinding::ImageFormat format) {
  switch (format) {
    case tint::inspector::ResourceBinding::ImageFormat::kR8Unorm:
      return "R8Unorm";
    case tint::inspector::ResourceBinding::ImageFormat::kR8Snorm:
      return "R8Snorm";
    case tint::inspector::ResourceBinding::ImageFormat::kR8Uint:
      return "R8Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kR8Sint:
      return "R8Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kR16Uint:
      return "R16Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kR16Sint:
      return "R16Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kR16Float:
      return "R16Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRg8Unorm:
      return "Rg8Unorm";
    case tint::inspector::ResourceBinding::ImageFormat::kRg8Snorm:
      return "Rg8Snorm";
    case tint::inspector::ResourceBinding::ImageFormat::kRg8Uint:
      return "Rg8Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRg8Sint:
      return "Rg8Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kR32Uint:
      return "R32Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kR32Sint:
      return "R32Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kR32Float:
      return "R32Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRg16Uint:
      return "Rg16Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRg16Sint:
      return "Rg16Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kRg16Float:
      return "Rg16Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba8Unorm:
      return "Rgba8Unorm";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba8UnormSrgb:
      return "Rgba8UnormSrgb";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba8Snorm:
      return "Rgba8Snorm";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba8Uint:
      return "Rgba8Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba8Sint:
      return "Rgba8Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kBgra8Unorm:
      return "Bgra8Unorm";
    case tint::inspector::ResourceBinding::ImageFormat::kBgra8UnormSrgb:
      return "Bgra8UnormSrgb";
    case tint::inspector::ResourceBinding::ImageFormat::kRgb10A2Unorm:
      return "Rgb10A2Unorm";
    case tint::inspector::ResourceBinding::ImageFormat::kRg11B10Float:
      return "Rg11B10Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRg32Uint:
      return "Rg32Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRg32Sint:
      return "Rg32Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kRg32Float:
      return "Rg32Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba16Uint:
      return "Rgba16Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba16Sint:
      return "Rgba16Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba16Float:
      return "Rgba16Float";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba32Uint:
      return "Rgba32Uint";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba32Sint:
      return "Rgba32Sint";
    case tint::inspector::ResourceBinding::ImageFormat::kRgba32Float:
      return "Rgba32Float";
    case tint::inspector::ResourceBinding::ImageFormat::kNone:
      return "None";
  }
  return "Unknown";
}

std::string ResourceTypeToString(
    tint::inspector::ResourceBinding::ResourceType type) {
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
    case tint::inspector::ResourceBinding::ResourceType::
        kReadOnlyStorageTexture:
      return "ReadOnlyStorageTexture";
    case tint::inspector::ResourceBinding::ResourceType::
        kWriteOnlyStorageTexture:
      return "WriteOnlyStorageTexture";
    case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
      return "DepthTexture";
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
    } else if (arg == "--dump-ast") {
      opts->dump_ast = true;
    } else if (arg == "--dawn-validation") {
      opts->dawn_validation = true;
    } else if (arg == "--demangle") {
      opts->demangle = true;
    } else if (arg == "--dump-inspector-bindings") {
      opts->dump_inspector_bindings = true;
    } else if (!arg.empty()) {
      if (arg[0] == '-') {
        std::cerr << "Unrecognized option: " << arg << std::endl;
        return false;
      }
      if (!opts->input_filename.empty()) {
        std::cerr << "More than one input file specified: '"
                  << opts->input_filename << "' and '" << arg << "'"
                  << std::endl;
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
  uint64_t tell_file_size = static_cast<uint64_t>(ftell(file));
  if (tell_file_size <= 0) {
    std::cerr << "Input file of incorrect size: " << input_file << std::endl;
    fclose(file);
    return {};
  }
  const auto file_size = static_cast<size_t>(tell_file_size);
  if (0 != (file_size % sizeof(T))) {
    std::cerr << "File " << input_file
              << " does not contain an integral number of objects: "
              << file_size << " bytes in the file, require " << sizeof(T)
              << " bytes per object" << std::endl;
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
bool WriteFile(const std::string& output_file,
               const std::string mode,
               const ContainerT& buffer) {
  const bool use_stdout = output_file.empty() || output_file == "-";
  FILE* file = stdout;

  if (!use_stdout) {
#if defined(_MSC_VER)
    fopen_s(&file, output_file.c_str(), mode.c_str());
#else
    file = fopen(output_file.c_str(), mode.c_str());
#endif
    if (!file) {
      std::cerr << "Could not open file " << output_file << " for writing"
                << std::endl;
      return false;
    }
  }

  size_t written =
      fwrite(buffer.data(), sizeof(typename ContainerT::value_type),
             buffer.size(), file);
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
                                    const spv_position_t& position,
                                    const char* message) {
    switch (level) {
      case SPV_MSG_FATAL:
      case SPV_MSG_INTERNAL_ERROR:
      case SPV_MSG_ERROR:
        spv_errors += "error: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_WARNING:
        spv_errors += "warning: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_INFO:
        spv_errors += "info: line " + std::to_string(position.index) + ": " +
                      message + "\n";
        break;
      case SPV_MSG_DEBUG:
        break;
    }
  };

  spvtools::SpirvTools tools(target_env);
  tools.SetMessageConsumer(msg_consumer);

  std::string result;
  if (!tools.Disassemble(data, &result,
                         SPV_BINARY_TO_TEXT_OPTION_INDENT |
                             SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES)) {
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
  tint::writer::wgsl::Generator writer(&program);
  writer.Generate();
  out << std::endl << writer.result() << std::endl;
#endif
}

}  // namespace

int main(int argc, const char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;

  tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

  if (!ParseArgs(args, &options)) {
    std::cerr << "Failed to parse arguments." << std::endl;
    return 1;
  }

  if (options.show_help) {
    std::cout << kUsage << std::endl;
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
#if TINT_BUILD_WGSL_READER
  if (options.input_filename.size() > 5 &&
      options.input_filename.substr(options.input_filename.size() - 5) ==
          ".wgsl") {
    std::vector<uint8_t> data;
    if (!ReadFile<uint8_t>(options.input_filename, &data)) {
      return 1;
    }
    source_file = std::make_unique<tint::Source::File>(
        options.input_filename, std::string(data.begin(), data.end()));
    program = std::make_unique<tint::Program>(
        tint::reader::wgsl::Parse(source_file.get()));
  }
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_READER
  // Handle SPIR-V binary input, in files ending with .spv
  if (options.input_filename.size() > 4 &&
      options.input_filename.substr(options.input_filename.size() - 4) ==
          ".spv") {
    std::vector<uint32_t> data;
    if (!ReadFile<uint32_t>(options.input_filename, &data)) {
      return 1;
    }
    program = std::make_unique<tint::Program>(tint::reader::spirv::Parse(data));
  }
  // Handle SPIR-V assembly input, in files ending with .spvasm
  if (options.input_filename.size() > 7 &&
      options.input_filename.substr(options.input_filename.size() - 7) ==
          ".spvasm") {
    std::vector<char> text;
    if (!ReadFile<char>(options.input_filename, &text)) {
      return 1;
    }
    // Use Vulkan 1.1, since this is what Tint, internally, is expecting.
    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
    tools.SetMessageConsumer([](spv_message_level_t, const char*,
                                const spv_position_t& pos, const char* msg) {
      std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg
                << std::endl;
    });
    std::vector<uint32_t> data;
    if (!tools.Assemble(text.data(), text.size(), &data,
                        SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS)) {
      return 1;
    }
    program = std::make_unique<tint::Program>(tint::reader::spirv::Parse(data));
  }
#endif  // TINT_BUILD_SPV_READER

  if (!program) {
    std::cerr << "Failed to create reader for input file: "
              << options.input_filename << std::endl;
    return 1;
  }
  if (program->Diagnostics().count() > 0) {
    diag_formatter.format(program->Diagnostics(), diag_printer.get());
  }

  if (options.dump_ast) {
    std::cout << std::endl << program->to_str(options.demangle) << std::endl;
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

    if (name == "bound_array_accessors") {
      transform_manager.Add<tint::transform::BoundArrayAccessors>();
    } else if (name == "first_index_offset") {
      transform_inputs.Add<tint::transform::FirstIndexOffset::BindingPoint>(0,
                                                                            0);
      transform_manager.Add<tint::transform::FirstIndexOffset>();
    } else if (name == "renamer") {
      transform_manager.Add<tint::transform::Renamer>();
    } else {
      std::cerr << "Unknown transform name: " << name << std::endl;
      return 1;
    }
  }

  if (options.emit_single_entry_point) {
    transform_manager.append(
        std::make_unique<tint::transform::SingleEntryPoint>());
    transform_inputs.Add<tint::transform::SingleEntryPoint::Config>(
        options.ep_name);
  }

  switch (options.format) {
#if TINT_BUILD_SPV_WRITER
    case Format::kSpirv:
    case Format::kSpvAsm:
      transform_manager.Add<tint::transform::Spirv>();
      transform_inputs.Add<tint::transform::Spirv::Config>(true);
      break;
#endif  // TINT_BUILD_SPV_WRITER
#if TINT_BUILD_MSL_WRITER
    case Format::kMsl: {
      tint::transform::Renamer::Config renamer_config{
          tint::transform::Renamer::Target::kMslKeywords};
      transform_manager.append(
          std::make_unique<tint::transform::Renamer>(renamer_config));
      transform_manager.Add<tint::transform::Msl>();
      break;
    }
#endif  // TINT_BUILD_MSL_WRITER
#if TINT_BUILD_HLSL_WRITER
    case Format::kHlsl: {
      tint::transform::Renamer::Config renamer_config{
          tint::transform::Renamer::Target::kHlslKeywords};
      transform_manager.append(
          std::make_unique<tint::transform::Renamer>(renamer_config));
      transform_manager.Add<tint::transform::Hlsl>();
      break;
    }
#endif  // TINT_BUILD_HLSL_WRITER
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
      std::cerr << "Failed to get entry points from Inspector: "
                << inspector.error() << std::endl;
      return 1;
    }

    for (auto& entry_point : entry_points) {
      auto bindings = inspector.GetResourceBindings(entry_point.name);
      if (!inspector.error().empty()) {
        std::cerr << "Failed to get bindings from Inspector: "
                  << inspector.error() << std::endl;
        return 1;
      }
      std::cout << "Entry Point = " << entry_point.name << std::endl;
      for (auto& binding : bindings) {
        std::cout << "\t[" << binding.bind_group << "][" << binding.binding
                  << "]:" << std::endl;
        std::cout << "\t\t resource_type = "
                  << ResourceTypeToString(binding.resource_type) << std::endl;
        std::cout << "\t\t dim = " << TextureDimensionToString(binding.dim)
                  << std::endl;
        std::cout << "\t\t sampled_kind = "
                  << SampledKindToString(binding.sampled_kind) << std::endl;
        std::cout << "\t\t image_format = "
                  << ImageFormatToString(binding.image_format) << std::endl;
      }
    }
    std::cout << std::string(80, '-') << std::endl;
  }

  std::unique_ptr<tint::writer::Writer> writer;

#if TINT_BUILD_SPV_WRITER
  if (options.format == Format::kSpirv || options.format == Format::kSpvAsm) {
    writer = std::make_unique<tint::writer::spirv::Generator>(program.get());
  }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
  if (options.format == Format::kWgsl) {
    writer = std::make_unique<tint::writer::wgsl::Generator>(program.get());
  }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
  if (options.format == Format::kMsl) {
    writer = std::make_unique<tint::writer::msl::Generator>(program.get());
  }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
  if (options.format == Format::kHlsl) {
    writer = std::make_unique<tint::writer::hlsl::Generator>(program.get());
  }
#endif  // TINT_BUILD_HLSL_WRITER

  if (!writer) {
    std::cerr << "Unknown output format specified" << std::endl;
    return 1;
  }

  if (!writer->Generate()) {
    PrintWGSL(std::cerr, out.program);
    std::cerr << "Failed to generate: " << writer->error() << std::endl;
    return 1;
  }

#if TINT_BUILD_SPV_WRITER
  bool dawn_validation_failed = false;
  std::ostringstream stream;

  if (options.dawn_validation &&
      (options.format == Format::kSpvAsm || options.format == Format::kSpirv)) {
    // Use Vulkan 1.1, since this is what Tint, internally, uses.
    spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
    tools.SetMessageConsumer([&stream](spv_message_level_t, const char*,
                                       const spv_position_t& pos,
                                       const char* msg) {
      stream << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg
             << std::endl;
    });
    auto* w = static_cast<tint::writer::spirv::Generator*>(writer.get());
    if (!tools.Validate(w->result().data(), w->result().size(),
                        spvtools::ValidatorOptions())) {
      dawn_validation_failed = true;
    }
  }

  if (options.format == Format::kSpvAsm) {
    auto* w = static_cast<tint::writer::spirv::Generator*>(writer.get());
    auto str = Disassemble(w->result());
    if (!WriteFile(options.output_file, "w", str)) {
      return 1;
    }
  }
  if (options.format == Format::kSpirv) {
    auto* w = static_cast<tint::writer::spirv::Generator*>(writer.get());
    if (!WriteFile(options.output_file, "wb", w->result())) {
      return 1;
    }
  }
  if (dawn_validation_failed) {
    std::cerr << std::endl << std::endl << "Validation Failure:" << std::endl;
    std::cerr << stream.str();
    return 1;
  }
#endif  // TINT_BUILD_SPV_WRITER

  if (options.format != Format::kSpvAsm && options.format != Format::kSpirv) {
    auto* w = static_cast<tint::writer::Text*>(writer.get());
    auto output = w->result();
    if (options.demangle) {
      output = tint::Demangler().Demangle(program->Symbols(), output);
    }
    if (!WriteFile(options.output_file, "w", output)) {
      return 1;
    }
  }

  return 0;
}
