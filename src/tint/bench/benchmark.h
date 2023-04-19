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

#ifndef SRC_TINT_BENCH_BENCHMARK_H_
#define SRC_TINT_BENCH_BENCHMARK_H_

#include <memory>
#include <string>
#include <variant>

#include "benchmark/benchmark.h"
#include "src/tint/utils/concat.h"
#include "tint/tint.h"

namespace tint::bench {

/// Error indicates an operation did not complete successfully.
struct Error {
    /// The error message.
    std::string msg;
};

/// ProgramAndFile holds a Program and a Source::File.
struct ProgramAndFile {
    /// The tint program parsed from file.
    Program program;
    /// The source file
    Source::File file;
};

/// LoadInputFile attempts to load a benchmark input file with the given file
/// name.
/// @param name the file name
/// @returns either the loaded Source::File or an Error
std::variant<Source::File, Error> LoadInputFile(std::string name);

/// LoadInputFile attempts to load a benchmark input program with the given file
/// name.
/// @param name the file name
/// @returns either the loaded Program or an Error
std::variant<ProgramAndFile, Error> LoadProgram(std::string name);

// If TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER is defined, include that to
// declare the TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS() macro, which appends
// external programs to the TINT_BENCHMARK_WGSL_PROGRAMS() list.
#ifdef TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER
#include TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAM_HEADER
#else
#define TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(x)
#endif

/// Declares a benchmark with the given function and WGSL file name
#define TINT_BENCHMARK_WGSL_PROGRAM(FUNC, WGSL_NAME) BENCHMARK_CAPTURE(FUNC, WGSL_NAME, WGSL_NAME);

/// Declares a set of benchmarks for the given function using a list of WGSL
/// files in `<tint>/test/benchmark`.
#define TINT_BENCHMARK_WGSL_PROGRAMS(FUNC)                                   \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "animometer.wgsl");                    \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "atan2-const-eval.wgsl");              \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "bloom-vertical-blur.wgsl");           \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "cluster-lights.wgsl");                \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "empty.wgsl");                         \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "metaball-isosurface.wgsl");           \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "particles.wgsl");                     \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "shadow-fragment.wgsl");               \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple-compute.wgsl");                \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple-fragment.wgsl");               \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple-vertex.wgsl");                 \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "skinned-shadowed-pbr-fragment.wgsl"); \
    TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "skinned-shadowed-pbr-vertex.wgsl");   \
    TINT_BENCHMARK_EXTERNAL_WGSL_PROGRAMS(FUNC)

}  // namespace tint::bench

#endif  // SRC_TINT_BENCH_BENCHMARK_H_
