// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_CMD_BENCH_BENCH_H_
#define SRC_TINT_CMD_BENCH_BENCH_H_

#include <memory>
#include <string>
#include <variant>

#include "benchmark/benchmark.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/macros/concat.h"
#include "src/tint/utils/result/result.h"

namespace tint::bench {

/// ProgramAndFile holds a Program and a Source::File.
struct ProgramAndFile {
    /// The tint program parsed from file.
    Program program;
    /// The source file
    std::unique_ptr<Source::File> file;
};

/// Initializes the internal state for benchmarking.
/// Must be called once by the benchmark executable entry point.
/// @returns true on success, false of failure
bool Initialize();

/// LoadInputFile attempts to load a benchmark input file with the given file
/// name. Accepts files with the .wgsl and .spv extension.
/// SPIR-V files are automatically converted to WGSL.
/// @param name the file name
/// @returns the loaded Source::File
Result<Source::File> LoadInputFile(std::string name);

/// LoadInputFile attempts to load a benchmark input program with the given file
/// name.
/// @param name the file name
/// @returns the loaded Program
Result<ProgramAndFile> LoadProgram(std::string name);

// If TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER is defined, include that to
// declare the TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS() and TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS()
// macros, which appends external programs to the TINT_BENCHMARK_WGSL_PROGRAMS() and
// TINT_BENCHMARK_SPV_PROGRAMS() list.
#ifdef TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER
#include TINT_BENCHMARK_EXTERNAL_SHADERS_HEADER
#else
#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(x)
#define TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS(x)
#endif

/// Declares a benchmark with the given function and WGSL file name
#define TINT_BENCHMARK_WGSL_PROGRAM(FUNC, WGSL_NAME) BENCHMARK_CAPTURE(FUNC, WGSL_NAME, WGSL_NAME)

/// Declares a set of benchmarks for the given function using a list of WGSL files.
#define TINT_BENCHMARK_WGSL_PROGRAMS(FUNC)                                   \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "atan2-const-eval.wgsl");              \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "cluster-lights.wgsl");                \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "metaball-isosurface.wgsl");           \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "particles.wgsl");                     \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "shadow-fragment.wgsl");               \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "skinned-shadowed-pbr-fragment.wgsl"); \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "skinned-shadowed-pbr-vertex.wgsl");   \
    TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC)

/// Declares a set of benchmarks for the given function using a list of SPIR-V files.
#define TINT_BENCHMARK_SPV_PROGRAMS(FUNC) TINT_BENCHMARK_EXTERNAL_SPV_PROGRAMS(FUNC)

/// Declares a set of benchmarks for the given function using a list of WGSL and SPIR-V files.
#define TINT_BENCHMARK_PROGRAMS(FUNC)  \
    TINT_BENCHMARK_WGSL_PROGRAMS(FUNC) \
    TINT_BENCHMARK_SPV_PROGRAMS(FUNC)  \
    TINT_REQUIRE_SEMICOLON

}  // namespace tint::bench

#endif  // SRC_TINT_CMD_BENCH_BENCH_H_
