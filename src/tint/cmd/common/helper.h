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

#ifndef SRC_TINT_CMD_COMMON_HELPER_H_
#define SRC_TINT_CMD_COMMON_HELPER_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/utils/diagnostic/source.h"

#if TINT_BUILD_SPV_READER
#include "src/tint/lang/spirv/reader/common/options.h"
#endif

// Forward declarations
namespace tint {
class Program;
class InternalCompilerError;
}  // namespace tint

namespace tint::cmd {

/// Information on a loaded program
struct ProgramInfo {
    /// The loaded program
    tint::Program program;
    /// The source file information
    std::unique_ptr<tint::Source::File> source_file;
};

/// Reporter callback for internal tint errors
[[noreturn]] void TintInternalCompilerErrorReporter(const InternalCompilerError& err);

/// PrintWGSL writes the WGSL of the program to the provided ostream, if the
/// WGSL writer is enabled, otherwise it does nothing.
/// @param out the output stream to write the WGSL to
/// @param program the program
void PrintWGSL(std::ostream& out, const tint::Program& program);

/// Prints inspector data information to stderr
/// @param inspector the inspector to print.
void PrintInspectorData(tint::inspector::Inspector& inspector);

/// Prints inspector binding information to stderr
/// @param inspector the inspector to print.
void PrintInspectorBindings(tint::inspector::Inspector& inspector);

/// Options for the LoadProgramInfo call
struct LoadProgramOptions {
    /// The file to be loaded
    std::string filename;
#if TINT_BUILD_SPV_READER
    /// Spirv-reader options
    tint::spirv::reader::Options spirv_reader_options;
#endif
};

/// Loads the source and program information for the given file.
/// If the file cannot be loaded then an error is printed and the program is aborted before
/// returning.
/// @param opts the loading options
ProgramInfo LoadProgramInfo(const LoadProgramOptions& opts);

/// @param stage the pipeline stage
/// @returns the string representation
std::string EntryPointStageToString(tint::inspector::PipelineStage stage);

/// @param dim the dimension
/// @returns the text name
std::string TextureDimensionToString(tint::inspector::ResourceBinding::TextureDimension dim);

/// @param kind the sample kind
/// @returns the text name
std::string SampledKindToString(tint::inspector::ResourceBinding::SampledKind kind);

/// @param format the texel format
/// @returns the text name
std::string TexelFormatToString(tint::inspector::ResourceBinding::TexelFormat format);

/// @param type the resource type
/// @returns the text name
std::string ResourceTypeToString(tint::inspector::ResourceBinding::ResourceType type);

/// @param type the composition type
/// @return the text name
std::string CompositionTypeToString(tint::inspector::CompositionType type);

/// @param type the component type
/// @return the text name
std::string ComponentTypeToString(tint::inspector::ComponentType type);

/// @param type the interpolation sampling type
/// @return the text name
std::string InterpolationSamplingToString(tint::inspector::InterpolationSampling type);

/// @param type the interpolation type
/// @return the text name
std::string InterpolationTypeToString(tint::inspector::InterpolationType type);

/// @param type the override type
/// @return the text name
std::string OverrideTypeToString(tint::inspector::Override::Type type);

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

}  // namespace tint::cmd

#endif  // SRC_TINT_CMD_COMMON_HELPER_H_
