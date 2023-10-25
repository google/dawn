#!/usr/bin/env python3
#
# Copyright 2022 The Dawn & Tint Authors
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

import argparse
import tarfile
import datetime
import os
import subprocess
import sys
import tempfile

script_root = os.path.dirname(os.path.abspath(sys.argv[0]))
dawn_root = os.path.abspath(os.path.join(script_root, "../.."))
build_dir = os.path.join(dawn_root, 'build')
if not os.path.isfile(os.path.join(build_dir, "find_depot_tools.py")):
    # try chromium path
    build_dir = os.path.abspath(os.path.join(dawn_root, "../../build"))
if not os.path.isfile(os.path.join(build_dir, "find_depot_tools.py")):
    raise SystemExit('could not find build directory')
sys.path.insert(0, build_dir)
import find_depot_tools

bucket = 'dawn-webgpu-cts-cache'


def cts_hash():
    deps_path = os.path.join(script_root, '../../third_party/webgpu-cts')
    hash = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=deps_path)
    return hash.decode('UTF-8').strip('\n')


def download_from_bucket(name, dst):
    gsutil = os.path.join(find_depot_tools.DEPOT_TOOLS_PATH, 'gsutil.py')
    subprocess.check_output(
        ['python3', gsutil, 'cp', 'gs://{}/{}'.format(bucket, name), dst],
        cwd=script_root)


def gen_cache(out_dir):
    # Obtain the current hash of the CTS repo
    hash = cts_hash()

    # Download the cache.tar.gz compressed data from the GCP bucket
    tmpDir = tempfile.TemporaryDirectory()
    cacheTarPath = os.path.join(tmpDir.name, 'cache.tar.gz')
    download_from_bucket(hash + "/data", cacheTarPath)

    # Extract the cache.tar.gz into out_dir
    tar = tarfile.open(cacheTarPath)
    tar.extractall(out_dir)

    # Update timestamps
    now = datetime.datetime.now().timestamp()
    for name in tar.getnames():
        path = os.path.join(out_dir, name)
        os.utime(path, (now, now))
    tar.close()


# Extract the cache for CTS runs.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    # TODO(bclayton): Unused. Remove
    parser.add_argument('js_script', help='Path to gen_cache.js')

    parser.add_argument('out_dir', help='Output directory for the cache')
    args = parser.parse_args()

    gen_cache(args.out_dir)
