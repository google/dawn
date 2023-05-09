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

#include "src/tint/cmd/helper.h"

#include <iostream>
#include <utility>
#include <vector>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif

#include "src/tint/utils/string.h"

namespace tint::cmd {
namespace {

enum class InputFormat {
    kUnknown,
    kWgsl,
    kSpirvBin,
    kSpirvAsm,
};

InputFormat InputFormatFromFilename(const std::string& filename) {
    auto input_format = InputFormat::kUnknown;
    if (utils::HasSuffix(filename, ".wgsl")) {
        input_format = InputFormat::kWgsl;
    } else if (utils::HasSuffix(filename, ".spv")) {
        input_format = InputFormat::kSpirvBin;
    } else if (utils::HasSuffix(filename, ".spvasm")) {
        input_format = InputFormat::kSpirvAsm;
    }
    return input_format;
}

void PrintBindings(tint::inspector::Inspector& inspector, const std::string& ep_name) {
    auto bindings = inspector.GetResourceBindings(ep_name);
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get bindings from Inspector: " << inspector.error() << std::endl;
        exit(1);
    }
    for (auto& binding : bindings) {
        std::cout << "\t[" << binding.bind_group << "][" << binding.binding << "]:" << std::endl;
        std::cout << "\t\t resource_type = " << ResourceTypeToString(binding.resource_type)
                  << std::endl;
        std::cout << "\t\t dim = " << TextureDimensionToString(binding.dim) << std::endl;
        std::cout << "\t\t sampled_kind = " << SampledKindToString(binding.sampled_kind)
                  << std::endl;
        std::cout << "\t\t image_format = " << TexelFormatToString(binding.image_format)
                  << std::endl;

        std::cout << std::endl;
    }
}

}  // namespace

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::diag::List& diagnostics) {
    auto printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter{}.format(diagnostics, printer.get());
    tint::diag::Style bold_red{tint::diag::Color::kRed, true};
    constexpr const char* please_file_bug = R"(
********************************************************************
*  The tint shader compiler has encountered an unexpected error.   *
*                                                                  *
*  Please help us fix this issue by submitting a bug report at     *
*  crbug.com/tint with the source program that triggered the bug.  *
********************************************************************
)";
    printer->write(please_file_bug, bold_red);
    exit(1);
}

void PrintWGSL(std::ostream& out, const tint::Program& program) {
#if TINT_BUILD_WGSL_WRITER
    tint::writer::wgsl::Options options;
    auto result = tint::writer::wgsl::Generate(&program, options);
    out << std::endl << result.wgsl << std::endl;
#else
    (void)out;
    (void)program;
#endif
}

ProgramInfo LoadProgramInfo(const LoadProgramOptions& opts) {
    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    auto input_format = InputFormatFromFilename(opts.filename);
    switch (input_format) {
        case InputFormat::kUnknown: {
            std::cerr << "Unknown input format" << std::endl;
            exit(1);
        }
        case InputFormat::kWgsl: {
#if TINT_BUILD_WGSL_READER
            std::vector<uint8_t> data;
            if (!ReadFile<uint8_t>(opts.filename, &data)) {
                exit(1);
            }
            source_file = std::make_unique<tint::Source::File>(
                opts.filename, std::string(data.begin(), data.end()));
            program = std::make_unique<tint::Program>(tint::reader::wgsl::Parse(source_file.get()));
            break;
#else
            std::cerr << "Tint not built with the WGSL reader enabled" << std::endl;
            exit(1);
#endif  // TINT_BUILD_WGSL_READER
        }
        case InputFormat::kSpirvBin: {
#if TINT_BUILD_SPV_READER
            std::vector<uint32_t> data;
            if (!ReadFile<uint32_t>(opts.filename, &data)) {
                exit(1);
            }
            program = std::make_unique<tint::Program>(
                tint::reader::spirv::Parse(data, opts.spirv_reader_options));
            break;
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            exit(1);
#endif  // TINT_BUILD_SPV_READER
        }
        case InputFormat::kSpirvAsm: {
#if TINT_BUILD_SPV_READER
            std::vector<char> text;
            if (!ReadFile<char>(opts.filename, &text)) {
                exit(1);
            }
            // Use Vulkan 1.1, since this is what Tint, internally, is expecting.
            spvtools::SpirvTools tools(SPV_ENV_VULKAN_1_1);
            tools.SetMessageConsumer([](spv_message_level_t, const char*, const spv_position_t& pos,
                                        const char* msg) {
                std::cerr << (pos.line + 1) << ":" << (pos.column + 1) << ": " << msg << std::endl;
            });
            std::vector<uint32_t> data;
            if (!tools.Assemble(text.data(), text.size(), &data,
                                SPV_TEXT_TO_BINARY_OPTION_PRESERVE_NUMERIC_IDS)) {
                exit(1);
            }
            program = std::make_unique<tint::Program>(
                tint::reader::spirv::Parse(data, opts.spirv_reader_options));
            break;
#else
            std::cerr << "Tint not built with the SPIR-V reader enabled" << std::endl;
            exit(1);
#endif  // TINT_BUILD_SPV_READER
        }
    }

    if (!program) {
        std::cerr << "Failed to parse input file: " << opts.filename << std::endl;
        exit(1);
    }
    if (program->Diagnostics().count() > 0) {
        if (!program->IsValid() && input_format != InputFormat::kWgsl) {
            // Invalid program from a non-wgsl source. Print the WGSL, to help
            // understand the diagnostics.
            PrintWGSL(std::cout, *program);
        }

        auto diag_printer = tint::diag::Printer::create(stderr, true);
        tint::diag::Formatter diag_formatter;
        diag_formatter.format(program->Diagnostics(), diag_printer.get());
    }

    if (!program->IsValid()) {
        exit(1);
    }

    return ProgramInfo{
        std::move(program),
        std::move(source_file),
    };
}

void PrintInspectorData(tint::inspector::Inspector& inspector) {
    auto entry_points = inspector.GetEntryPoints();
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get entry points from Inspector: " << inspector.error()
                  << std::endl;
        exit(1);
    }

    for (auto& entry_point : entry_points) {
        std::cout << "Entry Point = " << entry_point.name << " ("
                  << EntryPointStageToString(entry_point.stage) << ")" << std::endl;

        if (entry_point.workgroup_size) {
            std::cout << "  Workgroup Size (" << entry_point.workgroup_size->x << ", "
                      << entry_point.workgroup_size->y << ", " << entry_point.workgroup_size->z
                      << ")" << std::endl;
        }

        if (!entry_point.input_variables.empty()) {
            std::cout << "  Input Variables:" << std::endl;

            for (const auto& var : entry_point.input_variables) {
                std::cout << "\t";

                if (var.has_location_attribute) {
                    std::cout << "@location(" << var.location_attribute << ") ";
                }
                std::cout << var.name << std::endl;
            }
        }
        if (!entry_point.output_variables.empty()) {
            std::cout << "  Output Variables:" << std::endl;

            for (const auto& var : entry_point.output_variables) {
                std::cout << "\t";

                if (var.has_location_attribute) {
                    std::cout << "@location(" << var.location_attribute << ") ";
                }
                std::cout << var.name << std::endl;
            }
        }
        if (!entry_point.overrides.empty()) {
            std::cout << "  Overrides:" << std::endl;

            for (const auto& var : entry_point.overrides) {
                std::cout << "\tname: " << var.name << std::endl;
                std::cout << "\tid: " << var.id.value << std::endl;
            }
        }

        auto bindings = inspector.GetResourceBindings(entry_point.name);
        if (!inspector.error().empty()) {
            std::cerr << "Failed to get bindings from Inspector: " << inspector.error()
                      << std::endl;
            exit(1);
        }

        if (!bindings.empty()) {
            std::cout << "  Bindings:" << std::endl;
            PrintBindings(inspector, entry_point.name);
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }
}

void PrintInspectorBindings(tint::inspector::Inspector& inspector) {
    std::cout << std::string(80, '-') << std::endl;
    auto entry_points = inspector.GetEntryPoints();
    if (!inspector.error().empty()) {
        std::cerr << "Failed to get entry points from Inspector: " << inspector.error()
                  << std::endl;
        exit(1);
    }

    for (auto& entry_point : entry_points) {
        std::cout << "Entry Point = " << entry_point.name << std::endl;
        PrintBindings(inspector, entry_point.name);
    }
    std::cout << std::string(80, '-') << std::endl;
}

std::string EntryPointStageToString(tint::inspector::PipelineStage stage) {
    switch (stage) {
        case tint::inspector::PipelineStage::kVertex:
            return "vertex";
        case tint::inspector::PipelineStage::kFragment:
            return "fragment";
        case tint::inspector::PipelineStage::kCompute:
            return "compute";
    }
    return "Unknown";
}

std::string TextureDimensionToString(tint::inspector::ResourceBinding::TextureDimension dim) {
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

std::string SampledKindToString(tint::inspector::ResourceBinding::SampledKind kind) {
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

std::string TexelFormatToString(tint::inspector::ResourceBinding::TexelFormat format) {
    switch (format) {
        case tint::inspector::ResourceBinding::TexelFormat::kR32Uint:
            return "R32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Sint:
            return "R32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kR32Float:
            return "R32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kBgra8Unorm:
            return "Bgra8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Unorm:
            return "Rgba8Unorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Snorm:
            return "Rgba8Snorm";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Uint:
            return "Rgba8Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba8Sint:
            return "Rgba8Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Uint:
            return "Rg32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Sint:
            return "Rg32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRg32Float:
            return "Rg32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Uint:
            return "Rgba16Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Sint:
            return "Rgba16Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba16Float:
            return "Rgba16Float";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Uint:
            return "Rgba32Uint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Sint:
            return "Rgba32Sint";
        case tint::inspector::ResourceBinding::TexelFormat::kRgba32Float:
            return "Rgba32Float";
        case tint::inspector::ResourceBinding::TexelFormat::kNone:
            return "None";
    }
    return "Unknown";
}

std::string ResourceTypeToString(tint::inspector::ResourceBinding::ResourceType type) {
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
        case tint::inspector::ResourceBinding::ResourceType::kWriteOnlyStorageTexture:
            return "WriteOnlyStorageTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthTexture:
            return "DepthTexture";
        case tint::inspector::ResourceBinding::ResourceType::kDepthMultisampledTexture:
            return "DepthMultisampledTexture";
        case tint::inspector::ResourceBinding::ResourceType::kExternalTexture:
            return "ExternalTexture";
    }

    return "Unknown";
}

std::string ComponentTypeToString(tint::inspector::ComponentType type) {
    switch (type) {
        case tint::inspector::ComponentType::kUnknown:
            return "unknown";
        case tint::inspector::ComponentType::kF32:
            return "f32";
        case tint::inspector::ComponentType::kU32:
            return "u32";
        case tint::inspector::ComponentType::kI32:
            return "i32";
        case tint::inspector::ComponentType::kF16:
            return "f16";
    }
    return "unknown";
}

std::string CompositionTypeToString(tint::inspector::CompositionType type) {
    switch (type) {
        case tint::inspector::CompositionType::kUnknown:
            return "unknown";
        case tint::inspector::CompositionType::kScalar:
            return "scalar";
        case tint::inspector::CompositionType::kVec2:
            return "vec2";
        case tint::inspector::CompositionType::kVec3:
            return "vec3";
        case tint::inspector::CompositionType::kVec4:
            return "vec4";
    }
    return "unknown";
}

std::string InterpolationTypeToString(tint::inspector::InterpolationType type) {
    switch (type) {
        case tint::inspector::InterpolationType::kUnknown:
            return "unknown";
        case tint::inspector::InterpolationType::kPerspective:
            return "perspective";
        case tint::inspector::InterpolationType::kLinear:
            return "linear";
        case tint::inspector::InterpolationType::kFlat:
            return "flat";
    }
    return "unknown";
}

std::string InterpolationSamplingToString(tint::inspector::InterpolationSampling type) {
    switch (type) {
        case tint::inspector::InterpolationSampling::kUnknown:
            return "unknown";
        case tint::inspector::InterpolationSampling::kNone:
            return "none";
        case tint::inspector::InterpolationSampling::kCenter:
            return "center";
        case tint::inspector::InterpolationSampling::kCentroid:
            return "centroid";
        case tint::inspector::InterpolationSampling::kSample:
            return "sample";
    }
    return "unknown";
}

std::string OverrideTypeToString(tint::inspector::Override::Type type) {
    switch (type) {
        case tint::inspector::Override::Type::kBool:
            return "bool";
        case tint::inspector::Override::Type::kFloat32:
            return "f32";
        case tint::inspector::Override::Type::kFloat16:
            return "f16";
        case tint::inspector::Override::Type::kUint32:
            return "u32";
        case tint::inspector::Override::Type::kInt32:
            return "i32";
    }
    return "unknown";
}

}  // namespace tint::cmd
