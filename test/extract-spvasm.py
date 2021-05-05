#!/usr/bin/env python3

# Copyright 2021 The Tint Authors.
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

# Extract SPIR-V assembly dumps from the output of
#    tint_unittests --dump-spirv
# Writes each module to a distinct filename, which is a sanitized
# form of the test name, and with a ".spvasm" suffix.
#
# Usage:
#    tint_unittests --dump-spirv | python3 extract-spvasm.py


import sys
import re

def extract():
    test_name = ''
    in_spirv = False
    parts = []
    for line in sys.stdin:
        run_match = re.match('\[ RUN\s+\]\s+(\S+)', line)
        if run_match:
            test_name = run_match.group(1)
            test_name = re.sub('[^0-9a-zA-Z]', '_', test_name) + '.spvasm'
        elif re.match('BEGIN ConvertedOk', line):
            parts = []
            in_spirv = True
        elif re.match('END ConvertedOk', line):
            with open(test_name, 'w') as f:
                f.write('; Test: ' + test_name + '\n')
                for l in parts:
                    f.write(l)
                f.close()
        elif in_spirv:
            parts.append(line)

def main(argv):
    if '--help' in argv or '-h' in argv:
        print('Extract SPIR-V from the output of tint_unittests --dump-spirv\n')
        print('Usage:\n    tint_unittests --dump-spirv | python3 extract-spvasm.py\n')
        print('Writes each module to a distinct filename, which is a sanitized')
        print('form of the test name, and with a ".spvasm" suffix.')
        return 1
    else:
        extract()
        return 0

if __name__ == '__main__':
    exit(main(sys.argv[1:]))
