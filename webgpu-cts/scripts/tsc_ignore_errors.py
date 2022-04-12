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

import subprocess
import sys
import os

from dir_paths import node_dir

try:
    old_sys_path = sys.path
    sys.path = [node_dir] + sys.path

    from node import GetBinaryPath as get_node_binary_path
finally:
    sys.path = old_sys_path

tsc = os.path.join(node_dir, 'node_modules', 'typescript', 'lib', 'tsc.js')


def run_tsc_ignore_errors(args):
    cmd = [get_node_binary_path(), tsc] + args
    process = subprocess.Popen(cmd,
                               cwd=os.getcwd(),
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()

    # Typecheck errors go in stdout, not stderr. If we see something in stderr, raise an error.
    if len(stderr):
        raise RuntimeError('tsc \'%s\' failed\n%s' % (' '.join(cmd), stderr))

    return stdout


if __name__ == '__main__':
    run_tsc_ignore_errors(sys.argv[1:])
