#!/usr/bin/env python3
# Copyright 2026 The Dawn & Tint Authors
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
"""Runs the LiteRT-LM performance tests.

This script acts as a thin wrapper around the litert_lm_advanced_main binary
to support running it on Swarming with correct libraries and paths.

It runs each of the workloads with the binary: the default benchmark, and
some additional benchmarks and correctness checks. Each benchmark writes its metrics to
<workload>.pb in the output directory.
"""

import argparse
import dataclasses
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

REPO_ROOT = Path(__file__).resolve().parent.parent
LITERT_LM_DIR = REPO_ROOT / 'third_party' / 'litert-lm'
GEMMA4_2B_DIR = LITERT_LM_DIR / 'models' / 'gemma4_2b'
GEMMA4_2B_MODEL_PATH = GEMMA4_2B_DIR / 'model' / 'gemma4_2b.litertlm'


@dataclasses.dataclass
class Workload:
    """A single invocation of litert_lm_advanced_main."""

    # Name of the workload. Benchmarks write their metrics to <name>.pb.
    name: str
    model_path: Path
    args: list[str]


DEFAULT_ARGS = ['--async=true', '--clear_kv_cache_before_prefill=true']
DEFAULT_BENCHMARK_ARGS = DEFAULT_ARGS + [
    '--benchmark', '--report_peak_memory_footprint'
]
IMAGE_PROMPT = ('--input_prompt=Describe this image: '
                f"[image:{GEMMA4_2B_DIR / 'apple.jpg'}]")

WORKLOADS = [
    # Correctness check, fails if output of the model does not contain expected text.
    Workload(name='correctness_text_president',
             model_path=GEMMA4_2B_MODEL_PATH,
             args=DEFAULT_ARGS + [
                 '--input_prompt=Who is the first president of US? ',
                 '--expected_output=washington',
             ]),
    # Benchmarking workloads.
    Workload(name='default',
             model_path=LITERT_LM_DIR / 'data' / 'model.litertlm',
             args=[
                 '--benchmark',
                 '--benchmark_prefill_tokens=1024',
                 '--benchmark_decode_tokens=256',
                 '--num_iterations=6',
             ]),
    Workload(name='text_p1024_d256',
             model_path=GEMMA4_2B_MODEL_PATH,
             args=DEFAULT_BENCHMARK_ARGS + [
                 '--benchmark_prefill_tokens=1024',
                 '--benchmark_decode_tokens=256',
                 '--max_num_tokens=1535',
                 '--num_iterations=2',
             ]),
    Workload(name='text_p2048_d256',
             model_path=GEMMA4_2B_MODEL_PATH,
             args=DEFAULT_BENCHMARK_ARGS + [
                 '--benchmark_prefill_tokens=2048',
                 '--benchmark_decode_tokens=256',
                 '--num_iterations=2',
             ]),
    Workload(name='image',
             model_path=GEMMA4_2B_MODEL_PATH,
             args=DEFAULT_BENCHMARK_ARGS + [
                 IMAGE_PROMPT,
                 '--vision_backend=gpu',
                 '--expected_output=apple',
                 '--num_iterations=2',
             ]),
]


def run_litert_lm(
        workload: Workload,
        metric_proto_file_path: Path | None) -> subprocess.CompletedProcess:
    # The shared libraries (.so / .dylib) are isolated alongside the binary in the current
    # build out directory, so explicitly add them to the library path for execution on bots.
    env = os.environ.copy()
    env['LD_LIBRARY_PATH'] = f"{Path.cwd()}{os.pathsep}{env.get('LD_LIBRARY_PATH', '')}"
    env['DYLD_LIBRARY_PATH'] = f"{Path.cwd()}{os.pathsep}{env.get('DYLD_LIBRARY_PATH', '')}"

    # Disable host GPU driver shader disk caches so Init Executor deterministically
    # measures full cold shader compilation on every Swarming run.
    # NVIDIA shader cache.
    env['__GL_SHADER_DISK_CACHE'] = '0'

    # Mesa shader cache.
    env['MESA_SHADER_CACHE_DISABLE'] = 'true'

    # Mac shader cache.
    if sys.platform == 'darwin':
        try:
            darwin_cache = subprocess.check_output(
                ['getconf', 'DARWIN_USER_CACHE_DIR'], text=True).strip()
            for metal_cache in Path(darwin_cache).glob('**/com.apple.metal'):
                shutil.rmtree(metal_cache, ignore_errors=True)
        except Exception as e:
            print(f"Warning: Failed to clear Metal shader cache: {e}")

    binary_name = ('litert_lm_advanced_main.exe'
                   if sys.platform == 'win32' else 'litert_lm_advanced_main')
    binary_path = Path.cwd() / binary_name

    cmd = [
        str(binary_path),
        '--backend=gpu',
        '--disable_cache=true',
        # Run pipeline compilation and weight upload synchronously during Init
        # Executor so background threads do not race with and spike Prefill speed.
        # TODO(crbug.com/562993169): Remove num_threads_to_compile and
        # num_threads_to_upload if this can be fixed upstream since it seems like
        # a benchmark bug.
        '--num_threads_to_compile=0',
        '--num_threads_to_upload=0',
        f'--model_path={workload.model_path}',
    ] + workload.args
    if metric_proto_file_path:
        cmd.append(f'--metric_proto_file_path={metric_proto_file_path}')

    print(f"Executing {workload.name}: {' '.join(cmd)}")

    return subprocess.run(cmd, env=env, check=False)


def generate_results_for_resultdb_ingestion(failed_workloads: list[str],
                                            output_file: str) -> None:
    generated_test_results = {
        'failures': [],
        'valid': True,
    }
    for workload_name in failed_workloads:
        generated_test_results['failures'].append(
            f'litert_lm_advanced_main:{workload_name}')

    with open(output_file, 'w', encoding='utf-8') as outfile:
        json.dump(generated_test_results, outfile)


def main() -> None:
    parser = argparse.ArgumentParser(description='Runs LiteRT-LM benchmarks')

    parser.add_argument('--isolated-script-test-output',
                        help='Path to the location to output JSON results.')
    parser.add_argument('--isolated-script-test-perf-output',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-launcher-retry-limit',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-repeat',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-filter',
                        help='Currently unused, needed for bot support.')
    args = parser.parse_args()

    # Determine output directory for results.
    if args.isolated_script_test_output:
        output_dir = Path(args.isolated_script_test_output).parent
    else:
        output_dir = Path.cwd()

    # Run the workloads, and generate a metric results file for each.
    failed_workloads = []
    for workload in WORKLOADS:
        metric_file = None
        if '--benchmark' in workload.args:
            metric_file = output_dir / f'{workload.name}.pb'
        proc = run_litert_lm(workload, metric_file)
        if proc.returncode:
            failed_workloads.append(workload.name)
    if args.isolated_script_test_output:
        generate_results_for_resultdb_ingestion(
            failed_workloads, args.isolated_script_test_output)
    if failed_workloads:
        sys.exit(f"Failed workloads: {', '.join(failed_workloads)}")


if __name__ == '__main__':
    main()
