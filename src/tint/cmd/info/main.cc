
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

#include <iostream>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "src/tint/cmd/common/helper.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/inspector/entry_point.h"
#include "src/tint/utils/command/command.h"
#include "src/tint/utils/containers/transform.h"
#include "src/tint/utils/text/string.h"

namespace {

struct Options {
    bool show_help = false;

#if TINT_BUILD_SPV_READER
    tint::spirv::reader::Options spirv_reader_options;
#endif

    std::string input_filename;
    bool emit_json = false;
};

const char kUsage[] = R"(Usage: tint [options] <input-file>

 options:
   --json                    -- Emit JSON
   -h                        -- This help text

)";

bool ParseArgs(const std::vector<std::string>& args, Options* opts) {
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-h" || arg == "--help") {
            opts->show_help = true;
        } else if (arg == "--json") {
            opts->emit_json = true;
        } else if (!arg.empty()) {
            if (arg[0] == '-') {
                std::cerr << "Unrecognized option: " << arg << std::endl;
                return false;
            }
            if (!opts->input_filename.empty()) {
                std::cerr << "More than one input file specified: '" << opts->input_filename
                          << "' and '" << arg << "'" << std::endl;
                return false;
            }
            opts->input_filename = arg;
        }
    }
    return true;
}

void EmitJson(const tint::Program& program) {
    tint::inspector::Inspector inspector(program);

    std::cout << "{" << std::endl;
    std::cout << "\"extensions\": [" << std::endl;

    if (!inspector.GetUsedExtensionNames().empty()) {
        bool first = true;
        for (const auto& name : inspector.GetUsedExtensionNames()) {
            if (!first) {
                std::cout << ",";
            }
            first = false;
            std::cout << "\"" << name << "\"" << std::endl;
        }
    }
    std::cout << "]," << std::endl;

    std::cout << "\"entry_points\": [";

    auto stage_var = [&](const tint::inspector::StageVariable& var) {
        std::cout << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "\"name\": \"" << var.name << "\"";
        if (var.has_location_attribute) {
            std::cout << "," << std::endl;
            std::cout << "\"location\": " << var.location_attribute << "," << std::endl;
            std::cout << "\"component_type\": \""
                      << tint::cmd::ComponentTypeToString(var.component_type) << "\"," << std::endl;
            std::cout << "\"composition_type\": \""
                      << tint::cmd::CompositionTypeToString(var.composition_type) << "\","
                      << std::endl;
            std::cout << "\"interpolation\": {" << std::endl;
            std::cout << "\"type\": \""
                      << tint::cmd::InterpolationTypeToString(var.interpolation_type) << "\","
                      << std::endl;
            std::cout << "\"sampling\": \""
                      << tint::cmd::InterpolationSamplingToString(var.interpolation_sampling)
                      << "\"" << std::endl;
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
        std::cout << "}";
    };

    auto entry_points = inspector.GetEntryPoints();
    bool first = true;
    for (auto& entry_point : entry_points) {
        if (!first) {
            std::cout << ",";
        }
        first = false;

        std::cout << std::endl;
        std::cout << "{" << std::endl;

        std::cout << "\"name\": \"" << entry_point.name << "\""
                  << "," << std::endl;
        std::cout << "\"stage\": \"" << tint::cmd::EntryPointStageToString(entry_point.stage)
                  << "\""
                  << "," << std::endl;

        if (entry_point.workgroup_size) {
            std::cout << "\"workgroup_size\": [";
            std::cout << entry_point.workgroup_size->x << ", " << entry_point.workgroup_size->y
                      << ", " << entry_point.workgroup_size->z << "]"
                      << "," << std::endl;
        }

        std::cout << "\"input_variables\": [";
        bool input_first = true;
        for (const auto& var : entry_point.input_variables) {
            if (!input_first) {
                std::cout << ",";
            }
            input_first = false;
            stage_var(var);
        }
        std::cout << std::endl
                  << "]"
                  << "," << std::endl;

        std::cout << "\"output_variables\": [";
        bool output_first = true;
        for (const auto& var : entry_point.output_variables) {
            if (!output_first) {
                std::cout << ",";
            }
            output_first = false;
            stage_var(var);
        }
        std::cout << std::endl
                  << "]"
                  << "," << std::endl;

        std::cout << "\"overrides\": [";

        bool override_first = true;
        for (const auto& var : entry_point.overrides) {
            if (!override_first) {
                std::cout << ",";
            }
            override_first = false;

            std::cout << std::endl;
            std::cout << "{" << std::endl;
            std::cout << "\"name\": \"" << var.name << "\"," << std::endl;
            std::cout << "\"id\": " << var.id.value << "," << std::endl;
            std::cout << "\"type\": \"" << tint::cmd::OverrideTypeToString(var.type) << "\","
                      << std::endl;
            std::cout << "\"is_initialized\": " << (var.is_initialized ? "true" : "false") << ","
                      << std::endl;
            std::cout << "\"is_id_specified\": " << (var.is_id_specified ? "true" : "false")
                      << std::endl;
            std::cout << "}";
        }
        std::cout << std::endl
                  << "]"
                  << "," << std::endl;

        std::cout << "\"bindings\": [";
        auto bindings = inspector.GetResourceBindings(entry_point.name);
        bool ep_first = true;
        for (auto& binding : bindings) {
            if (!ep_first) {
                std::cout << ",";
            }
            ep_first = false;

            std::cout << std::endl;
            std::cout << "{" << std::endl;
            std::cout << "\"binding\": " << binding.binding << "," << std::endl;
            std::cout << "\"group\": " << binding.bind_group << "," << std::endl;
            std::cout << "\"size\": " << binding.size << "," << std::endl;
            std::cout << "\"resource_type\": \""
                      << tint::cmd::ResourceTypeToString(binding.resource_type) << "\","
                      << std::endl;
            std::cout << "\"dimemsions\": \"" << tint::cmd::TextureDimensionToString(binding.dim)
                      << "\"," << std::endl;
            std::cout << "\"sampled_kind\": \""
                      << tint::cmd::SampledKindToString(binding.sampled_kind) << "\"," << std::endl;
            std::cout << "\"image_format\": \""
                      << tint::cmd::TexelFormatToString(binding.image_format) << "\"" << std::endl;
            std::cout << "}";
        }
        std::cout << std::endl << "]" << std::endl;
        std::cout << "}";
    }
    std::cout << std::endl << "]," << std::endl;
    std::cout << "\"structures\": [";

    bool struct_first = true;
    for (const auto* ty : program.Types()) {
        if (!ty->Is<tint::core::type::Struct>()) {
            continue;
        }
        const auto* s = ty->As<tint::core::type::Struct>();

        if (!struct_first) {
            std::cout << ",";
        }
        struct_first = false;

        std::cout << std::endl;
        std::cout << "{" << std::endl;
        std::cout << "\"name\": \"" << s->FriendlyName() << "\"," << std::endl;
        std::cout << "\"align\": " << s->Align() << "," << std::endl;
        std::cout << "\"size\": " << s->Size() << "," << std::endl;
        std::cout << "\"members\": [";
        for (size_t i = 0; i < s->Members().Length(); ++i) {
            auto* const m = s->Members()[i];

            if (i != 0) {
                std::cout << ",";
            }
            std::cout << std::endl;

            // Output field alignment padding, if any
            auto* const prev_member = (i == 0) ? nullptr : s->Members()[i - 1];
            if (prev_member) {
                uint32_t padding = m->Offset() - (prev_member->Offset() + prev_member->Size());
                if (padding > 0) {
                    size_t padding_offset = m->Offset() - padding;
                    std::cout << "{" << std::endl;
                    std::cout << "\"name\": \"implicit_padding\"," << std::endl;
                    std::cout << "\"offset\": " << padding_offset << "," << std::endl;
                    std::cout << "\"align\": 1," << std::endl;
                    std::cout << "\"size\": " << padding << std::endl;
                    std::cout << "}," << std::endl;
                }
            }

            std::cout << "{" << std::endl;
            std::cout << "\"name\": \"" << m->Name().Name() << "\"," << std::endl;
            std::cout << "\"offset\": " << m->Offset() << "," << std::endl;
            std::cout << "\"align\": " << m->Align() << "," << std::endl;
            std::cout << "\"size\": " << m->Size() << std::endl;
            std::cout << "}";
        }
        std::cout << std::endl << "]" << std::endl;
        std::cout << "}";
    }
    std::cout << std::endl << "]" << std::endl;
    std::cout << "}" << std::endl;
}

void EmitText(const tint::Program& program) {
    tint::inspector::Inspector inspector(program);
    if (!inspector.GetUsedExtensionNames().empty()) {
        std::cout << "Extensions:" << std::endl;
        for (const auto& name : inspector.GetUsedExtensionNames()) {
            std::cout << "\t" << name << std::endl;
        }
    }
    std::cout << std::endl;

    tint::cmd::PrintInspectorData(inspector);

    bool has_struct = false;
    for (const auto* ty : program.Types()) {
        if (!ty->Is<tint::core::type::Struct>()) {
            continue;
        }
        has_struct = true;
        break;
    }

    if (has_struct) {
        std::cout << "Structures" << std::endl;
        for (const auto* ty : program.Types()) {
            if (!ty->Is<tint::core::type::Struct>()) {
                continue;
            }
            const auto* s = ty->As<tint::core::type::Struct>();
            std::cout << s->Layout() << std::endl << std::endl;
        }
    }
}

}  // namespace

int main(int argc, const char** argv) {
    std::vector<std::string> args(argv, argv + argc);
    Options options;

    tint::SetInternalCompilerErrorReporter(&tint::cmd::TintInternalCompilerErrorReporter);

    if (!ParseArgs(args, &options)) {
        std::cerr << "Failed to parse arguments." << std::endl;
        return 1;
    }

    if (options.show_help) {
        std::cout << kUsage << std::endl;
        return 0;
    }

    tint::cmd::LoadProgramOptions opts;
    opts.filename = options.input_filename;
#if TINT_BUILD_SPV_READER
    opts.spirv_reader_options = options.spirv_reader_options;
#endif

    auto info = tint::cmd::LoadProgramInfo(opts);

    if (options.emit_json) {
        EmitJson(info.program);
    } else {
        EmitText(info.program);
    }

    return 0;
}
