#!/usr/bin/env python3
# Copyright 2025 The Dawn & Tint Authors
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
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(__file__)
CMD_DIR = os.path.join(SCRIPT_DIR, 'src', 'cmd')
DAWN_ROOT = os.path.realpath(os.path.join(SCRIPT_DIR, '..'))
GO_BINARY = os.path.join(DAWN_ROOT, 'tools', 'golang', 'bin', 'go')
if sys.platform == 'win32':
    GO_BINARY += '.exe'


def _get_available_tools() -> list[str]:
    return os.listdir(CMD_DIR)


def main() -> int:
    parser = argparse.ArgumentParser(
        description='Builds and runs the specified Go tool in //tools/src/cmd')
    parser.add_argument(
        'tool_name',
        help='The name of the tool under //tools/src/cmd to run',
        choices=_get_available_tools())
    # Doing the opposite (passing in . for build and making other paths relative
    # to it) doesn't work properly in some cases. For example, "fuzz" doesn't
    # work properly when checking IR due to it not generating .tirb files due
    # to the -input value not matching the default WGSL corpus directory.
    parser.add_argument(
        '--append-cwd-as-build',
        action='store_true',
        help=('Automatically appends the current directory as a -build '
              'argument. Meant for use on bots.'))
    parser.add_argument('--isolated-script-test-output',
                        help='Currently unused, needed for bot support.')
    parser.add_argument('--isolated-script-test-perf-output',
                        help='Currently unused, needed for bot support.')
    args, unknown_args = parser.parse_known_args()

    cmd = [
        GO_BINARY,
        'run',
        os.path.join(CMD_DIR, args.tool_name),
    ]
    cmd.extend(unknown_args)
    if args.append_cwd_as_build:
        cmd.extend([
            '-build',
            os.getcwd(),
        ])

    proc = subprocess.run(cmd, check=False, cwd=DAWN_ROOT)
    return proc.returncode


if __name__ == '__main__':
    sys.exit(main())
