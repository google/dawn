// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_CMD_HELPER_H_
#define SRC_TINT_CMD_HELPER_H_

#include <memory>
#include <string>

#include "tint/tint.h"

namespace tint::cmd {

/// Information on a loaded program
struct ProgramInfo {
    /// The loaded program
    std::unique_ptr<tint::Program> program;
    /// The source file information
    std::unique_ptr<tint::Source::File> source_file;
};

/// Reporter callback for internal tint errors
/// @param diagnostics the diagnostics to emit
[[noreturn]] void TintInternalCompilerErrorReporter(const tint::diag::List& diagnostics);

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
    tint::reader::spirv::Options spirv_reader_options;
#endif
};

/// Loads the source and program information for the given file
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

}  // namespace tint::cmd

#endif  // SRC_TINT_CMD_HELPER_H_
