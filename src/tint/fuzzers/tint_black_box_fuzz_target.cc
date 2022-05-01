// Copyright 2021 The Tint Authors.
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

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "src/tint/fuzzers/tint_common_fuzzer.h"

namespace {

/// Controls the target language in which code will be generated.
enum class TargetLanguage {
    kHlsl,
    kMsl,
    kSpv,
    kWgsl,
    kTargetLanguageMax,
};

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

}  // namespace

int main(int argc, const char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <input file> [hlsl|msl|spv|wgsl]" << std::endl;
        return 1;
    }

    std::string input_filename(argv[1]);

    std::vector<uint8_t> data;
    if (!ReadFile<uint8_t>(input_filename, &data)) {
        return 1;
    }

    if (data.empty()) {
        return 0;
    }

    tint::fuzzers::DataBuilder builder(data.data(), data.size());

    TargetLanguage target_language;

    if (argc == 3) {
        std::string target_language_string = argv[2];
        if (target_language_string == "hlsl") {
            target_language = TargetLanguage::kHlsl;
        } else if (target_language_string == "msl") {
            target_language = TargetLanguage::kMsl;
        } else if (target_language_string == "spv") {
            target_language = TargetLanguage::kSpv;
        } else {
            assert(target_language_string == "wgsl" && "Unknown target language.");
            target_language = TargetLanguage::kWgsl;
        }
    } else {
        target_language = builder.enum_class<TargetLanguage>(
            static_cast<uint32_t>(TargetLanguage::kTargetLanguageMax));
    }

    switch (target_language) {
        case TargetLanguage::kHlsl: {
            tint::fuzzers::CommonFuzzer fuzzer(tint::fuzzers::InputFormat::kWGSL,
                                               tint::fuzzers::OutputFormat::kHLSL);
            return fuzzer.Run(data.data(), data.size());
        }
        case TargetLanguage::kMsl: {
            tint::writer::msl::Options options;
            GenerateMslOptions(&builder, &options);
            tint::fuzzers::CommonFuzzer fuzzer(tint::fuzzers::InputFormat::kWGSL,
                                               tint::fuzzers::OutputFormat::kMSL);
            fuzzer.SetOptionsMsl(options);
            return fuzzer.Run(data.data(), data.size());
        }
        case TargetLanguage::kSpv: {
            tint::writer::spirv::Options options;
            GenerateSpirvOptions(&builder, &options);
            tint::fuzzers::CommonFuzzer fuzzer(tint::fuzzers::InputFormat::kWGSL,
                                               tint::fuzzers::OutputFormat::kSpv);
            fuzzer.SetOptionsSpirv(options);
            return fuzzer.Run(data.data(), data.size());
        }
        case TargetLanguage::kWgsl: {
            tint::fuzzers::CommonFuzzer fuzzer(tint::fuzzers::InputFormat::kWGSL,
                                               tint::fuzzers::OutputFormat::kWGSL);
            return fuzzer.Run(data.data(), data.size());
        }
        default:
            std::cerr << "Aborting due to unknown target language; fuzzer must be "
                         "misconfigured."
                      << std::endl;
            abort();
    }
}
