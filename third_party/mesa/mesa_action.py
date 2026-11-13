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
"""Helper wrapper for Mesa GN and CMake codegen actions.

This script serves three roles for the native GN and CMake Mesa builds:
1. Bridging to vpython3 (--vpython-spec): Dawn's root .gn configures
   `script_executable` to use the bare hermetic CPython interpreter
   (//third_party/cpython3/host/bin/python3), which does not automatically load
   Dawn's root `vpython.toml`. Similarly, CMake invokes `${Python3_EXECUTABLE}`
   directly. Mesa's Python codegen scripts require third-party packages like
   `mako` and `pyyaml` defined in `vpython.toml`, so this wrapper re-invokes
   them under `vpython3 -vpython-spec`.
2. Running compiled build-time tools: GN `action()` targets can only execute a
   Python script directly, so this wrapper is also used to invoke compiled host
   binaries such as `glslang_validator`.
3. Capturing stdout (--capture-stdout): Several upstream Mesa codegen scripts
   print generated C/header source directly to stdout instead of accepting an
   output file flag. When `--capture-stdout <path>` is passed, this script
   captures the command's stdout and writes it to `<path>` only on success.
"""

import argparse
import os
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--capture-stdout",
                        help="Write command stdout to this file")
    parser.add_argument("--vpython-spec",
                        help="Run command via vpython3 with this spec")
    parser.add_argument("cmd", nargs=argparse.REMAINDER)
    args = parser.parse_args()

    cmd = args.cmd
    if cmd and cmd[0] == "--":
        cmd = cmd[1:]
    if not cmd:
        print("Error: no command specified", file=sys.stderr)
        return 1

    if args.vpython_spec:
        cmd = ["vpython3", "-vpython-spec", args.vpython_spec] + cmd

    if args.capture_stdout:
        res = subprocess.run(cmd, stdout=subprocess.PIPE)
        if res.returncode != 0:
            return res.returncode
        os.makedirs(
            os.path.dirname(os.path.abspath(args.capture_stdout)),
            exist_ok=True,
        )
        with open(args.capture_stdout, "wb") as f:
            f.write(res.stdout)
        return 0

    return subprocess.call(cmd)


if __name__ == "__main__":
    sys.exit(main())
