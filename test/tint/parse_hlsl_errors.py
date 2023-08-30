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

# Parses expected.*.hlsl files for errors and outputs a report.
#
# Usage:
#    parse_hlsl_erors

import glob
import re
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('--list-files', dest='list_files', action='store_true')
parser.add_argument('--no-list-files', dest='list_files', action='store_false')
parser.set_defaults(list_files=True)
args = parser.parse_args()

def add_error(error_to_file, error, file):
    if not error in error_to_file:
        error_to_file[error] = [file]
    else:
        error_to_file[error].append(file)

def find_and_print_errors(glob_pathname):
    files = glob.glob(glob_pathname, recursive=True)
    error_to_file = {}

    for f in files:
        found_error = False
        with open(f, "r") as fs:
            all_lines = fs.readlines()
            first_line = all_lines[0]
            if not first_line.startswith("SKIP:"):
                continue
            # The most refined errors are printed at the end, so search for error lines from bottom-up
            all_lines.reverse()
            for line in all_lines:
                m = re.search('.*\.hlsl:[0-9]+:.*?(error.*)', line)
                if m:
                    add_error(error_to_file, m.groups()[0], f)
                    found_error = True
                else:
                    m = re.search('error( X[0-9]+)*?:(.*)', line)
                    if m:
                        add_error(error_to_file, m.group(), f)
                        found_error = True
                    else:
                        if "exit status" in line:
                            add_error(error_to_file, line, f)
                            found_error = True
                if found_error:
                    break # Stop on first error string found

        if not found_error:
            # If no error message was found, add the SKIP line as it may contain the reason for skipping
            add_error(error_to_file, first_line.strip(), f)

    for error,files in sorted(error_to_file.items()):
        print('{} (count: {})'.format(error, len(files)))
        if args.list_files:
            for f in files:
                print('\t{}'.format(f))

print("=== FXC ===")
find_and_print_errors('./**/*.fxc.hlsl')

print("=== DXC ===")
find_and_print_errors('./**/*.dxc.hlsl')
