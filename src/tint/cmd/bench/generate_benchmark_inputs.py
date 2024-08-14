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
Generates a header file that declares all of the Tint benchmark programs as embedded WGSL shaders,
and declares macros that will be used to register them all with Google Benchmark.

The SPIR-V shaders are converted to WGSL using Tint before being emitted.

Usage:
   generate_benchmark_inputs.py <tint>
"""

import argparse
import subprocess
import sys
import tempfile
from os import path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('tint')
    args = parser.parse_args()

    script_dir = path.dirname(path.realpath(__file__))
    benchmark_dir = script_dir + '/../../../../test/tint/benchmark'
    header_output_path = script_dir + '/inputs_bench.h'

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
    with open(header_output_path, 'w') as output:
        print('''// Copyright 2024 The Dawn & Tint Authors
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

// AUTOMATICALLY GENERATED, DO NOT MODIFY DIRECTLY.
// Regenerate using `python3 src/tint/cmd/bench/generate_benchmark_inputs.py /path/to/tint`.

#ifndef SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_
#define SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_

#include <string>
#include <unordered_map>

// clang-format off

namespace tint::bench {

const std::unordered_map<std::string, std::string> kBenchmarkInputs = {''',
              file=output)

        # Helper to emit a WGSL shader as a char initializer list.
        def emit_wgsl(input):
            print(f'    {{"{f}", {{', file=output, end='')
            for char in input.read():
                print(char, file=output, end=', ')
            print(f'}}}},', file=output)

        # Create a temporary directory for converting SPIR-V shaders to WGSL.
        with tempfile.TemporaryDirectory() as tmp:
            # Add an entry to the array for each benchmark.
            for f in benchmark_files:
                if f.endswith('.wgsl'):
                    # Emit the WGSL shader as is.
                    with open(benchmark_dir + '/' + f, 'rb') as input:
                        emit_wgsl(input)
                elif f.endswith('.spv'):
                    # Convert SPIR-V inputs to WGSL using Tint.
                    tmpwgsl = tmp + '/tmp.wgsl'
                    tint_args = [
                        args.tint, '-o', tmpwgsl, '--format', 'wgsl', f,
                        '--allow-non-uniform-derivatives'
                    ]
                    subprocess.run(tint_args, check=True)
                    with open(tmpwgsl, 'rb') as input:
                        emit_wgsl(input)
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

// clang-format on

#endif  // SRC_TINT_CMD_BENCH_INPUTS_BENCH_H_''',
              file=output)


if __name__ == "__main__":
    sys.exit(main())
