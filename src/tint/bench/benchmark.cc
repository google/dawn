// Copyright 2022 The Tint Authors.
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

#include "src/tint/bench/benchmark.h"

#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

#include "src/tint/utils/string_stream.h"

namespace tint::bench {
namespace {

std::filesystem::path kInputFileDir;

/// Copies the content from the file named `input_file` to `buffer`,
/// assuming each element in the file is of type `T`.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a `T` object is divisible by its required alignment.
/// @returns true if we successfully read the file.
template <typename T>
std::variant<std::vector<T>, Error> ReadFile(const std::string& input_file) {
    FILE* file = nullptr;
#if defined(_MSC_VER)
    fopen_s(&file, input_file.c_str(), "rb");
#else
    file = fopen(input_file.c_str(), "rb");
#endif
    if (!file) {
        return Error{"Failed to open " + input_file};
    }

    fseek(file, 0, SEEK_END);
    const auto file_size = static_cast<size_t>(ftell(file));
    if (0 != (file_size % sizeof(T))) {
        utils::StringStream err;
        err << "File " << input_file
            << " does not contain an integral number of objects: " << file_size
            << " bytes in the file, require " << sizeof(T) << " bytes per object";
        fclose(file);
        return Error{err.str()};
    }
    fseek(file, 0, SEEK_SET);

    std::vector<T> buffer;
    buffer.resize(file_size / sizeof(T));

    size_t bytes_read = fread(buffer.data(), 1, file_size, file);
    fclose(file);
    if (bytes_read != file_size) {
        return Error{"Failed to read " + input_file};
    }

    return buffer;
}

bool FindBenchmarkInputDir() {
    // Attempt to find the benchmark input files by searching up from the current
    // working directory.
    auto path = std::filesystem::current_path();
    while (std::filesystem::is_directory(path)) {
        auto test = path / "test" / "tint" / "benchmark";
        if (std::filesystem::is_directory(test)) {
            kInputFileDir = test;
            return true;
        }
        auto parent = path.parent_path();
        if (path == parent) {
            break;
        }
        path = parent;
    }
    return false;
}

}  // namespace

std::variant<tint::Source::File, Error> LoadInputFile(std::string name) {
    auto path = (kInputFileDir / name).string();
    auto data = ReadFile<uint8_t>(path);
    if (auto* buf = std::get_if<std::vector<uint8_t>>(&data)) {
        return tint::Source::File(path, std::string(buf->begin(), buf->end()));
    }
    return std::get<Error>(data);
}

std::variant<ProgramAndFile, Error> LoadProgram(std::string name) {
    auto res = bench::LoadInputFile(name);
    if (auto err = std::get_if<bench::Error>(&res)) {
        return *err;
    }
    auto& file = std::get<Source::File>(res);
    auto program = reader::wgsl::Parse(&file);
    if (program.Diagnostics().contains_errors()) {
        return Error{program.Diagnostics().str()};
    }
    return ProgramAndFile{std::move(program), std::move(file)};
}

}  // namespace tint::bench

int main(int argc, char** argv) {
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    if (!tint::bench::FindBenchmarkInputDir()) {
        std::cerr << "failed to locate benchmark input files" << std::endl;
        return 1;
    }
    benchmark::RunSpecifiedBenchmarks();
}
