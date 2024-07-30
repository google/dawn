#!/usr/bin/env python3

# Copyright 2024 The Dawn & Tint Authors
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""
Generates a header file that declares all of the Tint benchmark programs as embedded WGSL and
SPIR-V shaders, and declares macros that will be used to register them all with Google Benchmark.

The SPIR-V shaders are emitted as an array of uint32_t values.

Usage:
   generate_benchmark_inputs.py <build_dir_path> <header_output_path>
"""

import argparse
import struct
import sys
from os import path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('build_dir_path')
    parser.add_argument('header_output_path')
    args = parser.parse_args()

    full_path_to_header = args.build_dir_path + '/' + args.header_output_path

    script_dir = path.dirname(path.realpath(__file__))
    benchmark_dir = script_dir + '/../../../../test/tint/benchmark'

    # The list of benchmark inputs.
    benchmark_files = [
        "atan2-const-eval.wgsl",
        "cluster-lights.wgsl",
        "metaball-isosurface.wgsl",
        "particles.wgsl",
        "shadow-fragment.wgsl",
        "skinned-shadowed-pbr-fragment.wgsl",
        "skinned-shadowed-pbr-vertex.wgsl",
    ]

    # Generate the header file.
    with open(full_path_to_header, 'w') as output:
        print('''// AUTOMATICALLY GENERATED, DO NOT MODIFY.

#ifndef SRC_TINT_CMD_BENCH_BENCHMARK_INPUTS_H_
#define SRC_TINT_CMD_BENCH_BENCHMARK_INPUTS_H_

#include <cstdint>
#include <vector>

namespace tint::bench {

struct BenchmarkInput {
    const char* name = nullptr;
    const std::string wgsl;
    const std::vector<uint32_t> spirv;
};
const BenchmarkInput kBenchmarkInputs[] = {''',
              file=output)

        # Add an entry to the array for each benchmark.
        for f in benchmark_files:
            if f.endswith('.wgsl'):
                # WGSL shaders are emitted as char initializer lists.
                with open(benchmark_dir + '/' + f, 'rb') as input:
                    print(f'    {{"{f}", {{', file=output, end='')
                    for char in input.read():
                        print(char, file=output, end=', ')
                    print(f'}}}},', file=output)
            elif f.endswith('.spv'):
                # SPIR-V shaders are emitted as uint32_t initializer lists.
                with open(benchmark_dir + '/' + f, 'rb') as input:
                    print(f'    {{"{f}", nullptr, {{', file=output, end='')
                    content = input.read()
                    for word in struct.unpack(
                            "<" + ("I" * ((len(content)) // 4)), content):
                        print(f'{word}', file=output, end=', ')
                    print(f'}}}},', file=output)
            else:
                print('unhandled file extension: ' + f)
                return 1

        print('};', file=output)
        print('', file=output)

        # Define the macro that registers each of the inputs with Google Benchmark.
        print('#define TINT_BENCHMARK_PROGRAMS(FUNC) \\', file=output)
        for f in sorted(benchmark_files):
            print(f'    BENCHMARK_CAPTURE(FUNC, {f}, "{f}"); \\', file=output)
        print('    TINT_REQUIRE_SEMICOLON', file=output)
        print('', file=output)

        print('''
}  // namespace tint::bench

#endif  // SRC_TINT_CMD_BENCH_BENCH_H_''',
              file=output)

    # Generate a depfile.
    with open(full_path_to_header + '.d', 'w') as depfile:
        print(args.header_output_path + ": \\", file=depfile)
        for f in benchmark_files:
            print("\t" + benchmark_dir + "/" + f + " \\", file=depfile)


if __name__ == "__main__":
    sys.exit(main())
