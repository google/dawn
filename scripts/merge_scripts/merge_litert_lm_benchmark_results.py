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
"""Merge script to aggregate LiteRT-LM benchmark results and upload metrics to GCS.
"""

import json
from pathlib import Path
import sys

DAWN_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(DAWN_ROOT))

from scripts.merge_scripts import perf_results_common
from testing.merge_scripts import merge_api


def main() -> int:
    parser = merge_api.ArgumentParser()
    args = parser.parse_args()

    # This benchmark is designed to run only on a single shard.
    # Raise an error if more than one shard output is passed to be merged.
    if len(args.jsons_to_merge) > 1:
        raise ValueError(
            f"Expected exactly 1 shard for litert_lm_benchmark, "
            f"but found {len(args.jsons_to_merge)} shards: {args.jsons_to_merge}"
        )

    # Merge standard test result JSONs (for exactly 0 or 1 shard).
    merged_results = {
        'failures': [],
        'valid': True,
    }
    if not args.jsons_to_merge:
        print("Error: No shard output files provided to merge.",
              file=sys.stderr)
        merged_results['failures'].append('missing_shard_output')
        merged_results['valid'] = False

    for json_file_str in args.jsons_to_merge:
        json_file = Path(json_file_str)
        if not json_file.exists():
            print(f"Error: Shard output file '{json_file}' does not exist.",
                  file=sys.stderr)
            merged_results['failures'].append('missing_shard_output')
            merged_results['valid'] = False
            continue
        try:
            with open(json_file, 'r', encoding='utf-8') as f:
                results = json.load(f)
            if results.get('failures'):
                merged_results['failures'].extend(results['failures'])
            if not results.get('valid', True):
                merged_results['valid'] = False
        except Exception as e:
            print(f"Error reading {json_file}: {e}", file=sys.stderr)
            merged_results['valid'] = False

    # Write the combined results JSON required by the merge API.
    output_json_path = Path(args.output_json)
    with open(output_json_path, 'w', encoding='utf-8') as f:
        json.dump(merged_results, f, indent=2)

    # Locate litert_lm_metrics.pb from the single shard output.
    metric_file = None
    if args.jsons_to_merge:
        json_file = Path(args.jsons_to_merge[0])
        isolated_outdir = json_file.parent
        pb_path = isolated_outdir / 'litert_lm_metrics.pb'
        if pb_path.exists():
            metric_file = pb_path

    if not metric_file:
        print("Error: No litert_lm_metrics.pb file found to upload.",
              file=sys.stderr)
        return 1

    # Upload metrics and generated metadata to GCS.
    perf_results_common.upload_perf_results(
        test_suite='litert_lm_benchmark',
        artifacts={'litert_lm_metrics.pb': metric_file},
        build_properties_str=getattr(args, 'build_properties', None),
    )

    return 0


if __name__ == '__main__':
    sys.exit(main())
