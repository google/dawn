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

#ifndef SRC_BENCHMARK_BENCHMARK_H_
#define SRC_BENCHMARK_BENCHMARK_H_

#include <memory>
#include <string>
#include <variant>  // NOLINT: Found C system header after C++ system header.

#include "benchmark/benchmark.h"
#include "src/utils/concat.h"
#include "tint/tint.h"

namespace tint::benchmark {

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

/// Declares a benchmark with the given function and WGSL file name
#define TINT_BENCHMARK_WGSL_PROGRAM(FUNC, WGSL_NAME) \
  BENCHMARK_CAPTURE(FUNC, WGSL_NAME, WGSL_NAME);

/// Declares a set of benchmarks for the given function using a list of WGSL
/// files in `<tint>/test/benchmark`.
#define TINT_BENCHMARK_WGSL_PROGRAMS(FUNC)                   \
  TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "empty.wgsl");           \
  TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "particles.wgsl");       \
  TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple_fragment.wgsl"); \
  TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple_vertex.wgsl");   \
  TINT_BENCHMARK_WGSL_PROGRAM(FUNC, "simple_compute.wgsl");

}  // namespace tint::benchmark

#endif  // SRC_BENCHMARK_BENCHMARK_H_
