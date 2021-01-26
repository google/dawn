// Copyright 2018 The Dawn Authors
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

#include <csetjmp>
#include <csignal>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include <spirv-tools/libspirv.hpp>
#ifdef DAWN_ENABLE_WGSL
#    include <tint/tint.h>
#endif  // DAWN_ENABLE_WGSL

#include "DawnSPIRVCrossFuzzer.h"

namespace {

    std::jmp_buf jump_buffer;
    void (*old_signal_handler)(int);

    // Handler to trap signals, so that it doesn't crash the fuzzer when running
    // the code under test. The code being fuzzed uses abort() to report errors
    // like bad input instead of returning an error code.
    [[noreturn]] static void sigabrt_trap(int) {
        std::longjmp(jump_buffer, 1);
    }

    // Setup the SIGABRT trap
    void BeginSIGABRTTrap() {
        old_signal_handler = signal(SIGABRT, sigabrt_trap);
        if (old_signal_handler == SIG_ERR)
            abort();
    }

    // Restore the previous signal handler
    void EndSIGABRTTrap() {
        signal(SIGABRT, old_signal_handler);
    }

}  // namespace

namespace DawnSPIRVCrossFuzzer {

    void ExecuteWithSignalTrap(std::function<void()> exec) {
        BeginSIGABRTTrap();

        // On the first pass through setjmp will return 0, if returning here
        // from the longjmp in the signal handler it will return 1.
        if (setjmp(jump_buffer) == 0) {
            exec();
        }

        EndSIGABRTTrap();
    }

    int Run(const uint8_t* data, size_t size, Task task) {
        size_t sizeInU32 = size / sizeof(uint32_t);
        const uint32_t* u32Data = reinterpret_cast<const uint32_t*>(data);
        std::vector<uint32_t> input(u32Data, u32Data + sizeInU32);

        // Using Tint SPIRV->SPIRV to normalize inputs if supported.
#ifdef DAWN_ENABLE_WGSL
        tint::reader::spirv::Parser parser(input);

        if (!parser.Parse()) {
            return 0;
        }

        tint::Program program = parser.program();
        if (!program.IsValid()) {
            return 0;
        }

        tint::TypeDeterminer type_determiner(&program);
        if (!type_determiner.Determine()) {
            return 0;
        }

       tint::writer::spirv::Generator generator(&program);
        if (!generator.Generate()) {
            return 0;
        }

        std::vector<uint32_t> spirv = generator.result();
#else
        std::vector<uint32_t> spirv = std::move(input);
#endif

        spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);
        spirvTools.SetMessageConsumer(
            [](spv_message_level_t, const char*, const spv_position_t&, const char*) {});

        // Dawn is responsible to validating inputs before they go to
        // SPIRV-Cross.
        if (!spirvTools.Validate(spirv.data(), spirv.size())) {
            return 0;
        }

        if (spirv.size() != 0) {
            task(spirv);
        }

        return 0;
    }

}  // namespace DawnSPIRVCrossFuzzer
