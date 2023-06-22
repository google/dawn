#!/usr/bin/env python3

# Copyright 2023 The Dawn Authors
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

# This script implements CMake's 'configure_file':
# https://cmake.org/cmake/help/latest/command/configure_file.html

import os
import sys
import re

re_cmake_vars = re.compile(r'\${(\w+)}|@(\w+)@')
re_cmakedefine_var = re.compile(r'^#cmakedefine (\w+)$')
re_cmakedefine_var_value = re.compile(r'^#cmakedefine\b\s*(\w+)\s*(.*)')
re_cmakedefine01_var = re.compile(r'^#cmakedefine01\b\s*(\w+)')


def is_cmake_falsy(val):
    # See https://cmake.org/cmake/help/latest/command/if.html#basic-expressions
    return val.upper() in [
        '', '""', '0', 'OFF', 'NO', 'FALSE', 'N', 'IGNORE', 'NOTFOUND'
    ]


def main():
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    values_list = sys.argv[3:]

    # Build dictionary of values
    values = {}
    for v in values_list:
        k, v = v.split('=')
        if k in values:
            print(f'Duplicate key found in args: {k}')
            return -1
        values[k] = v

    # Make sure all keys are consumed
    unused_keys = set(values.keys())

    # Use this to look up keys in values so that unused_keys
    # is automatically updated.
    def lookup_value(key):
        r = values[key]
        unused_keys.discard(key)
        return r

    fin = open(input_file, 'r')

    output_lines = []

    for line in fin.readlines():
        # First replace all cmake vars in line with values
        while True:
            m = re.search(re_cmake_vars, line)
            if not m:
                break
            var_name = line[m.start():m.end()]
            key = m.group(1) or m.group(2)
            if key not in values:
                print(f"Key '{key}' not found in 'values'")
                return -1
            line = line.replace(var_name, lookup_value(key))

        # Handle '#cmakedefine VAR'
        m = re.search(re_cmakedefine_var, line)
        if m:
            var = m.group(1)
            if is_cmake_falsy(lookup_value(var)):
                line = f'/* #undef {var} */\n'
            else:
                line = f'#define {var}\n'
            output_lines.append(line)
            continue

        # Handle '#cmakedefine VAR VAL'
        m = re.search(re_cmakedefine_var_value, line)
        if m:
            var, val = m.group(1), m.group(2)
            if is_cmake_falsy(lookup_value(var)):
                line = f'/* #undef {var} */\n'
            else:
                line = f'#define {var} {val}\n'
            output_lines.append(line)
            continue

        # Handle '#cmakedefine01 VAR'
        m = re.search(re_cmakedefine01_var, line)
        if m:
            var = m.group(1)
            val = lookup_value(var)
            out_val = '0' if is_cmake_falsy(val) else '1'
            line = f'#define {var} {out_val}\n'
            output_lines.append(line)
            continue

        output_lines.append(line)

    if len(unused_keys) > 0:
        print(f'Unused keys in args: {unused_keys}')
        return -1

    output_text = ''.join(output_lines)

    # Avoid needless incremental rebuilds if the output file exists and hasn't changed
    if os.path.exists(output_file):
        with open(output_file, 'r') as fout:
            if fout.read() == output_text:
                return 0

    fout = open(output_file, 'w')
    fout.write(output_text)
    return 0


if __name__ == '__main__':
    sys.exit(main())
