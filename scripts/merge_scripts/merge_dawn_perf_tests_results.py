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
"""Merge script to aggregate dawn_perf_tests results and upload metrics to GCS.
"""

import json
from pathlib import Path
import sys
import tempfile

DAWN_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(DAWN_ROOT))

from scripts.merge_scripts import perf_results_common
from testing.merge_scripts import merge_api


def main() -> int:
    parser = merge_api.ArgumentParser()
    args = parser.parse_args()

    # Aggregate dawn_perf_results.json across all shards, since gtest
    # suites can be sharded.
    combined_perf_results = []
    for json_file_str in args.jsons_to_merge:
        isolated_outdir = Path(json_file_str).parent
        results_path = isolated_outdir / 'dawn_perf_results.json'
        if not results_path.exists():
            continue
        try:
            with open(results_path, 'r', encoding='utf-8') as f:
                shard_data = json.load(f)
            combined_perf_results.extend(shard_data.get('results', []))
        except Exception as e:
            print(f"Error reading {results_path}: {e}", file=sys.stderr)

    if not combined_perf_results:
        print("Warning: No dawn_perf_results.json results found to upload.",
              file=sys.stderr)
        return 0

    with tempfile.TemporaryDirectory() as tempdir_str:
        tempdir = Path(tempdir_str)
        merged_results_file = tempdir / 'dawn_perf_results.json'
        with open(merged_results_file, 'w', encoding='utf-8') as f:
            json.dump({'results': combined_perf_results}, f, indent=2)

        perf_results_common.upload_perf_results(
            test_suite='dawn_perf_tests',
            artifacts={'dawn_perf_results.json': merged_results_file},
            build_properties_str=getattr(args, 'build_properties', None),
        )

    return 0


if __name__ == '__main__':
    sys.exit(main())
