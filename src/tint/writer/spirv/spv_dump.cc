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

#include "src/tint/writer/spirv/spv_dump.h"

#include "spirv-tools/libspirv.hpp"
#include "src/tint/writer/spirv/binary_writer.h"

namespace tint::writer::spirv {

std::string Disassemble(const std::vector<uint32_t>& data) {
    std::string spv_errors;
    spv_target_env target_env = SPV_ENV_UNIVERSAL_1_0;

    auto msg_consumer = [&spv_errors](spv_message_level_t level, const char*,
                                      const spv_position_t& position, const char* message) {
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                spv_errors +=
                    "error: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_WARNING:
                spv_errors +=
                    "warning: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_INFO:
                spv_errors +=
                    "info: line " + std::to_string(position.index) + ": " + message + "\n";
                break;
            case SPV_MSG_DEBUG:
                break;
        }
    };

    spvtools::SpirvTools tools(target_env);
    tools.SetMessageConsumer(msg_consumer);

    std::string result;
    if (!tools.Disassemble(data, &result, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER)) {
        return "*** Invalid SPIR-V ***\n" + spv_errors;
    }
    return result;
}

std::string DumpBuilder(Builder& builder) {
    return DumpModule(builder.Module());
}

std::string DumpModule(Module& module) {
    BinaryWriter writer;
    writer.WriteHeader(module.IdBound());
    writer.WriteModule(&module);
    return Disassemble(writer.result());
}

std::string DumpInstruction(const Instruction& inst) {
    BinaryWriter writer;
    writer.WriteHeader(kDefaultMaxIdBound);
    writer.WriteInstruction(inst);
    return Disassemble(writer.result());
}

std::string DumpInstructions(const InstructionList& insts) {
    BinaryWriter writer;
    writer.WriteHeader(kDefaultMaxIdBound);
    for (const auto& inst : insts) {
        writer.WriteInstruction(inst);
    }
    return Disassemble(writer.result());
}

}  // namespace tint::writer::spirv
