
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

#include <iostream>

#if TINT_BUILD_SPV_READER
#include "spirv-tools/libspirv.hpp"
#endif  // TINT_BUILD_SPV_READER

#include "src/tint/ast/module.h"
#include "src/tint/cmd/helper.h"
#include "src/tint/type/struct.h"
#include "src/tint/utils/io/command.h"
#include "src/tint/utils/string.h"
#include "src/tint/utils/transform.h"
#include "src/tint/val/val.h"
#include "tint/tint.h"

namespace {

struct Options {
    bool show_help = false;

#if TINT_BUILD_SPV_READER
    tint::reader::spirv::Options spirv_reader_options;
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

void EmitJson(const tint::Program* program) {
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
    for (const auto* ty : program->Types()) {
        if (!ty->Is<tint::type::Struct>()) {
            continue;
        }
        const auto* s = ty->As<tint::type::Struct>();

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

void EmitText(const tint::Program* program) {
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
    for (const auto* ty : program->Types()) {
        if (!ty->Is<tint::type::Struct>()) {
            continue;
        }
        has_struct = true;
        break;
    }

    if (has_struct) {
        std::cout << "Structures" << std::endl;
        for (const auto* ty : program->Types()) {
            if (!ty->Is<tint::type::Struct>()) {
                continue;
            }
            const auto* s = ty->As<tint::type::Struct>();
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

    auto diag_printer = tint::diag::Printer::create(stderr, true);
    tint::diag::Formatter diag_formatter;

    std::unique_ptr<tint::Program> program;
    std::unique_ptr<tint::Source::File> source_file;

    {
        tint::cmd::LoadProgramOptions opts;
        opts.filename = options.input_filename;
#if TINT_BUILD_SPV_READER
        opts.spirv_reader_options = options.spirv_reader_options;
#endif

        auto info = tint::cmd::LoadProgramInfo(opts);
        program = std::move(info.program);
        source_file = std::move(info.source_file);
    }

    if (options.emit_json) {
        EmitJson(program.get());
    } else {
        EmitText(program.get());
    }

    return 0;
}
