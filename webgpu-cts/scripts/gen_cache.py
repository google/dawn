#!/usr/bin/env python3
#
# Copyright 2022 The Dawn Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import sys

from dir_paths import node_dir


def gen_cache(js_script, out_dir):
    old_sys_path = sys.path
    try:
        sys.path = old_sys_path + [node_dir]
        from node import RunNode
    finally:
        sys.path = old_sys_path

    # Save the cwd. gen_cache.js needs to be run from a specific directory.
    cwd = os.getcwd()
    cts_dir = os.path.realpath(
        os.path.join(cwd, os.path.dirname(js_script), '..', '..', '..'))
    os.chdir(cts_dir)
    RunNode([
        os.path.join(cwd, js_script),
        os.path.join(cwd, out_dir),
        os.path.join('src-node', 'webgpu')
    ])


# Generate a cache for CTS runs.
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('js_script', help='Path to gen_cache.js')
    parser.add_argument('out_dir', help='Output directory for the cache')
    args = parser.parse_args()

    gen_cache(args.js_script, args.out_dir)
