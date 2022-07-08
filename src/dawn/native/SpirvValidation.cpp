// Copyright 2021 The Dawn Authors
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

#include "dawn/native/SpirvValidation.h"

#include <spirv-tools/libspirv.hpp>

#include <sstream>
#include <string>

#include "dawn/native/Device.h"

namespace dawn::native {

MaybeError ValidateSpirv(DeviceBase* device,
                         const uint32_t* spirv,
                         size_t wordCount,
                         bool dumpSpirv) {
    spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);
    spirvTools.SetMessageConsumer([device](spv_message_level_t level, const char*,
                                           const spv_position_t& position, const char* message) {
        WGPULoggingType wgpuLogLevel;
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                wgpuLogLevel = WGPULoggingType_Error;
                break;
            case SPV_MSG_WARNING:
                wgpuLogLevel = WGPULoggingType_Warning;
                break;
            case SPV_MSG_INFO:
                wgpuLogLevel = WGPULoggingType_Info;
                break;
            default:
                wgpuLogLevel = WGPULoggingType_Error;
                break;
        }

        std::ostringstream ss;
        ss << "SPIRV line " << position.index << ": " << message << std::endl;
        device->EmitLog(wgpuLogLevel, ss.str().c_str());
    });

    const bool valid = spirvTools.Validate(spirv, wordCount);
    if (dumpSpirv || !valid) {
        std::ostringstream dumpedMsg;
        std::string disassembly;
        if (spirvTools.Disassemble(
                spirv, wordCount, &disassembly,
                SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES | SPV_BINARY_TO_TEXT_OPTION_INDENT)) {
            dumpedMsg << "/* Dumped generated SPIRV disassembly */" << std::endl << disassembly;
        } else {
            dumpedMsg << "/* Failed to disassemble generated SPIRV */";
        }
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    DAWN_INVALID_IF(!valid, "Produced invalid SPIRV. Please file a bug at https://crbug.com/tint.");

    return {};
}

}  // namespace dawn::native
