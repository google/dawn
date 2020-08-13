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
#include <string>
#include <vector>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "tint/tint.h"

namespace {

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

  Format format = Format::kNone;
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
  --output-file <name>      -- Output file name.  Use "-" for standard output
  -o <name>                 -- Output file name.  Use "-" for standard output
  --parse-only              -- Stop after parsing the input
  --dump-ast                -- Dump the generated AST to stdout
  -h                        -- This help text)";

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
Format parse_format(const std::string& fmt) {
#pragma clang diagnostic pop

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
Format infer_format(const std::string& filename) {
#pragma clang diagnostic pop

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
#endif  // TINT_BUILD_WGSL_WRITER

  return Format::kNone;
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
    } else if (arg == "-o" || arg == "--output-name") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for " << arg << std::endl;
        return false;
      }
      opts->output_file = args[i];

    } else if (arg == "-h" || arg == "--help") {
      opts->show_help = true;
    } else if (arg == "--parse-only") {
      opts->parse_only = true;
    } else if (arg == "--dump-ast") {
      opts->dump_ast = true;
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

/// Copies the content from the file named |filename| to |buffer|,
/// assuming each element in the file is of type |T|.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a |T| object is divisible by its required alignment.
/// @returns true if we successfully read the file.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-template"
template <typename T>
bool ReadFile(const std::string& input_file, std::vector<T>* buffer) {
#pragma clang diagnostic pop

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

/// Writes the given |buffer| into the file named as |output_file| using the
/// given |mode|.  If |filename| is empty or "-", writes to standard output. If
/// any error occurs, returns false and outputs error message to standard error.
/// The ContainerT type must have data() and size() methods, like std::string
/// and std::vector do.
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

}  // namespace

int main(int argc, const char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;

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

  tint::Context ctx;

  std::unique_ptr<tint::reader::Reader> reader;
#if TINT_BUILD_WGSL_READER
  if (options.input_filename.size() > 5 &&
      options.input_filename.substr(options.input_filename.size() - 5) ==
          ".wgsl") {
    std::vector<uint8_t> data;
    if (!ReadFile<uint8_t>(options.input_filename, &data)) {
      return 1;
    }
    reader = std::make_unique<tint::reader::wgsl::Parser>(
        &ctx, std::string(data.begin(), data.end()));
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
    reader = std::make_unique<tint::reader::spirv::Parser>(&ctx, data);
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
    reader = std::make_unique<tint::reader::spirv::Parser>(&ctx, data);
  }
#endif  // TINT_BUILD_SPV_READER

  if (!reader) {
    std::cerr << "Failed to create reader for input file: "
              << options.input_filename << std::endl;
    return 1;
  }
  if (!reader->Parse()) {
    std::cerr << "Parse: " << reader->error() << std::endl;
    return 1;
  }

  auto mod = reader->module();
  if (options.dump_ast) {
    std::cout << std::endl << mod.to_str() << std::endl;
  }
  if (options.parse_only) {
    return 1;
  }

  if (!mod.IsValid()) {
    std::cerr << "Invalid module generated..." << std::endl;
    return 1;
  }

  tint::TypeDeterminer td(&ctx, &mod);
  if (!td.Determine()) {
    std::cerr << "Type Determination: " << td.error() << std::endl;
    return 1;
  }

  tint::Validator v;
  if (!v.Validate(&mod)) {
    std::cerr << "Validation: " << v.error() << std::endl;
    return 1;
  }

  std::unique_ptr<tint::writer::Writer> writer;

#if TINT_BUILD_SPV_WRITER
  if (options.format == Format::kSpirv || options.format == Format::kSpvAsm) {
    writer = std::make_unique<tint::writer::spirv::Generator>(std::move(mod));
  }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
  if (options.format == Format::kWgsl) {
    writer = std::make_unique<tint::writer::wgsl::Generator>(std::move(mod));
  }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
  if (options.format == Format::kMsl) {
    writer = std::make_unique<tint::writer::msl::Generator>(std::move(mod));
  }
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
  if (options.format == Format::kHlsl) {
    writer = std::make_unique<tint::writer::hlsl::Generator>(std::move(mod));
  }
#endif  // TINT_BUILD_HLSL_WRITER

  if (!writer) {
    std::cerr << "Unknown output format specified" << std::endl;
    return 1;
  }

  if (!writer->Generate()) {
    std::cerr << "Failed to generate: " << writer->error() << std::endl;
    return 1;
  }

#if TINT_BUILD_SPV_WRITER
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
#endif  // TINT_BUILD_SPV_WRITER

  if (options.format != Format::kSpvAsm && options.format != Format::kSpirv) {
    auto* w = static_cast<tint::writer::Text*>(writer.get());
    if (!WriteFile(options.output_file, "w", w->result())) {
      return 1;
    }
  }

  return 0;
}
