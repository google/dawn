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
"""Common code for perf results merging and uploading to GCS."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

DAWN_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_BUCKET = 'dawn-webgpu-perf-results'


def find_gsutil() -> list[str]:
    """Finds gsutil.py from depot_tools."""
    gsutil_path = DAWN_ROOT / 'third_party' / 'depot_tools' / 'gsutil.py'
    if not gsutil_path.exists():
        raise RuntimeError(f'Unable to find gsutil.py at {gsutil_path}')
    return [sys.executable, '-u', str(gsutil_path)]


def upload_file_to_gcs(local_file_path: Path, gcs_bucket: str,
                       gcs_dest_path: str) -> None:
    """Uploads a single local file to a GCS destination path."""
    if not local_file_path.exists():
        raise FileNotFoundError(f"Local file not found at: {local_file_path}")

    gsutil_cmd = find_gsutil()
    gcs_url = f"gs://{gcs_bucket}/{gcs_dest_path}"

    cmd = gsutil_cmd + ['cp', str(local_file_path), gcs_url]
    print(f"Uploading {local_file_path.name} to GCS: {' '.join(cmd)}")
    subprocess.run(cmd, check=True)


def generate_metadata(metadata_file_path: Path, timestamp: int,
                      build_properties_str: str | None) -> None:
    """Generates metadata.json containing builder and commit details."""
    props = {}
    if build_properties_str:
        try:
            props = json.loads(build_properties_str)
        except Exception as e:
            print(f"Warning: Failed to parse build-properties: {e}",
                  file=sys.stderr)

    metadata = {
        'timestamp': timestamp,
        'git_revision': props.get('got_revision'),
        'buildername': props.get('buildername'),
        'builder_group': props.get('builder_group'),
    }

    with open(metadata_file_path, 'w', encoding='utf-8') as f:
        json.dump(metadata, f, indent=2)


def get_run_id(timestamp: int) -> str:
    """Generates a run ID incorporating the timestamp and swarming task ID."""
    task_id = os.environ.get('SWARMING_TASK_ID', 'local')
    return f"{timestamp}_{task_id}"


def upload_perf_results(
    test_suite: str,
    artifacts: dict[str, Path],
    build_properties_str: str | None = None,
    bucket_name: str = DEFAULT_BUCKET,
) -> None:
    """Uploads run artifacts and metadata to GCS under test_suite/run_id/."""
    timestamp = int(time.time())
    run_id = get_run_id(timestamp)
    directory_name = f"{test_suite}/{run_id}"

    with tempfile.TemporaryDirectory() as tempdir_str:
        tempdir = Path(tempdir_str)
        metadata_file = tempdir / 'metadata.json'
        generate_metadata(metadata_file, timestamp, build_properties_str)

        upload_file_to_gcs(metadata_file, bucket_name,
                           f"{directory_name}/metadata.json")

        for dest_filename, local_path in artifacts.items():
            upload_file_to_gcs(local_path, bucket_name,
                               f"{directory_name}/{dest_filename}")
