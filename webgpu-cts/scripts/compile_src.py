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

import os
import shutil
import sys

from dir_paths import webgpu_cts_root_dir, node_dir
from tsc_ignore_errors import run_tsc_ignore_errors

try:
    old_sys_path = sys.path
    sys.path = [node_dir] + sys.path

    from node import RunNode
finally:
    sys.path = old_sys_path


def compile_src(out_dir):
    # First, clean the output directory so deleted files are pruned from old builds.
    shutil.rmtree(out_dir, ignore_errors=True)

    run_tsc_ignore_errors([
        "--project",
        os.path.join(webgpu_cts_root_dir, "tsconfig.json"),
        "--outDir",
        out_dir,
        "--noEmit",
        "false",
        "--noEmitOnError",
        "false",
        "--declaration",
        "false",
        "--sourceMap",
        "false",
        "--target",
        "ES2017",
    ])


def compile_src_for_node(out_dir, additional_args=None, clean=True):
    additional_args = additional_args or []
    if clean:
        # First, clean the output directory so deleted files are pruned from old builds.
        shutil.rmtree(out_dir, ignore_errors=True)

    args = [
        "--project",
        os.path.join(webgpu_cts_root_dir, "node.tsconfig.json"),
        "--outDir",
        out_dir,
        "--noEmit",
        "false",
        "--noEmitOnError",
        "false",
        "--declaration",
        "false",
        "--sourceMap",
        "false",
        "--target",
        "ES6",
    ]
    args.extend(additional_args)

    run_tsc_ignore_errors(args)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: compile_src.py GEN_DIR")
        sys.exit(1)

    gen_dir = sys.argv[1]

    # Compile the CTS src.
    compile_src(os.path.join(gen_dir, "src"))
    compile_src_for_node(os.path.join(gen_dir, "src-node"))

    # Run gen_listings.js to overwrite the placeholder src/webgpu/listings.js created
    # from transpiling src/
    RunNode([
        os.path.join(gen_dir, "src-node", "common", "tools",
                     "gen_listings.js"),
        "--no-validate",
        os.path.join(gen_dir, "src"),
        os.path.join(gen_dir, "src-node", "webgpu"),
    ])
