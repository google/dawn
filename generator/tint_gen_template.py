#!/usr/bin/env python3
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
import subprocess
import sys


def run_generator():
    parser = argparse.ArgumentParser(
        description=
        "Tint template generator for GN build. Use tools/run gen for non-GN build.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument('--output',
                        default='',
                        type=str,
                        help='Base output directory.')
    parser.add_argument('--template',
                        default='',
                        type=str,
                        help='Template to generate.')
    args = parser.parse_args()

    root = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..')
    go = os.path.join(root, "tools", "golang", "bin", "go")
    if sys.platform == 'win32':
        go += '.exe'

    gen = os.path.join(root, "tools", "src", "cmd", "gen")
    subprocess.check_call(
        [go, "run", gen, "-o", args.output,
         os.path.join(root, args.template)],
        cwd=root)

    return 0


if __name__ == '__main__':
    sys.exit(run_generator())
