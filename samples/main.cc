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

#include "spirv-tools/libspirv.hpp"
#include "src/reader/reader.h"
#include "src/reader/wgsl/parser.h"
#include "src/type_determiner.h"
#include "src/validator.h"
#include "src/writer/spv/generator.h"
#include "src/writer/wgsl/generator.h"
#include "src/writer/writer.h"

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

Format parse_format(const std::string& fmt) {
  if (fmt == "spirv")
    return Format::kSpirv;
  if (fmt == "spvasm")
    return Format::kSpvAsm;
  if (fmt == "wgsl")
    return Format::kWgsl;

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

std::vector<uint8_t> ReadFile(const std::string& input_file) {
  FILE* file = nullptr;
#if defined(_MSC_VER)
  fopen_s(&file, input_file.c_str(), "rb");
#else
  file = fopen(input_file.c_str(), "rb");
#endif
  if (!file) {
    std::cerr << "Failed to open " << input_file << std::endl;
    return {};
  }

  fseek(file, 0, SEEK_END);
  uint64_t tell_file_size = static_cast<uint64_t>(ftell(file));
  if (tell_file_size <= 0) {
    std::cerr << "Input file of incorrect size: " << input_file << std::endl;
    fclose(file);
    return {};
  }
  fseek(file, 0, SEEK_SET);

  size_t file_size = static_cast<size_t>(tell_file_size);

  std::vector<uint8_t> data;
  data.resize(file_size);

  size_t bytes_read = fread(data.data(), sizeof(uint8_t), file_size, file);
  fclose(file);
  if (bytes_read != file_size) {
    std::cerr << "Failed to read " << input_file << std::endl;
    return {};
  }

  return data;
}

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
  if (options.input_filename == "") {
    std::cerr << "Input file missing" << std::endl;
    std::cout << kUsage << std::endl;
    return 1;
  }

  auto data = ReadFile(options.input_filename);
  if (data.size() == 0)
    return 1;

  std::unique_ptr<tint::reader::Reader> reader;
  std::string ext = "wgsl";
  if (options.input_filename.size() > 4 &&
      options.input_filename.substr(options.input_filename.size() - 4) ==
          "wgsl") {
    reader = std::make_unique<tint::reader::wgsl::Parser>(
        std::string(data.begin(), data.end()));
  }
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
  if (options.format == Format::kSpirv || options.format == Format::kSpvAsm) {
    writer = std::make_unique<tint::writer::spv::Generator>(std::move(module));
  } else if (options.format == Format::kWgsl) {
    writer = std::make_unique<tint::writer::wgsl::Generator>(std::move(module));
  } else {
    std::cerr << "Unknown output format specified" << std::endl;
    return 1;
  }

  if (!writer->Generate()) {
    std::cerr << "Failed to generate SPIR-V: " << writer->error() << std::endl;
    return 1;
  }

  if (options.format == Format::kSpvAsm) {
    auto w = static_cast<tint::writer::spv::Generator*>(writer.get());
    auto str = Disassemble(w->result());
    // TODO(dsinclair): Write to file if output_file given
    std::cout << str << std::endl;
  } else if (options.format == Format::kSpirv) {
    // auto w = static_cast<tint::writer::spv::Generator*>(writer.get());
    // TODO(dsincliair): Write to to file
  } else if (options.format == Format::kWgsl) {
    auto w = static_cast<tint::writer::wgsl::Generator*>(writer.get());
    std::cout << w->result() << std::endl;
  }

  return 0;
}
