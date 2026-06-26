#!/usr/bin/env vpython3
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
"""Invokes the hermetic Bazelisk binary to build LiteRT-LM and copy the output binary to GN's output directory.
"""

import os
import shutil
import subprocess
import sys

BAZEL_TARGET = '//runtime/engine:litert_lm_advanced_main'
BAZEL_BIN_SUBPATH = 'runtime/engine/litert_lm_advanced_main'


def main():
    if len(sys.argv) < 2:
        print("Error: Missing output path argument.", file=sys.stderr)
        print("Usage: build_litert_lm.py <output_path>", file=sys.stderr)
        sys.exit(1)

    dest_path = os.path.abspath(sys.argv[1])

    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, '..', '..'))
    litert_lm_dir = os.path.join(script_dir, 'src')

    bazelisk_path = os.path.join(project_root, 'tools', 'bazelisk', 'bazelisk')
    prebuilt_cipd_dir = os.path.join(script_dir, 'data', 'prebuilt')
    prebuilt_src_dir = os.path.join(litert_lm_dir, 'prebuilt')
    backup_dir = prebuilt_src_dir + "_backup"

    # Back up the original Git LFS smudge prebuilt directory.
    if os.path.exists(
            prebuilt_src_dir) and not os.path.islink(prebuilt_src_dir):
        shutil.move(prebuilt_src_dir, backup_dir)

    try:
        # Symlink the CIPD-unpacked prebuilts directly so Bazel can resolve
        # targets.
        if os.path.exists(prebuilt_cipd_dir):
            print(
                "Symlinking prebuilt binaries from CIPD to LiteRT-LM workspace..."
            )
            os.symlink(prebuilt_cipd_dir, prebuilt_src_dir)

        # Compile the target using Bazelisk inside LiteRT-LM's standalone
        # workspace.
        print(f"Building Bazel target: {BAZEL_TARGET}...")
        build_cmd = [
            bazelisk_path,
            'build',
            '--compilation_mode=opt',
            '--define=litert_link_capi_so=true',
            '--define=resolve_symbols_in_exec=false',
            BAZEL_TARGET,
        ]

        proc = subprocess.run(build_cmd, cwd=litert_lm_dir)
        if proc.returncode != 0:
            print("Error: Bazel build failed.", file=sys.stderr)
            sys.exit(proc.returncode)

    finally:
        # Restore the original Git-tracked prebuilt directory regardless of
        # success or failure. This keeps the Git tree clean for gclient sync.
        print("Restoring original Git-tracked prebuilt directory...")
        if os.path.islink(prebuilt_src_dir):
            os.remove(prebuilt_src_dir)
        if os.path.exists(backup_dir):
            shutil.move(backup_dir, prebuilt_src_dir)

    # Locate and copy the compiled binary into the GN target directory.
    compiled_path = os.path.join(litert_lm_dir, 'bazel-bin', BAZEL_BIN_SUBPATH)
    if not os.path.exists(compiled_path):
        print(
            f"Error: Compiled binary not found at expected path: {compiled_path}",
            file=sys.stderr)
        sys.exit(1)

    print(f"Copying compiled binary to: {dest_path}")
    shutil.copy2(compiled_path, dest_path)
    os.chmod(dest_path, 0o755)

    print("Build and copy successful.")


if __name__ == '__main__':
    main()
