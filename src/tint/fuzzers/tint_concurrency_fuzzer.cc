// Copyright 2022 The Tint Authors.
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
#include <string>
#include <unordered_set>

#include <thread>

#include "src/tint/fuzzers/apply_substitute_overrides.h"
#include "src/tint/lang/glsl/writer/writer.h"
#include "src/tint/lang/hlsl/writer/writer.h"
#include "src/tint/lang/msl/writer/writer.h"
#include "src/tint/lang/spirv/writer/writer.h"
#include "src/tint/lang/wgsl/helpers/flatten_bindings.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "src/tint/utils/math/hash.h"

static constexpr size_t kNumThreads = 8;

[[noreturn]] void TintInternalCompilerErrorReporter(const tint::InternalCompilerError& err) {
    std::cerr << err.Error() << std::endl;
    __builtin_trap();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    tint::SetInternalCompilerErrorReporter(&TintInternalCompilerErrorReporter);

    std::string str(reinterpret_cast<const char*>(data), size);
    auto file = std::make_unique<tint::Source::File>("test.wgsl", str);
    auto program = tint::wgsl::reader::Parse(file.get());
    if (!program.IsValid()) {
        return 0;
    }

    program = tint::fuzzers::ApplySubstituteOverrides(std::move(program));
    if (!program.IsValid()) {
        return 0;
    }

    tint::inspector::Inspector inspector(&program);
    auto entry_points = inspector.GetEntryPoints();
    std::string entry_point = entry_points.empty() ? "" : entry_points.front().name;

    std::array<std::thread, kNumThreads> threads;

    for (size_t thread_idx = 0; thread_idx < kNumThreads; thread_idx++) {
        auto thread = std::thread([&program, thread_idx, entry_point] {
            enum class Writer {
#if TINT_BUILD_GLSL_WRITER
                kGLSL,
#endif
#if TINT_BUILD_HLSL_WRITER
                kHLSL,
#endif
#if TINT_BUILD_MSL_WRITER
                kMSL,
#endif
#if TINT_BUILD_SPV_WRITER
                kSPIRV,
#endif
#if TINT_BUILD_WGSL_WRITER
                kWGSL,
#endif
                kCount
            };
            switch (static_cast<Writer>(thread_idx % static_cast<size_t>(Writer::kCount))) {
#if TINT_BUILD_WGSL_WRITER
                case Writer::kWGSL: {
                    (void)tint::wgsl::writer::Generate(&program, {});
                    break;
                }
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_SPV_WRITER
                case Writer::kSPIRV: {
                    (void)tint::spirv::writer::Generate(&program, {});
                    break;
                }
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_HLSL_WRITER
                case Writer::kHLSL: {
                    (void)tint::hlsl::writer::Generate(&program, {});
                    break;
                }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_GLSL_WRITER
                case Writer::kGLSL: {
                    (void)tint::glsl::writer::Generate(&program, {}, entry_point);
                    break;
                }
#endif  // TINT_BUILD_GLSL_WRITER

#if TINT_BUILD_MSL_WRITER
                case Writer::kMSL: {
                    // Remap resource numbers to a flat namespace.
                    if (auto flattened = tint::writer::FlattenBindings(&program)) {
                        (void)tint::msl::writer::Generate(&flattened.value(), {});
                    }
                    break;
                }
#endif  // TINT_BUILD_MSL_WRITER

                case Writer::kCount:
                    break;
            }
        });
        threads[thread_idx] = std::move(thread);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
