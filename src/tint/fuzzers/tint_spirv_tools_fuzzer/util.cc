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

#include <fstream>
#include <iostream>

#include "src/tint/fuzzers/tint_spirv_tools_fuzzer/util.h"

namespace tint::fuzzers::spvtools_fuzzer::util {
namespace {

bool WriteBinary(const std::string& path, const uint8_t* data, size_t size) {
    std::ofstream spv(path, std::ios::binary);
    return spv &&
           spv.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
}

void LogError(uint32_t index,
              const std::string& type,
              const std::string& message,
              const std::string* path,
              const uint8_t* data,
              size_t size,
              const std::string* wgsl) {
    std::cout << index << " | " << type << ": " << message << std::endl;

    if (path) {
        auto prefix = *path + std::to_string(index);
        std::ofstream(prefix + ".log") << message << std::endl;

        WriteBinary(prefix + ".spv", data, size);

        if (wgsl) {
            std::ofstream(prefix + ".wgsl") << *wgsl << std::endl;
        }
    }
}

}  // namespace

spvtools::MessageConsumer GetBufferMessageConsumer(std::stringstream* buffer) {
    return [buffer](spv_message_level_t level, const char*, const spv_position_t& position,
                    const char* message) {
        std::string status;
        switch (level) {
            case SPV_MSG_FATAL:
            case SPV_MSG_INTERNAL_ERROR:
            case SPV_MSG_ERROR:
                status = "ERROR";
                break;
            case SPV_MSG_WARNING:
            case SPV_MSG_INFO:
            case SPV_MSG_DEBUG:
                status = "INFO";
                break;
        }
        *buffer << status << " " << position.line << ":" << position.column << ":" << position.index
                << ": " << message << std::endl;
    };
}

void LogMutatorError(const Mutator& mutator, const std::string& error_dir) {
    static uint32_t mutator_count = 0;
    auto error_path = error_dir.empty() ? error_dir : error_dir + "/mutator/";
    mutator.LogErrors(error_dir.empty() ? nullptr : &error_path, mutator_count++);
}

void LogWgslError(const std::string& message,
                  const uint8_t* data,
                  size_t size,
                  const std::string& wgsl,
                  OutputFormat output_format,
                  const std::string& error_dir) {
    static uint32_t wgsl_count = 0;
    std::string error_type;
    switch (output_format) {
        case OutputFormat::kSpv:
            error_type = "WGSL -> SPV";
            break;
        case OutputFormat::kMSL:
            error_type = "WGSL -> MSL";
            break;
        case OutputFormat::kHLSL:
            error_type = "WGSL -> HLSL";
            break;
        case OutputFormat::kWGSL:
            error_type = "WGSL -> WGSL";
            break;
    }
    auto error_path = error_dir.empty() ? error_dir : error_dir + "/wgsl/";
    LogError(wgsl_count++, error_type, message, error_dir.empty() ? nullptr : &error_path, data,
             size, &wgsl);
}

void LogSpvError(const std::string& message,
                 const uint8_t* data,
                 size_t size,
                 const std::string& error_dir) {
    static uint32_t spv_count = 0;
    auto error_path = error_dir.empty() ? error_dir : error_dir + "/spv/";
    LogError(spv_count++, "SPV -> WGSL", message, error_dir.empty() ? nullptr : &error_path, data,
             size, nullptr);
}

bool ReadBinary(const std::string& path, std::vector<uint32_t>* out) {
    if (!out) {
        return false;
    }

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    size_t size = static_cast<size_t>(file.tellg());
    if (!file) {
        return false;
    }

    file.seekg(0);
    if (!file) {
        return false;
    }

    std::vector<char> binary(size);
    if (!file.read(binary.data(), size)) {
        return false;
    }

    out->resize(binary.size() / sizeof(uint32_t));
    std::memcpy(out->data(), binary.data(), binary.size());
    return true;
}

bool WriteBinary(const std::string& path, const std::vector<uint32_t>& binary) {
    return WriteBinary(path, reinterpret_cast<const uint8_t*>(binary.data()),
                       binary.size() * sizeof(uint32_t));
}

}  // namespace tint::fuzzers::spvtools_fuzzer::util
