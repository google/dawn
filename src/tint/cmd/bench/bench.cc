// Copyright 2023 The Dawn & Tint Authors
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

#include <filesystem>
#include <iostream>
#include <utility>
#include <vector>

#include "src/tint/cmd/bench/bench.h"

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/reader.h"
#endif

#if TINT_BUILD_WGSL_WRITER
#include "src/tint/lang/wgsl/writer/writer.h"
#endif

#if TINT_BUILD_WGSL_READER
#include "src/tint/lang/wgsl/reader/reader.h"
#endif

#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::bench {
namespace {

std::filesystem::path kInputFileDir;

/// Copies the content from the file named `input_file` to `buffer`,
/// assuming each element in the file is of type `T`.  If any error occurs,
/// writes error messages to the standard error stream and returns false.
/// Assumes the size of a `T` object is divisible by its required alignment.
/// @returns true if we successfully read the file.
template <typename T>
Result<std::vector<T>> ReadFile(const std::string& input_file) {
    FILE* file = nullptr;
#if defined(_MSC_VER)
    fopen_s(&file, input_file.c_str(), "rb");
#else
    file = fopen(input_file.c_str(), "rb");
#endif
    if (!file) {
        return Failure{"Failed to open " + input_file};
    }

    fseek(file, 0, SEEK_END);
    const auto file_size = static_cast<size_t>(ftell(file));
    if (0 != (file_size % sizeof(T))) {
        StringStream err;
        err << "File " << input_file
            << " does not contain an integral number of objects: " << file_size
            << " bytes in the file, require " << sizeof(T) << " bytes per object";
        fclose(file);
        return Failure{err.str()};
    }
    fseek(file, 0, SEEK_SET);

    std::vector<T> buffer;
    buffer.resize(file_size / sizeof(T));

    size_t bytes_read = fread(buffer.data(), 1, file_size, file);
    fclose(file);
    if (bytes_read != file_size) {
        return Failure{"Failed to read " + input_file};
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

bool Initialize() {
    if (!FindBenchmarkInputDir()) {
        std::cerr << "failed to locate benchmark input files" << std::endl;
        return false;
    }
    return true;
}

Result<Source::File> LoadInputFile(std::string name) {
    auto path = std::filesystem::path(name).is_absolute() ? name : (kInputFileDir / name).string();
    if (tint::HasSuffix(path, ".wgsl")) {
#if TINT_BUILD_WGSL_READER
        auto data = ReadFile<uint8_t>(path);
        if (!data) {
            return data.Failure();
        }
        return tint::Source::File(path, std::string(data->begin(), data->end()));
#else
        return Failure{"cannot load " + path + " file as TINT_BUILD_WGSL_READER is not enabled"};
#endif
    }
    if (tint::HasSuffix(path, ".spv")) {
#if !TINT_BUILD_SPV_READER
        return Failure{"cannot load " + path + " as TINT_BUILD_SPV_READER is not enabled"};
#elif !TINT_BUILD_WGSL_WRITER
        return Failure{"cannot load " + path + " as TINT_BUILD_WGSL_WRITER is not enabled"};
#else

        auto spirv = ReadFile<uint32_t>(path);
        if (spirv) {
            auto program = tint::spirv::reader::Read(spirv.Get(), {});
            if (!program.IsValid()) {
                return Failure{program.Diagnostics()};
            }
            auto result = tint::wgsl::writer::Generate(program, {});
            if (!result) {
                return result.Failure();
            }
            return tint::Source::File(path, result->wgsl);
        }
        return spirv.Failure();
#endif
    }
    return Failure{"unsupported file extension: '" + name + "'"};
}

Result<ProgramAndFile> LoadProgram(std::string name) {
    auto res = bench::LoadInputFile(name);
    if (!res) {
        return res.Failure();
    }
    auto file = std::make_unique<Source::File>(res.Get());
    auto program = wgsl::reader::Parse(file.get());
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }
    return ProgramAndFile{std::move(program), std::move(file)};
}

}  // namespace tint::bench
