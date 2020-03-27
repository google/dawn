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

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "src/context.h"
#include "src/reader/reader.h"
#include "src/type_determiner.h"
#include "src/type_manager.h"
#include "src/validator.h"
#include "src/writer/writer.h"

#if TINT_BUILD_SPV_READER
#include "src/reader/spirv/parser.h"
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER
#include "src/reader/wgsl/parser.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#include "src/writer/spirv/generator.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/writer/wgsl/generator.h"
#endif  // TINT_BUILD_WGSL_WRITER

namespace {

enum class Format {
  kNone = -1,
  kSpirv,
  kSpvAsm,
  kWgsl,
};

struct Options {
  bool show_help = false;

  std::string input_filename;
  std::string output_name = "";
  std::string output_ext = "spv";

  bool parse_only = false;
  bool dump_ast = false;

  Format format = Format::kSpirv;
};

const char kUsage[] = R"(Usage: tint [options] SCRIPT [SCRIPTS...]

 options:
  --format <spirv|spvasm|wgsl>  -- Output format
  --output-name <name>      -- Name for the output file, without extension
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

      if (opts->format == Format::kSpvAsm)
        opts->output_ext = "spvasm";
      else if (opts->format == Format::kWgsl)
        opts->output_ext = "wgsl";
    }
    if (arg == "--output-name") {
      ++i;
      if (i >= args.size()) {
        std::cerr << "Missing value for --output_name argument." << std::endl;
        return false;
      }
      opts->output_name = args[i];

    } else if (arg == "-h" || arg == "--help") {
      opts->show_help = true;
    } else if (arg == "--parse-only") {
      opts->parse_only = true;
    } else if (arg == "--dump-ast") {
      opts->dump_ast = true;
    } else if (!arg.empty()) {
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
#pragma Clang pop

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
  tools.Disassemble(data, &result,
                    SPV_BINARY_TO_TEXT_OPTION_INDENT |
                        SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES);
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
  if (options.input_filename.empty()) {
    std::cerr << "Input file missing" << std::endl;
    std::cout << kUsage << std::endl;
    return 1;
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
  if (options.input_filename.size() > 4 &&
      options.input_filename.substr(options.input_filename.size() - 4) ==
          ".spv") {
    std::vector<uint32_t> data;
    if (!ReadFile<uint32_t>(options.input_filename, &data)) {
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
    std::cerr << reader->error() << std::endl;
    return 1;
  }

  auto module = reader->module();
  if (options.dump_ast) {
    std::cout << std::endl << module.to_str() << std::endl;
  }
  if (options.parse_only) {
    return 1;
  }

  tint::TypeDeterminer td;
  if (!td.Determine(&module)) {
    std::cerr << td.error() << std::endl;
    return 1;
  }

  tint::Validator v;
  if (!v.Validate(module)) {
    std::cerr << v.error() << std::endl;
    return 1;
  }

  std::unique_ptr<tint::writer::Writer> writer;

#if TINT_BUILD_SPV_WRITER
  if (options.format == Format::kSpirv || options.format == Format::kSpvAsm) {
    writer =
        std::make_unique<tint::writer::spirv::Generator>(std::move(module));
  }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
  if (options.format == Format::kWgsl) {
    writer = std::make_unique<tint::writer::wgsl::Generator>(std::move(module));
  }
#endif  // TINT_BUILD_WGSL_WRITER

  if (!writer) {
    std::cerr << "Unknown output format specified" << std::endl;
    return 1;
  }

  if (!writer->Generate()) {
    std::cerr << "Failed to generate SPIR-V: " << writer->error() << std::endl;
    return 1;
  }

#if TINT_BUILD_SPV_WRITER
  if (options.format == Format::kSpvAsm) {
    auto w = static_cast<tint::writer::spirv::Generator*>(writer.get());
    auto str = Disassemble(w->result());
    // TODO(dsinclair): Write to file if output_file given
    std::cout << str << std::endl;
  }
  if (options.format == Format::kSpirv) {
    // auto w = static_cast<tint::writer::spirv::Generator*>(writer.get());
    // TODO(dsincliair): Write to to file
  }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
  if (options.format == Format::kWgsl) {
    auto w = static_cast<tint::writer::wgsl::Generator*>(writer.get());
    std::cout << w->result() << std::endl;
  }
#endif  // TINT_BUILD_WGSL_WRITER

  return 0;
}
